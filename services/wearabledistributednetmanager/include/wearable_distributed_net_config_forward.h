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

#ifndef WEARABLE_DISTRIBUTED_NET_CONFIG_FORWARD_H
#define WEARABLE_DISTRIBUTED_NET_CONFIG_FORWARD_H

#include "netsys_controller.h"

namespace OHOS {
namespace NetManagerStandard {
class WearableDistributedNetForward {
public:
    /**
     * @brief Enable wearable distributed net forward
     *
     * @param tcpPortId TCP port ID
     * @param udpPortId UDP port ID
     * @return Return NETMANAGER_EXT_SUCCESS if process normal, others is error
     */
    int32_t EnableWearableDistributedNetForward(const int32_t tcpPortId, const int32_t udpPortId);

    /**
     * @brief Disable wearable distributed net forward
     *
     * @return Return NETMANAGER_EXT_SUCCESS if process normal, others is error
     */
    int32_t DisableWearableDistributedNetForward();
};
} // namespace NetManagerStandard
} // namespace ohos
#endif  // WEARABLE_DISTRIBUTED_NET_CONFIG_FORWARD_H
