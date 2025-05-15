/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
 
#include "ethernet_device_info.h"
 
namespace OHOS {
namespace NetManagerStandard {
bool EthernetDeviceInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(ifaceName_)) {
        return false;
    }
    if (!parcel.WriteString(deviceName_)) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(connectionMode_))) {
        return false;
    }
    if (!parcel.WriteString(supplierName_)) {
        return false;
    }
    if (!parcel.WriteString(supplierId_)) {
        return false;
    }
    if (!parcel.WriteString(productName_)) {
        return false;
    }
    if (!parcel.WriteString(maximumRate_)) {
        return false;
    }
    return true;
}
 
EthernetDeviceInfo* EthernetDeviceInfo::Unmarshalling(Parcel &parcel)
{
    EthernetDeviceInfo* ptr = new (std::nothrow) EthernetDeviceInfo();
    if (ptr == nullptr) {
        return nullptr;
    }
    if (!parcel.ReadString(ptr->ifaceName_)) {
        return nullptr;
    }
    if (!parcel.ReadString(ptr->deviceName_)) {
        return nullptr;
    }
    int32_t mode = 0;
    if (!parcel.ReadInt32(mode)) {
        return nullptr;
    }
    ptr->connectionMode_ = static_cast<DeviceConnectionType>(mode);
    if (!parcel.ReadString(ptr->supplierName_)) {
        return nullptr;
    }
    if (!parcel.ReadString(ptr->supplierId_)) {
        return nullptr;
    }
    if (!parcel.ReadString(ptr->productName_)) {
        return nullptr;
    }
    if (!parcel.ReadString(ptr->maximumRate_)) {
        return nullptr;
    }
    return ptr;
}
} // namespace NetManagerStandard
} // namespace OHOS