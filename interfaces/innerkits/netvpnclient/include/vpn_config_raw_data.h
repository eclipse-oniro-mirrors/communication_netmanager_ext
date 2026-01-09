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

#ifndef NETMANAGER_STANDARD_MY_RAWDATA_H
#define NETMANAGER_STANDARD_MY_RAWDATA_H

#include "parcel.h"
#include "message_parcel.h"
#include "securec.h"
#include "vpn_config.h"

namespace OHOS {
namespace NetManagerStandard {

constexpr int32_t MAX_RAW_DATA_SIZE = 2 * 1024 * 1024;
constexpr size_t MAX_PARCEL_DATA_SIZE = 2048000;
constexpr uint32_t ROUTE_MAX_SIZE = 10000;


class VpnConfigRawData : public OHOS::Parcelable {
public:
    uint32_t size = 0;
    const void* data = nullptr;

    VpnConfigRawData() = default;

    virtual ~VpnConfigRawData()
    {
        Cleanup();
    }

    int32_t RawDataCpy(const void* readData)
    {
        if (size == 0 || size > MAX_RAW_DATA_SIZE || readData == nullptr) {
            return -1;
        }

        uint8_t* buffer = new (std::nothrow) uint8_t[size];
        if (buffer == nullptr) {
            return -1;
        }

        auto ret = memcpy_s(buffer, size, readData, size);
        if (ret != 0) {
            delete[] buffer;
            return -1;
        }

        if (data != nullptr) {
            delete[] static_cast<uint8_t*>(const_cast<void*>(data));
            data = nullptr;
        }

        data = reinterpret_cast<void*>(buffer);
        return 0;
    }

    bool Marshalling(OHOS::Parcel &parcel) const override
    {
        OHOS::MessageParcel *parcelIn = static_cast<OHOS::MessageParcel*>(&parcel);
        if (data == nullptr) {
            return false;
        }

        return parcelIn->WriteRawData(data, size);
    }
    
    int32_t ToVpnConfig(VpnConfig& config) const
    {
        if (data == nullptr || size == 0) {
            return false;
        }
    
        OHOS::Parcel parcel;
        parcel.SetMaxCapacity(MAX_PARCEL_DATA_SIZE);

        if (!parcel.WriteBuffer(data, size)) {
            return false;
        }

        parcel.RewindRead(0);

        VpnConfig* tempConfig = VpnConfig::Unmarshalling(parcel);
        if (tempConfig == nullptr) {
            return false;
        }
        config = std::move(*tempConfig);
        delete tempConfig;
    
        return true;
    }

    bool SerializeFromVpnConfig(VpnConfig& config)
    {
        Cleanup();

        if (config.routes_.size() > ROUTE_MAX_SIZE) {
            config.routes_.resize(ROUTE_MAX_SIZE);
        }

        Parcel parcel;
        parcel.SetMaxCapacity(MAX_PARCEL_DATA_SIZE);
        if (!config.Marshalling(parcel)) {
            return false;
        }

        size = parcel.GetDataSize();
        if (size == 0 || size > MAX_RAW_DATA_SIZE) {
            return false;
        }

        uintptr_t dataAddr = parcel.GetData();
        const void* sourceData = reinterpret_cast<const void*>(dataAddr);
        if (sourceData == nullptr) {
            return false;
        }

        int32_t copyResult = RawDataCpy(sourceData);
        if (copyResult != 0) {
            return false;
        }

        return true;
    }

private:
    void Cleanup()
    {
        if (data != nullptr) {
            delete[] static_cast<uint8_t*>(const_cast<void*>(data));
            data = nullptr;
        }
        size = 0;
    }
};

} // namespace NetManagerStandard
} // namespace OHOS

#endif // NETMANAGER_STANDARD_MY_RAWDATA_H