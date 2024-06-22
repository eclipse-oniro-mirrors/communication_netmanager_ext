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

#ifndef NET_VPN_DATA_BEAN_H
#define NET_VPN_DATA_BEAN_H

#include <string>
#include <vector>

#include "refbase.h"
#include "inet_addr.h"
#include "vpn_config.h"
#include "openvpn_config.h"
#include "ipsecvpn_config.h"
#include "l2tpvpn_config.h"

namespace OHOS {
namespace NetManagerStandard {
struct VpnDataBean : public virtual RefBase {
    //common
    std::string vpnId_;
    std::string vpnName_;
    int32_t vpnType_ = -1;
    std::string vpnAddress_;
    std::string userName_;
    std::string password_;
    int32_t userId_ = 0;
    int32_t isLegacy_ = 1;
    int32_t saveLogin_ = 0;
    std::string forwardingRoutes_;
    std::string dnsAddresses_;
    std::string searchDomains_;

    //openvpn
    std::string ovpnPort_;
    int32_t ovpnProtocol_;
    std::string ovpnConfig_;
    int32_t ovpnAuthType_;
    std::string askpass_;
    std::string ovpnConfigFilePath_;
    std::string ovpnCaCertFilePath_;
    std::string ovpnUserCertFilePath_;
    std::string ovpnPrivateKeyFilePath_;

    //ipsec
    std::string ipsecPreSharedKey_;
    std::string ipsecIdentifier_;
    std::string swanctlConf_;
    std::string strongswanConf_;
    std::string ipsecCaCertConf_;
    std::string ipsecPrivateUserCertConf_;
    std::string ipsecPublicUserCertConf_;
    std::string ipsecPrivateServerCertConf_;
    std::string ipsecPublicServerCertConf_;
    std::string ipsecCaCertFilePath_;
    std::string ipsecPrivateUserCertFilePath_;
    std::string ipsecPublicUserCertFilePath_;
    std::string ipsecPrivateServerCertFilePath_;
    std::string ipsecPublicServerCertFilePath_;

    //l2tp
    std::string ipsecConf_;
    std::string ipsecSecrets_;
    std::string optionsL2tpdClient_;
    std::string xl2tpdConf_;
    std::string l2tpSharedKey_;

    static sptr<SysVpnConfig> ConvertVpnBeanToSysVpnConfig(sptr<VpnDataBean> &vpnBean);
    static sptr<OpenVpnConfig> ConvertVpnBeanToOpenVpnConfig(sptr<VpnDataBean> &vpnBean);
    static sptr<IpsecVpnConfig> ConvertVpnBeanToIpsecVpnConfig(sptr<VpnDataBean> &vpnBean);
    static sptr<L2tpVpnConfig> ConvertVpnBeanToL2tpVpnConfig(sptr<VpnDataBean> &vpnBean);
    static sptr<VpnDataBean> ConvertSysVpnConfigToVpnBean(sptr<SysVpnConfig> &sysVpnConfig);
    static void ConvertCommonVpnConfigToVpnBean(sptr<SysVpnConfig> &sysVpnConfig, sptr<VpnDataBean> &vpnBean);
    static void ConvertOpenVpnConfigToVpnBean(sptr<OpenVpnConfig> &openVpnConfig, sptr<VpnDataBean> &vpnBean);
    static void ConvertIpsecVpnConfigToVpnBean(sptr<IpsecVpnConfig> &ipsecVpnConfig, sptr<VpnDataBean> &vpnBean);
    static void ConvertL2tpVpnConfigToVpnBean(sptr<L2tpVpnConfig> &l2tpVpnConfig, sptr<VpnDataBean> &vpnBean);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_VPN_DATA_BEAN_H