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

#ifndef ETH_EAP_STATE_CALLBACK_STUB_H
#define ETH_EAP_STATE_CALLBACK_STUB_H

#include <map>

#include "ieth_eap_state_callback.h"
#include "iremote_stub.h"

namespace OHOS {
namespace NetManagerStandard {

// 802.1X 认证状态回调 Stub：校验接口描述符后按消息码分发到业务回调。
class EthEapStateCallbackStub : public IRemoteStub<IEthEapStateCallback> {
public:
    EthEapStateCallbackStub();

    ~EthEapStateCallbackStub() override = default;

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    using FuncPtr = int32_t (EthEapStateCallbackStub::*)(MessageParcel &, MessageParcel &);

    std::map<uint32_t, FuncPtr> memberFuncMap_;

    int32_t OnEthEapStateChangedInner(MessageParcel &data, MessageParcel &reply);
};

} // namespace NetManagerStandard
} // namespace OHOS
#endif // ETH_EAP_STATE_CALLBACK_STUB_H
