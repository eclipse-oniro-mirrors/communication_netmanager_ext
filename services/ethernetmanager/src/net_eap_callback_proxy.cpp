/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
 
#include "net_eap_callback_proxy.h"
#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"
 
namespace OHOS {
namespace NetManagerStandard {
NetEapPostbackCallbackProxy::NetEapPostbackCallbackProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<INetEapPostbackCallback>(impl)
{}
 
NetEapPostbackCallbackProxy::~NetEapPostbackCallbackProxy() {}
 
int32_t NetEapPostbackCallbackProxy::OnEapSupplicantPostback(NetType netType, const sptr<EapData> &eapData)
{
    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        NETMGR_EXT_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
 
    if (eapData == nullptr) {
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
 
    if (!dataParcel.WriteInt32(static_cast<int32_t>(netType))) {
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
 
    if (!eapData->Marshalling(dataParcel)) {
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
 
    if (eapData->eapBuffer.size() == 0) {
        NETMGR_EXT_LOG_E("%{public}s, eapBuffer size is 0, %{public}s", __func__, eapData->PrintLogInfo().c_str());
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
 
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel replyParcel;
    MessageOption option;
    int32_t retCode = remote->SendRequest(static_cast<uint32_t>(NetEapIpcCode::NET_EAP_POSTBACK),
                                          dataParcel, replyParcel, option);
    if (retCode != ERR_NONE) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, retCode: [%{public}d]", retCode);
        return retCode;
    }
 
    return replyParcel.ReadInt32();
}
 
bool NetEapPostbackCallbackProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(NetEapPostbackCallbackProxy::GetDescriptor())) {
        NETMGR_EXT_LOG_E("WriteInterfaceToken failed");
        return false;
    }
    return true;
}
 
NetRegisterEapCallbackProxy::NetRegisterEapCallbackProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<INetRegisterEapCallback>(impl)
{}
 
NetRegisterEapCallbackProxy::~NetRegisterEapCallbackProxy() {}
 
int32_t NetRegisterEapCallbackProxy::OnRegisterCustomEapCallback(const std::string &regCmd)
{
    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        NETMGR_EXT_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }

    if (!dataParcel.WriteString(regCmd)) {
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel replyParcel;
    MessageOption option;
    int32_t retCode = remote->SendRequest(static_cast<uint32_t>(NetEapIpcCode::NET_REGISTER_CUSTOM_EAP_CALLBACK),
                                          dataParcel, replyParcel, option);
    if (retCode != ERR_NONE) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, retCode: [%{public}d]", retCode);
        return retCode;
    }
 
    return replyParcel.ReadInt32();
}
 
int32_t NetRegisterEapCallbackProxy::OnReplyCustomEapDataEvent(int result, const sptr<EapData> &eapData)
{
    MessageParcel dataParcel;
    if (eapData == nullptr) {
        NETMGR_EXT_LOG_E("%{public}s, eapData is nullptr", __func__);
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    if (!WriteInterfaceToken(dataParcel)) {
        NETMGR_EXT_LOG_E("%{public}s, WriteInterfaceToken failed, %{public}s", __func__,
            eapData->PrintLogInfo().c_str());
        return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
 
    if (!dataParcel.WriteInt32(result)) {
        NETMGR_EXT_LOG_E("%{public}s, WriteInt32 result failed, %{public}s", __func__, eapData->PrintLogInfo().c_str());
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
 
    if (!eapData->Marshalling(dataParcel)) {
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("%{public}s, remote is nullptr, %{public}s", __func__, eapData->PrintLogInfo().c_str());
        return NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel replyParcel;
    MessageOption option;
    int32_t retCode = remote->SendRequest(static_cast<uint32_t>(NetEapIpcCode::NET_REPLY_CUSTOM_EAPDATA),
                                          dataParcel, replyParcel, option);
    if (retCode != ERR_NONE) {
        NETMGR_EXT_LOG_E("%{public}s, SendRequest failed, retCode: [%{public}d]", __func__, retCode);
        return retCode;
    }
 
    return replyParcel.ReadInt32();
}
 
bool NetRegisterEapCallbackProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(NetRegisterEapCallbackProxy::GetDescriptor())) {
        NETMGR_EXT_LOG_E("WriteInterfaceToken failed");
        return false;
    }
    return true;
}
 
} // namespace NetManagerStandard
} // namespace OHOS