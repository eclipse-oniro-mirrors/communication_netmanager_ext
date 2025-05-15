/*
 * Copyright (C) 2022-2024 Huawei Device Co., Ltd.
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

#include "ethernet_exec.h"

#include <cstdint>
#include <new>
#include <numeric>
#include <string>

#include "mac_address_info.h"
#include "ethernet_client.h"
#include "napi_utils.h"
#include "net_manager_constants.h"
#include "netmanager_ext_log.h"

namespace OHOS {
namespace NetManagerStandard {
namespace EthernetExec {
namespace {
constexpr const char *MODE = "mode";
constexpr const char *IP_ADDR = "ipAddr";
constexpr const char *ROUTE = "route";
constexpr const char *NET_MASK = "netMask";
constexpr const char *GATEWAY = "gateway";
constexpr const char *DNS_SERVERS = "dnsServers";
constexpr const char *DOMAIN = "domain";
constexpr const char *DEFAULT_SEPARATOR = ",";
constexpr const char *MAC_ADDR = "macAddress";
constexpr const char *IFACE = "iface";
constexpr const char *IFACE_NAME = "ifaceName";
constexpr const char *DEVICE_NAME = "deviceName";
constexpr const char *CONNECTION_MODE = "connectionMode";
constexpr const char *SUPPLIER_NAME = "supplierName";
constexpr const char *SUPPLIER_ID = "supplierId";
constexpr const char *PRODUCT_NAME = "productName";
constexpr const char *MAX_RATE = "maximumRate";

std::string AccumulateNetAddress(const std::vector<INetAddr> &netAddrList)
{
    return std::accumulate(
        netAddrList.begin(), netAddrList.end(), std::string(), [](const std::string &addr, const INetAddr &iter) {
            return addr.empty() ? (addr + iter.address_) : (addr + DEFAULT_SEPARATOR + iter.address_);
        });
}
} // namespace
bool ExecGetMacAddress(GetMacAddressContext *context)
{
    int32_t result = DelayedSingleton<EthernetClient>::GetInstance()->GetMacAddress(context->macAddrInfo_);
    if (result != NETMANAGER_EXT_SUCCESS) {
        NETMANAGER_EXT_LOGE("ExecGetMacAddress error, errorCode: %{public}d", result);
        context->SetErrorCode(result);
        return false;
    }
    return true;
}

napi_value GetMacAddressCallback(GetMacAddressContext *context)
{
    napi_value macAddressList = NapiUtils::CreateArray(context->GetEnv(), context->macAddrInfo_.size());
    uint32_t index = 0;
    for (auto &eachInfo : context->macAddrInfo_) {
        napi_value macAddrInfo = NapiUtils::CreateObject(context->GetEnv());
        NapiUtils::SetStringPropertyUtf8(
            context->GetEnv(), macAddrInfo, IFACE, eachInfo.iface_);
        NapiUtils::SetStringPropertyUtf8(
            context->GetEnv(), macAddrInfo, MAC_ADDR, eachInfo.macAddress_);
        NapiUtils::SetArrayElement(context->GetEnv(), macAddressList, index++, macAddrInfo);
    }
    return macAddressList;
}

bool ExecGetIfaceConfig(GetIfaceConfigContext *context)
{
    int32_t result = DelayedSingleton<EthernetClient>::GetInstance()->GetIfaceConfig(context->iface_, context->config_);
    if (context->config_ == nullptr || result != NETMANAGER_EXT_SUCCESS) {
        NETMANAGER_EXT_LOGE("ExecGetIfaceConfig error, errorCode: %{public}d", result);
        context->SetErrorCode(result);
        return false;
    }
    return true;
}

napi_value GetIfaceConfigCallback(GetIfaceConfigContext *context)
{
    napi_value interfaceConfiguration = NapiUtils::CreateObject(context->GetEnv());
    NapiUtils::SetInt32Property(context->GetEnv(), interfaceConfiguration, MODE, context->config_->mode_);

    std::string ipAddresses = AccumulateNetAddress(context->config_->ipStatic_.ipAddrList_);
    NapiUtils::SetStringPropertyUtf8(context->GetEnv(), interfaceConfiguration, IP_ADDR, ipAddresses);

    std::string routeAddresses = AccumulateNetAddress(context->config_->ipStatic_.routeList_);
    NapiUtils::SetStringPropertyUtf8(context->GetEnv(), interfaceConfiguration, ROUTE, routeAddresses);

    std::string gatewayAddresses = AccumulateNetAddress(context->config_->ipStatic_.gatewayList_);
    NapiUtils::SetStringPropertyUtf8(context->GetEnv(), interfaceConfiguration, GATEWAY, gatewayAddresses);

    std::string maskAddresses = AccumulateNetAddress(context->config_->ipStatic_.netMaskList_);
    NapiUtils::SetStringPropertyUtf8(context->GetEnv(), interfaceConfiguration, NET_MASK, maskAddresses);

    std::string dnsServers = AccumulateNetAddress(context->config_->ipStatic_.dnsServers_);
    NapiUtils::SetStringPropertyUtf8(context->GetEnv(), interfaceConfiguration, DNS_SERVERS, dnsServers);

    NapiUtils::SetStringPropertyUtf8(context->GetEnv(), interfaceConfiguration, DOMAIN,
                                     context->config_->ipStatic_.domain_);
    return interfaceConfiguration;
}

bool ExecSetIfaceConfig(SetIfaceConfigContext *context)
{
    int32_t result = DelayedSingleton<EthernetClient>::GetInstance()->SetIfaceConfig(context->iface_, context->config_);
    if (result != NETMANAGER_EXT_SUCCESS) {
        NETMANAGER_EXT_LOGE("ExecSetIfaceConfig error, errorCode: %{public}d", result);
        context->SetErrorCode(result);
        return false;
    }
    return true;
}

napi_value SetIfaceConfigCallback(SetIfaceConfigContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ExecIsIfaceActive(IsIfaceActiveContext *context)
{
    int32_t result =
        DelayedSingleton<EthernetClient>::GetInstance()->IsIfaceActive(context->iface_, context->ifActivate_);
    if (result != NETMANAGER_EXT_SUCCESS) {
        context->SetErrorCode(result);
        NETMANAGER_EXT_LOGE("ExecIsIfaceActive error, errorCode: %{public}d", result);
        return false;
    }
    return true;
}

napi_value IsIfaceActiveCallback(IsIfaceActiveContext *context)
{
    return NapiUtils::CreateInt32(context->GetEnv(), context->ifActivate_);
}

bool ExecGetAllActiveIfaces(GetAllActiveIfacesContext *context)
{
    int32_t result = DelayedSingleton<EthernetClient>::GetInstance()->GetAllActiveIfaces(context->ethernetNameList_);
    if (result != NETMANAGER_EXT_SUCCESS) {
        context->SetErrorCode(result);
        NETMANAGER_EXT_LOGE("ExecIsIfaceActive error, errorCode: %{public}d", result);
        return false;
    }
    return true;
}

napi_value GetAllActiveIfacesCallback(GetAllActiveIfacesContext *context)
{
    napi_value ifaces = NapiUtils::CreateArray(context->GetEnv(), context->ethernetNameList_.size());
    uint32_t index = 0;
    for (const auto &iface : context->ethernetNameList_) {
        napi_value ifaceStr = NapiUtils::CreateStringUtf8(context->GetEnv(), iface);
        NapiUtils::SetArrayElement(context->GetEnv(), ifaces, index++, ifaceStr);
    }
    return ifaces;
}

bool ExecGetDeviceInformation(GetDeviceInformationContext *context)
{
    int32_t result = DelayedSingleton<EthernetClient>::GetInstance()->GetDeviceInformation(context->deviceInfo_);
    if (result != NETMANAGER_EXT_SUCCESS) {
        NETMANAGER_EXT_LOGE("ExecGetDeviceInformation, err: %{public}d", result);
        context->SetErrorCode(result);
        return false;
    }
    return true;
}
 
napi_value GetDeviceInformationCallback(GetDeviceInformationContext *context)
{
    napi_value deviceInfoList = NapiUtils::CreateArray(context->GetEnv(), context->deviceInfo_.size());
    uint32_t index = 0;
    for (auto &eachInfo : context->deviceInfo_) {
        napi_value deviceInfo = NapiUtils::CreateObject(context->GetEnv());
        NapiUtils::SetStringPropertyUtf8(context->GetEnv(), deviceInfo, IFACE_NAME, eachInfo.ifaceName_);
        NapiUtils::SetStringPropertyUtf8(context->GetEnv(), deviceInfo, DEVICE_NAME, eachInfo.deviceName_);
        NapiUtils::SetInt32Property(context->GetEnv(), deviceInfo, CONNECTION_MODE, eachInfo.connectionMode_);
        NapiUtils::SetStringPropertyUtf8(context->GetEnv(), deviceInfo, SUPPLIER_NAME, eachInfo.supplierName_);
        NapiUtils::SetStringPropertyUtf8(context->GetEnv(), deviceInfo, SUPPLIER_ID, eachInfo.supplierId_);
        NapiUtils::SetStringPropertyUtf8(context->GetEnv(), deviceInfo, PRODUCT_NAME, eachInfo.productName_);
        NapiUtils::SetStringPropertyUtf8(context->GetEnv(), deviceInfo, MAX_RATE, eachInfo.maximumRate_);
        NapiUtils::SetArrayElement(context->GetEnv(), deviceInfoList, index++, deviceInfo);
    }
    return deviceInfoList;
}
} // namespace EthernetExec
} // namespace NetManagerStandard
} // namespace OHOS
