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

#ifndef NET_OPEN_VPN_CONFIG_H
#define NET_OPEN_VPN_CONFIG_H

#include <string>
#include <vector>

#include "parcel.h"

namespace OHOS {
namespace NetManagerStandard {
struct OpenVpnConfig : public Parcelable {
    std::string uuid_;
    std::string vpnName_;
    int32_t vpnType_ = 0;

    std::string userName_;
    std::string password_;
    int32_t saveLogin_ = 0;
    int32_t userId_ = 0;
    std::string vpnAddress_;

    // openvpn
    std::string ovpnPort_;
    int32_t ovpnProtocol_;
    std::string ovpnConfig_;
    int32_t ovpnAuthType_;
    std::string askpass_;
    std::string ovpnConfigFileName_;
    std::string ovpnCaCertFileName_;
    std::string ovpnUserCertFileName_;
    std::string ovpnPrivateKeyFileName_;

    bool Marshalling(Parcel &parcel) const override;
    static sptr<OpenVpnConfig> Unmarshalling(Parcel &parcel);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_OPEN_VPN_CONFIG_H
