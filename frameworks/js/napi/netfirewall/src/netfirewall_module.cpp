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
static constexpr const char *FUNCTION_GET_ALL_INTERCEPT_RECORDS = "getInterceptRecords";

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

napi_value GetAllInterceptRecords(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetAllInterceptRecordsContext>(env, info, FUNCTION_GET_ALL_INTERCEPT_RECORDS,
        nullptr, NetFirewallAsyncWork::ExecGetAllInterceptRecords,
        NetFirewallAsyncWork::GetAllInterceptRecordCallbacks);
}
} // namespace

napi_value DeclarePcfirewallInterface(napi_env env, napi_value exports)
{
    NapiUtils::DefineProperties(env, exports,
        {
        DECLARE_NAPI_FUNCTION(FUNCTION_SET_NET_FIREWALL_STATUS, SetNetFirewallStatus),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_NET_FIREWALL_STATUS, GetNetFirewallStatus),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_ALL_INTERCEPT_RECORDS, GetAllInterceptRecords),
        });
    return exports;
}

static napi_module g_netfirewallModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = DeclarePcfirewallInterface,
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
