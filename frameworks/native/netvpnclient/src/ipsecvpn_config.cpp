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

#include "ipsecvpn_config.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr uint32_t MAX_SIZE = 64;
}
bool IpsecVpnConfig::Marshalling(Parcel &parcel) const
{
    bool allOK = parcel.WriteString(uuid_) &&
                 parcel.WriteString(vpnName_) &&
                 parcel.WriteInt32(vpnType_) &&
                 parcel.WriteString(userName_) &&
                 parcel.WriteString(password_) &&
                 parcel.WriteInt32(saveLogin_) &&
                 parcel.WriteString(vpnAddress_) &&
                 parcel.WriteString(ipsecPreSharedKey_) &&
                 parcel.WriteString(ipsecIdentifier_) &&
                 parcel.WriteString(swanctlConf_) &&
                 parcel.WriteString(strongswanConf_) &&
                 parcel.WriteString(ipsecCaCertConf_) &&
                 parcel.WriteString(ipsecUserCertConf_) &&
                 parcel.WriteString(ipsecServerCertConf_) &&
                 parcel.WriteString(ipsecCaCertFileName_) &&
                 parcel.WriteString(ipsecUserCertFileName_) &&
                 parcel.WriteString(ipsecServerCertFileName_);
    return allOK;
}

sptr<IpsecVpnConfig> IpsecVpnConfig::Unmarshalling(Parcel &parcel)
{
    sptr<IpsecVpnConfig> ptr = new (std::nothrow) IpsecVpnConfig();
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
                 parcel.ReadString(ptr->ipsecPreSharedKey_) &&
                 parcel.ReadString(ptr->ipsecIdentifier_) &&
                 parcel.ReadString(ptr->swanctlConf_) &&
                 parcel.ReadString(ptr->strongswanConf_) &&
                 parcel.ReadString(ptr->ipsecCaCertConf_) &&
                 parcel.ReadString(ptr->ipsecUserCertConf_) &&
                 parcel.ReadString(ptr->ipsecServerCertConf_) &&
                 parcel.ReadString(ptr->ipsecCaCertFileName_) &&
                 parcel.ReadString(ptr->ipsecUserCertFileName_) &&
                 parcel.ReadString(ptr->ipsecServerCertFileName_);
    return allOK ? ptr : nullptr;
}
} // namespace NetManagerStandard
} // namespace OHOS