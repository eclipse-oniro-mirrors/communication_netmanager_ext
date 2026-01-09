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

#include "vpn_config.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr uint32_t MAX_SIZE = 64;
constexpr uint32_t APP_MAX_SIZE = 256;
constexpr uint32_t ROUTE_MAX_SIZE = 10000;
constexpr uint32_t ADDR_MAX_SIZE = 2000;
}
bool VpnConfig::Marshalling(Parcel &parcel) const
{
    bool allOK = parcel.WriteString(vpnId_) &&
                 MarshallingAddrRoute(parcel) && parcel.WriteInt32(mtu_) && parcel.WriteBool(isAcceptIPv4_) &&
                 parcel.WriteBool(isAcceptIPv6_) && parcel.WriteBool(isLegacy_) && parcel.WriteBool(isMetered_) &&
                 parcel.WriteBool(isBlocking_) && MarshallingVectorString(parcel, dnsAddresses_, MAX_SIZE) &&
                 MarshallingVectorString(parcel, searchDomains_, MAX_SIZE) &&
                 MarshallingVectorString(parcel, acceptedApplications_, APP_MAX_SIZE) &&
                 MarshallingVectorString(parcel, refusedApplications_, APP_MAX_SIZE);
    return allOK;
}

bool VpnConfig::MarshallingAddrRoute(Parcel &parcel) const
{
    int32_t addrSize = static_cast<int32_t>(addresses_.size());
    if (!parcel.WriteInt32(addrSize)) {
        return false;
    }
    for (auto addr : addresses_) {
        if (!addr.Marshalling(parcel)) {
            return false;
        }
    }

    int32_t routeSize = static_cast<int32_t>(routes_.size());
    if (!parcel.WriteInt32(routeSize)) {
        return false;
    }

    for (auto route : routes_) {
        if (!route.Marshalling(parcel)) {
            return false;
        }
    }
    return true;
}

bool VpnConfig::MarshallingVectorString(Parcel &parcel, const std::vector<std::string> &vec, uint32_t maxSize) const
{
    uint32_t size = static_cast<uint32_t>(vec.size());
    if (size > maxSize) {
        return false;
    }
    if (!parcel.WriteUint32(size)) {
        return false;
    }
    for (auto &elem : vec) {
        if (!parcel.WriteString(elem)) {
            return false;
        }
    }
    return true;
}

VpnConfig* VpnConfig::Unmarshalling(Parcel &parcel)
{
    std::unique_ptr<VpnConfig> ptr = std::make_unique<VpnConfig>();
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("ptr is null");
        return nullptr;
    }

    bool allOK = UnmarshallingVpnConfig(parcel, ptr.get());
    return allOK ? ptr.release() : nullptr;
}

bool VpnConfig::UnmarshallingVpnConfig(Parcel &parcel, VpnConfig* ptr)
{
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("VpnConfig ptr is null");
        return false;
    }
    bool allOK = parcel.ReadString(ptr->vpnId_) &&
                 UnmarshallingAddrRoute(parcel, ptr) && parcel.ReadInt32(ptr->mtu_) &&
                 parcel.ReadBool(ptr->isAcceptIPv4_) && parcel.ReadBool(ptr->isAcceptIPv6_) &&
                 parcel.ReadBool(ptr->isLegacy_) && parcel.ReadBool(ptr->isMetered_) &&
                 parcel.ReadBool(ptr->isBlocking_) && UnmarshallingVectorString(parcel, ptr->dnsAddresses_, MAX_SIZE) &&
                 UnmarshallingVectorString(parcel, ptr->searchDomains_, MAX_SIZE) &&
                 UnmarshallingVectorString(parcel, ptr->acceptedApplications_, APP_MAX_SIZE) &&
                 UnmarshallingVectorString(parcel, ptr->refusedApplications_, APP_MAX_SIZE);
    return allOK;
}

bool VpnConfig::UnmarshallingAddrRoute(Parcel &parcel, VpnConfig* config)
{
    int32_t addrSize = 0;
    if (!parcel.ReadInt32(addrSize)) {
        return false;
    }
    if (static_cast<uint32_t>(addrSize) > ADDR_MAX_SIZE) {
        NETMGR_EXT_LOG_E("addrSize=[%{public}d] is too large", addrSize);
        return false;
    }
    for (int32_t idx = 0; idx < addrSize; idx++) {
        sptr<INetAddr> address = INetAddr::Unmarshalling(parcel);
        if (address == nullptr) {
            NETMGR_EXT_LOG_E("address is null");
            return false;
        }
        config->addresses_.push_back(*address);
    }

    int32_t routeSize = 0;
    if (!parcel.ReadInt32(routeSize)) {
        return false;
    }
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

bool VpnConfig::UnmarshallingVectorString(Parcel &parcel, std::vector<std::string> &vec, uint32_t maxSize)
{
    uint32_t size = 0;
    if (!parcel.ReadUint32(size)) {
        return false;
    }
    if (size > maxSize) {
        NETMGR_EXT_LOG_E("size = [%{public}d] is too large", size);
        return false;
    }
    for (uint32_t idx = 0; idx < size; idx++) {
        std::string elem;
        if (!parcel.ReadString(elem)) {
            return false;
        }
        vec.push_back(elem);
    }
    return true;
}
} // namespace NetManagerStandard
} // namespace OHOS
