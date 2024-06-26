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

#ifndef VPN_CONFIG_UTILS_H
#define VPN_CONFIG_UTILS_H

#include <cstddef>
#include <napi/native_api.h>
#include <string>
#include <vector>

#include "route.h"
#include "inet_addr.h"
#include "vpn_config.h"
#include "ipsecvpn_config.h"
#include "l2tpvpn_config.h"

namespace OHOS {
namespace NetManagerStandard {
namespace VpnConfigUtils {
constexpr int32_t NET_MASK_MAX_LENGTH = 32;
constexpr int32_t IPV6_NET_PREFIX_MAX_LENGTH = 128;
constexpr const char *CONFIG_ADDRESSES = "addresses";
constexpr const char *CONFIG_ROUTES = "routes";
constexpr const char *NET_ADDRESS = "address";
constexpr const char *NET_FAMILY = "family";
constexpr const char *NET_PORT = "port";
constexpr const char *NET_PREFIXLENGTH = "prefixLength";
constexpr const char *NET_INTERFACE = "interface";
constexpr const char *NET_DESTINATION = "destination";
constexpr const char *NET_GATEWAY = "gateway";
constexpr const char *NET_HAS_GATEWAY = "hasGateway";
constexpr const char *NET_ISDEFAULTROUTE = "isDefaultRoute";
constexpr const char *CONFIG_DNSADDRESSES = "dnsAddresses";
constexpr const char *CONFIG_SEARCHDOMAINS = "searchDomains";
constexpr const char *CONFIG_MTU = "mtu";
constexpr const char *CONFIG_ISIPV4ACCEPTED = "isIPv4Accepted";
constexpr const char *CONFIG_ISIPV6ACCEPTED = "isIPv6Accepted";
constexpr const char *CONFIG_ISLEGACY = "isLegacy";
constexpr const char *CONFIG_ISMETERED = "isMetered";
constexpr const char *CONFIG_ISBLOCKING = "isBlocking";
constexpr const char *CONFIG_TRUSTEDAPPLICATIONS = "trustedApplications";
constexpr const char *CONFIG_BLOCKEDAPPLICATIONS = "blockedApplications";

constexpr const char *CONFIG_VPN_ID = "vpnId";
constexpr const char *CONFIG_VPN_NAME = "vpnName";
constexpr const char *CONFIG_VPN_TYPE = "vpnType";
constexpr const char *CONFIG_USER_NAME = "userName";
constexpr const char *CONFIG_PASSWORD = "password";
constexpr const char *CONFIG_SAVE_LOGIN = "saveLogin";
constexpr const char *CONFIG_FORWARDED_ROUTES = "forwardingRoutes";

constexpr const char *CONFIG_OVPN_PORT = "ovpnPort";
constexpr const char *CONFIG_OPEN_VPN_PROTOCOL = "ovpnProtocol";
constexpr const char *CONFIG_OPEN_VPN_CFG = "ovpnConfig";
constexpr const char *CONFIG_OPEN_VPN_AUTH_TYPE = "ovpnAuthType";
constexpr const char *CONFIG_ASKPASS = "askpass";
constexpr const char *CONFIG_OPEN_VPN_CFG_FILE_PATH = "ovpnConfigFilePath";
constexpr const char *CONFIG_OPEN_VPN_CA_CERT_FILE_PATH = "ovpnCaCertFilePath";
constexpr const char *CONFIG_OPEN_VPN_USER_CERT_FILE_PATH = "ovpnUserCertFilePath";
constexpr const char *CONFIG_OPEN_VPN_PRIVATE_KEY_FILE_PATH = "ovpnPrivateKeyFilePath";

constexpr const char *CONFIG_IPSEC_PRE_SHARE_KEY = "ipsecPreSharedKey";
constexpr const char *CONFIG_IPSEC_IDENTIFIER = "ipsecIdentifier";
constexpr const char *CONFIG_SWANCTL_CONF = "swanctlConf";
constexpr const char *CONFIG_STRONGSWAN_CONF = "strongswanConf";
constexpr const char *CONFIG_IPSEC_CA_CERT_CONF = "ipsecCaCertConf";
constexpr const char *CONFIG_IPSEC_PRIVATE_USER_CERT_CONF = "ipsecPrivateUserCertConf";
constexpr const char *CONFIG_IPSEC_PUBLIC_USER_CERT_CONF = "ipsecPublicUserCertConf";
constexpr const char *CONFIG_IPSEC_PRIVATE_SERVER_CERT_CONF = "ipsecPrivateServerCertConf";
constexpr const char *CONFIG_IPSEC_PUBLIC_SERVER_CERT_CONF = "ipsecPublicServerCertConf";
constexpr const char *CONFIG_IPSEC_CA_CERT_FILE_PATH = "ipsecCaCertFilePath";
constexpr const char *CONFIG_IPSEC_PRIVATE_USER_CERT_FILE_PATH = "ipsecPrivateUserCertFilePath";
constexpr const char *CONFIG_IPSEC_PUBLIC_USER_CERT_FILE_PATH = "ipsecPublicUserCertFilePath";
constexpr const char *CONFIG_IPSEC_PRIVATE_SERVER_CERT_FILE_PATH = "ipsecPrivateServerCertFilePath";
constexpr const char *CONFIG_IPSEC_PUBLIC_SERVER_CERT_FILE_PATH = "ipsecPublicServerCertFilePath";

constexpr const char *CONFIG_IPSEC_CONF = "ipsecConf";
constexpr const char *CONFIG_IPSEC_SECRETS = "ipsecSecrets";
constexpr const char *CONFIG_OPTIONS_L2TPD_CLIENT = "optionsL2tpdClient";
constexpr const char *CONFIG_XL2TPD_CONF = "xl2tpdConf";
constexpr const char *CONFIG_L2TP_SHARED_KEY = "l2tpSharedKey";

bool ParseSysVpnConfig(napi_env env, napi_value *params, sptr<SysVpnConfig> &vpnConfig);
bool ParseAddrRouteParams(napi_env env, napi_value config, sptr<SysVpnConfig> &vpnConfig);
bool ParseChoiceableParams(napi_env env, napi_value config, sptr<SysVpnConfig> &vpnConfig);

bool ParseIpsecVpnParams(napi_env env, napi_value config, sptr<SysVpnConfig> &vpnConfig);
bool ParseL2tpVpnParams(napi_env env, napi_value config, sptr<SysVpnConfig> &vpnConfig);

bool ParseAddress(napi_env env, napi_value address, struct INetAddr &iNetAddr);
bool ParseDestination(napi_env env, napi_value jsRoute, struct INetAddr &iNetAddr);
bool ParseGateway(napi_env env, napi_value jsRoute, struct INetAddr &iNetAddr);
bool ParseRoute(napi_env env, napi_value jsRoute, Route &route);
bool ParseOptionArrayString(napi_env env, napi_value config, const std::string &key,
    std::vector<std::string> &vector);
void GetBoolFromJsOptionItem(napi_env env, napi_value object, const std::string &key, bool &value);
void GetUint8FromJsOptionItem(napi_env env, napi_value object, const std::string &key, uint8_t &value);
void GetStringFromJsOptionItem(napi_env env, napi_value object, const std::string &key, std::string &value);
bool GetStringFromJsMandatoryItem(napi_env env, napi_value object, const std::string &key, std::string &value);
void GetInt32FromJsOptionItem(napi_env env, napi_value object, const std::string &key, int32_t &value);

napi_value CreateNapiVpnConfig(napi_env env, sptr<SysVpnConfig> &sysVpnConfig);
napi_value CreateNapiSysVpnConfig(napi_env env, sptr<SysVpnConfig> &sysVpnConfig);
napi_value CreateNapiIpsecVpnConfig(napi_env env, sptr<SysVpnConfig> &sysVpnConfig);
napi_value CreateNapiL2tpVpnConfig(napi_env env, sptr<SysVpnConfig> &sysVpnConfig);
}
} // namespace NetManagerStandard
} // namespace OHOS

#endif // VPN_CONFIG_UTILS_H