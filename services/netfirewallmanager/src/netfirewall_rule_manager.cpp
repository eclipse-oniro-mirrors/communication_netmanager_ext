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
std::shared_ptr<NetFirewallRuleManager> NetFirewallRuleManager::instance_ = nullptr;

std::shared_ptr<NetFirewallRuleManager> NetFirewallRuleManager::GetInstance()
{
    static std::mutex instanceMutex;
    std::lock_guard<std::mutex> guard(instanceMutex);
    if (instance_ == nullptr) {
        instance_.reset(new NetFirewallRuleManager());
    }
    return instance_;
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
    return AddNetFirewallRule(rule, true, ruleId);
}

int32_t NetFirewallRuleManager::AddNetFirewallRule(const sptr<NetFirewallRule> &rule, bool isNotify, int32_t &ruleId)
{
    std::lock_guard<std::shared_mutex> locker(setFirewallRuleMutex_);
    int32_t ret = CheckRuleConstraint(rule);
    if (ret != FIREWALL_OK) {
        NETMGR_EXT_LOG_E("addNetFirewallRule error code=%{public}d", ret);
        return ret;
    }
    ruleId = NetFirewallDbHelper::GetInstance()->AddFirewallRuleRecord(*rule);
    NETMGR_EXT_LOG_I("AddNetFirewallRule:: dbRuleId: %{public}d.", ruleId);
    if (ruleId < 0) {
        return FIREWALL_ERR_INTERNAL;
    }
    if (isNotify && rule->isEnabled) {
        return AddFirewallRuleToNative(*rule, ruleId);
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallRuleManager::AddDefaultNetFirewallRule(int32_t userId)
{
    if (!userRuleSize_.empty() && userRuleSize_.count(userId) && userRuleSize_.at(userId) > 0) {
        NETMGR_EXT_LOG_W("AddDefaultNetFirewallRule , current user rule is exits.");
        return 0;
    }
    std::vector<sptr<NetFirewallRule>> rules;
    NetFirewallDefaultRuleParser::GetDefaultRules(rules);
    if (rules.empty()) {
        return FIREWALL_SUCCESS;
    }
    maxDefaultRuleSize_ = rules.size();

    int32_t ret = FIREWALL_OK;
    for (const auto &rule : rules) {
        int32_t ruleId = 0;
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
    NETMGR_EXT_LOG_I("UpdateNetFirewallRule");
    std::lock_guard<std::shared_mutex> locker(setFirewallRuleMutex_);
    int32_t ret = CheckRuleConstraint(rule);
    if (ret != FIREWALL_OK) {
        return ret;
    }
    ret = CheckUserExsist(rule->userId);
    if (ret != FIREWALL_SUCCESS) {
        return ret;
    }
    NetFirewallRule oldRule;
    ret = CheckRuleExits(rule->ruleId, oldRule);
    if (ret != FIREWALL_SUCCESS) {
        return ret;
    }
    NetFirewallDbHelper::GetInstance()->UpdateFirewallRuleRecord(*rule);
    if (oldRule.ruleId <= 0 || (!oldRule.isEnabled && !rule->isEnabled)) {
        return FIREWALL_SUCCESS;
    }
    if (oldRule.isEnabled && (rule->ruleType != oldRule.ruleType || !rule->isEnabled)) {
        ret = DeleteNativeFirewallRule(oldRule);
    }
    if (rule->isEnabled) {
        ret += UpdateNativeFirewallRule(*rule);
    }
    return ret;
}

int32_t NetFirewallRuleManager::DeleteNetFirewallRule(const int32_t userId, const int32_t ruleId)
{
    NETMGR_EXT_LOG_I("DeleteNetFirewallRule");
    std::lock_guard<std::shared_mutex> locker(setFirewallRuleMutex_);
    int32_t ret = CheckUserExsist(userId);
    if (ret != FIREWALL_SUCCESS) {
        return ret;
    }

    NetFirewallRule oldRule;
    ret = CheckRuleExits(ruleId, oldRule);
    if (ret != FIREWALL_SUCCESS) {
        return ret;
    }
    ret = NetFirewallDbHelper::GetInstance()->DeleteFirewallRuleRecord(userId, ruleId);
    if (ret != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("DeleteFirewallRuleRecord error");
        return FIREWALL_ERR_INTERNAL;
    }

    if (oldRule.ruleId <= 0 || !oldRule.isEnabled) {
        return FIREWALL_SUCCESS;
    }
    return DeleteNativeFirewallRule(oldRule);
}

int32_t NetFirewallRuleManager::DeleteNetFirewallRuleByUserId(const int32_t userId)
{
    NETMGR_EXT_LOG_I("DeleteNetFirewallRule");
    std::lock_guard<std::shared_mutex> locker(setFirewallRuleMutex_);
    int32_t ret = NetFirewallDbHelper::GetInstance()->DeleteFirewallRuleRecordByUserId(userId);
    if (ret != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("DeleteFirewallRuleRecord error");
        return FIREWALL_ERR_INTERNAL;
    }

    return FIREWALL_SUCCESS;
}

int32_t NetFirewallRuleManager::DeleteNetFirewallRuleByAppId(const int32_t appUid)
{
    NETMGR_EXT_LOG_I("DeleteNetFirewallRuleByAppId");
    std::lock_guard<std::shared_mutex> locker(setFirewallRuleMutex_);
    std::shared_ptr<NetFirewallDbHelper> helper = NetFirewallDbHelper::GetInstance();
    std::vector<NetFirewallRule> rules;
    helper->QueryEnabledFirewallRules(GetCurrentAccountId(), appUid, rules);
    if (rules.empty()) {
        NETMGR_EXT_LOG_I("DeleteNetFirewallRuleByAppId: current appUid has no rule");
        return FIREWALL_SUCCESS;
    }
    int32_t ret = helper->DeleteFirewallRuleRecordByAppId(appUid);
    if (ret != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("DeleteNetFirewallRuleByAppId error");
        return FIREWALL_ERR_INTERNAL;
    }

    std::vector<int32_t> enabledIpRules;
    std::vector<int32_t> enabledDomainRules;
    bool hasEnabledDnsRule = false;
    for (const auto &rule : rules) {
        if (!rule.isEnabled) {
            continue;
        }
        if (rule.ruleType == NetFirewallRuleType::RULE_DNS) {
            hasEnabledDnsRule = true;
        } else if (rule.ruleType == NetFirewallRuleType::RULE_DOMAIN) {
            enabledDomainRules.emplace_back(rule.ruleId);
        } else if (rule.ruleType == NetFirewallRuleType::RULE_IP) {
            enabledIpRules.emplace_back(rule.ruleId);
        }
    }
    if (hasEnabledDnsRule) {
        ret = DistributeRulesToNative(NetFirewallRuleType::RULE_DNS);
    }
    if (!enabledDomainRules.empty()) {
        return NetFirewallRuleNativeHelper::GetInstance()->DeleteFirewallRules(NetFirewallRuleType::RULE_DOMAIN,
            enabledDomainRules);
    }
    if (!enabledIpRules.empty()) {
        return NetFirewallRuleNativeHelper::GetInstance()->DeleteFirewallRules(NetFirewallRuleType::RULE_IP,
            enabledIpRules);
    }
    return ret;
}

int32_t NetFirewallRuleManager::GetEnabledNetFirewallRules(const int32_t userId, std::vector<NetFirewallRule> &ruleList,
    NetFirewallRuleType type)
{
    NETMGR_EXT_LOG_I("GetEnabledNetFirewallRules:: type=%{public}d", static_cast<int32_t>(type));
    int32_t ret = CheckUserExsist(userId);
    if (ret != FIREWALL_SUCCESS) {
        return ret;
    }
    ret = NetFirewallDbHelper::GetInstance()->QueryAllUserEnabledFirewallRules(ruleList, type);
    if (ret < 0) {
        NETMGR_EXT_LOG_E("GetEnabledNetFirewallRules error");
        return FIREWALL_ERR_INTERNAL;
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallRuleManager::GetNetFirewallRules(const int32_t userId, const sptr<RequestParam> &requestParam,
    sptr<FirewallRulePage> &info)
{
    NETMGR_EXT_LOG_I("GetNetFirewallRules");
    std::shared_lock<std::shared_mutex> locker(setFirewallRuleMutex_);
    int32_t ret = CheckUserExsist(userId);
    if (ret != FIREWALL_SUCCESS) {
        return ret;
    }
    info->page = requestParam->page;
    info->pageSize = requestParam->pageSize;
    ret = NetFirewallDbHelper::GetInstance()->QueryFirewallRule(userId, requestParam, info);
    if (ret < 0) {
        NETMGR_EXT_LOG_E("QueryAllFirewallRuleRecord error");
        return FIREWALL_ERR_INTERNAL;
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallRuleManager::GetNetFirewallRule(const int32_t userId, const int32_t ruleId,
    sptr<NetFirewallRule> &rule)
{
    NETMGR_EXT_LOG_I("GetNetFirewallRule userId=%{public}d ruleId=%{public}d", userId, ruleId);
    std::shared_lock<std::shared_mutex> locker(setFirewallRuleMutex_);
    int32_t ret = CheckUserExsist(userId);
    if (ret != FIREWALL_SUCCESS) {
        return ret;
    }

    std::vector<struct NetFirewallRule> outRules;
    ret = NetFirewallDbHelper::GetInstance()->QueryFirewallRuleRecord(ruleId, userId, outRules);
    if (ret < 0) {
        NETMGR_EXT_LOG_E("QueryFirewallRuleRecord error");
        return FIREWALL_ERR_INTERNAL;
    }
    if (outRules.size() > 0) {
        rule = sptr<NetFirewallRule>(&(outRules[0]));
    } else {
        NETMGR_EXT_LOG_E("QueryFirewallRuleRecord size is 0");
        return FIREWALL_ERR_NO_RULE;
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallRuleManager::CheckUserExsist(const int32_t userId)
{
    AccountSA::OsAccountInfo accountInfo;
    if (AccountSA::OsAccountManager::QueryOsAccountById(userId, accountInfo) != ERR_OK) {
        NETMGR_EXT_LOG_E("QueryOsAccountById error, userId: %{public}d.", userId);
        return FIREWALL_ERR_NO_USER;
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallRuleManager::CheckRuleExits(const int32_t ruleId, NetFirewallRule &oldRule)
{
    std::shared_ptr<NetFirewallDbHelper> helper = NetFirewallDbHelper::GetInstance();
    bool isExits = helper->IsFirewallRuleExits(ruleId, oldRule);
    if (!isExits) {
        NETMGR_EXT_LOG_E("Query ruleId: %{public}d is not exits.", ruleId);
        return FIREWALL_ERR_NO_RULE;
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallRuleManager::GetAllRuleConstraint(const int32_t userId)
{
    std::shared_ptr<NetFirewallDbHelper> helper = NetFirewallDbHelper::GetInstance();
    helper->QueryFirewallRuleAllCount(allUserRule_);
    int64_t rowCount = 0;
    helper->QueryFirewallRuleByUserIdCount(userId, rowCount);
    helper->QueryFirewallRuleAllDomainCount(allUserDomain_);
    if (userRuleSize_.count(userId)) {
        userRuleSize_.at(userId) = rowCount;
    } else {
        userRuleSize_.insert({ userId, rowCount });
    }
    NETMGR_EXT_LOG_I(
        "GetAllRuleConstraint userId=%{public}d rowCount=%{public}d allUserRule=%{public}d allUserDomain=%{public}d",
        userId, static_cast<int32_t>(rowCount), static_cast<int32_t>(allUserRule_),
        static_cast<int32_t>(allUserDomain_));
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallRuleManager::CheckRuleConstraint(const sptr<NetFirewallRule> &rule)
{
    int32_t userId = rule->userId;
    GetAllRuleConstraint(userId);
    if (!CheckAccountExits(userId)) {
        NETMGR_EXT_LOG_E("current accoutn is not exits, userId: %{public}d.", userId);
        return FIREWALL_ERR_NO_RULE;
    }
    if (userRuleSize_.at(userId) < maxDefaultRuleSize_) {
        NETMGR_EXT_LOG_I("current user db size is not max than default rule size, ignore check.");
        return FIREWALL_SUCCESS;
    }

    if (allUserRule_ + 1 > FIREWALL_ALL_USER_MAX_RULE || userRuleSize_.at(userId) + 1 > FIREWALL_USER_MAX_RULE) {
        NETMGR_EXT_LOG_E("check rule constraint error, rule is large.");
        return FIREWALL_ERR_EXCEED_MAX_RULE;
    }
    int64_t domainsCount = 0;
    NetFirewallDbHelper::GetInstance()->QueryFirewallRuleDomainByUserIdCount(userId, domainsCount);
    if (domainsCount + rule->domains.size() > FIREWALL_SINGLE_USER_MAX_DOMAIN) {
        return FIREWALL_ERR_EXCEED_MAX_DOMAIN;
    }
    if (allUserDomain_ + rule->domains.size() > FIREWALL_ALL_USER_MAX_DOMAIN) {
        return FIREWALL_ERR_EXCEED_ALL_MAX_DOMAIN;
    }
    // DNS rule check duplicate
    if (NetFirewallDbHelper::GetInstance()->IsDnsRuleExist(rule)) {
        NETMGR_EXT_LOG_E("check rule constraint, the dns rule is exist");
        return FIREWALL_ERR_DNS_RULE_DUPLICATION;
    }
    return FIREWALL_SUCCESS;
}

bool NetFirewallRuleManager::CheckAccountExits(int32_t userId)
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

void NetFirewallRuleManager::ExtractDomainRule(const NetFirewallRule &rule,
    std::vector<sptr<NetFirewallDomainRule>> &outRuleList)
{
    for (const auto &domain : rule.domains) {
        sptr<NetFirewallDomainRule> domainRule = new (std::nothrow) NetFirewallDomainRule();
        domainRule->userId = rule.userId;
        domainRule->ruleId = rule.ruleId;
        domainRule->ruleAction = rule.ruleAction;
        domainRule->appUid = rule.appUid;
        domainRule->isWildcard = domain.isWildcard;
        domainRule->domain = domain.domain;
        outRuleList.emplace_back(std::move(domainRule));
    }
}

const sptr<NetFirewallIpRule> NetFirewallRuleManager::ExtractIpRule(const NetFirewallRule &rule)
{
    sptr<NetFirewallIpRule> ipRule = new (std::nothrow) NetFirewallIpRule();
    ipRule->userId = rule.userId;
    ipRule->ruleId = rule.ruleId;
    ipRule->ruleDirection = rule.ruleDirection;
    ipRule->ruleAction = rule.ruleAction;
    ipRule->appUid = rule.appUid;
    ipRule->localIps = rule.localIps;
    ipRule->remoteIps = rule.remoteIps;
    ipRule->protocol = rule.protocol;
    ipRule->localPorts = rule.localPorts;
    ipRule->remotePorts = rule.remotePorts;
    return ipRule;
}

bool NetFirewallRuleManager::ExtractIpRules(const std::vector<NetFirewallRule> &rules)
{
    if (rules.empty()) {
        return false;
    }
    // Release historical rule pointer
    for (auto &rule : ipRules_) {
        delete rule;
        rule = nullptr;
    }
    ipRules_.clear();
    for (const auto &rule : rules) {
        if (rule.ruleType != NetFirewallRuleType::RULE_IP) {
            continue;
        }
        sptr<NetFirewallIpRule> ipRule = ExtractIpRule(rule);
        ipRules_.emplace_back(std::move(ipRule));
    }
    return ipRules_.size() > 0;
}

bool NetFirewallRuleManager::ExtractDomainRules(const std::vector<NetFirewallRule> &rules)
{
    if (rules.empty()) {
        return false;
    }
    // Release historical rule pointer
    for (auto &rule : domainRules_) {
        delete rule;
        rule = nullptr;
    }
    domainRules_.clear();
    for (const auto &rule : rules) {
        if (rule.ruleType != NetFirewallRuleType::RULE_DOMAIN) {
            continue;
        }
        for (const auto &domain : rule.domains) {
            sptr<NetFirewallDomainRule> pRule = new (std::nothrow) NetFirewallDomainRule();
            pRule->userId = rule.userId;
            pRule->ruleId = rule.ruleId;
            pRule->ruleAction = rule.ruleAction;
            pRule->appUid = rule.appUid;
            pRule->isWildcard = domain.isWildcard;
            pRule->domain = domain.domain;
            domainRules_.emplace_back(std::move(pRule));
        }
    }
    return domainRules_.size() > 0;
}

bool NetFirewallRuleManager::ExtractDnsRules(const std::vector<NetFirewallRule> &rules)
{
    if (rules.empty()) {
        return false;
    }
    // Release historical rule pointer
    for (auto &rule : dnsRules_) {
        delete rule;
        rule = nullptr;
    }
    dnsRules_.clear();
    for (const auto &rule : rules) {
        if (rule.ruleType != NetFirewallRuleType::RULE_DNS) {
            continue;
        }
        sptr<NetFirewallDnsRule> pRule = new (std::nothrow) NetFirewallDnsRule();
        pRule->userId = rule.userId;
        pRule->appUid = rule.appUid;
        pRule->primaryDns = rule.dns.primaryDns;
        pRule->standbyDns = rule.dns.standbyDns;
        dnsRules_.emplace_back(std::move(pRule));
    }
    return dnsRules_.size() > 0;
}

int32_t NetFirewallRuleManager::HandleIpTypeForDistributeRules(std::vector<NetFirewallRule> &rules)
{
    if (ExtractIpRules(rules)) {
        NetFirewallRuleNativeHelper::GetInstance()->SetFirewallIpRules(ipRules_);
    } else {
        NetFirewallRuleNativeHelper::GetInstance()->ClearFirewallRules(NetFirewallRuleType::RULE_IP);
    }

    return FIREWALL_SUCCESS;
}

int32_t NetFirewallRuleManager::HandleDnsTypeForDistributeRules(std::vector<NetFirewallRule> &rules)
{
    if (ExtractDnsRules(rules)) {
        NetFirewallRuleNativeHelper::GetInstance()->SetFirewallDnsRules(dnsRules_);
    } else {
        NetFirewallRuleNativeHelper::GetInstance()->ClearFirewallRules(NetFirewallRuleType::RULE_DNS);
    }

    return FIREWALL_SUCCESS;
}

int32_t NetFirewallRuleManager::HandleDomainTypeForDistributeRules(std::vector<NetFirewallRule> &rules)
{
    if (ExtractDomainRules(rules)) {
        NetFirewallRuleNativeHelper::GetInstance()->SetFirewallDomainRules(domainRules_);
    } else {
        NetFirewallRuleNativeHelper::GetInstance()->ClearFirewallRules(NetFirewallRuleType::RULE_DOMAIN);
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
    NETMGR_EXT_LOG_I("OpenOrCloseNativeFirewall: type=%{public}d", NetFirewallRuleType::RULE_ALL);
    NetmanagerHiTrace::NetmanagerStartSyncTrace("OpenOrCloseNativeFirewall");
    auto userId = GetCurrentAccountId();
    if (!isOpen) {
        NETMGR_EXT_LOG_I("OpenOrCloseNativeFirewall: current userid %{public}d firewall disabled", userId);
        NetFirewallRuleNativeHelper::GetInstance()->SetFirewallDefaultAction(FirewallRuleAction::RULE_ALLOW,
            FirewallRuleAction::RULE_ALLOW);
        NetFirewallRuleNativeHelper::GetInstance()->ClearFirewallRules(NetFirewallRuleType::RULE_ALL);
        return FIREWALL_SUCCESS;
    }

    NetFirewallRuleNativeHelper::GetInstance()->SetFirewallDefaultAction(
        NetFirewallPolicyManager::GetInstance()->GetFirewallPolicyInAction(),
        NetFirewallPolicyManager::GetInstance()->GetFirewallPolicyOutAction());
    int32_t ret = SetRulesToNativeByType(userId, NetFirewallRuleType::RULE_ALL);
    SetNetFirewallDumpMessage(ret);
    NetmanagerHiTrace::NetmanagerFinishSyncTrace("OpenOrCloseNativeFirewall");
    return ret;
}

int32_t NetFirewallRuleManager::DistributeRulesToNative(NetFirewallRuleType type)
{
    NETMGR_EXT_LOG_I("DistributeRulesToNative: type=%{public}d", (int)type);
    NetmanagerHiTrace::NetmanagerStartSyncTrace("DistributeRulesToNative");
    auto userId = GetCurrentAccountId();
    if (!NetFirewallPolicyManager::GetInstance()->IsCurrentFirewallOpen()) {
        NETMGR_EXT_LOG_I("DistributeRulesToNative: current userid %{public}d firewall disabled", userId);
        NetFirewallRuleNativeHelper::GetInstance()->SetFirewallDefaultAction(FirewallRuleAction::RULE_ALLOW,
            FirewallRuleAction::RULE_ALLOW);
        NetFirewallRuleNativeHelper::GetInstance()->ClearFirewallRules(NetFirewallRuleType::RULE_ALL);
        return FIREWALL_SUCCESS;
    }

    NetFirewallRuleNativeHelper::GetInstance()->SetFirewallDefaultAction(
        NetFirewallPolicyManager::GetInstance()->GetFirewallPolicyInAction(),
        NetFirewallPolicyManager::GetInstance()->GetFirewallPolicyOutAction());
    int32_t ret = SetRulesToNativeByType(userId, type);
    NetmanagerHiTrace::NetmanagerFinishSyncTrace("DistributeRulesToNative");
    return ret;
}

int32_t NetFirewallRuleManager::SetRulesToNativeByType(const int32_t userId, const NetFirewallRuleType type)
{
    int32_t ret = FIREWALL_SUCCESS;
    std::vector<NetFirewallRule> rules;
    GetEnabledNetFirewallRules(userId, rules, type);
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
                NetFirewallRuleNativeHelper::GetInstance()->ClearFirewallRules(NetFirewallRuleType::RULE_ALL);
                break;
            }
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

int32_t NetFirewallRuleManager::AddFirewallRuleToNative(const NetFirewallRule &rule, int32_t ruleId)
{
    int32_t ret = FIREWALL_SUCCESS;
    switch (rule.ruleType) {
        case NetFirewallRuleType::RULE_IP: {
            sptr<NetFirewallIpRule> ipRule = ExtractIpRule(rule);
            ipRule->ruleId = ruleId;
            std::vector<sptr<NetFirewallIpRule>> ruleList;
            ruleList.emplace_back(std::move(ipRule));
            ret = NetFirewallRuleNativeHelper::GetInstance()->AddFirewallIpRules(ruleList, true);
            break;
        }
        case NetFirewallRuleType::RULE_DOMAIN: {
            std::vector<sptr<NetFirewallDomainRule>> domainRules;
            ExtractDomainRule(rule, domainRules);
            for (auto &domainRule : domainRules) {
                domainRule->ruleId = ruleId;
            }
            ret = NetFirewallRuleNativeHelper::GetInstance()->AddFirewallDomainRules(domainRules, true);
            break;
        }
        case NetFirewallRuleType::RULE_DNS:
            ret = DistributeRulesToNative(rule.ruleType);
            break;
        default:
            break;
    }
    SetNetFirewallDumpMessage(ret);
    return ret;
}

int32_t NetFirewallRuleManager::UpdateNativeFirewallRule(const NetFirewallRule &rule)
{
    int32_t ret = FIREWALL_SUCCESS;
    switch (rule.ruleType) {
        case NetFirewallRuleType::RULE_IP: {
            sptr<NetFirewallIpRule> ipRule = ExtractIpRule(rule);
            ret = NetFirewallRuleNativeHelper::GetInstance()->UpdateFirewallIpRule(ipRule);
            break;
        }
        case NetFirewallRuleType::RULE_DOMAIN: {
            std::vector<sptr<NetFirewallDomainRule>> domainRules;
            ExtractDomainRule(rule, domainRules);
            ret = NetFirewallRuleNativeHelper::GetInstance()->UpdateFirewallDomainRules(domainRules);
            break;
        }
        case NetFirewallRuleType::RULE_DNS:
            ret = DistributeRulesToNative(rule.ruleType);
            break;
        default:
            break;
    }
    SetNetFirewallDumpMessage(ret);
    return ret;
}

int32_t NetFirewallRuleManager::DeleteNativeFirewallRule(const NetFirewallRule &rule)
{
    int32_t ret = FIREWALL_SUCCESS;
    switch (rule.ruleType) {
        case NetFirewallRuleType::RULE_DOMAIN:
        case NetFirewallRuleType::RULE_IP: {
            std::vector<int32_t> ruleIds;
            ruleIds.emplace_back(rule.ruleId);
            ret = NetFirewallRuleNativeHelper::GetInstance()->DeleteFirewallRules(rule.ruleType, ruleIds);
            break;
        }
        case NetFirewallRuleType::RULE_DNS:
            ret = DistributeRulesToNative(rule.ruleType);
            break;
        default:
            break;
    }
    SetNetFirewallDumpMessage(ret);
    return ret;
}

void NetFirewallRuleManager::ChangeUserRuleSize(const int32_t userId)
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
