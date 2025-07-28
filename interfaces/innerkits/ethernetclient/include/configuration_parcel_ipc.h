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
#ifndef CONFIGURATION_PARCEL_IPC_H
#define CONFIGURATION_PARCEL_IPC_H

#include "http_proxy.h"
#include "parcel.h"
#include "interface_type.h"

namespace OHOS {
namespace NetManagerStandard {
struct ConfigurationParcelIpc : public Parcelable {
    std::string ifName_;
    std::string hwAddr_;
    std::string ipv4Addr_;
    int32_t prefixLength_;
    std::vector<std::string> flags_;
    static void ConvertEtherConfigParcelToNmd(const ConfigurationParcelIpc &ipc,
        OHOS::nmd::InterfaceConfigurationParcel &cfg);
    static void ConvertNmdToEtherConfigParcel(ConfigurationParcelIpc &ipc,
        const OHOS::nmd::InterfaceConfigurationParcel &cfg);

    virtual bool Marshalling(Parcel &parcel) const override;
    static ConfigurationParcelIpc* Unmarshalling(Parcel &parcel);
};
constexpr int32_t MAX_FLAG_NUM = 64;
}
}
#endif // CONFIGURATION_PARCEL_IPC_H
