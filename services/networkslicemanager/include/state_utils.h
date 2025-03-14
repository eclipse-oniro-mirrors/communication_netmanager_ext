/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_TELEPHONYEXT_STATE_UTILS_H
#define OHOS_TELEPHONYEXT_STATE_UTILS_H

#include <list>
#include <string>
#include "cellular_data_client.h"
#include "event_handler.h"
#include "signal_information.h"
#include "network_search_types.h"
#include "core_manager_inner.h"

namespace OHOS::NetManagerStandard {
static constexpr int32_t DEFAULT_SLOT = 0;
class StateUtils final {
public:
    StateUtils() = default;
    ~StateUtils() = default;

    static int64_t GetNowMilliSeconds();
    static int64_t GetCurrentSysTimeMs();
    static int32_t GetDefaultSlotId();
    static int32_t GetPrimarySlotId();
    static int32_t GetSlaveCardSlotId();
    static bool IsValidSlotId(int32_t slotId);
    static bool IsRoaming(int32_t slotId);
    static int32_t GetModemIdBySlotId(int32_t slotId);
    static int32_t GetSlotIdByModemId(int32_t modemId);
};
}
#endif
