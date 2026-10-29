/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "network_share_address_parcel.h"

#include <algorithm>
#include <memory>

#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
bool NetworkShareAddressParcel::Marshalling(Parcel &parcel) const
{
    uint32_t size = std::min(MAX_ADDRESS_SIZE, static_cast<uint32_t>(addresses_.size()));
    // LCOV_EXCL_START
    if (!parcel.WriteUint32(size)) {
        NETMGR_EXT_LOG_E("write addresses size to parcel failed");
        return false;
    }
    // LCOV_EXCL_STOP
    for (uint32_t index = 0; index < size; ++index) {
        // LCOV_EXCL_START
        if (!addresses_[index].Marshalling(parcel)) {
            NETMGR_EXT_LOG_E("write INetAddr to parcel failed");
            return false;
        }
        // LCOV_EXCL_STOP
    }
    return true;
}

NetworkShareAddressParcel *NetworkShareAddressParcel::Unmarshalling(Parcel &parcel)
{
    std::unique_ptr<NetworkShareAddressParcel> ptr = std::make_unique<NetworkShareAddressParcel>();
    uint32_t size = 0;
    if (!parcel.ReadUint32(size)) {
        NETMGR_EXT_LOG_E("read addresses size failed");
        return nullptr;
    }
    size = std::min(size, MAX_ADDRESS_SIZE);
    for (uint32_t index = 0; index < size; ++index) {
        sptr<INetAddr> netAddr = INetAddr::Unmarshalling(parcel);
        if (netAddr == nullptr) {
            NETMGR_EXT_LOG_E("read INetAddr failed");
            return nullptr;
        }
        ptr->addresses_.push_back(*netAddr);
    }
    return ptr.release();
}
} // namespace NetManagerStandard
} // namespace OHOS
