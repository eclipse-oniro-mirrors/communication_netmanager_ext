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

#ifndef NET_ETHERNET_TYPE_H
#define NET_ETHERNET_TYPE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ETHERNET_MAX_NET_SIZE 32
#define ETHERNET_MAX_STR_LEN 256

typedef struct Ethernet_MacAddressInfo {
    char ifaceName[ETHERNET_MAX_STR_LEN];
    char macAddr[ETHERNET_MAX_STR_LEN];
} Ethernet_MacAddressInfo;

typedef struct Ethernet_MacAddrInfoList {
    Ethernet_MacAddressInfo macInfoList[ETHERNET_MAX_NET_SIZE];
    int32_t macInfoListSize;
} Ethernet_MacAddrInfoList;

typedef struct Ethernet_NetAddr {
    uint8_t family;
    uint8_t prefixlen;
    uint16_t port;
    char address[ETHERNET_MAX_STR_LEN];
} Ethernet_NetAddr;

typedef struct Ethernet_NetAddrInfo {
    char ifaceName[ETHERNET_MAX_STR_LEN];
    Ethernet_NetAddr netAddrInfo[ETHERNET_MAX_NET_SIZE];
    int32_t netAddrInfoSize;
} Ethernet_NetAddrInfo;

typedef struct Ethernet_NetAddrList {
    Ethernet_NetAddrInfo netAddrList[ETHERNET_MAX_NET_SIZE];
    int32_t netAddrListSize;
} Ethernet_NetAddrList;

#ifdef __cplusplus
}
#endif

#endif // NET_ETHERNET_TYPE_H
