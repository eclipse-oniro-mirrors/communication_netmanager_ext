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
#include "net_eap_callback_stub.h"
#include "netmgr_ext_log_wrapper.h"
 
namespace OHOS {
namespace NetManagerStandard {
NetEapPostbackCallbackStub::~NetEapPostbackCallbackStub()
{
}
 
NetEapPostbackCallbackStub::NetEapPostbackCallbackStub()
{
    memberFuncMap_[static_cast<uint32_t>(NetEapIpcCode::NET_EAP_POSTBACK)] =
        &NetEapPostbackCallbackStub::OnEapSupplicantResult;
}
 
int32_t NetEapPostbackCallbackStub::OnEapSupplicantResult(MessageParcel &data, MessageParcel &reply)
{
    std::unique_lock<std::mutex> lock(postbackMtx_);
    int32_t netType = 0;
    if (!data.ReadInt32(netType)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
 
    sptr<EapData> eapData =  EapData::Unmarshalling(data);
    if (eapData == nullptr) {
        NETMGR_EXT_LOG_E("%{public}s, failed new eapData", __func__);
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
 
    if (eapData->eapBuffer.size() == 0) {
        NETMGR_EXT_LOG_E("%{public}s, eapBuffer size is 0, %{public}s", __func__, eapData->PrintLogInfo().c_str());
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
 
    int32_t ret = OnEapSupplicantPostback(static_cast<NetType>(netType), eapData);
    if (!reply.WriteInt32(ret)) {
        NETMGR_EXT_LOG_E("Write parcel failed");
        return ret;
    }
 
    return NETMANAGER_SUCCESS;
}
 
int32_t NetEapPostbackCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    NETMGR_EXT_LOG_D("Stub call start, code:[%{public}d]", code);
    std::u16string myDescripter = NetEapPostbackCallbackStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (myDescripter != remoteDescripter) {
        NETMGR_EXT_LOG_E("Descriptor checked failed");
        return NETMANAGER_ERR_DESCRIPTOR_MISMATCH;
    }
 
    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto requestFunc = itFunc->second;
        if (requestFunc != nullptr) {
            return (this->*requestFunc)(data, reply);
        }
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
 
int32_t NetEapPostbackCallbackStub::OnEapSupplicantPostback(NetType netType, const sptr<EapData> &eapData)
{
    return true;
}
 
NetRegisterEapCallbackStub::NetRegisterEapCallbackStub()
{
    memberFuncMap_[static_cast<uint32_t>(NetEapIpcCode::NET_REGISTER_CUSTOM_EAP_CALLBACK)] =
        &NetRegisterEapCallbackStub::OnRegisterCustomEapCallback;
    memberFuncMap_[static_cast<uint32_t>(NetEapIpcCode::NET_REPLY_CUSTOM_EAPDATA)] =
        &NetRegisterEapCallbackStub::OnReplyCustomEapDataEvent;
}
 
NetRegisterEapCallbackStub::~NetRegisterEapCallbackStub()
{
}
 
int32_t NetRegisterEapCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    NETMGR_EXT_LOG_D("Stub call start, code:[%{public}d]", code);
    std::u16string myDescripter = NetRegisterEapCallbackStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (myDescripter != remoteDescripter) {
        NETMGR_EXT_LOG_E("Descriptor checked failed");
        return NETMANAGER_ERR_DESCRIPTOR_MISMATCH;
    }
 
    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto requestFunc = itFunc->second;
        if (requestFunc != nullptr) {
            return (this->*requestFunc)(data, reply);
        }
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
 
int32_t NetRegisterEapCallbackStub::OnRegisterCustomEapCallback(MessageParcel &data, MessageParcel &reply)
{
    std::string regCmd;
    if (!data.ReadString(regCmd)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
 
    int32_t ret = OnRegisterCustomEapCallback(regCmd);
    if (!reply.WriteInt32(ret)) {
        NETMGR_EXT_LOG_E("Write parcel failed");
        return ret;
    }
 
    return NETMANAGER_SUCCESS;
}
 
int32_t NetRegisterEapCallbackStub::OnReplyCustomEapDataEvent(MessageParcel &data, MessageParcel &reply)
{
    std::unique_lock<std::mutex> lock(replyMtx_);
    int32_t result = -1;
    if (!data.ReadInt32(result)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
 
    sptr<EapData> eapData =  EapData::Unmarshalling(data);
    if (eapData == nullptr) {
        NETMGR_EXT_LOG_E("%{public}s, eapdata is nullptr", __func__);
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
 
    int32_t ret = OnReplyCustomEapDataEvent(result, eapData);
    if (!reply.WriteInt32(ret)) {
        NETMGR_EXT_LOG_E("Write parcel failed");
        return ret;
    }
 
    return NETMANAGER_SUCCESS;
}
 
int32_t NetRegisterEapCallbackStub::OnRegisterCustomEapCallback(const std::string &regCmd)
{
    return NETMANAGER_SUCCESS;
}
 
int32_t NetRegisterEapCallbackStub::OnReplyCustomEapDataEvent(int result, const sptr<EapData> &eapData)
{
    return NETMANAGER_SUCCESS;
}
 
} // namespace NetManagerStandard
} // namespace OHOS