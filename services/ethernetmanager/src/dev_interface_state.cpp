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

#include "netsys_controller.h"
#include "net_manager_center.h"
#include "netmgr_ext_log_wrapper.h"

#include "netLink_rtnl.h"
#include "ethernet_constants.h"

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

void DevInterfaceState::SetDevHWaddr(const std::vector<uint8_t> &hwAddr)
{
    devHWaddr_ = hwAddr;
}

void DevInterfaceState::SetNetCaps(const std::set<NetCap> &netCaps)
{
    netCaps_ = netCaps;
}

void DevInterfaceState::SetLinkUp(bool up)
{
    linkUp_ = up;
}

void DevInterfaceState::SetLowerUp(bool lowerUp)
{
    lowerUp_ = lowerUp;
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
        SetIpAddr();
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

std::vector<uint8_t> DevInterfaceState::GetHWaddr() const
{
    return devHWaddr_;
}

const std::set<NetCap> & DevInterfaceState::GetNetCaps() const
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

bool DevInterfaceState::GetLowerUp() const
{
    return lowerUp_;
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
        netCaps_.insert(NET_CAPABILITY_INTERNET);
        int32_t result = NetManagerCenter::GetInstance().RegisterNetSupplier(bearerType_,
            devName_, netCaps_, netSupplier_);
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
    for (auto it = ifcfg_->ipStatic_.dnsServers_.begin(); it != ifcfg_->ipStatic_.dnsServers_.end(); ++it) {
        linkInfo_->dnsList_.push_back(*it);
    }
}

void DevInterfaceState::UpdateLinkInfo(const INetAddr &ipAddr, const INetAddr &gateWay,
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
    linkInfo_->dnsList_.push_back(dns1);
    linkInfo_->dnsList_.push_back(dns2);
}

void DevInterfaceState::SetIpAddr()
{
    if (!(ifcfg_->ipStatic_.ipAddr_.address_.empty())) {
        NETMGR_EXT_LOG_D("DevInterfaceState SetIpAddr devName[%{private}s] address[%{private}s] prefixlen[%{private}d]",
            devName_.c_str(), ifcfg_->ipStatic_.ipAddr_.address_.c_str(), ifcfg_->ipStatic_.ipAddr_.prefixlen_);
        NetsysController::GetInstance().InterfaceAddAddress(devName_, ifcfg_->ipStatic_.ipAddr_.address_,
            ifcfg_->ipStatic_.ipAddr_.prefixlen_);
    }
}

void  DevInterfaceState::UpdateSupplierAvailable()
{
    bool isAvailable = linkUp_ & lowerUp_;
    netSupplierInfo_->isAvailable_ = isAvailable;
    if (isAvailable) {
        connLinkState_ = LINK_AVAILABLE;
    } else {
        connLinkState_ = LINK_UNAVAILABLE;
    }
}
} // namespace NetManagerStandard
} // namespace OHOS