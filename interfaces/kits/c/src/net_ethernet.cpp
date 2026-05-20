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

#include "net_ethernet.h"

#include <cstring>
#include <string>
#include <vector>

#include "ethernet_client.h"
#include "mac_address_info.h"
#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "net_conn_client.h"
#include "refbase.h"

using namespace OHOS;
using namespace OHOS::NetManagerStandard;
constexpr int32_t ETHERNET_ERR_INVALID_PARAMETER = 2200001;
constexpr int32_t ETHERNET_ERR_CONNECTION_FAILED = 2200002;

// LCOV_EXCL_START
void CopyStrToBuffer(char *dst, const std::string &src, size_t dstSize)
{
    if (dst == nullptr || dstSize == 0) {
        return;
    }
    size_t copyLen = src.size() < (dstSize - 1) ? src.size() : (dstSize - 1);
    if (copyLen > 0) {
        errno_t ret = memcpy_s(dst, dstSize - 1, src.c_str(), copyLen);
        if (ret != EOK) {
            NETMGR_EXT_LOG_E("memcpy_s failed");
            dst[0] = '\0';
            return;
        }
    }
    dst[copyLen] = '\0';
}
// LCOV_EXCL_STOP

int32_t ConvertErrorCode(int32_t innerCode)
{
    if (innerCode == NETMANAGER_ERR_PERMISSION_DENIED) {
        return innerCode;
    }
    // LCOV_EXCL_START
    if (innerCode == ETHERNET_ERR_DEVICE_INFORMATION_NOT_EXIST) {
        return innerCode;
    }
    // LCOV_EXCL_STOP
    return ETHERNET_ERR_CONNECTION_FAILED;
}

extern "C" {
int32_t OH_Ethernet_GetMacAddress(Ethernet_MacAddrInfoList *macAddrList)
{
    if (macAddrList == nullptr) {
        NETMGR_EXT_LOG_E("macAddrList is nullptr");
        return ETHERNET_ERR_INVALID_PARAMETER;
    }

    std::vector<MacAddressInfo> macList;
    int32_t ret = OHOS::DelayedSingleton<EthernetClient>::GetInstance()->GetMacAddress(macList);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("GetMacAddress failed, ret=%{public}d", ret);
        return ConvertErrorCode(ret);
    }

    // LCOV_EXCL_START
    macAddrList->macInfoListSize = 0;
    for (const auto &info : macList) {
        if (macAddrList->macInfoListSize >= ETHERNET_MAX_NET_SIZE) {
            break;
        }
        int32_t idx = macAddrList->macInfoListSize;
        CopyStrToBuffer(macAddrList->macInfoList[idx].ifaceName, info.iface_, ETHERNET_MAX_STR_LEN);
        CopyStrToBuffer(macAddrList->macInfoList[idx].macAddr, info.macAddress_, ETHERNET_MAX_STR_LEN);
        macAddrList->macInfoListSize++;
    }

    return 0;
    // LCOV_EXCL_STOP
}

// LCOV_EXCL_START
void FillNetAddrListFromEthernet(Ethernet_NetAddrList* netAddrList, const NetLinkInfo& info) {
    if (netAddrList->netAddrListSize >= ETHERNET_MAX_NET_SIZE) {
        return;
    }
    
    int32_t idx = netAddrList->netAddrListSize;
    CopyStrToBuffer(netAddrList->netAddrList[idx].ifaceName, info.ifaceName_, ETHERNET_MAX_STR_LEN);
    netAddrList->netAddrList[idx].netAddrInfoSize = 0;
    
    for (const auto& addr : info.netAddrList_) {
        if (netAddrList->netAddrList[idx].netAddrInfoSize >= ETHERNET_MAX_NET_SIZE) {
            break;
        }
        int32_t addrIdx = netAddrList->netAddrList[idx].netAddrInfoSize;
        netAddrList->netAddrList[idx].netAddrInfo[addrIdx].family = addr.family_;
        netAddrList->netAddrList[idx].netAddrInfo[addrIdx].prefixlen = static_cast<uint8_t>(addr.prefixlen_);
        netAddrList->netAddrList[idx].netAddrInfo[addrIdx].port = static_cast<uint16_t>(addr.port_);
        CopyStrToBuffer(netAddrList->netAddrList[idx].netAddrInfo[addrIdx].address, addr.address_,
                        ETHERNET_MAX_STR_LEN);
        netAddrList->netAddrList[idx].netAddrInfoSize++;
    }
    netAddrList->netAddrListSize++;
}
// LCOV_EXCL_STOP
int32_t OH_Ethernet_GetNetAddress(Ethernet_NetAddrList *netAddrList)
{
    if (netAddrList == nullptr) {
        NETMGR_EXT_LOG_E("netAddrList is nullptr");
        return ETHERNET_ERR_INVALID_PARAMETER;
    }

    std::list<sptr<NetHandle>> netList;
    int32_t ret = NetConnClient::GetInstance().GetAllNets(netList);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        return ConvertErrorCode(ret);
    }
    // LCOV_EXCL_START
    if (netList.empty()) {
        NETMGR_EXT_LOG_E("OH_Ethernet_GetNetAddress, no net");
        return ETHERNET_ERR_DEVICE_INFORMATION_NOT_EXIST;
    }

    netAddrList->netAddrListSize = 0;
    for (auto netHandlePtr : netList) {
        if (!netHandlePtr) {
            NETMGR_EXT_LOG_E("invalid netHandle");
            continue;
        }
        NetAllCapabilities netAllCap;
        NetConnClient::GetInstance().GetNetCapabilities(*netHandlePtr, netAllCap);
        if (netAllCap.bearerTypes_.count(BEARER_ETHERNET) > 0) {
            NetLinkInfo info;
            NetConnClient::GetInstance().GetConnectionProperties(*netHandlePtr, info);
            FillNetAddrListFromEthernet(netAddrList, info);
        }
    }
    if (netAddrList->netAddrListSize == 0) {
        return ETHERNET_ERR_DEVICE_INFORMATION_NOT_EXIST;
    }
    return NETMANAGER_EXT_SUCCESS;
    // LCOV_EXCL_STOP
}

} // extern "C"
