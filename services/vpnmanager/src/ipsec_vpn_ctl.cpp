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

#include "ipsec_vpn_ctl.h"

#include <string>

#include "base64_utils.h"
#include "netmgr_ext_log_wrapper.h"
#include "netmanager_base_common_utils.h"
#include "net_manager_ext_constants.h"

namespace OHOS {
namespace NetManagerStandard {
IpsecVpnCtl::IpsecVpnCtl(sptr<VpnConfig> config, const std::string &pkg, int32_t userId,
    std::vector<int32_t> &activeUserIds)
    : NetVpnImpl(config, pkg, userId, activeUserIds)
{}

IpsecVpnCtl::~IpsecVpnCtl()
{
    NETMGR_EXT_LOG_I("~IpsecVpnCtl");
}

int32_t IpsecVpnCtl::SetUp()
{
    return StartSysVpn();
}

int32_t IpsecVpnCtl::Destroy()
{
    return StopSysVpn();
}

int32_t IpsecVpnCtl::StopSysVpn()
{
    NETMGR_EXT_LOG_I("stop ipsec vpn");
    state_ = IpsecVpnStateCode::STATE_DISCONNECTED;
    NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_DOWN_HOME);
    NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_STOP);
    NotifyConnectState(VpnConnectState::VPN_DISCONNECTED);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t IpsecVpnCtl::StartSysVpn()
{
    NETMGR_EXT_LOG_I("start ipsec vpn");
    state_ = IpsecVpnStateCode::STATE_INIT;
    InitConfigFile();
    NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_RESTART);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t IpsecVpnCtl::InitConfigFile()
{
    CleanTempFiles();
    if (ipsecVpnConfig_ == nullptr) {
        NETMGR_EXT_LOG_E("InitConfigFile ipsecVpnConfig is null");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }

    if (!ipsecVpnConfig_->swanctlConf_.empty()) {
        std::string swanctlCfg = Base64::Decode(ipsecVpnConfig_->swanctlConf_);
        if (!swanctlCfg.empty()) {
            CommonUtils::WriteFile(SWAN_CTL_FILE, swanctlCfg);
        }
    }
    if (!ipsecVpnConfig_->strongswanConf_.empty()) {
        std::string strongswanCfg = Base64::Decode(ipsecVpnConfig_->strongswanConf_);
        if (!strongswanCfg.empty()) {
            CommonUtils::WriteFile(SWAN_CONFIG_FILE, strongswanCfg);
        }
    }
    return NETMANAGER_EXT_SUCCESS;
}

void IpsecVpnCtl::CleanTempFiles()
{
    DeleteTempFile(SWAN_CTL_FILE);
    DeleteTempFile(SWAN_CONFIG_FILE);
    DeleteTempFile(L2TP_CFG);
    DeleteTempFile(L2TP_IPSEC_CFG);
    DeleteTempFile(L2TP_IPSEC_SECRETS_CFG);
    DeleteTempFile(OPTIONS_L2TP_CLIENT);
}

void IpsecVpnCtl::DeleteTempFile(const std::string &fileName)
{
    if (std::filesystem::exists(fileName)) {
        if (!std::filesystem::remove(fileName)) {
            NETMGR_EXT_LOG_E("remove old cache file failed");
        }
    }
}

int32_t IpsecVpnCtl::NotifyConnectStage(const std::string &stage, const int32_t &result)
{
    if (stage.empty()) {
        NETMGR_EXT_LOG_E("stage is empty");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    if (result != NOTIFY_CONNECT_STAGE_SUCCESS) {
        NETMGR_EXT_LOG_E("vpn stage: %{public}s failed, result: %{public}d", stage.c_str(), result);
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    switch (state_) {
        case IpsecVpnStateCode::STATE_INIT:
            if (stage.compare(IPSEC_START_TAG) == 0) {
                // 1. start strongswan
                NETMGR_EXT_LOG_I("ipsec vpn setup step 1: start strongswan");
                state_ = IpsecVpnStateCode::STATE_STARTED;
                NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_SWANCTL_LOAD);
            }
            break;
        case IpsecVpnStateCode::STATE_STARTED:
            if (stage.compare(SWANCTL_START_TAG) == 0) {
                // 2. start connect
                NETMGR_EXT_LOG_I("ipsec vpn setup step 2: start connect");
                state_ = IpsecVpnStateCode::STATE_CONFIGED;
                NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_UP_HOME);
            }
            break;
        case IpsecVpnStateCode::STATE_CONFIGED:
            if (stage.compare(IPSEC_CONNECT_TAG) == 0) {
                // 3. is connected
                NETMGR_EXT_LOG_I("ipsec vpn setup step 3: is connected");
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

int32_t IpsecVpnCtl::GetConnectedSysVpnConfig(sptr<SysVpnConfig> &sysVpnConfig)
{
    if (state_ == IpsecVpnStateCode::STATE_CONNECTED && ipsecVpnConfig_ != nullptr) {
        NETMGR_EXT_LOG_I("GetConnectedSysVpnConfig success");
        sysVpnConfig = ipsecVpnConfig_;
    }
    return NETMANAGER_EXT_SUCCESS;
}

bool IpsecVpnCtl::IsInternalVpn()
{
    return true;
}
} // namespace NetManagerStandard
} // namespace OHOS