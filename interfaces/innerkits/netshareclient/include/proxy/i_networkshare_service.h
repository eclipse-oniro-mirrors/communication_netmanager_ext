/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef I_NETSHARE_SERVICE_H
#define I_NETSHARE_SERVICE_H

#include <string>
#include <vector>

#include "iremote_broker.h"
#include "iremote_object.h"

#include "net_manager_ext_constants.h"
#include "i_netshare_result_callback.h"
#include "i_sharing_event_callback.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t GET_CFG_SUC = 1;
class INetworkShareService : public IRemoteBroker {
public:
    enum class MessageCode {
        CMD_GET_SHARING_SUPPORTED,
        CMD_GET_IS_SHARING,
        CMD_START_NETWORKSHARE,
        CMD_STOP_NETWORKSHARE,
        CMD_GET_SHARABLE_REGEXS,
        CMD_GET_SHARING_STATE,
        CMD_GET_SHARING_IFACES,
        CMD_REGISTER_EVENT_CALLBACK,
        CMD_UNREGISTER_EVENT_CALLBACK,
        CMD_GET_ACTIVATE_INTERFACE,
        CMD_GET_RX_BYTES,
        CMD_GET_TX_BYTES,
        CMD_GET_TOTAL_BYTES,
    };

public:
    virtual int32_t IsNetworkSharingSupported() = 0;
    virtual int32_t IsSharing() = 0;
    virtual int32_t StartNetworkSharing(const SharingIfaceType &type) = 0;
    virtual int32_t StopNetworkSharing(const SharingIfaceType &type) = 0;
    virtual std::vector<std::string> GetSharableRegexs(SharingIfaceType type) = 0;
    virtual int32_t GetSharingState(SharingIfaceType type, SharingIfaceState &state) = 0;
    virtual std::vector<std::string> GetNetSharingIfaces(const SharingIfaceState &state) = 0;
    virtual int32_t RegisterSharingEvent(sptr<ISharingEventCallback> callback) = 0;
    virtual int32_t UnregisterSharingEvent(sptr<ISharingEventCallback> callback) = 0;
    virtual int32_t GetStatsRxBytes() = 0;
    virtual int32_t GetStatsTxBytes() = 0;
    virtual int32_t GetStatsTotalBytes() = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.NetManagerStandard.INetworkShareService");
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // I_NETSHARE_SERVICE_H
