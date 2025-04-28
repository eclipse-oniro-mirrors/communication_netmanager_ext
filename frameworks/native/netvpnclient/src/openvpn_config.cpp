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
bool OpenvpnConfig::Marshalling(Parcel &parcel) const
{
    if (!SysVpnConfig::Marshalling(parcel)) {
        NETMGR_EXT_LOG_E("OpenvpnConfig Marshalling failed");
        return false;
    }
    if (!parcel.WriteString(ovpnPort_)) {
        return false;
    }

    if (!parcel.WriteInt32(ovpnProtocol_)) {
        return false;
    }

    if (!parcel.WriteString(ovpnConfig_)) {
        return false;
    }

    if (!parcel.WriteInt32(ovpnAuthType_)) {
        return false;
    }

    if (!parcel.WriteString(askpass_)) {
        return false;
    }

    if (!parcel.WriteString(ovpnConfigFilePath_)) {
        return false;
    }

    if (!parcel.WriteString(ovpnCaCertFilePath_)) {
        return false;
    }

    if (!parcel.WriteString(ovpnUserCertFilePath_)) {
        return false;
    }

    if (!parcel.WriteString(ovpnPrivateKeyFilePath_)) {
        return false;
    }

    return true;
}

OpenvpnConfig* OpenvpnConfig::Unmarshalling(Parcel &parcel)
{
    std::unique_ptr<OpenvpnConfig> ptr = new (std::nothrow) OpenvpnConfig();
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("OpenvpnConfig ptr is null");
        return nullptr;
    }
    if (!SysVpnConfig::Unmarshalling(parcel, ptr.get())) {
        NETMGR_EXT_LOG_E("OpenvpnConfig Unmarshalling failed");
        return nullptr;
    }
    if (!parcel.ReadString(ptr->ovpnPort_)) {
        return nullptr;
    }

    if (!parcel.ReadInt32(ptr->ovpnProtocol_)) {
        return nullptr;
    }

    if (!parcel.ReadString(ptr->ovpnConfig_)) {
        return nullptr;
    }

    if (!parcel.ReadInt32(ptr->ovpnAuthType_)) {
        return nullptr;
    }

    if (!parcel.ReadString(ptr->askpass_)) {
        return nullptr;
    }

    if (!parcel.ReadString(ptr->ovpnConfigFilePath_)) {
        return nullptr;
    }

    if (!parcel.ReadString(ptr->ovpnCaCertFilePath_)) {
        return nullptr;
    }

    if (!parcel.ReadString(ptr->ovpnUserCertFilePath_)) {
        return nullptr;
    }

    if (!parcel.ReadString(ptr->ovpnPrivateKeyFilePath_)) {
        return nullptr;
    }
    return ptr.release();
}
} // namespace NetManagerStandard
} // namespace OHOS