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

#ifndef NET_L2TP_VPN_CONFIG_H
#define NET_L2TP_VPN_CONFIG_H

#include <string>
#include "sysvpn_config.h"

namespace OHOS {
namespace NetManagerStandard {
struct L2tpVpnConfig : SysVpnConfig {
    std::string ipsecPreSharedKey_;
    std::string ipsecIdentifier_;
    std::string strongswanConf_;
    std::string ipsecCaCertConf_;
    std::string ipsecPrivateUserCertConf_;
    std::string ipsecPublicUserCertConf_;
    std::string ipsecPrivateServerCertConf_;
    std::string ipsecPublicServerCertConf_;
    std::string ipsecCaCertFilePath_;
    std::string ipsecPrivateUserCertFilePath_;
    std::string ipsecPublicUserCertFilePath_;
    std::string ipsecPrivateServerCertFilePath_;
    std::string ipsecPublicServerCertFilePath_;

    std::string ipsecConf_;
    std::string ipsecSecrets_;
    std::string optionsL2tpdClient_;
    std::string xl2tpdConf_;
    std::string l2tpSharedKey_;

    bool Marshalling(Parcel &parcel) const override;
    static sptr<L2tpVpnConfig> Unmarshalling(Parcel &parcel);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_L2TP_VPN_CONFIG_H
