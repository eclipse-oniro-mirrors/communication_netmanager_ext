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

#ifndef NETWORKSLICE_EVENT_H
#define NETWORKSLICE_EVENT_H
#include <stdint.h>
#include <string>
namespace OHOS {
namespace NetManagerStandard {
enum NetworkSliceEvent {
    EVENT_NETWORK_PARA_FORBIDDEN_TIMEOUT,
    EVENT_GET_SLICE_PARA,
    EVENT_HANDLE_ALLOWED_NSSAI,
    EVENT_HANDLE_UE_POLICY,
    EVENT_HANDLE_NETWORK_ACTIVATE_RESULT,
    EVENT_HANDLE_SIM_STATE_CHANGED,
    EVENT_KERNEL_IP_ADDR_REPORT,
    EVENT_INIT_UE_POLICY,
    EVENT_HANDLE_EHPLMN,
    EVENT_SYSTEM_WIFI_NETWORK_STATE_CHANGED,
    EVENT_BIND_TO_NETWORK,
    EVENT_DEL_BIND_TO_NETWORK,
    EVENT_FOREGROUND_APP_CHANGED,
    EVENT_AIR_MODE_CHANGED,
    EVENT_NETWORK_STATE_CHANGED,
    EVENT_WIFI_CONN_CHANGED,
    EVENT_VPN_MODE_CHANGED,
    EVENT_CONNECTIVITY_CHANGE,
    EVENT_FQDN_UPDATED,
    EVENT_SCREEN_ON,
    EVENT_SCREEN_OFF,
    EVENT_URSP_CHANGED,
};
}
}
#endif
