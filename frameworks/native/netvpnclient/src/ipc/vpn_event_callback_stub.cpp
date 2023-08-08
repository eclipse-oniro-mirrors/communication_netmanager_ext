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

#include "vpn_event_callback_stub.h"
#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
int32_t VpnEventCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
                                              MessageOption &option)
{
    if (data.ReadInterfaceToken() != IVpnEventCallback::GetDescriptor()) {
        NETMGR_EXT_LOG_E("descriptor checked failed");
        return NETMANAGER_EXT_ERR_DESCRIPTOR_MISMATCH;
    }

    IVpnEventCallback::Message msgCode = static_cast<IVpnEventCallback::Message>(code);
    if (msgCode == IVpnEventCallback::Message::GLOBAL_VPN_STATE_CHANGED) {
        bool isConnected = false;
        if (!data.ReadBool(isConnected)) {
            NETMGR_EXT_LOG_E("IPC ReadBool failed");
            return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
        }
        OnVpnStateChanged(isConnected);
        return NETMANAGER_EXT_SUCCESS;
    } else if (msgCode == IVpnEventCallback::Message::GLOBAL_VPN_MULTI_USER_SETUP) {
        OnVpnMultiUserSetUp();
        return NETMANAGER_EXT_SUCCESS;
    }

    NETMGR_EXT_LOG_W("stub default case, need check");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
} // namespace NetManagerStandard
} // namespace OHOS
