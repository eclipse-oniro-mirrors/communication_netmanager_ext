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
#include "net_conn_client.h"
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
    std::lock_guard<ffrt::mutex> locker(netLinkInfoMutex_);
    NetLinkInfo oldLinkInfo;
    if (!devState->GetLinkInfo(oldLinkInfo)) {
        NETMGR_EXT_LOG_W("EthernetLanManagement:GetOldLinkInfo fail due to linkInfo is NULL");
        oldLinkInfo.Initialize();
        lanLinkInfoMap_[devState->GetDevName()] = oldLinkInfo;
        return;
    }
    lanLinkInfoMap_[devState->GetDevName()] = oldLinkInfo;
}

int32_t EthernetLanManagement::UpdateLanLinkInfo(sptr<DevInterfaceState> &devState)
{
    if (devState == nullptr) {
        NETMGR_EXT_LOG_D("EthernetLanManagement:UpdateLanLinkInfo fail due to devState is nullptr");
        return NETMANAGER_ERR_INTERNAL;
    }
    if (!devState->GetLinkUp()) {
        NETMGR_EXT_LOG_D("EthernetLanManagement:UpdateLanLinkInfo fail due to not link up");
        return ETHERNET_ERR_DEVICE_NOT_LINK;
    }
    std::lock_guard<ffrt::mutex> locker(netLinkInfoMutex_);
    NetLinkInfo newNetLinkInfo;
    if (!devState->GetLinkInfo(newNetLinkInfo)) {
        NETMGR_EXT_LOG_E("EthernetLanManagement:UpdateLanLinkInfo fail due to newNetLinkInfo is NULL");
        return NETMANAGER_ERR_INTERNAL;
    }
    NetLinkInfo oldNetLinkInfo;
    auto it = lanLinkInfoMap_.find(newNetLinkInfo.ifaceName_);
    if (it != lanLinkInfoMap_.end()) {
        oldNetLinkInfo = it->second;
    }
    int32_t ret = NETMANAGER_SUCCESS;
    ret += DelIp(oldNetLinkInfo, newNetLinkInfo);
    ret += SetIp(oldNetLinkInfo, newNetLinkInfo);
    ret += DelRoute(oldNetLinkInfo, newNetLinkInfo);
    ret += SetRoute(oldNetLinkInfo, newNetLinkInfo);
    if (ret == NETMANAGER_SUCCESS) {
        lanLinkInfoMap_[newNetLinkInfo.ifaceName_] = newNetLinkInfo;
    } else {
        /* keep the previous cached state so the next update retries the failed diff
         * (delete operations are idempotent, redoing them is harmless) */
        NETMGR_EXT_LOG_E("update lan[%{public}s] linkinfo partially failed, keep cached state for retry",
                         newNetLinkInfo.ifaceName_.c_str());
    }
    return ret;
}

int32_t EthernetLanManagement::ReleaseLanNetLink(sptr<DevInterfaceState> &devState)
{
    NETMGR_EXT_LOG_D("EthernetLanManagement:ReleaseLanNetLink...");
    if (devState == nullptr) {
        NETMGR_EXT_LOG_D("EthernetLanManagement:ReleaseLanNetLink fail due to devState is nullptr");
        return NETMANAGER_ERR_INTERNAL;
    }
    NetLinkInfo newNetLinkInfo;
    if (!devState->GetLinkInfo(newNetLinkInfo)) {
        NETMGR_EXT_LOG_E("EthernetLanManagement:ReleaseLanNetLink fail due to newNetLinkInfo is NULL");
        return NETMANAGER_ERR_INTERNAL;
    }
    int32_t ret = NETMANAGER_SUCCESS;
    for (const auto &inetAddr : newNetLinkInfo.netAddrList_) {
        auto family = CommonUtils::GetAddrFamily(inetAddr.address_);
        auto prefixLen = inetAddr.prefixlen_ ? static_cast<int32_t>(inetAddr.prefixlen_)
                                             : ((family == AF_INET6) ? CommonUtils::Ipv6PrefixLen(inetAddr.netMask_)
                                                                     : CommonUtils::Ipv4PrefixLen(inetAddr.netMask_));
        ret += NetConnClient::GetInstance().DelInterfaceAddress(newNetLinkInfo.ifaceName_,
                                                                inetAddr.address_, prefixLen);
        if (ret != NETMANAGER_SUCCESS) {
            NETMGR_EXT_LOG_E("del lan interface[%{public}s] address[%{private}s] failed",
                             newNetLinkInfo.ifaceName_.c_str(), inetAddr.address_.c_str());
        }
    }
    for (const auto &route : newNetLinkInfo.routeList_) {
        std::string destAddress = route.destination_.address_ + "/" + std::to_string(route.destination_.prefixlen_);
        ret += NetConnClient::GetInstance().RemoveNetworkRoute(LOCAL_NET_ID, route.iface_, destAddress,
                                                               route.gateway_.address_);
        if (ret != NETMANAGER_SUCCESS) {
            NETMGR_EXT_LOG_E("del lan[%{public}s] route failed, destAddress[%{private}s], nexthop[%{private}s]",
                             route.iface_.c_str(), destAddress.c_str(), route.gateway_.address_.c_str());
        }
    }
    /* addresses and routes of this interface have been removed from kernel, drop cached state */
    {
        std::lock_guard<ffrt::mutex> locker(netLinkInfoMutex_);
        lanLinkInfoMap_.erase(newNetLinkInfo.ifaceName_);
    }
    return ret;
}

int32_t EthernetLanManagement::SetIp(const NetLinkInfo &oldNetLinkInfo, const NetLinkInfo &newNetLinkInfo)
{
    NETMGR_EXT_LOG_D("EthernetLanManagement:SetIp...");
    for (const auto &inetAddr : newNetLinkInfo.netAddrList_) {
        if (oldNetLinkInfo.HasNetAddr(inetAddr)) {
            NETMGR_EXT_LOG_W("Same ip address:[%{public}s], there is no need to add it again",
                             CommonUtils::ToAnonymousIp(inetAddr.address_).c_str());
            continue;
        }
        auto family = CommonUtils::GetAddrFamily(inetAddr.address_);
        auto prefixLen = inetAddr.prefixlen_ ? static_cast<int32_t>(inetAddr.prefixlen_)
                                             : ((family == AF_INET6) ? CommonUtils::Ipv6PrefixLen(inetAddr.netMask_)
                                                                     : CommonUtils::Ipv4PrefixLen(inetAddr.netMask_));
        auto ret = NetConnClient::GetInstance().AddInterfaceAddress(newNetLinkInfo.ifaceName_,
                                                                    inetAddr.address_, prefixLen);
        if (ret != NETMANAGER_SUCCESS) {
            NETMGR_EXT_LOG_E("set lan interface address failed");
            return ret;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t EthernetLanManagement::DelIp(const NetLinkInfo &oldNetLinkInfo, const NetLinkInfo &newNetLinkInfo)
{
    NETMGR_EXT_LOG_D("EthernetLanManagement:DelIp...");
    for (const auto &inetAddr : oldNetLinkInfo.netAddrList_) {
        if (newNetLinkInfo.HasNetAddr(inetAddr)) {
            NETMGR_EXT_LOG_W("Same ip address:[%{public}s], there is not need to be deleted",
                             CommonUtils::ToAnonymousIp(inetAddr.address_).c_str());
            continue;
        }
        auto family = CommonUtils::GetAddrFamily(inetAddr.address_);
        auto prefixLen = inetAddr.prefixlen_ ? static_cast<int32_t>(inetAddr.prefixlen_)
                                             : ((family == AF_INET6) ? CommonUtils::Ipv6PrefixLen(inetAddr.netMask_)
                                                                     : CommonUtils::Ipv4PrefixLen(inetAddr.netMask_));
        auto ret = NetConnClient::GetInstance().DelInterfaceAddress(oldNetLinkInfo.ifaceName_,
                                                                    inetAddr.address_, prefixLen);
        if (ret != NETMANAGER_SUCCESS) {
            NETMGR_EXT_LOG_E("del lan interface address failed");
            return ret;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t EthernetLanManagement::SetRoute(const NetLinkInfo &oldNetLinkInfo, const NetLinkInfo &newNetLinkInfo)
{
    for (const auto &route : newNetLinkInfo.routeList_) {
        if (oldNetLinkInfo.HasRoute(route)) {
            NETMGR_EXT_LOG_W("Same route:[%{public}s]  ifo, there is no need to add it again",
                             CommonUtils::ToAnonymousIp(route.destination_.address_).c_str());
            continue;
        }
        std::string destAddress = route.destination_.address_ + "/" + std::to_string(route.destination_.prefixlen_);
        auto ret = NetConnClient::GetInstance().AddNetworkRoute(LOCAL_NET_ID, route.iface_, destAddress,
                                                                route.gateway_.address_);
        if (ret != NETMANAGER_SUCCESS) {
            NETMGR_EXT_LOG_E("Set lan route failed");
            return ret;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t EthernetLanManagement::DelRoute(const NetLinkInfo &oldNetLinkInfo, const NetLinkInfo &newNetLinkInfo)
{
    for (const auto &route : oldNetLinkInfo.routeList_) {
        if (newNetLinkInfo.HasRoute(route)) {
            NETMGR_EXT_LOG_W("Same route:[%{public}s]  ifo, there is not need to be deleted",
                             CommonUtils::ToAnonymousIp(route.destination_.address_).c_str());
            continue;
        }
        std::string destAddress = route.destination_.address_ + "/" + std::to_string(route.destination_.prefixlen_);
        auto ret = NetConnClient::GetInstance().RemoveNetworkRoute(LOCAL_NET_ID, route.iface_, destAddress,
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
