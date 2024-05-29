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

#include <napi/native_api.h>

#include "add_netfirewall_rule_context.h"
#include "base_async_work.h"
#include "delete_netfirewall_rule_context.h"
#include "get_all_intercept_records_context.h"
#include "get_all_netfirewall_rules_context.h"
#include "get_netfirewall_rule_context.h"
#include "get_netfirewall_policy_context.h"
#include "net_firewall_async_work.h"
#include "net_firewall_exec.h"
#include "set_netfirewall_policy_context.h"
#include "update_netfirewall_rule_context.h"

namespace OHOS {
namespace NetManagerStandard {
namespace NetFirewallAsyncWork {
void ExecSetNetFirewallPolicy(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<SetNetFirewallPolicyContext, NetFirewallExec::ExecSetNetFirewallPolicy>(env, data);
}

void SetNetFirewallPolicyCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<SetNetFirewallPolicyContext, NetFirewallExec::SetNetFirewallPolicyCallback>(env,
        status, data);
}

void ExecGetNetFirewallPolicy(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<GetNetFirewallPolicyContext, NetFirewallExec::ExecGetNetFirewallPolicy>(env, data);
}

void GetNetFirewallPolicyCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<GetNetFirewallPolicyContext, NetFirewallExec::GetNetFirewallPolicyCallback>(env,
        status, data);
}

void ExecAddNetFirewallRule(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<AddNetFirewallRuleContext, NetFirewallExec::ExecAddNetFirewallRule>(env, data);
}

void AddNetFirewallRuleCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<AddNetFirewallRuleContext, NetFirewallExec::AddNetFirewallRuleCallback>(env,
        status, data);
}

void ExecUpdateNetFirewallRule(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<UpdateNetFirewallRuleContext, NetFirewallExec::ExecUpdateNetFirewallRule>(env, data);
}

void UpdateNetFirewallRuleCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<UpdateNetFirewallRuleContext, NetFirewallExec::UpdateNetFirewallRuleCallback>(env,
        status, data);
}

void ExecDeleteNetFirewallRule(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<DeleteNetFirewallRuleContext, NetFirewallExec::ExecDeleteNetFirewallRule>(env, data);
}

void DeleteNetFirewallRuleCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<DeleteNetFirewallRuleContext, NetFirewallExec::DeleteNetFirewallRuleCallback>(env,
        status, data);
}

void ExecGetNetFirewallRules(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<GetNetFirewallRulesContext, NetFirewallExec::ExecGetNetFirewallRules>(env, data);
}

void GetNetFirewallRulesCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<GetNetFirewallRulesContext, NetFirewallExec::GetNetFirewallRulesCallback>(
        env, status, data);
}

void ExecGetNetFirewallRule(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<GetNetFirewallRuleContext, NetFirewallExec::ExecGetNetFirewallRule>(env, data);
}

void GetNetFirewallRuleCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<GetNetFirewallRuleContext, NetFirewallExec::GetNetFirewallRuleCallback>(env,
        status, data);
}

void ExecGetInterceptRecords(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<GetInterceptRecordsContext, NetFirewallExec::ExecGetInterceptRecords>(env, data);
}

void GetInterceptRecordCallbacks(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<GetInterceptRecordsContext, NetFirewallExec::GetInterceptRecordCallbacks>(
        env, status, data);
}
} // namespace NetFirewallAsyncWork
} // namespace NetManagerStandard
} // namespace OHOS