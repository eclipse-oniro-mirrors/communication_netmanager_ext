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
#include "get_netfirewall_status_context.h"
#include "set_netfirewall_status_context.h"
#include "update_netfirewall_rule_context.h"

namespace OHOS {
namespace NetManagerStandard {
namespace NetFirewallExec {
bool ExecSetNetFirewallStatus(SetNetFirewallStatusContext *context);

napi_value SetNetFirewallStatusCallback(SetNetFirewallStatusContext *context);

bool ExecGetNetFirewallStatus(GetNetFirewallStatusContext *context);

napi_value GetNetFirewallStatusCallback(GetNetFirewallStatusContext *context);

bool ExecAddNetFirewallRule(AddNetFirewallRuleContext *context);

napi_value AddNetFirewallRuleCallback(AddNetFirewallRuleContext *context);

bool ExecUpdateNetFirewallRule(UpdateNetFirewallRuleContext *context);

napi_value UpdateNetFirewallRuleCallback(UpdateNetFirewallRuleContext *context);

bool ExecDeleteNetFirewallRule(DeleteNetFirewallRuleContext *context);

napi_value DeleteNetFirewallRuleCallback(DeleteNetFirewallRuleContext *context);

bool ExecGetAllNetFirewallRules(GetAllNetFirewallRulesContext *context);

napi_value GetAllNetFirewallRulesCallback(GetAllNetFirewallRulesContext *context);

bool ExecGetNetFirewallRule(GetNetFirewallRuleContext *context);

napi_value GetNetFirewallRuleCallback(GetNetFirewallRuleContext *context);

bool ExecGetAllInterceptRecords(GetAllInterceptRecordsContext *context);

napi_value GetAllInterceptRecordCallbacks(GetAllInterceptRecordsContext *context);
} // namespace NetFirewallExec
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_FIREWALL_EXEC_H