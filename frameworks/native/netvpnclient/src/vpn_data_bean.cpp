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

#include "vpn_data_bean.h"
#include "netmgr_ext_log_wrapper.h"
#include "net_manager_ext_constants.h"

namespace OHOS {
namespace NetManagerStandard {
sptr<SysVpnConfig> VpnDataBean::ConvertVpnBeanToSysVpnConfig(sptr<VpnDataBean> &vpnBean)
{
    if (vpnBean == nullptr) {
        NETMGR_EXT_LOG_E("ConvertVpnBeanToSysVpnConfig vpnBean is null");
        return nullptr;
    }
    switch (vpnBean->vpnType_) {
        case VpnType::IKEV2_IPSEC_MSCHAPv2:
        case VpnType::IKEV2_IPSEC_PSK:
        case VpnType::IKEV2_IPSEC_RSA:
        case VpnType::IPSEC_XAUTH_PSK:
        case VpnType::IPSEC_XAUTH_RSA:
        case VpnType::IPSEC_HYBRID_RSA:
            return ConvertVpnBeanToIpsecVpnConfig(vpnBean);
        case VpnType::L2TP_IPSEC_PSK:
        case VpnType::L2TP_IPSEC_RSA:
            return ConvertVpnBeanToL2tpVpnConfig(vpnBean);
        default:
            NETMGR_EXT_LOG_E("ConvertVpnBeanToSysVpnConfig failed, invalid type=%{public}d", vpnBean->vpnType_);
            return nullptr;
    }
}

sptr<IpsecVpnConfig> VpnDataBean::ConvertVpnBeanToIpsecVpnConfig(sptr<VpnDataBean> &vpnBean)
{
    if (vpnBean == nullptr) {
        NETMGR_EXT_LOG_E("ConvertVpnBeanToIpsecVpnConfig vpnBean is null");
        return nullptr;
    }
    sptr<IpsecVpnConfig> ipsecVpnConfig = new (std::nothrow) IpsecVpnConfig();
    if (ipsecVpnConfig == nullptr) {
        NETMGR_EXT_LOG_E("ConvertVpnBeanToIpsecVpnConfig ipsecVpnConfig is null");
        return nullptr;
    }
    ipsecVpnConfig->vpnId_ = vpnBean->vpnId_;
    ipsecVpnConfig->vpnName_ = vpnBean->vpnName_;
    ipsecVpnConfig->vpnType_ = vpnBean->vpnType_;
    sptr<INetAddr> netAddr = new (std::nothrow) INetAddr();
    netAddr->address_ = vpnBean->vpnAddress_;
    ipsecVpnConfig->addresses_.push_back(*netAddr);
    ipsecVpnConfig->userName_ = vpnBean->userName_;
    ipsecVpnConfig->password_ = vpnBean->password_;
    ipsecVpnConfig->userId_ = vpnBean->userId_;
    ipsecVpnConfig->isLegacy_ = (vpnBean->isLegacy_) == 1;
    ipsecVpnConfig->saveLogin_ = (vpnBean->saveLogin_) == 1;
    ipsecVpnConfig->forwardingRoutes_ = vpnBean->forwardingRoutes_;
    ipsecVpnConfig->dnsAddresses_.push_back(vpnBean->dnsAddresses_);
    ipsecVpnConfig->searchDomains_.push_back(vpnBean->searchDomains_);

    ipsecVpnConfig->ipsecPreSharedKey_ = vpnBean->ipsecPreSharedKey_;
    ipsecVpnConfig->ipsecIdentifier_ = vpnBean->ipsecIdentifier_;
    ipsecVpnConfig->swanctlConf_ = vpnBean->swanctlConf_;
    ipsecVpnConfig->strongswanConf_ = vpnBean->strongswanConf_;
    ipsecVpnConfig->ipsecCaCertConf_ = vpnBean->ipsecCaCertConf_;
    ipsecVpnConfig->ipsecPrivateUserCertConf_ = vpnBean->ipsecPrivateUserCertConf_;
    ipsecVpnConfig->ipsecPublicUserCertConf_ = vpnBean->ipsecPublicUserCertConf_;
    ipsecVpnConfig->ipsecPrivateServerCertConf_ = vpnBean->ipsecPrivateServerCertConf_;
    ipsecVpnConfig->ipsecPublicServerCertConf_ = vpnBean->ipsecPublicServerCertConf_;
    ipsecVpnConfig->ipsecCaCertFilePath_ = vpnBean->ipsecCaCertFilePath_;
    ipsecVpnConfig->ipsecPrivateUserCertFilePath_ = vpnBean->ipsecPrivateUserCertFilePath_;
    ipsecVpnConfig->ipsecPublicUserCertFilePath_ = vpnBean->ipsecPublicUserCertFilePath_;
    ipsecVpnConfig->ipsecPrivateServerCertFilePath_ = vpnBean->ipsecPrivateServerCertFilePath_;
    ipsecVpnConfig->ipsecPublicServerCertFilePath_ = vpnBean->ipsecPublicServerCertFilePath_;
    return ipsecVpnConfig;
}

sptr<L2tpVpnConfig> VpnDataBean::ConvertVpnBeanToL2tpVpnConfig(sptr<VpnDataBean> &vpnBean)
{
    if (vpnBean == nullptr) {
        NETMGR_EXT_LOG_E("ConvertVpnBeanToL2tpVpnConfig vpnBean is null");
        return nullptr;
    }
    sptr<L2tpVpnConfig> l2tpVpnConfig = new (std::nothrow) L2tpVpnConfig();
    if (l2tpVpnConfig == nullptr) {
        NETMGR_EXT_LOG_E("ConvertVpnBeanToL2tpVpnConfig l2tpVpnConfig is null");
        return nullptr;
    }
    l2tpVpnConfig->vpnId_ = vpnBean->vpnId_;
    l2tpVpnConfig->vpnName_ = vpnBean->vpnName_;
    l2tpVpnConfig->vpnType_ = vpnBean->vpnType_;
    sptr<INetAddr> netAddr = new (std::nothrow) INetAddr();
    netAddr->address_ = vpnBean->vpnAddress_;
    l2tpVpnConfig->addresses_.push_back(*netAddr);
    l2tpVpnConfig->userName_ = vpnBean->userName_;
    l2tpVpnConfig->password_ = vpnBean->password_;
    l2tpVpnConfig->userId_ = vpnBean->userId_;
    l2tpVpnConfig->isLegacy_ = (vpnBean->isLegacy_) == 1;
    l2tpVpnConfig->saveLogin_ = (vpnBean->saveLogin_) == 1;
    l2tpVpnConfig->forwardingRoutes_ = vpnBean->forwardingRoutes_;
    l2tpVpnConfig->dnsAddresses_.push_back(vpnBean->dnsAddresses_);
    l2tpVpnConfig->searchDomains_.push_back(vpnBean->searchDomains_);

    l2tpVpnConfig->ipsecPreSharedKey_ = vpnBean->ipsecPreSharedKey_;
    l2tpVpnConfig->ipsecIdentifier_ = vpnBean->ipsecIdentifier_;
    l2tpVpnConfig->strongswanConf_ = vpnBean->strongswanConf_;
    l2tpVpnConfig->ipsecCaCertConf_ = vpnBean->ipsecCaCertConf_;
    l2tpVpnConfig->ipsecPrivateUserCertConf_ = vpnBean->ipsecPrivateUserCertConf_;
    l2tpVpnConfig->ipsecPublicUserCertConf_ = vpnBean->ipsecPublicUserCertConf_;
    l2tpVpnConfig->ipsecPrivateServerCertConf_ = vpnBean->ipsecPrivateServerCertConf_;
    l2tpVpnConfig->ipsecPublicServerCertConf_ = vpnBean->ipsecPublicServerCertConf_;
    l2tpVpnConfig->ipsecCaCertFilePath_ = vpnBean->ipsecCaCertFilePath_;
    l2tpVpnConfig->ipsecPrivateUserCertFilePath_ = vpnBean->ipsecPrivateUserCertFilePath_;
    l2tpVpnConfig->ipsecPublicUserCertFilePath_ = vpnBean->ipsecPublicUserCertFilePath_;
    l2tpVpnConfig->ipsecPrivateServerCertFilePath_ = vpnBean->ipsecPrivateServerCertFilePath_;
    l2tpVpnConfig->ipsecPublicServerCertFilePath_ = vpnBean->ipsecPublicServerCertFilePath_;

    l2tpVpnConfig->ipsecConf_ = vpnBean->ipsecConf_;
    l2tpVpnConfig->ipsecSecrets_ = vpnBean->ipsecSecrets_;
    l2tpVpnConfig->optionsL2tpdClient_ = vpnBean->optionsL2tpdClient_;
    l2tpVpnConfig->xl2tpdConf_ = vpnBean->xl2tpdConf_;
    l2tpVpnConfig->l2tpSharedKey_ = vpnBean->l2tpSharedKey_;
    return l2tpVpnConfig;
}

sptr<VpnDataBean> VpnDataBean::ConvertSysVpnConfigToVpnBean(sptr<SysVpnConfig> &sysVpnConfig)
{
    if (sysVpnConfig == nullptr) {
        NETMGR_EXT_LOG_E("ConvertSysVpnConfigToVpnBean sysVpnConfig is null");
        return nullptr;
    }
    sptr<VpnDataBean> vpnBean = new (std::nothrow) VpnDataBean();
    if (vpnBean == nullptr) {
        NETMGR_EXT_LOG_E("ConvertSysVpnConfigToVpnBean vpnBean is null");
        return nullptr;
    }
    ConvertCommonVpnConfigToVpnBean(sysVpnConfig, vpnBean);
    sptr<IpsecVpnConfig> ipsecVpnConfig;
    sptr<L2tpVpnConfig> l2tpVpnConfig;
    switch (sysVpnConfig->vpnType_) {
        case VpnType::IKEV2_IPSEC_MSCHAPv2:
        case VpnType::IKEV2_IPSEC_PSK:
        case VpnType::IKEV2_IPSEC_RSA:
        case VpnType::IPSEC_XAUTH_PSK:
        case VpnType::IPSEC_XAUTH_RSA:
        case VpnType::IPSEC_HYBRID_RSA:
            ipsecVpnConfig = sptr<IpsecVpnConfig>(static_cast<IpsecVpnConfig *>(sysVpnConfig.GetRefPtr()));
            ConvertIpsecVpnConfigToVpnBean(ipsecVpnConfig, vpnBean);
            break;
        case VpnType::L2TP_IPSEC_PSK:
        case VpnType::L2TP_IPSEC_RSA:
            l2tpVpnConfig = sptr<L2tpVpnConfig>(static_cast<L2tpVpnConfig *>(sysVpnConfig.GetRefPtr()));
            ConvertL2tpVpnConfigToVpnBean(l2tpVpnConfig, vpnBean);
            break;
        default:
            NETMGR_EXT_LOG_E("ConvertSysVpnConfigToVpnBean proxy vpn type is error");
            break;
    }
    return vpnBean;
}

void VpnDataBean::ConvertCommonVpnConfigToVpnBean(sptr<SysVpnConfig> &sysVpnConfig, sptr<VpnDataBean> &vpnBean)
{
    if (vpnBean == nullptr || sysVpnConfig == nullptr) {
        NETMGR_EXT_LOG_E("ConvertCommonVpnConfigToVpnBean params is null");
        return;
    }
    vpnBean->vpnId_ = sysVpnConfig->vpnId_;
    vpnBean->vpnName_ = sysVpnConfig->vpnName_;
    vpnBean->vpnType_ = sysVpnConfig->vpnType_;
    std::vector<INetAddr> addresses = sysVpnConfig->addresses_;
    if (!addresses.empty()) {
        vpnBean->vpnAddress_ = addresses[0].address_;
    }
    vpnBean->userName_ = sysVpnConfig->userName_;
    vpnBean->password_ = sysVpnConfig->password_;
    vpnBean->userId_ = sysVpnConfig->userId_;
    vpnBean->isLegacy_ = sysVpnConfig->isLegacy_ ? 1 : 0;
    vpnBean->saveLogin_ = sysVpnConfig->saveLogin_ ? 1 : 0;
    vpnBean->forwardingRoutes_ = sysVpnConfig->forwardingRoutes_;
    std::vector<std::string> dnsAddresses = sysVpnConfig->dnsAddresses_;
    if (!dnsAddresses.empty()) {
        vpnBean->dnsAddresses_ = dnsAddresses[0];
    }
    std::vector<std::string> searchDomains = sysVpnConfig->searchDomains_;
    if (!searchDomains.empty()) {
        vpnBean->searchDomains_ = searchDomains[0];
    }
}
void VpnDataBean::ConvertIpsecVpnConfigToVpnBean(sptr<IpsecVpnConfig> &ipsecVpnConfig, sptr<VpnDataBean> &vpnBean)
{
    if (vpnBean == nullptr || ipsecVpnConfig == nullptr) {
        NETMGR_EXT_LOG_E("ConvertIpsecVpnConfigToVpnBean params is null");
        return;
    }
    vpnBean->ipsecPreSharedKey_ = ipsecVpnConfig->ipsecPreSharedKey_;
    vpnBean->ipsecIdentifier_ = ipsecVpnConfig->ipsecIdentifier_;
    vpnBean->swanctlConf_ = ipsecVpnConfig->swanctlConf_;
    vpnBean->strongswanConf_ = ipsecVpnConfig->strongswanConf_;
    vpnBean->ipsecCaCertConf_ = ipsecVpnConfig->ipsecCaCertConf_;
    vpnBean->ipsecPrivateUserCertConf_ = ipsecVpnConfig->ipsecPrivateUserCertConf_;
    vpnBean->ipsecPublicUserCertConf_ = ipsecVpnConfig->ipsecPublicUserCertConf_;
    vpnBean->ipsecPrivateServerCertConf_ = ipsecVpnConfig->ipsecPrivateServerCertConf_;
    vpnBean->ipsecPublicServerCertConf_ = ipsecVpnConfig->ipsecPublicServerCertConf_;
    vpnBean->ipsecCaCertFilePath_ = ipsecVpnConfig->ipsecCaCertFilePath_;
    vpnBean->ipsecPrivateUserCertFilePath_ = ipsecVpnConfig->ipsecPrivateUserCertFilePath_;
    vpnBean->ipsecPublicUserCertFilePath_ = ipsecVpnConfig->ipsecPublicUserCertFilePath_;
    vpnBean->ipsecPrivateServerCertFilePath_ = ipsecVpnConfig->ipsecPrivateServerCertFilePath_;
    vpnBean->ipsecPublicServerCertFilePath_ = ipsecVpnConfig->ipsecPublicServerCertFilePath_;
}

void VpnDataBean::ConvertL2tpVpnConfigToVpnBean(sptr<L2tpVpnConfig> &l2tpVpnConfig, sptr<VpnDataBean> &vpnBean)
{
    if (vpnBean == nullptr || l2tpVpnConfig == nullptr) {
        NETMGR_EXT_LOG_E("ConvertL2tpVpnConfigToVpnBean params is null");
        return;
    }
    vpnBean->ipsecPreSharedKey_ = l2tpVpnConfig->ipsecPreSharedKey_;
    vpnBean->ipsecIdentifier_ = l2tpVpnConfig->ipsecIdentifier_;
    vpnBean->strongswanConf_ = l2tpVpnConfig->strongswanConf_;
    vpnBean->ipsecCaCertConf_ = l2tpVpnConfig->ipsecCaCertConf_;
    vpnBean->ipsecPrivateUserCertConf_ = l2tpVpnConfig->ipsecPrivateUserCertConf_;
    vpnBean->ipsecPublicUserCertConf_ = l2tpVpnConfig->ipsecPublicUserCertConf_;
    vpnBean->ipsecPrivateServerCertConf_ = l2tpVpnConfig->ipsecPrivateServerCertConf_;
    vpnBean->ipsecPublicServerCertConf_ = l2tpVpnConfig->ipsecPublicServerCertConf_;
    vpnBean->ipsecCaCertFilePath_ = l2tpVpnConfig->ipsecCaCertFilePath_;
    vpnBean->ipsecPrivateUserCertFilePath_ = l2tpVpnConfig->ipsecPrivateUserCertFilePath_;
    vpnBean->ipsecPublicUserCertFilePath_ = l2tpVpnConfig->ipsecPublicUserCertFilePath_;
    vpnBean->ipsecPrivateServerCertFilePath_ = l2tpVpnConfig->ipsecPrivateServerCertFilePath_;
    vpnBean->ipsecPublicServerCertFilePath_ = l2tpVpnConfig->ipsecPublicServerCertFilePath_;

    vpnBean->ipsecConf_ = l2tpVpnConfig->ipsecConf_;
    vpnBean->ipsecSecrets_ = l2tpVpnConfig->ipsecSecrets_;
    vpnBean->optionsL2tpdClient_ = l2tpVpnConfig->optionsL2tpdClient_;
    vpnBean->xl2tpdConf_ = l2tpVpnConfig->xl2tpdConf_;
    vpnBean->l2tpSharedKey_ = l2tpVpnConfig->l2tpSharedKey_;
}
} // namespace NetManagerStandard
} // namespace OHOS
