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
#include <system_error>
#include "netfirewall_uid_rule_generator.h"
#include "nettrafficfilter_iptables_command_builder.h"
#include "net_manager_constants.h"

namespace OHOS {
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
    return rule->uidStart_ != static_cast<uint32_t>(-1) || rule->uidEnd_ != static_cast<uint32_t>(-1);
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
    for (const auto& [isolationKey, ctx] : uidRuleContexts_) {
        if (isolationKey == excludeIsolationKey) {
            continue;
        }
        if (uidStart <= ctx->uidEnd && uidEnd >= ctx->uidStart) {
            return true;
        }
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

    std::string isolationKey = GenerateIsolationKey(bundleName, groupId);
    auto it = uidRuleContexts_.find(isolationKey);
    if (it != uidRuleContexts_.end()) {
        markValue = it->second->ctMarkValue;
        UpdateUidRangeIfNeeded(isolationKey, uidStart, uidEnd);
        return TRAFFICFILTER_OK;
    }
    if (IsUidRangeOverlap(uidStart, uidEnd)) {
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    uint32_t newMark = AllocateNextMark();
    if (newMark == 0) {
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    auto ctx = std::make_shared<UidRuleContext>();
    ctx->bundleName = bundleName;
    ctx->groupId = groupId;
    ctx->uidStart = uidStart;
    ctx->uidEnd = uidEnd;
    ctx->ctMarkValue = newMark;
    uidRuleContexts_[isolationKey] = ctx;
    markToIsolationKey_[newMark] = isolationKey;
    for (uint32_t uid = uidStart; uid <= uidEnd; uid++) {
        uidToIsolationKeys_[uid].insert(isolationKey);
    }

    markValue = newMark;
    return TRAFFICFILTER_OK;
}

void UidRuleGenerator::UpdateUidRangeIfNeeded(
    const std::string& isolationKey,
    uint32_t uidStart,
    uint32_t uidEnd)
{
    auto ctxIt = uidRuleContexts_.find(isolationKey);
    if (ctxIt == uidRuleContexts_.end()) {
        return;
    }

    auto& ctx = ctxIt->second;
    if (ctx->uidStart == uidStart && ctx->uidEnd == uidEnd) {
        return;
    }

    RemoveUidFromMapping(isolationKey, ctx->uidStart, ctx->uidEnd);
    ctx->uidStart = uidStart;
    ctx->uidEnd = uidEnd;
    for (uint32_t uid = uidStart; uid <= uidEnd; uid++) {
        uidToIsolationKeys_[uid].insert(isolationKey);
    }
}

void UidRuleGenerator::RemoveUidFromMapping(
    const std::string& isolationKey,
    uint32_t uidStart,
    uint32_t uidEnd)
{
    for (uint32_t uid = uidStart; uid <= uidEnd; uid++) {
        auto setIt = uidToIsolationKeys_.find(uid);
        if (setIt == uidToIsolationKeys_.end()) {
            continue;
        }
        setIt->second.erase(isolationKey);
        if (setIt->second.empty()) {
            uidToIsolationKeys_.erase(setIt);
        }
    }
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
    auto& ctx = ctxIt->second;
    uint32_t markValue = ctx->ctMarkValue;
    for (uint32_t uid = ctx->uidStart; uid <= ctx->uidEnd; uid++) {
        auto setIt = uidToIsolationKeys_.find(uid);
        if (setIt == uidToIsolationKeys_.end()) {
            continue;
        }
        setIt->second.erase(isolationKey);
        if (setIt->second.empty()) {
            uidToIsolationKeys_.erase(setIt);
        }
    }

    markToIsolationKey_.erase(markValue);
    uidRuleContexts_.erase(ctxIt);
    return TRAFFICFILTER_OK;
}

int32_t UidRuleGenerator::GetMarkByUid(uint32_t uid, uint32_t& markValue)
{
    auto it = uidToIsolationKeys_.find(uid);
    if (it == uidToIsolationKeys_.end() || it->second.empty()) {
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    const std::string& isolationKey = *it->second.begin();
    auto ctxIt = uidRuleContexts_.find(isolationKey);
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
    for (const auto& [isolationKey, ctx] : uidRuleContexts_) {
        if (uidStart == ctx->uidStart && uidEnd == ctx->uidEnd) {
            markValue = ctx->ctMarkValue;
            return TRAFFICFILTER_OK;
        }
    }
    return TRAFFICFILTER_ERROR_INVALID_PARAM;
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

    if (AddMangleMarkRule(chainName, uidStart, uidEnd, markStr.str()) != TRAFFICFILTER_OK) {
        RollbackMangleChainCreation(chainName, static_cast<int32_t>(MangleChainStage::STAGE_JUMP_INSERTED));
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
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

std::shared_ptr<UidRuleContext> UidRuleGenerator::CreateOrUpdateContext(
    const QueueInfo& info,
    uint32_t uidStart,
    uint32_t uidEnd,
    int32_t queueNum)
{
    std::string isolationKey = GenerateIsolationKey(info.bundleName, info.groupId);
    auto it = uidRuleContexts_.find(isolationKey);
    if (it != uidRuleContexts_.end()) {
        auto ctx = it->second;
        ctx->uidStart = uidStart;
        ctx->uidEnd = uidEnd;
        ctx->queueNum = queueNum;
        return ctx;
    }
    auto ctx = std::make_shared<UidRuleContext>();
    ctx->bundleName = info.bundleName;
    ctx->groupId = info.groupId;
    ctx->uidStart = uidStart;
    ctx->uidEnd = uidEnd;
    ctx->queueNum = queueNum;
    ctx->filterChainName = info.chainNameOut;
    ctx->mangleChainName = info.chainNameOut;
    uidRuleContexts_[isolationKey] = ctx;
    return ctx;
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
    auto ctx = CreateOrUpdateContext(info, rule->uidStart_, rule->uidEnd_, queueNum);
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
    auto it = uidRuleContexts_.find(isolationKey);
    std::shared_ptr<UidRuleContext> ctx = nullptr;
    bool isNewContext = false;
    if (it != uidRuleContexts_.end()) {
        ctx = it->second;
        ctx->uidStart = rule->uidStart_;
        ctx->uidEnd = rule->uidEnd_;
        ctx->queueNum = queueNum;
    } else {
        ctx = CreateOrUpdateContext(info, rule->uidStart_, rule->uidEnd_, queueNum);
        ctx->ctMarkValue = markValue;
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