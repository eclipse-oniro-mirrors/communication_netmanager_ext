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

#include "ipsecvpn_config.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
bool IpsecVpnConfig::Marshalling(Parcel &parcel) const
{
    bool allOK = SysVpnConfig::Marshalling(parcel) &&
                 parcel.WriteString(ipsecPreSharedKey_) &&
                 parcel.WriteString(ipsecIdentifier_) &&
                 parcel.WriteString(swanctlConf_) &&
                 parcel.WriteString(strongswanConf_) &&
                 parcel.WriteString(ipsecCaCertConf_) &&
                 parcel.WriteString(ipsecPrivateUserCertConf_) &&
                 parcel.WriteString(ipsecPublicUserCertConf_) &&
                 parcel.WriteString(ipsecPrivateServerCertConf_) &&
                 parcel.WriteString(ipsecPublicServerCertConf_) &&
                 parcel.WriteString(ipsecCaCertFilePath_) &&
                 parcel.WriteString(ipsecPrivateUserCertFilePath_) &&
                 parcel.WriteString(ipsecPublicUserCertFilePath_) &&
                 parcel.WriteString(ipsecPrivateServerCertFilePath_) &&
                 parcel.WriteString(ipsecPublicServerCertFilePath_);
    NETMGR_EXT_LOG_I("sysvpn ipsec Marshalling allOK=%{public}d key=%{public}s", allOK, ipsecPreSharedKey_.c_str());
    return allOK;
}

sptr<IpsecVpnConfig> IpsecVpnConfig::Unmarshalling(Parcel &parcel)
{
    sptr<IpsecVpnConfig> ptr = new (std::nothrow) IpsecVpnConfig();
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("IpsecVpnConfig ptr is null");
        return nullptr;
    }

    bool allOK = SysVpnConfig::Unmarshalling(parcel, ptr) &&
                 parcel.ReadString(ptr->ipsecPreSharedKey_) &&
                 parcel.ReadString(ptr->ipsecIdentifier_) &&
                 parcel.ReadString(ptr->swanctlConf_) &&
                 parcel.ReadString(ptr->strongswanConf_) &&
                 parcel.ReadString(ptr->ipsecCaCertConf_) &&
                 parcel.ReadString(ptr->ipsecPrivateUserCertConf_) &&
                 parcel.ReadString(ptr->ipsecPublicUserCertConf_) &&
                 parcel.ReadString(ptr->ipsecPrivateServerCertConf_) &&
                 parcel.ReadString(ptr->ipsecPublicServerCertConf_) &&
                 parcel.ReadString(ptr->ipsecCaCertFilePath_) &&
                 parcel.ReadString(ptr->ipsecPrivateUserCertFilePath_) &&
                 parcel.ReadString(ptr->ipsecPublicUserCertFilePath_) &&
                 parcel.ReadString(ptr->ipsecPrivateServerCertFilePath_) &&
                 parcel.ReadString(ptr->ipsecPublicServerCertFilePath_);
    NETMGR_EXT_LOG_I("sysvpn ipsec Unmarshalling allOK=%{public}d key=%{public}s", allOK, ptr->ipsecPreSharedKey_.c_str());
    return allOK ? ptr : nullptr;
}
} // namespace NetManagerStandard
} // namespace OHOS