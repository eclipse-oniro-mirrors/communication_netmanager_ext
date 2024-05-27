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

#include "openvpn_config.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr uint32_t MAX_SIZE = 64;
}
bool OpenVpnConfig::Marshalling(Parcel &parcel) const
{
    bool allOK = parcel.WriteString(uuid_) &&
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
                 parcel.WriteString(ovpnPrivateKeyFileName_);
    return allOK;
}

sptr<OpenVpnConfig> OpenVpnConfig::Unmarshalling(Parcel &parcel)
{
    sptr<OpenVpnConfig> ptr = new (std::nothrow) OpenVpnConfig();
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("ptr is null");
        return nullptr;
    }

    bool allOK = parcel.ReadString(ptr->uuid_) &&
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
                 parcel.ReadString(ptr->ovpnPrivateKeyFileName_);
    return allOK ? ptr : nullptr;
}
} // namespace NetManagerStandard
} // namespace OHOS