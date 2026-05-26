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

#include "nettrafficfilter_redirector_context.h"
#include "netmgr_ext_log_wrapper.h"
#include <algorithm>

namespace OHOS {
namespace NetManagerStandard {

namespace {
    const int32_t NETFIREWALL_SUCCESS = 0;
}

bool NetTrafficFilterRedirectorContext::HasRules() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return !rules_.empty();
}

std::set<TrafficFilterHookPoint> NetTrafficFilterRedirectorContext::GetUsedHookPoints() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::set<TrafficFilterHookPoint> usedHookPoints;
    for (const auto& rule : rules_) {
        usedHookPoints.insert(static_cast<TrafficFilterHookPoint>(rule.hookPoint_));
    }
    return usedHookPoints;
}

NetTrafficFilterRedirectorContext::NetTrafficFilterRedirectorContext(
    const std::string& redirectorId,
    const std::string& bundleName,
    uint32_t groupId,
    uint32_t priority,
    const NetTrafficFilterConfig* config)
    : redirectorId_(redirectorId),
      bundleName_(bundleName),
      groupId_(groupId),
      priority_(priority),
      config_(*config),
      isPaused_(false),
      callingUid_(-1),
      callingPid_(-1)
{
    NETMGR_EXT_LOG_I("NetTrafficFilterRedirectorContext created: "
        "redirectorId=%{public}s, bundleName=%{public}s, groupId=%{public}u",
        redirectorId_.c_str(), bundleName_.c_str(), groupId_);
}

NetTrafficFilterRedirectorContext::~NetTrafficFilterRedirectorContext() {}

int32_t NetTrafficFilterRedirectorContext::AddRuleWithPriority(const TrafficFilterRedirectRule& rule)
{
    std::lock_guard<std::mutex> lock(mutex_);

    rules_.push_back(rule);

    std::sort(rules_.begin(), rules_.end(),
        [](const TrafficFilterRedirectRule& a, const TrafficFilterRedirectRule& b) {
            return a.priority_ < b.priority_;
        });

    NETMGR_EXT_LOG_I("Rule added to redirector %{public}s with priority sorting: "
        "priority=%{public}u, hookPoint=%{public}d", redirectorId_.c_str(), rule.priority_, rule.hookPoint_);
    return NETFIREWALL_SUCCESS;
}

int32_t NetTrafficFilterRedirectorContext::ClearRules()
{
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t count = rules_.size();
    rules_.clear();
    NETMGR_EXT_LOG_I("Cleared %{public}d rules from redirector %{public}s", count, redirectorId_.c_str());
    return NETFIREWALL_SUCCESS;
}

std::vector<TrafficFilterRedirectRule> NetTrafficFilterRedirectorContext::GetRules() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return rules_;
}

std::vector<TrafficFilterRedirectRule> NetTrafficFilterRedirectorContext::GetSortedRules() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<TrafficFilterRedirectRule> sortedRules = rules_;

    std::sort(sortedRules.begin(), sortedRules.end(),
        [](const TrafficFilterRedirectRule& a, const TrafficFilterRedirectRule& b) {
            return a.priority_ < b.priority_;
        });
    return sortedRules;
}

int32_t NetTrafficFilterRedirectorContext::RestoreRules(const std::vector<TrafficFilterRedirectRule>& rules)
{
    std::lock_guard<std::mutex> lock(mutex_);
    rules_ = rules;
    std::sort(rules_.begin(), rules_.end(),
        [](const TrafficFilterRedirectRule& a, const TrafficFilterRedirectRule& b) {
            return a.priority_ < b.priority_;
        });
    NETMGR_EXT_LOG_I("Restored %{public}zu rules for redirector %{public}s",
        rules_.size(), redirectorId_.c_str());
    return NETFIREWALL_SUCCESS;
}

} // namespace NetManagerStandard
} // namespace OHOS
