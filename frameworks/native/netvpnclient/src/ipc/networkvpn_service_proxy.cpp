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
int32_t NetworkVpnServiceProxy::SetUpVpn(const sptr<SysVpnConfig> &config)
{
    if (config == nullptr) {
        NETMGR_EXT_LOG_E("SetUpVpn failed, config is null");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    NETMGR_EXT_LOG_I("NetworkVpnServiceProxy SetUpVpn id=%{public}s", config->vpnId_.c_str());
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceProxy::GetDescriptor())) {
        NETMGR_EXT_LOG_E("write interface token failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }

    if (!(data.WriteString(config->vpnId_) && data.WriteInt32(config->vpnType_))) {
        NETMGR_EXT_LOG_E("SetUpVpn proxy write data failed");
        return NETMANAGER_EXT_ERR_WRITE_DATA_FAIL;
    }
    MessageParcel reply;
    int32_t ret = SendRequest(INetworkVpnService::MessageCode::CMD_SETUP_SYS_VPN, data, reply);
    if (ret != ERR_NONE) {
        NETMGR_EXT_LOG_E("SetUpVpn proxy SendRequest failed, error code: [%{public}d]", ret);
        return ret;
    }
    int32_t result = NETMANAGER_EXT_ERR_INTERNAL;
    if (!reply.ReadInt32(result)) {
        NETMGR_EXT_LOG_E("reply read data failed");
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    return result;
}

int32_t NetworkVpnServiceProxy::AddSysVpnConfig(sptr<SysVpnConfig> &config)
{
    if (config == nullptr) {
        NETMGR_EXT_LOG_E("AddSysVpnConfig failed, config is null");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    NETMGR_EXT_LOG_I("NetworkVpnServiceProxy AddSysVpnConfig id=%{public}s", config->vpnId_.c_str());
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
    if (ret != ERR_NONE) {
        NETMGR_EXT_LOG_E("AddSysVpnConfig proxy SendRequest failed, error code: [%{public}d]", ret);
        return ret;
    }
    if (!reply.ReadInt32(ret)) {
        NETMGR_EXT_LOG_E("reply ReadInt32 failed");
        return NETMANAGER_EXT_ERR_READ_REPLY_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnServiceProxy::DeleteSysVpnConfig(const std::string &vpnId)
{
    if (vpnId.empty()) {
        NETMGR_EXT_LOG_E("DeleteSysVpnConfig failed, vpnId is empty");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    NETMGR_EXT_LOG_I("NetworkVpnServiceProxy DeleteSysVpnConfig id=%{public}s", vpnId.c_str());
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
    if (ret != ERR_NONE) {
        NETMGR_EXT_LOG_E("DeleteSysVpnConfig proxy SendRequest failed, error code: [%{public}d]", ret);
        return ret;
    }
    if (!reply.ReadInt32(ret)) {
        NETMGR_EXT_LOG_E("reply ReadInt32 failed");
        return NETMANAGER_EXT_ERR_READ_REPLY_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnServiceProxy::GetSysVpnConfigList(std::vector<SysVpnConfig> &vpnList)
{
    NETMGR_EXT_LOG_I("NetworkVpnServiceProxy GetSysVpnConfigList");
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = WriteTokenAndSendRequest(INetworkVpnService::MessageCode::CMD_GET_SYS_VPN_CONFIG_LIST, data, reply);
    if (ret != ERR_NONE) {
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
            NETMGR_EXT_LOG_E("GetSysVpnConfigList failed, vpnConfig is null");
            return NETMANAGER_EXT_ERR_INTERNAL;
        }
        reply.ReadString(vpnConfig->vpnId_);
        reply.ReadString(vpnConfig->vpnName_);
        reply.ReadInt32(vpnConfig->vpnType_);
        vpnList.push_back(*vpnConfig);
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnServiceProxy::GetSysVpnConfig(sptr<SysVpnConfig> &config, const std::string &vpnId)
{
    if (vpnId.empty()) {
        NETMGR_EXT_LOG_E("GetSysVpnConfig failed, vpnId is empty");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    NETMGR_EXT_LOG_I("NetworkVpnServiceProxy GetSysVpnConfig id=%{public}s", vpnId.c_str());
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
    if (ret != ERR_NONE) {
        NETMGR_EXT_LOG_E("GetSysVpnConfig proxy SendRequest failed, error code: [%{public}d]", ret);
        return ret;
    }

    config = SysVpnConfig::Unmarshalling(reply);
    if (config == nullptr) {
        NETMGR_EXT_LOG_I("GetSysVpnConfig: vpn does not exist, id=%{public}s", vpnId.c_str());
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnServiceProxy::GetConnectedSysVpnConfig(sptr<SysVpnConfig> &config)
{
    NETMGR_EXT_LOG_I("NetworkVpnServiceProxy GetConnectedSysVpnConfig");
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = 0;
    ret = WriteTokenAndSendRequest(INetworkVpnService::MessageCode::CMD_GET_CONNECTED_SYS_VPN_CONFIG, data, reply);
    if (ret != ERR_NONE) {
        NETMGR_EXT_LOG_E("GetConnectedSysVpnConfig proxy SendRequest failed, error code: [%{public}d]", ret);
        return ret;
    }
    config = SysVpnConfig::Unmarshalling(reply);
    if (config == nullptr) {
        NETMGR_EXT_LOG_I("GetConnectedSysVpnConfig: no connected vpn");
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnServiceProxy::NotifyConnectStage(const std::string &stage, const int32_t &result)
{
    NETMGR_EXT_LOG_I("NotifyConnectStage stage=%{public}s result=%{public}d", stage.c_str(), result);
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
