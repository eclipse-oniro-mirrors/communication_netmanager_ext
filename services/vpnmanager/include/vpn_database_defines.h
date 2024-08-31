/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef VPN_DATABASE_DEFINES_H
#define VPN_DATABASE_DEFINES_H

#include <map>
#include <string>

namespace OHOS {
namespace NetManagerStandard {
namespace VpnDatabaseDefines {
constexpr const char *VPN_DATABASE_PATH = "/data/service/el1/public/vpn/vpn_data.db";
constexpr const char *VPN_CONFIG_TABLE_CREATE_PARAM = "vpnId TEXT PRIMARY KEY NOT NULL,"
    "vpnName TEXT NOT NULL,"
    "vpnType INTEGER NOT NULL,"
    "vpnAddress TEXT NOT NULL,"
    "userName TEXT NOT NULL,"
    "password TEXT NOT NULL,"
    "userId INTEGER NOT NULL,"
    "isLegacy INTEGER NOT NULL,"
    "saveLogin INTEGER NOT NULL,"
    "forwardingRoutes TEXT NOT NULL,"
    "dnsAddresses TEXT NOT NULL,"
    "searchDomains TEXT NOT NULL,"
    "ovpnPort TEXT NOT NULL,"
    "ovpnProtocol INTEGER NOT NULL,"
    "ovpnConfig TEXT NOT NULL,"
    "ovpnAuthType INTEGER NOT NULL,"
    "askpass TEXT NOT NULL,"
    "ovpnConfigFilePath TEXT NOT NULL,"
    "ovpnCaCertFilePath TEXT NOT NULL,"
    "ovpnUserCertFilePath TEXT NOT NULL,"
    "ovpnPrivateKeyFilePath TEXT NOT NULL,"
    "ipsecPreSharedKey TEXT NOT NULL,"
    "ipsecIdentifier TEXT NOT NULL,"
    "swanctlConf TEXT NOT NULL,"
    "strongswanConf TEXT NOT NULL,"
    "ipsecCaCertConf TEXT NOT NULL,"
    "ipsecPrivateUserCertConf TEXT NOT NULL,"
    "ipsecPublicUserCertConf TEXT NOT NULL,"
    "ipsecPrivateServerCertConf TEXT NOT NULL,"
    "ipsecPublicServerCertConf TEXT NOT NULL,"
    "ipsecCaCertFilePath TEXT NOT NULL,"
    "ipsecPrivateUserCertFilePath TEXT NOT NULL,"
    "ipsecPublicUserCertFilePath TEXT NOT NULL,"
    "ipsecPrivateServerCertFilePath TEXT NOT NULL,"
    "ipsecPublicServerCertFilePath TEXT NOT NULL,"
    "ipsecConf TEXT NOT NULL,"
    "ipsecSecrets TEXT NOT NULL,"
    "optionsL2tpdClient TEXT NOT NULL,"
    "xl2tpdConf TEXT NOT NULL,"
    "l2tpSharedKey TEXT NOT NULL";

const std::string VPN_CONFIG_TABLE = "T_vpn_config";

constexpr int32_t VPN_CONFIG_TABLE_PARAM_NUM = 40;

constexpr int32_t DATABASE_OPEN_VERSION = 1;

const std::string VPN_ID = "vpnId";
const std::string VPN_NAME = "vpnName";
const std::string VPN_TYPE = "vpnType";
const std::string VPN_ADDRESS = "vpnAddress";
const std::string USER_NAME = "userName";
const std::string PASSWORD = "password";
const std::string USER_ID = "userId";
const std::string VPN_IS_LEGACY = "isLegacy";
const std::string VPN_SAVE_LOGIN = "saveLogin";
const std::string VPN_FORWARDED_ROUTES = "forwardingRoutes";
const std::string VPN_DNS_ADDRESSES = "dnsAddresses";
const std::string VPN_SEARCH_DOMAINS = "searchDomains";

const std::string OPEN_VPN_PORT = "ovpnPort";
const std::string OPEN_VPN_PROTOCOL = "ovpnProtocol";
const std::string OPEN_VPN_CFG = "ovpnConfig";
const std::string OPEN_VPN_AUTH_TYPE = "ovpnAuthType";
const std::string OPEN_VPN_ASKPASS = "askpass";
const std::string OPEN_VPN_CFG_FILE_PATH = "ovpnConfigFilePath";
const std::string OPEN_VPN_CA_CERT_FILE_PATH = "ovpnCaCertFilePath";
const std::string OPEN_VPN_USER_CERT_FILE_PATH = "ovpnUserCertFilePath";
const std::string OPEN_VPN_PRIVATE_KEY_FILE_PATH = "ovpnPrivateKeyFilePath";

const std::string IPSEC_PRE_SHARE_KEY = "ipsecPreSharedKey";
const std::string IPSEC_IDENTIFIER = "ipsecIdentifier";
const std::string SWANCTL_CONF = "swanctlConf";
const std::string STRONGSWAN_CONF = "strongswanConf";
const std::string IPSEC_CA_CERT_CONF = "ipsecCaCertConf";
const std::string IPSEC_PRIVATE_USER_CERT_CONF = "ipsecPrivateUserCertConf";
const std::string IPSEC_PUBLIC_USER_CERT_CONF = "ipsecPublicUserCertConf";
const std::string IPSEC_PRIVATE_SERVER_CERT_CONF = "ipsecPrivateServerCertConf";
const std::string IPSEC_PUBLIC_SERVER_CERT_CONF = "ipsecPublicServerCertConf";
const std::string IPSEC_CA_CERT_FILE_PATH = "ipsecCaCertFilePath";
const std::string IPSEC_PRIVATE_USER_CERT_FILE_PATH = "ipsecPrivateUserCertFilePath";
const std::string IPSEC_PUBLIC_USER_CERT_FILE_PATH = "ipsecPublicUserCertFilePath";
const std::string IPSEC_PRIVATE_SERVER_CERT_FILE_PATH = "ipsecPrivateServerCertFilePath";
const std::string IPSEC_PUBLIC_SERVER_CERT_FILE_PATH = "ipsecPublicServerCertFilePath";

const std::string IPSEC_CONF = "ipsecConf";
const std::string IPSEC_SECRETS = "ipsecSecrets";
const std::string OPTIONS_L2TPD_CLIENT = "optionsL2tpdClient";
const std::string XL2TPD_CONF = "xl2tpdConf";
const std::string L2TP_SHARED_KEY = "l2tpSharedKey";

constexpr int32_t INDEX_VPN_ID = 0;
constexpr int32_t INDEX_VPN_NAME = 1;
constexpr int32_t INDEX_VPN_TYPE = 2;
constexpr int32_t INDEX_VPN_ADDRESS = 3;
constexpr int32_t INDEX_USER_NAME = 4;
constexpr int32_t INDEX_PASSWORD = 5;
constexpr int32_t INDEX_USER_ID = 6;
constexpr int32_t INDEX_VPN_IS_LEGACY = 7;
constexpr int32_t INDEX_VPN_SAVE_LOGIN = 8;
constexpr int32_t INDEX_VPN_FORWARDED_ROUTES = 9;
constexpr int32_t INDEX_VPN_DNS_ADDRESSES = 10;
constexpr int32_t INDEX_VPN_SEARCH_DOMAINS = 11;
constexpr int32_t INDEX_OPEN_VPN_PORT = 12;
constexpr int32_t INDEX_OPEN_VPN_PROTOCOL = 13;
constexpr int32_t INDEX_OPEN_VPN_CFG = 14;
constexpr int32_t INDEX_OPEN_VPN_AUTH_TYPE = 15;
constexpr int32_t INDEX_OPEN_VPN_ASKPASS = 16;
constexpr int32_t INDEX_OPEN_VPN_CFG_FILE_PATH = 17;
constexpr int32_t INDEX_OPEN_VPN_CA_CERT_FILE_PATH = 18;
constexpr int32_t INDEX_OPEN_VPN_USER_CERT_FILE_PATH = 19;
constexpr int32_t INDEX_OPEN_VPN_PRIVATE_KEY_FILE_PATH = 20;
constexpr int32_t INDEX_IPSEC_PRE_SHARE_KEY = 21;
constexpr int32_t INDEX_IPSEC_IDENTIFIER = 22;
constexpr int32_t INDEX_SWANCTL_CONF = 23;
constexpr int32_t INDEX_STRONGSWAN_CONF = 24;
constexpr int32_t INDEX_IPSEC_CA_CERT_CONF = 25;
constexpr int32_t INDEX_IPSEC_PRIVATE_USER_CERT_CONF = 26;
constexpr int32_t INDEX_IPSEC_PUBLIC_USER_CERT_CONF = 27;
constexpr int32_t INDEX_IPSEC_PRIVATE_SERVER_CERT_CONF = 28;
constexpr int32_t INDEX_IPSEC_PUBLIC_SERVER_CERT_CONF = 29;
constexpr int32_t INDEX_IPSEC_CA_CERT_FILE_PATH = 30;
constexpr int32_t INDEX_IPSEC_PRIVATE_USER_CERT_FILE_PATH = 31;
constexpr int32_t INDEX_IPSEC_PUBLIC_USER_CERT_FILE_PATH = 32;
constexpr int32_t INDEX_IPSEC_PRIVATE_SERVER_CERT_FILE_PATH = 33;
constexpr int32_t INDEX_IPSEC_PUBLIC_SERVER_CERT_FILE_PATH = 34;
constexpr int32_t INDEX_IPSEC_CONF = 35;
constexpr int32_t INDEX_IPSEC_SECRETS = 36;
constexpr int32_t INDEX_OPTIONS_L2TPD_CLIENT = 37;
constexpr int32_t INDEX_XL2TPD_CONF = 38;
constexpr int32_t INDEX_L2TP_SHARED_KEY = 39;
} // namespace VpnDatabaseDefines
} // namespace NetManagerStandard
} // namespace OHOS

#endif // VPN_DATABASE_DEFINES_H
