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

#ifndef WEARABLE_DISTRIBUTED_NET_CONFIG_CONSTANTS_H
#define WEARABLE_DISTRIBUTED_NET_CONFIG_CONSTANTS_H

#include <cstdint>
#include <cstdbool>
#include "netmgr_ext_log_wrapper.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {
class WearableDistributedNetConstant final {
    public:
        static constexpr bool AVALIABLE = true;
        static constexpr bool ROAMING_STATUS = false;
        static constexpr uint16_t WEARABLE_DISTRIBUTED_NET_MTU = 1500;
        static constexpr uint32_t LINKUP_BAND_WIDTH_KBPS = 220;
        static constexpr uint32_t LINKDOWN_BAND_WIDTH_KBPS = 220;
    };
} // namespace NetManagerStandard
} // namespace OHOS

using CONSTANTS = OHOS::NetManagerStandard::WearableDistributedNetConstant;

#endif // WEARABLE_DISTRIBUTED_NET_CONFIG_CONSTANTS_H