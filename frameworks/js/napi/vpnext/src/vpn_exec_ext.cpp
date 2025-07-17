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

#include "vpn_exec_ext.h"

#include <cstdint>
#include <securec.h>

#include "napi_utils.h"
#include "net_manager_constants.h"
#include "netmanager_ext_log.h"
#include "networkvpn_client.h"
#include "want.h"
#include "ability_manager_client.h"
#include "extension_ability_info.h"
#include "hi_app_event_report.h"
#ifdef SUPPORT_SYSVPN
#include "uuid.h"
#endif // SUPPORT_SYSVPN

namespace OHOS {
namespace NetManagerStandard {
namespace VpnExecExt {
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
    if (context == nullptr) {
        NETMANAGER_EXT_LOGE("context is nullptr");
        return false;
    }
    
    HiAppEventReport hiAppEventReport("NetworkKit", "VpnSetUp");
    auto vpnClient = GetVpnConnectionInstance(context);
    if (vpnClient == nullptr) {
        NETMANAGER_EXT_LOGE("vpnClient is nullptr");
        return false;
    }

    int32_t result = NETMANAGER_EXT_SUCCESS;
#ifdef SUPPORT_SYSVPN
    if (context->sysVpnConfig_ != nullptr) {
        result = vpnClient->SetUpVpn(context->sysVpnConfig_, true);
    } else {
        result = vpnClient->SetUpVpn(context->vpnConfig_, context->fd_, true);
    }
#else
    result = vpnClient->SetUpVpn(context->vpnConfig_, context->fd_, true);
#endif // SUPPORT_SYSVPN
    if (result != NETMANAGER_EXT_SUCCESS) {
        context->SetErrorCode(result);
        hiAppEventReport.ReportSdkEvent(RESULT_SUCCESS, result);
        return false;
    }
    hiAppEventReport.ReportSdkEvent(RESULT_SUCCESS, result);
    return true;
}

bool ExecProtect(ProtectContext *context)
{
    HiAppEventReport hiAppEventReport("NetworkKit", "VpnProtect");
    auto vpnClient = GetVpnConnectionInstance(context);
    if (vpnClient == nullptr) {
        NETMANAGER_EXT_LOGE("vpnClient is nullptr");
        return false;
    }
    int32_t result = vpnClient->Protect(context->socketFd_, true);
    if (result != NETMANAGER_EXT_SUCCESS) {
        context->SetErrorCode(result);
        hiAppEventReport.ReportSdkEvent(RESULT_SUCCESS, result);
        return false;
    }
    hiAppEventReport.ReportSdkEvent(RESULT_SUCCESS, result);
    return true;
}

bool ExecDestroy(DestroyContext *context)
{
    HiAppEventReport hiAppEventReport("NetworkKit", "VpnDestroy");
    auto vpnClient = GetVpnConnectionInstance(context);
    if (vpnClient == nullptr) {
        NETMANAGER_EXT_LOGE("vpnClient is nullptr");
        return false;
    }
    int32_t result = 0;
#ifdef SUPPORT_SYSVPN
    if (!context->vpnId_.empty()) {
        result = vpnClient->DestroyVpn(context->vpnId_);
    } else {
        result = vpnClient->DestroyVpn(true);
    }
#else
    result = vpnClient->DestroyVpn(true);
#endif // SUPPORT_SYSVPN
    if (result != NETMANAGER_EXT_SUCCESS) {
        context->SetErrorCode(result);
        hiAppEventReport.ReportSdkEvent(RESULT_SUCCESS, result);
        return false;
    }
    hiAppEventReport.ReportSdkEvent(RESULT_SUCCESS, ERR_NONE);
    return true;
}

#ifdef SUPPORT_SYSVPN
bool ExecGenerateVpnId(GenerateVpnIdContext *context)
{
    if (context == nullptr) {
        NETMANAGER_EXT_LOGE("context is nullptr");
        return false;
    }
    context->vpnId_ = UUID::RandomUUID().ToString();
    return true;
}
#endif // SUPPORT_SYSVPN

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
#ifdef SUPPORT_SYSVPN
napi_value GenerateVpnIdCallback(GenerateVpnIdContext *context)
{
    return NapiUtils::CreateStringUtf8(context->GetEnv(), context->vpnId_);
}
#endif // SUPPORT_SYSVPN
} // namespace VpnExecExt
} // namespace NetManagerStandard
} // namespace OHOS