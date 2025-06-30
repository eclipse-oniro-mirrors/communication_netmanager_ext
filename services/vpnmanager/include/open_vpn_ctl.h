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

#ifndef OPENVPN_CTL_H
#define OPENVPN_CTL_H

#include "cJSON.h"
#include "netsys_controller.h"
#include "net_vpn_impl.h"
#include "openvpn_config.h"

#define VPN_PIDDIR "/data/service/el1/public/vpn"

namespace OHOS {
namespace NetManagerStandard {
using namespace NetsysNative;

constexpr const char *OPENVPN_NODE_ROOT = "openvpn";
constexpr const char *OPENVPN_NODE_MTU = "mtu";
constexpr const char *OPENVPN_NODE_ADDRESS = "address";
constexpr const char *OPENVPN_NODE_NETMASK = "netmask";
constexpr const char *OPENVPN_NODE_CONFIG = "config";
constexpr const char *OPENVPN_NODE_STATE = "state";
constexpr const char *OPENVPN_NODE_UPDATE_STATE = "updateState";
constexpr const char *OPENVPN_NODE_SETUP_VPN_TUN = "setupVpnTun";
constexpr const char *OPENVPN_MASK_TAG = "***";

enum OpenvpnStateCode : int32_t {
    OPENVPN_STATE_UNKNOWN = 1,
    OPENVPN_STATE_SETUP,
    OPENVPN_STATE_STARTED,
    OPENVPN_STATE_CONNECTED,
    OPENVPN_STATE_DISCONNECTED,
    OPENVPN_STATE_ERROR_PRIVATE_KEY = 200,
    OPENVPN_STATE_ERROR_CLIENT_CRT,
    OPENVPN_STATE_ERROR_CA_CAT,
    OPENVPN_STATE_ERROR_TIME_OUT,
};

enum OpenVpnConfigType : int32_t {
    OPENVPN_ASKPASS = 0,
    OPENVPN_CONF,
};

class OpenvpnCtl : public NetVpnImpl {
public:
    OpenvpnCtl(sptr<VpnConfig> config, const std::string &pkg, int32_t userId, std::vector<int32_t> &activeUserIds);
    ~OpenvpnCtl() = default;

    int32_t GetVpnCertData(const int32_t certType, std::vector<int8_t> &certData) override;
    bool IsInternalVpn() override;
    int32_t SetUp() override;
    int32_t Destroy() override;
    int32_t GetConnectedSysVpnConfig(sptr<SysVpnConfig> &sysVpnConfig) override;
    int32_t NotifyConnectStage(const std::string &stage, const int32_t &result) override;
    int32_t GetSysVpnCertUri(const int32_t certType, std::string &certUri) override;
    bool IsSystemVpn() override;
    sptr<OpenvpnConfig> openvpnConfig_;

private:
    const std::string  OPENVPN_ASKPASS_FILE = VPN_PIDDIR "/askpass";
    const std::string  OPENVPN_ASKPASS_PARAM = "askpass " + std::string(OPENVPN_ASKPASS_FILE);
    int32_t openvpnState_ = OPENVPN_STATE_UNKNOWN;
    void UpdateOpenvpnState(const int32_t state);
    int32_t StartOpenvpn();
    std::string MaskOpenvpnMessage(const std::string &msg);
    int32_t HandleClientMessage(const std::string &msg);
    int32_t SetUpVpnTun();
    void UpdateConfig(cJSON* jConfig);
    void UpdateState(cJSON* state);
    void StopOpenvpn();
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // OPENVPN_CTL_H