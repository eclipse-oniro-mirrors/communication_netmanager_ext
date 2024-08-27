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

#include "l2tp_vpn_ctl.h"

#include <fstream>
#include <unistd.h>
#include <iostream>
#include <filesystem>
#include <string>

#include "base64_utils.h"
#include "netmgr_ext_log_wrapper.h"

#include "netmanager_base_common_utils.h"
#include "net_manager_ext_constants.h"

namespace OHOS {
namespace NetManagerStandard {
L2tpVpnCtl::L2tpVpnCtl(sptr<VpnConfig> config, const std::string &pkg, int32_t userId,
    std::vector<int32_t> &activeUserIds) : IpsecVpnCtl(config, pkg, userId, activeUserIds)
{
}

int32_t L2tpVpnCtl::StopSysVpn()
{
    NETMGR_EXT_LOG_I("stop l2tp vpn");
    state_ = IpsecVpnStateCode::STATE_DISCONNECTED;
    NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_DOWN_HOME);
    NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_STOP);
    NotifyConnectState(VpnConnectState::VPN_DISCONNECTED);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t L2tpVpnCtl::StartSysVpn()
{
    NETMGR_EXT_LOG_I("start l2tp vpn");
    state_ = IpsecVpnStateCode::STATE_INIT;
    InitConfigFile();
    NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_RESTART);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t L2tpVpnCtl::InitConfigFile()
{
    CleanTempFiles();
    if (l2tpVpnConfig_ == nullptr) {
        NETMGR_EXT_LOG_E("InitConfigFile failed, l2tpVpnConfig_ is null");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    if (!l2tpVpnConfig_->strongswanConf_.empty()) {
        std::string strongswanCfg = Base64::Decode(l2tpVpnConfig_->strongswanConf_);
        std::ofstream ofs(SWAN_CONFIG_FILE);
        ofs << strongswanCfg;
    }
    if (!l2tpVpnConfig_->xl2tpdConf_.empty()) {
        std::string xl2tpdConf = Base64::Decode(l2tpVpnConfig_->xl2tpdConf_);
        std::ofstream ofs(L2TP_CFG);
        ofs << xl2tpdConf;
    }
    if (!l2tpVpnConfig_->ipsecConf_.empty()) {
        std::string ipsecConf = Base64::Decode(l2tpVpnConfig_->ipsecConf_);
        std::ofstream ofs(L2TP_IPSEC_CFG);
        ofs << ipsecConf;
    }
    if (!l2tpVpnConfig_->ipsecSecrets_.empty()) {
        std::string ipsecSecrets = Base64::Decode(l2tpVpnConfig_->ipsecSecrets_);
        std::ofstream ofs(L2TP_IPSEC_SECRETS_CFG);
        ofs << ipsecSecrets;
    }
    if (!l2tpVpnConfig_->optionsL2tpdClient_.empty()) {
        std::string optionsL2tpdClient = Base64::Decode(l2tpVpnConfig_->optionsL2tpdClient_);
        std::ofstream ofs(OPTIONS_L2TP_CLIENT);
        ofs << optionsL2tpdClient;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t L2tpVpnCtl::NotifyConnectStage(std::string &stage, int32_t &errorCode)
{
    if (errorCode != NOTIFY_CONNECT_STAGE_SUCCESS) {
        NETMGR_EXT_LOG_E("invalid vpn stage, stage: %{public}s, error: %{public}d", stage.c_str(), errorCode);
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    NETMGR_EXT_LOG_I("parse vpn stage, stage: %{public}s, state_: %{public}d", stage.c_str(), state_);
    switch (state_) {
        case IpsecVpnStateCode::STATE_INIT:
            if (stage.compare(IPSEC_START_TAG) == 0) {
                // 1. start l2tp
                NETMGR_EXT_LOG_I("l2tp vpn setup step 1: start l2tp");
                state_ = IpsecVpnStateCode::STATE_STARTED;
                NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_L2TP_LOAD);
            }
            return NETMANAGER_EXT_SUCCESS;
        case IpsecVpnStateCode::STATE_STARTED:
            if (stage.compare(L2TP_IPSEC_CONFIGURED_TAG) == 0) {
                // 2. start connect
                NETMGR_EXT_LOG_I("l2tp vpn setup step 2: start connect");
                state_ = IpsecVpnStateCode::STATE_CONFIGED;
                NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_UP_HOME);
            }
            return NETMANAGER_EXT_SUCCESS;
        case IpsecVpnStateCode::STATE_CONFIGED:
            if (stage.compare(IPSEC_CONNECT_TAG) == 0) {
                // 3. set stage IPSEC_L2TP_CTL
                NETMGR_EXT_LOG_I("l2tp vpn setup step 3: set stage IPSEC_L2TP_CTL");
                NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_L2TP_CTL);
            } else if (stage.compare(L2TP_IPSEC_CONNECTED_TAG) == 0) {
                // 4. is connected
                NETMGR_EXT_LOG_I("l2tp vpn setup step 4: is connected");
                state_ = IpsecVpnStateCode::STATE_CONNECTED;
                NotifyConnectState(VpnConnectState::VPN_CONNECTED);
            }
            return NETMANAGER_EXT_SUCCESS;
        default:
            NETMGR_EXT_LOG_E("invalid state: %{public}d", state_);
            return NETMANAGER_EXT_ERR_INTERNAL;
    }
}

int32_t L2tpVpnCtl::GetConnectedSysVpnConfig(sptr<SysVpnConfig> &sysVpnConfig)
{
    if (state_ == IpsecVpnStateCode::STATE_CONNECTED && l2tpVpnConfig_ != nullptr) {
        NETMGR_EXT_LOG_I("GetConnectedSysVpnConfig success.");
        sysVpnConfig = l2tpVpnConfig_;
    }
    return NETMANAGER_EXT_SUCCESS;
}

bool L2tpVpnCtl::isSysVpnImpl()
{
    return true;
}
}
}
