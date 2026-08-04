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
#ifndef NET_FIREWALL_UID_RULE_GENERATOR_H
#define NET_FIREWALL_UID_RULE_GENERATOR_H

#include <cstdint>
#include <string>
#include <map>
#include <set>
#include <mutex>
#include <memory>
#include <vector>
#include <utility>
#include <refbase.h>
#include "netfirewall_common.h"
#include "nettrafficfilter_nfqueue_core.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr uint32_t MAX_MARK = 0xFF;
constexpr uint32_t MARK_MASK = 0xFF;

enum class MangleChainStage : int32_t {
    STAGE_CHAIN_CREATED = 1,
    STAGE_JUMP_INSERTED = 2,
    STAGE_MARK_RULE_ADDED = 3
};

struct UidRuleContext {
    std::string bundleName;
    uint32_t groupId;
    uint32_t uidStart;
    uint32_t uidEnd;
    uint32_t ctMarkValue;
    bool hasOutputRule = false;
    bool hasInputRule = false;
    std::string mangleChainName;
    std::string filterChainName;
    int32_t queueNum;
};

class UidRuleGenerator {
public:
    static UidRuleGenerator& GetInstance();

    int32_t HandleAddUidRule(
        const QueueInfo& info,
        const sptr<TrafficFilterPacketRule>& rule,
        int32_t queueNum);

    int32_t RestoreInputUidRule(int32_t queueNum, TrafficFilterIPFamily family);

    int32_t HandleClearUidRules(const QueueInfo& info);
private:
    UidRuleGenerator() = default;
    ~UidRuleGenerator() = default;
    UidRuleGenerator(const UidRuleGenerator&) = delete;
    UidRuleGenerator& operator=(const UidRuleGenerator&) = delete;
    bool HasUidCondition(const sptr<TrafficFilterPacketRule>& rule);

    std::string GenerateCTMarkMatchParam(uint32_t markValue);

    std::string BuildInputCtmarkRule(
        const std::string& chainName,
        int32_t queueNum,
        uint32_t markValue);

    std::string GenerateIsolationKey(const std::string& bundleName, uint32_t groupId);

    bool IsUidRangeOverlap(uint32_t uidStart, uint32_t uidEnd,
                           const std::string& excludeIsolationKey = "");

    uint32_t AllocateNextMark();

    void RemoveUidFromMapping(const std::string& isolationKey,
                              uint32_t uidStart,
                              uint32_t uidEnd);

    void UpdateUidRangeIfNeeded(const std::string& isolationKey,
                                uint32_t uidStart,
                                uint32_t uidEnd);

    int32_t AllocateMarkForUidRange(
        const std::string& bundleName,
        uint32_t groupId,
        uint32_t uidStart,
        uint32_t uidEnd,
        uint32_t& markValue);

    int32_t ReleaseMarkForUidRange(
        const std::string& bundleName,
        uint32_t groupId);

    int32_t GetMarkByUid(uint32_t uid, uint32_t& markValue);

    int32_t GetMarkByUidRange(
        uint32_t uidStart,
        uint32_t uidEnd,
        uint32_t& markValue);

    int32_t GenerateDeleteMangleChainCommands(const QueueInfo& info);

    int32_t GenerateCreateMangleRulesCommands(
        const QueueInfo& info,
        uint32_t uidStart,
        uint32_t uidEnd,
        uint32_t markValue);

    int32_t CreateMangleChain(const std::string& chainName);

    int32_t InsertMangleChainJump(const std::string& chainName);

    int32_t AddMangleMarkRule(
        const std::string& chainName,
        uint32_t uidStart,
        uint32_t uidEnd,
        const std::string& markStr);

    int32_t AddMangleConnmarkRule(
        const std::string& chainName,
        const std::string& markStr);

    void RollbackMangleChainCreation(const std::string& chainName, int32_t stage);

    int32_t HandleOutputUidRule(
        const QueueInfo& info,
        const sptr<TrafficFilterPacketRule>& rule,
        int32_t queueNum);

    int32_t HandleInputUidRule(
        const QueueInfo& info,
        const sptr<TrafficFilterPacketRule>& rule,
        int32_t queueNum);

    std::shared_ptr<UidRuleContext> CreateOrUpdateContext(
        const QueueInfo& info,
        uint32_t uidStart,
        uint32_t uidEnd,
        int32_t queueNum);

    int32_t ExecuteCmd(std::string cmd);

    std::recursive_mutex mutex_;
    std::map<std::string, std::shared_ptr<UidRuleContext>> uidRuleContexts_;
    std::map<uint32_t, std::set<std::string>> uidToIsolationKeys_;
    std::map<uint32_t, std::string> markToIsolationKey_;
    uint32_t nextMark_ = 0x01;
};

} // namespace NetManagerStandard
} // namespace OHOS

#endif // NET_FIREWALL_UID_RULE_GENERATOR_H