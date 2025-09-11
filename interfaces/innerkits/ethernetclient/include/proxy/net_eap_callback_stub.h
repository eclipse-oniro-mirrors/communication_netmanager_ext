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
 
#ifndef NET_EAP_CALLBACK_STUB_H
#define NET_EAP_CALLBACK_STUB_H
 
#include <map>
#include "iremote_stub.h"
#include "inet_register_eap_callback.h"
#include "inet_eap_postback_callback.h"
 
namespace OHOS {
namespace NetManagerStandard {
class NetEapPostbackCallbackStub : public IRemoteStub<INetEapPostbackCallback> {
public:
    using NetEapPostbackCallbackFunc = int32_t (NetEapPostbackCallbackStub::*)(MessageParcel &, MessageParcel &);
    NetEapPostbackCallbackStub();
    virtual ~NetEapPostbackCallbackStub();
 
    int32_t OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
 
    int32_t OnEapSupplicantPostback(NetType netType, const sptr<EapData> &eapData) override;
 
private:
    int32_t OnEapSupplicantResult(MessageParcel &data, MessageParcel &reply);
 
private:
    std::map<uint32_t, NetEapPostbackCallbackFunc> memberFuncMap_;
    std::mutex postbackMtx_;
};
 
class NetRegisterEapCallbackStub : public IRemoteStub<INetRegisterEapCallback> {
public:
    using NetRegisterEapCallbackFunc = int32_t (NetRegisterEapCallbackStub::*)(MessageParcel &, MessageParcel &);
    NetRegisterEapCallbackStub();
    virtual ~NetRegisterEapCallbackStub();
 
    int32_t OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
 
    int32_t OnRegisterCustomEapCallback(const std::string &regCmd) override;
    int32_t OnReplyCustomEapDataEvent(int result, const sptr<EapData> &eapData) override;
 
private:
    int32_t OnRegisterCustomEapCallback(MessageParcel &data, MessageParcel &reply);
    int32_t OnReplyCustomEapDataEvent(MessageParcel &data, MessageParcel &reply);
 
private:
    std::map<uint32_t, NetRegisterEapCallbackFunc> memberFuncMap_;
    std::mutex replyMtx_;
};
 
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_EAP_CALLBACK_STUB_H