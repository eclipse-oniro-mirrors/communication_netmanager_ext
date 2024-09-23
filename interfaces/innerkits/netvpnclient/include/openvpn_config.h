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

#ifndef OPENVPN_CONFIG_H
#define OPENVPN_CONFIG_H

#include <string>
#include "sysvpn_config.h"

namespace OHOS {
namespace NetManagerStandard {
struct OpenvpnConfig : SysVpnConfig {
    std::string ovpnPort_;
    int32_t ovpnProtocol_;
    std::string ovpnConfig_;
    int32_t ovpnAuthType_;
    std::string askpass_;
    std::string ovpnConfigFilePath_;
    std::string ovpnCaCertFilePath_;
    std::string ovpnUserCertFilePath_;
    std::string ovpnPrivateKeyFilePath_;

    bool Marshalling(Parcel &parcel) const override;
    static sptr<OpenvpnConfig> Unmarshalling(Parcel &parcel);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // OPENVPN_CONFIG_H
