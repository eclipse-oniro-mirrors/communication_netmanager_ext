/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef NET_ETHERNET_H
#define NET_ETHERNET_H

#include "net_ethernet_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get the ethernet mac address list.
 * @permission ohos.permission.GET_ETHERNET_LOCAL_MAC
 * @param macAddrList - The ethernet iface mac address list.
 * @return 0 - success.
 * 201 - Permission denied.
 * 2200001 - Invalid parameter value.
 * 2200002 - Operation failed. Cannot connect to service.
 * 2201005 - Device information does not exist.
 * @syscap SystemCapability.Communication.NetManager.Ethernet
 * @since 26.0.0
 */
int32_t OH_Ethernet_GetMacAddress(Ethernet_MacAddrInfoList *macAddrList);

/**
 * @brief Get the ethernet ip address list.
 * @permission ohos.permission.GET_NETWORK_INFO
 * @param netAddrList - The ethernet iface ip address list.
 * @return 0 - success.
 * 201 - Permission denied.
 * 2200001 - Invalid parameter value.
 * 2200002 - Operation failed. Cannot connect to service.
 * 2201005 - Device information does not exist.
 * @syscap SystemCapability.Communication.NetManager.Ethernet
 * @since 26.0.0
 */
int32_t OH_Ethernet_GetNetAddress(Ethernet_NetAddrList *netAddrList);

#ifdef __cplusplus
}
#endif

#endif // NET_ETHERNET_H
