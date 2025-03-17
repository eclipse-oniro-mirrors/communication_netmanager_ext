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

#ifndef WEARABLE_DISTRIBUTED_NET_MANAGEMENT_H
#define WEARABLE_DISTRIBUTED_NET_MANAGEMENT_H

#include <cstdint>
#include "wearable_distributed_net_agent.h"

namespace OHOS {
namespace NetManagerStandard {
class WearableDistributedNetManagement {
public:
    static WearableDistributedNetManagement &GetInstance();
    int32_t SetupWearableDistributedNetwork(const int32_t tcpPortId, const int32_t udpPortId, const bool isMetered);
    int32_t EnableWearableDistributedNetwork(bool enableFlag);
    int32_t StopWearableDistributedNetwork();
    void UpdateNetScore(const bool isCharging);
    int32_t UpdateMeteredStatus(const bool isMetered);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // WEARABLE_DISTRIBUTED_NET_MANAGEMENT_H
