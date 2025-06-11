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

#include "destroy_context.h"
#include "prepare_context.h"
#include "protect_context.h"
#include "setup_context.h"
#ifdef SUPPORT_SYSVPN
#include "add_context.h"
#include "delete_context.h"
#include "get_app_info_context.h"
#include "get_list_context.h"
#include "get_context.h"
#include "get_connected_context.h"
#endif // SUPPORT_SYSVPN

namespace OHOS {
namespace NetManagerStandard {
namespace VpnExec {
bool ExecPrepare(PrepareContext *context);
napi_value PrepareCallback(PrepareContext *context);

bool ExecSetUp(SetUpContext *context);
napi_value SetUpCallback(SetUpContext *context);

bool ExecProtect(ProtectContext *context);
napi_value ProtectCallback(ProtectContext *context);

bool ExecDestroy(DestroyContext *context);
napi_value DestroyCallback(DestroyContext *context);

#ifdef SUPPORT_SYSVPN
bool ExecAddSysVpnConfig(AddContext *context);
napi_value AddSysVpnConfigCallback(AddContext *context);

bool ExecDeleteSysVpnConfig(DeleteContext *context);
napi_value DeleteSysVpnConfigCallback(DeleteContext *context);

bool ExecGetSysVpnConfigList(GetListContext *context);
napi_value GetSysVpnConfigListCallback(GetListContext *context);

bool ExecGetSysVpnConfig(GetContext *context);
napi_value GetSysVpnConfigCallback(GetContext *context);

bool ExecGetConnectedSysVpnConfig(GetConnectedContext *context);
napi_value GetConnectedSysVpnConfigCallback(GetConnectedContext *context);

bool ExecGetConnectedVpnAppInfo(GetAppInfoContext *context);
napi_value GetConnectedVpnAppInfoCallback(GetAppInfoContext *context);
#endif // SUPPORT_SYSVPN
} // namespace VpnExec
} // namespace NetManagerStandard
} // namespace OHOS
#endif // VPN_EXEC_H