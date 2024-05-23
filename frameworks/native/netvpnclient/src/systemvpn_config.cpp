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

#include "systemvpn_config.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr uint32_t MAX_SIZE = 64;
}
bool SystemVpnConfig::Marshalling(Parcel &parcel) const
{
    bool allOK = MarshallingAddrRoute(parcel) &&
                 parcel.WriteInt32(mtu_) &&
                 parcel.WriteBool(isAcceptIPv4_) &&
                 parcel.WriteBool(isAcceptIPv6_) &&
                 parcel.WriteBool(isLegacy_) &&
                 parcel.WriteBool(isMetered_) &&
                 parcel.WriteBool(isBlocking_) &&
                 parcel.WriteString(uuid_) &&
                 parcel.WriteString(vpnName_) &&
                 parcel.WriteInt32(vpnType_) &&
                 parcel.WriteString(userName_) &&
                 parcel.WriteString(password_) &&
                 parcel.WriteInt32(saveLogin_) &&
                 parcel.WriteString(vpnAddress_) &&
                 parcel.WriteString(ovpnPort_) &&
                 parcel.WriteInt32(ovpnProtocol_) &&
                 parcel.WriteString(ovpnConfig_) &&
                 parcel.WriteInt32(ovpnAuthType_) &&
                 parcel.WriteString(askpass_) &&
                 parcel.WriteString(ovpnConfigFileName_) &&
                 parcel.WriteString(ovpnCaCertFileName_) &&
                 parcel.WriteString(ovpnUserCertFileName_) &&
                 parcel.WriteString(ovpnPrivateKeyFileName_) &&
                 parcel.WriteString(ipsecPreSharedKey_) &&
                 parcel.WriteString(ipsecIdentifier_) &&
                 parcel.WriteString(swanctlConf_) &&
                 parcel.WriteString(strongswanConf_) &&
                 parcel.WriteString(ipsecCaCertConf_) &&
                 parcel.WriteString(ipsecUserCertConf_) &&
                 parcel.WriteString(ipsecServerCertConf_) &&
                 parcel.WriteString(ipsecCaCertFileName_) &&
                 parcel.WriteString(ipsecUserCertFileName_) &&
                 parcel.WriteString(ipsecServerCertFileName_) &&
                 MarshallingVectorString(parcel, dnsAddresses_) &&
                 MarshallingVectorString(parcel, searchDomains_) &&
                 MarshallingVectorString(parcel, acceptedApplications_) &&
                 MarshallingVectorString(parcel, refusedApplications_);
    return allOK;
}

bool SystemVpnConfig::MarshallingAddrRoute(Parcel &parcel) const
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

bool SystemVpnConfig::MarshallingVectorString(Parcel &parcel, const std::vector<std::string> &vec) const
{
    int32_t size = static_cast<int32_t>(vec.size());
    if (!parcel.WriteInt32(size)) {
        return false;
    }
    for (auto &elem : vec) {
        if (!parcel.WriteString(elem)) {
            return false;
        }
    }
    return true;
}

sptr<SystemVpnConfig> SystemVpnConfig::Unmarshalling(Parcel &parcel)
{
    sptr<SystemVpnConfig> ptr = new (std::nothrow) SystemVpnConfig();
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("ptr is null");
        return nullptr;
    }

    bool allOK = UnmarshallingAddrRoute(parcel, ptr) &&
                 parcel.ReadInt32(ptr->mtu_) &&
                 parcel.ReadBool(ptr->isAcceptIPv4_) &&
                 parcel.ReadBool(ptr->isAcceptIPv6_) &&
                 parcel.ReadBool(ptr->isLegacy_) &&
                 parcel.ReadBool(ptr->isMetered_) &&
                 parcel.ReadBool(ptr->isBlocking_) &&
                 parcel.ReadString(ptr->uuid_) &&
                 parcel.ReadString(ptr->vpnName_) &&
                 parcel.ReadInt32(ptr->vpnType_) &&
                 parcel.ReadString(ptr->userName_) &&
                 parcel.ReadString(ptr->password_) &&
                 parcel.ReadInt32(ptr->saveLogin_) &&
                 parcel.ReadString(ptr->vpnAddress_) &&
                 parcel.ReadString(ptr->ovpnPort_) &&
                 parcel.ReadInt32(ptr->ovpnProtocol_) &&
                 parcel.ReadString(ptr->ovpnConfig_) &&
                 parcel.ReadInt32(ptr->ovpnAuthType_) &&
                 parcel.ReadString(ptr->askpass_) &&
                 parcel.ReadString(ptr->ovpnConfigFileName_) &&
                 parcel.ReadString(ptr->ovpnCaCertFileName_) &&
                 parcel.ReadString(ptr->ovpnUserCertFileName_) &&
                 parcel.ReadString(ptr->ovpnPrivateKeyFileName_) &&
                 parcel.ReadString(ptr->ipsecPreSharedKey_) &&
                 parcel.ReadString(ptr->ipsecIdentifier_) &&
                 parcel.ReadString(ptr->swanctlConf_) &&
                 parcel.ReadString(ptr->strongswanConf_) &&
                 parcel.ReadString(ptr->ipsecCaCertConf_) &&
                 parcel.ReadString(ptr->ipsecUserCertConf_) &&
                 parcel.ReadString(ptr->ipsecServerCertConf_) &&
                 parcel.ReadString(ptr->ipsecCaCertFileName_) &&
                 parcel.ReadString(ptr->ipsecUserCertFileName_) &&
                 parcel.ReadString(ptr->ipsecServerCertFileName_) &&
                 UnmarshallingVectorString(parcel, ptr->dnsAddresses_) &&
                 UnmarshallingVectorString(parcel, ptr->searchDomains_) &&
                 UnmarshallingVectorString(parcel, ptr->acceptedApplications_) &&
                 UnmarshallingVectorString(parcel, ptr->refusedApplications_);
    return allOK ? ptr : nullptr;
}

bool SystemVpnConfig::UnmarshallingAddrRoute(Parcel &parcel, sptr<SystemVpnConfig> &config)
{
    int32_t addrSize = 0;
    if (!parcel.ReadInt32(addrSize)) {
        return false;
    }
    if (static_cast<uint32_t>(addrSize) > MAX_SIZE) {
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
    if (static_cast<uint32_t>(routeSize) > MAX_SIZE) {
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

bool SystemVpnConfig::UnmarshallingVectorString(Parcel &parcel, std::vector<std::string> &vec)
{
    int32_t size = 0;
    if (!parcel.ReadInt32(size)) {
        return false;
    }
    if (static_cast<uint32_t>(size) > MAX_SIZE) {
        NETMGR_EXT_LOG_E("size = [%{public}d] is too large", size);
        return false;
    }
    for (int32_t idx = 0; idx < size; idx++) {
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