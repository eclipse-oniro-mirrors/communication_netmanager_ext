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

#ifndef NETTRAFFICFILTER_PACKETRULE_MANAGER_H
#define NETTRAFFICFILTER_PACKETRULE_MANAGER_H

#include <string>
#include <mutex>
#include <map>
#include <set>
#include "netfirewall_common.h"
#include "nettrafficfilter_nfqueue_core.h"

namespace OHOS {
namespace NetManagerStandard {
struct FilterRuleCtx {
    uint32_t priority;
    std::string chainNameIn;
    std::string chainNameOut;
    std::string chainNameFwd;
};

struct ResumeEntry {
    int32_t queueNum;
    std::string chainName;
    int32_t hookPoint;
    int32_t priority;
    bool needV6;
};

struct HookPointRules {
    std::vector<TrafficFilterPacketRule> input;
    std::vector<TrafficFilterPacketRule> output;
    std::vector<TrafficFilterPacketRule> forward;
};

class NetTrafficFilterPacketRuleManager {
public:
    static NetTrafficFilterPacketRuleManager& GetInstance();

    int32_t AddPacketRule(const std::string& controllerId, const sptr<TrafficFilterPacketRule>& rule);
    int32_t ClearPacketRule(const QueueInfo& info);

    int32_t PauseAllRules();
    int32_t ResumeAllRules();
    bool ParseAndValidateControllerId(const std::string& controllerId, int32_t& queueNum);

private:
    NetTrafficFilterPacketRuleManager();
    ~NetTrafficFilterPacketRuleManager();
    NetTrafficFilterPacketRuleManager(const NetTrafficFilterPacketRuleManager&) = delete;
    NetTrafficFilterPacketRuleManager& operator=(const NetTrafficFilterPacketRuleManager&) = delete;

    uint32_t GetQueueRuleCountLocked(uint32_t queueNum) const;

    int32_t ExecuteRulesForIpFamily(const std::vector<TrafficFilterPacketRule>& rules, const std::string& chainName,
        int32_t queueNum, TrafficFilterIPFamily family);
    bool ValidateRuleParam(const sptr<TrafficFilterPacketRule>& rule);
    void DeleteJumpRulesForHookPoints(const std::set<int32_t>& hookPoints, const std::string& chainNameIn,
        const std::string& chainNameOut, const std::string& chainNameFwd);
    void FlushChainForIpFamilies(const std::string& chainName);
    int32_t ApplyRulesForHookPoint(int32_t queueNum, int32_t hookPoint, const std::string& chainName,
        TrafficFilterIPFamily family);
    int32_t ApplyRulesForHookPointBothFamilies(int32_t queueNum, int32_t hookPoint,
        const std::string& chainName, bool needV6);
    void CleanPhysicalRules(const QueueInfo& info, const std::set<int32_t>& hookPoints);
    void DeleteChainForIpFamilies(const std::string& chainName);
    std::vector<ResumeEntry> CollectResumeEntries();
    int32_t ResumeJumpRules(const std::vector<ResumeEntry>& entries);

    std::mutex mutex_;
    std::map<uint32_t, HookPointRules> queueNumToRules_;
    std::map<int32_t, FilterRuleCtx> queueNumToRuleCtx_;
};

} // namespace NetManagerStandard
} // namespace OHOS

#endif // NETTRAFFICFILTER_PACKETRULE_MANAGER_H
