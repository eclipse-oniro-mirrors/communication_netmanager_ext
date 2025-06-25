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

#include "base64_utils.h"
#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr const char* KEY_VPN_ADDRESS = "vpn_address_value";
constexpr const char* KEY_VPN_USERNAME = "vpn_username_value";
constexpr const char* KEY_VPN_IPSEC_IDENTIFIER = "vpn_ipsec_identifier_value";
constexpr const char* KEY_VPN_PASSWORD = "vpn_password_value";
constexpr const char* KEY_VPN_IPSEC_SHAREDKEY = "vpn_ipsec_sharedKey_value";
constexpr const char* IKE2_IPSEC_MSCHAPV2_SWANCTL_TEMPCONFIG = R"(
connections {
    home {
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
                remote_ts=0.0.0.0/0
                esp_proposals = aes128gcm128-x25519
            }
        }
        version = 2
        proposals = aes128-sha256-x25519
    }
}
secrets {
    eap-carol {
        id = ipsec_identifier_value
        secret = password_value
    }
    eap-dave {
        id = vpn_username_value
        secret = vpn_password_value
    }
})";

constexpr const char* IKE2_IPSEC_PSK_SWANCTL_TEMPCONFIG = R"(
connections {
    home {
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
                remote_ts=0.0.0.0/0
                esp_proposals = aes128gcm128-x25519
            }
        }
    version = 2
    proposals = aes128-sha256-x25519
    }
}
secrets {
    ike-moon {
        id = vpn_ipsec_identifier_value
        secret = vpn_ipsec_sharedKey_value
    }
})";

constexpr const char* IKE2_IPSEC_RSA_SWANCTL_TEMPCONFIG = R"(
connections {
    home {
        remote_addrs = vpn_address_value
        vips = 0.0.0.0
        local {
            auth = pubkey
            certs = /data/service/el1/public/vpn/client.cert.pem
            id = vpn_ipsec_identifier_value
        }
        remote {
            auth = pubkey
        }
        children {
            home {
                remote_ts=0.0.0.0/0
                esp_proposals = aes128gcm128-x25519
            }
        }
        version = 2
        proposals = aes128-sha256-x25519
    }
})";

constexpr const char* IPSEC_HYBRID_RSA_SWANCTL_TEMPCONFIG = R"(
connections {
    home {
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
                remote_ts=0.0.0.0/0
                esp_proposals = aes256-sha2_384
            }
        }
        version = 1
        proposals = aes256-sha2_384-modp1024
    }
}
secrets {
    xauth {
        id = vpn_username_value
        secret = vpn_password_value
    }
})";

constexpr const char* IPSEC_XAUTH_PSK_SWANCTL_TEMPCONFIG = R"(
connections {
   home {
      remote_addrs = vpn_address_value
      vips = 0.0.0.0
      local {
         auth = psk
      }
      local-xauth {
         auth = xauth
         xauth_id = vpn_username_value
      }
      remote {
         auth = psk
      }
      children {
         home {
            remote_ts=0.0.0.0/0
            esp_proposals = aes256-sha2_384
         }
      }
      version = 1
      proposals = aes256-sha2_384-modp1024
      aggressive=yes
   }
}
secrets {
   ike-moon {
      secret = vpn_ipsec_sharedKey_value
   }
   xauth{
      id = vpn_username_value
      secret = vpn_password_value
   }
})";

constexpr const char* IPSEC_XAUTH_RSA_SWANCTL_TEMPCONFIG = R"(
connections {
   home {
      remote_addrs = vpn_address_value
      vips = 0.0.0.0
      local {
        auth = pubkey
        certs = /data/service/el1/public/vpn/client.cert.pem
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
        esp_proposals = aes256-sha2_384
        }
      }
      version = 1
      proposals = aes256-sha2_384-modp1024
   }
}
secrets {
   xauth-carol {
      id = vpn_username_value
      secret = vpn_password_value
   }
})";

constexpr const char* L2TP_IPSEC_XL2TP_TEMPCONFIG = R"(
[lac myVPN]
; set this to the ip address of your vpn server
lns = vpn_address_value
ppp debug = yes
pppoptfile = /data/service/el1/public/vpn/options.l2tpd.client.conf
length bit = yes
)";

constexpr const char* L2TP_IPSEC_OPTION_L2TP_TEMPCONFIG = R"(
ipcp-accept-local
ipcp-accept-remote
refuse-eap
require-mschap-v2
noccp
noauth
logfile /data/service/el1/public/vpn/xl2tpd.log
idle 1800
mtu 1410
mru 1410
defaultroute
usepeerdns
debug
connect-delay 5000
name vpn_username_value
password vpn_password_value
)";

constexpr const char* L2TP_IPSEC_RSA_IPSEC_TEMPCONFIG = R"(
config setup
    uniqueids=no
    charondebug="ike 4, knl 3, cfg 4"
conn %default
    dpdaction=clear
    dpddelay=300s
    rekey=no
    left=%defaultroute
    leftfirewall=yes
    ikelifetime=60m
    keylife=20m
    rekeymargin=3m
    keyingtries=1
    auto=add
conn home
    type=transport
    ike=aes256-sha2_384-modp1024
    esp = aes256-sha2_384
    keyexchange=ikev1
    authby=pubkey
    leftcert=/data/service/el1/public/vpn/client.cert.pem
    leftid=192.168.1.11
    leftprotoport=udp/l2tp
    keyingtries=1
    right=vpn_address_value
)";

constexpr const char* L2TP_IPSEC_PSK_IPSEC_TEMPCONFIG = R"(
config setup
conn %default
    ikelifetime=60m
    keylife=20m
    rekeymargin=3m
    keyingtries=1
    keyexchange=ikev1
    authby=secret
    ike=aes128-sha1-modp1024, 3des-sha1-modp1024!
    esp=aes128-sha1-modp1024, 3des-sha1-modp1024!
conn home
    keyexchange=ikev1
    left=%defaultroute
    auto=add
    authby=secret
    type=transport
    leftprotoport=17/1701
    rightprotoport=17/1701
    right=vpn_address_value
    rightid=%any
)";

constexpr const char* L2TP_IPSEC_RSA_IPSEC_SECERETS_TEMPCONFIG = R"(
: RSA /data/service/el1/public/vpn/client.key.pem)";

constexpr const char* L2TP_IPSEC_PSK_IPSEC_SECERETS_TEMPCONFIG = R"(
: PSK vpn_ipsec_sharedKey_value)";

constexpr const char* SWANCTL_STRONGSWAN_TEMPCONFIG = R"(
swanctl {
    load = pem pkcs1 x509 revocation constraints pubkey openssl random
})";

constexpr const char* SWANCTL_PSK_STRONGSWAN_TEMPCONFIG = R"(
swanctl {
    load = random openssl
}
charon-systemd {
    load = random nonce aes sha1 sha2 hmac kdf curve25519 kernel-netlink socket-default updown vici kernel-libipsec
})";

constexpr const char* CHARON_STRONGSWAN_TEMPCONFIG_START = "charon {";
constexpr const char* CHARON_IKEV2_STRONGSWAN_TEMPCONFIG_START = R"(
charon {
    plugins {
        kernel-libipsec {
            allow_peer_ts = yes
        }
    }
)";

constexpr const char* CHARON_L2TP_STRONGSWAN_TEMPCONFIG_START = R"(
charon {
load_modular = yes
    plugins {
        include /system/etc/strongswan/strongswan.d/charon/*.conf
        kernel-libipsec {
            load = no
        }
    }
)";

constexpr const char* CHARON_XAUTH_PSK_STRONGSWAN_TEMPCONFIG_START = R"(
charon {
    i_dont_care_about_security_and_use_aggressive_mode_psk = yes
    plugins {
        kernel-netlink {
            install_routes_xfrmi = yes
        }
    }
)";

constexpr const char* CHARON_XAUTH_RSA_STRONGSWAN_TEMPCONFIG_START = R"(
charon {
    i_dont_care_about_security_and_use_aggressive_mode_psk = yes
)";

constexpr const char* CHARON_STRONGSWAN_TEMPCONFIG_END = R"(
})";

constexpr const char* INCLUDE_STRONGSWAN_TEMPCONFIG = R"(
include /system/etc/strongswan/strongswan.d/*.conf)";

int32_t VpnTemplateProcessor::BuildConfig(sptr<L2tpVpnConfig> &l2tpConfig)
{
    if (l2tpConfig == nullptr) {
        NETMGR_EXT_LOG_E("config is null.");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    GenOptionsL2tpdClient(l2tpConfig);
    GenXl2tpdConf(l2tpConfig);
    GenIpsecConf(l2tpConfig);
    GenIpsecSecrets(l2tpConfig);
    GenStrongSwanConf(l2tpConfig->vpnType_, l2tpConfig->strongswanConf_);
    l2tpConfig->strongswanConf_ = Base64::Encode(l2tpConfig->strongswanConf_);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t VpnTemplateProcessor::BuildConfig(sptr<IpsecVpnConfig> &ipsecConfig)
{
    if (ipsecConfig == nullptr) {
        NETMGR_EXT_LOG_E("config is null.");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    GenSwanctlConf(ipsecConfig);
    GenStrongSwanConf(ipsecConfig->vpnType_, ipsecConfig->strongswanConf_);
    ipsecConfig->strongswanConf_ = Base64::Encode(ipsecConfig->strongswanConf_);
    return NETMANAGER_EXT_SUCCESS;
}

void VpnTemplateProcessor::GenSwanctlConf(sptr<IpsecVpnConfig> &config)
{
    std::string conf;
    switch (config->vpnType_) {
        case VpnType::IKEV2_IPSEC_MSCHAPv2:
            conf.append(IKE2_IPSEC_MSCHAPV2_SWANCTL_TEMPCONFIG);
            break;
        case VpnType::IKEV2_IPSEC_PSK:
            conf.append(IKE2_IPSEC_PSK_SWANCTL_TEMPCONFIG);
            break;
        case VpnType::IKEV2_IPSEC_RSA:
            conf.append(IKE2_IPSEC_RSA_SWANCTL_TEMPCONFIG);
            break;
        case VpnType::IPSEC_HYBRID_RSA:
            conf.append(IPSEC_HYBRID_RSA_SWANCTL_TEMPCONFIG);
            break;
        case VpnType::IPSEC_XAUTH_PSK:
            conf.append(IPSEC_XAUTH_PSK_SWANCTL_TEMPCONFIG);
            break;
        case VpnType::IPSEC_XAUTH_RSA:
            conf.append(IPSEC_XAUTH_RSA_SWANCTL_TEMPCONFIG);
            break;
        default:
            break;
    }
    std::unordered_map<std::string, std::string> params;
    params[KEY_VPN_ADDRESS] = config->addresses_[0].address_;
    params[KEY_VPN_IPSEC_IDENTIFIER] = config->ipsecIdentifier_;
    params[KEY_VPN_IPSEC_SHAREDKEY] = config->ipsecPreSharedKey_;
    params[KEY_VPN_USERNAME] = config->userName_;
    params[KEY_VPN_PASSWORD] = config->password_;
    InflateConf(conf, params);
    config->swanctlConf_ = Base64::Encode(conf);
}

void VpnTemplateProcessor::GenXl2tpdConf(sptr<L2tpVpnConfig> &config)
{
    int32_t configType = config->vpnType_;
    std::string conf;
    if (configType == VpnType::L2TP_IPSEC_PSK || configType == VpnType::L2TP_IPSEC_RSA
        || configType == VpnType::L2TP) {
        conf.append(L2TP_IPSEC_XL2TP_TEMPCONFIG);
    }
    std::unordered_map<std::string, std::string> params;
    params[KEY_VPN_ADDRESS] = config->addresses_[0].address_;
    InflateConf(conf, params);
    config->xl2tpdConf_ = Base64::Encode(conf);
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
    config->optionsL2tpdClient_ = Base64::Encode(conf);
}

void VpnTemplateProcessor::GenIpsecConf(sptr<L2tpVpnConfig> &config)
{
    int32_t configType = config->vpnType_;
    std::string conf;
    if (configType == VpnType::L2TP_IPSEC_RSA) {
        conf.append(L2TP_IPSEC_RSA_IPSEC_TEMPCONFIG);
    }
    if (configType == VpnType::L2TP_IPSEC_PSK) {
        conf.append(L2TP_IPSEC_PSK_IPSEC_TEMPCONFIG);
    }
    std::unordered_map<std::string, std::string> params;
    params[KEY_VPN_ADDRESS] = config->addresses_[0].address_;
    InflateConf(conf, params);
    config->ipsecConf_ = Base64::Encode(conf);
}

void VpnTemplateProcessor::GenIpsecSecrets(sptr<L2tpVpnConfig> &config)
{
    int32_t configType = config->vpnType_;
    std::string conf;
    if (configType == VpnType::L2TP_IPSEC_RSA) {
        conf.append(L2TP_IPSEC_RSA_IPSEC_SECERETS_TEMPCONFIG);
    }
    if (configType == VpnType::L2TP_IPSEC_PSK) {
        conf.append(L2TP_IPSEC_PSK_IPSEC_SECERETS_TEMPCONFIG);
    }
    std::unordered_map<std::string, std::string> params;
    params[KEY_VPN_IPSEC_SHAREDKEY] = config->ipsecPreSharedKey_;
    InflateConf(conf, params);
    config->ipsecSecrets_ = Base64::Encode(conf);
}

void VpnTemplateProcessor::GenStrongSwanConf(int32_t configType, std::string &outConf)
{
    outConf = "# /etc/strongswan.conf - strongSwan configuration file";
    if (configType == VpnType::IKEV2_IPSEC_MSCHAPv2 || configType == VpnType::IKEV2_IPSEC_RSA
        || configType == VpnType::IPSEC_HYBRID_RSA || configType == VpnType::IPSEC_XAUTH_PSK
        || configType == VpnType::IPSEC_XAUTH_RSA) {
        outConf.append(SWANCTL_STRONGSWAN_TEMPCONFIG);
    } else if (configType == VpnType::IKEV2_IPSEC_PSK) {
        outConf.append(SWANCTL_PSK_STRONGSWAN_TEMPCONFIG);
    }
    if (configType == VpnType::IKEV2_IPSEC_PSK) {
        outConf.append(CHARON_IKEV2_STRONGSWAN_TEMPCONFIG_START);
    } else if (configType == VpnType::L2TP_IPSEC_PSK || configType == VpnType::L2TP_IPSEC_RSA) {
        outConf.append(CHARON_L2TP_STRONGSWAN_TEMPCONFIG_START);
    } else if (configType == VpnType::IPSEC_XAUTH_PSK) {
        outConf.append(CHARON_XAUTH_PSK_STRONGSWAN_TEMPCONFIG_START);
    } else if (configType == VpnType::IPSEC_XAUTH_RSA) {
        outConf.append(CHARON_XAUTH_RSA_STRONGSWAN_TEMPCONFIG_START);
    } else {
        outConf.append(CHARON_STRONGSWAN_TEMPCONFIG_START);
    }
    outConf.append(CHARON_STRONGSWAN_TEMPCONFIG_END);
    if (configType == VpnType::L2TP_IPSEC_PSK || configType == VpnType::L2TP_IPSEC_RSA) {
        outConf.append(INCLUDE_STRONGSWAN_TEMPCONFIG);
    }
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
} // namespace NetManagerStandard
} // namespace OHOS
