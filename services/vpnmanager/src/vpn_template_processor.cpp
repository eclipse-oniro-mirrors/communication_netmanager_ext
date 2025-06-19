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
constexpr const char* KEY_VPN_ADDRESS = "vpn_address_value";
constexpr const char* KEY_VPN_USERNAME = "vpn_username_value";
constexpr const char* KEY_VPN_IPSEC_IDENTIFIER = "vpn_ipsec_identifier_value";
constexpr const char* KEY_VPN_PASSWORD = "vpn_password_value";
constexpr const char* KEY_VPN_IPSEC_SHAREDKEY = "vpn_ipsec_sharedKey_value";
constexpr const char* KEY_VPN_HOME_ELEMENT = "homeElement";
constexpr const char* KEY_VPN_HOME = "home";
constexpr const char* KEY_VPN_MYVPN_NAME = "lac l2tp";
constexpr const char* KEY_VPN_CLIENT_CONFIG_NAME = "options.l2tpd.client.conf";
constexpr const char* KEY_VPN_IF_ID_NUM = "if_id_num";
constexpr const char* KEY_VPN_L2TP_PSK_ID = "l2tp_psk_id";

constexpr const char* IKE2_IPSEC_MSCHAPV2_SWANCTL_CONNECTIONS_TEMPCONFIG = R"(
  homeElement {
    remote_addrs = vpn_address_value
    vips = 0.0.0.0
    local {
      auth = eap-mschapv2
      eap_id = vpn_username_value
    }
    remote {
      auth = pubkey
    }
    children {
      home {
        if_id_in=if_id_num
        if_id_out=if_id_num
        remote_ts=0.0.0.0/0
        esp_proposals = default
      }
    }
    version = 2
    proposals = default
  })";

constexpr const char* IKE2_IPSEC_PSK_SWANCTL_CONNECTIONS_TEMPCONFIG = R"(
  homeElement {
    remote_addrs = vpn_address_value
    vips = 0.0.0.0
    local {
      auth = psk
    }
    remote {
      auth = psk
      id = vpn_ipsec_identifier_value
    }
    children {
      home {
        if_id_in=if_id_num
        if_id_out=if_id_num
        remote_ts=0.0.0.0/0
        esp_proposals = default
      }
    }
  version = 2
  proposals = default
  })";

constexpr const char* IKE2_IPSEC_RSA_SWANCTL_CONNECTIONS_TEMPCONFIG = R"(
  homeElement {
    remote_addrs = vpn_address_value
    vips = 0.0.0.0
    local {
      auth = pubkey
      id = vpn_ipsec_identifier_value
    }
    remote {
      auth = pubkey
    }
    children {
      home {
        if_id_in=if_id_num
        if_id_out=if_id_num
        remote_ts=0.0.0.0/0
        esp_proposals = default
      }
    }
    version = 2
    proposals = default
  })";

constexpr const char* IPSEC_HYBRID_RSA_SWANCTL_CONNECTIONS_TEMPCONFIG = R"(
  homeElement {
    remote_addrs = vpn_address_value
    vips = 0.0.0.0
    local {
      auth = xauth
      xauth_id = vpn_username_value
    }
    remote {
      auth = pubkey
    }
    children {
      home {
        if_id_in=if_id_num
        if_id_out=if_id_num
        remote_ts=0.0.0.0/0
        esp_proposals = default
      }
    }
    version = 1
    proposals = default
  })";

constexpr const char* IPSEC_XAUTH_PSK_SWANCTL_CONNECTIONS_TEMPCONFIG = R"(
  homeElement {
    remote_addrs = vpn_address_value
    vips = 0.0.0.0
    local {
      id = vpn_ipsec_identifier_value
      auth = psk
    }
    local-xauth {
      auth = xauth
      xauth_id = vpn_username_value
    }
    remote {
      id = vpn_ipsec_identifier_value
      auth = psk
    }
    children {
      home {
        if_id_in=if_id_num
        if_id_out=if_id_num
        remote_ts=0.0.0.0/0
        esp_proposals = default
      }
    }
    version = 1
    proposals = default
    aggressive=yes
   })";

constexpr const char* L2TP_PSK_SWANCTL_CONNECTIONS_TEMPCONFIG = R"(
  homeElement {
    remote_addrs = vpn_address_value
    local {
      id = l2tp_psk_id
      auth = psk
    }
    remote {
      id = vpn_ipsec_identifier_value
      auth = psk
    }
    children {
    homel2tp {
      mode=transport
      local_ts = 0.0.0.0/0[udp/1701]
      remote_ts = vpn_address_value/32[udp/1701]
      esp_proposals = aes256-sha1, aes128-sha1, 3des-sha1
    }
  }
  version = 1
  proposals = 3des-sha1-modp1024, aes128-sha1-modp1024, aes256-sha1-modp1024
  })";

constexpr const char* L2TP_RSA_SWANCTL_CONNECTIONS_TEMPCONFIG = R"(
  homeElement {
  remote_addrs = vpn_address_value
  local {
    auth = psk
  }
  remote {
    auth = psk
  }
  children {
    homel2tp {
      mode=transport
      local_ts = 0.0.0.0/0[udp/1701]
      remote_ts = vpn_address_value/32[udp/1701]
      esp_proposals = aes256-sha1, aes128-sha1, 3des-sha1
    }
  }
  version = 1
  proposals = 3des-sha1-modp1024, aes128-sha1-modp1024, aes256-sha1-modp1024
  })";

constexpr const char* IPSEC_XAUTH_RSA_SWANCTL_CONNECTIONS_TEMPCONFIG = R"(
  homeElement {
  remote_addrs = vpn_address_value
  vips = 0.0.0.0
  local {
    auth = pubkey
    id = vpn_username_value
  }
  local-xauth {
    auth = xauth
  }
  remote {
    auth = pubkey
  }
  children {
    home {
      remote_ts=0.0.0.0/0
      esp_proposals = default
    }
  }
  version = 1
  proposals = default
  })";

constexpr const char* L2TP_IPSEC_XL2TP_TEMPCONFIG = R"([lac l2tp]
; set this to the ip address of your vpn server
lns = vpn_address_value
ppp debug = yes
pppoptfile = options.l2tpd.client.conf
length bit = yes
)";

constexpr const char* L2TP_IPSEC_OPTION_L2TP_TEMPCONFIG = R"(
ipcp-accept-local
ipcp-accept-remote
refuse-eap
require-mschap-v2
noccp
noauth
idle 1800
mtu 1410
mru 1410
defaultroute
usepeerdns
debug
connect-delay 5000
name vpn_username_value
password vpn_password_value)";

constexpr const char* L2TP_IPSEC_PSK_IPSEC_SECERETS_TEMPCONFIG = R"(
: PSK vpn_ipsec_sharedKey_value)";

constexpr const char* STRONGSWAN_CONF_TEMPCONFIG = R"(
# /etc/strongswan.conf - strongSwan configuration file
charon {
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
    if (sysVpnObj->ipsecVpnConfig_ != nullptr) {
        GenSwanctlOrIpsecConf(sysVpnObj->ipsecVpnConfig_, sysVpnObj->l2tpVpnConfig_, ifNameId, vpnObjMap);
        sysVpnObj->ipsecVpnConfig_->strongswanConf_ = STRONGSWAN_CONF_TEMPCONFIG;
    } else if (sysVpnObj->l2tpVpnConfig_ != nullptr) {
        GenOptionsL2tpdClient(sysVpnObj->l2tpVpnConfig_);
        GenXl2tpdConf(sysVpnObj->l2tpVpnConfig_, ifNameId, vpnObjMap);
        GenSwanctlOrIpsecConf(sysVpnObj->ipsecVpnConfig_, sysVpnObj->l2tpVpnConfig_, ifNameId, vpnObjMap);
        GenIpsecSecrets(sysVpnObj->l2tpVpnConfig_);
        sysVpnObj->l2tpVpnConfig_->strongswanConf_ = STRONGSWAN_CONF_TEMPCONFIG;
    }
    return NETMANAGER_EXT_SUCCESS;
}

void VpnTemplateProcessor::GenSwanctlOrIpsecConf(sptr<IpsecVpnConfig> &ipsecConfig, sptr<L2tpVpnConfig> &l2tpConfig,
    int32_t ifNameId, std::map<std::string, std::shared_ptr<NetVpnImpl>> &vpnObjMap)
{
    std::string connects;
    std::string secrets;
    for (const auto& pair : vpnObjMap) {
        if (pair.second != nullptr) {
            std::shared_ptr<IpsecVpnCtl> vpnObj = std::static_pointer_cast<IpsecVpnCtl>(pair.second);
            if (vpnObj != nullptr && vpnObj->multiVpnInfo_ != nullptr) {
                CreateConnectAndSecret(vpnObj->ipsecVpnConfig_, vpnObj->l2tpVpnConfig_,
                    vpnObj->multiVpnInfo_->ifNameId, connects, secrets);
            }
        }
    }
    CreateConnectAndSecret(ipsecConfig, l2tpConfig, ifNameId, connects, secrets);
    std::string conf = "connections {\n" + connects + "\n}" + "\nsecrets {\n" + secrets + "\n}";
    if (ipsecConfig != nullptr) {
        ipsecConfig->swanctlConf_ = conf;
    } else if (l2tpConfig != nullptr) {
        l2tpConfig->ipsecConf_ =  conf;
    } else {
        NETMGR_EXT_LOG_W("invalid config");
    }
}

void VpnTemplateProcessor::GenXl2tpdConf(sptr<L2tpVpnConfig> &config, int32_t ifNameId,
                                         std::map<std::string, std::shared_ptr<NetVpnImpl>> &vpnObjMap)
{
    std::string conf;
    for (const auto& pair : vpnObjMap) {
        if (pair.second != nullptr) {
            std::shared_ptr<IpsecVpnCtl> vpnObj = std::static_pointer_cast<IpsecVpnCtl>(pair.second);
            if (vpnObj != nullptr && vpnObj->multiVpnInfo_ != nullptr) {
                CreateXl2tpdConf(vpnObj->l2tpVpnConfig_, vpnObj->multiVpnInfo_->ifNameId, conf);
            }
        }
    }
    CreateXl2tpdConf(config, ifNameId, conf);
    config->xl2tpdConf_ = conf;
}

void VpnTemplateProcessor::GenOptionsL2tpdClient(sptr<L2tpVpnConfig> &config)
{
    int32_t configType = config->vpnType_;
    std::string conf;
    if (configType == VpnType::L2TP_IPSEC_PSK || configType == VpnType::L2TP_IPSEC_RSA
        || configType == VpnType::L2TP) {
        conf.append(L2TP_IPSEC_OPTION_L2TP_TEMPCONFIG);
    }
    std::unordered_map<std::string, std::string> params;
    params[KEY_VPN_USERNAME] = config->userName_;
    params[KEY_VPN_PASSWORD] = config->password_;
    InflateConf(conf, params);
    config->optionsL2tpdClient_ = conf;
}

void VpnTemplateProcessor::GenIpsecSecrets(sptr<L2tpVpnConfig> &config)
{
    int32_t configType = config->vpnType_;
    std::string conf;
    if (configType == VpnType::L2TP_IPSEC_PSK) {
        conf.append(L2TP_IPSEC_PSK_IPSEC_SECERETS_TEMPCONFIG);
    }
    std::unordered_map<std::string, std::string> params;
    params[KEY_VPN_IPSEC_SHAREDKEY] = config->ipsecPreSharedKey_;
    InflateConf(conf, params);
    config->ipsecSecrets_ = conf;
}

void VpnTemplateProcessor::InflateConf(std::string &conf,
    const std::unordered_map<std::string, std::string>& params)
{
    for (const auto& [key, value] : params) {
        if (value.empty()) {
            continue;
        }
        size_t pos = 0;
        while ((pos = conf.find(key, pos)) != std::string::npos) {
            conf.replace(pos, key.length(), value);
            pos += value.length();
        }
    }
}

void VpnTemplateProcessor::CreateXl2tpdConf(sptr<L2tpVpnConfig> &config, int32_t ifNameId, std::string &outConf)
{
    if (config == nullptr) {
        return;
    }
    std::string conf = L2TP_IPSEC_XL2TP_TEMPCONFIG;
    std::unordered_map<std::string, std::string> params;
    params[KEY_VPN_ADDRESS] = config->addresses_[0].address_;
    params[KEY_VPN_MYVPN_NAME] = KEY_VPN_MYVPN_NAME + std::to_string(ifNameId);
    params[KEY_VPN_CLIENT_CONFIG_NAME] = std::string(KEY_VPN_CLIENT_CONFIG_NAME) + "-" + std::to_string(ifNameId);
    InflateConf(conf, params);
    outConf += conf;
}

void VpnTemplateProcessor::GetConnectAndSecretTemp(int32_t type, std::string &outConnect, std::string &outSecret)
{
    const std::string secretsUsername = "\nid = vpn_username_value\nsecret = vpn_password_value\n";
    const std::string secretsId = "\nid = vpn_ipsec_identifier_value\nsecret = vpn_ipsec_sharedKey_value\n";
    const std::string l2tpIpsecSecret =
        "\nid-1 = l2tp_psk_id\nid-2 = vpn_ipsec_identifier_value\nsecret = vpn_ipsec_sharedKey_value\n";
    switch (type) {
        case VpnType::IKEV2_IPSEC_MSCHAPv2:
            outConnect = IKE2_IPSEC_MSCHAPV2_SWANCTL_CONNECTIONS_TEMPCONFIG;
            outSecret = "eap-homeElement {" + secretsUsername + "}\n";
            break;
        case VpnType::IKEV2_IPSEC_PSK:
            outConnect = IKE2_IPSEC_PSK_SWANCTL_CONNECTIONS_TEMPCONFIG;
            outSecret = "ike-homeElement {" + secretsId + "}\n";
            break;
        case VpnType::IKEV2_IPSEC_RSA:
            outConnect = IKE2_IPSEC_RSA_SWANCTL_CONNECTIONS_TEMPCONFIG;
            break;
        case VpnType::IPSEC_XAUTH_PSK:
            outConnect = IPSEC_XAUTH_PSK_SWANCTL_CONNECTIONS_TEMPCONFIG;
            outSecret = "ike-homeElement {" + secretsId + "}\n"
                + "xauth-homeElement {" + secretsUsername + "}\n";
            break;
        case VpnType::IPSEC_XAUTH_RSA:
            outConnect = IPSEC_XAUTH_RSA_SWANCTL_CONNECTIONS_TEMPCONFIG;
            outSecret = "xauth-homeElement {" + secretsUsername + "}\n";
            break;
        case VpnType::IPSEC_HYBRID_RSA:
            outConnect = IPSEC_HYBRID_RSA_SWANCTL_CONNECTIONS_TEMPCONFIG;
            outSecret = "xauth-homeElement {" + secretsUsername + "}\n";
            break;
        case VpnType::L2TP_IPSEC_PSK:
            outConnect = L2TP_PSK_SWANCTL_CONNECTIONS_TEMPCONFIG;
            outSecret = "ike-homeElement {" + l2tpIpsecSecret + "}\n";
            break;
        case VpnType::L2TP_IPSEC_RSA:
            outConnect = L2TP_RSA_SWANCTL_CONNECTIONS_TEMPCONFIG;
            break;
        default:
            break;
    }
}

void VpnTemplateProcessor::CreateConnectAndSecret(sptr<IpsecVpnConfig> &ipsecConfig, sptr<L2tpVpnConfig> &l2tpConfig,
    int32_t ifNameId, std::string &outConnect, std::string &outSecret)
{
    std::string connect;
    std::string secret;
    std::string emptyId = "%any";
    std::unordered_map<std::string, std::string> params;
    if (l2tpConfig != nullptr) {
        if (l2tpConfig->vpnType_ == L2TP) {
            return;
        }
        GetConnectAndSecretTemp(l2tpConfig->vpnType_, connect, secret);

        params[KEY_VPN_L2TP_PSK_ID] =
            l2tpConfig->ipsecIdentifier_.empty() ? KEY_VPN_HOME_ELEMENT : l2tpConfig->ipsecIdentifier_;
        InflateConf(connect, params);
        InflateConf(secret, params);
        params[KEY_VPN_ADDRESS] = l2tpConfig->addresses_[0].address_;
        params[KEY_VPN_HOME_ELEMENT] = KEY_VPN_HOME + std::to_string(ifNameId);
        params[KEY_VPN_IPSEC_IDENTIFIER] = l2tpConfig->ipsecIdentifier_.empty()
            ? emptyId : l2tpConfig->ipsecIdentifier_;
        params[KEY_VPN_IPSEC_SHAREDKEY] = l2tpConfig->ipsecPreSharedKey_;
        InflateConf(connect, params);
        InflateConf(secret, params);
    } else if (ipsecConfig != nullptr) {
        GetConnectAndSecretTemp(ipsecConfig->vpnType_, connect, secret);

        params[KEY_VPN_ADDRESS] = ipsecConfig->addresses_[0].address_;
        params[KEY_VPN_IPSEC_IDENTIFIER] = ipsecConfig->ipsecIdentifier_.empty()
            ? emptyId : ipsecConfig->ipsecIdentifier_;
        params[KEY_VPN_IPSEC_SHAREDKEY] = ipsecConfig->ipsecPreSharedKey_;
        params[KEY_VPN_USERNAME] = ipsecConfig->userName_;
        params[KEY_VPN_PASSWORD] = ipsecConfig->password_;
        params[KEY_VPN_HOME_ELEMENT] = KEY_VPN_HOME + std::to_string(ifNameId);
        params[KEY_VPN_IF_ID_NUM] = std::to_string(ifNameId);
        InflateConf(connect, params);
        InflateConf(secret, params);
    } else {
        connect = "";
        secret = "";
    }
    outConnect += connect;
    outSecret += secret;
}
} // namespace NetManagerStandard
} // namespace OHOS
