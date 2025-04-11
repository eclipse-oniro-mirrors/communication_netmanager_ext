/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#include "configuration_parcel_ipc.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
bool ConfigurationParcelIpc::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(ifName_)) {
        return false;
    }
    if (!parcel.WriteString(hwAddr_)) {
        return false;
    }
    if (!parcel.WriteString(ipv4Addr_)) {
        return false;
    }
    if (!parcel.WriteInt32(prefixLength_)) {
        return false;
    }
    int32_t flagsSize = static_cast<int32_t>(flags_.size());
    if (!parcel.WriteInt32(flagsSize)) {
        return false;
    }

    for (auto flag : flags_) {
        if (!parcel.WriteString(flag)) {
            return false;
        }
    }
    return true;
}

ConfigurationParcelIpc* ConfigurationParcelIpc::Unmarshalling(Parcel &parcel)
{
    ConfigurationParcelIpc* ptr = new (std::nothrow) ConfigurationParcelIpc();
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("ptr is null");
        return nullptr;
    }
    if (!parcel.ReadString(ptr->ifName_)) {
        return nullptr;
    }
    if (!parcel.ReadString(ptr->hwAddr_)) {
        return nullptr;
    }
    if (!parcel.ReadString(ptr->ipv4Addr_)) {
        return nullptr;
    }
    if (!parcel.ReadInt32(ptr->prefixLength_)) {
        return nullptr;
    }
    int32_t length = 0;
    if (!parcel.ReadInt32(length)) {
        return nullptr;
    }
    std::string flags;
    for (int32_t idx = 0; idx < length; idx++) {
        if (!parcel.ReadString(flags)) {
            return nullptr;
        }
        ptr->flags_.push_back(flags);
    }
    return ptr;
}

void ConfigurationParcelIpc::ConvertEtherConfigParcelToNmd(const ConfigurationParcelIpc &ipc,
    OHOS::nmd::InterfaceConfigurationParcel &cfg)
{
    cfg.ifName = ipc.ifName_;
    cfg.hwAddr = ipc.hwAddr_;
    cfg.ipv4Addr = ipc.ipv4Addr_;
    cfg.prefixLength = ipc.prefixLength_;
    cfg.flags = ipc.flags_;
}

void ConfigurationParcelIpc::ConvertNmdToEtherConfigParcel(ConfigurationParcelIpc &ipc,
    const OHOS::nmd::InterfaceConfigurationParcel &cfg)
{
    ipc.ifName_ = cfg.ifName;
    ipc.hwAddr_ = cfg.hwAddr;
    ipc.ipv4Addr_ = cfg.ipv4Addr;
    ipc.prefixLength_ = cfg.prefixLength;
    ipc.flags_ = cfg.flags;
}
}
}
