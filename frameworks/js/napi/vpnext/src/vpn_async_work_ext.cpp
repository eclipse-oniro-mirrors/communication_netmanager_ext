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

#include "vpn_async_work_ext.h"

#include <napi/native_api.h>

#include "base_async_work.h"
#include "destroy_context_ext.h"
#include "prepare_context_ext.h"
#include "protect_context_ext.h"
#include "setup_context_ext.h"
#include "vpn_exec_ext.h"

namespace OHOS {
namespace NetManagerStandard {
namespace VpnAsyncWorkExt {
void ExecPrepare(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<PrepareContext, VpnExecExt::ExecPrepare>(env, data);
}

void PrepareCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<PrepareContext, VpnExecExt::PrepareCallback>(env, status, data);
}

void ExecSetUp(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<SetUpContext, VpnExecExt::ExecSetUp>(env, data);
}

void SetUpCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<SetUpContext, VpnExecExt::SetUpCallback>(env, status, data);
}

void ExecProtect(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<ProtectContext, VpnExecExt::ExecProtect>(env, data);
}

void ProtectCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<ProtectContext, VpnExecExt::ProtectCallback>(env, status, data);
}

void ExecDestroy(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<DestroyContext, VpnExecExt::ExecDestroy>(env, data);
}

void DestroyCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<DestroyContext, VpnExecExt::DestroyCallback>(env, status, data);
}
} // namespace VpnAsyncWorkExt
} // namespace NetManagerStandard
} // namespace OHOS