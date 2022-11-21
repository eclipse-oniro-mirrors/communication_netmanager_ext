/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "netshare_exec.h"

#include "constant.h"
#include "napi_utils.h"
#include "netmanager_ext_log.h"
#include "networkshare_constants.h"
#include "networkshare_client.h"
#include "net_manager_ext_constants.h"

namespace OHOS {
namespace NetManagerStandard {
bool NetShareExec::ExecIsSharingSupported(IsSharingSupportedContext *context)
{
    context->SetSharingSupported(DelayedSingleton<NetworkShareClient>::GetInstance()->IsSharingSupported());
    return true;
}

napi_value NetShareExec::IsSharingSupportedCallback(IsSharingSupportedContext *context)
{
    return NapiUtils::GetBoolean(context->GetEnv(), static_cast<bool>(context->GetSharingSupported()));
}

bool NetShareExec::ExecIsSharing(NetShareIsSharingContext *context)
{
    context->SetSharing(DelayedSingleton<NetworkShareClient>::GetInstance()->IsSharing());
    return true;
}

napi_value NetShareExec::IsSharingCallback(NetShareIsSharingContext *context)
{
    return NapiUtils::GetBoolean(context->GetEnv(), static_cast<bool>(context->GetSharing()));
}

bool NetShareExec::ExecStartSharing(NetShareStartSharingContext *context)
{
    SharingIfaceType ifaceType = static_cast<SharingIfaceType>(context->GetParam());
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->StartSharing(ifaceType);
    if (result != NETMANAGER_EXT_SUCCESS) {
        NETMANAGER_EXT_LOGE("ExecStartSharing error, errorCode: %{public}d", result);
        return false;
    }
    return true;
}

napi_value NetShareExec::StartSharingCallback(NetShareStartSharingContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool NetShareExec::ExecStopSharing(StopSharingContext *context)
{
    SharingIfaceType ifaceType = static_cast<SharingIfaceType>(context->GetParam());
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->StopSharing(ifaceType);
    if (result != NETMANAGER_EXT_SUCCESS) {
        NETMANAGER_EXT_LOGE("ExecStopSharing error, errorCode: %{public}d", result);
        return false;
    }
    return true;
}

napi_value NetShareExec::StopSharingCallback(StopSharingContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool NetShareExec::ExecGetSharingIfaces(GetSharingIfacesContext *context)
{
    SharingIfaceState ifaceState = static_cast<SharingIfaceState>(context->GetParam());
    context->SetIface(DelayedSingleton<NetworkShareClient>::GetInstance()->GetSharingIfaces(ifaceState));
    return true;
}

napi_value NetShareExec::GetSharingIfacesCallback(GetSharingIfacesContext *context)
{
    napi_value ifacesArray = NapiUtils::CreateArray(context->GetEnv(), context->GetIfaces().size());
    uint32_t index = 0;
    for (auto iface : context->GetIfaces()) {
        napi_value item = NapiUtils::CreateStringUtf8(context->GetEnv(), iface);
        NapiUtils::SetArrayElement(context->GetEnv(), ifacesArray, index++, item);
    }
    return ifacesArray;
}

bool NetShareExec::ExecGetSharingState(GetSharingStateContext *context)
{
    SharingIfaceType ifaceType = static_cast<SharingIfaceType>(context->GetParam());
    SharingIfaceState ifaceState;

    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->GetSharingState(ifaceType, ifaceState);
    if (result != NETMANAGER_EXT_SUCCESS) {
        NETMANAGER_EXT_LOGE("ExecGetSharingState error, errorCode: %{public}d", result);
        return false;
    }
    context->SetParam(static_cast<int32_t>(ifaceState));
    return true;
}

napi_value NetShareExec::GetSharingStateCallback(GetSharingStateContext *context)
{
    return NapiUtils::CreateInt32(context->GetEnv(), context->GetParam());
}

bool NetShareExec::ExecGetSharableRegexes(GetSharableRegexesContext *context)
{
    SharingIfaceType ifaceType = static_cast<SharingIfaceType>(context->GetParam());
    context->SetIface(DelayedSingleton<NetworkShareClient>::GetInstance()->GetSharableRegexs(ifaceType));
    return true;
}

napi_value NetShareExec::GetSharableRegexesCallback(GetSharableRegexesContext *context)
{
    napi_value ifacesArray = NapiUtils::CreateArray(context->GetEnv(), context->GetIfaces().size());
    uint32_t index = 0;
    for (auto iface : context->GetIfaces()) {
        napi_value item = NapiUtils::CreateStringUtf8(context->GetEnv(), iface);
        NapiUtils::SetArrayElement(context->GetEnv(), ifacesArray, index++, item);
    }
    return ifacesArray;
}

bool NetShareExec::ExecGetStatsRxBytes(GetStatsRxBytesContext *context)
{
    context->SetBytes32(DelayedSingleton<NetworkShareClient>::GetInstance()->GetStatsRxBytes());
    return true;
}

napi_value NetShareExec::GetStatsRxBytesCallback(GetStatsRxBytesContext *context)
{
    return NapiUtils::CreateInt32(context->GetEnv(), context->GetBytes32());
}

bool NetShareExec::ExecGetStatsTxBytes(GetStatsTxBytesContext *context)
{
    context->SetBytes32(DelayedSingleton<NetworkShareClient>::GetInstance()->GetStatsTxBytes());
    return true;
}

napi_value NetShareExec::GetStatsTxBytesCallback(GetStatsTxBytesContext *context)
{
    return NapiUtils::CreateInt32(context->GetEnv(), context->GetBytes32());
}

bool NetShareExec::ExecGetStatsTotalBytes(GetStatsTotalBytesContext *context)
{
    context->SetBytes32(DelayedSingleton<NetworkShareClient>::GetInstance()->GetStatsTotalBytes());
    return true;
}

napi_value NetShareExec::GetStatsTotalBytesCallback(GetStatsTotalBytesContext *context)
{
    return NapiUtils::CreateInt32(context->GetEnv(), context->GetBytes32());
}
} // namespace NetManagerStandard
} // namespace OHOS::NetManagerStandard
