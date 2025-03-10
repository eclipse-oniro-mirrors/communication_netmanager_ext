/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "state_utils.h"
#include <sys/time.h>
#include "netmgr_ext_log_wrapper.h"
#include "core_service_client.h"

namespace OHOS::NetManagerStandard {
constexpr int32_t SLOT_0 = 0;
constexpr int32_t SLOT_1 = 1;
constexpr int32_t UTIL_SLOT_0 = 0;
constexpr int32_t UTIL_SLOT_2 = 2;
constexpr int32_t MODEM_ID_0 = 0x00;
constexpr int32_t MODEM_ID_1 = 0x01;
constexpr int32_t TIME_RATE = 1000;
constexpr int32_t HTTP_DETECTION_TIMEOUT = 3000;

int64_t StateUtils::GetNowMilliSeconds()
{
    auto nowSys = AppExecFwk::InnerEvent::Clock::now();
    auto epoch = nowSys.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();
}

int64_t StateUtils::GetCurrentSysTimeMs()
{
    struct timeval time = {0};
    gettimeofday(&time, nullptr);
    return static_cast<int64_t>(
        static_cast<uint64_t>(time.tv_sec * TIME_RATE) + static_cast<uint64_t>(time.tv_usec / TIME_RATE));
}

int32_t StateUtils::GetDefaultSlotId()
{
    int32_t slotId = DelayedRefSingleton<Telephony::CellularDataClient>::GetInstance().GetDefaultCellularDataSlotId();
    return slotId;
}

int32_t StateUtils::GetPrimarySlotId()
{
    int32_t slotId = -1;
    int32_t ret = DelayedRefSingleton<Telephony::CoreServiceClient>::GetInstance().GetPrimarySlotId(slotId);
    if (ret != Telephony::TELEPHONY_SUCCESS) {
        NETMGR_EXT_LOG_E("get primary slotid fail, ret:%{public}d", ret);
        return -1;
    }
    return slotId;
}

int32_t StateUtils::GetSlaveCardSlotId()
{
    return (GetPrimarySlotId() == SLOT_0) ? SLOT_1 : SLOT_0;
}

bool StateUtils::IsValidSlotId(int32_t slotId)
{
    return (slotId >= UTIL_SLOT_0) && (slotId <= UTIL_SLOT_2);
}

bool StateUtils::IsRoaming(int32_t slotId)
{
    if (!StateUtils::IsValidSlotId(slotId)) {
        NETMGR_EXT_LOG_E("IsRoaming slotId is inValid");
        return false;
    }
    sptr<Telephony::NetworkState> networkState = nullptr;
    DelayedRefSingleton<Telephony::CoreServiceClient>::GetInstance().GetNetworkState(slotId, networkState);
    if (networkState != nullptr) {
        return networkState->IsRoaming();
    }
    return false;
}

int32_t StateUtils::GetModemIdBySlotId(int32_t slotId)
{
    int32_t masterCardSlotId = GetPrimarySlotId();
    if (!IsValidSlotId(masterCardSlotId)) {
        NETMGR_EXT_LOG_E("invalid masterCardSlotId: %{public}d", masterCardSlotId);
        masterCardSlotId = 0;
    }

    if (slotId == UTIL_SLOT_2) {
        return MODEM_ID_0;
    }

    if (masterCardSlotId == slotId) {
        return MODEM_ID_0;
    } else {
        return MODEM_ID_1;
    }
}

int32_t StateUtils::GetSlotIdByModemId(int32_t modemId)
{
    int32_t masterCardSlotId = GetPrimarySlotId();
    int32_t slaveCardSlotId = GetSlaveCardSlotId();

    if (MODEM_ID_0 == modemId) {
        return IsValidSlotId(masterCardSlotId) ? masterCardSlotId : UTIL_SLOT_0;
    } else {
        return IsValidSlotId(slaveCardSlotId) ? slaveCardSlotId : UTIL_SLOT_0;
    }
}
}  // namespace OHOS::NetManagerStandard
