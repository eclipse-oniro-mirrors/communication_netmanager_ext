/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "sharing_ani.h"

#include <cstdint>

#include "cxx.h"
#include "net_manager_ext_constants.h"
#include "net_manager_constants.h"
#include "errorcode_convertor.h"
#include "wrapper.rs.h"

namespace OHOS {
namespace NetManagerAni {

sptr<NetShareCallbackObserverAni> g_netShareCallbackObserverAni =
    sptr<NetShareCallbackObserverAni>(new (std::nothrow) NetShareCallbackObserverAni());

bool g_isNetShareObserverRegistered = false;

rust::String GetErrorCodeAndMessage(int32_t &errorCode)
{
    NetManagerStandard::NetBaseErrorCodeConvertor convertor;
    return rust::string(convertor.ConvertErrorCode(errorCode));
}

void NetShareCallbackObserverAni::OnSharingStateChanged(const bool &isRunning)
{
    execute_sharing_state_changed(isRunning);
}

void NetShareCallbackObserverAni::OnInterfaceSharingStateChanged(NetManagerStandard::SharingIfaceType const &type,
    std::string const &iface, NetManagerStandard::SharingIfaceState const &state)
{
    InterfaceSharingStateInfo info{
        .share_type = type,
        .iface = rust::String(iface),
        .state = state,
    };
    execute_interface_sharing_state_change(info);
}

void NetShareCallbackObserverAni::OnSharingUpstreamChanged(sptr<NetManagerStandard::NetHandle> netHandle)
{
    NetHandle handle{
        .net_id = netHandle->GetNetId(),
    };
    execute_sharing_upstream_change(handle);
}

int32_t NetShareObserverRegister()
{
    if (g_isNetShareObserverRegistered) {
        return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
    }

    if (g_netShareCallbackObserverAni == nullptr) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    int32_t result = DelayedSingleton<NetManagerStandard::NetworkShareClient>::GetInstance()->RegisterSharingEvent(
        g_netShareCallbackObserverAni);
    if (result == NetManagerStandard::NETMANAGER_EXT_SUCCESS) {
        g_isNetShareObserverRegistered = true;
    }
    return result;
}

int32_t NetShareObserverUnRegister()
{
    if (g_netShareCallbackObserverAni == nullptr) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }

    int32_t result = DelayedSingleton<NetManagerStandard::NetworkShareClient>::GetInstance()->UnregisterSharingEvent(
        g_netShareCallbackObserverAni);
    if (result == NetManagerStandard::NETMANAGER_EXT_SUCCESS) {
        g_isNetShareObserverRegistered = false;
    }
    return result;
}

} // namespace NetManagerAni
} // namespace OHOS
