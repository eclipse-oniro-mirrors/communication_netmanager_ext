/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "sysvpn_config.h"
#include "openvpn_config.h"
#include "ipsecvpn_config.h"
#include "l2tpvpn_config.h"
#include "netmgr_ext_log_wrapper.h"
#include "net_manager_ext_constants.h"

namespace OHOS {
namespace NetManagerStandard {

bool SysVpnConfig::Marshalling(Parcel &parcel) const
{
    // add vpnType first
    parcel.WriteInt32(vpnType_);

    bool allOK = VpnConfig::Marshalling(parcel) &&
                 parcel.WriteString(vpnId_) &&
                 parcel.WriteString(vpnName_) &&
                 parcel.WriteInt32(vpnType_) &&
                 parcel.WriteString(userName_) &&
                 parcel.WriteString(password_) &&
                 parcel.WriteBool(saveLogin_) &&
                 parcel.WriteInt32(userId_) &&
                 parcel.WriteString(forwardingRoutes_);
    if (!allOK) {
        NETMGR_EXT_LOG_I("sysvpn SysVpnConfig Marshalling failed");
    }
    return allOK;
}

sptr<SysVpnConfig> SysVpnConfig::Unmarshalling(Parcel &parcel)
{
    // get vpnType first
    int32_t type = -1;
    parcel.ReadInt32(type);

    switch (type) {
        case static_cast<int32_t>(VpnType::IKEV2_IPSEC_MSCHAPv2):
        case static_cast<int32_t>(VpnType::IKEV2_IPSEC_PSK):
        case static_cast<int32_t>(VpnType::IKEV2_IPSEC_RSA):
        case static_cast<int32_t>(VpnType::IPSEC_XAUTH_PSK):
        case static_cast<int32_t>(VpnType::IPSEC_XAUTH_RSA):
        case static_cast<int32_t>(VpnType::IPSEC_HYBRID_RSA):
            return IpsecVpnConfig::Unmarshalling(parcel);
        case static_cast<int32_t>(VpnType::OPENVPN):
            return OpenVpnConfig::Unmarshalling(parcel);
        case static_cast<int32_t>(VpnType::L2TP_IPSEC_PSK):
        case static_cast<int32_t>(VpnType::L2TP_IPSEC_RSA):
            return L2tpVpnConfig::Unmarshalling(parcel);
        default:
            NETMGR_EXT_LOG_E("sysvpn SysVpnConfig Unmarshalling failed, type=%{public}d", type);
            return nullptr;
    }
}

bool SysVpnConfig::Unmarshalling(Parcel &parcel, sptr<SysVpnConfig> ptr)
{
    bool allOK = VpnConfig::UnmarshallingVpnConfig(parcel, ptr) &&
                 parcel.ReadString(ptr->vpnId_) &&
                 parcel.ReadString(ptr->vpnName_) &&
                 parcel.ReadInt32(ptr->vpnType_) &&
                 parcel.ReadString(ptr->userName_) &&
                 parcel.ReadString(ptr->password_) &&
                 parcel.ReadBool(ptr->saveLogin_) &&
                 parcel.ReadInt32(ptr->userId_) &&
                 parcel.ReadString(ptr->forwardingRoutes_);
    if (!allOK) {
        NETMGR_EXT_LOG_I("sysvpn SysVpnConfig Unmarshalling failed");
    }
    return allOK;
}
} // namespace NetManagerStandard
} // namespace OHOS