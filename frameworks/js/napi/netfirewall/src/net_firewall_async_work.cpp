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

#include "net_firewall_async_work.h"

#include <napi/native_api.h>

#include "add_netfirewall_rule_context.h"
#include "base_async_work.h"
#include "delete_netfirewall_rule_context.h"
#include "get_all_intercept_records_context.h"
#include "get_all_netfirewall_rules_context.h"
#include "get_netfirewall_rule_context.h"
#include "get_netfirewall_status_context.h"
#include "net_firewall_exec.h"
#include "set_netfirewall_status_context.h"
#include "update_netfirewall_rule_context.h"

namespace OHOS {
namespace NetManagerStandard {
namespace NetFirewallAsyncWork {
void ExecSetNetFirewallStatus(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<SetNetFirewallStatusContext, NetFirewallExec::ExecSetNetFirewallStatus>(env, data);
}

void SetNetFirewallStatusCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<SetNetFirewallStatusContext, NetFirewallExec::SetNetFirewallStatusCallback>(env,
        status, data);
}

void ExecGetNetFirewallStatus(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<GetNetFirewallStatusContext, NetFirewallExec::ExecGetNetFirewallStatus>(env, data);
}

void GetNetFirewallStatusCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<GetNetFirewallStatusContext, NetFirewallExec::GetNetFirewallStatusCallback>(env,
        status, data);
}
} // namespace NetFirewallAsyncWork
} // namespace NetManagerStandard
} // namespace OHOS