/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
EthernetLanManagement::EthernetLanManagement(){}

int32_t LOCAL_NET_ID = 99;
constexpr const char *LOCAL_ROUTE_NEXT_HOP = "0.0.0.0";
constexpr const char *LOCAL_ROUTE_IPV6_DESTINATION = "::";

int32_t EthernetLanManagement::UpdateLanLinkInfo(const NetLinkInfo &newNetLinkInfo){
    DelIp(newNetLinkInfo);
    SetIp(newNetLinkInfo);
    DelRoute(newNetLinkInfo);
    SetRoute(newNetLinkInfo);
    netLinkInfo_ = newNetLinkInfo;
    return 0;
}

int32_t EthernetLanManagement::SetIp(const NetLinkInfo &newNetLinkInfo){
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
        if (NETMANAGER_SUCCESS != NetsysController::GetInstance().AddInterfaceAddress(newNetLinkInfo.ifaceName_,
                                                                                        inetAddr.address_, prefixLen)) {
            NETMGR_EXT_LOG_E("set interface address failed");
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t EthernetLanManagement::DelIp(const NetLinkInfo &newNetLinkInfo){
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
        if (NETMANAGER_SUCCESS != NetsysController::GetInstance().DelInterfaceAddress(netLinkInfo_.ifaceName_,
                                                                                      inetAddr.address_, prefixLen)) {
            NETMGR_EXT_LOG_E("del interface address failed");
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t EthernetLanManagement::SetRoute(const NetLinkInfo &newNetLinkInfo){
    for (const auto &route : newNetLinkInfo.routeList_) {
        if (netLinkInfo_.HasRoute(route)) {
            NETMGR_EXT_LOG_W("Same route:[%{public}s]  ifo, there is no need to add it again",
                         CommonUtils::ToAnonymousIp(route.destination_.address_).c_str());
            continue;
        }

        std::string destAddress = route.destination_.address_ + "/" + std::to_string(route.destination_.prefixlen_);
        auto ret =
            NetsysController::GetInstance().NetworkAddRoute(LOCAL_NET_ID, route.iface_, destAddress, route.gateway_.address_);
        int32_t res = NETMANAGER_SUCCESS;
        if (route.destination_.address_ != LOCAL_ROUTE_NEXT_HOP &&
            route.destination_.address_ != LOCAL_ROUTE_IPV6_DESTINATION) {
            auto family = CommonUtils::GetAddrFamily(route.destination_.address_);
            std::string nextHop = (family == AF_INET6) ? "" : LOCAL_ROUTE_NEXT_HOP;
            res = NetsysController::GetInstance().NetworkAddRoute(LOCAL_NET_ID, route.iface_, destAddress, nextHop);
        }
        if (ret != NETMANAGER_SUCCESS || res != NETMANAGER_SUCCESS) {
            NETMGR_EXT_LOG_E("Set route failed]");
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t EthernetLanManagement::DelRoute(const NetLinkInfo &newNetLinkInfo){
    for (const auto &route : netLinkInfo_.routeList_) {
        if (newNetLinkInfo.HasRoute(route)) {
            NETMGR_EXT_LOG_W("Same route:[%{public}s]  ifo, there is not need to be deleted",
                         CommonUtils::ToAnonymousIp(route.destination_.address_).c_str());
            continue;
        }
        std::string destAddress = route.destination_.address_ + "/" + std::to_string(route.destination_.prefixlen_);
        auto ret = NetsysController::GetInstance().NetworkRemoveRoute(LOCAL_NET_ID, route.iface_, destAddress,
                                                                      route.gateway_.address_);
        int32_t res = NETMANAGER_SUCCESS;
        if (route.destination_.address_ != LOCAL_ROUTE_NEXT_HOP &&
            route.destination_.address_ != LOCAL_ROUTE_IPV6_DESTINATION) {
            auto family = CommonUtils::GetAddrFamily(route.destination_.address_);
            std::string nextHop = (family == AF_INET6) ? "" : LOCAL_ROUTE_NEXT_HOP;
            res = NetsysController::GetInstance().NetworkRemoveRoute(LOCAL_NET_ID, route.iface_, destAddress, nextHop);
        }
        if (ret != NETMANAGER_SUCCESS || res != NETMANAGER_SUCCESS) {
            NETMGR_EXT_LOG_E("del route failed");
        }
    }
    return NETMANAGER_SUCCESS;
}

} // namespace NetManagerStandard
} // namespace OHOS

