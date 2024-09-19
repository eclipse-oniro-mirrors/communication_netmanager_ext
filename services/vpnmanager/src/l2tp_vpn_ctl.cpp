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

#include <string>

#include "base64_utils.h"
#include "netmgr_ext_log_wrapper.h"
#include "netmanager_base_common_utils.h"
#include "net_manager_ext_constants.h"

namespace OHOS {
namespace NetManagerStandard {
L2tpVpnCtl::L2tpVpnCtl(sptr<VpnConfig> config, const std::string &pkg, int32_t userId,
    std::vector<int32_t> &activeUserIds)
    : IpsecVpnCtl(config, pkg, userId, activeUserIds)
{}

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
        if (!strongswanCfg.empty()) {
            CommonUtils::WriteFile(SWAN_CONFIG_FILE, strongswanCfg);
        }
    }
    if (!l2tpVpnConfig_->xl2tpdConf_.empty()) {
        std::string xl2tpdConf = Base64::Decode(l2tpVpnConfig_->xl2tpdConf_);
        if (!xl2tpdConf.empty()) {
            CommonUtils::WriteFile(L2TP_CFG, xl2tpdConf);
        }
    }
    if (!l2tpVpnConfig_->ipsecConf_.empty()) {
        std::string ipsecConf = Base64::Decode(l2tpVpnConfig_->ipsecConf_);
        if (!ipsecConf.empty()) {
            CommonUtils::WriteFile(L2TP_IPSEC_CFG, ipsecConf);
        }
    }
    if (!l2tpVpnConfig_->ipsecSecrets_.empty()) {
        std::string ipsecSecrets = Base64::Decode(l2tpVpnConfig_->ipsecSecrets_);
        if (!ipsecSecrets.empty()) {
            CommonUtils::WriteFile(L2TP_IPSEC_SECRETS_CFG, ipsecSecrets);
        }
    }
    if (!l2tpVpnConfig_->optionsL2tpdClient_.empty()) {
        std::string optionsL2tpdClient = Base64::Decode(l2tpVpnConfig_->optionsL2tpdClient_);
        if (!optionsL2tpdClient.empty()) {
            CommonUtils::WriteFile(OPTIONS_L2TP_CLIENT, optionsL2tpdClient);
        }
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t L2tpVpnCtl::NotifyConnectStage(const std::string &stage, const int32_t &result)
{
    if (stage.empty()) {
        NETMGR_EXT_LOG_E("stage is empty");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    if (result != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("l2tpVpn stage: %{public}s failed, result: %{public}d", stage.c_str(), result);
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    switch (state_) {
        case IpsecVpnStateCode::STATE_INIT:
            if (stage.compare(IPSEC_START_TAG) == 0) {
                // 1. start l2tp
                NETMGR_EXT_LOG_I("l2tp vpn setup step 1: start l2tp");
                state_ = IpsecVpnStateCode::STATE_STARTED;
                NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_L2TP_LOAD);
            }
            break;
        case IpsecVpnStateCode::STATE_STARTED:
            if (stage.compare(L2TP_IPSEC_CONFIGURED_TAG) == 0) {
                // 2. start connect
                NETMGR_EXT_LOG_I("l2tp vpn setup step 2: start connect");
                state_ = IpsecVpnStateCode::STATE_CONFIGED;
                NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_UP_HOME);
            }
            break;
        case IpsecVpnStateCode::STATE_CONFIGED:
            if (stage.compare(IPSEC_CONNECT_TAG) == 0) {
                // 3. set stage IPSEC_L2TP_CTL
                NETMGR_EXT_LOG_I("l2tp vpn setup step 3: set stage IPSEC_L2TP_CTL");
                state_ = IpsecVpnStateCode::STATE_CONTROLLED;
                NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_L2TP_CTL);
            }
            break;
        case IpsecVpnStateCode::STATE_CONTROLLED:
            if (stage.compare(L2TP_IPSEC_CONNECTED_TAG) == 0) {
                // 4. is connected
                NETMGR_EXT_LOG_I("l2tp vpn setup step 4: is connected");
                state_ = IpsecVpnStateCode::STATE_CONNECTED;
                NotifyConnectState(VpnConnectState::VPN_CONNECTED);
            }
            break;
        default:
            NETMGR_EXT_LOG_E("invalid state: %{public}d", state_);
            return NETMANAGER_EXT_ERR_INTERNAL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t L2tpVpnCtl::GetSysVpnCertUri(const int32_t certType, std::string &certUri)
{
    if (l2tpVpnConfig_ == nullptr) {
        NETMGR_EXT_LOG_E("GetSysVpnCertUri l2tpVpnConfig_ is null");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    switch (certType) {
        case IpsecVpnCertType::CA_CERT:
            certUri = l2tpVpnConfig_->ipsecCaCertConf_;
            break;
        case IpsecVpnCertType::USER_CERT:
            certUri = l2tpVpnConfig_->ipsecPublicUserCertConf_;
            break;
        case IpsecVpnCertType::SERVER_CERT:
            certUri = l2tpVpnConfig_->ipsecPublicServerCertConf_;
            break;
        default:
            NETMGR_EXT_LOG_E("invalid certType: %{public}d", certType);
            break;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t L2tpVpnCtl::GetConnectedSysVpnConfig(sptr<SysVpnConfig> &sysVpnConfig)
{
    if (state_ == IpsecVpnStateCode::STATE_CONNECTED && l2tpVpnConfig_ != nullptr) {
        NETMGR_EXT_LOG_I("GetConnectedSysVpnConfig success");
        sysVpnConfig = l2tpVpnConfig_;
    }
    return NETMANAGER_EXT_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS
