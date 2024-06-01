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
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
bool SysVpnConfig::Marshalling(Parcel &parcel) const
{
    bool allOK = parcel.WriteString(uuid_) &&
                 parcel.WriteString(vpnName_) &&
                 parcel.WriteInt32(vpnType_) &&
                 parcel.WriteString(userName_) &&
                 parcel.WriteString(password_) &&
                 parcel.WriteBool(saveLogin_) &&
                 parcel.WriteInt32(userId_) &&
                 parcel.WriteString(forwardingRoutes_);
    return allOK;
}

sptr<SysVpnConfig> SysVpnConfig::Unmarshalling(Parcel &parcel)
{
    sptr<SysVpnConfig> ptr = new (std::nothrow) SysVpnConfig();
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("SysVpnConfig ptr is null");
        return nullptr;
    }

    bool allOK = parcel.ReadString(ptr->uuid_) &&
                 parcel.ReadString(ptr->vpnName_) &&
                 parcel.ReadInt32(ptr->vpnType_) &&
                 parcel.ReadString(ptr->userName_) &&
                 parcel.ReadString(ptr->password_) &&
                 parcel.ReadBool(ptr->saveLogin_) &&
                 parcel.ReadInt32(ptr->userId_) &&
                 parcel.ReadString(ptr->forwardingRoutes_);
    return allOK ? ptr : nullptr;
}
} // namespace NetManagerStandard
} // namespace OHOS