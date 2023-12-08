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

#include "net_vpn_impl.h"

#include <list>

#include "bundle_mgr_client.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "os_account_manager.h"
#include "system_ability_definition.h"

#include "net_conn_client.h"
#include "net_manager_constants.h"
#include "net_manager_ext_constants.h"
#include "netmanager_base_common_utils.h"
#include "netmgr_ext_log_wrapper.h"
#include "netsys_controller.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr int32_t INVALID_UID = -1;
constexpr int32_t IPV4_NET_MASK_MAX_LENGTH = 32;
constexpr const char *IPV4_DEFAULT_ROUTE_ADDR = "0.0.0.0";
constexpr const char *IPV6_DEFAULT_ROUTE_ADDR = "";
constexpr const char *IPV6_ROUTE_ADDR = "fe80::";
} // namespace

NetVpnImpl::NetVpnImpl(sptr<VpnConfig> config, const std::string &pkg, int32_t userId, std::vector<int32_t> &activeUserIds)
    : vpnConfig_(config), pkgName_(pkg), userId_(userId), activeUserIds_(activeUserIds)
{
    netSupplierInfo_ = new (std::nothrow) NetSupplierInfo();
    if (netSupplierInfo_ == nullptr) {
        NETMGR_EXT_LOG_E("NetSupplierInfo new failed");
    }
}

int32_t NetVpnImpl::RegisterConnectStateChangedCb(std::shared_ptr<IVpnConnStateCb> callback)
{
    if (callback == nullptr) {
        NETMGR_EXT_LOG_E("Register vpn connect callback is null.");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    connChangedCb_ = callback;
    return NETMANAGER_EXT_SUCCESS;
}

void NetVpnImpl::NotifyConnectState(const VpnConnectState &state)
{
    if (connChangedCb_ == nullptr) {
        NETMGR_EXT_LOG_E("NotifyConnectState connect callback is null.");
        return;
    }
    connChangedCb_->OnVpnConnStateChanged(state);
}

int32_t NetVpnImpl::SetUp()
{
    if (vpnConfig_ == nullptr) {
        NETMGR_EXT_LOG_E("VpnConnect vpnConfig_ is nullptr");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    NETMGR_EXT_LOG_I("SetUp interface name:%{public}s", TUN_CARD_NAME);
    VpnEventType legacy = IsInternalVpn() ? VpnEventType::TYPE_LEGACY : VpnEventType::TYPE_EXTENDED;

    auto &netConnClientIns = NetConnClient::GetInstance();
    if (!RegisterNetSupplier(netConnClientIns)) {
        VpnHisysEvent::SendFaultEventConnSetting(legacy, VpnEventErrorType::ERROR_REG_NET_SUPPLIER_ERROR,
                                                 "register Supplier failed");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }

    if (!UpdateNetSupplierInfo(netConnClientIns, true)) {
        VpnHisysEvent::SendFaultEventConnSetting(legacy, VpnEventErrorType::ERROR_UPDATE_SUPPLIER_INFO_ERROR,
                                                 "update Supplier info failed");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }

    if (!UpdateNetLinkInfo(netConnClientIns)) {
        VpnHisysEvent::SendFaultEventConnSetting(legacy, VpnEventErrorType::ERROR_UPDATE_NETLINK_INFO_ERROR,
                                                 "update link info failed");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }

    std::list<int32_t> netIdList;
    netConnClientIns.GetNetIdByIdentifier(TUN_CARD_NAME, netIdList);
    if (netIdList.size() == 0) {
        NETMGR_EXT_LOG_E("get netId failed, netId list size is 0");
        VpnHisysEvent::SendFaultEventConnSetting(legacy, VpnEventErrorType::ERROR_INTERNAL_ERROR, "get Net id failed");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    netId_ = *(netIdList.begin());
    NETMGR_EXT_LOG_I("vpn network netid: %{public}d", netId_);

    GenerateUidRanges(userId_, beginUids_, endUids_);

    for (auto &elem : activeUserIds_) {
        GenerateUidRanges(elem, beginUids_, endUids_);
    }

    if (NetsysController::GetInstance().NetworkAddUids(netId_, beginUids_, endUids_)) {
        NETMGR_EXT_LOG_E("vpn set whitelist rule error");
        VpnHisysEvent::SendFaultEventConnSetting(legacy, VpnEventErrorType::ERROR_SET_APP_UID_RULE_ERROR,
                                                 "set app uid rule failed");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }

    NotifyConnectState(VpnConnectState::VPN_CONNECTED);
    isVpnConnecting_ = true;
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetVpnImpl::ResumeUids()
{
    if (!isVpnConnecting_) {
        NETMGR_EXT_LOG_I("unecessary to resume uids");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }

    if (NetsysController::GetInstance().NetworkAddUids(netId_, beginUids_, endUids_)) {
        NETMGR_EXT_LOG_E("vpn set whitelist rule error");
        VpnEventType legacy = IsInternalVpn() ? VpnEventType::TYPE_LEGACY : VpnEventType::TYPE_EXTENDED;
        VpnHisysEvent::SendFaultEventConnSetting(legacy, VpnEventErrorType::ERROR_SET_APP_UID_RULE_ERROR,
            "set app uid rule failed");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }

    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetVpnImpl::Destroy()
{
    VpnEventType legacy = IsInternalVpn() ? VpnEventType::TYPE_LEGACY : VpnEventType::TYPE_EXTENDED;
    if (NetsysController::GetInstance().NetworkDelUids(netId_, beginUids_, endUids_)) {
        NETMGR_EXT_LOG_W("vpn remove whitelist rule error");
        VpnHisysEvent::SendFaultEventConnDestroy(legacy, VpnEventErrorType::ERROR_SET_APP_UID_RULE_ERROR,
                                                 "remove app uid rule failed");
    }

    auto &netConnClientIns = NetConnClient::GetInstance();
    DelNetLinkInfo(netConnClientIns);
    UpdateNetSupplierInfo(netConnClientIns, false);
    UnregisterNetSupplier(netConnClientIns);

    NotifyConnectState(VpnConnectState::VPN_DISCONNECTED);
    isVpnConnecting_ = false;
    return NETMANAGER_EXT_SUCCESS;
}

bool NetVpnImpl::RegisterNetSupplier(NetConnClient &netConnClientIns)
{
    if (netSupplierId_) {
        NETMGR_EXT_LOG_E("NetSupplier [%{public}d] has been registered ", netSupplierId_);
        return false;
    }
    std::set<NetCap> netCap;
    netCap.insert(NET_CAPABILITY_INTERNET);
    if (vpnConfig_->isMetered_ == false) {
        netCap.insert(NET_CAPABILITY_NOT_METERED);
    }
    if (netConnClientIns.RegisterNetSupplier(BEARER_VPN, TUN_CARD_NAME, netCap, netSupplierId_) != NETMANAGER_SUCCESS) {
        NETMGR_EXT_LOG_E("vpn netManager RegisterNetSupplier error.");
        return false;
    }
    NETMGR_EXT_LOG_I("vpn RegisterNetSupplier netSupplierId_[%{public}d]", netSupplierId_);
    return true;
}

void NetVpnImpl::UnregisterNetSupplier(NetConnClient &netConnClientIns)
{
    if (!netSupplierId_) {
        NETMGR_EXT_LOG_E("NetSupplier [%{public}d] has been unregistered ", netSupplierId_);
        return;
    }
    if (!netConnClientIns.UnregisterNetSupplier(netSupplierId_)) {
        netSupplierId_ = 0;
    }
}

bool NetVpnImpl::UpdateNetSupplierInfo(NetConnClient &netConnClientIns, bool isAvailable)
{
    if (!netSupplierId_) {
        NETMGR_EXT_LOG_E("vpn UpdateNetSupplierInfo error, netSupplierId is zero");
        return false;
    }
    if (netSupplierInfo_ == nullptr) {
        NETMGR_EXT_LOG_E("vpn UpdateNetSupplierInfo netSupplierInfo_ is nullptr");
        return false;
    }
    netSupplierInfo_->isAvailable_ = isAvailable;
    netConnClientIns.UpdateNetSupplierInfo(netSupplierId_, netSupplierInfo_);
    return true;
}

bool NetVpnImpl::UpdateNetLinkInfo(NetConnClient &netConnClientIns)
{
    if (vpnConfig_ == nullptr) {
        NETMGR_EXT_LOG_E("vpnConfig_ is nullptr");
        return false;
    }
    sptr<NetLinkInfo> linkInfo = new (std::nothrow) NetLinkInfo();
    if (linkInfo == nullptr) {
        NETMGR_EXT_LOG_E("linkInfo is nullptr");
        return false;
    }

    linkInfo->ifaceName_ = TUN_CARD_NAME;
    linkInfo->netAddrList_.assign(vpnConfig_->addresses_.begin(), vpnConfig_->addresses_.end());

    if (vpnConfig_->routes_.empty()) {
        if (vpnConfig_->isAcceptIPv4_ == true) {
            Route ipv4DefaultRoute;
            SetIpv4DefaultRoute(ipv4DefaultRoute);
            linkInfo->routeList_.emplace_back(ipv4DefaultRoute);
        }
        if (vpnConfig_->isAcceptIPv6_== true) {
            Route ipv6DefaultRoute;
            SetIpv6DefaultRoute(ipv6DefaultRoute);
            linkInfo->routeList_.emplace_back(ipv6DefaultRoute);
        }
    } else {
        linkInfo->routeList_.assign(vpnConfig_->routes_.begin(), vpnConfig_->routes_.end());
        for (auto &route : linkInfo->routeList_) {
            AdjustRouteInfo(route);
        }
    }

    for (auto dnsServer : vpnConfig_->dnsAddresses_) {
        INetAddr dns;
        if (vpnConfig_->isAcceptIPv4_ == true) {
            dns.type_ = INetAddr::IpType::IPV4;
        } else {
            dns.type_ = INetAddr::IpType::IPV6;
        }
        dns.address_ = dnsServer;
        linkInfo->dnsList_.emplace_back(dns);
    }

    for (auto domain : vpnConfig_->searchDomains_) {
        linkInfo->domain_.append(domain).append(" ");
    }
    linkInfo->mtu_ = vpnConfig_->mtu_;
    netConnClientIns.UpdateNetLinkInfo(netSupplierId_, linkInfo);
    return true;
}

void NetVpnImpl::SetIpv4DefaultRoute(Route &ipv4DefaultRoute)
{
    ipv4DefaultRoute.iface_ = TUN_CARD_NAME;
    ipv4DefaultRoute.destination_.type_ = INetAddr::IPV4;
    ipv4DefaultRoute.destination_.address_ = IPV4_DEFAULT_ROUTE_ADDR;
    ipv4DefaultRoute.destination_.prefixlen_ = CommonUtils::GetMaskLength(IPV4_DEFAULT_ROUTE_ADDR);
    ipv4DefaultRoute.gateway_.address_ = IPV4_DEFAULT_ROUTE_ADDR;
}

void NetVpnImpl::SetIpv6DefaultRoute(Route &ipv6DefaultRoute)
{
    ipv6DefaultRoute.iface_ = TUN_CARD_NAME;
    ipv6DefaultRoute.destination_.type_ = INetAddr::IPV6;
    ipv6DefaultRoute.destination_.address_ = IPV6_DEFAULT_ROUTE_ADDR;
    ipv6DefaultRoute.destination_.prefixlen_ = CommonUtils::GetMaskLength(IPV6_DEFAULT_ROUTE_ADDR);
    ipv6DefaultRoute.gateway_.address_ = IPV6_DEFAULT_ROUTE_ADDR;
}

void NetVpnImpl::DelNetLinkInfo(NetConnClient &netConnClientIns)
{
    for (auto &route : vpnConfig_->routes_) {
        AdjustRouteInfo(route);
        std::string destAddress = route.destination_.address_ + "/" + std::to_string(route.destination_.prefixlen_);
        NetsysController::GetInstance().NetworkRemoveRoute(netId_, route.iface_, destAddress, route.gateway_.address_);
    }
}

void NetVpnImpl::AdjustRouteInfo(Route &route)
{
    if (route.iface_.empty()) {
        route.iface_ = TUN_CARD_NAME;
    }
    if (vpnConfig_->isAcceptIPv4_ == true) {
        uint32_t maskUint = (0xFFFFFFFF << (IPV4_NET_MASK_MAX_LENGTH - route.destination_.prefixlen_));
        uint32_t ipAddrUint = CommonUtils::ConvertIpv4Address(route.destination_.address_);
        uint32_t subNetAddress = ipAddrUint & maskUint;
        route.destination_.address_ = CommonUtils::ConvertIpv4Address(subNetAddress);
    } else {
        route.destination_.address_ = IPV6_ROUTE_ADDR;
    }
}

void NetVpnImpl::GenerateUidRangesByAcceptedApps(const std::set<int32_t> &uids, std::vector<int32_t> &beginUids,
                                                 std::vector<int32_t> &endUids)
{
    int32_t start = INVALID_UID;
    int32_t stop = INVALID_UID;
    for (int32_t uid : uids) {
        if (start == INVALID_UID) {
            start = uid;
        } else if (uid != stop + 1) {
            beginUids.push_back(start);
            endUids.push_back(stop);
            start = uid;
        }
        stop = uid;
    }
    if (start != INVALID_UID) {
        beginUids.push_back(start);
        endUids.push_back(stop);
    }
}

void NetVpnImpl::GenerateUidRangesByRefusedApps(int32_t userId, const std::set<int32_t> &uids, std::vector<int32_t> &beginUids,
                                                std::vector<int32_t> &endUids)
{
    int32_t start = userId * AppExecFwk::Constants::BASE_USER_RANGE;
    int32_t stop = userId * AppExecFwk::Constants::BASE_USER_RANGE + AppExecFwk::Constants::MAX_APP_UID;
    for (int32_t uid : uids) {
        if (uid == start) {
            start++;
        } else {
            beginUids.push_back(start);
            endUids.push_back(uid - 1);
            start = uid + 1;
        }
    }
    if (start <= stop) {
        beginUids.push_back(start);
        endUids.push_back(stop);
    }
}

std::set<int32_t> NetVpnImpl::GetAppsUids(int32_t userId, const std::vector<std::string> &applications)
{
    std::set<int32_t> uids;
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        NETMGR_EXT_LOG_E("systemAbilityManager is null.");
        return uids;
    }
    auto bundleMgrSa = systemAbilityManager->GetSystemAbility(OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bundleMgrSa == nullptr) {
        NETMGR_EXT_LOG_E("bundleMgrSa is null.");
        return uids;
    }
    auto bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(bundleMgrSa);
    if (bundleMgr == nullptr) {
        NETMGR_EXT_LOG_E("iface_cast is null.");
        return uids;
    }

    NETMGR_EXT_LOG_I("userId: %{public}d.", userId);
    AppExecFwk::ApplicationFlag flags = AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO;
    for (auto app : applications) {
        AppExecFwk::ApplicationInfo appInfo;
        if (bundleMgr->GetApplicationInfo(app, flags, userId, appInfo)) {
            NETMGR_EXT_LOG_I("app: %{public}s success, uid=%{public}d.", app.c_str(), appInfo.uid);
            uids.insert(appInfo.uid);
        } else {
            NETMGR_EXT_LOG_E("app: %{public}s error.", app.c_str());
        }
    }
    NETMGR_EXT_LOG_I("uids.size: %{public}zd.", uids.size());
    return uids;
}

int32_t NetVpnImpl::GenerateUidRanges(int32_t userId, std::vector<int32_t> &beginUids, std::vector<int32_t> &endUids)
{
    NETMGR_EXT_LOG_I("GenerateUidRanges userId:%{public}d.", userId);
    if (userId == AppExecFwk::Constants::INVALID_USERID) {
        userId = AppExecFwk::Constants::START_USERID;
    }
    if (vpnConfig_->acceptedApplications_.size()) {
        std::set<int32_t> uids = GetAppsUids(userId, vpnConfig_->acceptedApplications_);
        GenerateUidRangesByAcceptedApps(uids, beginUids, endUids);
    } else if (vpnConfig_->refusedApplications_.size()) {
        std::set<int32_t> uids = GetAppsUids(userId, vpnConfig_->refusedApplications_);
        GenerateUidRangesByRefusedApps(userId, uids, beginUids, endUids);
    } else {
        int32_t start = userId * AppExecFwk::Constants::BASE_USER_RANGE;
        int32_t stop = userId * AppExecFwk::Constants::BASE_USER_RANGE + AppExecFwk::Constants::MAX_APP_UID;
        beginUids.push_back(start);
        endUids.push_back(stop);
        NETMGR_EXT_LOG_I("GenerateUidRanges default all app, uid range: %{public}d -- %{public}d.", start, stop);
    }
    return NETMANAGER_EXT_SUCCESS;
}

} // namespace NetManagerStandard
} // namespace OHOS
