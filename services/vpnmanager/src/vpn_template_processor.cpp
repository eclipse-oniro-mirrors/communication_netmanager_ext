/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "vpn_template_processor.h"
#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "ipsec_vpn_ctl.h"
#include "netmanager_base_common_utils.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr const char *MULTI_TUN_CARD_NAME = "multitun-vpn";
constexpr const char* STRONGSWAN_CONF_TEMPCONFIG = R"(charon {
  i_dont_care_about_security_and_use_aggressive_mode_psk = yes
  install_virtual_ip = no
  plugins {
    include /system/etc/strongswan/strongswan.d/charon/*.conf
    kernel-libipsec {
      load = no
    }
    kernel-netlink {
      load = yes
    }
  }
}
include /system/etc/strongswan/strongswan.d/*.conf)";

int32_t VpnTemplateProcessor::BuildConfig(std::shared_ptr<NetVpnImpl> &vpnObj,
                                          std::map<std::string, std::shared_ptr<NetVpnImpl>> &vpnObjMap)
{
    if (vpnObj == nullptr) {
        NETMGR_EXT_LOG_E("invalid vpnObj");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    std::shared_ptr<IpsecVpnCtl> sysVpnObj = std::static_pointer_cast<IpsecVpnCtl>(vpnObj);
    if (sysVpnObj == nullptr || sysVpnObj->multiVpnInfo_ == nullptr) {
        NETMGR_EXT_LOG_E("invalid sysVpnObj");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    int32_t ifNameId = sysVpnObj->multiVpnInfo_->ifNameId;
    GenSwanctlOrIpsecConf(sysVpnObj->ipsecVpnConfig_, sysVpnObj->l2tpVpnConfig_, ifNameId, vpnObjMap);
    if (sysVpnObj->ipsecVpnConfig_ != nullptr) {
        sysVpnObj->ipsecVpnConfig_->strongswanConf_ = STRONGSWAN_CONF_TEMPCONFIG;
    } else if (sysVpnObj->l2tpVpnConfig_ != nullptr) {
        GenOptionsL2tpdClient(sysVpnObj->l2tpVpnConfig_);
        GenXl2tpdConf(sysVpnObj->l2tpVpnConfig_, ifNameId, vpnObjMap);
        GenIpsecSecrets(sysVpnObj->l2tpVpnConfig_);
        sysVpnObj->l2tpVpnConfig_->strongswanConf_ = STRONGSWAN_CONF_TEMPCONFIG;
    } else {
        NETMGR_EXT_LOG_W("invalid config");
    }
    return NETMANAGER_EXT_SUCCESS;
}

void VpnTemplateProcessor::GenSwanctlOrIpsecConf(sptr<IpsecVpnConfig> &ipsecConfig, sptr<L2tpVpnConfig> &l2tpConfig,
    int32_t ifNameId, std::map<std::string, std::shared_ptr<NetVpnImpl>> &vpnObjMap)
{
    if (ipsecConfig == nullptr && l2tpConfig == nullptr) {
        NETMGR_EXT_LOG_W("invalid config");
        return;
    }
    std::string connects;
    std::string secrets;
    for (const auto& pair : vpnObjMap) {
        if (pair.second == nullptr || pair.second->multiVpnInfo_ == nullptr) {
            continue;
        }
        if (strstr(pair.second->multiVpnInfo_->ifName.c_str(), MULTI_TUN_CARD_NAME) != NULL) {
            continue;
        }
        std::shared_ptr<IpsecVpnCtl> vpnObj = std::static_pointer_cast<IpsecVpnCtl>(pair.second);
        if (vpnObj != nullptr && vpnObj->multiVpnInfo_ != nullptr) {
            CreateConnectAndSecret(vpnObj->ipsecVpnConfig_, vpnObj->l2tpVpnConfig_,
                vpnObj->multiVpnInfo_->ifNameId, connects, secrets);
        }
    }
    CreateConnectAndSecret(ipsecConfig, l2tpConfig, ifNameId, connects, secrets);
    std::string conf = "connections {\n" + connects + "\n}\nsecrets {\n" + secrets + "\n}";
    if (ipsecConfig != nullptr) {
        ipsecConfig->swanctlConf_ = conf;
    }
    if (l2tpConfig != nullptr) {
        l2tpConfig->ipsecConf_ =  conf;
    }
}

void VpnTemplateProcessor::GenXl2tpdConf(sptr<L2tpVpnConfig> &config, int32_t ifNameId,
                                         std::map<std::string, std::shared_ptr<NetVpnImpl>> &vpnObjMap)
{
    if (config == nullptr) {
        NETMGR_EXT_LOG_W("invalid config");
        return;
    }
    std::string conf;
    for (const auto& pair : vpnObjMap) {
        if (pair.second == nullptr || pair.second->multiVpnInfo_ == nullptr) {
            continue;
        }
        if (strstr(pair.second->multiVpnInfo_->ifName.c_str(), MULTI_TUN_CARD_NAME) != NULL) {
            continue;
        }
        std::shared_ptr<IpsecVpnCtl> vpnObj = std::static_pointer_cast<IpsecVpnCtl>(pair.second);
        if (vpnObj != nullptr && vpnObj->multiVpnInfo_ != nullptr) {
            CreateXl2tpdConf(vpnObj->l2tpVpnConfig_, vpnObj->multiVpnInfo_->ifNameId, conf);
        }
    }
    CreateXl2tpdConf(config, ifNameId, conf);
    config->xl2tpdConf_ = conf;
}

void VpnTemplateProcessor::GenOptionsL2tpdClient(sptr<L2tpVpnConfig> &config)
{
    if (config == nullptr) {
        NETMGR_EXT_LOG_W("invalid config");
        return;
    }
    int32_t configType = config->vpnType_;
    if (configType == VpnType::L2TP_IPSEC_PSK || configType == VpnType::L2TP_IPSEC_RSA
            || configType == VpnType::L2TP) {
        std::ostringstream oss;
        oss << "ipcp-accept-local\nipcp-accept-remote\nrefuse-eap\nrequire-mschap-v2\n";
        oss << "noccp\nnoauth\nidle 1800\nmtu 1410\nmru 1410\n";
        oss << "defaultroute\nusepeerdns\ndebug\nconnect-delay 5000\n";
        oss << "name " << config->userName_ << " \npassword " << config->password_;
        config->optionsL2tpdClient_ = oss.str();
    }
}

void VpnTemplateProcessor::GenIpsecSecrets(sptr<L2tpVpnConfig> &config)
{
    if (config != nullptr && config->vpnType_ == VpnType::L2TP_IPSEC_PSK) {
        config->ipsecSecrets_ = ": PSK " + config->ipsecPreSharedKey_;
    }
}

void VpnTemplateProcessor::CreateXl2tpdConf(sptr<L2tpVpnConfig> &config, int32_t ifNameId, std::string &outConf)
{
    if (config == nullptr || config->addresses_.empty()) {
        NETMGR_EXT_LOG_W("invalid config");
        return;
    }
    std::ostringstream oss;
    oss << "[lac l2tp" << ifNameId << "]" << std::endl;
    oss << "lns = " << config->addresses_[0].address_ << std::endl;
    oss << "ppp debug = yes" << std::endl;
    oss << "pppoptfile = options.l2tpd.client.conf-" << ifNameId << std::endl;
    oss << "length bit = yes" << std::endl;
    outConf.append(oss.str());
}

void VpnTemplateProcessor::GetSecret(sptr<IpsecVpnConfig> &ipsecConfig, int32_t ifNameId, std::string &outSecret)
{
    if (ipsecConfig == nullptr) {
        NETMGR_EXT_LOG_W("invalid config");
        return;
    }
    std::string homeElement = "home" + std::to_string(ifNameId);
    std::string ipsecId =  ipsecConfig->ipsecIdentifier_.empty() ? "%any" : ipsecConfig->ipsecIdentifier_;
    std::ostringstream oss;
    switch (ipsecConfig->vpnType_) {
        case VpnType::IKEV2_IPSEC_MSCHAPv2: {
            oss << "eap-" << homeElement << " {\nid = " << ipsecConfig->userName_;
            oss << "\nsecret = " << ipsecConfig->password_ << "\n}\n";
            break;
        }
        case VpnType::IKEV2_IPSEC_PSK: {
            oss << "ike-" << homeElement << " {\nid = " << ipsecId;
            oss << "\nsecret = " << ipsecConfig->ipsecPreSharedKey_ << "\n}\n";
            break;
        }
        case VpnType::IPSEC_XAUTH_PSK: {
            oss << "ike-" << homeElement << " {\nid = " << ipsecId;
            oss << "\nsecret = " << ipsecConfig->ipsecPreSharedKey_ << "\n}\n";
            oss << "xauth-" << homeElement << " {\nid = " << ipsecConfig->userName_;
            oss << "\nsecret = " << ipsecConfig->password_ << "\n}\n";
            break;
        }
        case VpnType::IPSEC_XAUTH_RSA:
        case VpnType::IPSEC_HYBRID_RSA: {
            oss << "xauth-" << homeElement << " {id = " << ipsecConfig->userName_;
            oss << "\nsecret = " << ipsecConfig->password_ << "\n}\n";
            break;
        }
        default:
            break;
    }
    outSecret.append(oss.str());
}

void VpnTemplateProcessor::GetConnect(sptr<IpsecVpnConfig> &ipsecConfig, int32_t ifNameId, std::string &outConnect)
{
    if (ipsecConfig == nullptr || ipsecConfig->addresses_.empty()) {
        NETMGR_EXT_LOG_W("invalid config");
        return;
    }
    std::string homeElement = "home" + std::to_string(ifNameId);
    std::string ipsecId = ipsecConfig->ipsecIdentifier_.empty() ? "%any" : ipsecConfig->ipsecIdentifier_;
    std::string children = "children {\n home {\n if_id_in=" + std::to_string(ifNameId) + "\n if_id_out="
        + std::to_string(ifNameId) + "\nremote_ts=0.0.0.0/0\n esp_proposals = default\n}\n}";
    outConnect = homeElement + " {\n remote_addrs = " + ipsecConfig->addresses_[0].address_ + "\n vips = 0.0.0.0\n";
    std::ostringstream oss;
    switch (ipsecConfig->vpnType_) {
        case VpnType::IKEV2_IPSEC_MSCHAPv2:
            oss << "local {\n auth = eap-mschapv2\n eap_id = " << ipsecConfig->userName_ << "\n}\n";
            oss << "remote {\n auth = pubkey\n}\n" << children << "\nversion = 2\n proposals = default\n}\n";
            break;
        case VpnType::IKEV2_IPSEC_PSK:
            oss << "local {\n auth = psk\n}\n remote {\n auth = psk\n id = " << ipsecId << "\n}\n";
            oss << children << "\nversion = 2\n proposals = default\n}\n";
            break;
        case VpnType::IKEV2_IPSEC_RSA:
            oss << "local {\n auth = pubkey\n id = " << ipsecId << "\n}\n";
            oss << "remote {\n auth = pubkey\n}\n" << children << "\nversion = 2\n proposals = default\n}\n";
            break;
        case VpnType::IPSEC_XAUTH_PSK:
            oss << "local {\n auth = psk\n id = " << ipsecId << "\n}\n";
            oss << "local-xauth {\n auth = xauth\n xauth_id = " << ipsecConfig->userName_ << "\n}\n";
            oss << "remote {\n auth = psk\n id = " << ipsecId << "\n}\n";
            oss << children << "\n version = 1\n proposals = default\n aggressive=yes\n}\n";
            break;
        case VpnType::IPSEC_XAUTH_RSA:
            oss << "local {\n auth = pubkey\n id = " << ipsecId << "\n}\n";
            oss << "local-xauth {\n auth = xauth\n}\n remote {\n auth = pubkey\n}\n";
            oss << "children {\n home {\n remote_ts=0.0.0.0/0\n esp_proposals = default\n}\n}\n";
            oss << "version = 1\n proposals = default\n}\n";
            break;
        case VpnType::IPSEC_HYBRID_RSA:
            oss << "local {\n auth = xauth\n xauth_id = " << ipsecConfig->userName_ << "\n}\n";
            oss << "remote {\n auth = pubkey\n}\n";
            oss << children << "{\n version = 1\n proposals = default\n}\n";
            break;
        default:
            break;
    }
    outConnect.append(oss.str());
}

void VpnTemplateProcessor::CreateConnectAndSecret(sptr<IpsecVpnConfig> &ipsecConfig, sptr<L2tpVpnConfig> &l2tpConfig,
    int32_t ifNameId, std::string &outConnect, std::string &outSecret)
{
    if (ipsecConfig == nullptr && l2tpConfig == nullptr) {
        NETMGR_EXT_LOG_W("invalid config");
        return;
    }
    std::string connect;
    std::string secret;
    if (l2tpConfig != nullptr) {
        if (l2tpConfig->vpnType_ == L2TP) {
            return;
        }
        std::string homeElement = "home" + std::to_string(ifNameId);
        std::string address = !l2tpConfig->addresses_.empty() ? l2tpConfig->addresses_[0].address_ : "";
        std::string localId = !l2tpConfig->ipsecIdentifier_.empty() ? l2tpConfig->ipsecIdentifier_ :
            (l2tpConfig->vpnType_ == L2TP_IPSEC_PSK) ? homeElement : "%any";
        std::string remoteId = l2tpConfig->ipsecIdentifier_.empty() ? "%any" : l2tpConfig->ipsecIdentifier_;
        std::string authType = (l2tpConfig->vpnType_ == L2TP_IPSEC_PSK) ? "psk" : "pubkey";

        std::ostringstream connectOss;
        std::ostringstream secretOss;
        connectOss << "home" << ifNameId << " {\nremote_addrs = " << address << "\n";
        connectOss << "local {\nid = " << localId << "\nauth = " << authType << "\n}\n";
        connectOss << "remote {\nid = " << remoteId << "\nauth = " << authType << "\n}\n";
        connectOss << "children {\nhomel2tp {\nmode=transport\nlocal_ts = 0.0.0.0/0[udp/1701]\n";
        connectOss << "remote_ts = " << address << "/32[udp/1701]\n";
        connectOss << "esp_proposals = aes256-sha1, aes128-sha1, 3des-sha1\n}\n}\nversion = 1\n";
        connectOss << "proposals = 3des-sha1-modp1024, aes128-sha1-modp1024, aes256-sha1-modp1024\n}\n";

        std::string l2tpId = l2tpConfig->ipsecIdentifier_.empty() ? homeElement : l2tpConfig->ipsecIdentifier_;
        secretOss << "ike-" << homeElement << " {\nid-1 = " << l2tpId;
        secretOss << "\nid-2 = " << l2tpId << "\nsecret = " << l2tpConfig->ipsecPreSharedKey_ << "\n}\n";

        connect = connectOss.str();
        secret = secretOss.str();
    }
    if (ipsecConfig != nullptr) {
        GetConnect(ipsecConfig, ifNameId, connect);
        GetSecret(ipsecConfig, ifNameId, secret);
    }
    outConnect.append(connect);
    outSecret.append(secret);
}
} // namespace NetManagerStandard
} // namespace OHOS
