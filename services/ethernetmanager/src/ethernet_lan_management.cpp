/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "ethernet_lan_management.h"
#include "netmgr_ext_log_wrapper.h"
#include "netsys_controller.h"
#include "netmanager_base_common_utils.h"
#include "route.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr const int32_t LOCAL_NET_ID = 99;
} // namespace

EthernetLanManagement::EthernetLanManagement()
{
}

void EthernetLanManagement::GetOldLinkInfo(sptr<DevInterfaceState> &devState)
{
    if (devState == nullptr) {
        NETMGR_EXT_LOG_D("EthernetLanManagement:GetOldLinkInfo fail due to devState is nullptr");
        return;
    }
    netLinkInfo_ = *(devState->GetLinkInfo());
}

void EthernetLanManagement::UpdateLanLinkInfo(sptr<DevInterfaceState> &devState)
{
    if (devState == nullptr) {
        NETMGR_EXT_LOG_D("EthernetLanManagement:UpdateLanLinkInfo fail due to devState is nullptr");
        return;
    }
    if (!devState->GetLinkUp()) {
        NETMGR_EXT_LOG_D("EthernetLanManagement:UpdateLanLinkInfo fail due to not link up");
        return;
    }
    NetLinkInfo newNetLinkInfo = *(devState->GetLinkInfo());
    DelIp(newNetLinkInfo);
    SetIp(newNetLinkInfo);
    DelRoute(newNetLinkInfo);
    SetRoute(newNetLinkInfo);
}

void EthernetLanManagement::ReleaseLanNetLink(sptr<DevInterfaceState> &devState)
{
    NETMGR_EXT_LOG_D("EthernetLanManagement:ReleaseLanNetLink...");
    if (devState == nullptr) {
        NETMGR_EXT_LOG_D("EthernetLanManagement:ReleaseLanNetLink fail due to devState is nullptr");
        return;
    }
    NetLinkInfo newNetLinkInfo = *(devState->GetLinkInfo());
    for (const auto &inetAddr : newNetLinkInfo.netAddrList_) {
        auto family = CommonUtils::GetAddrFamily(inetAddr.address_);
        auto prefixLen = inetAddr.prefixlen_ ? static_cast<int32_t>(inetAddr.prefixlen_)
                                             : ((family == AF_INET6) ? CommonUtils::Ipv6PrefixLen(inetAddr.netMask_)
                                                                     : CommonUtils::Ipv4PrefixLen(inetAddr.netMask_));
        int ret = NetsysController::GetInstance().DelInterfaceAddress(newNetLinkInfo.ifaceName_,
                                                                      inetAddr.address_, prefixLen);
        if (ret != NETMANAGER_SUCCESS) {
            NETMGR_EXT_LOG_E("del lan interface[%{public}s] address[%{private}s] failed",
                             newNetLinkInfo.ifaceName_.c_str(), inetAddr.address_.c_str());
        }
    }
    for (const auto &route : newNetLinkInfo.routeList_) {
        std::string destAddress = route.destination_.address_ + "/" + std::to_string(route.destination_.prefixlen_);
        auto ret = NetsysController::GetInstance().NetworkRemoveRoute(LOCAL_NET_ID, route.iface_, destAddress,
                                                                      route.gateway_.address_);
        if (ret != NETMANAGER_SUCCESS) {
            NETMGR_EXT_LOG_E("del lan[%{public}s] route failed, destAddress[%{private}s], nexthop[%{private}s]",
                             route.iface_.c_str(), destAddress.c_str(), route.gateway_.address_.c_str());
        }
    }
}

int32_t EthernetLanManagement::SetIp(const NetLinkInfo &newNetLinkInfo)
{
    NETMGR_EXT_LOG_D("EthernetLanManagement:SetIp...");
    for (const auto &inetAddr : newNetLinkInfo.netAddrList_) {
        if (netLinkInfo_.HasNetAddr(inetAddr)) {
            NETMGR_EXT_LOG_W("Same ip address:[%{public}s], there is no need to add it again",
                             CommonUtils::ToAnonymousIp(inetAddr.address_).c_str());
            continue;
        }
        auto family = CommonUtils::GetAddrFamily(inetAddr.address_);
        auto prefixLen = inetAddr.prefixlen_ ? static_cast<int32_t>(inetAddr.prefixlen_)
                                             : ((family == AF_INET6) ? CommonUtils::Ipv6PrefixLen(inetAddr.netMask_)
                                                                     : CommonUtils::Ipv4PrefixLen(inetAddr.netMask_));
        int ret = NetsysController::GetInstance().AddInterfaceAddress(newNetLinkInfo.ifaceName_,
                                                                      inetAddr.address_, prefixLen);
        if (ret != NETMANAGER_SUCCESS) {
            NETMGR_EXT_LOG_E("set lan interface address failed");
            return ret;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t EthernetLanManagement::DelIp(const NetLinkInfo &newNetLinkInfo)
{
    NETMGR_EXT_LOG_D("EthernetLanManagement:DelIp...");
    for (const auto &inetAddr : netLinkInfo_.netAddrList_) {
        if (newNetLinkInfo.HasNetAddr(inetAddr)) {
            NETMGR_EXT_LOG_W("Same ip address:[%{public}s], there is not need to be deleted",
                             CommonUtils::ToAnonymousIp(inetAddr.address_).c_str());
            continue;
        }
        auto family = CommonUtils::GetAddrFamily(inetAddr.address_);
        auto prefixLen = inetAddr.prefixlen_ ? static_cast<int32_t>(inetAddr.prefixlen_)
                                             : ((family == AF_INET6) ? CommonUtils::Ipv6PrefixLen(inetAddr.netMask_)
                                                                     : CommonUtils::Ipv4PrefixLen(inetAddr.netMask_));
        int ret = NetsysController::GetInstance().DelInterfaceAddress(netLinkInfo_.ifaceName_,
                                                                      inetAddr.address_, prefixLen);
        if (ret != NETMANAGER_SUCCESS) {
            NETMGR_EXT_LOG_E("del lan interface address failed");
            return ret;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t EthernetLanManagement::SetRoute(const NetLinkInfo &newNetLinkInfo)
{
    for (const auto &route : newNetLinkInfo.routeList_) {
        if (netLinkInfo_.HasRoute(route)) {
            NETMGR_EXT_LOG_W("Same route:[%{public}s]  ifo, there is no need to add it again",
                             CommonUtils::ToAnonymousIp(route.destination_.address_).c_str());
            continue;
        }
        std::string destAddress = route.destination_.address_ + "/" + std::to_string(route.destination_.prefixlen_);
        auto ret = NetsysController::GetInstance().NetworkAddRoute(LOCAL_NET_ID, route.iface_, destAddress,
                                                                   route.gateway_.address_);
        if (ret != NETMANAGER_SUCCESS) {
            NETMGR_EXT_LOG_E("Set lan route failed");
            return ret;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t EthernetLanManagement::DelRoute(const NetLinkInfo &newNetLinkInfo)
{
    for (const auto &route : netLinkInfo_.routeList_) {
        if (newNetLinkInfo.HasRoute(route)) {
            NETMGR_EXT_LOG_W("Same route:[%{public}s]  ifo, there is not need to be deleted",
                             CommonUtils::ToAnonymousIp(route.destination_.address_).c_str());
            continue;
        }
        std::string destAddress = route.destination_.address_ + "/" + std::to_string(route.destination_.prefixlen_);
        auto ret = NetsysController::GetInstance().NetworkRemoveRoute(LOCAL_NET_ID, route.iface_, destAddress,
                                                                      route.gateway_.address_);
        if (ret != NETMANAGER_SUCCESS) {
            NETMGR_EXT_LOG_E("del lan route failed");
            return ret;
        }
    }
    return NETMANAGER_SUCCESS;
}

} // namespace NetManagerStandard
} // namespace OHOS

