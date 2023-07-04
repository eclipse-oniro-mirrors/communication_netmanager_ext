/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef NET_VPN_CONFIG_H
#define NET_VPN_CONFIG_H

#include <string>
#include <vector>

#include "inet_addr.h"
#include "parcel.h"
#include "route.h"

namespace OHOS {
namespace NetManagerStandard {
struct VpnConfig : public Parcelable {
    std::vector<INetAddr> addresses_;
    std::vector<Route> routes_;
    int32_t mtu_ = 0;
    bool isAcceptIPv4_ = true;
    bool isAcceptIPv6_ = false;
    bool isLegacy_ = false;
    bool isMetered_ = false;
    bool isBlocking_ = false;
    std::vector<std::string> dnsAddresses_;
    std::vector<std::string> searchDomains_;
    std::vector<std::string> acceptedApplications_;
    std::vector<std::string> refusedApplications_;

    bool Marshalling(Parcel &parcel) const override;
    bool MarshallingAddrRoute(Parcel &parcel) const;
    bool MarshallingVectorString(Parcel &parcel, const std::vector<std::string> &vec) const;

    static sptr<VpnConfig> Unmarshalling(Parcel &parcel);
    static bool UnmarshallingAddrRoute(Parcel &parcel, sptr<VpnConfig> &config);
    static bool UnmarshallingVectorString(Parcel &parcel, std::vector<std::string> &vec);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_VPN_CONFIG_H
