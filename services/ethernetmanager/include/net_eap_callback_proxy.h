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
 
#ifndef NET_EAP_CALLBACK_PROXY_H
#define NET_EAP_CALLBACK_PROXY_H
 
#include "iremote_proxy.h"
#include "inet_register_eap_callback.h"
#include "inet_eap_postback_callback.h"
 
namespace OHOS {
namespace NetManagerStandard {
class NetEapPostbackCallbackProxy : public IRemoteProxy<INetEapPostbackCallback> {
public:
    explicit NetEapPostbackCallbackProxy(const sptr<IRemoteObject> &impl);
    virtual ~NetEapPostbackCallbackProxy();
 
public:
    int32_t OnEapSupplicantPostback(NetType netType, const sptr<EapData> &eapData) override;
 
private:
    bool WriteInterfaceToken(MessageParcel &data);
 
private:
    static inline BrokerDelegator<NetEapPostbackCallbackProxy> delegator_;
};
 
class NetRegisterEapCallbackProxy : public IRemoteProxy<INetRegisterEapCallback> {
public:
    explicit NetRegisterEapCallbackProxy(const sptr<IRemoteObject> &impl);
    virtual ~NetRegisterEapCallbackProxy();
 
public:
    int32_t OnRegisterCustomEapCallback(const std::string &regCmd) override;
    int32_t OnReplyCustomEapDataEvent(int result, const sptr<EapData> &eapData) override;
 
private:
    bool WriteInterfaceToken(MessageParcel &data);
 
private:
    static inline BrokerDelegator<NetRegisterEapCallbackProxy> delegator_;
};
 
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_EAP_CALLBACK_PROXY_H