/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sys/socket.h>
#include <sys/types.h>

#include "netfirewall_rule_native_helper.h"
#include "net_manager_constants.h"
#include "netfirewall_db_helper.h"
#include "netmgr_ext_log_wrapper.h"
#include "netfirewall_rule_manager.h"
#include "netfirewall_policy_manager.h"
#include "netfirewall_default_rule_parser.h"
#include "netmanager_hitrace.h"
#include "os_account_manager.h"


namespace OHOS {
namespace NetManagerStandard {
NetFirewallRuleManager &NetFirewallRuleManager::GetInstance()
{
    static NetFirewallRuleManager instance;
    return instance;
}

NetFirewallRuleManager::NetFirewallRuleManager()
{
    NETMGR_EXT_LOG_I("NetFirewallRuleManager()");
}

NetFirewallRuleManager::~NetFirewallRuleManager()
{
    NETMGR_EXT_LOG_I("~NetFirewallRuleManager()");
}

int32_t NetFirewallRuleManager::AddNetFirewallRule(const sptr<NetFirewallRule> &rule, int32_t &ruleId)
{
    if (rule == nullptr) {
        NETMGR_EXT_LOG_E("AddNetFirewallRule rule is null");
        return FIREWALL_ERR_PARAMETER_ERROR;
    }
    return AddNetFirewallRule(rule, true, ruleId);
}

int32_t NetFirewallRuleManager::AddNetFirewallRule(const sptr<NetFirewallRule> &rule, bool isNotify, int32_t &ruleId)
{
    std::lock_guard<std::shared_mutex> locker(setFirewallRuleMutex_);
    int32_t ret = CheckRuleConstraint(rule);
    if (ret != FIREWALL_OK) {
        NETMGR_EXT_LOG_E("addNetFirewallRule error code=%{public}d", ret);
    } else {
        ruleId = NetFirewallDbHelper::GetInstance().AddFirewallRuleRecord(*rule);
        NETMGR_EXT_LOG_I("AddNetFirewallRule:: dbRuleId: %{public}d.", ruleId);
        if (ruleId < 0) {
            ret = FIREWALL_ERR_INTERNAL;
        } else {
            allUserRule_++;
            UpdateUserRuleSize(rule->userId, true);
        }
    }
    if (ret == FIREWALL_OK && isNotify && rule->isEnabled) {
        ret = DistributeRulesToNative(rule->ruleType);
    }
    return ret;
}

int32_t NetFirewallRuleManager::AddDefaultNetFirewallRule(int32_t userId)
{
    if (!userRuleSize_.empty() && userRuleSize_.count(userId) && userRuleSize_.at(userId) > 0) {
        NETMGR_EXT_LOG_W("AddDefaultNetFirewallRule , current user rule is exist.");
        return 0;
    }
    std::vector<sptr<NetFirewallRule>> rules;
    NetFirewallDefaultRuleParser::GetDefaultRules(rules);
    if (rules.empty()) {
        return FIREWALL_SUCCESS;
    }
    maxDefaultRuleSize_ = static_cast<int64_t>(rules.size());

    int32_t ret = FIREWALL_OK;
    int32_t ruleId = 0;
    for (const auto &rule : rules) {
        ret = AddNetFirewallRule(rule, false, ruleId);
        if (ret != FIREWALL_SUCCESS) {
            NETMGR_EXT_LOG_W("AddDefaultNetFirewallRule error, ret=%{public}d", ret);
            return ret;
        }
    }
    std::lock_guard<std::shared_mutex> locker(setFirewallRuleMutex_);
    return DistributeRulesToNative();
}

int32_t NetFirewallRuleManager::UpdateNetFirewallRule(const sptr<NetFirewallRule> &rule)
{
    if (rule == nullptr) {
        NETMGR_EXT_LOG_E("UpdateNetFirewallRule rule is null");
        return FIREWALL_ERR_PARAMETER_ERROR;
    }
    NETMGR_EXT_LOG_I("UpdateNetFirewallRule");
    std::lock_guard<std::shared_mutex> locker(setFirewallRuleMutex_);
    NetFirewallRule oldRule;
    int32_t ret = CheckRuleConstraint(rule);
    if (ret == FIREWALL_SUCCESS) {
        ret = CheckRuleExist(rule->ruleId, oldRule);
        if (ret == FIREWALL_SUCCESS) {
            ret = NetFirewallDbHelper::GetInstance().UpdateFirewallRuleRecord(*rule);
        }
    }
    if (ret != FIREWALL_SUCCESS) {
        return ret;
    }
    if (oldRule.ruleId <= 0 || (!oldRule.isEnabled && !rule->isEnabled)) {
        return FIREWALL_SUCCESS;
    }
    if (oldRule.isEnabled && (rule->ruleType != oldRule.ruleType || !rule->isEnabled)) {
        ret = DistributeRulesToNative(oldRule.ruleType);
    }
    if (rule->isEnabled) {
        ret += DistributeRulesToNative(rule->ruleType);
    }
    return ret;
}

int32_t NetFirewallRuleManager::DeleteNetFirewallRule(const int32_t userId, const int32_t ruleId)
{
    NETMGR_EXT_LOG_I("DeleteNetFirewallRule");
    std::lock_guard<std::shared_mutex> locker(setFirewallRuleMutex_);
    int32_t ret = CheckUserExist(userId);
    if (ret != FIREWALL_SUCCESS) {
        return ret;
    }

    NetFirewallRule oldRule;
    ret = CheckRuleExist(ruleId, oldRule);
    if (ret != FIREWALL_SUCCESS) {
        return ret;
    }
    ret = NetFirewallDbHelper::GetInstance().DeleteFirewallRuleRecord(userId, ruleId);
    if (ret != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("DeleteFirewallRuleRecord error");
        return FIREWALL_ERR_INTERNAL;
    }
    allUserRule_--;
    if (oldRule.ruleId > 0) {
        UpdateUserRuleSize(userId, false);
    }
    if (oldRule.ruleId <= 0 || !oldRule.isEnabled) {
        return FIREWALL_SUCCESS;
    }
    return DistributeRulesToNative(oldRule.ruleType);
}

int32_t NetFirewallRuleManager::DeleteNetFirewallRuleByUserId(const int32_t userId)
{
    NETMGR_EXT_LOG_I("DeleteNetFirewallRule");
    std::lock_guard<std::shared_mutex> locker(setFirewallRuleMutex_);
    int32_t ret = NetFirewallDbHelper::GetInstance().DeleteFirewallRuleRecordByUserId(userId);
    if (ret != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("DeleteFirewallRuleRecord error");
        return FIREWALL_ERR_INTERNAL;
    }
    // reset
    allUserRule_ = userRuleSize_.count(userId) ? (allUserRule_ - userRuleSize_.at(userId)) : 0;
    DeleteUserRuleSize(userId);
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallRuleManager::DeleteNetFirewallRuleByAppId(const int32_t appUid)
{
    NETMGR_EXT_LOG_I("DeleteNetFirewallRuleByAppId");
    std::lock_guard<std::shared_mutex> locker(setFirewallRuleMutex_);
    std::vector<NetFirewallRule> rules;
    NetFirewallDbHelper::GetInstance().QueryEnabledFirewallRules(GetCurrentAccountId(), appUid, rules);
    if (rules.empty()) {
        NETMGR_EXT_LOG_I("DeleteNetFirewallRuleByAppId: current appUid has no rule");
        return FIREWALL_SUCCESS;
    }
    int32_t ret = NetFirewallDbHelper::GetInstance().DeleteFirewallRuleRecordByAppId(appUid);
    if (ret != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("DeleteNetFirewallRuleByAppId error");
        return FIREWALL_ERR_INTERNAL;
    }
    allUserRule_ = 0;
    userRuleSize_.clear();
    bool hasEnabledIpRule = false;
    bool hasEnabledDomainRule = false;
    bool hasEnabledDnsRule = false;
    for (const auto &rule : rules) {
        if (!rule.isEnabled) {
            continue;
        }
        if (rule.ruleType == NetFirewallRuleType::RULE_DNS) {
            hasEnabledDnsRule = true;
        } else if (rule.ruleType == NetFirewallRuleType::RULE_DOMAIN) {
            hasEnabledDomainRule = true;
        } else if (rule.ruleType == NetFirewallRuleType::RULE_IP) {
            hasEnabledIpRule = true;
        }
    }
    if (hasEnabledDnsRule) {
        ret = DistributeRulesToNative(NetFirewallRuleType::RULE_DNS);
    }
    if (hasEnabledDomainRule) {
        ret += DistributeRulesToNative(NetFirewallRuleType::RULE_DOMAIN);
    }
    if (hasEnabledIpRule) {
        ret += DistributeRulesToNative(NetFirewallRuleType::RULE_IP);
    }
    return ret;
}

int32_t NetFirewallRuleManager::GetEnabledNetFirewallRules(std::vector<NetFirewallRule> &ruleList,
    NetFirewallRuleType type)
{
    NETMGR_EXT_LOG_I("GetEnabledNetFirewallRules:: type=%{public}d", static_cast<int32_t>(type));
    int32_t ret = NetFirewallDbHelper::GetInstance().QueryAllUserEnabledFirewallRules(ruleList, type);
    if (ret < 0) {
        NETMGR_EXT_LOG_E("GetEnabledNetFirewallRules error");
        return FIREWALL_ERR_INTERNAL;
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallRuleManager::GetNetFirewallRules(const int32_t userId, const sptr<RequestParam> &requestParam,
    sptr<FirewallRulePage> &info)
{
    if (requestParam == nullptr) {
        NETMGR_EXT_LOG_E("GetNetFirewallRules requestParam is null");
        return FIREWALL_ERR_PARAMETER_ERROR;
    }
    if (info == nullptr) {
        NETMGR_EXT_LOG_E("GetNetFirewallRules info is null");
        return FIREWALL_ERR_INTERNAL;
    }
    NETMGR_EXT_LOG_I("GetNetFirewallRules");
    std::shared_lock<std::shared_mutex> locker(setFirewallRuleMutex_);
    int32_t ret = CheckUserExist(userId);
    if (ret != FIREWALL_SUCCESS) {
        return ret;
    }
    info->page = requestParam->page;
    info->pageSize = requestParam->pageSize;
    ret = NetFirewallDbHelper::GetInstance().QueryFirewallRule(userId, requestParam, info);
    if (ret < 0) {
        NETMGR_EXT_LOG_E("QueryAllFirewallRuleRecord error");
        return FIREWALL_ERR_INTERNAL;
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallRuleManager::GetNetFirewallRule(const int32_t userId, const int32_t ruleId,
    sptr<NetFirewallRule> &rule)
{
    if (rule == nullptr) {
        NETMGR_EXT_LOG_E("GetNetFirewallRule rule is null");
        return FIREWALL_ERR_INTERNAL;
    }
    NETMGR_EXT_LOG_I("GetNetFirewallRule userId=%{public}d ruleId=%{public}d", userId, ruleId);
    std::shared_lock<std::shared_mutex> locker(setFirewallRuleMutex_);
    int32_t ret = CheckUserExist(userId);
    if (ret != FIREWALL_SUCCESS) {
        return ret;
    }

    std::vector<struct NetFirewallRule> outRules;
    ret = NetFirewallDbHelper::GetInstance().QueryFirewallRuleRecord(ruleId, userId, outRules);
    if (ret < 0) {
        NETMGR_EXT_LOG_E("QueryFirewallRuleRecord error");
        return FIREWALL_ERR_INTERNAL;
    }
    if (outRules.size() > 0) {
        const NetFirewallRule &outRule = outRules[0];
        rule->userId = outRule.userId;
        rule->ruleId = outRule.ruleId;
        rule->ruleName = outRule.ruleName;
        rule->ruleDescription = outRule.ruleDescription;
        rule->ruleAction = outRule.ruleAction;
        rule->ruleDirection = outRule.ruleDirection;
        rule->ruleType = outRule.ruleType;
        rule->appUid = outRule.appUid;
        rule->protocol = outRule.protocol;
        rule->dns = outRule.dns;
        rule->localIps.assign(outRule.localIps.begin(), outRule.localIps.end());
        rule->remoteIps.assign(outRule.remoteIps.begin(), outRule.remoteIps.end());
        rule->localPorts.assign(outRule.localPorts.begin(), outRule.localPorts.end());
        rule->remotePorts.assign(outRule.remotePorts.begin(), outRule.remotePorts.end());
    } else {
        NETMGR_EXT_LOG_E("QueryFirewallRuleRecord size is 0");
        return FIREWALL_ERR_NO_RULE;
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallRuleManager::CheckUserExist(const int32_t userId)
{
    AccountSA::OsAccountInfo accountInfo;
    if (AccountSA::OsAccountManager::QueryOsAccountById(userId, accountInfo) != ERR_OK) {
        NETMGR_EXT_LOG_E("QueryOsAccountById error, userId: %{public}d.", userId);
        return FIREWALL_ERR_NO_USER;
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallRuleManager::CheckRuleExist(const int32_t ruleId, NetFirewallRule &oldRule)
{
    bool isExist = NetFirewallDbHelper::GetInstance().IsFirewallRuleExist(ruleId, oldRule);
    if (!isExist) {
        NETMGR_EXT_LOG_E("Query ruleId: %{public}d is not exist.", ruleId);
        return FIREWALL_ERR_NO_RULE;
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallRuleManager::GetAllRuleConstraint(const int32_t userId)
{
    int64_t rowCount = 0;
    if (allUserRule_ <= 0) {
        NetFirewallDbHelper::GetInstance().QueryFirewallRuleAllCount(rowCount);
        allUserRule_ = rowCount;
    }
    if (!userRuleSize_.count(userId)) {
        rowCount = 0;
        NetFirewallDbHelper::GetInstance().QueryFirewallRuleByUserIdCount(userId, rowCount);
        userRuleSize_.insert({ userId, rowCount });
    }
    allUserDomain_ = NetFirewallDbHelper::GetInstance().QueryFirewallRuleAllDomainCount();
    NETMGR_EXT_LOG_I(
        "GetAllRuleConstraint userId=%{public}d rowCount=%{public}d allUserRule=%{public}d allUserDomain=%{public}d",
        userId, static_cast<int32_t>(userRuleSize_.at(userId)), static_cast<int32_t>(allUserRule_), allUserDomain_);
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallRuleManager::CheckRuleConstraint(const sptr<NetFirewallRule> &rule)
{
    int32_t ret = CheckUserExist(rule->userId);
    if (ret != FIREWALL_SUCCESS) {
        return ret;
    }
    int32_t userId = rule->userId;
    GetAllRuleConstraint(userId);
    if (userRuleSize_.at(userId) < maxDefaultRuleSize_) {
        NETMGR_EXT_LOG_I("current user db size is not max than default rule size, ignore check.");
        return FIREWALL_SUCCESS;
    }

    if (allUserRule_ + 1 > FIREWALL_ALL_USER_MAX_RULE || userRuleSize_.at(userId) + 1 > FIREWALL_USER_MAX_RULE) {
        NETMGR_EXT_LOG_E("check rule constraint error, rule is large.");
        return FIREWALL_ERR_EXCEED_MAX_RULE;
    }
    int32_t domainsCount = NetFirewallDbHelper::GetInstance().QueryFirewallRuleDomainByUserIdCount(userId);
    int32_t size = static_cast<int32_t>(rule->domains.size());
    if (domainsCount + size > FIREWALL_SINGLE_USER_MAX_DOMAIN) {
        return FIREWALL_ERR_EXCEED_MAX_DOMAIN;
    }
    domainsCount = NetFirewallDbHelper::GetInstance().QueryFirewallRuleAllFuzzyDomainCount();
    if (allUserDomain_ + size > FIREWALL_ALL_USER_MAX_DOMAIN ||
        domainsCount + size > FIREWALL_ALL_USER_MAX_FUZZY_DOMAIN) {
        NETMGR_EXT_LOG_E(
            "check rule constraint domain number is more than max, all domain=%{public}d all fuzzy=%{public}d",
            allUserDomain_, static_cast<int32_t>(domainsCount));
        return FIREWALL_ERR_EXCEED_ALL_MAX_DOMAIN;
    }
    // DNS rule check duplicate
    if (NetFirewallDbHelper::GetInstance().IsDnsRuleExist(rule)) {
        NETMGR_EXT_LOG_E("check rule constraint, the dns rule is exist");
        return FIREWALL_ERR_DNS_RULE_DUPLICATION;
    }
    return FIREWALL_SUCCESS;
}

bool NetFirewallRuleManager::CheckAccountExist(int32_t userId)
{
    AccountSA::OsAccountInfo accountInfo;
    if (AccountSA::OsAccountManager::QueryOsAccountById(userId, accountInfo) != ERR_OK) {
        NETMGR_EXT_LOG_E("QueryOsAccountById error, userId: %{public}d.", userId);
        return false;
    }

    if (accountInfo.GetType() == AccountSA::OsAccountType::GUEST) {
        NETMGR_EXT_LOG_W("The guest account.");
    }
    return true;
}

bool NetFirewallRuleManager::ExtractIpRules(const std::vector<NetFirewallRule> &rules,
    std::vector<sptr<NetFirewallIpRule>> &ipRules)
{
    if (rules.empty()) {
        return false;
    }
    for (const auto &rule : rules) {
        if (!NetFirewallPolicyManager::GetInstance().GetNetFirewallStatus(rule.userId)) {
            continue;
        }
        if (rule.ruleType != NetFirewallRuleType::RULE_IP) {
            continue;
        }
        sptr<NetFirewallIpRule> ipRule = new (std::nothrow) NetFirewallIpRule();
        if (ipRule == nullptr) {
            NETMGR_EXT_LOG_E("ExtractIpRules ipRule is null");
            return false;
        }
        ipRule->userId = rule.userId;
        ipRule->ruleDirection = rule.ruleDirection;
        ipRule->ruleAction = rule.ruleAction;
        ipRule->appUid = rule.appUid;
        ipRule->localIps = rule.localIps;
        ipRule->remoteIps = rule.remoteIps;
        ipRule->protocol = rule.protocol;
        ipRule->localPorts = rule.localPorts;
        ipRule->remotePorts = rule.remotePorts;
        ipRules.emplace_back(std::move(ipRule));
    }
    return ipRules.size() > 0;
}

bool NetFirewallRuleManager::ExtractDomainRules(const std::vector<NetFirewallRule> &rules,
    std::vector<sptr<NetFirewallDomainRule>> &domainRules)
{
    if (rules.empty()) {
        return false;
    }
    for (const auto &rule : rules) {
        if (!NetFirewallPolicyManager::GetInstance().GetNetFirewallStatus(rule.userId)) {
            continue;
        }
        if (rule.ruleType != NetFirewallRuleType::RULE_DOMAIN) {
            continue;
        }
        sptr<NetFirewallDomainRule> domainRule = new (std::nothrow) NetFirewallDomainRule();
        if (domainRule == nullptr) {
            NETMGR_EXT_LOG_E("ExtractDomainRules domainRule is null");
            return false;
        }
        domainRule->userId = rule.userId;
        domainRule->appUid = rule.appUid;
        domainRule->ruleAction = rule.ruleAction;
        domainRule->domains = rule.domains;
        domainRules.emplace_back(std::move(domainRule));
    }
    return domainRules.size() > 0;
}

bool NetFirewallRuleManager::ExtractDnsRules(const std::vector<NetFirewallRule> &rules,
    std::vector<sptr<NetFirewallDnsRule>> &dnsRules)
{
    if (rules.empty()) {
        return false;
    }
    for (const auto &rule : rules) {
        if (!NetFirewallPolicyManager::GetInstance().GetNetFirewallStatus(rule.userId)) {
            continue;
        }
        if (rule.ruleType != NetFirewallRuleType::RULE_DNS) {
            continue;
        }
        sptr<NetFirewallDnsRule> dnsRule = new (std::nothrow) NetFirewallDnsRule();
        if (dnsRule == nullptr) {
            NETMGR_EXT_LOG_E("ExtractDnsRules dnsRule is null");
            return false;
        }
        dnsRule->userId = rule.userId;
        dnsRule->appUid = rule.appUid;
        dnsRule->primaryDns = rule.dns.primaryDns;
        dnsRule->standbyDns = rule.dns.standbyDns;
        dnsRules.emplace_back(std::move(dnsRule));
    }
    return dnsRules.size() > 0;
}

int32_t NetFirewallRuleManager::HandleIpTypeForDistributeRules(std::vector<NetFirewallRule> &rules)
{
    std::vector<sptr<NetFirewallIpRule>> ipRules;
    if (ExtractIpRules(rules, ipRules)) {
        NetFirewallRuleNativeHelper::GetInstance().SetFirewallIpRules(ipRules);
    } else {
        NetFirewallRuleNativeHelper::GetInstance().ClearFirewallRules(NetFirewallRuleType::RULE_IP);
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallRuleManager::HandleDnsTypeForDistributeRules(std::vector<NetFirewallRule> &rules)
{
    std::vector<sptr<NetFirewallDnsRule>> dnsRules;
    if (ExtractDnsRules(rules, dnsRules)) {
        NetFirewallRuleNativeHelper::GetInstance().SetFirewallDnsRules(dnsRules);
    } else {
        NetFirewallRuleNativeHelper::GetInstance().ClearFirewallRules(NetFirewallRuleType::RULE_DNS);
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallRuleManager::HandleDomainTypeForDistributeRules(std::vector<NetFirewallRule> &rules)
{
    std::vector<sptr<NetFirewallDomainRule>> domainRules;
    NetFirewallRuleNativeHelper::GetInstance().ClearFirewallRules(NetFirewallRuleType::RULE_DOMAIN);
    if (ExtractDomainRules(rules, domainRules)) {
        NetFirewallRuleNativeHelper::GetInstance().SetFirewallDomainRules(domainRules);
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallRuleManager::GetCurrentAccountId()
{
    std::vector<int32_t> accountIds;
    auto ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(accountIds);
    if (ret != ERR_OK || accountIds.empty()) {
        NETMGR_EXT_LOG_E("query active user failed errCode=%{public}d", ret);
        return FIREWALL_ERR_INTERNAL;
    }
    return accountIds.front();
}

int32_t NetFirewallRuleManager::OpenOrCloseNativeFirewall(bool isOpen)
{
    std::lock_guard<std::shared_mutex> locker(setFirewallRuleMutex_);
    NETMGR_EXT_LOG_I("OpenOrCloseNativeFirewall: isOpen=%{public}d", isOpen);
    NetmanagerHiTrace::NetmanagerStartSyncTrace("OpenOrCloseNativeFirewall");
    if (!isOpen) {
        NETMGR_EXT_LOG_I("OpenOrCloseNativeFirewall: firewall disabled");
        NetFirewallRuleNativeHelper::GetInstance().ClearFirewallRules(NetFirewallRuleType::RULE_ALL);
        return FIREWALL_SUCCESS;
    }
    NetFirewallPolicyManager::GetInstance().InitNetfirewallPolicy();
    int32_t ret = SetRulesToNativeByType(NetFirewallRuleType::RULE_ALL);
    SetNetFirewallDumpMessage(ret);
    NetmanagerHiTrace::NetmanagerFinishSyncTrace("OpenOrCloseNativeFirewall");
    return ret;
}

int32_t NetFirewallRuleManager::DistributeRulesToNative(NetFirewallRuleType type)
{
    NETMGR_EXT_LOG_I("DistributeRulesToNative: type=%{public}d", (int)type);
    NetmanagerHiTrace::NetmanagerStartSyncTrace("DistributeRulesToNative");
    if (!NetFirewallPolicyManager::GetInstance().IsFirewallOpen()) {
        NETMGR_EXT_LOG_I("DistributeRulesToNative: firewall disabled");
        NetFirewallRuleNativeHelper::GetInstance().ClearFirewallRules(NetFirewallRuleType::RULE_ALL);
        return FIREWALL_SUCCESS;
    }

    int32_t ret = SetRulesToNativeByType(type);
    NetmanagerHiTrace::NetmanagerFinishSyncTrace("DistributeRulesToNative");
    SetNetFirewallDumpMessage(ret);
    return ret;
}

int32_t NetFirewallRuleManager::SetRulesToNativeByType(const NetFirewallRuleType type)
{
    int32_t ret = FIREWALL_SUCCESS;
    std::vector<NetFirewallRule> rules;
    GetEnabledNetFirewallRules(rules, type);
    switch (type) {
        case NetFirewallRuleType::RULE_IP:
            ret = HandleIpTypeForDistributeRules(rules);
            break;
        case NetFirewallRuleType::RULE_DNS:
            ret = HandleDnsTypeForDistributeRules(rules);
            break;
        case NetFirewallRuleType::RULE_DOMAIN:
            ret = HandleDomainTypeForDistributeRules(rules);
            break;
        case NetFirewallRuleType::RULE_ALL: {
            if (rules.empty()) {
                break;
            }
            NetFirewallRuleNativeHelper::GetInstance().ClearFirewallRules(NetFirewallRuleType::RULE_ALL);
            // set default action to bpf map
            NetFirewallPolicyManager::GetInstance().InitNetfirewallPolicy();
            ret = HandleIpTypeForDistributeRules(rules);
            ret += HandleDnsTypeForDistributeRules(rules);
            ret += HandleDomainTypeForDistributeRules(rules);
            break;
        }
        default:
            break;
    }
    return ret;
}

void NetFirewallRuleManager::UpdateUserRuleSize(const int32_t userId, bool isInc)
{
    if (!userRuleSize_.count(userId)) {
        return;
    }
    int64_t old = userRuleSize_.at(userId);
    userRuleSize_.at(userId) = isInc ? (old + 1) : (old - 1);
}

void NetFirewallRuleManager::DeleteUserRuleSize(const int32_t userId)
{
    if (!userRuleSize_.empty() && userRuleSize_.count(userId)) {
        userRuleSize_.erase(userId);
    }
}

void NetFirewallRuleManager::SetNetFirewallDumpMessage(const int32_t result)
{
    if (result == FIREWALL_SUCCESS) {
        currentSetRuleSecond_ = GetCurrentMilliseconds();
    }
    lastRulePushResult_ = result;
}

uint64_t NetFirewallRuleManager::GetCurrentSetRuleSecond()
{
    return currentSetRuleSecond_;
}

int64_t NetFirewallRuleManager::GetLastRulePushResult()
{
    return lastRulePushResult_;
}
} // namespace NetManagerStandard
} // namespace OHOS
