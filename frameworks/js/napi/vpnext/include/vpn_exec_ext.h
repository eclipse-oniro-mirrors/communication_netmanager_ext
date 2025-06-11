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

#ifndef VPN_EXEC_H
#define VPN_EXEC_H

#include <napi/native_api.h>

#include "destroy_context_ext.h"
#include "prepare_context_ext.h"
#include "protect_context_ext.h"
#include "setup_context_ext.h"
#ifdef SUPPORT_SYSVPN
#include "generate_vpnId_context_ext.h"
#endif // SUPPORT_SYSVPN

namespace OHOS {
namespace NetManagerStandard {
namespace VpnExecExt {
bool ExecPrepare(PrepareContext *context);
napi_value PrepareCallback(PrepareContext *context);

bool ExecSetUp(SetUpContext *context);
napi_value SetUpCallback(SetUpContext *context);

bool ExecProtect(ProtectContext *context);
napi_value ProtectCallback(ProtectContext *context);

bool ExecDestroy(DestroyContext *context);
napi_value DestroyCallback(DestroyContext *context);

#ifdef SUPPORT_SYSVPN
bool ExecGenerateVpnId(GenerateVpnIdContext *context);
napi_value GenerateVpnIdCallback(GenerateVpnIdContext *context);
#endif // SUPPORT_SYSVPN
} // namespace VpnExecExt
} // namespace NetManagerStandard
} // namespace OHOS
#endif // VPN_EXEC_H