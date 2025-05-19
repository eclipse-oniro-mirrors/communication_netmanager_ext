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
 
#ifndef ETHERNET_DEVICE_INFO_H
#define ETHERNET_DEVICE_INFO_H
 
#include <string>
 
#include "parcel.h"
 
namespace OHOS {
namespace NetManagerStandard {
typedef enum {
    BUILT_IN,
    EXTERNAL,
} DeviceConnectionType;
 
struct EthernetDeviceInfo : public Parcelable {
    std::string ifaceName_;
    std::string deviceName_;
    DeviceConnectionType connectionMode_;
    std::string supplierName_;
    std::string supplierId_;
    std::string productName_;
    std::string maximumRate_;
 
    virtual bool Marshalling(Parcel &parcel) const override;
    static EthernetDeviceInfo* Unmarshalling(Parcel &parcel);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // ETHERNET_DEVICE_INFO_H