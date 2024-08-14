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

#include "mac_address_info.h"

#include "netmgr_ext_log_wrapper.h"
#include "parcel.h"
#include "refbase.h"

namespace OHOS {
namespace NetManagerStandard {
bool MacAddressInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(macAddress_)) {
        return false;
    }
    return true;
}

sptr<MacAddressInfo> MacAddressInfo::Unmarshalling(Parcel &parcel)
{
    sptr<MacAddressInfo> ptr = new (std::nothrow) MacAddressInfo();
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("ptr is null");
        return nullptr;
    }
    if (!parcel.ReadString(ptr->macAddress_)) {
        return nullptr;
    }
    return ptr;
}
} // namespace NetManagerStandard
} // namespace OHOS