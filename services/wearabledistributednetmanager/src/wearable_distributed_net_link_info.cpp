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
#include <string>
#include "netmanager_base_common_utils.h"
#include "netsys_controller.h"
#include "net_manager_constants.h"
#include "wearable_distributed_net_link_info.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const std::string DNSLISTS_FIRST = "dnslistsfirst";
const std::string DNSLISTS_SECOND = "dnslistssecond";
const std::string IFACENAME = "ifacename";
const std::string DEFAULT_NET_MASK = "defaultnetmask";
const std::string NET_IFACE_ADDRESS = "netifaceaddress";
const std::string IPV4_DEFAULT_ROUTE_ADDR = "ipv4defaultrouteaddr";
const std::string DUMMY_ADDRESS = "dummyaddress";
const std::string IPV4_ADDR_NET_MASK = "ipv4addrnetmask";
const std::string ROUTE_DESTINATION_ADDR = "routedestinationaddr";
const std::string INTERFACE_DUMMY = "dummy0";
} // namespace

std::string WearableDistributedNetLinkInfo::GetIfaceName()
{
    return ifaceName_;
}

std::string WearableDistributedNetLinkInfo::GetPrimaryDnsLists()
{
    return primaryDnsLists_;
}

std::string WearableDistributedNetLinkInfo::GetSecondDnsLists()
{
    return secondDnsLists_;
}

std::string WearableDistributedNetLinkInfo::GetDefaultNetMask()
{
    return defaultNetMask_;
}

std::string WearableDistributedNetLinkInfo::GetNetIfaceAddress()
{
    return netIfaceAddress_;
}

std::string WearableDistributedNetLinkInfo::GetIpv4DeRouteAddr()
{
    return ipv4DeRouteAddr_;
}

std::string WearableDistributedNetLinkInfo::GetDummyAddress()
{
    return dummyAddress_;
}

std::string WearableDistributedNetLinkInfo::GetIpv4AddrNetMask()
{
    return ipv4AddrNetMask_;
}

std::string WearableDistributedNetLinkInfo::GetRouteEstinationAddr()
{
    return routeDestinationAddr_;
}

bool WearableDistributedNetLinkInfo::ParseDnsLists(const cJSON &json)
{
    cJSON *dnsListsFirst = cJSON_GetObjectItemCaseSensitive(&json, DNSLISTS_FIRST.c_str());
    cJSON *dnsListsSecond = cJSON_GetObjectItemCaseSensitive(&json, DNSLISTS_SECOND.c_str());
    if (dnsListsFirst == nullptr || dnsListsSecond == nullptr) {
        NETMGR_EXT_LOG_E("Failed to find dnslists information!");
        return false;
    }
    primaryDnsLists_ = cJSON_GetStringValue(dnsListsFirst);
    secondDnsLists_ = cJSON_GetStringValue(dnsListsSecond);
    return true;
}

bool WearableDistributedNetLinkInfo::ParseIfaceName(const cJSON &json)
{
    cJSON *ifaceNameItem = cJSON_GetObjectItemCaseSensitive(&json, IFACENAME.c_str());
    if (ifaceNameItem == nullptr) {
        NETMGR_EXT_LOG_E("Failed to find ifacename information!");
        return false;
    }
    ifaceName_ = cJSON_GetStringValue(ifaceNameItem);
    return true;
}

bool WearableDistributedNetLinkInfo::ParseDefaultNetMask(const cJSON &json)
{
    cJSON *defaultNetmaskItem = cJSON_GetObjectItemCaseSensitive(&json, DEFAULT_NET_MASK.c_str());
    if (defaultNetmaskItem == nullptr) {
        NETMGR_EXT_LOG_E("Failed to find defaultNetMask information!");
        return false;
    }
    defaultNetMask_ = cJSON_GetStringValue(defaultNetmaskItem);
    return true;
}

bool WearableDistributedNetLinkInfo::ParseNetIfaceAddress(const cJSON &json)
{
    cJSON *networkInterfaceAddressItem = cJSON_GetObjectItemCaseSensitive(&json, NET_IFACE_ADDRESS.c_str());
    if (networkInterfaceAddressItem == nullptr) {
        NETMGR_EXT_LOG_E("Failed to find netifaceaddress information!");
        return false;
    }
    netIfaceAddress_ = cJSON_GetStringValue(networkInterfaceAddressItem);
    return true;
}

bool WearableDistributedNetLinkInfo::ParseIpv4DeRouteAddr(const cJSON &json)
{
    cJSON *ipv4DefaultRouteAddressItem = cJSON_GetObjectItemCaseSensitive(&json, IPV4_DEFAULT_ROUTE_ADDR.c_str());
    if (ipv4DefaultRouteAddressItem == nullptr) {
        NETMGR_EXT_LOG_E("Failed to find ipv4derouteaddr information!");
        return false;
    }
    ipv4DeRouteAddr_ = cJSON_GetStringValue(ipv4DefaultRouteAddressItem);
    return true;
}

bool WearableDistributedNetLinkInfo::ParseDummyAddress(const cJSON &json)
{
    cJSON *dummyAddressItem = cJSON_GetObjectItemCaseSensitive(&json, DUMMY_ADDRESS.c_str());
    if (dummyAddressItem == nullptr) {
        NETMGR_EXT_LOG_E("Failed to find dummyaddress information!");
        return false;
    }
    dummyAddress_ = cJSON_GetStringValue(dummyAddressItem);
    return true;
}

bool WearableDistributedNetLinkInfo::ParseIpv4AddrNetMask(const cJSON &json)
{
    cJSON *ipv4AddressNetMaskItem = cJSON_GetObjectItemCaseSensitive(&json, IPV4_ADDR_NET_MASK.c_str());
    if (ipv4AddressNetMaskItem == nullptr) {
        NETMGR_EXT_LOG_E("Failed to find ipv4addrnetmask information!");
        return false;
    }
    ipv4AddrNetMask_ = cJSON_GetStringValue(ipv4AddressNetMaskItem);
    return true;
}

bool WearableDistributedNetLinkInfo::ParseRouteDestinationAddr(const cJSON &json)
{
    cJSON *routeDestinationAddrItem = cJSON_GetObjectItemCaseSensitive(&json, ROUTE_DESTINATION_ADDR.c_str());
    if (routeDestinationAddrItem == nullptr) {
        NETMGR_EXT_LOG_E("Failed to find routedestinationaddr information!");
        return false;
    }
    routeDestinationAddr_ = cJSON_GetStringValue(routeDestinationAddrItem);
    return true;
}

std::string WearableDistributedNetLinkInfo::ReadJsonFile()
{
    std::ifstream infile;
    std::string lineConfigInfo;
    std::string allConfigInfo;
    infile.open(configPath_);
    if (!infile.is_open()) {
        NETMGR_EXT_LOG_E("ReadJsonFile filePath failed");
        return allConfigInfo;
    }
    while (getline(infile, lineConfigInfo)) {
        allConfigInfo.append(lineConfigInfo);
    }
    infile.close();
    return allConfigInfo;
}

bool WearableDistributedNetLinkInfo::ReadNetlinkinfoInterfaces(const cJSON &json)
{
    if (!ParseDnsLists(json)) {
        NETMGR_EXT_LOG_E("ParseDnsLists failed");
        return false;
    }
    if (!ParseIfaceName(json)) {
        NETMGR_EXT_LOG_E("ParseIfaceName failed");
        return false;
    }
    if (!ParseDefaultNetMask(json)) {
        NETMGR_EXT_LOG_E("ParseDefaultNetMask failed");
        return false;
    }
    if (!ParseNetIfaceAddress(json)) {
        NETMGR_EXT_LOG_E("ParseNetIfaceAddress failed");
        return false;
    }
    if (!ParseIpv4DeRouteAddr(json)) {
        NETMGR_EXT_LOG_E("ParseIpv4DeRouteAddr failed");
        return false;
    }
    if (!ParseDummyAddress(json)) {
        NETMGR_EXT_LOG_E("ParseDummyAddress failed");
        return false;
    }
    if (!ParseIpv4AddrNetMask(json)) {
        NETMGR_EXT_LOG_E("ParseIpv4AddrNetMask failed");
        return false;
    }
    if (!ParseRouteDestinationAddr(json)) {
        NETMGR_EXT_LOG_E("ParseRouteDestinationAddr failed");
        return false;
    }
    return true;
}

bool WearableDistributedNetLinkInfo::ReadSystemNetlinkinfoConfiguration()
{
    const auto &jsonStr = ReadJsonFile();
    if (jsonStr.length() == 0) {
        NETMGR_EXT_LOG_E("ReadConfigData config file is return empty!");
        return false;
    }
    cJSON *json = cJSON_Parse(jsonStr.c_str());
    if (json == nullptr) {
        NETMGR_EXT_LOG_E("json parse failed!");
        return false;
    }
    bool result = ReadNetlinkinfoInterfaces(*json);
    if (result == false) {
        NETMGR_EXT_LOG_E("parse files failed!");
        cJSON_Delete(json);
        return false;
    }
    cJSON_Delete(json);
    return true;
}

void WearableDistributedNetLinkInfo::SetInterFaceName(NetLinkInfo &linkInfo)
{
    if (!GetIfaceName().empty()) {
        linkInfo.ifaceName_ = GetIfaceName();
    }
}

int32_t WearableDistributedNetLinkInfo::SetDnsLists(NetLinkInfo &linkInfo)
{
    INetAddr dnsFirst;
    INetAddr dnsSecond;
    dnsFirst.type_ = INetAddr::IPV4;
    dnsFirst.family_ = AF_INET;
    dnsFirst.address_ = GetPrimaryDnsLists();
    linkInfo.dnsList_.push_back(dnsFirst);

    dnsSecond.type_ = INetAddr::IPV4;
    dnsSecond.family_ = AF_INET;
    dnsSecond.address_ = GetSecondDnsLists();
    linkInfo.dnsList_.push_back(dnsSecond);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t WearableDistributedNetLinkInfo::SetNetLinkIPInfo(NetLinkInfo &linkInfo)
{
    INetAddr netAddr;
    netAddr.type_ = INetAddr::IPV4;
    netAddr.family_ = AF_INET;
    netAddr.address_ = GetIpv4DeRouteAddr();
    netAddr.netMask_ = GetDefaultNetMask();
    netAddr.prefixlen_ = CommonUtils::GetMaskLength(GetDefaultNetMask());

    linkInfo.netAddrList_.push_back(netAddr);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t WearableDistributedNetLinkInfo::SetNetLinkRouteInfo(NetLinkInfo &linkInfo)
{
    Route route;
    route.iface_ = GetIfaceName();
    route.destination_.type_ = INetAddr::IPV4;
    route.destination_.family_ = AF_INET;
    route.destination_.address_ = GetRouteEstinationAddr();
    route.gateway_.address_ = GetNetIfaceAddress();
    route.gateway_.family_ = AF_INET;

    linkInfo.routeList_.push_back(route);
    return NETMANAGER_EXT_SUCCESS;
}

void WearableDistributedNetLinkInfo::SetMtu(NetLinkInfo &linkInfo)
{
    linkInfo.mtu_ = CONSTANTS::WEARABLE_DISTRIBUTED_NET_MTU;
}

int32_t SetInterfaceDummyDown()
{
    if (NetsysController::GetInstance().SetInterfaceDown(INTERFACE_DUMMY.c_str()) != 0) {
        NETMGR_EXT_LOG_E("Failed setting dummy0 interface down");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

// Add and pull up interface dummy0
int32_t WearableDistributedNetLinkInfo::SetInterfaceDummyUp()
{
    std::string addr = GetDummyAddress();
    auto prefixLen = CommonUtils::GetMaskLength(GetIpv4AddrNetMask());
    if (NetsysController::GetInstance().AddInterfaceAddress(INTERFACE_DUMMY.c_str(), addr, prefixLen) != 0) {
        NETMGR_EXT_LOG_E("Failed setting dummy0 address");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    if (NetsysController::GetInstance().SetInterfaceUp(INTERFACE_DUMMY.c_str()) != 0) {
        NETMGR_EXT_LOG_E("Failed setting dummy0 interface up");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t CreateNetLinkInfo(NetLinkInfo &linkInfo)
{
    WearableDistributedNetLinkInfo info;
    if (!info.ReadSystemNetlinkinfoConfiguration()) {
        NETMGR_EXT_LOG_E("ReadSystemNetlinkinfoConfiguration failed");
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    info.SetNetLinkIPInfo(linkInfo);
    info.SetInterFaceName(linkInfo);
    info.SetDnsLists(linkInfo);
    info.SetNetLinkRouteInfo(linkInfo);
    info.SetMtu(linkInfo);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t SetInterfaceDummyUp()
{
    WearableDistributedNetLinkInfo info;
    if (!info.ReadSystemNetlinkinfoConfiguration()) {
        NETMGR_EXT_LOG_E("ReadSystemNetlinkinfoConfiguration failed");
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    int32_t result = info.SetInterfaceDummyUp();
    if (result != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("SetInterfaceDummyUp failed, result: [%{public}d]", result);
    }
    return result;
}
} // namespace NetManagerStandard
} // namespace OHOS
