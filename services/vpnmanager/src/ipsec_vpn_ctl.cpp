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

#include "multi_vpn_helper.h"
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

bool IpsecVpnCtl::IsSystemVpn()
{
    return true;
}

int32_t IpsecVpnCtl::SetUp()
{
    return StartSysVpn();
}

int32_t IpsecVpnCtl::Destroy()
{
    StopSysVpn();
    if (multiVpnInfo_ != nullptr) {
        NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_SET_VPN_CALL_MODE,
            multiVpnInfo_->isVpnExtCall ? "0" : "1");
    }
    int result = NetVpnImpl::Destroy();
    NETMGR_EXT_LOG_I("ipsec Destroy result %{public}d", result);
    return result;
}

int32_t IpsecVpnCtl::StopSysVpn()
{
    NETMGR_EXT_LOG_I("stop ipsec vpn");
    state_ = IpsecVpnStateCode::STATE_DISCONNECTED;
    std::string baseConnectName = IPSEC_CONNECT_NAME;
    std::string connectName = multiVpnInfo_ == nullptr ? baseConnectName :
        baseConnectName + std::to_string(multiVpnInfo_->ifNameId);
    NetsysController::GetInstance().ProcessVpnStage(
        SysVpnStageCode::VPN_STAGE_DOWN_HOME, connectName);
    MultiVpnHelper::GetInstance().StopIpsec();
    NotifyConnectState(VpnConnectState::VPN_DISCONNECTED);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t IpsecVpnCtl::StartSysVpn()
{
    NETMGR_EXT_LOG_I("start ipsec vpn");
    state_ = IpsecVpnStateCode::STATE_INIT;
    InitConfigFile();
    if (!MultiVpnHelper::GetInstance().StartIpsec()) {
        state_ = IpsecVpnStateCode::STATE_STARTED;
        NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_SWANCTL_LOAD);
    };
    return NETMANAGER_EXT_SUCCESS;
}

int32_t IpsecVpnCtl::InitConfigFile()
{
    CleanTempFiles();
    if (ipsecVpnConfig_ == nullptr) {
        NETMGR_EXT_LOG_E("InitConfigFile ipsecVpnConfig is null");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    if (!ipsecVpnConfig_->strongswanConf_.empty()) {
        CommonUtils::WriteFile(SWAN_CONFIG_FILE, ipsecVpnConfig_->strongswanConf_);
    }
    return NETMANAGER_EXT_SUCCESS;
}

void IpsecVpnCtl::CleanTempFiles()
{
    DeleteTempFile(SWAN_CONFIG_FILE);
    DeleteTempFile(L2TP_CFG);
}

void IpsecVpnCtl::DeleteTempFile(const std::string &fileName)
{
    if (std::filesystem::exists(fileName)) {
        if (!std::filesystem::remove(fileName)) {
            NETMGR_EXT_LOG_E("remove old cache file failed");
        }
    }
}

int32_t IpsecVpnCtl::SetUpVpnTun()
{
    if (multiVpnInfo_ != nullptr) {
        NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_SET_VPN_CALL_MODE,
            multiVpnInfo_->isVpnExtCall ? "0" : "1");
    }
    if (NetVpnImpl::SetUp() != NETMANAGER_EXT_SUCCESS) {
        StopSysVpn();
        NETMGR_EXT_LOG_I("ipsec SetUp failed");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    if (multiVpnInfo_ != nullptr && vpnConfig_ != nullptr) {
        multiVpnInfo_->localAddress = vpnConfig_->addresses_.empty() ?
            "" : vpnConfig_->addresses_.back().address_;
    }
    NETMGR_EXT_LOG_I("ipsec SetUp success");
    return NETMANAGER_EXT_SUCCESS;
}

int32_t IpsecVpnCtl::UpdateConfig(const std::string &msg)
{
    if (msg.empty()) {
        NETMGR_EXT_LOG_E("msg is empty");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    const char *ret = strstr(msg.c_str(), "{");
    if (ret == nullptr) {
        NETMGR_EXT_LOG_E("client rootJson format error");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    cJSON* rootJson = cJSON_Parse(ret);
    if (rootJson == nullptr) {
        NETMGR_EXT_LOG_E("not json string");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }

    cJSON* jConfig = cJSON_GetObjectItem(rootJson, IPSEC_NODE_UPDATE_CONFIG);
    if (!cJSON_IsObject(jConfig)) {
        NETMGR_EXT_LOG_E("jConfig format error");
        cJSON_Delete(rootJson);
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    ProcessUpdateConfig(jConfig);

    cJSON_Delete(rootJson);
    rootJson = nullptr;
    if (vpnConfig_ != nullptr) {
        std::string addr = vpnConfig_->addresses_.empty() ? "" : vpnConfig_->addresses_.back().address_;
        if (MultiVpnHelper::GetInstance().CheckAndCompareMultiVpnLocalAddress(addr) != NETMANAGER_EXT_SUCCESS) {
            NETMGR_EXT_LOG_E("ipsec check ip address is same error.");
            return NETMANAGER_EXT_ERR_INTERNAL;
        }
    }
    return NETMANAGER_EXT_SUCCESS;
}


int32_t IpsecVpnCtl::NotifyConnectStage(const std::string &stage, const int32_t &result)
{
    if (stage.empty()) {
        NETMGR_EXT_LOG_E("stage is empty");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    if (result != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("ipsec vpn connect failed");
        if (multiVpnInfo_ != nullptr) {
            VpnHisysEvent::SetFaultVpnEvent(multiVpnInfo_->userId, multiVpnInfo_->bundleName,
                VpnOperatorType::OPERATION_SETUP_VPN,
                VpnOperatorErrorType::ERROR_CONFIG_WRONG, "ipsec vpn setup failed");
        }
        Destroy();
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    switch (state_) {
        case IpsecVpnStateCode::STATE_INIT:
            if (stage.compare(IPSEC_START_TAG) == 0) {
                ProcessSwanctlLoad();
            }
            break;
        case IpsecVpnStateCode::STATE_STARTED:
            if (stage.compare(SWANCTL_START_TAG) == 0) {
                ProcessIpsecUp();
            }
            break;
        case IpsecVpnStateCode::STATE_CONFIGED:
            if (stage.compare(IPSEC_CONNECT_TAG) == 0) {
                HandleConnected();
            }
            if (stage.find(IPSEC_NODE_UPDATE_CONFIG) != std::string::npos) {
                if (HandleUpdateConfig(stage) != NETMANAGER_EXT_SUCCESS) {
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

int32_t IpsecVpnCtl::GetSysVpnCertUri(const int32_t certType, std::string &certUri)
{
    if (ipsecVpnConfig_ == nullptr) {
        NETMGR_EXT_LOG_E("GetSysVpnCertUri ipsecVpnConfig is null");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    switch (certType) {
        case IpsecVpnCertType::CA_CERT:
            certUri = ipsecVpnConfig_->ipsecCaCertConf_;
            break;
        case IpsecVpnCertType::USER_CERT:
            certUri = ipsecVpnConfig_->ipsecPublicUserCertConf_;
            break;
        case IpsecVpnCertType::SERVER_CERT:
            certUri = ipsecVpnConfig_->ipsecPublicServerCertConf_;
            break;
        case IpsecVpnCertType::SWAN_CTL_CONF:
            certUri = ipsecVpnConfig_->swanctlConf_;
            break;
        default:
            NETMGR_EXT_LOG_E("invalid certType: %{public}d", certType);
            break;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t IpsecVpnCtl::GetVpnCertData(const int32_t certType, std::vector<int8_t> &certData)
{
    if (ipsecVpnConfig_ == nullptr) {
        NETMGR_EXT_LOG_E("GetSysVpnCertUri ipsecVpnConfig is null");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    switch (certType) {
        case IpsecVpnCertType::PKCS12_PASSWD: {
            if (!ipsecVpnConfig_->pkcs12Password_.empty()) {
                certData.assign(ipsecVpnConfig_->pkcs12Password_.begin(),
                    ipsecVpnConfig_->pkcs12Password_.end());
            } else {
                NETMGR_EXT_LOG_D("GetVpnCertData pkcs12 password is empty");
            }
            break;
        }
        case IpsecVpnCertType::PKCS12_DATA: {
            if (!ipsecVpnConfig_->pkcs12FileData_.empty()) {
                certData.assign(ipsecVpnConfig_->pkcs12FileData_.begin(),
                    ipsecVpnConfig_->pkcs12FileData_.end());
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

void IpsecVpnCtl::ProcessUpdateConfig(cJSON* jConfig)
{
    if (vpnConfig_ == nullptr) {
        NETMGR_EXT_LOG_E("UpdateConfig vpnConfig_ is null");
        return;
    }
    cJSON *mtu = cJSON_GetObjectItem(jConfig, IPSEC_NODE_MTU);
    if (mtu != nullptr && cJSON_IsNumber(mtu)) {
        int32_t ipsecVpnMtu = static_cast<int32_t>(cJSON_GetNumberValue(mtu));
        vpnConfig_->mtu_ = ipsecVpnMtu;
        NETMGR_EXT_LOG_I("UpdateConfig mtu %{public}d", ipsecVpnMtu);
    }

    INetAddr iNetAddr;
    INetAddr destination;
    INetAddr gateway;
    cJSON *address = cJSON_GetObjectItem(jConfig, IPSEC_NODE_ADDRESS);
    if (address != nullptr && cJSON_IsString(address)) {
        std::string ipsecVpnAddress = cJSON_GetStringValue(address);
        iNetAddr.address_ = ipsecVpnAddress;
        gateway.address_ = ipsecVpnAddress;
        destination.address_ = ipsecVpnAddress;
    }

    cJSON *netmask = cJSON_GetObjectItem(jConfig, IPSEC_NODE_NETMASK);
    if (netmask != nullptr && cJSON_IsString(netmask)) {
        std::string ipsecVpnNetmask = cJSON_GetStringValue(netmask);
        iNetAddr.netMask_ = ipsecVpnNetmask;
        destination.prefixlen_ = CommonUtils::GetMaskLength(ipsecVpnNetmask);
    }

    cJSON *phyIfNameObj = cJSON_GetObjectItem(jConfig, IPSEC_NODE_PHY_NAME);
    if (phyIfNameObj != nullptr && cJSON_IsString(phyIfNameObj)) {
        std::string phyIfName = cJSON_GetStringValue(phyIfNameObj);
        NETMGR_EXT_LOG_I("phyIfName:%{public}s", phyIfName.c_str());
        NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_SET_XFRM_PHY_IFNAME, phyIfName);
    }

    cJSON *dstIpObj = cJSON_GetObjectItem(jConfig, IPSEC_NODE_REMOTE_IP);
    if (dstIpObj != nullptr && cJSON_IsString(dstIpObj)) {
        std::string remoteIp = cJSON_GetStringValue(dstIpObj);
        NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_SET_VPN_REMOTE_ADDRESS, remoteIp);
    }
    vpnConfig_->addresses_.emplace_back(iNetAddr);
    ProcessDnsConfig(jConfig);
}

void IpsecVpnCtl::ProcessDnsConfig(cJSON* jConfig)
{
    if (vpnConfig_ == nullptr) {
        NETMGR_EXT_LOG_E("UpdateConfig vpnConfig_ is null");
        return;
    }

    for (auto it = vpnConfig_->dnsAddresses_.begin(); it != vpnConfig_->dnsAddresses_.end(); ) {
        if (it->empty()) {
            it = vpnConfig_->dnsAddresses_.erase(it);
        } else {
            ++it;
        }
    }
    cJSON *dns1Obj = cJSON_GetObjectItem(jConfig, PRIMARY_DNS);
    if (dns1Obj != nullptr && cJSON_IsString(dns1Obj)) {
        std::string dns1 = cJSON_GetStringValue(dns1Obj);
        if (!dns1.empty()) {
            vpnConfig_->dnsAddresses_.emplace_back(dns1);
        }
    }

    cJSON *dns2Obj = cJSON_GetObjectItem(jConfig, SECONDARY_DNS);
    if (dns2Obj != nullptr && cJSON_IsString(dns2Obj)) {
        std::string dns2 = cJSON_GetStringValue(dns2Obj);
        if (!dns2.empty()) {
            vpnConfig_->dnsAddresses_.emplace_back(dns2);
        }
    }
}

void IpsecVpnCtl::ProcessSwanctlLoad()
{
    // 1. start strongswan
    NETMGR_EXT_LOG_I("ipsec vpn setup step 1: start strongswan");
    state_ = IpsecVpnStateCode::STATE_STARTED;
    NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_SWANCTL_LOAD);
}

void IpsecVpnCtl::ProcessIpsecUp()
{
    // 2. start connect
    NETMGR_EXT_LOG_I("ipsec vpn setup step 2: start connect");
    state_ = IpsecVpnStateCode::STATE_CONFIGED;
    std::string baseConnectName = IPSEC_CONNECT_NAME;
    std::string connectName = multiVpnInfo_ == nullptr ? baseConnectName :
        baseConnectName + std::to_string(multiVpnInfo_->ifNameId);
    NetsysController::GetInstance().ProcessVpnStage(
        SysVpnStageCode::VPN_STAGE_UP_HOME, std::string(connectName));
}

void IpsecVpnCtl::HandleConnected()
{
    // 3. is connected
    NETMGR_EXT_LOG_I("ipsec vpn setup step 3: is connected");
    state_ = IpsecVpnStateCode::STATE_CONNECTED;
    NotifyConnectState(VpnConnectState::VPN_CONNECTED);
}

int32_t IpsecVpnCtl::HandleUpdateConfig(const std::string &config)
{
    if (UpdateConfig(config) != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_I("ipsec vpn config update failed");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    if (SetUpVpnTun() != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_I("set up l2tp vpn failed");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    return NETMANAGER_EXT_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS