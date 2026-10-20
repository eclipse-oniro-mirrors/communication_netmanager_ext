/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include "ethernet_ani.h"

#include <cstdlib>
#include <numeric>

#include "errorcode_convertor.h"
#include "ethernet_client.h"
#include "interface_configuration.h"
#include "net_manager_constants.h"
#include "netmanager_base_common_utils.h"

#include "wrapper.rs.h"

namespace OHOS {
namespace NetManagerAni {

constexpr const char *DEFAULT_SEPARATOR = ",";

namespace {
sptr<InterfaceStateCallbackObserverAni> g_interfaceStateCallbackObserverAni =
    sptr<InterfaceStateCallbackObserverAni>(new (std::nothrow) InterfaceStateCallbackObserverAni());

bool g_isInterfaceStateObserverRegistered = false;
} // namespace

int32_t IsEthernetEnabled(bool &enabled)
{
    enabled = false;
#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
    int32_t isEnabled = 0;
    auto ret = DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->IsEthernetEnabled(isEnabled);
    if (ret == NetManagerStandard::NETMANAGER_EXT_SUCCESS) {
        enabled = isEnabled != 0;
    }
    return ret;
#else
    return NetManagerStandard::NETMANAGER_ERR_OPERATION_FAILED;
#endif
}

int32_t DisableEthernetInterface()
{
#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
    return DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->DisableEthernetInterface();
#else
    return NetManagerStandard::NETMANAGER_ERR_OPERATION_FAILED;
#endif
}

int32_t EnableEthernetInterface()
{
#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
    return DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->EnableEthernetInterface();
#else
    return NetManagerStandard::NETMANAGER_ERR_OPERATION_FAILED;
#endif
}

int32_t g_getAllActiveIfaces(rust::Vec<rust::String> &activeIfaces)
{
    std::vector<std::string> ifacesVec;
    int32_t ret =
        DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->GetAllActiveIfaces(ifacesVec);
    for (auto &iface : ifacesVec) {
        activeIfaces.push_back(iface);
    }
    return ret;
}

int32_t IsIfaceActive(const rust::String &iface, int32_t &activeStatus)
{
    if (iface.empty()) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    int32_t ret = DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->IsIfaceActive(
        std::string(iface), activeStatus);
    return ret;
}

static std::string AccumulateNetAddress(const std::vector<NetManagerStandard::INetAddr> &netAddrList)
{
    return std::accumulate(
        netAddrList.begin(), netAddrList.end(), std::string(), [](
            const std::string &addr, const NetManagerStandard::INetAddr &iter) {
            return addr.empty() ? (addr + iter.address_) : (addr + DEFAULT_SEPARATOR + iter.address_);
        });
}

int32_t GetIfaceConfig(const rust::String &iface, InterfaceConfigurationAni &ifaceConfig)
{
    if (iface.empty()) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    sptr<NetManagerStandard::InterfaceConfiguration> config;
    auto ret = DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->GetIfaceConfig(
        std::string(iface), config);
    if (ret == NetManagerStandard::NETMANAGER_EXT_SUCCESS && config != nullptr) {
        ifaceConfig.mode = config->mode_;
        ifaceConfig.ip_addr = AccumulateNetAddress(config->ipStatic_.ipAddrList_);
        ifaceConfig.route = AccumulateNetAddress(config->ipStatic_.routeList_);
        ifaceConfig.gateway = AccumulateNetAddress(config->ipStatic_.gatewayList_);
        ifaceConfig.net_mask = AccumulateNetAddress(config->ipStatic_.netMaskList_);
        ifaceConfig.dns_servers = AccumulateNetAddress(config->ipStatic_.dnsServers_);
        ifaceConfig.http_proxy.host = config->httpProxy_.GetHost();
        ifaceConfig.http_proxy.port = config->httpProxy_.GetPort();
        ifaceConfig.http_proxy.username = config->httpProxy_.GetUsername();
        ifaceConfig.http_proxy.password = config->httpProxy_.GetPassword();
        auto lists = config->httpProxy_.GetExclusionList();
        for (auto item : lists) {
            ifaceConfig.http_proxy.exclusion_list.push_back(item);
        }
    }
    
    return ret;
}

static void ExtractNetAddrBySeparator(const std::string &input, std::vector<NetManagerStandard::INetAddr> &netAddrList)
{
    std::vector<NetManagerStandard::INetAddr>().swap(netAddrList);
    for (const auto &netAddr : NetManagerStandard::CommonUtils::Split(input, DEFAULT_SEPARATOR)) {
        NetManagerStandard::INetAddr addr;
        addr.address_ = netAddr;
        netAddrList.push_back(addr);
    }
}

int32_t SetIfaceConfig(const rust::String &iface, const InterfaceConfigurationAni &ifaceConfig)
{
    if (iface.empty()) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    if (ifaceConfig.mode < static_cast<int32_t>(NetManagerStandard::IPSetMode::STATIC) ||
        ifaceConfig.mode > static_cast<int32_t>(NetManagerStandard::IPSetMode::LAN_DHCP)) {
        return NetManagerStandard::NETMANAGER_ERR_INVALID_PARAMETER;
    }
    sptr<NetManagerStandard::InterfaceConfiguration> config =
        new (std::nothrow) NetManagerStandard::InterfaceConfiguration();
    if (config == nullptr) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_INTERNAL;
    }
    config->mode_ = static_cast<NetManagerStandard::IPSetMode>(ifaceConfig.mode);
    ExtractNetAddrBySeparator(std::string(ifaceConfig.ip_addr), config->ipStatic_.ipAddrList_);
    ExtractNetAddrBySeparator(std::string(ifaceConfig.route), config->ipStatic_.routeList_);
    ExtractNetAddrBySeparator(std::string(ifaceConfig.gateway), config->ipStatic_.gatewayList_);
    ExtractNetAddrBySeparator(std::string(ifaceConfig.net_mask), config->ipStatic_.netMaskList_);
    ExtractNetAddrBySeparator(std::string(ifaceConfig.dns_servers), config->ipStatic_.dnsServers_);
    config->ipStatic_.domain_ = std::string("");

    auto host = std::string(ifaceConfig.http_proxy.host);
    uint16_t port = 0;
    std::list<std::string> exclusionList;
    if (!host.empty()) {
        port = ifaceConfig.http_proxy.port;
        for (auto item : ifaceConfig.http_proxy.exclusion_list) {
            exclusionList.push_back(std::string(item));
        }
    }
    config->httpProxy_.SetHost(std::move(host));
    config->httpProxy_.SetPort(port);
    config->httpProxy_.SetExclusionList(exclusionList);

    return DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->SetIfaceConfig(
        std::string(iface), config);
}

int32_t GetDeviceInformation(rust::Vec<EthernetDeviceInfosAni> &deviceInfos)
{
    std::vector<NetManagerStandard::EthernetDeviceInfo> deviceInfoList;
    auto ret = DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->GetDeviceInformation(
        deviceInfoList);
    if (ret == NetManagerStandard::NETMANAGER_EXT_SUCCESS && !deviceInfoList.empty()) {
        for (auto item : deviceInfoList) {
            EthernetDeviceInfosAni deviceInfo;
            deviceInfo.iface_name = item.ifaceName_;
            deviceInfo.device_name = item.deviceName_;
            deviceInfo.connection_mode = static_cast<int32_t>(item.connectionMode_);
            deviceInfo.supplier_name = item.supplierName_;
            deviceInfo.supplier_id = item.supplierId_;
            deviceInfo.product_name = item.productName_;
            deviceInfo.maximum_rate = item.maximumRate_;
            deviceInfos.push_back(deviceInfo);
        }
    }
    return ret;
}

int32_t GetMacAddress(rust::Vec<MacAddressInfoAni> &macAddrs)
{
    std::vector<NetManagerStandard::MacAddressInfo> macAddrList;
    auto ret = DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->GetMacAddress(macAddrList);
    if (ret == NetManagerStandard::NETMANAGER_EXT_SUCCESS && !macAddrList.empty()) {
        for (auto item : macAddrList) {
            MacAddressInfoAni macAddr;
            macAddr.iface = item.iface_;
            macAddr.mac_address = item.macAddress_;
            macAddrs.push_back(macAddr);
        }
    }
    return ret;
}

rust::String GetErrorCodeAndMessage(int32_t &errorCode)
{
    NetManagerStandard::NetBaseErrorCodeConvertor convertor;
    rust::String result = rust::string(convertor.ConvertErrorCode(errorCode));
    return result;
}

int32_t InterfaceStateCallbackObserverAni::OnInterfaceAdded(const std::string &ifName)
{
    execute_interface_added(rust::String(ifName));
    return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
}

int32_t InterfaceStateCallbackObserverAni::OnInterfaceRemoved(const std::string &ifName)
{
    execute_interface_removed(rust::String(ifName));
    return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
}

int32_t InterfaceStateCallbackObserverAni::OnInterfaceChanged(const std::string &ifName, bool up)
{
    InterfaceStateInfoAni info{
        .iface = rust::String(ifName),
        .active = up,
    };
    execute_interface_changed(info);
    return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
}

int32_t RegisterInterfaceStateCallback()
{
    if (g_isInterfaceStateObserverRegistered) {
        return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
    }

    if (g_interfaceStateCallbackObserverAni == nullptr) {
        g_interfaceStateCallbackObserverAni =
            sptr<InterfaceStateCallbackObserverAni>(new (std::nothrow) InterfaceStateCallbackObserverAni());
        if (g_interfaceStateCallbackObserverAni == nullptr) {
            return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
        }
    }

    auto client = DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance();
    if (client == nullptr) {
        return NetManagerStandard::NETMANAGER_ERR_OPERATION_FAILED;
    }

    int32_t result = client->RegisterIfacesStateChanged(g_interfaceStateCallbackObserverAni);
    if (result == NetManagerStandard::NETMANAGER_EXT_SUCCESS) {
        g_isInterfaceStateObserverRegistered = true;
    }
    return result;
}

int32_t UnregisterInterfaceStateCallback()
{
    if (g_interfaceStateCallbackObserverAni == nullptr) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }

    auto client = DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance();
    if (client == nullptr) {
        return NetManagerStandard::NETMANAGER_ERR_OPERATION_FAILED;
    }

    int32_t result = client->UnregisterIfacesStateChanged(g_interfaceStateCallbackObserverAni);
    if (result == NetManagerStandard::NETMANAGER_EXT_SUCCESS) {
        g_isInterfaceStateObserverRegistered = false;
    }
    return result;
}

} // namespace NetManagerAni
} // namespace OHOS
