/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "vpn_connection.h"

#include "destroy_context.h"
#include "module_template.h"
#include "prepare_context.h"
#include "protect_context.h"
#include "setup_context.h"
#include "vpn_async_work.h"
#include "vpn_monitor.h"
#include "add_context.h"
#include "delete_context.h"
#include "get_list_context.h"
#include "get_context.h"
#include "get_connected_context.h"

namespace OHOS {
namespace NetManagerStandard {
namespace VpnConnection {
napi_value Prepare(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<PrepareContext>(env, info, PREPARE, nullptr, VpnAsyncWork::ExecPrepare,
                                                     VpnAsyncWork::PrepareCallback);
}

napi_value SetUp(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<SetUpContext>(env, info, SET_UP, nullptr, VpnAsyncWork::ExecSetUp,
                                                   VpnAsyncWork::SetUpCallback);
}

napi_value Protect(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<ProtectContext>(env, info, PROTECT, nullptr, VpnAsyncWork::ExecProtect,
                                                     VpnAsyncWork::ProtectCallback);
}

napi_value Destroy(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<DestroyContext>(env, info, DESTROY, nullptr, VpnAsyncWork::ExecDestroy,
                                                     VpnAsyncWork::DestroyCallback);
}

napi_value On(napi_env env, napi_callback_info info)
{
    return VpnMonitor::GetInstance().On(env, info);
}

napi_value Off(napi_env env, napi_callback_info info)
{
    return VpnMonitor::GetInstance().Off(env, info);
}

napi_value AddSystemVpn(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<AddContext>(env, info, SAVE_SYSTEM_VPN, nullptr,
        VpnAsyncWork::ExecAddSystemVpn, VpnAsyncWork::AddSystemVpnCallback);
}

napi_value DeleteSystemVpn(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<DeleteContext>(env, info, DELETE_SYSTEM_VPN, nullptr,
        VpnAsyncWork::ExecDeleteSystemVpn, VpnAsyncWork::DeleteSystemVpnCallback);
}

napi_value GetSystemVpnList(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetListContext>(env, info, GET_SYSTEM_VPN_LIST, nullptr,
        VpnAsyncWork::ExecGetSystemVpnList, VpnAsyncWork::GetSystemVpnListCallback);
}

napi_value GetSystemVpn(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetContext>(env, info, GET_SYSTEM_VPN, nullptr,
        VpnAsyncWork::ExecGetSystemVpn, VpnAsyncWork::GetSystemVpnCallback);
}

napi_value GetConnectedSystemVpn(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetConnectedContext>(env, info, GET_CONNECTED_SYSTEM_VPN, nullptr,
        VpnAsyncWork::ExecGetConnectedSystemVpn, VpnAsyncWork::GetConnectedSystemVpnCallback);
}
} // namespace VpnConnection
} // namespace NetManagerStandard
} // namespace OHOS