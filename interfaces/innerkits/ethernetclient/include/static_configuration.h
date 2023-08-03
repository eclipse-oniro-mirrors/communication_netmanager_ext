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

#ifndef STATIC_CONFIGURATION_H
#define STATIC_CONFIGURATION_H

#include <string>
#include <vector>

#include "inet_addr.h"
#include "parcel.h"

namespace OHOS {
namespace NetManagerStandard {
struct StaticConfiguration final : public Parcelable {
    std::vector<INetAddr> ipAddrList_;
    std::vector<INetAddr> routeList_;
    std::vector<INetAddr> gatewayList_;
    std::vector<INetAddr> netMaskList_;
    std::vector<INetAddr> dnsServers_;
    std::string domain_;

    bool Marshalling(Parcel &parcel) const override;
    bool MarshallingNetAddressList(const std::vector<INetAddr> &netAddrList, uint32_t maxSize, Parcel &parcel) const;

    static sptr<StaticConfiguration> Unmarshalling(Parcel &parcel);
    static bool UnmarshallingNetAddressList(Parcel &parcel, std::vector<INetAddr> &netAddrList, uint32_t maxSize);
    static void ExtractNetAddrBySeparator(const std::string &input, std::vector<INetAddr> &netAddrList);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif