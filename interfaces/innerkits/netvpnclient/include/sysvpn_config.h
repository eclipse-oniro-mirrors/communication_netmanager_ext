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

#ifndef NET_SYS_VPN_CONFIG_H
#define NET_SYS_VPN_CONFIG_H

#include <string>
#include "vpn_config.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t SYSVPN_MAX_SIZE = 1024;

enum VpnType : int32_t {
    IKEV2_IPSEC_MSCHAPv2 = 1,
    IKEV2_IPSEC_PSK,
    IKEV2_IPSEC_RSA,
    L2TP_IPSEC_PSK,
    L2TP_IPSEC_RSA,
    IPSEC_XAUTH_PSK,
    IPSEC_XAUTH_RSA,
    IPSEC_HYBRID_RSA,
    OPENVPN,
    L2TP
};

struct SysVpnConfig : public VpnConfig {
    std::string vpnId_;
    std::string vpnName_;
    int32_t vpnType_ = 0;
    std::string userName_;
    std::string password_;
    bool saveLogin_ = false;
    int32_t userId_ = 0;
    std::string forwardingRoutes_;

    bool Marshalling(Parcel &parcel) const override;
    static SysVpnConfig* Unmarshalling(Parcel &parcel);
    static bool Unmarshalling(Parcel &parcel, SysVpnConfig* ptr);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_SYS_VPN_CONFIG_H
