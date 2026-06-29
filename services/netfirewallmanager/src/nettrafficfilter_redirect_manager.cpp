/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "nettrafficfilter_redirect_manager.h"
#include "nettrafficfilter_iptables_command_builder.h"
#include "netsys_controller.h"
#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "singleton.h"
#include "ipc_skeleton.h"
#include "bundle_mgr_proxy.h"
#include "bundle_mgr_client.h"
#include <arpa/inet.h>
#include <sstream>
#include <iomanip>

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t IPV6_PREFIX_MAX = 128;
constexpr int32_t IPV4_PREFIX_MAX = 32;
constexpr int32_t BUNDLE_LEN_MAX = 255;
constexpr int32_t REDIRECTOR_ID_START = 1000;

static bool ValidateIPMatchType(int32_t type)
{
    return type >= static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY) &&
           type <= static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_MULTI);
}

static bool ValidatePortMatchType(int32_t type)
{
    return type >= static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY) &&
           type <= static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_MULTI);
}

static bool IsValidIPFamilyValue(int32_t family)
{
    return family == static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4) ||
           family == static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V6);
}
static uint8_t GetMaxPrefixLenByFamily(int32_t family)
{
    if (family == static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V6)) {
        return IPV6_PREFIX_MAX;
    }
    return IPV4_PREFIX_MAX;
}

void NetTrafficFilterRedirectManager::SortRedirectorList()
{
    NETMGR_EXT_LOG_I("SortRedirectorList started with %{public}zu redirectors", redirectorIdList_.size());
    std::sort(redirectorIdList_.begin(), redirectorIdList_.end(),
        [this](const std::string& a, const std::string& b) {
            auto itA = redirectors_.find(a);
            auto itB = redirectors_.find(b);
            if (itA == redirectors_.end() || itB == redirectors_.end() ||
                itA->second == nullptr || itB->second == nullptr) {
                return false;
            }
            return itA->second->GetPriority() < itB->second->GetPriority();
        });
    NETMGR_EXT_LOG_I("SortRedirectorList completed");
}

std::vector<std::string> NetTrafficFilterRedirectManager::GetActiveRedirectorsForHookPoint(
    TrafficFilterHookPoint hookPoint, TrafficFilterIPFamily family) const
{
    NETMGR_EXT_LOG_I("GetActiveRedirectorsForHookPoint: hookPoint=%{public}d, family=%{public}d",
        static_cast<int32_t>(hookPoint), static_cast<int32_t>(family));
    std::vector<std::string> activeRedirectors;
    for (const auto& redirectorId : redirectorIdList_) {
        auto it = redirectors_.find(redirectorId);
        if (it == redirectors_.end()) {
            continue;
        }
        auto redirector = it->second;
        if (redirector == nullptr) {
            continue;
        }
        if (redirector->IsPaused()) {
            continue;
        }
        if (!redirector->HasRules()) {
            continue;
        }
        std::vector<TrafficFilterRedirectRule> rules = redirector->GetRules();
        bool hasMatchedRule = false;
        for (const auto& rule : rules) {
            if (rule.hookPoint_ != static_cast<int32_t>(hookPoint)) {
                continue;
            }
            TrafficFilterIPFamily ruleFamily = DetermineRuleFamily(rule);
            if (ruleFamily == family) {
                hasMatchedRule = true;
                break;
            }
        }
        if (!hasMatchedRule) {
            continue;
        }
        activeRedirectors.push_back(redirectorId);
    }
    return activeRedirectors;
}

int32_t NetTrafficFilterRedirectManager::RemoveJumpRulesFromHookPoint(
    TrafficFilterHookPoint hookPoint, TrafficFilterIPFamily family)
{
    NETMGR_EXT_LOG_I("RemoveJumpRulesFromHookPoint: hookPoint=%{public}d, family=%{public}d",
        static_cast<int32_t>(hookPoint), static_cast<int32_t>(family));
    std::string hookPointName = NetTrafficFilterIptablesCommandBuilder::GetHookPointName(hookPoint);
    if (hookPointName.empty()) {
        NETMGR_EXT_LOG_E("invalid hook point name, hookPoint=%{public}d", static_cast<int32_t>(hookPoint));
        return -1;
    }
    for (const auto& [redirectorId, redirector] : redirectors_) {
        if (redirector == nullptr) {
            continue;
        }
        std::string chainName = NetTrafficFilterIptablesCommandBuilder::GenerateChainName(
            redirector->GetCallingUid(), redirector->GetGroupId());
        std::string jumpCmd = NetTrafficFilterIptablesCommandBuilder::BuildDeleteJumpCommand(hookPointName, chainName);
        if (ExecuteIptablesCommand(jumpCmd, family) != TRAFFICFILTER_OK) {
            NETMGR_EXT_LOG_I("Jump rule not found or already removed, hookPoint=%{public}d, chain=%{public}s",
                static_cast<int32_t>(hookPoint), chainName.c_str());
        }
    }
    NETMGR_EXT_LOG_I("RemoveJumpRulesFromHookPoint completed");
    return TRAFFICFILTER_OK;
}

int32_t NetTrafficFilterRedirectManager::UpdateGlobalJumpRules(
    TrafficFilterHookPoint hookPoint, TrafficFilterIPFamily family)
{
    NETMGR_EXT_LOG_I("UpdateGlobalJumpRules: hookPoint=%{public}d, family=%{public}d",
        static_cast<int32_t>(hookPoint), static_cast<int32_t>(family));
    if (RemoveJumpRulesFromHookPoint(hookPoint, family) != TRAFFICFILTER_OK) {
        NETMGR_EXT_LOG_W("Failed to remove jump rules");
    }
    std::vector<std::string> activeRedirectors = GetActiveRedirectorsForHookPoint(hookPoint, family);
    std::string hookPointName = NetTrafficFilterIptablesCommandBuilder::GetHookPointName(hookPoint);
    if (hookPointName.empty()) {
        NETMGR_EXT_LOG_E("invalid hook point name, hookPoint=%{public}d",
            static_cast<int32_t>(hookPoint));
        return -1;
    }
    uint32_t position = 1;
    for (const auto& redirectorId : activeRedirectors) {
        auto it = redirectors_.find(redirectorId);
        if (it == redirectors_.end()) {
            continue;
        }
        auto redirector = it->second;
        std::string chainName = NetTrafficFilterIptablesCommandBuilder::GenerateChainName(
            redirector->GetCallingUid(), redirector->GetGroupId());
        std::string addJumpCmd = NetTrafficFilterIptablesCommandBuilder::BuildInsertJumpToChainCommand(
            hookPointName, chainName, position);
        if (addJumpCmd.empty()) {
            NETMGR_EXT_LOG_E("empty add jump command");
            return -1;
        }
        int32_t ret = ExecuteIptablesCommand(addJumpCmd, family);
        if (ret != TRAFFICFILTER_OK) {
            NETMGR_EXT_LOG_E("Failed to add jump rule for redirector %{public}s, position=%{public}u",
                redirectorId.c_str(), position);
            return ret;
        }
        position++;
    }
    NETMGR_EXT_LOG_I("UpdateGlobalJumpRules completed");
    return TRAFFICFILTER_OK;
}

TrafficFilterIPFamily NetTrafficFilterRedirectManager::GetIPFamilyFromMatch(const TrafficFilterIPMatch& ipMatch)
{
    NETMGR_EXT_LOG_I("GetIPFamilyFromMatch: type=%{public}d", ipMatch.type_);
    switch (ipMatch.type_) {
        case static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE):
            return static_cast<TrafficFilterIPFamily>(ipMatch.single_.family_);
        case static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_CIDR):
            return static_cast<TrafficFilterIPFamily>(ipMatch.cidr_.base_.family_);
        case static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_RANGE):
            return static_cast<TrafficFilterIPFamily>(ipMatch.range_.start_.family_);
        case static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_MULTI):
            if (ipMatch.multi_.ipCount_ > 0) {
                return static_cast<TrafficFilterIPFamily>(ipMatch.multi_.ips_[0].family_);
            }
            break;
        default:
            break;
    }
    return TrafficFilterIPFamily::IP_FAMILY_UNSPEC;
}

TrafficFilterIPFamily NetTrafficFilterRedirectManager::DetermineRuleFamily(const TrafficFilterRedirectRule& rule)
{
    NETMGR_EXT_LOG_I("DetermineRuleFamily started");
    TrafficFilterIPFamily srcFamily = GetIPFamilyFromMatch(rule.srcIp_);
    TrafficFilterIPFamily dstFamily = GetIPFamilyFromMatch(rule.dstIp_);
    TrafficFilterIPFamily proxyFamily = static_cast<TrafficFilterIPFamily>(rule.proxyIp_.family_);
    if (srcFamily != TrafficFilterIPFamily::IP_FAMILY_UNSPEC &&
        dstFamily != TrafficFilterIPFamily::IP_FAMILY_UNSPEC &&
        srcFamily != dstFamily) {
        NETMGR_EXT_LOG_E("DetermineRuleFamily failed: src/dst family mismatch, src=%{public}d, dst=%{public}d",
            static_cast<int32_t>(srcFamily), static_cast<int32_t>(dstFamily));
        return TrafficFilterIPFamily::IP_FAMILY_UNSPEC;
    }
    if (srcFamily != TrafficFilterIPFamily::IP_FAMILY_UNSPEC) {
        return srcFamily;
    }
    if (dstFamily != TrafficFilterIPFamily::IP_FAMILY_UNSPEC) {
        return dstFamily;
    }
    if (IsValidIPFamilyValue(static_cast<int32_t>(proxyFamily))) {
        NETMGR_EXT_LOG_I("DetermineRuleFamily result from proxyFamily=%{public}d",
            static_cast<int32_t>(proxyFamily));
        return proxyFamily;
    }
    NETMGR_EXT_LOG_E("DetermineRuleFamily failed: no valid family");
    return TrafficFilterIPFamily::IP_FAMILY_UNSPEC;
}

bool NetTrafficFilterRedirectManager::ValidateCidrIPMatch(const TrafficFilterIPMatch& ipMatch)
{
    int32_t family = ipMatch.cidr_.base_.family_;
    if (!IsValidIPFamilyValue(family)) {
        return false;
    }
    uint8_t maxPrefix = GetMaxPrefixLenByFamily(family);
    if (ipMatch.cidr_.prefixLen_ > maxPrefix) {
        NETMGR_EXT_LOG_E("invalid CIDR prefix, family=%{public}d, prefix=%{public}u, max=%{public}u",
            family, ipMatch.cidr_.prefixLen_, maxPrefix);
        return false;
    }
    return true;
}

bool NetTrafficFilterRedirectManager::ValidateRangeIPMatch(
    const TrafficFilterIPMatch& ipMatch)
{
    int32_t startFamily = ipMatch.range_.start_.family_;
    int32_t endFamily = ipMatch.range_.end_.family_;
    if (!IsValidIPFamilyValue(startFamily) || !IsValidIPFamilyValue(endFamily)) {
        return false;
    }
    if (startFamily != endFamily) {
        NETMGR_EXT_LOG_E("invalid IP range family mismatch, start=%{public}d, end=%{public}d",
            startFamily, endFamily);
        return false;
    }
    return true;
}

bool NetTrafficFilterRedirectManager::ValidateMultiIPMatch(
    const TrafficFilterIPMatch& ipMatch)
{
    if (ipMatch.multi_.ipCount_ == 0 || ipMatch.multi_.ipCount_ > NETTRAFFICFILTER_MAX_MULTI_IP_COUNT) {
        return false;
    }
    int32_t firstFamily = ipMatch.multi_.ips_[0].family_;
    if (!IsValidIPFamilyValue(firstFamily)) {
        return false;
    }
    for (uint32_t i = 1; i < ipMatch.multi_.ipCount_; i++) {
        int32_t family = ipMatch.multi_.ips_[i].family_;
        if (!IsValidIPFamilyValue(family)) {
            return false;
        }
        if (family != firstFamily) {
            NETMGR_EXT_LOG_E("invalid IP multi family mismatch, first=%{public}d, family=%{public}d",
                firstFamily, family);
            return false;
        }
    }
    return true;
}

bool NetTrafficFilterRedirectManager::ValidateIPMatch(const TrafficFilterIPMatch& ipMatch)
{
    if (!ValidateIPMatchType(ipMatch.type_)) {
        return false;
    }
    switch (ipMatch.type_) {
        case static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY):
            return true;
        case static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE):
            return IsValidIPFamilyValue(ipMatch.single_.family_);
        case static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_CIDR):
            return ValidateCidrIPMatch(ipMatch);
        case static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_RANGE):
            return ValidateRangeIPMatch(ipMatch);
        case static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_MULTI):
            return ValidateMultiIPMatch(ipMatch);
        default:
            return false;
    }
}

bool NetTrafficFilterRedirectManager::ValidatePortMatch(const TrafficFilterPortMatch& portMatch)
{
    if (!ValidatePortMatchType(portMatch.type_)) {
        return false;
    }
    if (portMatch.type_ == static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_RANGE)) {
        if (portMatch.range_.startPort_ > portMatch.range_.endPort_) {
            NETMGR_EXT_LOG_E("invalid port range, start=%{public}u, end=%{public}u",
                portMatch.range_.startPort_, portMatch.range_.endPort_);
            return false;
        }
    }
    if (portMatch.type_ == static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_MULTI)) {
        if (portMatch.multi_.portCount_ > NETTRAFFICFILTER_MAX_MULTI_PORT_COUNT) {
            return false;
        }
        if (portMatch.multi_.portCount_ == 0) {
            return false;
        }
    }
    return true;
}

bool NetTrafficFilterRedirectManager::ValidateInterfaceMatch(const TrafficFilterInterfaceMatch& ifMatch)
{
    if (ifMatch.enabled_ && ifMatch.ifName_.empty()) {
        return false;
    }
    return true;
}

bool NetTrafficFilterRedirectManager::ValidateIPFamilyConsistency(
    const TrafficFilterIPMatch& srcIp, const TrafficFilterIPMatch& dstIp)
{
    TrafficFilterIPFamily srcFamily = GetIPFamilyFromMatch(srcIp);
    TrafficFilterIPFamily dstFamily = GetIPFamilyFromMatch(dstIp);
    if (srcFamily == TrafficFilterIPFamily::IP_FAMILY_UNSPEC &&
        dstFamily == TrafficFilterIPFamily::IP_FAMILY_UNSPEC) {
        return true;
    }
    if (srcFamily == TrafficFilterIPFamily::IP_FAMILY_UNSPEC) {
        return true;
    }
    if (dstFamily == TrafficFilterIPFamily::IP_FAMILY_UNSPEC) {
        return true;
    }
    if (srcFamily != dstFamily) {
        return false;
    }
    return true;
}

bool NetTrafficFilterRedirectManager::ValidateUidMatch(const TrafficFilterRedirectRule& rule)
{
    bool uidStartSet = rule.uidStart_ != static_cast<uint32_t>(-1);
    bool uidEndSet = rule.uidEnd_ != static_cast<uint32_t>(-1);
    // 两者都未设置，表示不启用 UID 匹配
    if (!uidStartSet && !uidEndSet) {
        return true;
    }
    // 只设置一边，参数不完整
    if (uidStartSet != uidEndSet) {
        NETMGR_EXT_LOG_E("UID match invalid: uidStartSet=%{public}d, uidEndSet=%{public}d",
            uidStartSet, uidEndSet);
        return false;
    }
    if (rule.uidStart_ > rule.uidEnd_) {
        NETMGR_EXT_LOG_E("UID match invalid: uidStart=%{public}u > uidEnd=%{public}u",
            rule.uidStart_, rule.uidEnd_);
        return false;
    }
    if (rule.hookPoint_ != static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT)) {
        NETMGR_EXT_LOG_E("UID owner match only supports OUTPUT hook, hookPoint=%{public}d",
            rule.hookPoint_);
        return false;
    }
    return true;
}

bool NetTrafficFilterRedirectManager::ValidateCreateRedirectorParams(
    const std::string& bundleName, uint32_t groupId, uint32_t priority)
{
    if (bundleName.empty()) {
        return false;
    }
    if (bundleName.length() > BUNDLE_LEN_MAX) {
        return false;
    }

    if (groupId < NETTRAFFICFILTER_MIN_GROUP_ID || groupId > NETTRAFFICFILTER_MAX_GROUP_ID) {
        return false;
    }

    if (priority < NETTRAFFICFILTER_MIN_PRIORITY || priority > NETTRAFFICFILTER_MAX_PRIORITY) {
        return false;
    }
    return true;
}

bool NetTrafficFilterRedirectManager::ValidateProxyFamilyConsistency(const TrafficFilterRedirectRule& rule)
{
    TrafficFilterIPFamily ruleFamily = DetermineRuleFamily(rule);
    TrafficFilterIPFamily proxyFamily = static_cast<TrafficFilterIPFamily>(rule.proxyIp_.family_);
    if (!IsValidIPFamilyValue(static_cast<int32_t>(ruleFamily))) {
        NETMGR_EXT_LOG_E("invalid rule family: %{public}d", static_cast<int32_t>(ruleFamily));
        return false;
    }
    if (!IsValidIPFamilyValue(static_cast<int32_t>(proxyFamily))) {
        NETMGR_EXT_LOG_E("invalid proxy family: %{public}d", static_cast<int32_t>(proxyFamily));
        return false;
    }
    if (ruleFamily != proxyFamily) {
        NETMGR_EXT_LOG_E("proxy family mismatch, ruleFamily=%{public}d, proxyFamily=%{public}d",
            static_cast<int32_t>(ruleFamily), static_cast<int32_t>(proxyFamily));
        return false;
    }
    return true;
}

NetTrafficFilterRedirectManager& NetTrafficFilterRedirectManager::GetInstance()
{
    static NetTrafficFilterRedirectManager instance;
    return instance;
}

NetTrafficFilterRedirectManager::NetTrafficFilterRedirectManager()
    : redirectorIdCounter_(REDIRECTOR_ID_START)
{}

NetTrafficFilterRedirectManager::~NetTrafficFilterRedirectManager() {}

int32_t NetTrafficFilterRedirectManager::CreateRedirector(const std::string& bundleName,
    uint32_t groupId, uint32_t priority, std::string& redirectorId)
{
    NETMGR_EXT_LOG_I("CreateRedirector called: bundleName=%{public}s, groupId=%{public}u, priority=%{public}u",
        bundleName.c_str(), groupId, priority);
    if (!ValidateCreateRedirectorParams(bundleName, groupId, priority)) {
        NETMGR_EXT_LOG_E("CreateRedirector parameter validation failed");
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    int32_t callingUid = IPCSkeleton::GetCallingUid();
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    std::lock_guard<std::mutex> lock(mutex_);

    if (IsRedirectorExists(bundleName, groupId)) {
        NETMGR_EXT_LOG_E("Redirector already exists for bundleName=%{public}s, groupId=%{public}u",
            bundleName.c_str(), groupId);
        return TRAFFICFILTER_ERROR_GROUP_ID_IN_USE;
    }

    redirectorId = GenerateRedirectorId();
    std::string chainName = NetTrafficFilterIptablesCommandBuilder::GenerateChainName(callingUid, groupId);
    std::string createChainCmd = NetTrafficFilterIptablesCommandBuilder::BuildCreateChainCommand(chainName);
    int32_t ret = ExecuteIptablesCommand(createChainCmd, TrafficFilterIPFamily::IP_FAMILY_V4V6);
    if (ret != TRAFFICFILTER_OK) {
        NETMGR_EXT_LOG_E("Failed to create iptables chain for redirector");
        return -1;
    }

    auto redirector = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId, bundleName, groupId, priority);
    redirector->SetCallingInfo(callingUid, callingPid);
    redirectors_[redirectorId] = redirector;
    bundleNameToRedirectorsMap_[bundleName].push_back(redirectorId);
    redirectorIdList_.push_back(redirectorId);
    SortRedirectorList();

    NETMGR_EXT_LOG_I("Redirector created: id=%{public}s, bundleName=%{public}s, groupId=%{public}u, "
        "uid=%{public}d, pid=%{public}d, chain=%{public}s",
        redirectorId.c_str(), bundleName.c_str(), groupId, callingUid, callingPid, chainName.c_str());
    HandleTrafficFilterObserverRegistration(bundleName, callingUid, callingPid);
    return TRAFFICFILTER_OK;
}

bool NetTrafficFilterRedirectManager::IsRedirectorExists(
    const std::string& bundleName, uint32_t groupId)
{
    auto bundleMapIt = bundleNameToRedirectorsMap_.find(bundleName);
    if (bundleMapIt == bundleNameToRedirectorsMap_.end()) {
        return false;
    }

    for (const auto& existingRedirectorId : bundleMapIt->second) {
        auto existingRedirectorIt = redirectors_.find(existingRedirectorId);
        if (existingRedirectorIt != redirectors_.end()) {
            if (existingRedirectorIt->second->GetGroupId() == groupId) {
                return true;
            }
        }
    }
    return false;
}

int32_t NetTrafficFilterRedirectManager::CleanupRedirectorIptablesResources(const std::string& chainName,
    const std::set<TrafficFilterHookPoint>& usedHookPoints)
{
    NETMGR_EXT_LOG_I("CleanupRedirectorIptablesResources: chainName=%{public}s", chainName.c_str());

    for (auto hookPoint : usedHookPoints) {
        std::string hookPointName = NetTrafficFilterIptablesCommandBuilder::GetHookPointName(hookPoint);
        std::string jumpCmd = NetTrafficFilterIptablesCommandBuilder::BuildDeleteJumpCommand(
            hookPointName, chainName);
        if (ExecuteIptablesCommand(jumpCmd, TrafficFilterIPFamily::IP_FAMILY_V4V6) != TRAFFICFILTER_OK) {
            NETMGR_EXT_LOG_I("Jump rule not found for hook point %{public}d",
                static_cast<int32_t>(hookPoint));
        }
    }

    std::string clearChainCmd = NetTrafficFilterIptablesCommandBuilder::BuildFlushChainCommand(chainName);
    if (ExecuteIptablesCommand(clearChainCmd, TrafficFilterIPFamily::IP_FAMILY_V4V6) != TRAFFICFILTER_OK) {
        NETMGR_EXT_LOG_E("Failed to flush chain");
    }

    std::string deleteChainCmd = NetTrafficFilterIptablesCommandBuilder::BuildDeleteChainCommand(chainName);
    if (ExecuteIptablesCommand(deleteChainCmd, TrafficFilterIPFamily::IP_FAMILY_V4V6) != TRAFFICFILTER_OK) {
        NETMGR_EXT_LOG_W("Failed to delete chain");
    }

    NETMGR_EXT_LOG_I("CleanupRedirectorIptablesResources completed");
    return TRAFFICFILTER_OK;
}

void NetTrafficFilterRedirectManager::RemoveRedirectorFromDataStructures(const std::string& redirectorId,
    const std::string& bundleName)
{
    NETMGR_EXT_LOG_I("RemoveRedirectorFromDataStructures: redirectorId=%{public}s, bundleName=%{public}s",
        redirectorId.c_str(), bundleName.c_str());

    redirectors_.erase(redirectorId);

    auto& bundleRedirectors = bundleNameToRedirectorsMap_[bundleName];
    auto redirectorIt = std::find(bundleRedirectors.begin(), bundleRedirectors.end(), redirectorId);
    if (redirectorIt != bundleRedirectors.end()) {
        bundleRedirectors.erase(redirectorIt);
    }
    if (bundleRedirectors.empty()) {
        bundleNameToRedirectorsMap_.erase(bundleName);
    }

    auto listIt = std::remove(redirectorIdList_.begin(), redirectorIdList_.end(), redirectorId);
    if (listIt != redirectorIdList_.end()) {
        redirectorIdList_.erase(listIt, redirectorIdList_.end());
    }

    NETMGR_EXT_LOG_I("RemoveRedirectorFromDataStructures completed");
}

void NetTrafficFilterRedirectManager::RebuildGlobalJumpRulesAfterDestroy(
    const std::set<TrafficFilterHookPoint>& usedHookPoints)
{
    NETMGR_EXT_LOG_I("RebuildGlobalJumpRulesAfterDestroy: %{public}zu hook points", usedHookPoints.size());

    for (auto hookPoint : usedHookPoints) {
        int32_t retV4 = UpdateGlobalJumpRules(hookPoint, TrafficFilterIPFamily::IP_FAMILY_V4);
        if (retV4 != TRAFFICFILTER_OK) {
            NETMGR_EXT_LOG_W("Failed to update IPv4 jump rules for hook point %{public}d",
                static_cast<int32_t>(hookPoint));
        }

        int32_t retV6 = UpdateGlobalJumpRules(hookPoint, TrafficFilterIPFamily::IP_FAMILY_V6);
        if (retV6 != TRAFFICFILTER_OK) {
            NETMGR_EXT_LOG_W("Failed to update IPv6 jump rules for hook point %{public}d",
                static_cast<int32_t>(hookPoint));
        }
    }

    NETMGR_EXT_LOG_I("RebuildGlobalJumpRulesAfterDestroy completed");
}

int32_t NetTrafficFilterRedirectManager::RollbackRedirectorRules(
    const std::shared_ptr<NetTrafficFilterRedirectorContext>& redirector, const std::string& chainName,
    const std::vector<TrafficFilterRedirectRule>& oldRules, const std::set<TrafficFilterHookPoint>& affectedHookPoints)
{
    NETMGR_EXT_LOG_W("Rolling back redirector rules, chainName=%{public}s",
        chainName.c_str());
    if (redirector == nullptr) {
        NETMGR_EXT_LOG_E("Rollback failed: redirector is null");
        return -1;
    }
    if (redirector->RestoreRules(oldRules) != TRAFFICFILTER_OK) {
        NETMGR_EXT_LOG_E("Rollback failed: RestoreRules failed");
        return -1;
    }
    int32_t ret = ApplyRulesToChain(redirector, chainName);
    if (ret != TRAFFICFILTER_OK) {
        NETMGR_EXT_LOG_E("Rollback failed: failed to restore old rules to iptables chain");
        return -1;
    }
    for (auto hookPoint : affectedHookPoints) {
        ret = ApplyGlobalJumpRules(hookPoint);
        if (ret != TRAFFICFILTER_OK) {
            NETMGR_EXT_LOG_E(
                "Rollback failed: failed to rebuild global jump rules, hookPoint=%{public}d",
                static_cast<int32_t>(hookPoint));
            return -1;
        }
    }
    NETMGR_EXT_LOG_I("Rollback redirector rules completed");
    return TRAFFICFILTER_OK;
}

int32_t NetTrafficFilterRedirectManager::DestroyRedirector(const std::string& redirectorId)
{
    NETMGR_EXT_LOG_I("DestroyRedirector called: redirectorId=%{public}s", redirectorId.c_str());

    int32_t callingUid = -1;
    std::string bundleName;
    std::string chainName;
    std::set<TrafficFilterHookPoint> usedHookPoints;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = redirectors_.find(redirectorId);
        if (it == redirectors_.end()) {
            NETMGR_EXT_LOG_E("Redirector not found: %{public}s", redirectorId.c_str());
            return TRAFFICFILTER_ERROR_NOT_FOUND;
        }

        auto& redirector = it->second;
        bundleName = redirector->GetBundleName();
        callingUid = redirector->GetCallingUid();
        chainName = NetTrafficFilterIptablesCommandBuilder::GenerateChainName(
            callingUid, redirector->GetGroupId());

        std::vector<TrafficFilterRedirectRule> rules = redirector->GetRules();
        for (const auto& rule : rules) {
            usedHookPoints.insert(static_cast<TrafficFilterHookPoint>(rule.hookPoint_));
        }

        CleanupRedirectorIptablesResources(chainName, usedHookPoints);

        RemoveRedirectorFromDataStructures(redirectorId, bundleName);

        RebuildGlobalJumpRulesAfterDestroy(usedHookPoints);

        if (callingUid != -1) {
            bool hasOtherRedirectorsForUid = false;
            for (const auto& [id, redir] : redirectors_) {
                if (redir->GetCallingUid() == callingUid) {
                    hasOtherRedirectorsForUid = true;
                    break;
                }
            }
            if (!hasOtherRedirectorsForUid) {
                std::lock_guard<std::mutex> obsLock(observerMutex_);
                uidToObserverMap_.erase(callingUid);
                NETMGR_EXT_LOG_I("Cleaned up observer for uid=%{public}d (last redirector destroyed)", callingUid);
            }
        }
    }

    NETMGR_EXT_LOG_I("Redirector destroyed: id=%{public}s, bundleName=%{public}s",
        redirectorId.c_str(), bundleName.c_str());
    return TRAFFICFILTER_OK;
}

int32_t NetTrafficFilterRedirectManager::DestroyRedirectorsByBundleName(const std::string& bundleName)
{
    NETMGR_EXT_LOG_I("DestroyRedirectorsByBundleName called: bundleName=%{public}s", bundleName.c_str());
    std::vector<std::string> redirectorsToDestroy;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto mapIt = bundleNameToRedirectorsMap_.find(bundleName);
        if (mapIt == bundleNameToRedirectorsMap_.end()) {
            NETMGR_EXT_LOG_I("No redirectors found for bundleName: %{public}s", bundleName.c_str());
            return -1;
        }
        redirectorsToDestroy = mapIt->second;
    }
    for (const auto& redirectorId : redirectorsToDestroy) {
        int32_t ret = DestroyRedirector(redirectorId);
        if (ret != TRAFFICFILTER_OK) {
            NETMGR_EXT_LOG_E("Failed to destroy redirector %{public}s for bundle %{public}s",
                redirectorId.c_str(), bundleName.c_str());
        }
    }
    NETMGR_EXT_LOG_I("Destroyed all redirectors for bundleName: %{public}s", bundleName.c_str());
    return TRAFFICFILTER_OK;
}

bool NetTrafficFilterRedirectManager::ValidateRuleForAdd(const TrafficFilterRedirectRule& rule)
{
    if (!ValidateRedirectRuleFields(rule)) {
        NETMGR_EXT_LOG_E("AddRedirectRule field validation failed");
        return false;
    }
    if (!ValidateIPMatch(rule.srcIp_)) {
        NETMGR_EXT_LOG_E("AddRedirectRule src IP validation failed");
        return false;
    }
    if (!ValidateIPMatch(rule.dstIp_)) {
        NETMGR_EXT_LOG_E("AddRedirectRule dst IP validation failed");
        return false;
    }
    if (!ValidatePortMatch(rule.srcPort_)) {
        NETMGR_EXT_LOG_E("AddRedirectRule src port validation failed");
        return false;
    }
    if (!ValidatePortMatch(rule.dstPort_)) {
        NETMGR_EXT_LOG_E("AddRedirectRule dst port validation failed");
        return false;
    }
    if (!ValidateInterfaceMatch(rule.inInterface_)) {
        NETMGR_EXT_LOG_E("AddRedirectRule in interface validation failed");
        return false;
    }
    if (!ValidateInterfaceMatch(rule.outInterface_)) {
        NETMGR_EXT_LOG_E("AddRedirectRule out interface validation failed");
        return false;
    }
    if (!ValidateUidMatch(rule)) {
        NETMGR_EXT_LOG_E("AddRedirectRule UID match validation failed");
        return false;
    }
    if (!ValidateIPFamilyConsistency(rule.srcIp_, rule.dstIp_)) {
        NETMGR_EXT_LOG_E("AddRedirectRule IP family consistency validation failed");
        return false;
    }
    if (!ValidateProxyFamilyConsistency(rule)) {
        NETMGR_EXT_LOG_E("AddRedirectRule proxy family consistency validation failed");
        return false;
    }
    if (rule.proxyPort_ == 0 || rule.proxyPort_ > 0xFFFF) {
        NETMGR_EXT_LOG_E("AddRedirectRule proxy port validation failed");
        return false;
    }
    return true;
}

int32_t NetTrafficFilterRedirectManager::ApplyRulesToChain(
    const std::shared_ptr<NetTrafficFilterRedirectorContext>& redirector,
    const std::string& chainName)
{
    if (redirector == nullptr) {
        NETMGR_EXT_LOG_E("ApplyRulesToChain failed: redirector is null");
        return -1;
    }
    std::string clearChainCmd = NetTrafficFilterIptablesCommandBuilder::BuildFlushChainCommand(chainName);
    int32_t ret = ExecuteIptablesCommand(clearChainCmd, TrafficFilterIPFamily::IP_FAMILY_V4V6);
    if (ret != TRAFFICFILTER_OK) {
        NETMGR_EXT_LOG_E("Failed to flush chain: %{public}s", clearChainCmd.c_str());
        return -1;
    }
    if (redirector->IsPaused()) {
        NETMGR_EXT_LOG_I("Redirector is paused, appending pause rule first");
        std::string pauseCmd = NetTrafficFilterIptablesCommandBuilder::BuildAppendPauseRuleCommand(chainName);
        if (pauseCmd.empty()) {
            NETMGR_EXT_LOG_E("Empty append pause command");
            return -1;
        }
        ret = ExecuteIptablesCommand(pauseCmd, TrafficFilterIPFamily::IP_FAMILY_V4V6);
        if (ret != TRAFFICFILTER_OK) {
            NETMGR_EXT_LOG_E("Failed to append pause rule: %{private}s", pauseCmd.c_str());
            return -1;
        }
    }
    std::vector<TrafficFilterRedirectRule> sortedRules = redirector->GetSortedRules();
    if (sortedRules.empty()) {
        NETMGR_EXT_LOG_I("No rules to apply for chain: %{public}s", chainName.c_str());
        return TRAFFICFILTER_OK;
    }
    return AppendRedirectRulesToChain(sortedRules, chainName);
}

int32_t NetTrafficFilterRedirectManager::AppendRedirectRulesToChain(
    const std::vector<TrafficFilterRedirectRule>& sortedRules, const std::string& chainName)
{
    NETMGR_EXT_LOG_I("Appending %{public}zu sorted rules to chain", sortedRules.size());
    uint32_t successCount = 0;
    uint32_t failedCount = 0;
    for (const auto& sortedRule : sortedRules) {
        TrafficFilterIPFamily ruleFamily = DetermineRuleFamily(sortedRule);
        if (ruleFamily != TrafficFilterIPFamily::IP_FAMILY_V4 &&
            ruleFamily != TrafficFilterIPFamily::IP_FAMILY_V6) {
            NETMGR_EXT_LOG_E("Invalid rule family, skip rule: %{public}d", static_cast<int32_t>(ruleFamily));
            failedCount++;
            continue;
        }
        std::string addCmd = NetTrafficFilterIptablesCommandBuilder::BuildAppendRedirectCommand(sortedRule, chainName);
        if (addCmd.empty()) {
            NETMGR_EXT_LOG_E("Empty append redirect command, skip rule");
            failedCount++;
            continue;
        }
        int32_t ret = ExecuteIptablesCommand(addCmd, ruleFamily);
        if (ret != TRAFFICFILTER_OK) {
            NETMGR_EXT_LOG_E("Failed to append redirect rule, continue next. family=%{public}d, command=%{private}s",
                static_cast<int32_t>(ruleFamily), addCmd.c_str());
            failedCount++;
            continue;
        }
        successCount++;
    }
    NETMGR_EXT_LOG_I("AppendRedirectRulesToChain finished: success=%{public}u, failed=%{public}u",
        successCount, failedCount);
    if (successCount > 0) {
        if (failedCount > 0) {
            NETMGR_EXT_LOG_W("Some rules failed, but successful rules have been applied");
        }
        return TRAFFICFILTER_OK;
    }
    NETMGR_EXT_LOG_E("All redirect rules failed to apply");
    return -1;
}

int32_t NetTrafficFilterRedirectManager::ApplyGlobalJumpRules(TrafficFilterHookPoint hookPoint)
{
    int32_t ret = UpdateGlobalJumpRules(hookPoint, TrafficFilterIPFamily::IP_FAMILY_V4);
    if (ret != TRAFFICFILTER_OK) {
        NETMGR_EXT_LOG_E("Failed to update jump rules");
        return -1;
    }
    ret = UpdateGlobalJumpRules(hookPoint, TrafficFilterIPFamily::IP_FAMILY_V6);
    if (ret != TRAFFICFILTER_OK) {
        NETMGR_EXT_LOG_E("Failed to update jump rules");
        return -1;
    }
    return TRAFFICFILTER_OK;
}

int32_t NetTrafficFilterRedirectManager::AddRedirectRule(const std::string& redirectorId,
    const TrafficFilterRedirectRule* rule)
{
    if (rule == nullptr) {
        NETMGR_EXT_LOG_E("AddRedirectRule failed: rule is null");
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    if (!ValidateRuleForAdd(*rule)) {
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = redirectors_.find(redirectorId);
    if (it == redirectors_.end()) {
        NETMGR_EXT_LOG_E("AddRedirectRule redirector not found: %{public}s", redirectorId.c_str());
        return TRAFFICFILTER_ERROR_NOT_FOUND;
    }
    auto& redirector = it->second;
    std::string chainName = NetTrafficFilterIptablesCommandBuilder::GenerateChainName(
        redirector->GetCallingUid(), redirector->GetGroupId());
    TrafficFilterHookPoint newHookPoint = static_cast<TrafficFilterHookPoint>(rule->hookPoint_);
    std::vector<TrafficFilterRedirectRule> oldRules = redirector->GetRules();
    std::set<TrafficFilterHookPoint> oldHookPoints = redirector->GetUsedHookPoints();
    std::set<TrafficFilterHookPoint> affectedHookPoints = oldHookPoints;
    affectedHookPoints.insert(newHookPoint);
    int32_t addRet = redirector->AddRuleWithPriority(*rule);
    if (addRet != TRAFFICFILTER_OK) {
        NETMGR_EXT_LOG_E("AddRedirectRule failed to add rule with priority");
        redirector->RestoreRules(oldRules);
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    if (isGloballyEnabled_) {
        if (ApplyRulesToChain(redirector, chainName) != TRAFFICFILTER_OK) {
            if (RollbackRedirectorRules(redirector, chainName, oldRules, affectedHookPoints) != TRAFFICFILTER_OK) {
                NETMGR_EXT_LOG_E("Rollback failed after ApplyRulesToChain failure");
            }
            return -1;
        }
        if (ApplyGlobalJumpRules(newHookPoint) != TRAFFICFILTER_OK) {
            if (RollbackRedirectorRules(redirector, chainName, oldRules, affectedHookPoints) != TRAFFICFILTER_OK) {
                NETMGR_EXT_LOG_E("Rollback failed after ApplyGlobalJumpRules failure");
            }
            return -1;
        }
    } else {
        NETMGR_EXT_LOG_I("Global disabled, rules added internally but not applied to iptables");
        if (redirector->HasRules() && !redirector->IsPaused()) {
            redirector->SetPaused(true);
        }
    }
    NETMGR_EXT_LOG_I("Redirect rule added successfully");
    return TRAFFICFILTER_OK;
}

int32_t NetTrafficFilterRedirectManager::ClearRedirectRule(const std::string& redirectorId)
{
    NETMGR_EXT_LOG_I("ClearRedirectRule called: redirectorId=%{public}s", redirectorId.c_str());
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = redirectors_.find(redirectorId);
    if (it == redirectors_.end()) {
        NETMGR_EXT_LOG_E("ClearRedirectRule redirector not found: %{public}s", redirectorId.c_str());
        return -1;
    }

    auto& redirector = it->second;
    std::string chainName = NetTrafficFilterIptablesCommandBuilder::GenerateChainName(
        redirector->GetCallingUid(), redirector->GetGroupId());

    NETMGR_EXT_LOG_I("Removing jump rules for hookPoints");
    std::set<TrafficFilterHookPoint> usedHookPoints = redirector->GetUsedHookPoints();
    for (auto hookPoint : usedHookPoints) {
        std::string hookPointName = NetTrafficFilterIptablesCommandBuilder::GetHookPointName(hookPoint);
        std::string jumpCmd = NetTrafficFilterIptablesCommandBuilder::BuildDeleteJumpCommand(
            hookPointName, chainName);
        if (ExecuteIptablesCommand(jumpCmd, TrafficFilterIPFamily::IP_FAMILY_V4V6) != TRAFFICFILTER_OK) {
            NETMGR_EXT_LOG_I("Jump rule not found for hook point %{public}d",
                static_cast<int32_t>(hookPoint));
        }
    }

    std::string clearChainCmd = NetTrafficFilterIptablesCommandBuilder::BuildFlushChainCommand(chainName);
    NETMGR_EXT_LOG_I("Flushing chain for clear rules: %{public}s", chainName.c_str());
    int32_t ret = ExecuteIptablesCommand(clearChainCmd, TrafficFilterIPFamily::IP_FAMILY_V4V6);
    if (ret != TRAFFICFILTER_OK) {
        NETMGR_EXT_LOG_E("Failed to flush chain during clear rules");
        return -1;
    }

    if (redirector->ClearRules() != TRAFFICFILTER_OK) {
        NETMGR_EXT_LOG_W("Failed to clear rules from internal storage");
    }
    NETMGR_EXT_LOG_I("Cleared all rules from redirector: %{public}s", redirectorId.c_str());
    return TRAFFICFILTER_OK;
}

int32_t NetTrafficFilterRedirectManager::PauseAllRedirectors()
{
    NETMGR_EXT_LOG_I("PauseAllRedirectors called");
    std::lock_guard<std::mutex> lock(mutex_);
    if (redirectors_.empty()) {
        NETMGR_EXT_LOG_I("No redirectors to pause");
        return TRAFFICFILTER_OK;
    }

    for (const auto& [redirectorId, redirector] : redirectors_) {
        if (redirector == nullptr) {
            continue;
        }
        if (redirector->IsPaused()) {
            NETMGR_EXT_LOG_I("Redirector %{public}s already paused, skip", redirectorId.c_str());
            continue;
        }
        if (!redirector->HasRules()) {
            continue;
        }

        std::string chainName = NetTrafficFilterIptablesCommandBuilder::GenerateChainName(
            redirector->GetCallingUid(), redirector->GetGroupId());

        auto usedHookPoints = redirector->GetUsedHookPoints();
        for (auto hookPoint : usedHookPoints) {
            std::string hookPointName = NetTrafficFilterIptablesCommandBuilder::GetHookPointName(hookPoint);
            std::string jumpCmd = NetTrafficFilterIptablesCommandBuilder::BuildDeleteJumpCommand(
                hookPointName, chainName);
            if (ExecuteIptablesCommand(jumpCmd, TrafficFilterIPFamily::IP_FAMILY_V4V6) != TRAFFICFILTER_OK) {
                NETMGR_EXT_LOG_I("Jump rule not found for hook point %{public}d",
                    static_cast<int32_t>(hookPoint));
            }
        }

        redirector->SetPaused(true);
    }

    NETMGR_EXT_LOG_I("Paused all redirectors");
    return TRAFFICFILTER_OK;
}

int32_t NetTrafficFilterRedirectManager::ResumeAllRedirectors()
{
    NETMGR_EXT_LOG_I("ResumeAllRedirectors called");
    std::lock_guard<std::mutex> lock(mutex_);
    if (redirectors_.empty()) {
        NETMGR_EXT_LOG_I("No redirectors to resume");
        return TRAFFICFILTER_OK;
    }

    for (const auto& [redirectorId, redirector] : redirectors_) {
        if (redirector == nullptr) {
            continue;
        }
        if (!redirector->IsPaused()) {
            NETMGR_EXT_LOG_I("Redirector %{public}s not paused, skip resuming", redirectorId.c_str());
            continue;
        }

        redirector->SetPaused(false);
    }

    std::set<TrafficFilterHookPoint> hookPointsToReorder;
    for (const auto& [redirectorId, redirector] : redirectors_) {
        if (redirector->HasRules() && !redirector->IsPaused()) {
            auto usedHookPoints = redirector->GetUsedHookPoints();
            for (auto hookPoint : usedHookPoints) {
                hookPointsToReorder.insert(hookPoint);
            }
        }
    }

    for (auto hookPoint : hookPointsToReorder) {
        int32_t retV4 = UpdateGlobalJumpRules(hookPoint, TrafficFilterIPFamily::IP_FAMILY_V4);
        if (retV4 != TRAFFICFILTER_OK) {
            NETMGR_EXT_LOG_E("Failed to update IPv4 jump rules for hook point %{public}d",
                static_cast<int32_t>(hookPoint));
            return -1;
        }
        int32_t retV6 = UpdateGlobalJumpRules(hookPoint, TrafficFilterIPFamily::IP_FAMILY_V6);
        if (retV6 != TRAFFICFILTER_OK) {
            NETMGR_EXT_LOG_E("Failed to update IPv6 jump rules for hook point %{public}d",
                static_cast<int32_t>(hookPoint));
            return -1;
        }
    }

    NETMGR_EXT_LOG_I("Resumed all redirectors");
    return TRAFFICFILTER_OK;
}

int32_t NetTrafficFilterRedirectManager::PauseRedirectorsByBundleName(const std::string& bundleName)
{
    NETMGR_EXT_LOG_I("PauseRedirectorsByBundleName called: bundleName=%{public}s", bundleName.c_str());
    std::lock_guard<std::mutex> lock(mutex_);

    auto mapIt = bundleNameToRedirectorsMap_.find(bundleName);
    if (mapIt == bundleNameToRedirectorsMap_.end()) {
        NETMGR_EXT_LOG_I("No redirectors found for bundleName: %{public}s", bundleName.c_str());
        return TRAFFICFILTER_OK;
    }

    for (const auto& redirectorId : mapIt->second) {
        auto it = redirectors_.find(redirectorId);
        if (it == redirectors_.end()) {
            continue;
        }
        auto& redirector = it->second;
        if (redirector == nullptr) {
            continue;
        }
        if (redirector->IsPaused() || !redirector->HasRules()) {
            NETMGR_EXT_LOG_I("Redirector %{public}s already paused or has no rules, skip", redirectorId.c_str());
            continue;
        }

        std::string chainName = NetTrafficFilterIptablesCommandBuilder::GenerateChainName(
            redirector->GetCallingUid(), redirector->GetGroupId());

        auto usedHookPoints = redirector->GetUsedHookPoints();
        for (auto hookPoint : usedHookPoints) {
            std::string hookPointName = NetTrafficFilterIptablesCommandBuilder::GetHookPointName(hookPoint);
            std::string jumpCmd = NetTrafficFilterIptablesCommandBuilder::BuildDeleteJumpCommand(
                hookPointName, chainName);
            if (ExecuteIptablesCommand(jumpCmd, TrafficFilterIPFamily::IP_FAMILY_V4V6) != TRAFFICFILTER_OK) {
                NETMGR_EXT_LOG_I("Jump rule not found for hook point %{public}d",
                    static_cast<int32_t>(hookPoint));
            }
        }

        redirector->SetPaused(true);
    }

    NETMGR_EXT_LOG_I("Paused redirectors for bundleName: %{public}s", bundleName.c_str());
    return TRAFFICFILTER_OK;
}

int32_t NetTrafficFilterRedirectManager::ResumeRedirectorStateByBundleName(const std::string& bundleName)
{
    NETMGR_EXT_LOG_I("ResumeRedirectorStateByBundleName called: bundleName=%{public}s", bundleName.c_str());
    std::lock_guard<std::mutex> lock(mutex_);

    auto mapIt = bundleNameToRedirectorsMap_.find(bundleName);
    if (mapIt == bundleNameToRedirectorsMap_.end()) {
        NETMGR_EXT_LOG_I("No redirectors found for bundleName: %{public}s", bundleName.c_str());
        return TRAFFICFILTER_OK;
    }

    for (const auto& redirectorId : mapIt->second) {
        auto it = redirectors_.find(redirectorId);
        if (it == redirectors_.end()) {
            continue;
        }
        auto& redirector = it->second;
        if (redirector == nullptr) {
            continue;
        }
        if (!redirector->IsPaused()) {
            NETMGR_EXT_LOG_I("Redirector %{public}s not paused, skip resuming", redirectorId.c_str());
            continue;
        }
        redirector->SetPaused(false);
    }
    NETMGR_EXT_LOG_I("Resumed redirector state for bundleName: %{public}s", bundleName.c_str());
    return TRAFFICFILTER_OK;
}

int32_t NetTrafficFilterRedirectManager::RebuildRedirectorRulesByBundleName(const std::string& bundleName)
{
    NETMGR_EXT_LOG_I("RebuildRedirectorRulesByBundleName called: bundleName=%{public}s", bundleName.c_str());
    std::lock_guard<std::mutex> lock(mutex_);
    std::set<TrafficFilterHookPoint> hookPointsToReorder;
    auto mapIt = bundleNameToRedirectorsMap_.find(bundleName);
    if (mapIt == bundleNameToRedirectorsMap_.end()) {
        NETMGR_EXT_LOG_I("No redirectors found for bundleName: %{public}s", bundleName.c_str());
    } else {
        for (const auto& redirectorId : mapIt->second) {
            auto it = redirectors_.find(redirectorId);
            if (it == redirectors_.end()) {
                continue;
            }
            auto redirector = it->second;
            if (redirector == nullptr) {
                continue;
            }
            if (!redirector->HasRules() || redirector->IsPaused()) {
                continue;
            }
            auto usedHookPoints = redirector->GetUsedHookPoints();
            for (auto hookPoint : usedHookPoints) {
                hookPointsToReorder.insert(hookPoint);
            }
        }
    }
    for (auto hookPoint : hookPointsToReorder) {
        if (UpdateGlobalJumpRules(hookPoint, TrafficFilterIPFamily::IP_FAMILY_V4) != TRAFFICFILTER_OK) {
            NETMGR_EXT_LOG_E("Failed to update IPv4 jump rules for hook point %{public}d",
                static_cast<int32_t>(hookPoint));
            return -1;
        }
        if (UpdateGlobalJumpRules(hookPoint, TrafficFilterIPFamily::IP_FAMILY_V6) != TRAFFICFILTER_OK) {
            NETMGR_EXT_LOG_E("Failed to update IPv6 jump rules for hook point %{public}d",
                static_cast<int32_t>(hookPoint));
            return -1;
        }
    }
    NETMGR_EXT_LOG_I("Rebuilt redirector rules for bundleName: %{public}s", bundleName.c_str());
    return TRAFFICFILTER_OK;
}

int32_t NetTrafficFilterRedirectManager::ResumeRedirectorsByBundleName(const std::string& bundleName)
{
    int32_t ret = ResumeRedirectorStateByBundleName(bundleName);
    if (ret != TRAFFICFILTER_OK) {
        return ret;
    }
    return RebuildRedirectorRulesByBundleName(bundleName);
}

int32_t NetTrafficFilterRedirectManager::ExecuteIptablesCommand(
    const std::string& command, TrafficFilterIPFamily family)
{
    if (command.empty()) {
        NETMGR_EXT_LOG_E("empty iptables command");
        return -1;
    }
    std::string respond;
    NetsysNative::IptablesType ipType = NetsysNative::IptablesType::IPTYPE_IPV4;
    switch (family) {
        case TrafficFilterIPFamily::IP_FAMILY_UNSPEC:
            ipType = NetsysNative::IptablesType::IPTYPE_IPV4V6;
            break;
        case TrafficFilterIPFamily::IP_FAMILY_V4:
            ipType = NetsysNative::IptablesType::IPTYPE_IPV4;
            break;
        case TrafficFilterIPFamily::IP_FAMILY_V6:
            ipType = NetsysNative::IptablesType::IPTYPE_IPV6;
            break;
        case TrafficFilterIPFamily::IP_FAMILY_V4V6:
            ipType = NetsysNative::IptablesType::IPTYPE_IPV4V6;
            break;
        default:
            NETMGR_EXT_LOG_E("invalid ipType");
            return -1;
    }
    int32_t ret = NetsysController::GetInstance().SetIptablesCommandForRes(
        command, respond, ipType);
    if (ret != 0) {
        NETMGR_EXT_LOG_E("Failed to execute iptables command: %{private}s, error: %{public}s",
            command.c_str(), respond.c_str());
        return -1;
    }
    NETMGR_EXT_LOG_I("Executed iptables command: %{private}s", command.c_str());
    return TRAFFICFILTER_OK;
}

std::string NetTrafficFilterRedirectManager::GenerateRedirectorId()
{
    uint32_t id = redirectorIdCounter_++;
    if (redirectorIdCounter_ == 0) {
        redirectorIdCounter_ = REDIRECTOR_ID_START;
    }
    return "redir_" + std::to_string(id);
}

bool NetTrafficFilterRedirectManager::ValidateRedirectRuleFields(
    const TrafficFilterRedirectRule& rule)
{
    if (rule.priority_ < NETTRAFFICFILTER_MIN_PRIORITY ||
        rule.priority_ > NETTRAFFICFILTER_MAX_PRIORITY) {
        return false;
    }
    if (rule.protocol_ != NETTRAFFICFILTER_PROTO_TCP) {
        return false;
    }
    if (rule.hookPoint_ != static_cast<int32_t>(TrafficFilterHookPoint::HOOK_PREROUTING) &&
        rule.hookPoint_ != static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT)) {
        return false;
    }
    if (!ValidateIPMatchType(rule.srcIp_.type_)) {
        return false;
    }
    if (!ValidateIPMatchType(rule.dstIp_.type_)) {
        return false;
    }
    if (!ValidatePortMatchType(rule.srcPort_.type_)) {
        return false;
    }
    if (!ValidatePortMatchType(rule.dstPort_.type_)) {
        return false;
    }
    return true;
}

void NetTrafficFilterRedirectManager::HandleTrafficFilterObserverRegistration(
    const std::string& bundleName, int32_t uid, int32_t pid)
{
    if (bundleName.empty()) {
        return;
    }
    std::lock_guard<std::mutex> lock(observerMutex_);
    if (uidToObserverMap_.find(uid) != uidToObserverMap_.end()) {
        NETMGR_EXT_LOG_I("Observer for uid=%{public}d already exists, skip registration", uid);
        return;
    }
    std::vector<std::string> list = {bundleName, bundleName + ":trafficfilter"};
    sptr<TrafficFilterHapObserver> observer =
        new TrafficFilterHapObserver(*this, bundleName, uid);
    Singleton<AppExecFwk::AppMgrClient>::GetInstance().RegisterApplicationStateObserver(observer, list);
    uidToObserverMap_[uid] = observer;
    NETMGR_EXT_LOG_I("TrafficFilter observer registered: bundleName=%{public}s, uid=%{public}d, pid=%{public}d",
        bundleName.c_str(), uid, pid);
}

void NetTrafficFilterRedirectManager::UnregisterTrafficFilterObserver(
    int32_t uid, const sptr<TrafficFilterHapObserver>& observer)
{
    std::lock_guard<std::mutex> lock(observerMutex_);
    auto it = uidToObserverMap_.find(uid);
    if (it != uidToObserverMap_.end() && it->second == observer) {
        uidToObserverMap_.erase(it);
        NETMGR_EXT_LOG_I("TrafficFilter observer unregistered: uid=%{public}d", uid);
    }
}

int32_t NetTrafficFilterRedirectManager::CleanupRedirectorsByUid(int32_t uid, int32_t pid)
{
    NETMGR_EXT_LOG_I("CleanupRedirectorsByUid: uid=%{public}d, pid=%{public}d", uid, pid);
    std::vector<std::string> toRemove;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& [redirectorId, redirector] : redirectors_) {
            if (redirector == nullptr) {
                continue;
            }
            if (redirector->GetCallingUid() == uid) {
                if (pid == 0 || redirector->GetCallingPid() == pid) {
                    toRemove.push_back(redirectorId);
                }
            }
        }
    }
    for (const auto& redirectorId : toRemove) {
        NETMGR_EXT_LOG_I("Cleaning up orphaned redirector: %{public}s", redirectorId.c_str());
        DestroyRedirector(redirectorId);
    }
    return TRAFFICFILTER_OK;
}

int32_t NetTrafficFilterRedirectManager::CleanupRedirectorsByBundleName(
    const std::string& bundleName, int32_t uid, int32_t pid)
{
    NETMGR_EXT_LOG_I("CleanupRedirectorsByBundleName: bundleName=%{public}s, uid=%{public}d, pid=%{public}d",
        bundleName.c_str(), uid, pid);
    std::vector<std::string> toRemove;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = bundleNameToRedirectorsMap_.find(bundleName);
        if (it == bundleNameToRedirectorsMap_.end()) {
            NETMGR_EXT_LOG_I("No redirectors found for bundleName=%{public}s", bundleName.c_str());
            return TRAFFICFILTER_OK;
        }
        for (const auto& redirectorId : it->second) {
            auto redirectorIt = redirectors_.find(redirectorId);
            if (redirectorIt == redirectors_.end()) {
                continue;
            }
            if (redirectorIt->second == nullptr) {
                continue;
            }
            if (redirectorIt->second->GetCallingUid() != uid) {
                continue;
            }
            if (pid != 0 && redirectorIt->second->GetCallingPid() != pid) {
                continue;
            }
            toRemove.push_back(redirectorId);
        }
    }
    for (const auto& redirectorId : toRemove) {
        NETMGR_EXT_LOG_I("Cleaning up orphaned redirector by bundle: %{public}s", redirectorId.c_str());
        DestroyRedirector(redirectorId);
    }
    return TRAFFICFILTER_OK;
}
void NetTrafficFilterRedirectManager::TrafficFilterHapObserver::OnProcessDied(
    const AppExecFwk::ProcessData& processData)
{
    NETMGR_EXT_LOG_I("TrafficFilter OnProcessDied: died uid=%{public}d, died pid=%{public}d, observed uid=%{public}d",
        processData.uid, processData.pid, uid_);

    if (processData.uid != uid_) {
        NETMGR_EXT_LOG_I("OnProcessDied: uid mismatch, ignore");
        return;
    }

    redirectManager_.CleanupRedirectorsByBundleName(bundleName_, processData.uid, processData.pid);
    redirectManager_.UnregisterTrafficFilterObserver(processData.uid, this);
}

int32_t NetTrafficFilterRedirectManager::GlobalEnableTrafficFilter()
{
    NETMGR_EXT_LOG_I("GlobalEnableTrafficFilter called");
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (isGloballyEnabled_) {
            NETMGR_EXT_LOG_I("Global traffic filter already enabled");
            return TRAFFICFILTER_OK;
        }
    }
    int32_t ret = ResumeAllRedirectors();
    if (ret != TRAFFICFILTER_OK) {
        NETMGR_EXT_LOG_E("GlobalEnableTrafficFilter failed to resume redirectors, ret: %{public}d", ret);
        return ret;
    }
    {
        std::lock_guard<std::mutex> lock(mutex_);
        isGloballyEnabled_ = true;
    }
    NETMGR_EXT_LOG_I("Global traffic filter enabled successfully");
    return TRAFFICFILTER_OK;
}

int32_t NetTrafficFilterRedirectManager::GlobalDisableTrafficFilter()
{
    NETMGR_EXT_LOG_I("GlobalDisableTrafficFilter called");
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!isGloballyEnabled_) {
            NETMGR_EXT_LOG_I("Global traffic filter already disabled");
            return TRAFFICFILTER_OK;
        }
    }
    int32_t ret = PauseAllRedirectors();
    if (ret != TRAFFICFILTER_OK) {
        NETMGR_EXT_LOG_E("GlobalDisableTrafficFilter failed to pause redirectors, ret: %{public}d", ret);
        return ret;
    }
    {
        std::lock_guard<std::mutex> lock(mutex_);
        isGloballyEnabled_ = false;
    }
    NETMGR_EXT_LOG_I("Global traffic filter disabled successfully");
    return TRAFFICFILTER_OK;
}

int32_t NetTrafficFilterRedirectManager::GetTrafficFilterGlobalStatus(bool& isEnabled)
{
    NETMGR_EXT_LOG_I("GetTrafficFilterGlobalStatus called");
    std::lock_guard<std::mutex> lock(mutex_);
    isEnabled = isGloballyEnabled_;
    NETMGR_EXT_LOG_I("GetTrafficFilterGlobalStatus result: %{public}d", isEnabled);
    return TRAFFICFILTER_OK;
}

int32_t NetTrafficFilterRedirectManager::QueryProcess(const std::string& srcIp, uint16_t srcPort,
    const std::string& dstIp, uint16_t dstPort, uint8_t protocol, uint32_t& uid, uint32_t& pid)
{
    NetPortStatesInfo netPortStatesInfo;
    int32_t ret = NetsysController::GetInstance().GetSystemNetPortStates(netPortStatesInfo);
    if (ret != 0) {
        NETMGR_EXT_LOG_E("QueryProcess: GetSystemNetPortStates failed, ret=%{public}d", ret);
        return ret;
    }
    if (protocol == NETTRAFFICFILTER_PROTO_TCP) {
        for (const auto& tcpInfo : netPortStatesInfo.tcpNetPortStatesInfo_) {
            if (MatchTcpConnection(tcpInfo, srcIp, srcPort, dstIp, dstPort)) {
                uid = tcpInfo.tcpUid_;
                pid = tcpInfo.tcpPid_;
                NETMGR_EXT_LOG_I("QueryProcess found TCP connection: uid=%{public}u, pid=%{public}u", uid, pid);
                return TRAFFICFILTER_OK;
            }
        }
    } else if (protocol == NETTRAFFICFILTER_PROTO_UDP) {
        for (const auto& udpInfo : netPortStatesInfo.udpNetPortStatesInfo_) {
            if (MatchUdpConnection(udpInfo, srcIp, srcPort, dstIp, dstPort)) {
                uid = udpInfo.udpUid_;
                pid = udpInfo.udpPid_;
                NETMGR_EXT_LOG_I("QueryProcess found UDP connection: uid=%{public}u, pid=%{public}u", uid, pid);
                return TRAFFICFILTER_OK;
            }
        }
    }
    NETMGR_EXT_LOG_E("QueryProcess: no matching process found");
    return TRAFFICFILTER_ERROR_NOT_FOUND;
}

bool NetTrafficFilterRedirectManager::MatchTcpConnection(const TcpNetPortStatesInfo& tcpInfo,
    const std::string& srcIp, uint16_t srcPort, const std::string& dstIp, uint16_t dstPort)
{
    bool localMatch =
        (tcpInfo.tcpLocalIp_ == dstIp &&
            (dstPort == 0 || tcpInfo.tcpLocalPort_ == dstPort)) ||
        (tcpInfo.tcpLocalIp_ == srcIp &&
            (srcPort == 0 || tcpInfo.tcpLocalPort_ == srcPort));
    bool remoteMatch =
        (tcpInfo.tcpRemoteIp_ == srcIp &&
            (srcPort == 0 || tcpInfo.tcpRemotePort_ == srcPort)) ||
        (tcpInfo.tcpRemoteIp_ == dstIp &&
            (dstPort == 0 || tcpInfo.tcpRemotePort_ == dstPort));
    return localMatch && remoteMatch;
}

bool NetTrafficFilterRedirectManager::MatchUdpConnection(const UdpNetPortStatesInfo& udpInfo,
    const std::string& srcIp, uint16_t srcPort, const std::string& dstIp, uint16_t dstPort)
{
    bool srcMatch = udpInfo.udpLocalIp_ == srcIp &&
        (srcPort == 0 || udpInfo.udpLocalPort_ == srcPort);

    bool dstMatch = udpInfo.udpLocalIp_ == dstIp &&
        (dstPort == 0 || udpInfo.udpLocalPort_ == dstPort);
    return srcMatch || dstMatch;
}
} // namespace NetManagerStandard
} // namespace OHOS
