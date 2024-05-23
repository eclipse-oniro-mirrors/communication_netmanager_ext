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

#ifndef NET_SYSTEM_VPN_CONFIG_H
#define NET_SYSTEM_VPN_CONFIG_H

#include <string>
#include <vector>

#include "inet_addr.h"
#include "parcel.h"
#include "route.h"

namespace OHOS {
namespace NetManagerStandard {
struct SystemVpnConfig : public Parcelable {
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

    //common
    std::string uuid_;
    std::string vpnName_;
    std::string userName_;
    std::string password_;
    int32_t vpnType_ = -1;
    int32_t saveLogin_ = 0;
    int32_t userId_ = 0;
    std::string vpnAddress_;

    //openvpn
    std::string ovpnPort_;
    int32_t ovpnProtocol_;
    std::string ovpnConfig_;
    int32_t ovpnAuthType_;
    std::string askpass_;
    std::string ovpnConfigFileName_;
    std::string ovpnCaCertFileName_;
    std::string ovpnUserCertFileName_;
    std::string ovpnPrivateKeyFileName_;

    //ipsec
    std::string ipsecPreSharedKey_;
    std::string ipsecIdentifier_;
    std::string swanctlConf_;
    std::string strongswanConf_;
    std::string ipsecCaCertConf_;
    std::string ipsecUserCertConf_;
    std::string ipsecServerCertConf_;
    std::string ipsecCaCertFileName_;
    std::string ipsecUserCertFileName_;
    std::string ipsecServerCertFileName_;

    bool Marshalling(Parcel &parcel) const override;
    bool MarshallingAddrRoute(Parcel &parcel) const;
    bool MarshallingVectorString(Parcel &parcel, const std::vector<std::string> &vec) const;

    static sptr<SystemVpnConfig> Unmarshalling(Parcel &parcel);
    static bool UnmarshallingAddrRoute(Parcel &parcel, sptr<SystemVpnConfig> &config);
    static bool UnmarshallingVectorString(Parcel &parcel, std::vector<std::string> &vec);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_SYSTEM_VPN_CONFIG_H
