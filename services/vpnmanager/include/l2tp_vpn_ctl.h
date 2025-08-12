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

#include "ipsecvpn_config.h"
#include "ipsec_vpn_ctl.h"
#include "l2tpvpn_config.h"
#include "net_vpn_impl.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr const char *L2TP_IPSEC_CONFIGURED_TAG = "xl2tpdstart";
constexpr const char *L2TP_IPSEC_CONNECTED_TAG = "pppdstart";
constexpr const char *VPN_NAME_KEY = "l2tp";
constexpr const char *VPN_CLIENT_CONFIG_NAME_KEY = "options.l2tpd.client.conf";
constexpr const char *SINGLE_XL2TP_CONFIG_LNS = " lns = ";
constexpr const char *SINGLE_XL2TP_CONFIG_PPP = ";ppp debug = yes;pppoptfile = ";
constexpr const char *SINGLE_XL2TP_CONFIG_LENGTH = ";length bit = yes;";
} // namespace
class L2tpVpnCtl : public IpsecVpnCtl {
public:
    L2tpVpnCtl(sptr<VpnConfig> config, const std::string &pkg, int32_t userId, std::vector<int32_t> &activeUserIds);
    ~L2tpVpnCtl() = default;

    int32_t GetVpnCertData(const int32_t certType, std::vector<int8_t> &certData) override;
    int32_t GetConnectedSysVpnConfig(sptr<SysVpnConfig> &sysVpnConfig) override;
    int32_t NotifyConnectStage(const std::string &stage, const int32_t &result) override;
    int32_t GetSysVpnCertUri(const int32_t certType, std::string &certUri) override;

private:
    int32_t StartSysVpn() override;
    int32_t StopSysVpn() override;
    int32_t InitConfigFile() override;
    std::string GetXl2tpdConfig();
    void AddConfigToL2tpdConf();
    void HandleIpdecStarted();
    void HandleSwanCtlLoaded();
    void HandleL2tpConfiged();
    void HandleL2tpdCtl();
    void HandleL2tpConnected();
    void HandleConnectFailed(const int32_t result);
    int32_t ProcessUpdateConfig(const std::string &config);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // L2TP_VPN_INTERFACE_H
