/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef NETMANAGER_EXT_CONSTANTS_H
#define NETMANAGER_EXT_CONSTANTS_H

#include <errors.h>

namespace OHOS {
namespace NetManagerStandard {
constexpr int NETMANAGER_EXT_ERROR = -1;
constexpr int NETMANAGER_EXT_SUCCESS = 0;

enum {
    NETMANAGER_EXT_COMMON = 0x00,
    EXTERNALVPN_MANAGER = 0x01,
    NETMANAGER_ETHERNET_MANAGER = 0x02,
};

// Error code for common
constexpr ErrCode COMMON_EXT_ERR_OFFSET = ErrCodeOffset(SUBSYS_COMMUNICATION, NETMANAGER_EXT_COMMON);

enum {
    NETMANAGER_EXT_ERR_FAIL = COMMON_EXT_ERR_OFFSET,
    NETMANAGER_EXT_ERR_MEMCPY_FAIL,
    NETMANAGER_EXT_ERR_MEMSET_FAIL,
    NETMANAGER_EXT_ERR_STRCPY_FAIL,
    NETMANAGER_EXT_ERR_STRING_EMPTY,
    NETMANAGER_EXT_ERR_LOCAL_PTR_NULL,
    NETMANAGER_EXT_ERR_PERMISSION_ERR,
    NETMANAGER_EXT_ERR_DESCRIPTOR_MISMATCH,
    NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL,
    NETMANAGER_EXT_ERR_WRITE_DATA_FAIL,
    NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL,
    NETMANAGER_EXT_ERR_READ_DATA_FAIL,
    NETMANAGER_EXT_ERR_READ_REPLY_FAIL,
    NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL,
    NETMANAGER_EXT_ERR_ADD_DEATH_RECIPIENT_FAIL,
    NETMANAGER_EXT_ERR_REGISTER_CALLBACK_FAIL,
    NETMANAGER_EXT_ERR_UNINIT,
};
// Error code for netmanager ethernet
constexpr ErrCode ETHERNET_ERR_OFFSET = ErrCodeOffset(SUBSYS_COMMUNICATION, NETMANAGER_ETHERNET_MANAGER);
// Error code for netmanager external vpn
constexpr ErrCode EXTERNALVPN_ERR_OFFSET = ErrCodeOffset(SUBSYS_COMMUNICATION, EXTERNALVPN_MANAGER);

// for network sharing
enum class SharingIfaceType {
    SHARING_WIFI = 0,
    SHARING_USB,
    SHARING_BLUETOOTH,
};
enum class SharingIfaceState {
    SHARING_NIC_SERVING = 1,
    SHARING_NIC_CAN_SERVER,
    SHARING_NIC_ERROR,
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETMANAGER_EXT_CONSTANTS_H
