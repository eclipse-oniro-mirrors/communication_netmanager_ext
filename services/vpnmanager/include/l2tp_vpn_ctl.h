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

#ifndef L2TP_VPN_INTERFACE_H
#define L2TP_VPN_INTERFACE_H

#include <cstdint>
#include <sys/types.h>

#include "net_vpn_impl.h"
#include "ipsecvpn_config.h"
#include "l2tpvpn_config.h"
#include "ipsec_vpn_ctl.h"

namespace OHOS {
namespace NetManagerStandard {
class L2tpVpnCtl : public IpsecVpnCtl {
public:
    L2tpVpnCtl(sptr<VpnConfig> config, const std::string &pkg, int32_t userId, std::vector<int32_t> &activeUserIds);
    ~L2tpVpnCtl() = default;

    int32_t GetConnectedSysVpnConfig(sptr<SysVpnConfig> &sysVpnConfig) override;
    bool isSysVpnImpl() override;

private:
    static constexpr const char* L2TP_IPSEC_CONFIGURED_TAG = "xl2tpdstart";
    static constexpr const char* L2TP_IPSEC_CONNECTED_TAG = "pppdstart";

    int32_t StartIpsecVpn() override;
    int32_t StopIpsecVpn() override;
    int32_t InitConfigFile() override;
    void ParseIpsecStatus(std::string &content, int32_t &status) override;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // L2TP_VPN_INTERFACE_H
