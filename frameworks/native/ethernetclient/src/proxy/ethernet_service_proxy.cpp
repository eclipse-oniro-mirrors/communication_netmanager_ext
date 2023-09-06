/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "message_option.h"
#include "message_parcel.h"
#include "refbase.h"

#include "i_ethernet_service.h"
#include "interface_configuration.h"
#include "net_manager_constants.h"
#include "net_manager_ext_constants.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr int32_t MAX_SIZE = 16;
}

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
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(EthernetInterfaceCode::CMD_SET_IF_CFG),
                                      data, reply, option);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
    }
    return ret;
}

int32_t EthernetServiceProxy::GetIfaceConfig(const std::string &iface, sptr<InterfaceConfiguration> &ifaceConfig)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = SendRequest(data, iface, reply,
                              static_cast<uint32_t>(EthernetInterfaceCode::CMD_GET_IF_CFG));
    if (ret != ERR_NONE) {
        return ret;
    }
    int32_t res = 0;
    if (!reply.ReadInt32(res)) {
        return NETMANAGER_EXT_ERR_LOCAL_PTR_NULL;
    }
    if (res != GET_CFG_SUC) {
        return NETMANAGER_EXT_ERR_LOCAL_PTR_NULL;
    }
    ifaceConfig = InterfaceConfiguration::Unmarshalling(reply);
    if (ifaceConfig == nullptr) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t EthernetServiceProxy::IsIfaceActive(const std::string &iface, int32_t &activeStatus)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = SendRequest(data, iface, reply,
                              static_cast<uint32_t>(EthernetInterfaceCode::CMD_IS_ACTIVATE));
    if (ret != ERR_NONE) {
        return ret;
    }

    activeStatus = reply.ReadInt32();
    return NETMANAGER_EXT_SUCCESS;
}

int32_t EthernetServiceProxy::GetAllActiveIfaces(std::vector<std::string> &activeIfaces)
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
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(EthernetInterfaceCode::CMD_GET_ACTIVATE_INTERFACE),
                                      data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return ret;
    }

    int32_t size = reply.ReadInt32();
    if (size > MAX_SIZE) {
        NETMGR_EXT_LOG_E("size=[%{public}d] is too large", size);
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    for (int i = 0; i < size; i++) {
        activeIfaces.push_back(reply.ReadString());
    }
    return NETMANAGER_EXT_SUCCESS;
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
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(EthernetInterfaceCode::CMD_RESET_FACTORY),
                                      data, reply, option);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
    }
    return ret;
}

int32_t EthernetServiceProxy::RegisterIfacesStateChanged(const sptr<InterfaceStateCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_EXT_LOG_E("proxy callback is null");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_EXT_LOG_E("proxy write token error");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!data.WriteRemoteObject(callback->AsObject())) {
        NETMGR_EXT_LOG_E("proxy write callback failed");
        return NETMANAGER_EXT_ERR_WRITE_DATA_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(EthernetInterfaceCode::CMD_REGISTER_INTERFACE_CB),
                                      data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return reply.ReadInt32();
}

int32_t EthernetServiceProxy::UnregisterIfacesStateChanged(const sptr<InterfaceStateCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_EXT_LOG_E("proxy callback is null");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!data.WriteRemoteObject(callback->AsObject())) {
        NETMGR_EXT_LOG_E("proxy write callback failed");
        return NETMANAGER_EXT_ERR_WRITE_DATA_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(EthernetInterfaceCode::CMD_UNREGISTER_INTERFACE_CB),
                                      data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return reply.ReadInt32();
}

int32_t EthernetServiceProxy::SetInterfaceUp(const std::string &iface)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = SendRequest(data, iface, reply,
                              static_cast<uint32_t>(EthernetInterfaceCode::CMD_SET_INTERFACE_UP));
    if (ret != ERR_NONE) {
        return ret;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t EthernetServiceProxy::SetInterfaceDown(const std::string &iface)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = SendRequest(data, iface, reply,
                              static_cast<uint32_t>(EthernetInterfaceCode::CMD_SET_INTERFACE_DOWN));
    if (ret != ERR_NONE) {
        return ret;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t EthernetServiceProxy::GetInterfaceConfig(const std::string &iface, OHOS::nmd::InterfaceConfigurationParcel &cfg)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = SendRequest(data, iface, reply,
                              static_cast<uint32_t>(EthernetInterfaceCode::CMD_GET_INTERFACE_CONFIG));
    if (ret != ERR_NONE) {
        return ret;
    }
    reply.ReadString(cfg.ifName);
    reply.ReadString(cfg.hwAddr);
    reply.ReadString(cfg.ipv4Addr);
    reply.ReadInt32(cfg.prefixLength);
    int32_t vSize = reply.ReadInt32();
    if (vSize > MAX_SIZE) {
        NETMGR_EXT_LOG_E("vSize=[%{public}d] is too large", vSize);
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    std::vector<std::string> vecString;
    for (int i = 0; i < vSize; i++) {
        vecString.push_back(reply.ReadString());
    }
    if (vSize > 0) {
        cfg.flags.assign(vecString.begin(), vecString.end());
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t EthernetServiceProxy::SetInterfaceConfig(const std::string &iface, OHOS::nmd::InterfaceConfigurationParcel &cfg)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!data.WriteString(iface)) {
        return NETMANAGER_EXT_ERR_WRITE_DATA_FAIL;
    }
    if (!data.WriteString(cfg.ifName)) {
        return NETMANAGER_EXT_ERR_WRITE_DATA_FAIL;
    }
    if (!data.WriteString(cfg.hwAddr)) {
        return NETMANAGER_EXT_ERR_WRITE_DATA_FAIL;
    }
    if (!data.WriteString(cfg.ipv4Addr)) {
        return NETMANAGER_EXT_ERR_WRITE_DATA_FAIL;
    }
    if (!data.WriteInt32(cfg.prefixLength)) {
        return NETMANAGER_EXT_ERR_WRITE_DATA_FAIL;
    }
    uint32_t vecSize = static_cast<uint32_t>(cfg.flags.size());
    if (!data.WriteUint32(vecSize)) {
        return NETMANAGER_EXT_ERR_WRITE_DATA_FAIL;
    }
    for (auto str : cfg.flags) {
        if (!data.WriteString(str)) {
            return NETMANAGER_EXT_ERR_WRITE_DATA_FAIL;
        }
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(EthernetInterfaceCode::CMD_SET_INTERFACE_CONFIG),
                                      data, reply, option);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return ret;
}

int32_t EthernetServiceProxy::SendRequest(MessageParcel &data, const std::string &iface,
                                          MessageParcel &reply, uint32_t code)
{
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

    MessageOption option;
    int32_t ret = remote->SendRequest(code, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return ret;
    }

    return ERR_NONE;
}

} // namespace NetManagerStandard
} // namespace OHOS
