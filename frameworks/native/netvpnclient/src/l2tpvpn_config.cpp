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

#include "l2tpvpn_config.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
bool L2tpVpnConfig::Marshalling(Parcel &parcel) const
{
    bool allOK = SysVpnConfig::Marshalling(parcel) &&
                 parcel.WriteString(ipsecConf_) &&
                 parcel.WriteString(ipsecSecrets_) &&
                 parcel.WriteString(optionsL2tpdClient_) &&
                 parcel.WriteString(xl2tpdConf_) &&
                 parcel.WriteString(l2tpSharedKey_) &&
                 parcel.WriteString(ipsecPreSharedKey_) &&
                 parcel.WriteString(ipsecIdentifier_) &&
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
    return allOK;
}

L2tpVpnConfig* L2tpVpnConfig::Unmarshalling(Parcel &parcel)
{
    std::unique_ptr<L2tpVpnConfig> ptr = std::make_unique<L2tpVpnConfig>();
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("L2tpVpnConfig ptr is null");
        return nullptr;
    }

    bool allOK = SysVpnConfig::Unmarshalling(parcel, ptr.get()) &&
                 parcel.ReadString(ptr->ipsecConf_) &&
                 parcel.ReadString(ptr->ipsecSecrets_) &&
                 parcel.ReadString(ptr->optionsL2tpdClient_) &&
                 parcel.ReadString(ptr->xl2tpdConf_) &&
                 parcel.ReadString(ptr->l2tpSharedKey_) &&
                 parcel.ReadString(ptr->ipsecPreSharedKey_) &&
                 parcel.ReadString(ptr->ipsecIdentifier_) &&
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
    return allOK ? ptr.release() : nullptr;
}
} // namespace NetManagerStandard
} // namespace OHOS