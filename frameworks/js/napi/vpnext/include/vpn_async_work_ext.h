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

#ifndef VPN_ASYNC_WORK_H
#define VPN_ASYNC_WORK_H

#include <napi/native_api.h>

namespace OHOS {
namespace NetManagerStandard {
namespace VpnAsyncWorkExt {
void ExecPrepare(napi_env env, void *data);
void PrepareCallback(napi_env env, napi_status status, void *data);

void ExecSetUp(napi_env env, void *data);
void SetUpCallback(napi_env env, napi_status status, void *data);

void ExecProtect(napi_env env, void *data);
void ProtectCallback(napi_env env, napi_status status, void *data);

void ExecDestroy(napi_env env, void *data);
void DestroyCallback(napi_env env, napi_status status, void *data);
} // namespace VpnAsyncWorkExt
} // namespace NetManagerStandard
} // namespace OHOS
#endif // VPN_ASYNC_WORK_H