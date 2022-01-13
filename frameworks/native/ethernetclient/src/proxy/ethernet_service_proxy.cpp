/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "ethernet_service_proxy.h"
#include "ipc_types.h"
#include "netmgr_ext_log_wrapper.h"
#include "ethernet_constants.h"

namespace OHOS {
namespace NetManagerStandard {
EthernetServiceProxy::EthernetServiceProxy(const sptr<IRemoteObject> &impl)
    :IRemoteProxy<IEthernetService>(impl)
{
}

EthernetServiceProxy::~EthernetServiceProxy() {}

bool EthernetServiceProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(EthernetServiceProxy::GetDescriptor())) {
        NETMGR_EXT_LOG_E("WriteInterfaceToken failed");
        return false;
    }
    return true;
}

int32_t EthernetServiceProxy::SetIfaceConfig(const std::string &iface, sptr<InterfaceConfiguration> &ic)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!data.WriteString(iface)) {
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!ic->Marshalling(data)) {
        NETMGR_EXT_LOG_E("proxy Marshalling failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(CMD_SET_IF_CFG, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return reply.ReadInt32();
}

sptr<InterfaceConfiguration> EthernetServiceProxy::GetIfaceConfig(const std::string &iface)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return nullptr;
    }
    if (!data.WriteString(iface)) {
        return nullptr;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return nullptr;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(CMD_GET_IF_CFG, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return nullptr;
    }
    int32_t res = 0;
    if (!reply.ReadInt32(res)) {
        return nullptr;
    }
    if (res != GET_CFG_SUC) {
        return nullptr;
    }
    return InterfaceConfiguration::Unmarshalling(reply);
}

int32_t EthernetServiceProxy::IsIfaceActive(const std::string &iface)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!data.WriteString(iface)) {
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(CMD_IS_ACTIVATE, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }

    return reply.ReadInt32();
}

std::vector<std::string> EthernetServiceProxy::GetAllActiveIfaces()
{
    MessageParcel data;
    std::vector<std::string> ifaces;
    if (!WriteInterfaceToken(data)) {
        return ifaces;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return ifaces;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(CMD_GET_ACTIVATE_INTERFACE, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return ifaces;
    }

    int32_t size = reply.ReadInt32();
    for (int i = 0; i < size; i++) {
        ifaces.push_back(reply.ReadString());
    }
    return  ifaces;
}

int32_t EthernetServiceProxy::ResetFactory()
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(CMD_RESET_FACTORY, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return reply.ReadInt32();
}
} // namespace NetManagerStandard
} // namespace OHOS