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
#include <cstdint>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <system_error>
#include "netfirewall_uid_rule_generator.h"
#include "nettrafficfilter_iptables_command_builder.h"
#include "net_manager_constants.h"

namespace OHOS {
struct RealUidRange {
    uint32_t start;
    uint32_t end;
};

const std::vector<RealUidRange> REAL_UID_RANGES = {
    {0, 65535},
    {100000, 165535},
    {20000000, 20065535},
    {30000000, 30065535},
};
namespace NetManagerStandard {
UidRuleGenerator& UidRuleGenerator::GetInstance()
{
    static UidRuleGenerator instance;
    return instance;
}

std::string UidRuleGenerator::GenerateIsolationKey(
    const std::string& bundleName,
    uint32_t groupId)
{
    std::ostringstream oss;
    oss << bundleName << "_GRP_" << groupId;
    return oss.str();
}

bool UidRuleGenerator::HasUidCondition(const sptr<TrafficFilterPacketRule>& rule)
{
    if (rule == nullptr) {
        return false;
    }
    return rule->uidStart_ != static_cast<uint32_t>(-1) || rule->uidEnd_ != static_cast<uint32_t>(-1);
}

std::vector<std::pair<uint32_t, uint32_t>> UidRuleGenerator::ClipToRealUidRanges(
    uint32_t start, uint32_t end)
{
    std::vector<std::pair<uint32_t, uint32_t>> result;
    for (const auto& range : REAL_UID_RANGES) {
        uint32_t s = std::max(start, range.start);
        uint32_t e = std::min(end, range.end);
        if (s <= e) {
            result.emplace_back(s, e);
        }
    }
    return result;
}

void UidRuleGenerator::ClearIntervalsForKey(const std::string& isolationKey)
{
    auto it = keyToIntervalStarts_.find(isolationKey);
    if (it == keyToIntervalStarts_.end()) {
        return;
    }
    for (uint32_t start : it->second) {
        uidIntervals_.erase(start);
    }
    keyToIntervalStarts_.erase(it);
}

void UidRuleGenerator::AddIntervalsForKey(const std::string& isolationKey,
    const std::vector<std::pair<uint32_t, uint32_t>>& ranges)
{
    for (const auto& [start, end] : ranges) {
        uidIntervals_[start] = {end, isolationKey};
        keyToIntervalStarts_[isolationKey].insert(start);
    }
}

int32_t UidRuleGenerator::SyncUidIntervals(const std::string& isolationKey,
    uint32_t uidStart, uint32_t uidEnd)
{
    auto ranges = ClipToRealUidRanges(uidStart, uidEnd);
    if (ranges.empty()) {
        NETMGR_EXT_LOG_E("SyncUidIntervals no real uid in range %{public}u-%{public}u", uidStart, uidEnd);
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    ClearIntervalsForKey(isolationKey);
    AddIntervalsForKey(isolationKey, ranges);

    auto ctxIt = uidRuleContexts_.find(isolationKey);
    if (ctxIt != uidRuleContexts_.end()) {
        ctxIt->second->uidStart = ranges.front().first;
        ctxIt->second->uidEnd = ranges.back().second;
    }
    return TRAFFICFILTER_OK;
}

std::string UidRuleGenerator::GenerateCTMarkMatchParam(uint32_t markValue)
{
    std::ostringstream oss;
    oss << "-m connmark --mark " << "0x" << std::hex << markValue;
    return oss.str();
}

bool UidRuleGenerator::IsUidRangeOverlap(
    uint32_t uidStart,
    uint32_t uidEnd,
    const std::string& excludeIsolationKey)
{
    auto it = uidIntervals_.lower_bound(uidStart);
    if (it != uidIntervals_.begin()) {
        auto prev = std::prev(it);
        if (prev->second.end >= uidStart && prev->second.isolationKey != excludeIsolationKey) {
            return true;
        }
    }
    while (it != uidIntervals_.end() && it->first <= uidEnd) {
        if (it->second.isolationKey != excludeIsolationKey) {
            return true;
        }
        ++it;
    }
    return false;
}

uint32_t UidRuleGenerator::AllocateNextMark()
{
    for (uint32_t i = 0; i < MAX_MARK; i++) {
        uint32_t candidate = ((nextMark_ + i) & MARK_MASK);
        if (candidate == 0) continue;
        if (markToIsolationKey_.find(candidate) == markToIsolationKey_.end()) {
            nextMark_ = (candidate + 1) & MARK_MASK;
            if (nextMark_ == 0) nextMark_ = 0x01;
            return candidate;
        }
    }
    return 0;
}

int32_t UidRuleGenerator::AllocateMarkForUidRange(
    const std::string& bundleName,
    uint32_t groupId,
    uint32_t uidStart,
    uint32_t uidEnd,
    uint32_t& markValue)
{
    if (uidStart > uidEnd) {
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    auto ranges = ClipToRealUidRanges(uidStart, uidEnd);
    if (ranges.empty()) {
        NETMGR_EXT_LOG_E("AllocateMarkForUidRange no real uid in range %{public}u-%{public}u", uidStart, uidEnd);
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    std::string isolationKey = GenerateIsolationKey(bundleName, groupId);
    auto it = uidRuleContexts_.find(isolationKey);
    if (it != uidRuleContexts_.end()) {
        markValue = it->second->ctMarkValue;
        return UpdateUidRangeIfNeeded(isolationKey, uidStart, uidEnd);
    }

    for (const auto& [start, end] : ranges) {
        if (IsUidRangeOverlap(start, end)) {
            return TRAFFICFILTER_ERROR_INVALID_PARAM;
        }
    }

    uint32_t newMark = AllocateNextMark();
    if (newMark == 0) {
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    auto ctx = std::make_shared<UidRuleContext>();
    ctx->bundleName = bundleName;
    ctx->groupId = groupId;
    ctx->ctMarkValue = newMark;
    uidRuleContexts_[isolationKey] = ctx;
    markToIsolationKey_[newMark] = isolationKey;

    int32_t ret = SyncUidIntervals(isolationKey, uidStart, uidEnd);
    if (ret != TRAFFICFILTER_OK) {
        uidRuleContexts_.erase(isolationKey);
        markToIsolationKey_.erase(newMark);
        return ret;
    }

    markValue = newMark;
    return TRAFFICFILTER_OK;
}

int32_t UidRuleGenerator::UpdateUidRangeIfNeeded(
    const std::string& isolationKey,
    uint32_t uidStart,
    uint32_t uidEnd)
{
    if (uidStart > uidEnd) {
        NETMGR_EXT_LOG_E("UpdateUidRangeIfNeeded invalid range %{public}u-%{public}u", uidStart, uidEnd);
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    auto ctxIt = uidRuleContexts_.find(isolationKey);
    if (ctxIt == uidRuleContexts_.end()) {
        return TRAFFICFILTER_OK;
    }

    auto& ctx = ctxIt->second;
    auto ranges = ClipToRealUidRanges(uidStart, uidEnd);
    if (ranges.empty()) {
        NETMGR_EXT_LOG_E("UpdateUidRangeIfNeeded no real uid in range %{public}u-%{public}u", uidStart, uidEnd);
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    for (const auto& [start, end] : ranges) {
        if (IsUidRangeOverlap(start, end, isolationKey)) {
            NETMGR_EXT_LOG_E("UpdateUidRangeIfNeeded range overlaps with other context");
            return TRAFFICFILTER_ERROR_INVALID_PARAM;
        }
    }

    return SyncUidIntervals(isolationKey, uidStart, uidEnd);
}

void UidRuleGenerator::RemoveUidFromMapping(const std::string& isolationKey)
{
    ClearIntervalsForKey(isolationKey);
}

int32_t UidRuleGenerator::ReleaseMarkForUidRange(
    const std::string& bundleName,
    uint32_t groupId)
{
    std::string isolationKey = GenerateIsolationKey(bundleName, groupId);
    auto ctxIt = uidRuleContexts_.find(isolationKey);
    if (ctxIt == uidRuleContexts_.end()) {
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    uint32_t markValue = ctxIt->second->ctMarkValue;

    ClearIntervalsForKey(isolationKey);
    markToIsolationKey_.erase(markValue);
    uidRuleContexts_.erase(ctxIt);
    return TRAFFICFILTER_OK;
}

int32_t UidRuleGenerator::GetMarkByUid(uint32_t uid, uint32_t& markValue)
{
    auto clipped = ClipToRealUidRanges(uid, uid);
    if (clipped.empty()) {
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    auto it = uidIntervals_.upper_bound(uid);
    if (it == uidIntervals_.begin()) {
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    --it;
    if (uid > it->second.end) {
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    auto ctxIt = uidRuleContexts_.find(it->second.isolationKey);
    if (ctxIt == uidRuleContexts_.end()) {
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    markValue = ctxIt->second->ctMarkValue;
    return TRAFFICFILTER_OK;
}

int32_t UidRuleGenerator::GetMarkByUidRange(
    uint32_t uidStart,
    uint32_t uidEnd,
    uint32_t& markValue)
{
    auto ranges = ClipToRealUidRanges(uidStart, uidEnd);
    if (ranges.empty()) {
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    std::string isolationKey;
    for (const auto& [start, end] : ranges) {
        auto it = uidIntervals_.upper_bound(start);
        if (it == uidIntervals_.begin()) {
            return TRAFFICFILTER_ERROR_INVALID_PARAM;
        }
        --it;
        if (start < it->first || end > it->second.end) {
            return TRAFFICFILTER_ERROR_INVALID_PARAM;
        }
        if (isolationKey.empty()) {
            isolationKey = it->second.isolationKey;
        } else if (isolationKey != it->second.isolationKey) {
            return TRAFFICFILTER_ERROR_INVALID_PARAM;
        }
    }

    auto ctxIt = uidRuleContexts_.find(isolationKey);
    if (ctxIt == uidRuleContexts_.end()) {
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    markValue = ctxIt->second->ctMarkValue;
    return TRAFFICFILTER_OK;
}

int32_t UidRuleGenerator::GenerateDeleteMangleChainCommands(const QueueInfo& info)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::string chainName = info.chainNameOut;
    std::string isolationKey = GenerateIsolationKey(info.bundleName, info.groupId);
    auto ctxIt = uidRuleContexts_.find(isolationKey);
    if (ctxIt == uidRuleContexts_.end() || !ctxIt->second->hasOutputRule) {
        return TRAFFICFILTER_OK;
    }
    std::ostringstream deleteJumpCmd;
    deleteJumpCmd << "-t mangle -D OUTPUT -j " << chainName;
    int32_t ret = ExecuteCmd(deleteJumpCmd.str());
    if (ret != TRAFFICFILTER_OK) {
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    std::ostringstream flushCmd;
    flushCmd << "-t mangle -F " << chainName;
    ret = ExecuteCmd(flushCmd.str());
    if (ret != TRAFFICFILTER_OK) {
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    std::ostringstream deleteChainCmd;
    deleteChainCmd << "-t mangle -X " << chainName;
    ret = ExecuteCmd(deleteChainCmd.str());
    if (ret != TRAFFICFILTER_OK) {
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    ctxIt->second->hasOutputRule = false;
    return TRAFFICFILTER_OK;
}

int32_t UidRuleGenerator::CreateMangleChain(const std::string& chainName)
{
    std::ostringstream createCmd;
    createCmd << "-t mangle -N " << chainName;
    return ExecuteCmd(createCmd.str());
}

int32_t UidRuleGenerator::InsertMangleChainJump(const std::string& chainName)
{
    std::ostringstream insertCmd;
    insertCmd << "-t mangle -I OUTPUT 1 -j " << chainName;
    return ExecuteCmd(insertCmd.str());
}

int32_t UidRuleGenerator::AddMangleMarkRule(
    const std::string& chainName,
    uint32_t uidStart,
    uint32_t uidEnd,
    const std::string& markStr)
{
    std::ostringstream rule;
    if (uidStart == uidEnd) {
        rule << "-t mangle -A " << chainName
             << " -m owner --uid-owner "
             << uidStart << " -j MARK --set-mark "
             << markStr;
    } else {
        rule << "-t mangle -A " << chainName
             << " -m owner --uid-owner "
             << uidStart << "-"
             << uidEnd << " -j MARK --set-mark " << markStr;
    }
    return ExecuteCmd(rule.str());
}

int32_t UidRuleGenerator::AddMangleConnmarkRule(
    const std::string& chainName,
    const std::string& markStr)
{
    std::ostringstream rule;
    rule << "-t mangle -A " << chainName << " -m mark --mark " << markStr << " -j CONNMARK --save-mark";
    return ExecuteCmd(rule.str());
}

void UidRuleGenerator::RollbackMangleChainCreation(const std::string& chainName, int32_t stage)
{
    if (stage >= static_cast<int32_t>(MangleChainStage::STAGE_MARK_RULE_ADDED)) {
        std::ostringstream flushCmd;
        flushCmd << "-t mangle -F " << chainName;
        ExecuteCmd(flushCmd.str());
    }
    if (stage >= static_cast<int32_t>(MangleChainStage::STAGE_JUMP_INSERTED)) {
        std::ostringstream deleteJumpCmd;
        deleteJumpCmd << "-t mangle -D OUTPUT -j " << chainName;
        ExecuteCmd(deleteJumpCmd.str());
    }
    if (stage >= static_cast<int32_t>(MangleChainStage::STAGE_CHAIN_CREATED)) {
        std::ostringstream deleteChainCmd;
        deleteChainCmd << "-t mangle -X " << chainName;
        ExecuteCmd(deleteChainCmd.str());
    }
}

int32_t UidRuleGenerator::GenerateCreateMangleRulesCommands(
    const QueueInfo& info,
    uint32_t uidStart,
    uint32_t uidEnd,
    uint32_t markValue)
{
    auto ranges = ClipToRealUidRanges(uidStart, uidEnd);
    if (ranges.empty()) {
        NETMGR_EXT_LOG_E("GenerateCreateMangleRulesCommands no real uid in range %{public}u-%{public}u",
            uidStart, uidEnd);
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    const std::string chainName = info.chainNameOut;
    std::ostringstream markStr;
    markStr << "0x" << std::hex << markValue;

    if (CreateMangleChain(chainName) != TRAFFICFILTER_OK) {
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    if (InsertMangleChainJump(chainName) != TRAFFICFILTER_OK) {
        RollbackMangleChainCreation(chainName, static_cast<int32_t>(MangleChainStage::STAGE_CHAIN_CREATED));
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    for (const auto& [start, end] : ranges) {
        if (AddMangleMarkRule(chainName, start, end, markStr.str()) != TRAFFICFILTER_OK) {
            RollbackMangleChainCreation(chainName, static_cast<int32_t>(MangleChainStage::STAGE_JUMP_INSERTED));
            return TRAFFICFILTER_ERROR_INVALID_PARAM;
        }
    }

    if (AddMangleConnmarkRule(chainName, markStr.str()) != TRAFFICFILTER_OK) {
        RollbackMangleChainCreation(chainName, static_cast<int32_t>(MangleChainStage::STAGE_MARK_RULE_ADDED));
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    return TRAFFICFILTER_OK;
}

std::string UidRuleGenerator::BuildInputCtmarkRule(
    const std::string& chainName,
    int32_t queueNum,
    uint32_t markValue)
{
    std::ostringstream rule;
    rule << "-t filter -A " << chainName
         << " " << GenerateCTMarkMatchParam(markValue);
    rule << " -j NFQUEUE --queue-num " << queueNum;
    return rule.str();
}

int32_t UidRuleGenerator::CreateOrUpdateContext(
    const QueueInfo& info,
    uint32_t uidStart,
    uint32_t uidEnd,
    int32_t queueNum,
    std::shared_ptr<UidRuleContext>& ctx)
{
    std::string isolationKey = GenerateIsolationKey(info.bundleName, info.groupId);
    auto it = uidRuleContexts_.find(isolationKey);
    if (it != uidRuleContexts_.end()) {
        ctx = it->second;
        ctx->uidStart = uidStart;
        ctx->uidEnd = uidEnd;
        ctx->queueNum = queueNum;
        return TRAFFICFILTER_OK;
    }
    ctx = std::make_shared<UidRuleContext>();
    ctx->bundleName = info.bundleName;
    ctx->groupId = info.groupId;
    ctx->uidStart = uidStart;
    ctx->uidEnd = uidEnd;
    ctx->queueNum = queueNum;
    ctx->filterChainName = info.chainNameOut;
    ctx->mangleChainName = info.chainNameOut;
    uidRuleContexts_[isolationKey] = ctx;
    return TRAFFICFILTER_OK;
}

int32_t UidRuleGenerator::HandleOutputUidRule(
    const QueueInfo& info,
    const sptr<TrafficFilterPacketRule>& rule,
    int32_t queueNum)
{
    std::string isolationKey = GenerateIsolationKey(info.bundleName, info.groupId);
    uint32_t markValue = 0;
    int32_t ret = AllocateMarkForUidRange(
        info.bundleName, info.groupId, rule->uidStart_, rule->uidEnd_, markValue);
    if (ret != TRAFFICFILTER_OK) {
        return ret;
    }
    std::shared_ptr<UidRuleContext> ctx = nullptr;
    ret = CreateOrUpdateContext(info, rule->uidStart_, rule->uidEnd_, queueNum, ctx);
    if (ret != TRAFFICFILTER_OK) {
        ReleaseMarkForUidRange(info.bundleName, info.groupId);
        return ret;
    }
    ctx->ctMarkValue = markValue;
    ctx->hasOutputRule = true;
    ret = GenerateCreateMangleRulesCommands(
        info, rule->uidStart_, rule->uidEnd_, markValue);
    if (ret != TRAFFICFILTER_OK) {
        ReleaseMarkForUidRange(info.bundleName, info.groupId);
        uidRuleContexts_.erase(isolationKey);
        return ret;
    }
    return TRAFFICFILTER_OK;
}

int32_t UidRuleGenerator::RestoreInputUidRule(int32_t queueNum, TrafficFilterIPFamily family)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    QueueInfo info = NetTrafficFilterNFQueueCore::GetInstance().GetQueueInfo(queueNum);
    std::string isolationKey = GenerateIsolationKey(info.bundleName, info.groupId);
    auto it = uidRuleContexts_.find(isolationKey);
    if (it == uidRuleContexts_.end()) {
        return TRAFFICFILTER_OK;
    }
    auto ctx = it->second;
    if (!ctx->hasInputRule) {
        return TRAFFICFILTER_OK;
    }
    std::string filterCmd = BuildInputCtmarkRule(info.chainNameIn, queueNum, ctx->ctMarkValue);
    int32_t ret = NetTrafficFilterIptablesCommandBuilder::ExecuteIptablesCommand(filterCmd, family);
    if (ret != TRAFFICFILTER_OK) {
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    return TRAFFICFILTER_OK;
}

int32_t UidRuleGenerator::HandleInputUidRule(
    const QueueInfo& info,
    const sptr<TrafficFilterPacketRule>& rule,
    int32_t queueNum)
{
    std::string isolationKey = GenerateIsolationKey(info.bundleName, info.groupId);
    uint32_t markValue = 0;
    int32_t ret = GetMarkByUidRange(rule->uidStart_, rule->uidEnd_, markValue);
    if (ret != TRAFFICFILTER_OK && rule->uidStart_ == rule->uidEnd_) {
        ret = GetMarkByUid(rule->uidStart_, markValue);
    }
    if (ret != TRAFFICFILTER_OK) {
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    std::shared_ptr<UidRuleContext> ctx = nullptr;
    bool isNewContext = false;
    auto it = uidRuleContexts_.find(isolationKey);
    if (it != uidRuleContexts_.end()) {
        ret = UpdateUidRangeIfNeeded(isolationKey, rule->uidStart_, rule->uidEnd_);
        if (ret != TRAFFICFILTER_OK) {
            return ret;
        }
        ctx = it->second;
        ctx->queueNum = queueNum;
    } else {
        ret = CreateOrUpdateContext(info, rule->uidStart_, rule->uidEnd_, queueNum, ctx);
        if (ret != TRAFFICFILTER_OK) {
            return ret;
        }
        ctx->ctMarkValue = markValue;
        ret = SyncUidIntervals(isolationKey, rule->uidStart_, rule->uidEnd_);
        if (ret != TRAFFICFILTER_OK) {
            uidRuleContexts_.erase(isolationKey);
            return ret;
        }
        isNewContext = true;
    }
    std::string filterCmd = BuildInputCtmarkRule(info.chainNameIn, queueNum, markValue);
    ret = ExecuteCmd(filterCmd);
    if (ret != TRAFFICFILTER_OK) {
        if (isNewContext) {
            uidRuleContexts_.erase(isolationKey);
        }
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    ctx->hasInputRule = true;
    return TRAFFICFILTER_OK;
}

int32_t UidRuleGenerator::HandleAddUidRule(
    const QueueInfo& info,
    const sptr<TrafficFilterPacketRule>& rule,
    int32_t queueNum)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!HasUidCondition(rule)) {
        return TRAFFICFILTER_OK;
    }
    if (rule->uidStart_ > rule->uidEnd_) {
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    int32_t ret = HandleOutputUidRule(info, rule, queueNum);
    if (ret != TRAFFICFILTER_OK) {
        return ret;
    }
    ret = HandleInputUidRule(info, rule, queueNum);
    if (ret != TRAFFICFILTER_OK) {
        GenerateDeleteMangleChainCommands(info);
        ReleaseMarkForUidRange(info.bundleName, info.groupId);
        uidRuleContexts_.erase(GenerateIsolationKey(info.bundleName, info.groupId));
        return ret;
    }
    return TRAFFICFILTER_OK;
}

int32_t UidRuleGenerator::HandleClearUidRules(const QueueInfo& info)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::string isolationKey = GenerateIsolationKey(info.bundleName, info.groupId);
    auto it = uidRuleContexts_.find(isolationKey);
    if (it == uidRuleContexts_.end()) {
        return TRAFFICFILTER_OK;
    }

    auto ctx = it->second;
    if (ctx->hasOutputRule) {
        GenerateDeleteMangleChainCommands(info);
        ReleaseMarkForUidRange(info.bundleName, info.groupId);
    }
    uidRuleContexts_.erase(isolationKey);
    return TRAFFICFILTER_OK;
}

int32_t UidRuleGenerator::ExecuteCmd(std::string cmd)
{
    int32_t retV4 = NetTrafficFilterIptablesCommandBuilder::ExecuteIptablesCommand(cmd,
        TrafficFilterIPFamily::IP_FAMILY_V4);
    if (retV4 != TRAFFICFILTER_OK) {
    }
    int32_t retV6 = NetTrafficFilterIptablesCommandBuilder::ExecuteIptablesCommand(cmd,
        TrafficFilterIPFamily::IP_FAMILY_V6);
    if (retV6 != TRAFFICFILTER_OK) {
    }
    if (retV4 != TRAFFICFILTER_OK || retV6 != TRAFFICFILTER_OK) {
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    return TRAFFICFILTER_OK;
}
} // namespace NetManagerStandard
} // namespace OHOS