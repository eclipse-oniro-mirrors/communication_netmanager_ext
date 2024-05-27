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

#ifndef NET_IPSEC_VPN_CONFIG_H
#define NET_IPSEC_VPN_CONFIG_H

#include <string>
#include <vector>

#include "inet_addr.h"
#include "parcel.h"
#include "route.h"

namespace OHOS {
namespace NetManagerStandard {
struct IpsecVpnConfig : public Parcelable {
    std::string uuid_;
    int32_t vpnType_ = 0;
    std::string vpnName_;

    std::string userName_;
    std::string password_;
    bool saveLogin_ = false;
    int32_t userId_ = 0;
    std::string vpnAddress_;

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
    static sptr<IpsecVpnConfig> Unmarshalling(Parcel &parcel);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_IPSEC_VPN_CONFIG_H
