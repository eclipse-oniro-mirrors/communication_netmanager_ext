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
#include "ipsecvpn_config.h"
#include "l2tpvpn_config.h"
#include "netmgr_ext_log_wrapper.h"
#include "openvpn_config.h"

namespace OHOS {
namespace NetManagerStandard {
static constexpr const uint8_t MAX_REMOTE_ADDR_SIZE = 2;

bool SysVpnConfig::Marshalling(Parcel &parcel) const
{
    // add vpnType first
    parcel.WriteInt32(vpnType_);

    bool allOK = VpnConfig::Marshalling(parcel) &&
                 parcel.WriteString(vpnName_) &&
                 parcel.WriteInt32(vpnType_) &&
                 parcel.WriteString(userName_) &&
                 parcel.WriteString(password_) &&
                 parcel.WriteBool(saveLogin_) &&
                 parcel.WriteInt32(userId_) &&
                 parcel.WriteString(forwardingRoutes_) &&
                 parcel.WriteString(pkcs12Password_) &&
                 VpnConfig::MarshallingVectorString(parcel, remoteAddresses_, MAX_REMOTE_ADDR_SIZE);

    allOK = allOK && parcel.WriteInt32(static_cast<int32_t>(pkcs12FileData_.size()));
    for (uint8_t byte : pkcs12FileData_) {
        allOK = allOK && parcel.WriteUint8(byte);
    }
    if (!allOK) {
        NETMGR_EXT_LOG_I("sysvpn SysVpnConfig Marshalling failed");
    }
    return allOK;
}

SysVpnConfig* SysVpnConfig::Unmarshalling(Parcel &parcel)
{
    // get vpnType first
    int32_t type = -1;
    parcel.ReadInt32(type);

    switch (type) {
        case VpnType::IKEV2_IPSEC_MSCHAPv2:
        case VpnType::IKEV2_IPSEC_PSK:
        case VpnType::IKEV2_IPSEC_RSA:
        case VpnType::IPSEC_XAUTH_PSK:
        case VpnType::IPSEC_XAUTH_RSA:
        case VpnType::IPSEC_HYBRID_RSA:
            return IpsecVpnConfig::Unmarshalling(parcel);
        case VpnType::OPENVPN:
            return OpenvpnConfig::Unmarshalling(parcel);
        case VpnType::L2TP:
        case VpnType::L2TP_IPSEC_PSK:
        case VpnType::L2TP_IPSEC_RSA:
            return L2tpVpnConfig::Unmarshalling(parcel);
        default:
            NETMGR_EXT_LOG_E("sysvpn SysVpnConfig Unmarshalling failed, type=%{public}d", type);
            return nullptr;
    }
}

bool SysVpnConfig::Unmarshalling(Parcel &parcel, SysVpnConfig* ptr)
{
    bool allOK = VpnConfig::UnmarshallingVpnConfig(parcel, ptr) &&
                 parcel.ReadString(ptr->vpnName_) &&
                 parcel.ReadInt32(ptr->vpnType_) &&
                 parcel.ReadString(ptr->userName_) &&
                 parcel.ReadString(ptr->password_) &&
                 parcel.ReadBool(ptr->saveLogin_) &&
                 parcel.ReadInt32(ptr->userId_) &&
                 parcel.ReadString(ptr->forwardingRoutes_) &&
                 parcel.ReadString(ptr->pkcs12Password_) &&
                 VpnConfig::UnmarshallingVectorString(parcel, ptr->remoteAddresses_, MAX_REMOTE_ADDR_SIZE);

    int32_t pkcs12FileDataSize = 0;
    uint8_t data = 0;
    allOK = allOK && parcel.ReadInt32(pkcs12FileDataSize);
    if (pkcs12FileDataSize > MAX_LEN_CERTIFICATE_CHAIN) {
        NETMGR_EXT_LOG_E("pkcs12FileDataSize is too large");
        return false;
    }
    for (int32_t i = 0; i < pkcs12FileDataSize; i++) {
        allOK = allOK && parcel.ReadUint8(data);
        ptr->pkcs12FileData_.push_back(data);
    }
    if (!allOK) {
        NETMGR_EXT_LOG_I("sysvpn SysVpnConfig Unmarshalling failed");
    }
    return allOK;
}
} // namespace NetManagerStandard
} // namespace OHOS