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

#include "ipsecvpn_config.h"
#include "l2tpvpn_config.h"
#include "net_vpn_impl.h"
#include "netsys_controller.h"

#define IPSEC_PIDDIR "/data/service/el1/public/vpn"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const std::string SWAN_CTL_FILE = IPSEC_PIDDIR "/swanctl.conf";
const std::string SWAN_CONFIG_FILE = IPSEC_PIDDIR "/strongswan.conf";
const std::string L2TP_IPSEC_CFG = IPSEC_PIDDIR "/ipsec.conf";
const std::string L2TP_CFG = IPSEC_PIDDIR "/xl2tpd.conf";
const std::string L2TP_IPSEC_SECRETS_CFG = IPSEC_PIDDIR "/ipsec.secrets.conf";
const std::string OPTIONS_L2TP_CLIENT = IPSEC_PIDDIR "/options.l2tpd.client.conf";
const std::string IPSEC_START_TAG = "start";
const std::string SWANCTL_START_TAG = "config";
const std::string IPSEC_CONNECT_TAG = "connect";
const int32_t NOTIFY_CONNECT_STAGE_SUCCESS = 100;
} // namespace
using namespace NetsysNative;
enum IpsecVpnStateCode {
    STATE_INIT = 0,
    STATE_STARTED,      // ipsec restart compelete
    STATE_CONFIGED,     // swanctl load files compelete or xl2tpd start
    STATE_CONNECTED,    // ipsec up home or pppd started
    STATE_DISCONNECTED, // stop
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
    int32_t NotifyConnectStage(std::string &stage, int32_t &result) override;

protected:
    int32_t state_ = STATE_INIT;

    virtual int32_t StartSysVpn();
    virtual int32_t StopSysVpn();
    virtual int32_t InitConfigFile();
    void CleanTempFiles();
    void DeleteTempFile(const std::string &fileName);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // IPSEC_VPN_CTL_H
