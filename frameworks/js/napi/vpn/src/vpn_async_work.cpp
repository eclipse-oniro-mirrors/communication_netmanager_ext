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

#include "vpn_async_work.h"

#include <napi/native_api.h>

#include "base_async_work.h"
#include "destroy_context.h"
#include "prepare_context.h"
#include "protect_context.h"
#include "setup_context.h"
#include "vpn_exec.h"

namespace OHOS {
namespace NetManagerStandard {
namespace VpnAsyncWork {
void ExecPrepare(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<PrepareContext, VpnExec::ExecPrepare>(env, data);
}

void PrepareCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<PrepareContext, VpnExec::PrepareCallback>(env, status, data);
}

void ExecSetUp(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<SetUpContext, VpnExec::ExecSetUp>(env, data);
}

void SetUpCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<SetUpContext, VpnExec::SetUpCallback>(env, status, data);
}

void ExecProtect(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<ProtectContext, VpnExec::ExecProtect>(env, data);
}

void ProtectCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<ProtectContext, VpnExec::ProtectCallback>(env, status, data);
}

void ExecDestroy(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<DestroyContext, VpnExec::ExecDestroy>(env, data);
}

void DestroyCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<DestroyContext, VpnExec::DestroyCallback>(env, status, data);
}

void ExecAddSystemVpn(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<AddContext, VpnExec::ExecAddSystemVpn>(env, data);
}

void AddSystemVpnCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<AddContext, VpnExec::AddSystemVpnCallback>(env, status, data);
}

void ExecDeleteSystemVpn(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<DeleteContext, VpnExec::ExecDeleteSystemVpn>(env, data);
}

void DeleteSystemVpnCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<DeleteContext, VpnExec::DeleteSystemVpnCallback>(env, status, data);
}

void ExecGetSystemVpnList(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<GetListContext, VpnExec::ExecGetSystemVpnList>(env, data);
}

void GetSystemVpnListCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<GetListContext, VpnExec::GetSystemVpnListCallback>(env, status, data);
}

void ExecGetSystemVpn(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<GetContext, VpnExec::ExecGetSystemVpn>(env, data);
}

void GetSystemVpnCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<GetContext, VpnExec::GetSystemVpnCallback>(env, status, data);
}

void ExecGetConnectedSystemVpn(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<GetConnectedContext, VpnExec::ExecGetConnectedSystemVpn>(env, data);
}

void GetConnectedSystemVpnCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<GetConnectedContext, VpnExec::GetConnectedSystemVpnCallback>(env, status, data);
}
} // namespace VpnAsyncWork
} // namespace NetManagerStandard
} // namespace OHOS