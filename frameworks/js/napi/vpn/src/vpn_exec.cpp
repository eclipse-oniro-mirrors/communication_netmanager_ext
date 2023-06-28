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

#include "vpn_exec.h"

#include <cstdint>
#include <securec.h>

#include "napi_utils.h"
#include "net_manager_constants.h"
#include "netmanager_ext_log.h"
#include "networkvpn_client.h"

namespace OHOS {
namespace NetManagerStandard {
namespace VpnExec {
template <typename ContextT> static inline NetworkVpnClient *GetVpnConnectionInstance(ContextT *context)
{
    auto manager = context->GetManager();
    return (manager == nullptr) ? nullptr : reinterpret_cast<NetworkVpnClient *>(manager->GetData());
}

bool ExecPrepare(PrepareContext *context)
{
    auto vpnClient = GetVpnConnectionInstance(context);
    if (vpnClient == nullptr) {
        NETMANAGER_EXT_LOGE("vpnClient is nullptr");
        return false;
    }
    int32_t result = vpnClient->Prepare(context->isExistVpn_, context->isRun_, context->package_);
    if (result != NETMANAGER_EXT_SUCCESS) {
        context->SetErrorCode(result);
        return false;
    }
    return true;
}

bool ExecSetUp(SetUpContext *context)
{
    auto vpnClient = GetVpnConnectionInstance(context);
    if (vpnClient == nullptr) {
        NETMANAGER_EXT_LOGE("vpnClient is nullptr");
        return false;
    }
    int32_t result = vpnClient->SetUpVpn(context->vpnConfig_, context->fd_);
    if (result != NETMANAGER_EXT_SUCCESS) {
        context->SetErrorCode(result);
        return false;
    }
    return true;
}

bool ExecProtect(ProtectContext *context)
{
    auto vpnClient = GetVpnConnectionInstance(context);
    if (vpnClient == nullptr) {
        NETMANAGER_EXT_LOGE("vpnClient is nullptr");
        return false;
    }
    int32_t result = vpnClient->Protect(context->socketFd_);
    if (result != NETMANAGER_EXT_SUCCESS) {
        context->SetErrorCode(result);
        return false;
    }
    return true;
}

bool ExecDestroy(DestroyContext *context)
{
    auto vpnClient = GetVpnConnectionInstance(context);
    if (vpnClient == nullptr) {
        NETMANAGER_EXT_LOGE("vpnClient is nullptr");
        return false;
    }
    int32_t result = vpnClient->DestroyVpn();
    if (result != NETMANAGER_EXT_SUCCESS) {
        context->SetErrorCode(result);
        return false;
    }
    return true;
}

napi_value PrepareCallback(PrepareContext *context)
{
    napi_value obj = NapiUtils::CreateObject(context->GetEnv());
    NapiUtils::SetBooleanProperty(context->GetEnv(), obj, "isExistVpn", context->isExistVpn_);
    NapiUtils::SetBooleanProperty(context->GetEnv(), obj, "isRun", context->isRun_);
    NapiUtils::SetStringPropertyUtf8(context->GetEnv(), obj, "package", context->package_);
    return obj;
}

napi_value SetUpCallback(SetUpContext *context)
{
    return NapiUtils::CreateInt32(context->GetEnv(), context->fd_);
}

napi_value ProtectCallback(ProtectContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

napi_value DestroyCallback(DestroyContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}
} // namespace VpnExec
} // namespace NetManagerStandard
} // namespace OHOS