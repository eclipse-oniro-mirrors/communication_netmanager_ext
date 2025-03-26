/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "networkslice_proxy.h"
#include "hilog/log.h"
#include "iremote_object.h"
#include "message_option.h"
#include "message_parcel.h"
#include "net_manager_constants.h"
#include "net_manager_ext_constants.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t BUFFER_MAX = 65538;
NetworkSliceProxy::NetworkSliceProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<INetworkSliceService>(impl) {}

NetworkSliceProxy::~NetworkSliceProxy() = default;

int32_t NetworkSliceProxy::SetNetworkSliceUePolicy(std::vector<uint8_t> buffer)
{
    NETMGR_EXT_LOG_I("SetNetworkSliceUePolicy");
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        NETMGR_EXT_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    int32_t buffersize = (int)buffer.size();
    NETMGR_EXT_LOG_I("Proxy::SetNetworkSliceUePolicy:buffersize = %{public}d", buffersize);
    if (buffersize <= 0 || buffersize > BUFFER_MAX) {
        NETMGR_EXT_LOG_E("buffer length is invalid: %{public}d", buffersize);
        return NETMANAGER_EXT_ERR_INVALID_PARAMETER;
    }
    data.WriteInt32(buffersize);
    for (int i = 0; i < (int)buffer.size(); ++i) {
        data.WriteUint8(buffer[i]);
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(NetworkSliceInterfaceCode::SET_NETWORKSLICE_UEPOLICY),
        data, reply, option);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
    }
    return ret;
}

int32_t NetworkSliceProxy::NetworkSliceAllowedNssaiRpt(std::vector<uint8_t> buffer)
{
    NETMGR_EXT_LOG_I("NetworkSliceAllowedNssaiRpt");
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        NETMGR_EXT_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    int32_t buffersize = (int)buffer.size();
    if (buffersize <= 0 || buffersize > BUFFER_MAX) {
        NETMGR_EXT_LOG_E("buffer length is invalid: %{public}d", buffersize);
        return NETMANAGER_EXT_ERR_INVALID_PARAMETER;
    }
    data.WriteInt32(buffersize);
    for (int i = 0; i < (int)buffer.size(); ++i) {
        data.WriteUint8(buffer[i]);
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(NetworkSliceInterfaceCode::NETWORKSLICE_ALLOWEDNSSAI_RPT),
        data, reply, option);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
    }
    return ret;
}

int32_t NetworkSliceProxy::NetworkSliceEhplmnRpt(std::vector<uint8_t> buffer)
{
    NETMGR_EXT_LOG_I("NetworkSliceEhplmnRpt");
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        NETMGR_EXT_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    int32_t buffersize = (int)buffer.size();
    if (buffersize <= 0 || buffersize > BUFFER_MAX) {
        NETMGR_EXT_LOG_E("buffer length is invalid: %{public}d", buffersize);
        return NETMANAGER_EXT_ERR_INVALID_PARAMETER;
    }
    data.WriteInt32(buffersize);
    for (int i = 0; i < (int)buffer.size(); ++i) {
        data.WriteUint8(buffer[i]);
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(NetworkSliceInterfaceCode::NETWORKSLICE_EHPLMN_RPT),
        data, reply, option);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
    }
    return ret;
}

int32_t NetworkSliceProxy::NetworkSliceInitUePolicy()
{
    NETMGR_EXT_LOG_I("NetworkSliceInitUePolicy");
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        NETMGR_EXT_LOG_E("InitUePolicy WriteInterfaceToken failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("InitUePolicy Remote is null");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(NetworkSliceInterfaceCode::NETWORKSLICE_INIT_UEPOLICY),
        data, reply, option);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
    }
    return ret;
}

int32_t NetworkSliceProxy::GetRouteSelectionDescriptorByDNN(std::string dnn, std::string& snssai, uint8_t& sscMode)
{
    NETMGR_EXT_LOG_I("GetRouteSelectionDescriptorByDNN");
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        NETMGR_EXT_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    data.WriteString(dnn);
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(NetworkSliceInterfaceCode::NETWORKSLICE_GETRSDBYDNN),
        data, reply, option);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
    }
    snssai = reply.ReadString();
    sscMode = reply.ReadUint8();
    return ret;
}

int32_t NetworkSliceProxy::GetRSDByNetCap(int32_t netcap, std::map<std::string, std::string>& networkSliceParas)
{
    NETMGR_EXT_LOG_I("GetRSDByNetCap");
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        NETMGR_EXT_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    data.WriteInt32(netcap);
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(NetworkSliceInterfaceCode::NETWORKSLICE_GETRSDBYNETCAP),
        data, reply, option);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
    }
    networkSliceParas["dnn"] = reply.ReadString();
    networkSliceParas["snssai"] = reply.ReadString();
    networkSliceParas["sscmode"] = reply.ReadString();
    networkSliceParas["pdusessiontype"] = reply.ReadString();
    networkSliceParas["routebitmap"] = reply.ReadString();
    if (networkSliceParas["dnn"] == "Invalid") {
        networkSliceParas["dnn"] = "";
    }
    if (networkSliceParas["snssai"] == "Invalid") {
        networkSliceParas["snssai"] = "";
    }
    return ret;
}

int32_t NetworkSliceProxy::SetSaState(bool isSaState)
{
    NETMGR_EXT_LOG_I("NetworkSliceProxy SetSaState");
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        NETMGR_EXT_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    data.WriteBool(isSaState);
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(NetworkSliceInterfaceCode::NETWORKSLICE_SETSASTATE),
        data, reply, option);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
    }
    return ret;
}
} // namespace NetManagerStandard
} // namespace OHOS
