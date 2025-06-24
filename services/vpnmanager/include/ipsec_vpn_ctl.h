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

#ifndef IPSEC_VPN_CTL_H
#define IPSEC_VPN_CTL_H

#include <cstdint>

#include "cJSON.h"
#include "ipsecvpn_config.h"
#include "l2tpvpn_config.h"
#include "net_vpn_impl.h"
#include "netsys_controller.h"

#define IPSEC_PIDDIR "/data/service/el1/public/vpn"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr const char *SWAN_CONFIG_FILE = IPSEC_PIDDIR "/strongswan.conf";
constexpr const char *L2TP_CFG = IPSEC_PIDDIR "/xl2tpd.conf";
constexpr const char *IPSEC_START_TAG = "start";
constexpr const char *SWANCTL_START_TAG = "config";
constexpr const char *IPSEC_CONNECT_TAG = "connect";
constexpr const char *IPSEC_CONNECT_NAME = "home";
constexpr const char *L2TP_CONNECT_NAME = "l2tp";
constexpr const char *IPSEC_NODE_UPDATE_CONFIG = "updateconfig";
constexpr const char *IPSEC_NODE_MTU = "mtu";
constexpr const char *IPSEC_NODE_ADDRESS = "address";
constexpr const char *IPSEC_NODE_NETMASK = "netmask";
constexpr const char *IPSEC_NODE_PHY_NAME = "phyifname";
constexpr const char *IPSEC_NODE_REMOTE_IP = "remoteip";
} // namespace
using namespace NetsysNative;
enum IpsecVpnStateCode {
    STATE_INIT = 0,
    STATE_STARTED,      // ipsec restart compelete
    STATE_CONFIGED,     // swanctl load files compelete
    STATE_CONTROLLED,   // control pppd startup
    STATE_CONNECTED,    // ipsec up home or pppd started
    STATE_DISCONNECTED, // stop
    STATE_L2TP_STARTED, // xl2tpd start
};

enum IpsecVpnCertType : int32_t {
    CA_CERT = 0,
    USER_CERT,
    SERVER_CERT,
    SWAN_CTL_CONF,
    OPTIONS_L2TP_CLIENT_CONF,
    L2TP_IPSEC_SECRETS_CONF,
};

class IpsecVpnCtl : public NetVpnImpl {
public:
    IpsecVpnCtl(sptr<VpnConfig> config, const std::string &pkg, int32_t userId, std::vector<int32_t> &activeUserIds);
    virtual ~IpsecVpnCtl();

    sptr<IpsecVpnConfig> ipsecVpnConfig_ = nullptr;
    sptr<L2tpVpnConfig> l2tpVpnConfig_ = nullptr;

    bool IsInternalVpn() override;
    int32_t SetUp() override;
    int32_t Destroy() override;
    int32_t GetConnectedSysVpnConfig(sptr<SysVpnConfig> &sysVpnConfig) override;
    int32_t NotifyConnectStage(const std::string &stage, const int32_t &result) override;
    int32_t GetSysVpnCertUri(const int32_t certType, std::string &certUri) override;
    bool IsSystemVpn() override;

protected:
    int32_t state_ = STATE_INIT;
    virtual int32_t StartSysVpn();
    virtual int32_t StopSysVpn();
    virtual int32_t InitConfigFile();
    void CleanTempFiles();
    void DeleteTempFile(const std::string &fileName);
    int32_t SetUpVpnTun();
    int32_t UpdateConfig(const std::string &msg);
private:
    void ProcessUpdateConfig(cJSON* jConfig);
    void ProcessSwanctlLoad();
    void ProcessIpsecUp();
    void HandleConnected();
    int32_t HandleUpdateConfig(const std::string &config);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // IPSEC_VPN_CTL_H
