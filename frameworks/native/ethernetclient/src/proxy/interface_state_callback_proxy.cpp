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

#include "interface_state_callback_proxy.h"

#include "ipc_types.h"
#include "parcel.h"

#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
InterfaceStateCallbackProxy::InterfaceStateCallbackProxy(const sptr<IRemoteObject> &object)
    : IRemoteProxy<InterfaceStateCallback>(object)
{
}

int32_t InterfaceStateCallbackProxy::OnInterfaceAdded(const std::string &ifName)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("OnInterfaceAdded get Remote() error.");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(InterfaceStateCallback::GetDescriptor())) {
        NETMGR_EXT_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }

    if (!data.WriteString(ifName)) {
        NETMGR_EXT_LOG_E("OnInterfaceAdded WriteString error.");
        return NETMANAGER_EXT_ERR_WRITE_DATA_FAIL;
    }
    int32_t ret = remote->SendRequest(static_cast<int32_t>(InterfaceStateCallback::Message::INTERFACE_STATE_ADD), data,
                                      reply, option);
    if (ret != ERR_NONE) {
        NETMGR_EXT_LOG_E("OnInterfaceAdded SendRequest error=[%{public}d].", ret);
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t InterfaceStateCallbackProxy::OnInterfaceRemoved(const std::string &ifName)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("OnInterfaceAdded get Remote() error.");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(InterfaceStateCallback::GetDescriptor())) {
        NETMGR_EXT_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }

    if (!data.WriteString(ifName)) {
        NETMGR_EXT_LOG_E("OnInterfaceRemoved WriteString error.");
        return NETMANAGER_EXT_ERR_WRITE_DATA_FAIL;
    }
    int32_t ret = remote->SendRequest(static_cast<int32_t>(InterfaceStateCallback::Message::INTERFACE_STATE_REMOVE),
                                      data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_EXT_LOG_E("OnInterfaceRemoved SendRequest error=[%{public}d].", ret);
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t InterfaceStateCallbackProxy::OnInterfaceChanged(const std::string &ifName, bool up)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("OnInterfaceChanged get Remote() error.");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(InterfaceStateCallback::GetDescriptor())) {
        NETMGR_EXT_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }

    if (!data.WriteString(ifName)) {
        NETMGR_EXT_LOG_E("OnInterfaceChanged WriteString error.");
        return NETMANAGER_EXT_ERR_WRITE_DATA_FAIL;
    }

    if (!data.WriteBool(up)) {
        NETMGR_EXT_LOG_E("OnInterfaceAdded WriteBool error.");
        return NETMANAGER_EXT_ERR_WRITE_DATA_FAIL;
    }

    int32_t ret = remote->SendRequest(static_cast<int32_t>(InterfaceStateCallback::Message::INTERFACE_STATE_CHANGE),
                                      data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_EXT_LOG_E("OnInterfaceAdded SendRequest error=[%{public}d].", ret);
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS
