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

#ifndef NET_FIREWALL_EXEC_H
#define NET_FIREWALL_EXEC_H

#include <napi/native_api.h>

#include "add_netfirewall_rule_context.h"
#include "delete_netfirewall_rule_context.h"
#include "get_all_intercept_records_context.h"
#include "get_all_netfirewall_rules_context.h"
#include "get_netfirewall_rule_context.h"
#include "get_netfirewall_policy_context.h"
#include "set_netfirewall_policy_context.h"
#include "update_netfirewall_rule_context.h"

namespace OHOS {
namespace NetManagerStandard {
namespace NetFirewallExec {
bool ExecSetNetFirewallPolicy(SetNetFirewallPolicyContext *context);

napi_value SetNetFirewallPolicyCallback(SetNetFirewallPolicyContext *context);

bool ExecGetNetFirewallPolicy(GetNetFirewallPolicyContext *context);

napi_value GetNetFirewallPolicyCallback(GetNetFirewallPolicyContext *context);

bool ExecAddNetFirewallRule(AddNetFirewallRuleContext *context);

napi_value AddNetFirewallRuleCallback(AddNetFirewallRuleContext *context);

bool ExecUpdateNetFirewallRule(UpdateNetFirewallRuleContext *context);

napi_value UpdateNetFirewallRuleCallback(UpdateNetFirewallRuleContext *context);

bool ExecDeleteNetFirewallRule(DeleteNetFirewallRuleContext *context);

napi_value DeleteNetFirewallRuleCallback(DeleteNetFirewallRuleContext *context);

bool ExecGetNetFirewallRules(GetNetFirewallRulesContext *context);

napi_value GetNetFirewallRulesCallback(GetNetFirewallRulesContext *context);

bool ExecGetNetFirewallRule(GetNetFirewallRuleContext *context);

napi_value GetNetFirewallRuleCallback(GetNetFirewallRuleContext *context);

bool ExecGetInterceptRecords(GetInterceptRecordsContext *context);

napi_value GetInterceptRecordCallbacks(GetInterceptRecordsContext *context);
} // namespace NetFirewallExec
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_FIREWALL_EXEC_H