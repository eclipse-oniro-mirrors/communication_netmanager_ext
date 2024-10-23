/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef WEARABLE_DISTRIBUTED_NET_STATIC_CONFIGURATION_H
#define WEARABLE_DISTRIBUTED_NET_STATIC_CONFIGURATION_H

#include "net_supplier_info.h"
#include "net_all_capabilities.h"
#include "wearable_distributed_net_config_forward.h"
#include "wearable_distributed_net_supplier_info.h"
#include "wearable_distributed_net_link_info.h"

namespace OHOS {
namespace NetManagerStandard {
class WearableDistributedNetStaticConfiguration final {
public:
    WearableDistributedNetStaticConfiguration();
    ~WearableDistributedNetStaticConfiguration();

    int32_t GetNetLinkInfo(NetLinkInfo &linkInfo);
    void GetNetSupplierInfo(NetSupplierInfo &supplierInfo);
    std::set<NetCap> GetNetCaps(const bool isMetered);
    int32_t EnableWearableDistributedNetForward(const int32_t tcpPortId, const int32_t udpPortId);
    int32_t DisableWearableDistributedNetForward();
private:
    int32_t SetNetLinkInfo(NetLinkInfo &linkInfo);
    void SetNetMetered(const bool isMetered);
private:
    WearableDistributedNetForward wearableDistributedNetForward_;
    std::set<NetCap> netCaps_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // WEARABLE_DISTRIBUTED_NET_STATIC_CONFIGURATION_H
