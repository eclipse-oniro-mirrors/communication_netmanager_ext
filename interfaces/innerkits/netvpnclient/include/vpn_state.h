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

#ifndef NET_VPN_STATE_H
#define NET_VPN_STATE_H

#include <string>
#include <vector>

#include "parcel.h"
#include "route.h"

namespace OHOS {
namespace NetManagerStandard {
struct VpnState : public Parcelable {
    VpnState() {};
    VpnState(const std::string &vpnIfName, const std::string &vpnIfAddr,
             const std::string &vpnId, bool isGlobalVpn, const std::vector<Route> &routes,
             const std::vector<std::string> &dnsServers = {});

    std::string vpnIfName_ = "";
    std::string vpnIfAddr_ = "";
    std::string vpnId_ = "";
    std::string vpnPacketName_ = "";
    bool isGlobalVpn_ = true;
    std::vector<Route> routes_ = {};
    std::vector<std::string> dnsServers_ = {};

    bool Marshalling(Parcel &parcel) const override;
    bool MarshallingRoute(Parcel &parcel) const;
    bool MarshallingDns(Parcel &parcel) const;

    static VpnState* Unmarshalling(Parcel &parcel);
    static bool UnmarshallingVpnState(Parcel &parcel, VpnState* config);
    static bool UnmarshallingRoute(Parcel &parcel, VpnState* config);
    static bool UnmarshallingDns(Parcel &parcel, VpnState* config);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_VPN_STATE_H
