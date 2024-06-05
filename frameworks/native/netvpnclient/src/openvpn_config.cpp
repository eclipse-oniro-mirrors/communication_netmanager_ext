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

#include "openvpn_config.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
bool OpenVpnConfig::Marshalling(Parcel &parcel) const
{
    bool allOK = SysVpnConfig::Marshalling(parcel) &&
                 parcel.WriteString(ovpnPort_) &&
                 parcel.WriteInt32(ovpnProtocol_) &&
                 parcel.WriteString(ovpnConfig_) &&
                 parcel.WriteInt32(ovpnAuthType_) &&
                 parcel.WriteString(askpass_) &&
                 parcel.WriteString(ovpnConfigFilePath_) &&
                 parcel.WriteString(ovpnCaCertFilePath_) &&
                 parcel.WriteString(ovpnUserCertFilePath_) &&
                 parcel.WriteString(ovpnPrivateKeyFilePath_);
    return allOK;
}

sptr<OpenVpnConfig> OpenVpnConfig::Unmarshalling(Parcel &parcel)
{
    sptr<OpenVpnConfig> ptr = new (std::nothrow) OpenVpnConfig();
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("OpenVpnConfig ptr is null");
        return nullptr;
    }

    bool allOK = SysVpnConfig::Unmarshalling(parcel, ptr) &&
                 parcel.ReadString(ptr->ovpnPort_) &&
                 parcel.ReadInt32(ptr->ovpnProtocol_) &&
                 parcel.ReadString(ptr->ovpnConfig_) &&
                 parcel.ReadInt32(ptr->ovpnAuthType_) &&
                 parcel.ReadString(ptr->askpass_) &&
                 parcel.ReadString(ptr->ovpnConfigFilePath_) &&
                 parcel.ReadString(ptr->ovpnCaCertFilePath_) &&
                 parcel.ReadString(ptr->ovpnUserCertFilePath_) &&
                 parcel.ReadString(ptr->ovpnPrivateKeyFilePath_);
    return allOK ? ptr : nullptr;
}
} // namespace NetManagerStandard
} // namespace OHOS