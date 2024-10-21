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
#include "net_manager_constants.h"
#include "wearable_distributed_net_config_forward.h"

namespace OHOS {
namespace NetManagerStandard {
static constexpr int32_t MAX_PORT_ID = 65535;

int32_t WearableDistributedNetForward::EnableWearableDistributedNetForward(const int32_t tcpPortId,
                                                                           const int32_t udpPortId)
{
    NETMGR_EXT_LOG_I("Wearable Distributed Net Enable Forward");
    if (tcpPortId <= 0 || tcpPortId > MAX_PORT_ID) {
        NETMGR_EXT_LOG_E("Invalid TCP port ID");
        return NETMANAGER_EXT_ERR_INVALID_PARAMETER;
    }
    if (udpPortId <= 0 || udpPortId > MAX_PORT_ID) {
        NETMGR_EXT_LOG_E("Invalid UDP port ID");
        return NETMANAGER_EXT_ERR_INVALID_PARAMETER;
    }
    return NetsysController::GetInstance().EnableWearableDistributedNetForward(tcpPortId, udpPortId);
}

int32_t WearableDistributedNetForward::DisableWearableDistributedNetForward()
{
    NETMGR_EXT_LOG_I("Wearable Distributed Net Disable Forward");
    return NetsysController::GetInstance().DisableWearableDistributedNetForward();
}
} // namespace NetManagerStandard
}  // namespace OHOS
