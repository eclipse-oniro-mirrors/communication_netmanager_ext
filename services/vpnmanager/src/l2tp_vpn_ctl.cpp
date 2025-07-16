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
    std::string connectName = L2TP_CONNECT_NAME;
    std::string ipsecName = IPSEC_CONNECT_NAME;
    if (multiVpnInfo_ != nullptr) {
        int32_t id = multiVpnInfo_->ifNameId;
        connectName = connectName + std::to_string(id);
        ipsecName = ipsecName + std::to_string(id);
    }
    if (l2tpVpnConfig_ != nullptr && l2tpVpnConfig_->vpnType_ != VpnType::L2TP) {
        NetsysController::GetInstance().ProcessVpnStage(
            SysVpnStageCode::VPN_STAGE_DOWN_HOME, ipsecName);
        MultiVpnHelper::GetInstance().StopIpsec();
    }
    NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_L2TP_STOP, connectName);
    MultiVpnHelper::GetInstance().StopL2tp();
    NotifyConnectState(VpnConnectState::VPN_DISCONNECTED);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t L2tpVpnCtl::StartSysVpn()
{
    NETMGR_EXT_LOG_I("start l2tp vpn");
    state_ = IpsecVpnStateCode::STATE_INIT;
    InitConfigFile();
    if (multiVpnInfo_ != nullptr) {
        NetsysController::GetInstance().ProcessVpnStage(
            SysVpnStageCode::VPN_STAGE_CREATE_PPP_FD, multiVpnInfo_->ifName);
    }
    if (l2tpVpnConfig_->vpnType_ == VpnType::L2TP) {
        state_ = IpsecVpnStateCode::STATE_CONFIGED;
        if (!MultiVpnHelper::GetInstance().StartL2tp()) {
            AddConfigToL2tpdConf();
        }
    } else {
        if (!MultiVpnHelper::GetInstance().StartIpsec()) {
            state_ = IpsecVpnStateCode::STATE_STARTED;
            NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_SWANCTL_LOAD);
        }
    }
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
        CommonUtils::WriteFile(SWAN_CONFIG_FILE, l2tpVpnConfig_->strongswanConf_);
    }
    if (!l2tpVpnConfig_->xl2tpdConf_.empty()) {
        CommonUtils::WriteFile(L2TP_CFG, l2tpVpnConfig_->xl2tpdConf_);
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
        NETMGR_EXT_LOG_E("l2tp vpn connect failed");
        HandleConnectFailed();
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    switch (state_) {
        case IpsecVpnStateCode::STATE_INIT:
            if (stage.compare(IPSEC_START_TAG) == 0) {
                HandleIpdecStarted();
            }
            break;
        case IpsecVpnStateCode::STATE_STARTED:
            if (stage.compare(SWANCTL_START_TAG) == 0) {
                HandleSwanCtlLoaded();
            }
            break;
        case IpsecVpnStateCode::STATE_CONFIGED:
            if (stage.compare(L2TP_IPSEC_CONFIGURED_TAG) == 0) {
                HandleL2tpConfiged();
            }
            break;
        case IpsecVpnStateCode::STATE_L2TP_STARTED:
            if (stage.compare(IPSEC_CONNECT_TAG) == 0) {
                HandleL2tpdCtl();
            }
            break;
        case IpsecVpnStateCode::STATE_CONTROLLED:
            if (stage.compare(L2TP_IPSEC_CONNECTED_TAG) == 0) {
                HandleL2tpConnected();
            }
            break;
        case IpsecVpnStateCode::STATE_CONNECTED:
            if (stage.find(IPSEC_NODE_UPDATE_CONFIG) != std::string::npos) {
                if (ProcessUpdateConfig(stage) != NETMANAGER_EXT_SUCCESS) {
                    return NETMANAGER_EXT_ERR_INTERNAL;
                }
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
        case IpsecVpnCertType::OPTIONS_L2TP_CLIENT_CONF:
            certUri = l2tpVpnConfig_->optionsL2tpdClient_;
            break;
        case IpsecVpnCertType::L2TP_IPSEC_SECRETS_CONF:
            certUri = l2tpVpnConfig_->ipsecSecrets_;
            break;
        case IpsecVpnCertType::SWAN_CTL_CONF:
            certUri = l2tpVpnConfig_->ipsecConf_;
            break;
        default:
            NETMGR_EXT_LOG_E("invalid certType: %{public}d", certType);
            break;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t L2tpVpnCtl::GetVpnCertData(const int32_t certType, std::vector<int8_t> &certData)
{
    if (l2tpVpnConfig_ == nullptr) {
        NETMGR_EXT_LOG_E("GetVpnCertData ipsecVpnConfig is null");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    switch (certType) {
        case IpsecVpnCertType::PKCS12_PASSWD: {
            if (!l2tpVpnConfig_->pkcs12Password_.empty()) {
                certData.assign(l2tpVpnConfig_->pkcs12Password_.begin(),
                    l2tpVpnConfig_->pkcs12Password_.end());
            } else {
                NETMGR_EXT_LOG_D("GetVpnCertData pkcs12 password is empty");
            }
            break;
        }
        case IpsecVpnCertType::PKCS12_DATA: {
            if (!l2tpVpnConfig_->pkcs12FileData_.empty()) {
                certData.assign(l2tpVpnConfig_->pkcs12FileData_.begin(),
                    l2tpVpnConfig_->pkcs12FileData_.end());
            } else {
                NETMGR_EXT_LOG_D("GetVpnCertData pkcs12 data is empty");
            }
            break;
        }
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

std::string L2tpVpnCtl::GetXl2tpdConfig()
{
    std::string templateContent;
    if (l2tpVpnConfig_ != nullptr && multiVpnInfo_ != nullptr && !l2tpVpnConfig_->addresses_.empty()) {
        templateContent.append(VPN_NAME_KEY).append(std::to_string(multiVpnInfo_->ifNameId))
            .append(SINGLE_XL2TP_CONFIG_LNS).append(l2tpVpnConfig_->addresses_[0].address_)
            .append(SINGLE_XL2TP_CONFIG_PPP).append(VPN_CLIENT_CONFIG_NAME_KEY)
            .append(std::to_string(multiVpnInfo_->ifNameId))
            .append(SINGLE_XL2TP_CONFIG_LENGTH);
    }
    templateContent = "\"" + templateContent + "\"";
    return templateContent;
}

void L2tpVpnCtl::AddConfigToL2tpdConf()
{
    std::string tempConfig = GetXl2tpdConfig();
    NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_SET_L2TP_CONF, tempConfig);
}

void L2tpVpnCtl::HandleIpdecStarted()
{
    NETMGR_EXT_LOG_I("1:ipsec started, process load swanctl config");
    state_ = IpsecVpnStateCode::STATE_STARTED;
    NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_SWANCTL_LOAD);
}

void L2tpVpnCtl::HandleSwanCtlLoaded()
{
    NETMGR_EXT_LOG_I("2:swanctl loaded, process start l2tp or add l2tp config");
    state_ = IpsecVpnStateCode::STATE_CONFIGED;
    if (!MultiVpnHelper::GetInstance().StartL2tp()) {
        AddConfigToL2tpdConf();
    }
}

void L2tpVpnCtl::HandleL2tpConfiged()
{
    NETMGR_EXT_LOG_I("3:l2tpd started or configed, process ipsec up");
    if (l2tpVpnConfig_->vpnType_ == VpnType::L2TP) {
        state_ = IpsecVpnStateCode::STATE_CONTROLLED;
        if (multiVpnInfo_ != nullptr) {
            NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_L2TP_CTL,
                std::string(L2TP_CONNECT_NAME) + std::to_string(multiVpnInfo_->ifNameId));
        } else {
            NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_L2TP_CTL);
        }
    } else {
        state_ = IpsecVpnStateCode::STATE_L2TP_STARTED;
        std::string baseConnectName = IPSEC_CONNECT_NAME;
        std::string connectName = multiVpnInfo_ == nullptr ? baseConnectName :
            baseConnectName + std::to_string(multiVpnInfo_->ifNameId);
        NetsysController::GetInstance().ProcessVpnStage(
            SysVpnStageCode::VPN_STAGE_UP_HOME, connectName);
    }
}

void L2tpVpnCtl::HandleL2tpdCtl()
{
    NETMGR_EXT_LOG_I("4:set stage IPSEC_L2TP_CTL, process echo c");
    state_ = IpsecVpnStateCode::STATE_CONTROLLED;
    if (multiVpnInfo_ != nullptr) {
        NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_L2TP_CTL,
            std::string(L2TP_CONNECT_NAME) + std::to_string(multiVpnInfo_->ifNameId));
    } else {
        NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_L2TP_CTL);
    }
}

void L2tpVpnCtl::HandleL2tpConnected()
{
    NETMGR_EXT_LOG_I("5:l2tp vpn is connected");
    state_ = IpsecVpnStateCode::STATE_CONNECTED;
    NotifyConnectState(VpnConnectState::VPN_CONNECTED);
}

int32_t L2tpVpnCtl::ProcessUpdateConfig(const std::string &config)
{
    NETMGR_EXT_LOG_I("6:l2tp vpn config update");
    if (UpdateConfig(config) != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_I("l2tp vpn config update failed");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    if (SetUpVpnTun() != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_I("set up l2tp vpn failed");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

void L2tpVpnCtl::HandleConnectFailed()
{
    if (multiVpnInfo_ != nullptr) {
        VpnHisysEvent::SetFaultVpnEvent(multiVpnInfo_->userId, multiVpnInfo_->bundleName,
            VpnOperatorType::OPERATION_SETUP_VPN,
            VpnOperatorErrorType::ERROR_CONFIG_WRONG, "l2tp vpn setup failed");
    }
    Destroy();
}
} // namespace NetManagerStandard
} // namespace OHOS
