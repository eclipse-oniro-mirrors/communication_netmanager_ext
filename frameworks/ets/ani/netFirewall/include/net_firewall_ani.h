/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef NET_FIREWALL_ANI_H
#define NET_FIREWALL_ANI_H

#include <cstdint>
#include <memory>

#include "cxx.h"
#include "netmgr_ext_log_wrapper.h"
#include "netfirewall_parcel.h"

namespace OHOS {
namespace NetManagerAni {

struct NetFirewallPolicyAni;
struct NetFirewallRuleAni;
struct RequestParamAni;
struct FirewallRulePageAni;
struct InterceptedRecordPageAni;

int32_t SetNetFirewallPolicy(int32_t userId, const NetFirewallPolicyAni &policyAni);
int32_t GetNetFirewallPolicy(int32_t userId, NetFirewallPolicyAni &policyAni);
int32_t AddNetFirewallRule(const NetFirewallRuleAni &ruleAni, int32_t &ruleId);
int32_t UpdateNetFirewallRule(const NetFirewallRuleAni &ruleAni);
int32_t RemoveNetFirewallRule(int32_t userId, int32_t ruleId);
int32_t GetNetFirewallRules(int32_t userId, const RequestParamAni &requestParamAni, FirewallRulePageAni &pageInfoAni);
int32_t GetNetFirewallRule(int32_t userId, int32_t ruleId, NetFirewallRuleAni &ruleAni);
int32_t GetInterceptedRecords(int32_t userId, const RequestParamAni &requestParamAni,
    InterceptedRecordPageAni &pageInfoAni);
} // namespace NetManagerAni
} // namespace OHOS

#endif // NET_FIREWALL_ANI_H
