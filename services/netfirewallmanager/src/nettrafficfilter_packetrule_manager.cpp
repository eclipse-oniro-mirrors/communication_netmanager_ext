/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "nettrafficfilter_packetrule_manager.h"
#include "nettrafficfilter_iptables_command_builder.h"
#include "netsys_controller.h"
#include "net_manager_constants.h"
#include "net_manager_ext_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "ipc_skeleton.h"
#include "netfirewall_uid_rule_generator.h"
#include <charconv>

namespace OHOS {
namespace NetManagerStandard {

namespace {
constexpr int32_t HOOK_POINTS[] = {
    static_cast<int32_t>(TrafficFilterHookPoint::HOOK_INPUT),
    static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT),
    static_cast<int32_t>(TrafficFilterHookPoint::HOOK_FORWARD)
};
constexpr size_t HOOK_POINT_COUNT = sizeof(HOOK_POINTS) / sizeof(HOOK_POINTS[0]);
constexpr uint32_t MAX_PACKET_RULE_COUNT = 2000;
constexpr TrafficFilterIPFamily FAMILIES[] = {
    TrafficFilterIPFamily::IP_FAMILY_V4,
    TrafficFilterIPFamily::IP_FAMILY_V6
};
constexpr size_t FAMILY_COUNT = sizeof(FAMILIES) / sizeof(FAMILIES[0]);

static bool IsIpMatchV6(const TrafficFilterIPMatch& ipMatch)
{
    switch (ipMatch.type_) {
        case static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE):
            return ipMatch.single_.family_ == static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V6);
        case static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_CIDR):
            return ipMatch.cidr_.base_.family_ == static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V6);
        case static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_RANGE):
            return ipMatch.range_.start_.family_ == static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V6);
        case static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_MULTI):
            if (ipMatch.multi_.ipCount_ > 0) {
                return ipMatch.multi_.ips_[0].family_ == static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V6);
            }
            return false;
        default:
            return false;
    }
}

static bool IsIpMatchV4(const TrafficFilterIPMatch& ipMatch)
{
    switch (ipMatch.type_) {
        case static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE):
            return ipMatch.single_.family_ == static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4);
        case static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_CIDR):
            return ipMatch.cidr_.base_.family_ == static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4);
        case static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_RANGE):
            return ipMatch.range_.start_.family_ == static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4);
        case static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_MULTI):
            if (ipMatch.multi_.ipCount_ > 0) {
                return ipMatch.multi_.ips_[0].family_ == static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4);
            }
            return false;
        default:
            return false;
    }
}

static TrafficFilterIPFamily GetIPFamilyFromMatch(const TrafficFilterIPMatch& ipMatch)
{
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
            return TrafficFilterIPFamily::IP_FAMILY_UNSPEC;
        default:
            return TrafficFilterIPFamily::IP_FAMILY_UNSPEC;
    }
}

static bool IsValidIPFamilyForPacketRule(int32_t family)
{
    return family == static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4) ||
           family == static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V6);
}

static bool ValidateIPMatchForPacketRule(const TrafficFilterIPMatch& ipMatch)
{
    switch (ipMatch.type_) {
        case static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY):
            return true;
        case static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE):
            return IsValidIPFamilyForPacketRule(ipMatch.single_.family_);
        case static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_CIDR):
            return IsValidIPFamilyForPacketRule(ipMatch.cidr_.base_.family_);
        case static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_RANGE): {
            int32_t startFamily = ipMatch.range_.start_.family_;
            int32_t endFamily = ipMatch.range_.end_.family_;
            return IsValidIPFamilyForPacketRule(startFamily) &&
                   IsValidIPFamilyForPacketRule(endFamily) &&
                   startFamily == endFamily;
        }
        case static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_MULTI): {
            if (ipMatch.multi_.ipCount_ == 0 ||
                ipMatch.multi_.ipCount_ > NETTRAFFICFILTER_MAX_MULTI_IP_COUNT) {
                NETMGR_EXT_LOG_E("invalid multi ip count %{public}u", ipMatch.multi_.ipCount_);
                return false;
            }
            int32_t firstFamily = ipMatch.multi_.ips_[0].family_;
            if (!IsValidIPFamilyForPacketRule(firstFamily)) {
                return false;
            }
            for (uint32_t i = 1; i < ipMatch.multi_.ipCount_; i++) {
                if (!IsValidIPFamilyForPacketRule(ipMatch.multi_.ips_[i].family_) ||
                    ipMatch.multi_.ips_[i].family_ != firstFamily) {
                    NETMGR_EXT_LOG_E("multi IP family mismatch at index %{public}u", i);
                    return false;
                }
            }
            return true;
        }
        default:
            return false;
    }
}

static bool ValidateIPFamilyConsistencyForPacketRule(
    const TrafficFilterIPMatch& srcIp, const TrafficFilterIPMatch& dstIp)
{
    TrafficFilterIPFamily srcFamily = GetIPFamilyFromMatch(srcIp);
    TrafficFilterIPFamily dstFamily = GetIPFamilyFromMatch(dstIp);
    if (srcFamily == TrafficFilterIPFamily::IP_FAMILY_UNSPEC ||
        dstFamily == TrafficFilterIPFamily::IP_FAMILY_UNSPEC) {
        return true;
    }
    return srcFamily == dstFamily;
}

static bool PacketRulePriorityLess(const TrafficFilterPacketRule& a, const TrafficFilterPacketRule& b)
{
    return a.priority_ < b.priority_;
}

static int32_t ExecuteIptablesForFamilies(const std::string& cmd, bool stopOnError, const char* context)
{
    int32_t lastRet = FIREWALL_SUCCESS;
    for (size_t i = 0; i < FAMILY_COUNT; ++i) {
        int32_t ret = NetTrafficFilterIptablesCommandBuilder::ExecuteIptablesCommand(cmd, FAMILIES[i]);
        if (ret != FIREWALL_SUCCESS) {
            if (context != nullptr && context[0] != '\0') {
                NETMGR_EXT_LOG_W("%s failed, family=%{public}d, ret=%{public}d",
                    context, static_cast<int32_t>(FAMILIES[i]), ret);
            }
            lastRet = ret;
            if (stopOnError) {
                return ret;
            }
        }
    }
    return lastRet;
}

static std::vector<TrafficFilterPacketRule>* GetRulesByHookPoint(HookPointRules& rules, int32_t hookPoint)
{
    switch (static_cast<TrafficFilterHookPoint>(hookPoint)) {
        case TrafficFilterHookPoint::HOOK_INPUT:   return &rules.input;
        case TrafficFilterHookPoint::HOOK_OUTPUT:  return &rules.output;
        case TrafficFilterHookPoint::HOOK_FORWARD: return &rules.forward;
        default: return nullptr;
    }
}

static const std::vector<TrafficFilterPacketRule>* GetRulesByHookPoint(
    const HookPointRules& rules, int32_t hookPoint)
{
    switch (static_cast<TrafficFilterHookPoint>(hookPoint)) {
        case TrafficFilterHookPoint::HOOK_INPUT:   return &rules.input;
        case TrafficFilterHookPoint::HOOK_OUTPUT:  return &rules.output;
        case TrafficFilterHookPoint::HOOK_FORWARD: return &rules.forward;
        default: return nullptr;
    }
}
} // namespace

NetTrafficFilterPacketRuleManager& NetTrafficFilterPacketRuleManager::GetInstance()
{
    static NetTrafficFilterPacketRuleManager instance;
    return instance;
}

NetTrafficFilterPacketRuleManager::NetTrafficFilterPacketRuleManager()
{
}

NetTrafficFilterPacketRuleManager::~NetTrafficFilterPacketRuleManager()
{
}

static bool GetChainNameByHookPoint(const QueueInfo& info, int32_t hookPoint, std::string& chainName)
{
    switch (static_cast<TrafficFilterHookPoint>(hookPoint)) {
        case TrafficFilterHookPoint::HOOK_INPUT:    chainName = info.chainNameIn;  return true;
        case TrafficFilterHookPoint::HOOK_OUTPUT:   chainName = info.chainNameOut; return true;
        case TrafficFilterHookPoint::HOOK_FORWARD:  chainName = info.chainNameFwd; return true;
        default: return false;
    }
}

static bool HasV6Address(const TrafficFilterPacketRule& rule)
{
    return IsIpMatchV6(rule.srcIp_) || IsIpMatchV6(rule.dstIp_);
}

static bool HasV4Address(const TrafficFilterPacketRule& rule)
{
    return IsIpMatchV4(rule.srcIp_) || IsIpMatchV4(rule.dstIp_);
}

static bool IsRuleForFamily(const TrafficFilterPacketRule& rule, TrafficFilterIPFamily family)
{
    if (family == TrafficFilterIPFamily::IP_FAMILY_V4) {
        return !HasV6Address(rule) || HasV4Address(rule);
    }
    if (family == TrafficFilterIPFamily::IP_FAMILY_V6) {
        return !HasV4Address(rule) || HasV6Address(rule);
    }
    NETMGR_EXT_LOG_E("IsRuleForFamily: unsupported family=%{public}d", static_cast<int32_t>(family));
    return false;
}

static const std::string IPTABLES_APPEND_ACTION = " -A ";
static const std::string IPTABLES_DELETE_ACTION = " -D ";

static std::string BuildDeleteCommandFromAppend(const std::string& appendCmd)
{
    std::string deleteCmd = appendCmd;
    size_t pos = deleteCmd.find(IPTABLES_APPEND_ACTION);
    if (pos != std::string::npos) {
        deleteCmd.replace(pos, IPTABLES_APPEND_ACTION.length(), IPTABLES_DELETE_ACTION);
    }
    return deleteCmd;
}

static void RollbackAppliedCommands(const std::vector<std::string>& appliedCommands,
    TrafficFilterIPFamily family)
{
    for (auto it = appliedCommands.rbegin(); it != appliedCommands.rend(); ++it) {
        std::string deleteCmd = BuildDeleteCommandFromAppend(*it);
        NetTrafficFilterIptablesCommandBuilder::ExecuteIptablesCommand(deleteCmd, family);
    }
}

static int32_t ExecuteCommandsForRule(const std::vector<std::string>& commands,
    std::vector<std::string>& appliedCommands, TrafficFilterIPFamily family)
{
    for (const auto& cmd : commands) {
        if (cmd.empty()) {
            continue;
        }
        int32_t ret = NetTrafficFilterIptablesCommandBuilder::ExecuteIptablesCommand(cmd, family);
        if (ret != FIREWALL_SUCCESS) {
            NETMGR_EXT_LOG_E("insert rule failed, ret=%{public}d, rollback %{public}zu commands",
                ret, appliedCommands.size());
            RollbackAppliedCommands(appliedCommands, family);
            return ret;
        }
        appliedCommands.push_back(cmd);
    }
    return FIREWALL_SUCCESS;
}

int32_t NetTrafficFilterPacketRuleManager::ExecuteRulesForIpFamily(
    const std::vector<TrafficFilterPacketRule>& rules, const std::string& chainName, int32_t queueNum,
    TrafficFilterIPFamily family)
{
    std::vector<std::string> appliedCommands;
    for (const auto& rule : rules) {
        if (!IsRuleForFamily(rule, family)) {
            continue;
        }
        std::vector<std::string> commands =
            NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommands(rule, chainName, queueNum);
        int32_t ret = ExecuteCommandsForRule(commands, appliedCommands, family);
        if (ret != FIREWALL_SUCCESS) {
            return ret;
        }
    }
    return FIREWALL_SUCCESS;
}

int32_t NetTrafficFilterPacketRuleManager::ApplyRulesForHookPoint(int32_t queueNum, int32_t hookPoint,
    const std::string& chainName, TrafficFilterIPFamily family)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (chainName.empty()) {
        NETMGR_EXT_LOG_E("ApplyRulesForHookPoint failed: chainName is empty");
        return FIREWALL_ERR_INVALID_PARAMETER;
    }

    std::string flushCmd = NetTrafficFilterIptablesCommandBuilder::BuildFlushChainCommand(
        chainName, IptablesName::FILTER);
    int32_t ret = NetTrafficFilterIptablesCommandBuilder::ExecuteIptablesCommand(flushCmd, family);
    if (ret != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("flush chain failed, ret=%{public}d", ret);
        return ret;
    }

    std::vector<TrafficFilterPacketRule> rules;
    {
        auto qIt = queueNumToRules_.find(queueNum);
        if (qIt != queueNumToRules_.end()) {
            const auto* ruleList = GetRulesByHookPoint(qIt->second, hookPoint);
            if (ruleList != nullptr) {
                rules = *ruleList;
            }
        }
    }
    std::sort(rules.begin(), rules.end(), PacketRulePriorityLess);

    ret = ExecuteRulesForIpFamily(rules, chainName, queueNum, family);
    if (ret != FIREWALL_SUCCESS) {
        return ret;
    }

    if (hookPoint == static_cast<int32_t>(TrafficFilterHookPoint::HOOK_INPUT)) {
        int32_t uidRet = UidRuleGenerator::GetInstance().RestoreInputUidRule(queueNum, family);
        if (uidRet != 0) {
            NETMGR_EXT_LOG_W("restore uid rule failed, ret=%{public}d", uidRet);
        }
    }
    return FIREWALL_SUCCESS;
}

int32_t NetTrafficFilterPacketRuleManager::ApplyRulesForHookPointBothFamilies(
    int32_t queueNum, int32_t hookPoint, const std::string& chainName, bool needV6)
{
    int32_t ret = ApplyRulesForHookPoint(queueNum, hookPoint, chainName, TrafficFilterIPFamily::IP_FAMILY_V4);
    if (ret != FIREWALL_SUCCESS) {
        return ret;
    }
    if (needV6) {
        ret = ApplyRulesForHookPoint(queueNum, hookPoint, chainName, TrafficFilterIPFamily::IP_FAMILY_V6);
    }
    return ret;
}

bool NetTrafficFilterPacketRuleManager::ValidateRuleParam(const sptr<TrafficFilterPacketRule>& rule)
{
    if (rule == nullptr) {
        NETMGR_EXT_LOG_E("rule is null");
        return false;
    }
    if (!ValidateIPMatchForPacketRule(rule->srcIp_) || !ValidateIPMatchForPacketRule(rule->dstIp_)) {
        NETMGR_EXT_LOG_E("invalid src or dst IP match");
        return false;
    }
    if (!ValidateIPFamilyConsistencyForPacketRule(rule->srcIp_, rule->dstIp_)) {
        NETMGR_EXT_LOG_E("src/dst IP family mismatch");
        return false;
    }
    if (!rule->srcIp_.IsValidType() || !rule->dstIp_.IsValidType() ||
        !rule->srcPort_.IsValidType() || !rule->dstPort_.IsValidType()) {
        NETMGR_EXT_LOG_E("invalid ip or port type");
        return false;
    }
    switch (static_cast<TrafficFilterHookPoint>(rule->hookPoint_)) {
        case TrafficFilterHookPoint::HOOK_INPUT:
        case TrafficFilterHookPoint::HOOK_OUTPUT:
        case TrafficFilterHookPoint::HOOK_FORWARD:
            return true;
        default:
            NETMGR_EXT_LOG_E("invalid hookPoint=%{public}d", rule->hookPoint_);
            return false;
    }
}

bool NetTrafficFilterPacketRuleManager::ParseAndValidateControllerId(const std::string& controllerId,
    int32_t& queueNum)
{
    std::string::size_type pos1 = controllerId.find(':');
    if (pos1 == std::string::npos) {
        NETMGR_EXT_LOG_E("invalid controllerId");
        return false;
    }
    const char* strStart = controllerId.data() + pos1 + 1;
    const char* strEnd = controllerId.data() + controllerId.size();
    auto [ptr, ec] = std::from_chars(strStart, strEnd, queueNum);
    if (ptr != strEnd || ec != std::errc() || queueNum <= 0) {
        return false;
    }
    return true;
}

uint32_t NetTrafficFilterPacketRuleManager::GetQueueRuleCountLocked(uint32_t queueNum) const
{
    auto qIt = queueNumToRules_.find(queueNum);
    if (qIt == queueNumToRules_.end()) {
        return 0;
    }
    return qIt->second.input.size() + qIt->second.output.size() + qIt->second.forward.size();
}

int32_t NetTrafficFilterPacketRuleManager::AddPacketRule(const std::string& controllerId,
    const sptr<TrafficFilterPacketRule>& rule)
{
    if (!ValidateRuleParam(rule)) {
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    int32_t queueNum = 0;
    if (!ParseAndValidateControllerId(controllerId, queueNum)) {
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    QueueInfo info = NetTrafficFilterNFQueueCore::GetInstance().GetQueueInfo(queueNum);
    int32_t hookPoint = rule->hookPoint_;
    std::string chainName;
    if (!GetChainNameByHookPoint(info, hookPoint, chainName)) {
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = queueNumToRuleCtx_.find(queueNum);
        if (it == queueNumToRuleCtx_.end()) {
            FilterRuleCtx ctx;
            ctx.priority = info.priority;
            ctx.chainNameIn = info.chainNameIn;
            ctx.chainNameOut = info.chainNameOut;
            ctx.chainNameFwd = info.chainNameFwd;
            queueNumToRuleCtx_[queueNum] = std::move(ctx);
        }
        if (GetQueueRuleCountLocked(queueNum) >= MAX_PACKET_RULE_COUNT) {
            return TRAFFICFILTER_ERROR_TOO_MANY_RULES;
        }
        auto* ruleList = GetRulesByHookPoint(queueNumToRules_[queueNum], hookPoint);
        if (ruleList != nullptr) {
            ruleList->push_back(*rule);
        }
    }

    if (hookPoint == static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT) &&
        rule->uidStart_ != static_cast<uint32_t>(-1)) {
        int32_t ret = UidRuleGenerator::GetInstance().HandleAddUidRule(info, rule, queueNum);
        if (ret != FIREWALL_SUCCESS) {
            return ret;
        }
    }
    bool needV6 = HasV6Address(*rule);
    int32_t ret = ApplyRulesForHookPointBothFamilies(queueNum, hookPoint, chainName, needV6);
    if (ret != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("apply rules failed, ret=%{public}d", ret);
        return ret;
    }
    return FIREWALL_SUCCESS;
}

static const std::string& GetChainNameByHookPoint(const std::string& chainNameIn,
    const std::string& chainNameOut, const std::string& chainNameFwd, int32_t hookPoint)
{
    switch (static_cast<TrafficFilterHookPoint>(hookPoint)) {
        case TrafficFilterHookPoint::HOOK_INPUT:   return chainNameIn;
        case TrafficFilterHookPoint::HOOK_OUTPUT:  return chainNameOut;
        case TrafficFilterHookPoint::HOOK_FORWARD: return chainNameFwd;
        default: {
            static const std::string empty;
            return empty;
        }
    }
}

void NetTrafficFilterPacketRuleManager::FlushChainForIpFamilies(const std::string& chainName)
{
    if (chainName.empty()) {
        NETMGR_EXT_LOG_E("FlushChainForIpFamilies failed: chainName is empty");
        return;
    }
    std::string flushCmd = NetTrafficFilterIptablesCommandBuilder::BuildFlushChainCommand(
        chainName, IptablesName::FILTER);
    ExecuteIptablesForFamilies(flushCmd, false, "flush chain");
}

void NetTrafficFilterPacketRuleManager::CleanPhysicalRules(const QueueInfo& info, const std::set<int32_t>& hookPoints)
{
    if (UidRuleGenerator::GetInstance().HandleClearUidRules(info) != FIREWALL_SUCCESS) {
#ifdef NETMGR_DEBUG
        NETMGR_EXT_LOG_W("uid rule clear warning");
#endif
    }
    const std::string* chains[] = {&info.chainNameIn, &info.chainNameOut, &info.chainNameFwd};
    for (const std::string* chain : chains) {
        FlushChainForIpFamilies(*chain);
    }
}

int32_t NetTrafficFilterPacketRuleManager::ClearPacketRule(const QueueInfo& info)
{
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t queueNum = info.queueNum;
    std::set<int32_t> hookPoints;
    {
        auto qIt = queueNumToRules_.find(queueNum);
        if (qIt != queueNumToRules_.end()) {
            for (size_t i = 0; i < HOOK_POINT_COUNT; ++i) {
                int32_t hookPoint = HOOK_POINTS[i];
                const auto* ruleList = GetRulesByHookPoint(qIt->second, hookPoint);
                if (ruleList != nullptr && !ruleList->empty()) {
                    hookPoints.insert(hookPoint);
                }
            }
        }
    }

    CleanPhysicalRules(info, hookPoints);
    {
        queueNumToRules_.erase(queueNum);
        queueNumToRuleCtx_.erase(queueNum);
    }
    return FIREWALL_SUCCESS;
}

static bool CheckNeedV6Rule(const std::map<uint32_t, HookPointRules>& rulesMap,
    int32_t queueNum, int32_t hookPoint)
{
    auto qIt = rulesMap.find(queueNum);
    if (qIt == rulesMap.end()) {
        return false;
    }
    const auto* ruleList = GetRulesByHookPoint(qIt->second, hookPoint);
    if (ruleList == nullptr || ruleList->empty()) {
        return false;
    }
    for (const auto& rule : *ruleList) {
        if (IsRuleForFamily(rule, TrafficFilterIPFamily::IP_FAMILY_V6)) {
            return true;
        }
    }
    return false;
}
} // namespace NetManagerStandard
} // namespace OHOS
