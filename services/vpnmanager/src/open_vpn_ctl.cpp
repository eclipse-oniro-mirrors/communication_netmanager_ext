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

#include "open_vpn_ctl.h"

#include <fstream>

#include "base64_utils.h"
#include "netmanager_base_common_utils.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {

OpenvpnCtl::OpenvpnCtl(sptr<VpnConfig> config, const std::string &pkg, int32_t userId,
    std::vector<int32_t> &activeUserIds) : NetVpnImpl(config, pkg, userId, activeUserIds)
{
}

int32_t OpenvpnCtl::SetUp()
{
    UpdateOpenvpnState(OPENVPN_STATE_SETUP);
    return StartOpenvpn();
}

int32_t OpenvpnCtl::StartOpenvpn()
{
    if (openvpnConfig_ == nullptr) {
        NETMGR_EXT_LOG_E("StartOpenvpn openvpnConfig_ is null");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    UpdateOpenvpnState(OPENVPN_STATE_STARTED);
    if (!std::filesystem::exists(VPN_PIDDIR) || !std::filesystem::is_directory(VPN_PIDDIR)) {
        NETMGR_EXT_LOG_E("StartOpenvpn config dir check error.");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    std::string cfg = Base64::Decode(openvpnConfig_->ovpnConfig_);
    std::ofstream ofs(OPENVPN_CONFIG_FILE, std::ios::out | std::ios::trunc);
    if (!ofs.is_open()) {
        NETMGR_EXT_LOG_E("StartOpenvpn config file open failed");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    ofs << cfg;
    if (!openvpnConfig_->askpass_.empty()) {
        std::ofstream askpassOfs(OPENVPN_ASKPASS_FILE, std::ios::out | std::ios::trunc);
        if (!askpassOfs.is_open()) {
            NETMGR_EXT_LOG_E("StartOpenvpn askpass file open failed");
            return NETMANAGER_EXT_ERR_INTERNAL;
        }
        askpassOfs << openvpnConfig_->askpass_ << std::endl;
        ofs << OPENVPN_ASKPASS_PARAM << std::endl;
    }
    NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_OPENVPN_RESTART);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t OpenvpnCtl::NotifyConnectStage(const std::string &stage, const int32_t &result)
{
    if (stage.empty()) {
        NETMGR_EXT_LOG_E("stage is empty");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    if (result != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("vpn stage failed, result: %{public}d", result);
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    return HandleClientMessage(stage);
}

int32_t OpenvpnCtl::SetUpVpnTun()
{
    int result = NetVpnImpl::SetUp();
    if (result != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_W("openvpn SetUp failed");
        StopOpenvpn();
        UpdateOpenvpnState(OPENVPN_STATE_DISCONNECTED);
    }
    NETMGR_EXT_LOG_I("openvpn SetUp %{public}d", result);
    return result;
}

int32_t OpenvpnCtl::HandleClientMessage(const std::string &msg)
{
    if (msg.empty()) {
        NETMGR_EXT_LOG_E("msg is empty");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    NETMGR_EXT_LOG_I("Process Request  message:  %{public}s", MaskOpenvpnMessage(msg).c_str());
    int result = NETMANAGER_EXT_SUCCESS;
    if (strstr(msg.c_str(), OPENVPN_NODE_ROOT) != 0) {
        const char *ret = strstr(msg.c_str(), "{");
        if (ret == nullptr) {
            NETMGR_EXT_LOG_E("client message format error");
            result = NETMANAGER_EXT_ERR_PARAMETER_ERROR;
        }
        cJSON* message = cJSON_Parse(ret);
        if (message == nullptr) {
            NETMGR_EXT_LOG_E("not json string");
            result = NETMANAGER_EXT_ERR_PARAMETER_ERROR;
        }
        cJSON* config = cJSON_GetObjectItem(message, OPENVPN_NODE_CONFIG);
        if (config != nullptr && cJSON_IsObject(config)) {
            UpdateConfig(config);
        }
        cJSON* updateState = cJSON_GetObjectItem(message, OPENVPN_NODE_UPDATE_STATE);
        if (updateState != nullptr && cJSON_IsObject(updateState)) {
            UpdateState(updateState);
        }
        cJSON* setupVpnTun = cJSON_GetObjectItem(message, OPENVPN_NODE_SETUP_VPN_TUN);
        if (setupVpnTun != nullptr && cJSON_IsObject(setupVpnTun)) {
            result = SetUpVpnTun();
        }
        cJSON_Delete(message);
    }
    return result;
}

void OpenvpnCtl::UpdateState(cJSON* state)
{
    cJSON* updateState = cJSON_GetObjectItem(state, OPENVPN_NODE_STATE);
    if (updateState != nullptr && cJSON_IsNumber(updateState)) {
        int32_t openVpnState = static_cast<int32_t>(cJSON_GetNumberValue(updateState));
        UpdateOpenvpnState(openVpnState);
        if (openVpnState == OPENVPN_STATE_DISCONNECTED || openVpnState >= OPENVPN_STATE_ERROR_PRIVATE_KEY) {
            NETMGR_EXT_LOG_I("UpdatesState:  %{public}d", openVpnState);
            StopOpenvpn();
        }
    }
}

void OpenvpnCtl::UpdateConfig(cJSON *jConfig)
{
    if (vpnConfig_ == nullptr) {
        NETMGR_EXT_LOG_E("UpdateConfig vpnConfig_ is null");
        return;
    }
    cJSON *mtu = cJSON_GetObjectItem(jConfig, OPENVPN_NODE_MTU);
    if (mtu != nullptr && cJSON_IsNumber(mtu)) {
        int32_t openVpnMtu = static_cast<int32_t>(cJSON_GetNumberValue(mtu));
        vpnConfig_->mtu_ = openVpnMtu;
        NETMGR_EXT_LOG_I("UpdateConfig mtu %{public}d", openVpnMtu);
    }
    INetAddr iNetAddr;
    INetAddr destination;
    INetAddr gateway;
    Route iRoute;
    cJSON *address = cJSON_GetObjectItem(jConfig, OPENVPN_NODE_ADDRESS);
    if (address != nullptr && cJSON_IsString(address)) {
        std::string openVpnAddress = cJSON_GetStringValue(address);
        iNetAddr.address_ = openVpnAddress;
        gateway.address_ = openVpnAddress;
        destination.address_ = openVpnAddress;
    }
    cJSON *netmask = cJSON_GetObjectItem(jConfig, OPENVPN_NODE_NETMASK);
    if (netmask != nullptr && cJSON_IsString(netmask)) {
        std::string openVpnNetmask = cJSON_GetStringValue(netmask);
        iNetAddr.netMask_ = openVpnNetmask;
        destination.prefixlen_ = CommonUtils::GetMaskLength(openVpnNetmask);
        NETMGR_EXT_LOG_I("UpdateConfig prefixlen %{public}d", destination.prefixlen_);
    }
    vpnConfig_->addresses_.emplace_back(iNetAddr);

    iRoute.iface_ = TUN_CARD_NAME;
    iRoute.isDefaultRoute_ = true;
    iRoute.destination_ = destination;
    iRoute.gateway_ = gateway;
    vpnConfig_->routes_.emplace_back(iRoute);
}

void OpenvpnCtl::UpdateOpenvpnState(const int32_t state)
{
    switch (state) {
        case OPENVPN_STATE_CONNECTED:
            NotifyConnectState(VpnConnectState::VPN_CONNECTED);
            break;
        case OPENVPN_STATE_DISCONNECTED:
        case OPENVPN_STATE_ERROR_PRIVATE_KEY:
        case OPENVPN_STATE_ERROR_CLIENT_CRT:
        case OPENVPN_STATE_ERROR_CA_CAT:
        case OPENVPN_STATE_ERROR_TIME_OUT:
            NotifyConnectState(VpnConnectState::VPN_DISCONNECTED);
            break;
        default:
            NETMGR_EXT_LOG_E("unknown openvpn state: %{public}d", state);
            break;
    }
    openvpnState_ = state;
}

bool OpenvpnCtl::IsSystemVpn()
{
    return true;
}

int32_t OpenvpnCtl::Destroy()
{
    StopOpenvpn();
    int result = NetVpnImpl::Destroy();
    NETMGR_EXT_LOG_I("openvpn Destroy result %{public}d}", result);
    return result;
}

void OpenvpnCtl::StopOpenvpn()
{
    NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_OPENVPN_STOP);
    UpdateOpenvpnState(OPENVPN_STATE_DISCONNECTED);
}

int32_t OpenvpnCtl::GetConnectedSysVpnConfig(sptr<SysVpnConfig> &sysVpnConfig)
{
    if (openvpnState_ == OPENVPN_STATE_CONNECTED && openvpnConfig_ != nullptr) {
        sysVpnConfig = openvpnConfig_;
    }
    return NETMANAGER_EXT_SUCCESS;
}

bool OpenvpnCtl::IsInternalVpn()
{
    return true;
}

std::string OpenvpnCtl::MaskOpenvpnMessage(const std::string &msg)
{
    if (msg.empty()) {
        NETMGR_EXT_LOG_E("msg is empty");
        return "";
    }
    std::string result = msg;
    size_t addressPos = result.find(OPENVPN_NODE_CONFIG);
    if (addressPos != std::string::npos) {
        size_t pos = addressPos + strlen(OPENVPN_NODE_CONFIG);
        size_t replaceLen = result.size() - pos;
        if (replaceLen > 0) {
            result.replace(pos, replaceLen, OPENVPN_MASK_TAG);
        }
        return result;
    }

    return msg;
}
} // namespace NetManagerStandard
} // namespace OHOS