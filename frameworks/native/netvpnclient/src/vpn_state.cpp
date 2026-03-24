/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "vpn_state.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {

namespace {
constexpr uint32_t ROUTE_MAX_SIZE = 2000;
constexpr uint32_t DNS_MAX_COUNT = 64;
}

VpnState::VpnState(const std::string &vpnIfName, const std::string &vpnIfAddr,
                   const std::string &vpnId, bool isGlobalVpn, const std::vector<Route> &routes,
                   const std::vector<std::string> &dnsServers):
    vpnIfName_(vpnIfName),
    vpnIfAddr_(vpnIfAddr),
    vpnId_(vpnId),
    isGlobalVpn_(isGlobalVpn),
    routes_(routes),
    dnsServers_(dnsServers)
{}

bool VpnState::Marshalling(Parcel &parcel) const
{
    bool allOK = parcel.WriteString(vpnIfName_) &&
                 parcel.WriteString(vpnIfAddr_) &&
                 parcel.WriteString(vpnId_) &&
                 parcel.WriteString(vpnPacketName_) &&
                 parcel.WriteBool(isGlobalVpn_) &&
                 MarshallingRoute(parcel) &&
                 MarshallingDns(parcel);
    return allOK;
}

bool VpnState::MarshallingRoute(Parcel &parcel) const
{
    int32_t routeSize = static_cast<int32_t>(routes_.size());

    // LCOV_EXCL_START
    if (!parcel.WriteInt32(routeSize)) {
        return false;
    }
    // LCOV_EXCL_STOP

    for (auto route : routes_) {
        if (!route.Marshalling(parcel)) {
            return false;
        }
    }
    return true;
}

bool VpnState::MarshallingDns(Parcel &parcel) const
{
    uint32_t size = static_cast<uint32_t>(dnsServers_.size());
    if (size > DNS_MAX_COUNT) {
        return false;
    }

    // LCOV_EXCL_START
    if (!parcel.WriteUint32(size)) {
        return false;
    }
    // LCOV_EXCL_STOP

    for (auto &elem : dnsServers_) {
        // LCOV_EXCL_START
        if (!parcel.WriteString(elem)) {
            return false;
        }
        // LCOV_EXCL_STOP
    }
    return true;
}

VpnState* VpnState::Unmarshalling(Parcel &parcel)
{
    std::unique_ptr<VpnState> ptr = std::make_unique<VpnState>();

    // LCOV_EXCL_START
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("ptr is null");
        return nullptr;
    }
    // LCOV_EXCL_STOP

    bool allOK = UnmarshallingVpnState(parcel, ptr.get());
    return allOK ? ptr.release() : nullptr;
}

bool VpnState::UnmarshallingVpnState(Parcel &parcel, VpnState* ptr)
{
    // LCOV_EXCL_START
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("VpnConfig ptr is null");
        return false;
    }
    // LCOV_EXCL_STOP
    bool allOK = parcel.ReadString(ptr->vpnIfName_) &&
                 parcel.ReadString(ptr->vpnIfAddr_) &&
                 parcel.ReadString(ptr->vpnId_) &&
                 parcel.ReadString(ptr->vpnPacketName_) &&
                 parcel.ReadBool(ptr->isGlobalVpn_) &&
                 UnmarshallingRoute(parcel, ptr) &&
                 UnmarshallingDns(parcel, ptr);
    return allOK;
}

bool VpnState::UnmarshallingRoute(Parcel &parcel, VpnState* config)
{
    int32_t routeSize = 0;
    // LCOV_EXCL_START
    if (!parcel.ReadInt32(routeSize)) {
        NETMGR_EXT_LOG_E("read route size failed");
        return false;
    }
    // LCOV_EXCL_STOP
    if (static_cast<uint32_t>(routeSize) > ROUTE_MAX_SIZE) {
        NETMGR_EXT_LOG_E("routeSize=[%{public}d] is too large", routeSize);
        return false;
    }
    for (int32_t idx = 0; idx < routeSize; idx++) {
        sptr<Route> route = Route::Unmarshalling(parcel);
        if (route == nullptr) {
            NETMGR_EXT_LOG_E("route is null");
            return false;
        }
        config->routes_.push_back(*route);
    }
    return true;
}

bool VpnState::UnmarshallingDns(Parcel &parcel, VpnState* config)
{
    uint32_t size = 0;
    // LCOV_EXCL_START
    if (!parcel.ReadUint32(size)) {
        return false;
    }
    // LCOV_EXCL_STOP

    if (size > DNS_MAX_COUNT) {
        NETMGR_EXT_LOG_E("size = [%{public}d] is too large", size);
        return false;
    }

    for (uint32_t idx = 0; idx < size; idx++) {
        std::string elem;
        // LCOV_EXCL_START
        if (!parcel.ReadString(elem)) {
            return false;
        }
        // LCOV_EXCL_STOP
        config->dnsServers_.push_back(elem);
    }

    return true;
}

} // namespace NetManagerStandard
} // namespace OHOS
