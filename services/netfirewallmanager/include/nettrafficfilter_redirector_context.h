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

#ifndef NETTRAFFICFILTER_REDIRECTOR_CONTEXT_H
#define NETTRAFFICFILTER_REDIRECTOR_CONTEXT_H

#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>
#include "netfirewall_common.h"

namespace OHOS {
namespace NetManagerStandard {

class NetTrafficFilterRedirectorContext {
public:
    NetTrafficFilterRedirectorContext(const std::string& redirectorId, const std::string& bundleName,
        uint32_t groupId, uint32_t priority);
    ~NetTrafficFilterRedirectorContext();

    int32_t AddRuleWithPriority(const TrafficFilterRedirectRule& rule);
    int32_t ClearRules();
    std::vector<TrafficFilterRedirectRule> GetRules() const;
    std::vector<TrafficFilterRedirectRule> GetSortedRules() const;
    bool HasRules() const;
    std::set<TrafficFilterHookPoint> GetUsedHookPoints() const;
    int32_t RestoreRules(const std::vector<TrafficFilterRedirectRule>& rules);

    std::string GetRedirectorId() const { return redirectorId_; }
    std::string GetBundleName() const { return bundleName_; }
    uint32_t GetGroupId() const { return groupId_; }
    uint32_t GetPriority() const { return priority_; }
    bool IsPaused() const { return isPaused_; }
    void SetPaused(bool paused) { isPaused_ = paused; }
    int32_t GetCallingUid() const { return callingUid_; }
    int32_t GetCallingPid() const { return callingPid_; }
    void SetCallingInfo(int32_t uid, int32_t pid)
    {
        callingUid_ = uid;
        callingPid_ = pid;
    }

private:
    std::string redirectorId_;
    std::string bundleName_;
    uint32_t groupId_;
    uint32_t priority_;
    std::vector<TrafficFilterRedirectRule> rules_;
    mutable std::mutex mutex_;
    bool isPaused_;
    int32_t callingUid_;
    int32_t callingPid_;
};

} // namespace NetManagerStandard
} // namespace OHOS

#endif // NETTRAFFICFILTER_REDIRECTOR_CONTEXT_H
