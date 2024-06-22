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

#ifndef VPN_DATABASE_DEFINES_H
#define VPN_DATABASE_DEFINES_H

#include <map>

namespace OHOS {
namespace NetManagerStandard {
namespace VpnDatabaseDefines {
constexpr const char *VPN_DATABASE_PATH = "/data/service/el1/public/netmanager/vpn_data.db";
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
constexpr const char *VPN_CONFIG_TABLE_PARAM_LIST = "vpnId,vpnName,vpnType,vpnAddress,userName,password,"
    "userId,isLegacy,saveLogin,forwardingRoutes,dnsAddresses,searchDomains,ovpnPort,ovpnProtocol,ovpnConfig,"
    "ovpnAuthType,askpass,ovpnConfigFilePath,ovpnCaCertFilePath,ovpnUserCertFilePath,ovpnPrivateKeyFilePath,"
    "ipsecPreSharedKey,ipsecIdentifier,swanctlConf,strongswanConf,ipsecCaCertConf,ipsecPrivateUserCertConf,"
    "ipsecPublicUserCertConf,ipsecPrivateServerCertConf,ipsecPublicServerCertConf,ipsecCaCertFilePath,"
    "ipsecPrivateUserCertFilePath,ipsecPublicUserCertFilePath,ipsecPrivateServerCertFilePath,"
    "ipsecPublicServerCertFilePath,ipsecConf,ipsecSecrets,optionsL2tpdClient,xl2tpdConf,l2tpSharedKey";
constexpr const char *VPN_CONFIG_TABLE = "T_vpn_config";
constexpr const char *VPN_CONFIG_TABLE_UPDATE_SQL = "vpnName=?,vpnType=?,vpnAddress=?,userName=?,password=?,"
    "isLegacy=?,saveLogin=?,forwardingRoutes=?,dnsAddresses=?,searchDomains=?,ovpnPort=?,ovpnProtocol=?,"
    "ovpnConfig=?,ovpnAuthType=?,askpass=?,ovpnConfigFilePath=?,ovpnCaCertFilePath=?,ovpnUserCertFilePath=?,"
    "ovpnPrivateKeyFilePath=?,ipsecPreSharedKey=?,ipsecIdentifier=?,swanctlConf=?,strongswanConf=?,ipsecCaCertConf=?,"
    "ipsecPrivateUserCertConf=?,ipsecPublicUserCertConf=?,ipsecPrivateServerCertConf=?,ipsecPublicServerCertConf=?,"
    "ipsecCaCertFilePath=?,ipsecPrivateUserCertFilePath=?,ipsecPublicUserCertFilePath=?,"
    "ipsecPrivateServerCertFilePath=?,ipsecPublicServerCertFilePath=?,ipsecConf=?,ipsecSecrets=?,"
    "optionsL2tpdClient=?,xl2tpdConf=?,l2tpSharedKey=?";

constexpr int32_t VPN_CONFIG_TABLE_PARAM_NUM = 40;

} // namespace VpnDatabaseDefines
} // namespace NetManagerStandard
} // namespace OHOS

#endif // VPN_DATABASE_DEFINES_H
