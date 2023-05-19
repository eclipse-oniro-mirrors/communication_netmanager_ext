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

#ifndef I_NETWORK_VPN_SERVICE_H
#define I_NETWORK_VPN_SERVICE_H

#include <string>
#include <vector>

#include "iremote_broker.h"
#include "iremote_object.h"

#include "i_vpn_event_callback.h"
#include "net_manager_ext_constants.h"
#include "vpn_config.h"

namespace OHOS {
namespace NetManagerStandard {
class INetworkVpnService : public IRemoteBroker {
public:
    enum class MessageCode {
        CMD_START_INTERNAL_VPN, // for start internal vpn
        CMD_PREPARE,
        CMD_START_SETUP, // for start extended vpn
        CMD_STOP_VPN,
        CMD_REGISTER_EVENT_CALLBACK,
        CMD_UNREGISTER_EVENT_CALLBACK,
    };

public:
    virtual int32_t Prepare(bool &isExistVpn, bool &isRun, std::string &pkg) = 0;
    virtual int32_t SetUp(const sptr<VpnConfig> &config) = 0;
    virtual int32_t DestroyVpn() = 0;
    virtual int32_t RegisterVpnEvent(const sptr<IVpnEventCallback> callback) = 0;
    virtual int32_t UnregisterVpnEvent(const sptr<IVpnEventCallback> callback) = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.NetManagerStandard.INetworkVpnService");
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // I_NETWORK_VPN_SERVICE_H
