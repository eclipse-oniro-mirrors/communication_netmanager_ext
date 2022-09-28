/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "netmanager_base_common_utils.h"
#include "netmgr_ext_log_wrapper.h"
#include "netsys_controller.h"
#include "route.h"
#include "static_configuration.h"

namespace OHOS {
namespace NetManagerStandard {
DevInterfaceState::DevInterfaceState()
{
    netSupplierInfo_ = std::make_unique<NetSupplierInfo>().release();
}

DevInterfaceState::~DevInterfaceState() {}

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
    linkInfo_ = linkInfo;
}

void DevInterfaceState::SetIfcfg(sptr<InterfaceConfiguration> &ifcfg)
{
    ifcfg_ = ifcfg;
    if (ifcfg_->mode_ == STATIC) {
        UpdateLinkInfo();
        if (connLinkState_ == LINK_AVAILABLE) {
            RemoteUpdateNetLinkInfo();
        }
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

sptr<NetLinkInfo> DevInterfaceState::GetLinkInfo() const
{
    return linkInfo_;
}

sptr<InterfaceConfiguration> DevInterfaceState::GetIfcfg() const
{
    return ifcfg_;
}

IPSetMode DevInterfaceState::GetIPSetMode() const
{
    if (ifcfg_ == nullptr) {
        return IPSetMode::STATIC;
    }
    return ifcfg_->mode_;
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
        if (result == 0) {
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
    if (!ret) {
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
    if (linkInfo_ == nullptr) {
        NETMGR_EXT_LOG_E("DevInterfaceCfg RemoteUpdateNetLinkInfo linkInfo_ is nullptr");
        return;
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

void DevInterfaceState::UpdateLinkInfo()
{
    if (ifcfg_ == nullptr || ifcfg_->mode_ != STATIC) {
        return;
    }
    if (linkInfo_ == nullptr) {
        linkInfo_ = std::make_unique<NetLinkInfo>().release();
    }
    std::list<INetAddr>().swap(linkInfo_->netAddrList_);
    std::list<Route>().swap(linkInfo_->routeList_);
    std::list<INetAddr>().swap(linkInfo_->dnsList_);
    linkInfo_->ifaceName_ = devName_;
    linkInfo_->netAddrList_.push_back(ifcfg_->ipStatic_.ipAddr_);
    struct Route route;
    route.iface_ = devName_;
    route.destination_ = ifcfg_->ipStatic_.route_;
    route.gateway_ = ifcfg_->ipStatic_.gateway_;
    linkInfo_->routeList_.push_back(route);
    const auto &routeLocal =
        CreateLocalRoute(devName_, ifcfg_->ipStatic_.ipAddr_.address_, ifcfg_->ipStatic_.netMask_.address_);
    linkInfo_->routeList_.push_back(routeLocal);
    for (auto it = ifcfg_->ipStatic_.dnsServers_.begin(); it != ifcfg_->ipStatic_.dnsServers_.end(); ++it) {
        linkInfo_->dnsList_.push_back(*it);
    }
}
void DevInterfaceState::UpdateLinkInfo(const INetAddr &ipAddr, const INetAddr &netMask, const INetAddr &gateWay,
                                       const INetAddr &route, const INetAddr &dns1, const INetAddr &dns2)
{
    NETMGR_EXT_LOG_D("DevInterfaceCfg::UpdateLinkInfo");
    if (linkInfo_ == nullptr) {
        linkInfo_ = std::make_unique<NetLinkInfo>().release();
    }
    std::list<INetAddr>().swap(linkInfo_->netAddrList_);
    std::list<Route>().swap(linkInfo_->routeList_);
    std::list<INetAddr>().swap(linkInfo_->dnsList_);
    linkInfo_->ifaceName_ = devName_;
    linkInfo_->netAddrList_.push_back(ipAddr);
    struct Route routeStc;
    INetAddr gate;
    INetAddr destination;
    routeStc.iface_ = devName_;
    routeStc.destination_ = route;
    routeStc.gateway_ = gateWay;
    linkInfo_->routeList_.push_back(routeStc);
    struct Route routeLocal = CreateLocalRoute(devName_, ipAddr.address_, netMask.address_);
    linkInfo_->routeList_.push_back(routeLocal);
    linkInfo_->dnsList_.push_back(dns1);
    linkInfo_->dnsList_.push_back(dns2);
}

void DevInterfaceState::UpdateSupplierAvailable()
{
    netSupplierInfo_->isAvailable_ = linkUp_;
    connLinkState_ = linkUp_ ? LINK_AVAILABLE : LINK_UNAVAILABLE;
}

Route DevInterfaceState::CreateLocalRoute(
    const std::string &iface, const std::string &ipAddr, const std::string &maskAddr)
{
    NETMGR_EXT_LOG_I("create local route ipAddr=%{public}s maskAddr=%{public}s.",
                     CommonUtils::ToAnonymousIp(ipAddr).c_str(), maskAddr.c_str());
    struct Route localRoute;
    int prefixLength = CommonUtils::GetMaskLength(maskAddr);
    unsigned int ipInt = CommonUtils::ConvertIpv4Address(ipAddr);
    unsigned int maskInt = CommonUtils::ConvertIpv4Address(maskAddr);
    std::string strLocalRoute = CommonUtils::ConvertIpv4Address(ipInt & maskInt);
    localRoute.iface_ = iface;
    localRoute.destination_.type_ = INetAddr::IPV4;
    localRoute.destination_.address_ = strLocalRoute;
    localRoute.destination_.prefixlen_ = prefixLength;
    localRoute.gateway_.address_ = "0.0.0.0";
    return localRoute;
}
} // namespace NetManagerStandard
} // namespace OHOS
