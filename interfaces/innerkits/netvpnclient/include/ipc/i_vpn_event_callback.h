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

#ifndef I_VPN_EVENT_CALLBACK_H
#define I_VPN_EVENT_CALLBACK_H

#include <iremote_broker.h>

#include "net_handle.h"
#include "net_manager_ext_constants.h"

namespace OHOS {
namespace NetManagerStandard {
class IVpnEventCallback : public IRemoteBroker {
public:
    virtual void OnVpnStateChanged(const bool &isConnected) = 0;
    virtual void OnVpnMultiUserSetUp() = 0;

    enum class Message {
        GLOBAL_VPN_STATE_CHANGED,
        GLOBAL_VPN_MULTI_USER_SETUP,
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.NetManagerStandard.INetworkVpnService.IVpnEventCallback");
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // I_VPN_EVENT_CALLBACK_H
