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

#include <memory>

#include "networkvpn_service_proxy.h"

#include "ipc_types.h"

#include "net_manager_constants.h"
#include "net_manager_ext_constants.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetworkVpnServiceProxy::NetworkVpnServiceProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<INetworkVpnService>(impl)
{
}

int32_t NetworkVpnServiceProxy::WriteTokenAndSendRequest(INetworkVpnService::MessageCode code, MessageParcel &data,
                                                         MessageParcel &reply)
{
    if (!data.WriteInterfaceToken(NetworkVpnServiceProxy::GetDescriptor())) {
        NETMGR_EXT_LOG_E("write interface token failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("remote is nullptr");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageOption option(MessageOption::TF_SYNC);
    return remote->SendRequest(static_cast<uint32_t>(code), data, reply, option);
}

int32_t NetworkVpnServiceProxy::SendRequest(INetworkVpnService::MessageCode code, MessageParcel &data,
                                            MessageParcel &reply)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("remote is nullptr");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageOption option(MessageOption::TF_SYNC);
    return remote->SendRequest(static_cast<uint32_t>(code), data, reply, option);
}

int32_t NetworkVpnServiceProxy::Prepare(bool &isExistVpn, bool &isRun, std::string &pkg)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = WriteTokenAndSendRequest(INetworkVpnService::MessageCode::CMD_PREPARE, data, reply);
    if (ERR_NONE != ret) {
        NETMGR_EXT_LOG_E("Prepare proxy SendRequest failed, error code: [%{public}d]", ret);
        return ret;
    }

    int32_t result = NETMANAGER_EXT_ERR_INTERNAL;
    if (!reply.ReadInt32(result)) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    if (!reply.ReadBool(isExistVpn) || !reply.ReadBool(isRun) || !reply.ReadString(pkg)) {
        NETMGR_EXT_LOG_E("Prepare proxy read data failed");
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    return result;
}

int32_t NetworkVpnServiceProxy::GetSelfAppName(std::string &selfAppName)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceProxy::GetDescriptor())) {
        NETMGR_EXT_LOG_E("write interface token failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    MessageParcel reply;
    auto ret = SendRequest(INetworkVpnService::MessageCode::CMD_GET_SELF_APP_NAME, data, reply);
    if (ret != ERR_NONE) {
        NETMGR_EXT_LOG_E("SendRequest failed %{public}d", ret);
        return ret;
    }
    if (!reply.ReadString(selfAppName)) {
        NETMGR_EXT_LOG_E("IPC ReadString failed");
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    return ERR_NONE;
}

int32_t NetworkVpnServiceProxy::SetUpVpn(const sptr<VpnConfig> &config, bool isVpnExtCall)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceProxy::GetDescriptor())) {
        NETMGR_EXT_LOG_E("write interface token failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!config->Marshalling(data)) {
        NETMGR_EXT_LOG_E("SetUpVpn proxy Marshalling failed");
        return NETMANAGER_EXT_ERR_WRITE_DATA_FAIL;
    }

    MessageParcel reply;
    int32_t ret = 0;
    if (isVpnExtCall) {
        ret = SendRequest(INetworkVpnService::MessageCode::CMD_START_VPN_EXT, data, reply);
    } else {
        ret = SendRequest(INetworkVpnService::MessageCode::CMD_START_VPN, data, reply);
    }
    if (ERR_NONE != ret) {
        NETMGR_EXT_LOG_E("SetUpVpn proxy SendRequest failed, error code: [%{public}d]", ret);
        return ret;
    }
    int32_t result = NETMANAGER_EXT_ERR_INTERNAL;
    if (!reply.ReadInt32(result)) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    return result;
}

int32_t NetworkVpnServiceProxy::Protect(bool isVpnExtCall)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = 0;
    if (isVpnExtCall) {
        ret = WriteTokenAndSendRequest(INetworkVpnService::MessageCode::CMD_PROTECT_EXT, data, reply);
    } else {
        ret = WriteTokenAndSendRequest(INetworkVpnService::MessageCode::CMD_PROTECT, data, reply);
    }
    if (ERR_NONE != ret) {
        NETMGR_EXT_LOG_E("Protect proxy SendRequest failed, error code: [%{public}d]", ret);
        return ret;
    }
    int32_t result = NETMANAGER_EXT_ERR_INTERNAL;
    if (!reply.ReadInt32(result)) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    return result;
}

int32_t NetworkVpnServiceProxy::DestroyVpn(bool isVpnExtCall)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = 0;
    if (isVpnExtCall) {
        ret = WriteTokenAndSendRequest(INetworkVpnService::MessageCode::CMD_STOP_VPN_EXT, data, reply);
    } else {
        ret = WriteTokenAndSendRequest(INetworkVpnService::MessageCode::CMD_STOP_VPN, data, reply);
    }
    if (ERR_NONE != ret) {
        NETMGR_EXT_LOG_E("DestroyVpn proxy SendRequest failed, error code: [%{public}d]", ret);
        return ret;
    }
    int32_t result = NETMANAGER_EXT_ERR_INTERNAL;
    if (!reply.ReadInt32(result)) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    return result;
}

#ifdef SUPPORT_SYSVPN
int32_t NetworkVpnServiceProxy::AddSysVpnConfig(sptr<SysVpnConfig> &config)
{
    NETMGR_EXT_LOG_D("AddSysVpnConfig start");
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceProxy::GetDescriptor())) {
        NETMGR_EXT_LOG_E("AddSysVpnConfig write interface token failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!config->Marshalling(data)) {
        NETMGR_EXT_LOG_E("AddSysVpnConfig proxy Marshalling failed");
        return NETMANAGER_EXT_ERR_WRITE_DATA_FAIL;
    }
    MessageParcel reply;
    int32_t ret = SendRequest(INetworkVpnService::MessageCode::CMD_ADD_SYS_VPN_CONFIG, data, reply);
    if (ERR_NONE != ret) {
        NETMGR_EXT_LOG_E("AddSysVpnConfig proxy SendRequest failed, error code: [%{public}d]", ret);
        return ret;
    }
    if (!reply.ReadInt32(ret)) {
        return NETMANAGER_EXT_ERR_READ_REPLY_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnServiceProxy::DeleteSysVpnConfig(std::string &vpnId)
{
    NETMGR_EXT_LOG_D("DeleteSysVpnConfig start");
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceProxy::GetDescriptor())) {
        NETMGR_EXT_LOG_E("DeleteSysVpnConfig write interface token failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!data.WriteString(vpnId)) {
        NETMGR_EXT_LOG_E("DeleteSysVpnConfig proxy write data failed");
        return NETMANAGER_EXT_ERR_WRITE_DATA_FAIL;
    }
    MessageParcel reply;
    int32_t ret = SendRequest(INetworkVpnService::MessageCode::CMD_DELETE_SYS_VPN_CONFIG, data, reply);
    if (ERR_NONE != ret) {
        NETMGR_EXT_LOG_E("DeleteSysVpnConfig proxy SendRequest failed, error code: [%{public}d]", ret);
        return ret;
    }
    if (!reply.ReadInt32(ret)) {
        return NETMANAGER_EXT_ERR_READ_REPLY_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnServiceProxy::GetSysVpnConfigList(std::vector<SysVpnConfig> &vpnList)
{
    NETMGR_EXT_LOG_D("GetSysVpnConfigList start");
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = WriteTokenAndSendRequest(INetworkVpnService::MessageCode::CMD_GET_SYS_VPN_CONFIG_LIST, data, reply);
    if (ERR_NONE != ret) {
        NETMGR_EXT_LOG_E("GetSysVpnConfigList proxy WriteTokenAndSendRequest failed, error code: [%{public}d]", ret);
        return ret;
    }
    int vpnListSize = 0;
    if (!reply.ReadInt32(vpnListSize)) {
        NETMGR_EXT_LOG_E("GetSysVpnConfigList read data size failed");
        return NETMANAGER_EXT_ERR_READ_REPLY_FAIL;
    }
    for (int32_t idx = 0; idx < vpnListSize; idx++) {
        sptr<SysVpnConfig> vpnConfig = new (std::nothrow) SysVpnConfig();
        if (vpnConfig == nullptr) {
            NETMGR_EXT_LOG_E("GetSysVpnConfigList vpnConfig is null");
            return NETMANAGER_EXT_ERR_READ_REPLY_FAIL;
        }
        reply.ReadString(vpnConfig->vpnId_);
        reply.ReadString(vpnConfig->vpnName_);
        reply.ReadInt32(vpnConfig->vpnType_);
        vpnList.push_back(*vpnConfig);
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnServiceProxy::GetSysVpnConfig(sptr<SysVpnConfig> &config, std::string &vpnId)
{
    NETMGR_EXT_LOG_D("GetSysVpnConfig start");
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceProxy::GetDescriptor())) {
        NETMGR_EXT_LOG_E("GetSysVpnConfig write interface token failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!data.WriteString(vpnId)) {
        NETMGR_EXT_LOG_E("GetSysVpnConfig proxy write data failed");
        return NETMANAGER_EXT_ERR_WRITE_DATA_FAIL;
    }

    MessageParcel reply;
    int32_t ret = SendRequest(INetworkVpnService::MessageCode::CMD_GET_SYS_VPN_CONFIG, data, reply);
    if (ERR_NONE != ret) {
        NETMGR_EXT_LOG_E("GetSysVpnConfig proxy SendRequest failed, error code: [%{public}d]", ret);
        return ret;
    }

    config = SysVpnConfig::Unmarshalling(reply);
    if (config == nullptr) {
        NETMGR_EXT_LOG_I("GetSysVpnConfig config == nullptr");
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnServiceProxy::GetConnectedSysVpnConfig(sptr<SysVpnConfig> &config)
{
    NETMGR_EXT_LOG_D("GetConnectedSysVpnConfig start");
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = 0;
    ret = WriteTokenAndSendRequest(INetworkVpnService::MessageCode::CMD_GET_CONNECTED_SYS_VPN_CONFIG, data, reply);
    if (ERR_NONE != ret) {
        NETMGR_EXT_LOG_E("GetConnectedSysVpnConfig proxy SendRequest failed, error code: [%{public}d]", ret);
        return ret;
    }
    config = SysVpnConfig::Unmarshalling(reply);
    if (config == nullptr) {
        NETMGR_EXT_LOG_I("GetConnectedSysVpnConfig config == nullptr");
    }
    return NETMANAGER_EXT_SUCCESS;
}
#endif // SUPPORT_SYSVPN

int32_t NetworkVpnServiceProxy::RegisterVpnEvent(sptr<IVpnEventCallback> callback)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceProxy::GetDescriptor())) {
        NETMGR_EXT_LOG_E("write interface token failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!data.WriteRemoteObject(callback->AsObject())) {
        NETMGR_EXT_LOG_E("RegisterVpnEvent proxy write callback failed");
        return NETMANAGER_EXT_ERR_WRITE_DATA_FAIL;
    }

    MessageParcel reply;
    int32_t ret = SendRequest(INetworkVpnService::MessageCode::CMD_REGISTER_EVENT_CALLBACK, data, reply);
    if (ERR_NONE != ret) {
        NETMGR_EXT_LOG_E("RegisterVpnEvent proxy SendRequest failed, error code: [%{public}d]", ret);
        return ret;
    }
    int32_t result = NETMANAGER_EXT_ERR_INTERNAL;
    if (!reply.ReadInt32(result)) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    return result;
}

int32_t NetworkVpnServiceProxy::UnregisterVpnEvent(sptr<IVpnEventCallback> callback)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceProxy::GetDescriptor())) {
        NETMGR_EXT_LOG_E("write interface token failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!data.WriteRemoteObject(callback->AsObject())) {
        NETMGR_EXT_LOG_E("UnregisterVpnEvent proxy write callback failed");
        return NETMANAGER_EXT_ERR_WRITE_DATA_FAIL;
    }

    MessageParcel reply;
    int32_t ret = SendRequest(INetworkVpnService::MessageCode::CMD_UNREGISTER_EVENT_CALLBACK, data, reply);
    if (ERR_NONE != ret) {
        NETMGR_EXT_LOG_E("UnregisterVpnEvent proxy SendRequest failed, error code: [%{public}d]", ret);
        return ret;
    }
    int32_t result = NETMANAGER_EXT_ERR_INTERNAL;
    if (!reply.ReadInt32(result)) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    return result;
}

int32_t NetworkVpnServiceProxy::CreateVpnConnection(bool isVpnExtCall)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = 0;
    if (isVpnExtCall) {
        ret = WriteTokenAndSendRequest(INetworkVpnService::MessageCode::CMD_CREATE_VPN_CONNECTION_EXT, data, reply);
    } else {
        ret = WriteTokenAndSendRequest(INetworkVpnService::MessageCode::CMD_CREATE_VPN_CONNECTION, data, reply);
    }
    if (ERR_NONE != ret) {
        NETMGR_EXT_LOG_E("CreateVpnConnection proxy SendRequest failed, error code: [%{public}d]", ret);
        return ret;
    }
    int32_t result = NETMANAGER_EXT_ERR_INTERNAL;
    if (!reply.ReadInt32(result)) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    return result;
}

int32_t NetworkVpnServiceProxy::RegisterBundleName(const std::string &bundleName)
{
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnServiceProxy::FactoryResetVpn()
{
    return NETMANAGER_EXT_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS
