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

#include "vpn_connection_ext.h"

#include "destroy_context_ext.h"
#include "module_template.h"
#include "prepare_context_ext.h"
#include "protect_context_ext.h"
#include "setup_context_ext.h"
#include "vpn_async_work_ext.h"
#include "vpn_monitor_ext.h"
#include "netmanager_ext_log.h"

namespace OHOS {
namespace NetManagerStandard {
namespace VpnConnectionExt {
napi_value Prepare(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<PrepareContext>(env, info, PREPARE, nullptr, VpnAsyncWorkExt::ExecPrepare,
                                                     VpnAsyncWorkExt::PrepareCallback);
}

napi_value SetUp(napi_env env, napi_callback_info info)
{
    NETMANAGER_EXT_LOGI("enter VpnConnectionExt SetUp");
    return ModuleTemplate::Interface<SetUpContext>(env, info, SET_UP_EXT, nullptr, VpnAsyncWorkExt::ExecSetUp,
                                                   VpnAsyncWorkExt::SetUpCallback);
}

napi_value Protect(napi_env env, napi_callback_info info)
{
    NETMANAGER_EXT_LOGI("enter VpnConnectionExt Protect");
    return ModuleTemplate::Interface<ProtectContext>(env, info, PROTECT_EXT, nullptr, VpnAsyncWorkExt::ExecProtect,
                                                     VpnAsyncWorkExt::ProtectCallback);
}

napi_value Destroy(napi_env env, napi_callback_info info)
{
    NETMANAGER_EXT_LOGI("enter VpnConnectionExt Destroy");
    return ModuleTemplate::Interface<DestroyContext>(env, info, DESTROY_EXT, nullptr, VpnAsyncWorkExt::ExecDestroy,
                                                     VpnAsyncWorkExt::DestroyCallback);
}

napi_value On(napi_env env, napi_callback_info info)
{
    return VpnMonitor::GetInstance().On(env, info);
}

napi_value Off(napi_env env, napi_callback_info info)
{
    return VpnMonitor::GetInstance().Off(env, info);
}
} // namespace VpnConnectionExt
} // namespace NetManagerStandard
} // namespace OHOS