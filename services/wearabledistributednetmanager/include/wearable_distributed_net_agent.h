/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef WEARABLE_DISTRIBUTED_NET_AGENT_H
#define WEARABLE_DISTRIBUTED_NET_AGENT_H

#include <cstdint>
#include "battery_srv_client.h"
#include "net_all_capabilities.h"
#include "net_conn_client.h"
#include "net_link_info.h"
#include "net_manager_ext_constants.h"
#include "net_supplier_info.h"
#include "wearable_distributed_net_static_configuration.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t NET_SCORE_WITH_CHARGE_STATE = 50;
constexpr int32_t NET_SCORE_WITH_UNCHARGE_STATE = 80;
namespace {
    using ChargeState = OHOS::PowerMgr::BatteryChargeState;
}
class WearableDistributedNetAgent : public std::enable_shared_from_this<WearableDistributedNetAgent> {
public:
    static WearableDistributedNetAgent &GetInstance();
    int32_t SetupWearableDistributedNetwork(const int32_t tcpPortId, const int32_t udpPortId, const bool isMetered);
    int32_t EnableWearableDistributedNetwork(bool enableFlag);
    int32_t TearDownWearableDistributedNetwork();
    int32_t UpdateNetScore(const bool isCharging);
    int32_t UpdateMeteredStatus(const bool isMetered);

private:
    int32_t RegisterNetSupplier(const bool isMetered);
    int32_t UnregisterNetSupplier();
    int32_t UpdateNetSupplierInfo(const bool isAvailable);
    int32_t UpdateNetLinkInfo();
    void ObtainNetCaps(const bool isMetered);
    void SetNetSupplierInfo(NetSupplierInfo &networkSupplierInfo);
    int32_t SetNetLinkInfo(NetLinkInfo &networkLinkInfo);
    int32_t DisableWearableDistributedNetForward();
    int32_t EnableWearableDistributedNetForward(const int32_t tcpPortId, const int32_t udpPortId);
    int32_t ClearWearableDistributedNetForwardConfig();
    void SetInitNetScore(OHOS::PowerMgr::BatteryChargeState chargeState);
    void SetScoreBaseNetStatus(const bool isAvailable);
    void SetScoreBaseChargeStatus(const bool isCharging);
    int32_t UpdateNetCaps(const bool isMetered);

private:
    uint32_t netSupplierId_ = 0;
    std::set<NetCap> netCaps_;
    WearableDistributedNetStaticConfiguration staticConfiguration_;
    NetSupplierInfo netSupplierInfo_;
    NetLinkInfo netLinkInfo_;
    bool isMetered_ = false;
    int32_t score_ = NET_SCORE_WITH_UNCHARGE_STATE;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // WEARABLE_DISTRIBUTED_NET_MANAGEMENT_H
