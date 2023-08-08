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

#ifndef VPN_EVENT_CALLBACK_PROXY_H
#define VPN_EVENT_CALLBACK_PROXY_H

#include "i_vpn_event_callback.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace NetManagerStandard {
class VpnEventCallbackProxy : public IRemoteProxy<IVpnEventCallback> {
public:
    explicit VpnEventCallbackProxy(const sptr<IRemoteObject> &object);
    virtual ~VpnEventCallbackProxy() = default;

    virtual void OnVpnStateChanged(const bool &isConnected) override;
    virtual void OnVpnMultiUserSetUp() override;

private:
    static inline BrokerDelegator<VpnEventCallbackProxy> delegator_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // VPN_EVENT_CALLBACK_PROXY_H
