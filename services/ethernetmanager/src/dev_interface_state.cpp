/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "dev_interface_state.h"

#include "inet_addr.h"
#include "net_manager_center.h"
#include "net_manager_constants.h"
#include "netmanager_base_common_utils.h"
#include "netmgr_ext_log_wrapper.h"
#include "netsys_controller.h"
#include "route.h"
#include "static_configuration.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr const char *DEFAULT_IPV4_ADDR = "0.0.0.0";
constexpr const char *DEFAULT_IPV6_ADDR = "::";
} // namespace

DevInterfaceState::DevInterfaceState()
{
    netSupplierInfo_ = new (std::nothrow) NetSupplierInfo();
    if (netSupplierInfo_ == nullptr) {
        NETMGR_EXT_LOG_E("NetSupplierInfo new failed");
    }
}

void DevInterfaceState::SetDevName(const std::string &devName)
{
    devName_ = devName;
}

void DevInterfaceState::SetNetCaps(const std::set<NetCap> &netCaps)
{
    netCaps_ = netCaps;
}

void DevInterfaceState::SetLinkUp(bool up)
{
    linkUp_ = up;
}

void DevInterfaceState::SetlinkInfo(sptr<NetLinkInfo> &linkInfo)
{
    std::unique_lock<std::shared_mutex> lock(linkInfoMutex_);
    linkInfo_ = linkInfo;
}

void DevInterfaceState::SetIfcfg(sptr<InterfaceConfiguration> &ifCfg)
{
    // LCOV_EXCL_START
    if (ifCfg == NULL) {
        return;
    }
    // LCOV_EXCL_STOP
    ifCfg_ = ifCfg;
    if (ifCfg_->mode_ == STATIC) {
        UpdateLinkInfo();
        if (connLinkState_ == LINK_AVAILABLE) {
            RemoteUpdateNetLinkInfo();
        }
    }
}

void DevInterfaceState::SetLancfg(sptr<InterfaceConfiguration> &ifCfg)
{
    // LCOV_EXCL_START
    if (ifCfg == NULL) {
        return;
    }
    // LCOV_EXCL_STOP
    ifCfg_ = ifCfg;
    if (ifCfg_->mode_ == LAN_STATIC) {
        UpdateLanLinkInfo();
    }
}

void DevInterfaceState::SetDhcpReqState(bool dhcpReqState)
{
    dhcpReqState_ = dhcpReqState;
}

std::string DevInterfaceState::GetDevName() const
{
    return devName_;
}

const std::set<NetCap> &DevInterfaceState::GetNetCaps() const
{
    return netCaps_;
}

std::set<NetCap> DevInterfaceState::GetNetCaps()
{
    return netCaps_;
}

bool DevInterfaceState::GetLinkUp() const
{
    return linkUp_;
}

bool DevInterfaceState::GetLinkInfo(NetLinkInfo &linkInfo)
{
    std::shared_lock<std::shared_mutex> lock(linkInfoMutex_);
    if (linkInfo_ == nullptr) {
        return false;
    }
    linkInfo = *linkInfo_;
    return true;
}

sptr<InterfaceConfiguration> DevInterfaceState::GetIfcfg() const
{
    return ifCfg_;
}

bool DevInterfaceState::IsLanIface()
{
    if (ifCfg_ == nullptr) {
        return false;
    }
    if (ifCfg_->mode_ == LAN_STATIC || ifCfg_->mode_ == LAN_DHCP) {
        return true;
    }
    return false;
}

IPSetMode DevInterfaceState::GetIPSetMode() const
{
    if (ifCfg_ == nullptr) {
        return IPSetMode::STATIC;
    }
    return ifCfg_->mode_;
}

bool DevInterfaceState::GetDhcpReqState() const
{
    return dhcpReqState_;
}

void DevInterfaceState::RemoteRegisterNetSupplier()
{
    if (connLinkState_ == UNREGISTERED) {
        if (netCaps_.empty()) {
            netCaps_.insert(NET_CAPABILITY_INTERNET);
        }
        int32_t result =
            NetManagerCenter::GetInstance().RegisterNetSupplier(bearerType_, devName_, netCaps_, netSupplier_);
        if (result == NETMANAGER_SUCCESS) {
            connLinkState_ = REGISTERED;
        }
        NETMGR_EXT_LOG_D("DevInterfaceCfg RemoteRegisterNetSupplier netSupplier_[%{public}d]", netSupplier_);
    }
}

void DevInterfaceState::RemoteUnregisterNetSupplier()
{
    if (connLinkState_ == UNREGISTERED) {
        return;
    }
    int ret = NetManagerCenter::GetInstance().UnregisterNetSupplier(netSupplier_);
    if (ret == NETMANAGER_SUCCESS) {
        connLinkState_ = UNREGISTERED;
        netSupplier_ = 0;
    }
}

void DevInterfaceState::RemoteUpdateNetLinkInfo()
{
    if (connLinkState_ == LINK_UNAVAILABLE) {
        NETMGR_EXT_LOG_E("DevInterfaceCfg RemoteUpdateNetLinkInfo regState_:LINK_UNAVAILABLE");
        return;
    }
    std::shared_lock<std::shared_mutex> lock(linkInfoMutex_);
    if (linkInfo_ == nullptr) {
        NETMGR_EXT_LOG_E("DevInterfaceCfg RemoteUpdateNetLinkInfo linkInfo_ is nullptr");
        return;
    }

    for (auto &netAddr: linkInfo_->netAddrList_) {
        if (netAddr.family_ == AF_INET) {
            netAddr.family_ = INetAddr::IpType::IPV4;
        } else if (netAddr.family_ == AF_INET6) {
            netAddr.family_ = INetAddr::IpType::IPV6;
        }
    }
    NetManagerCenter::GetInstance().UpdateNetLinkInfo(netSupplier_, linkInfo_);
}

void DevInterfaceState::RemoteUpdateNetSupplierInfo()
{
    if (connLinkState_ == UNREGISTERED) {
        NETMGR_EXT_LOG_E("DevInterfaceCfg RemoteUpdateNetSupplierInfo regState_:UNREGISTERED");
        return;
    }
    if (netSupplierInfo_ == nullptr) {
        NETMGR_EXT_LOG_E("DevInterfaceCfg RemoteUpdateNetSupplierInfo netSupplierInfo_ is nullptr");
        return;
    }
    UpdateSupplierAvailable();
    NetManagerCenter::GetInstance().UpdateNetSupplierInfo(netSupplier_, netSupplierInfo_);
}

void DevInterfaceState::UpdateNetHttpProxy(const HttpProxy &httpProxy)
{
    if (httpProxy == ifCfg_->httpProxy_) {
        NETMGR_EXT_LOG_E("The currently set http proxy is the same as the entered http proxy");
        return;
    }
    ifCfg_->httpProxy_ = httpProxy;
    if (connLinkState_ == LINK_AVAILABLE) {
        std::unique_lock<std::shared_mutex> lock(linkInfoMutex_);
        if (linkInfo_ == nullptr) {
            NETMGR_EXT_LOG_E("linkInfo_ is nullptr");
            return;
        }
        linkInfo_->httpProxy_ = httpProxy;
        lock.unlock();
        RemoteUpdateNetLinkInfo();
    }
}

void DevInterfaceState::UpdateLinkInfo()
{
    if (ifCfg_ == nullptr || ifCfg_->mode_ != STATIC) {
        return;
    }
    std::unique_lock<std::shared_mutex> lock(linkInfoMutex_);
    if (linkInfo_ == nullptr) {
        linkInfo_ = new (std::nothrow) NetLinkInfo();
        if (linkInfo_ == nullptr) {
            NETMGR_EXT_LOG_E("linkInfo_ is nullptr");
            return;
        }
    }
    std::list<INetAddr>().swap(linkInfo_->netAddrList_);
    std::list<Route>().swap(linkInfo_->routeList_);
    std::list<INetAddr>().swap(linkInfo_->dnsList_);
    linkInfo_->ifaceName_ = devName_;
    bool hasIpv4Addr = false;
    bool hasIpv6Addr = false;
    for (const auto &ipAddr : ifCfg_->ipStatic_.ipAddrList_) {
        linkInfo_->netAddrList_.push_back(ipAddr);
        auto family = CommonUtils::GetAddrFamily(ipAddr.address_);
        bool isValidAddr = CommonUtils::IsValidAddress(ipAddr.address_);
        hasIpv4Addr = (family == AF_INET && isValidAddr) ? true : hasIpv4Addr;
        hasIpv6Addr = (family == AF_INET6 && isValidAddr) ? true : hasIpv6Addr;
    }

    if (ifCfg_->ipStatic_.routeList_.empty()) {
        CreateDefaultRoute(ifCfg_->ipStatic_.gatewayList_, hasIpv4Addr, hasIpv6Addr);
    } else {
        for (const auto &netAddr : ifCfg_->ipStatic_.routeList_) {
            Route route;
            route.iface_ = devName_;
            route.destination_ = netAddr;
            route.destination_.type_ = GetIpType(netAddr.address_);
            GetTargetNetAddrWithSameFamily(netAddr.address_, ifCfg_->ipStatic_.gatewayList_, route.gateway_);
            linkInfo_->routeList_.push_back(route);
        }
    }

    CreateLocalRoute(devName_, ifCfg_->ipStatic_.ipAddrList_, ifCfg_->ipStatic_.netMaskList_);

    for (auto dnsServer : ifCfg_->ipStatic_.dnsServers_) {
        if (dnsServer.address_.empty()) {
            continue;
        }
        linkInfo_->dnsList_.push_back(dnsServer);
    }
    linkInfo_->httpProxy_ = ifCfg_->httpProxy_;
}

void DevInterfaceState::UpdateLanLinkInfo()
{
    if (ifCfg_ == nullptr || ifCfg_->mode_ != LAN_STATIC) {
        return;
    }
    std::unique_lock<std::shared_mutex> lock(linkInfoMutex_);
    if (linkInfo_ == nullptr) {
        linkInfo_ = new (std::nothrow) NetLinkInfo();
        if (linkInfo_ == nullptr) {
            NETMGR_EXT_LOG_E("linkInfo_ is nullptr");
            return;
        }
    }
    std::list<INetAddr>().swap(linkInfo_->netAddrList_);
    std::list<Route>().swap(linkInfo_->routeList_);
    linkInfo_->ifaceName_ = devName_;
    for (const auto &ipAddr : ifCfg_->ipStatic_.ipAddrList_) {
        linkInfo_->netAddrList_.push_back(ipAddr);
    }

    for (const auto &netAddr : ifCfg_->ipStatic_.routeList_) {
        Route route;
        route.iface_ = devName_;
        route.destination_ = netAddr;
        route.destination_.type_ = GetIpType(netAddr.address_);
        GetRoutePrefixlen(netAddr.address_, ifCfg_->ipStatic_.netMaskList_, route.destination_);
        GetTargetNetAddrWithSameFamily(netAddr.address_, ifCfg_->ipStatic_.gatewayList_, route.gateway_);
        linkInfo_->routeList_.push_back(route);
    }
}

void DevInterfaceState::UpdateLanLinkInfo(const StaticConfiguration &config)
{
    std::unique_lock<std::shared_mutex> lock(linkInfoMutex_);
    if (linkInfo_ == nullptr) {
        linkInfo_ = new (std::nothrow) NetLinkInfo();
        if (linkInfo_ == nullptr) {
            NETMGR_EXT_LOG_E("NetLinkInfo new failed");
            return;
        }
    }
    std::list<INetAddr>().swap(linkInfo_->netAddrList_);
    std::list<Route>().swap(linkInfo_->routeList_);
    linkInfo_->ifaceName_ = devName_;
    for (const auto &ipAddr : config.ipAddrList_) {
        linkInfo_->netAddrList_.push_back(ipAddr);
    }

    for (const auto &routeAddr : config.routeList_) {
        Route routeStc;
        routeStc.iface_ = devName_;
        routeStc.destination_ = routeAddr;
        routeStc.destination_.type_ = GetIpType(routeAddr.address_);
        GetRoutePrefixlen(routeAddr.address_, config.netMaskList_, routeStc.destination_);
        GetTargetNetAddrWithSameFamily(routeAddr.address_, config.gatewayList_, routeStc.gateway_);
        linkInfo_->routeList_.push_back(routeStc);
    }
}

void DevInterfaceState::UpdateLinkInfo(const StaticConfiguration &config)
{
    std::unique_lock<std::shared_mutex> lock(linkInfoMutex_);
    if (linkInfo_ == nullptr) {
        linkInfo_ = new (std::nothrow) NetLinkInfo();
        if (linkInfo_ == nullptr) {
            NETMGR_EXT_LOG_E("NetLinkInfo new failed");
            return;
        }
    }

    std::list<INetAddr>().swap(linkInfo_->netAddrList_);
    std::list<Route>().swap(linkInfo_->routeList_);
    std::list<INetAddr>().swap(linkInfo_->dnsList_);
    linkInfo_->ifaceName_ = devName_;
    for (const auto &ipAddr : config.ipAddrList_) {
        linkInfo_->netAddrList_.push_back(ipAddr);
    }

    for (const auto &routeAddr : config.routeList_) {
        Route routeStc;
        routeStc.iface_ = devName_;
        routeStc.destination_ = routeAddr;
        routeStc.destination_.type_ = GetIpType(routeAddr.address_);
        GetTargetNetAddrWithSameFamily(routeAddr.address_, config.gatewayList_, routeStc.gateway_);
        linkInfo_->routeList_.push_back(routeStc);
    }
    CreateLocalRoute(devName_, config.ipAddrList_, config.netMaskList_);

    for (auto dnsServer : config.dnsServers_) {
        if (dnsServer.address_.empty()) {
            continue;
        }
        linkInfo_->dnsList_.push_back(dnsServer);
    }
    if (ifCfg_) {
        linkInfo_->httpProxy_ = ifCfg_->httpProxy_;
    }
}

void DevInterfaceState::UpdateSupplierAvailable()
{
    if (netSupplierInfo_ == nullptr) {
        return;
    }
    netSupplierInfo_->isAvailable_ = linkUp_;
    connLinkState_ = linkUp_ ? LINK_AVAILABLE : LINK_UNAVAILABLE;
}

void DevInterfaceState::CreateLocalRoute(const std::string &iface, const std::vector<INetAddr> &ipAddrList,
                                         const std::vector<INetAddr> &netMaskList)
{
    /* no need to add lock for linkInfo_, the func is called by UpdateLinkInfo and UpdateLanLinkInfo,
     * have locked allready.
     */
    if (linkInfo_ == nullptr) {
        NETMGR_EXT_LOG_E("linkInfo_ is nullptr");
        return;
    }

    for (const auto &ipAddr : ipAddrList) {
        auto family = CommonUtils::GetAddrFamily(ipAddr.address_);
        std::string routeAddr = (family == AF_INET6) ? CommonUtils::GetIpv6Prefix(ipAddr.address_, ipAddr.prefixlen_)
                                                     : GetIpv4Prefix(ipAddr.address_, netMaskList);
        Route localRoute;
        localRoute.iface_ = iface;
        localRoute.destination_.type_ = GetIpType(ipAddr.address_);
        localRoute.destination_.address_ = routeAddr;
        localRoute.destination_.prefixlen_ = ipAddr.prefixlen_;
        localRoute.gateway_.address_ = (family == AF_INET) ? DEFAULT_IPV4_ADDR : "";
        linkInfo_->routeList_.push_back(localRoute);
    }
}

std::string DevInterfaceState::GetIpv4Prefix(const std::string &ipv4Addr, const std::vector<INetAddr> &netMaskList)
{
    INetAddr maskAddr;
    GetTargetNetAddrWithSameFamily(ipv4Addr, netMaskList, maskAddr);
    uint32_t ipInt = CommonUtils::ConvertIpv4Address(ipv4Addr);
    uint32_t maskInt = CommonUtils::ConvertIpv4Address(maskAddr.address_);
    return CommonUtils::ConvertIpv4Address(ipInt & maskInt);
}

void DevInterfaceState::GetTargetNetAddrWithSameFamily(const std::string &bySrcAddr,
                                                       const std::vector<INetAddr> &fromAddrList,
                                                       INetAddr &targetNetAddr)
{
    auto family = CommonUtils::GetAddrFamily(bySrcAddr);
    for (const auto &addr : fromAddrList) {
        if (family != CommonUtils::GetAddrFamily(addr.address_)) {
            continue;
        }
        targetNetAddr = addr;
        return;
    }
}

void DevInterfaceState::GetRoutePrefixlen(const std::string &bySrcAddr,
                                          const std::vector<INetAddr> &fromAddrList,
                                          INetAddr &targetNetAddr)
{
    auto route_family = CommonUtils::GetAddrFamily(bySrcAddr);
    for (const auto &netMask : fromAddrList) {
        auto route_mask_family = CommonUtils::GetAddrFamily(netMask.address_);
        if (route_family == route_mask_family) {
            targetNetAddr.prefixlen_ = (route_family == AF_INET6)
                ? static_cast<uint32_t>(CommonUtils::Ipv6PrefixLen(netMask.address_))
                : static_cast<uint32_t>(CommonUtils::Ipv4PrefixLen(netMask.address_));
        }
    }
}

void DevInterfaceState::GetDumpInfo(std::string &info)
{
    const std::string TAB = "  ";
    std::list<std::string> dumpInfo = {
        "DevName: " + devName_,
        "ConnLinkState: " + std::to_string(connLinkState_),
        "LinkUp: " + std::to_string(linkUp_),
        "DHCPReqState: " + std::to_string(dhcpReqState_),
    };
    std::string data = "DevInterfaceState: \n";
    std::for_each(dumpInfo.begin(), dumpInfo.end(),
                  [&data, &TAB](const auto &msg) { data.append(TAB + TAB + msg + "\n"); });
    std::shared_lock<std::shared_mutex> lock(linkInfoMutex_);
    if (linkInfo_ != nullptr) {
        data.append(linkInfo_->ToString(TAB) + "\n");
    }
    lock.unlock();
    if (netSupplierInfo_ != nullptr) {
        data.append(netSupplierInfo_->ToString(TAB) + "\n");
    }
    if (ifCfg_ != nullptr) {
        data.append("\n" + TAB + TAB + "InterfaceConfig: \n" + TAB + TAB + TAB +
                    "Mode: " + std::to_string(ifCfg_->mode_) + "\n");
        data.append("\nConfig: \n");
        data.append(TAB + TAB + "IpAddr: ");
        std::for_each(ifCfg_->ipStatic_.ipAddrList_.begin(), ifCfg_->ipStatic_.ipAddrList_.end(),
                      [&data, &TAB](const auto &ipAddr) { data.append(TAB + TAB + ipAddr.ToString(TAB)); });

        data.append("\n" + TAB + TAB + "Route: ");
        std::for_each(ifCfg_->ipStatic_.routeList_.begin(), ifCfg_->ipStatic_.routeList_.end(),
                      [&data, &TAB](const auto &routeAddr) { data.append(TAB + TAB + routeAddr.ToString(TAB)); });

        data.append("\n" + TAB + TAB + "GateWay: ");
        std::for_each(ifCfg_->ipStatic_.gatewayList_.begin(), ifCfg_->ipStatic_.gatewayList_.end(),
                      [&data, &TAB](const auto &gateway) { data.append(TAB + TAB + gateway.ToString(TAB)); });

        data.append("\n" + TAB + TAB + "NetMask: ");
        std::for_each(ifCfg_->ipStatic_.netMaskList_.begin(), ifCfg_->ipStatic_.netMaskList_.end(),
                      [&data, &TAB](const auto &netMask) { data.append(TAB + TAB + netMask.ToString(TAB)); });

        data.append("\n" + TAB + TAB + "DNSServers: ");
        std::for_each(ifCfg_->ipStatic_.dnsServers_.begin(), ifCfg_->ipStatic_.dnsServers_.end(),
                      [&data, &TAB](const auto &server) { data.append(TAB + TAB + server.ToString(TAB)); });

        data.append("\n" + TAB + TAB + "Domain: " + ifCfg_->ipStatic_.domain_ + "\n" + TAB + TAB + "NetCaps: {");
        std::for_each(netCaps_.begin(), netCaps_.end(),
                      [&data, &TAB](const auto &cap) { data.append(std::to_string(cap) + ", "); });
        data.append("}\n");
    }
    data.append(TAB + TAB + "BearerType :" + std::to_string(bearerType_) + "\n");
    info.append(data);
}

uint8_t DevInterfaceState::GetIpType(const std::string& ipAddr)
{
    auto family = CommonUtils::GetAddrFamily(ipAddr);
    if (family == AF_INET) {
        return INetAddr::IpType::IPV4;
    } else if (family == AF_INET6) {
        return INetAddr::IpType::IPV6;
    } else {
        return INetAddr::IpType::UNKNOWN;
    }
}

void DevInterfaceState::CreateDefaultRoute(
    const std::vector<INetAddr> &gatewayList, bool hasIpv4Addr, bool hasIpv6Addr)
{
    for (const auto &gateway : gatewayList) {
        Route route;
        auto ipType = GetIpType(gateway.address_);
        if (ipType == INetAddr::IpType::IPV4 && hasIpv4Addr) {
            route.destination_.address_ = DEFAULT_IPV4_ADDR;
        } else if (ipType == INetAddr::IpType::IPV6 && hasIpv6Addr) {
            route.destination_.address_ = DEFAULT_IPV6_ADDR;
        } else {
            continue;
        }
        route.destination_.type_ = ipType;
        route.destination_.prefixlen_ = 0;
        route.gateway_.family_ = ipType;
        route.gateway_.address_ = gateway.address_;
        // no need to add lock for linkInfo_, the func is called by UpdateLinkInfo, have locked allready.
        linkInfo_->routeList_.push_back(route);
    }
}
} // namespace NetManagerStandard
} // namespace OHOS
