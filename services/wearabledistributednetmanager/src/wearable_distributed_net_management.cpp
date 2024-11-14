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

#include "netmgr_ext_log_wrapper.h"
#include "wearable_distributed_net_management.h"

namespace OHOS {
namespace NetManagerStandard {
WearableDistributedNetManagement &WearableDistributedNetManagement::GetInstance()
{
    static WearableDistributedNetManagement instance;
    return instance;
}

int32_t WearableDistributedNetManagement::StartWearableDistributedNetwork(const int32_t tcpPortId,
                                                                          const int32_t udpPortId, const bool isMetered)
{
    NETMGR_EXT_LOG_I("Wearable Distributed Net Management Start Network");
    return WearableDistributedNetAgent::GetInstance().SetupWearableDistributedNetwork(tcpPortId, udpPortId, isMetered);
}

int32_t WearableDistributedNetManagement::StopWearableDistributedNetwork()
{
    NETMGR_EXT_LOG_I("Wearable Distributed Net Management Stop Network");
    return WearableDistributedNetAgent::GetInstance().TearDownWearableDistributedNetwork();
}

void WearableDistributedNetManagement::UpdateNetScore(const bool isCharging)
{
    WearableDistributedNetAgent::GetInstance().UpdateNetScore(isCharging);
}

int32_t WearableDistributedNetManagement::UpdateMeteredStatus(const bool isMetered)
{
    NETMGR_EXT_LOG_I("Update wearable distributed net metered status");
    return WearableDistributedNetAgent::GetInstance().UpdateMeteredStatus(isMetered);
}
} // namespace NetManagerStandard
} // namespace OHOS
