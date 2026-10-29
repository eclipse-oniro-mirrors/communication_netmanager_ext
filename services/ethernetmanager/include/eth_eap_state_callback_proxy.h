/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef ETH_EAP_STATE_CALLBACK_PROXY_H
#define ETH_EAP_STATE_CALLBACK_PROXY_H

#include "ieth_eap_state_callback.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace NetManagerStandard {

// 802.1X 认证状态回调 Proxy：服务端将状态信息序列化后经 Binder 发往客户端 Stub。
class EthEapStateCallbackProxy : public IRemoteProxy<IEthEapStateCallback> {
public:
    explicit EthEapStateCallbackProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IEthEapStateCallback>(impl) {}

    ~EthEapStateCallbackProxy() override = default;

    int32_t OnEthEapStateChanged(const EthEapStateInfo &stateInfo) override;

private:
    static constexpr uint32_t ON_ETH_EAP_STATE_CHANGED = 0;

    // static inline：否则不生成静态初始化，BrokerRegistration 缺本接口工厂，
    // 服务端 iface_cast<IEthEapStateCallback> 将返回 nullptr
    static inline BrokerDelegator<EthEapStateCallbackProxy> delegator_;
};

} // namespace NetManagerStandard
} // namespace OHOS
#endif // ETH_EAP_STATE_CALLBACK_PROXY_H
