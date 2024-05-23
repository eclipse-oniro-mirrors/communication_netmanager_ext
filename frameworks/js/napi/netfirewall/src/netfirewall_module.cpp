/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#include <napi/native_common.h>

#include "add_netfirewall_rule_context.h"
#include "delete_netfirewall_rule_context.h"
#include "get_all_intercept_records_context.h"
#include "get_all_netfirewall_rules_context.h"
#include "get_netfirewall_rule_context.h"
#include "get_netfirewall_status_context.h"
#include "module_template.h"
#include "napi_utils.h"
#include "net_firewall_async_work.h"
#include "set_netfirewall_status_context.h"
#include "update_netfirewall_rule_context.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
static constexpr const char *FUNCTION_SET_NET_FIREWALL_STATUS = "setNetFirewallStatus";
static constexpr const char *FUNCTION_GET_NET_FIREWALL_STATUS = "getNetFirewallStatus";
static constexpr const char *FUNCTION_ADD_NET_FIREWALL_RULE = "addNetFirewallRule";
static constexpr const char *FUNCTION_UPDATE_NET_FIREWALL_RULE = "updateNetFirewallRule";
static constexpr const char *FUNCTION_DELETE_NET_FIREWALL_RULE = "deleteNetFirewallRule";
static constexpr const char *FUNCTION_GET_NET_FIREWALL_RULES = "getNetFirewallRules";
static constexpr const char *FUNCTION_GET_NET_FIREWALL_RULE = "getNetFirewallRule";
static constexpr const char *FUNCTION_GET_INTERCEPT_RECORDS = "getInterceptRecords";

napi_value SetNetFirewallStatus(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<SetNetFirewallStatusContext>(env, info, FUNCTION_SET_NET_FIREWALL_STATUS, nullptr,
        NetFirewallAsyncWork::ExecSetNetFirewallStatus, NetFirewallAsyncWork::SetNetFirewallStatusCallback);
}

napi_value GetNetFirewallStatus(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetNetFirewallStatusContext>(env, info, FUNCTION_GET_NET_FIREWALL_STATUS, nullptr,
        NetFirewallAsyncWork::ExecGetNetFirewallStatus, NetFirewallAsyncWork::GetNetFirewallStatusCallback);
}

napi_value AddNetFirewallRule(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<AddNetFirewallRuleContext>(env, info, FUNCTION_ADD_NET_FIREWALL_RULE, nullptr,
        NetFirewallAsyncWork::ExecAddNetFirewallRule, NetFirewallAsyncWork::AddNetFirewallRuleCallback);
}

napi_value UpdateNetFirewallRule(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<UpdateNetFirewallRuleContext>(env, info, FUNCTION_UPDATE_NET_FIREWALL_RULE,
        nullptr, NetFirewallAsyncWork::ExecUpdateNetFirewallRule, NetFirewallAsyncWork::UpdateNetFirewallRuleCallback);
}

napi_value DeleteNetFirewallRule(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<DeleteNetFirewallRuleContext>(env, info, FUNCTION_DELETE_NET_FIREWALL_RULE,
        nullptr, NetFirewallAsyncWork::ExecDeleteNetFirewallRule, NetFirewallAsyncWork::DeleteNetFirewallRuleCallback);
}

napi_value GetNetFirewallRules(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetNetFirewallRulesContext>(env, info, FUNCTION_GET_NET_FIREWALL_RULES,
        nullptr, NetFirewallAsyncWork::ExecGetNetFirewallRules,
        NetFirewallAsyncWork::GetNetFirewallRulesCallback);
}

napi_value GetNetFirewallRule(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetNetFirewallRuleContext>(env, info, FUNCTION_GET_NET_FIREWALL_RULE, nullptr,
        NetFirewallAsyncWork::ExecGetNetFirewallRule, NetFirewallAsyncWork::GetNetFirewallRuleCallback);
}

napi_value GetInterceptRecords(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetInterceptRecordsContext>(env, info, FUNCTION_GET_INTERCEPT_RECORDS,
        nullptr, NetFirewallAsyncWork::ExecGetInterceptRecords,
        NetFirewallAsyncWork::GetInterceptRecordCallbacks);
}
} // namespace

napi_value DeclareNetFirewallInterface(napi_env env, napi_value exports)
{
    NapiUtils::DefineProperties(env, exports,
        {
        DECLARE_NAPI_FUNCTION(FUNCTION_SET_NET_FIREWALL_STATUS, SetNetFirewallStatus),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_NET_FIREWALL_STATUS, GetNetFirewallStatus),
        DECLARE_NAPI_FUNCTION(FUNCTION_ADD_NET_FIREWALL_RULE, AddNetFirewallRule),
        DECLARE_NAPI_FUNCTION(FUNCTION_UPDATE_NET_FIREWALL_RULE, UpdateNetFirewallRule),
        DECLARE_NAPI_FUNCTION(FUNCTION_DELETE_NET_FIREWALL_RULE, DeleteNetFirewallRule),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_NET_FIREWALL_RULES, GetNetFirewallRules),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_NET_FIREWALL_RULE, GetNetFirewallRule),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_INTERCEPT_RECORDS, GetInterceptRecords),
        });
    return exports;
}

static napi_module g_netfirewallModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = DeclareNetFirewallInterface,
    .nm_modname = "net.netfirewall",
    .nm_priv = (0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterNetFirewallModule(void)
{
    napi_module_register(&g_netfirewallModule);
}
} // namespace NetManagerStandard
} // namespace OHOS
