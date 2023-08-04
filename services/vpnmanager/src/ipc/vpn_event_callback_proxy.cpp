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

#include "vpn_event_callback_proxy.h"
#include "netmgr_ext_log_wrapper.h"

#include "ipc_types.h"
#include "parcel.h"

namespace OHOS {
namespace NetManagerStandard {
VpnEventCallbackProxy::VpnEventCallbackProxy(const sptr<IRemoteObject> &object)
    : IRemoteProxy<IVpnEventCallback>(object)
{
}

void VpnEventCallbackProxy::OnVpnStateChanged(const bool &isConnected)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(IVpnEventCallback::GetDescriptor())) {
        NETMGR_EXT_LOG_E("write interface token failed");
        return;
    }

    if (!data.WriteBool(isConnected)) {
        NETMGR_EXT_LOG_E("OnVpnStateChanged WriteBool error.");
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("OnVpnStateChanged get Remote() error.");
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(IVpnEventCallback::Message::GLOBAL_VPN_STATE_CHANGED), data,
                                      reply, option);
    if (ret != ERR_NONE) {
        NETMGR_EXT_LOG_E("OnVpnStateChanged SendRequest error=[%{public}d].", ret);
    }
}

void VpnEventCallbackProxy::OnVpnMultiUserSetUp()
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(IVpnEventCallback::GetDescriptor())) {
        NETMGR_EXT_LOG_E("write interface token failed");
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("OnVpnMultiUserSetUp get Remote() error.");
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(IVpnEventCallback::Message::GLOBAL_VPN_MULTI_USER_SETUP),
                                      data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_EXT_LOG_E("OnVpnMultiUserSetUp SendRequest error=[%{public}d].", ret);
    }
}
} // namespace NetManagerStandard
} // namespace OHOS
