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

#ifndef EXTENDED_VPN_CTL_H
#define EXTENDED_VPN_CTL_H

#include <cstdint>

#include "net_vpn_impl.h"

namespace OHOS {
namespace NetManagerStandard {
class ExtendedVpnCtl : public NetVpnImpl {
public:
    ExtendedVpnCtl(sptr<VpnConfig> config, const std::string &pkg, int32_t userId, std::vector<int32_t> &activeUserIds);
    ~ExtendedVpnCtl() = default;

    bool IsInternalVpn() override;
    int32_t SetUp() override;
    int32_t Destroy() override;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // EXTENDED_VPN_CTL_H
