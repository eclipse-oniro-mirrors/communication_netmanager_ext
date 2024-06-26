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

#include "vpn_config_utils.h"

#include "napi_utils.h"
#include "inet_addr.h"
#include "net_manager_constants.h"
#include "netmanager_base_common_utils.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
namespace VpnConfigUtils {
bool ParseSysVpnConfig(napi_env env, napi_value *params, sptr<SysVpnConfig> &vpnConfig)
{
    int vpnType = -1;
    GetInt32FromJsOptionItem(env, params[0], CONFIG_VPN_TYPE, vpnType);
    switch (vpnType) {
        case static_cast<int32_t>(VpnType::IKEV2_IPSEC_MSCHAPv2):
        case static_cast<int32_t>(VpnType::IKEV2_IPSEC_PSK):
        case static_cast<int32_t>(VpnType::IKEV2_IPSEC_RSA):
        case static_cast<int32_t>(VpnType::IPSEC_XAUTH_PSK):
        case static_cast<int32_t>(VpnType::IPSEC_XAUTH_RSA):
        case static_cast<int32_t>(VpnType::IPSEC_HYBRID_RSA):
            vpnConfig = new (std::nothrow) IpsecVpnConfig();
            if (!ParseIpsecVpnParams(env, params[0], vpnConfig)) {
                NETMGR_EXT_LOG_E("ParseIpsecVpnParams failed");
                return false;
            }
            break;
        case static_cast<int32_t>(VpnType::L2TP_IPSEC_PSK):
        case static_cast<int32_t>(VpnType::L2TP_IPSEC_RSA):
            vpnConfig = new (std::nothrow) L2tpVpnConfig();
            if (!ParseL2tpVpnParams(env, params[0], vpnConfig)) {
                NETMGR_EXT_LOG_E("ParseL2tpVpnParams failed");
                return false;
            }
            break;
        default:
            NETMGR_EXT_LOG_E("sysvpn ParseSysVpnConfig failed! invalid type=%{public}d", vpnType);
            return false;
    }

    if (!ParseAddrRouteParams(env, params[0], vpnConfig) || !ParseChoiceableParams(env, params[0], vpnConfig)) {
        return false;
    }
    return true;
}

bool ParseAddrRouteParams(napi_env env, napi_value config, sptr<SysVpnConfig> &vpnConfig)
{
    if (vpnConfig == nullptr) {
        NETMGR_EXT_LOG_E("vpnConfig is null");
        return false;
    }
    // parse addresses.
    if (NapiUtils::HasNamedProperty(env, config, CONFIG_ADDRESSES)) {
        napi_value addrArray = NapiUtils::GetNamedProperty(env, config, CONFIG_ADDRESSES);
        if (!NapiUtils::IsArray(env, addrArray)) {
            NETMGR_EXT_LOG_E("addresses is not array");
            return false;
        }
        uint32_t addrLength = NapiUtils::GetArrayLength(env, addrArray);
        for (uint32_t i = 0; i < addrLength; ++i) { // set length limit.
            INetAddr iNetAddr;
            if (!ParseAddress(env, NapiUtils::GetArrayElement(env, addrArray, i), iNetAddr)) {
                NETMGR_EXT_LOG_E("ParseAddress failed");
                return false;
            }
            vpnConfig->addresses_.emplace_back(iNetAddr);
            bool isIpv6 = CommonUtils::IsValidIPV6(iNetAddr.address_);
            vpnConfig->isAcceptIPv4_ = !isIpv6;
            vpnConfig->isAcceptIPv6_ = isIpv6;
        }
    }

    // parse routes.
    if (NapiUtils::HasNamedProperty(env, config, CONFIG_ROUTES)) {
        napi_value routes = NapiUtils::GetNamedProperty(env, config, CONFIG_ROUTES);
        if (!NapiUtils::IsArray(env, routes)) {
            NETMGR_EXT_LOG_E("routes is not array");
            return false;
        }
        uint32_t routesLength = NapiUtils::GetArrayLength(env, routes);
        for (uint32_t idx = 0; idx < routesLength; ++idx) { // set length limit.
            struct Route routeInfo;
            if (!ParseRoute(env, NapiUtils::GetArrayElement(env, routes, idx), routeInfo)) {
                NETMGR_EXT_LOG_E("ParseRoute failed");
                return false;
            }
            vpnConfig->routes_.emplace_back(routeInfo);
        }
    }
    return true;
}

bool ParseChoiceableParams(napi_env env, napi_value config, sptr<SysVpnConfig> &vpnConfig)
{
    if (vpnConfig == nullptr) {
        NETMGR_EXT_LOG_E("sysVpnConfig is null");
        return false;
    }
    ParseOptionArrayString(env, config, CONFIG_DNSADDRESSES, vpnConfig->dnsAddresses_);
    ParseOptionArrayString(env, config, CONFIG_SEARCHDOMAINS, vpnConfig->searchDomains_);
    GetInt32FromJsOptionItem(env, config, CONFIG_MTU, vpnConfig->mtu_);
    GetBoolFromJsOptionItem(env, config, CONFIG_ISIPV4ACCEPTED, vpnConfig->isAcceptIPv4_);
    GetBoolFromJsOptionItem(env, config, CONFIG_ISIPV6ACCEPTED, vpnConfig->isAcceptIPv6_);
    GetBoolFromJsOptionItem(env, config, CONFIG_ISLEGACY, vpnConfig->isLegacy_);
    GetBoolFromJsOptionItem(env, config, CONFIG_ISMETERED, vpnConfig->isMetered_);
    GetBoolFromJsOptionItem(env, config, CONFIG_ISBLOCKING, vpnConfig->isBlocking_);
    ParseOptionArrayString(env, config, CONFIG_TRUSTEDAPPLICATIONS, vpnConfig->acceptedApplications_);
    ParseOptionArrayString(env, config, CONFIG_BLOCKEDAPPLICATIONS, vpnConfig->refusedApplications_);
    return true;
}

bool ParseSystemVpnParams(napi_env env, napi_value config, sptr<SysVpnConfig> &sysVpnConfig)
{
    if (sysVpnConfig == nullptr) {
        NETMGR_EXT_LOG_E("sysVpnConfig is null");
        return false;
    }
    GetStringFromJsOptionItem(env, config, CONFIG_VPN_ID, sysVpnConfig->vpnId_);
    GetStringFromJsOptionItem(env, config, CONFIG_VPN_NAME, sysVpnConfig->vpnName_);
    GetInt32FromJsOptionItem(env, config, CONFIG_VPN_TYPE, sysVpnConfig->vpnType_);
    GetStringFromJsOptionItem(env, config, CONFIG_USER_NAME, sysVpnConfig->userName_);
    GetStringFromJsOptionItem(env, config, CONFIG_PASSWORD, sysVpnConfig->password_);
    GetStringFromJsOptionItem(env, config, CONFIG_FORWARDED_ROUTES, sysVpnConfig->forwardingRoutes_);
    GetBoolFromJsOptionItem(env, config, CONFIG_SAVE_LOGIN, sysVpnConfig->saveLogin_);
    return true;
}

bool ParseIpsecVpnParams(napi_env env, napi_value config, sptr<SysVpnConfig> &sysVpnConfig)
{
    if (sysVpnConfig == nullptr) {
        NETMGR_EXT_LOG_E("sysVpnConfig is null");
        return false;
    }
    if (!ParseSystemVpnParams(env, config, sysVpnConfig)) {
        NETMGR_EXT_LOG_E("ParseSystemVpnParams failed");
        return false;
    }

    sptr<IpsecVpnConfig> ipsecVpnConfig = sptr<IpsecVpnConfig>(
            static_cast<IpsecVpnConfig*>(sysVpnConfig.GetRefPtr()));
    if (ipsecVpnConfig) {
        GetStringFromJsOptionItem(env, config, CONFIG_IPSEC_PRE_SHARE_KEY, ipsecVpnConfig->ipsecPreSharedKey_);
        GetStringFromJsOptionItem(env, config, CONFIG_IPSEC_IDENTIFIER, ipsecVpnConfig->ipsecIdentifier_);
        GetStringFromJsOptionItem(env, config, CONFIG_SWANCTL_CONF, ipsecVpnConfig->swanctlConf_);
        GetStringFromJsOptionItem(env, config, CONFIG_STRONGSWAN_CONF, ipsecVpnConfig->strongswanConf_);
        GetStringFromJsOptionItem(env, config, CONFIG_IPSEC_CA_CERT_CONF, ipsecVpnConfig->ipsecCaCertConf_);
        GetStringFromJsOptionItem(env, config, CONFIG_IPSEC_PRIVATE_USER_CERT_CONF,
            ipsecVpnConfig->ipsecPrivateUserCertConf_);
        GetStringFromJsOptionItem(env, config, CONFIG_IPSEC_PUBLIC_USER_CERT_CONF,
            ipsecVpnConfig->ipsecPublicUserCertConf_);
        GetStringFromJsOptionItem(env, config, CONFIG_IPSEC_PRIVATE_SERVER_CERT_CONF,
            ipsecVpnConfig->ipsecPrivateServerCertConf_);
        GetStringFromJsOptionItem(env, config, CONFIG_IPSEC_PUBLIC_SERVER_CERT_CONF,
            ipsecVpnConfig->ipsecPublicServerCertConf_);
        GetStringFromJsOptionItem(env, config, CONFIG_IPSEC_CA_CERT_FILE_PATH, ipsecVpnConfig->ipsecCaCertFilePath_);
        GetStringFromJsOptionItem(env, config, CONFIG_IPSEC_PRIVATE_USER_CERT_FILE_PATH,
            ipsecVpnConfig->ipsecPrivateUserCertFilePath_);
        GetStringFromJsOptionItem(env, config, CONFIG_IPSEC_PUBLIC_USER_CERT_FILE_PATH,
            ipsecVpnConfig->ipsecPublicUserCertFilePath_);
        GetStringFromJsOptionItem(env, config, CONFIG_IPSEC_PRIVATE_SERVER_CERT_FILE_PATH,
            ipsecVpnConfig->ipsecPrivateServerCertFilePath_);
        GetStringFromJsOptionItem(env, config, CONFIG_IPSEC_PUBLIC_SERVER_CERT_FILE_PATH,
            ipsecVpnConfig->ipsecPublicServerCertFilePath_);
        return true;
    } else {
        return false;
    }
}

bool ParseL2tpVpnParams(napi_env env, napi_value config, sptr<SysVpnConfig> &sysVpnConfig)
{
    if (sysVpnConfig == nullptr) {
        NETMGR_EXT_LOG_E("sysVpnConfig is null");
        return false;
    }
    if (!ParseSystemVpnParams(env, config, sysVpnConfig)) {
        NETMGR_EXT_LOG_E("ParseSystemVpnParams failed");
        return false;
    }

    sptr<L2tpVpnConfig> l2tpVpnConfig = sptr<L2tpVpnConfig>(static_cast<L2tpVpnConfig*>(sysVpnConfig.GetRefPtr()));
    if (l2tpVpnConfig) {
        GetStringFromJsOptionItem(env, config, CONFIG_IPSEC_PRE_SHARE_KEY, l2tpVpnConfig->ipsecPreSharedKey_);
        GetStringFromJsOptionItem(env, config, CONFIG_IPSEC_IDENTIFIER, l2tpVpnConfig->ipsecIdentifier_);
        GetStringFromJsOptionItem(env, config, CONFIG_STRONGSWAN_CONF, l2tpVpnConfig->strongswanConf_);
        GetStringFromJsOptionItem(env, config, CONFIG_IPSEC_CA_CERT_CONF, l2tpVpnConfig->ipsecCaCertConf_);
        GetStringFromJsOptionItem(env, config, CONFIG_IPSEC_PRIVATE_USER_CERT_CONF,
            l2tpVpnConfig->ipsecPrivateUserCertConf_);
        GetStringFromJsOptionItem(env, config, CONFIG_IPSEC_PUBLIC_USER_CERT_CONF,
            l2tpVpnConfig->ipsecPublicUserCertConf_);
        GetStringFromJsOptionItem(env, config, CONFIG_IPSEC_PRIVATE_SERVER_CERT_CONF,
            l2tpVpnConfig->ipsecPrivateServerCertConf_);
        GetStringFromJsOptionItem(env, config, CONFIG_IPSEC_PUBLIC_SERVER_CERT_CONF,
            l2tpVpnConfig->ipsecPublicServerCertConf_);
        GetStringFromJsOptionItem(env, config, CONFIG_IPSEC_CA_CERT_FILE_PATH,
            l2tpVpnConfig->ipsecCaCertFilePath_);
        GetStringFromJsOptionItem(env, config, CONFIG_IPSEC_PRIVATE_USER_CERT_FILE_PATH,
            l2tpVpnConfig->ipsecPrivateUserCertFilePath_);
        GetStringFromJsOptionItem(env, config, CONFIG_IPSEC_PUBLIC_USER_CERT_FILE_PATH,
            l2tpVpnConfig->ipsecPublicUserCertFilePath_);
        GetStringFromJsOptionItem(env, config, CONFIG_IPSEC_PRIVATE_SERVER_CERT_FILE_PATH,
            l2tpVpnConfig->ipsecPrivateServerCertFilePath_);
        GetStringFromJsOptionItem(env, config, CONFIG_IPSEC_PUBLIC_SERVER_CERT_FILE_PATH,
            l2tpVpnConfig->ipsecPublicServerCertFilePath_);
        GetStringFromJsOptionItem(env, config, CONFIG_IPSEC_CONF, l2tpVpnConfig->ipsecConf_);
        GetStringFromJsOptionItem(env, config, CONFIG_IPSEC_SECRETS, l2tpVpnConfig->ipsecSecrets_);
        GetStringFromJsOptionItem(env, config, CONFIG_OPTIONS_L2TPD_CLIENT, l2tpVpnConfig->optionsL2tpdClient_);
        GetStringFromJsOptionItem(env, config, CONFIG_XL2TPD_CONF, l2tpVpnConfig->xl2tpdConf_);
        GetStringFromJsOptionItem(env, config, CONFIG_L2TP_SHARED_KEY, l2tpVpnConfig->l2tpSharedKey_);
        return true;
    } else {
        return false;
    }
}

bool ParseAddress(napi_env env, napi_value address, struct INetAddr &iNetAddr)
{
    napi_value netAddress = NapiUtils::GetNamedProperty(env, address, NET_ADDRESS);
    if (NapiUtils::GetValueType(env, netAddress) != napi_object) {
        NETMGR_EXT_LOG_E("param address type is mismatch");
        return false;
    }

    if (!GetStringFromJsMandatoryItem(env, netAddress, NET_ADDRESS, iNetAddr.address_)) {
        NETMGR_EXT_LOG_E("get address-address failed");
        return false;
    }

    bool isIpv6 = CommonUtils::IsValidIPV6(iNetAddr.address_);
    if (!isIpv6) {
        if (!CommonUtils::IsValidIPV4(iNetAddr.address_)) {
            NETMGR_EXT_LOG_E("invalid ip address");
            return false;
        }
    }

    GetUint8FromJsOptionItem(env, netAddress, NET_FAMILY, iNetAddr.family_);
    GetUint8FromJsOptionItem(env, netAddress, NET_PORT, iNetAddr.port_);

    if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, address, NET_PREFIXLENGTH)) != napi_number) {
        NETMGR_EXT_LOG_E("param NET_PREFIXLENGTH type is mismatch");
        return false;
    }
    if (!isIpv6) {
        iNetAddr.prefixlen_ = static_cast<uint8_t>(NapiUtils::GetUint32Property(env, address, NET_PREFIXLENGTH));
    } else {
        iNetAddr.prefixlen_ = CommonUtils::Ipv6PrefixLen(iNetAddr.address_);
    }

    uint32_t prefix = iNetAddr.prefixlen_;
    if (prefix == 0 || prefix >= (isIpv6 ? IPV6_NET_PREFIX_MAX_LENGTH : NET_MASK_MAX_LENGTH)) {
        NETMGR_EXT_LOG_E("prefixlen_ error");
        return false;
    }
    if (!isIpv6) {
        uint32_t maskUint = (0xFFFFFFFF << (NET_MASK_MAX_LENGTH - prefix));
        uint32_t ipAddrUint = CommonUtils::ConvertIpv4Address(iNetAddr.address_);
        uint32_t subNetAddress = ipAddrUint & maskUint;
        uint32_t boardcastAddress = subNetAddress | (~maskUint);
        if ((ipAddrUint == subNetAddress) || (ipAddrUint == boardcastAddress)) {
            NETMGR_EXT_LOG_E("invalid ip address");
            return false;
        }
    }
    return true;
}

bool ParseDestination(napi_env env, napi_value jsRoute, struct INetAddr &iNetAddr)
{
    napi_value destination = NapiUtils::GetNamedProperty(env, jsRoute, NET_DESTINATION);
    if (NapiUtils::GetValueType(env, destination) != napi_object) {
        NETMGR_EXT_LOG_E("param destination type is mismatch");
        return false;
    }

    napi_value netAddress = NapiUtils::GetNamedProperty(env, destination, NET_ADDRESS);
    if (NapiUtils::GetValueType(env, netAddress) != napi_object) {
        NETMGR_EXT_LOG_E("param address type is mismatch");
        return false;
    }

    if (!GetStringFromJsMandatoryItem(env, netAddress, NET_ADDRESS, iNetAddr.address_)) {
        NETMGR_EXT_LOG_E("get destination-address failed");
        return false;
    }

    if (!CommonUtils::IsValidIPV4(iNetAddr.address_) && !CommonUtils::IsValidIPV6(iNetAddr.address_)) {
        NETMGR_EXT_LOG_E("invalid ip address");
        return false;
    }

    GetUint8FromJsOptionItem(env, netAddress, NET_FAMILY, iNetAddr.family_);
    GetUint8FromJsOptionItem(env, netAddress, NET_PORT, iNetAddr.port_);
    GetUint8FromJsOptionItem(env, destination, NET_PREFIXLENGTH, iNetAddr.prefixlen_);
    return true;
}

bool ParseGateway(napi_env env, napi_value jsRoute, struct INetAddr &iNetAddr)
{
    napi_value gateway = NapiUtils::GetNamedProperty(env, jsRoute, NET_GATEWAY);
    if (NapiUtils::GetValueType(env, gateway) != napi_object) {
        NETMGR_EXT_LOG_E("param gateway type is mismatch");
        return false;
    }

    if (!GetStringFromJsMandatoryItem(env, gateway, NET_ADDRESS, iNetAddr.address_)) {
        NETMGR_EXT_LOG_E("get gateway-address failed");
        return false;
    }

    GetUint8FromJsOptionItem(env, gateway, NET_FAMILY, iNetAddr.family_);
    GetUint8FromJsOptionItem(env, gateway, NET_PORT, iNetAddr.port_);
    return true;
}

bool ParseRoute(napi_env env, napi_value jsRoute, Route &route)
{
    GetStringFromJsOptionItem(env, jsRoute, NET_INTERFACE, route.iface_);

    if (!ParseDestination(env, jsRoute, route.destination_)) {
        NETMGR_EXT_LOG_E("ParseDestination failed");
        return false;
    }
    if (!ParseGateway(env, jsRoute, route.gateway_)) {
        NETMGR_EXT_LOG_E("ParseGateway failed");
        return false;
    }

    GetBoolFromJsOptionItem(env, jsRoute, NET_HAS_GATEWAY, route.hasGateway_);
    GetBoolFromJsOptionItem(env, jsRoute, NET_ISDEFAULTROUTE, route.isDefaultRoute_);
    return true;
}

bool ParseOptionArrayString(napi_env env, napi_value config, const std::string &key,
    std::vector<std::string> &vector)
{
    if (NapiUtils::HasNamedProperty(env, config, key)) {
        napi_value array = NapiUtils::GetNamedProperty(env, config, key);
        if (!NapiUtils::IsArray(env, array)) {
            NETMGR_EXT_LOG_E("param [%{public}s] is not array", key.c_str());
            return false;
        }
        uint32_t arrayLength = NapiUtils::GetArrayLength(env, array);
        for (uint32_t i = 0; i < arrayLength; ++i) {
            std::string item = NapiUtils::GetStringFromValueUtf8(env, NapiUtils::GetArrayElement(env, array, i));
            vector.push_back(item);
        }
    }
    return true;
}

bool GetStringFromJsMandatoryItem(napi_env env, napi_value object, const std::string &key, std::string &value)
{
    if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, object, key)) != napi_string) {
        NETMGR_EXT_LOG_E("param [%{public}s] type is mismatch", key.c_str());
        return false;
    }
    value = NapiUtils::GetStringPropertyUtf8(env, object, key);
    return (value.empty()) ? false : true;
}

void GetStringFromJsOptionItem(napi_env env, napi_value object, const std::string &key, std::string &value)
{
    if (NapiUtils::HasNamedProperty(env, object, key)) {
        if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, object, key)) == napi_string) {
            value = NapiUtils::GetStringPropertyUtf8(env, object, key);
        } else {
            NETMGR_EXT_LOG_E("param [%{public}s] type is mismatch", key.c_str());
        }
    }
}

void GetUint8FromJsOptionItem(napi_env env, napi_value object, const std::string &key, uint8_t &value)
{
    if (NapiUtils::HasNamedProperty(env, object, key)) {
        if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, object, key)) == napi_number) {
            value = static_cast<uint8_t>(NapiUtils::GetUint32Property(env, object, key));
        } else {
            NETMGR_EXT_LOG_E("param [%{public}s] type is mismatch", key.c_str());
        }
    }
}

void GetBoolFromJsOptionItem(napi_env env, napi_value object, const std::string &key, bool &value)
{
    if (NapiUtils::HasNamedProperty(env, object, key)) {
        if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, object, key)) == napi_boolean) {
            value = NapiUtils::GetBooleanProperty(env, object, key);
        } else {
            NETMGR_EXT_LOG_E("param [%{public}s] type is mismatch", key.c_str());
        }
    }
}

void GetInt32FromJsOptionItem(napi_env env, napi_value object, const std::string &key, int32_t &value)
{
    if (NapiUtils::HasNamedProperty(env, object, key)) {
        if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, object, key)) == napi_number) {
            value = NapiUtils::GetInt32Property(env, object, key);
        } else {
            NETMGR_EXT_LOG_E("param [%{public}s] type is mismatch", key.c_str());
        }
    }
}

napi_value CreateNapiVpnConfig(napi_env env, sptr<SysVpnConfig> &sysVpnConfig)
{
    if (sysVpnConfig == nullptr) {
        return NapiUtils::GetUndefined(env);
    }
    switch (sysVpnConfig->vpnType_) {
        case static_cast<int32_t>(VpnType::IKEV2_IPSEC_MSCHAPv2):
        case static_cast<int32_t>(VpnType::IKEV2_IPSEC_PSK):
        case static_cast<int32_t>(VpnType::IKEV2_IPSEC_RSA):
        case static_cast<int32_t>(VpnType::IPSEC_XAUTH_PSK):
        case static_cast<int32_t>(VpnType::IPSEC_XAUTH_RSA):
        case static_cast<int32_t>(VpnType::IPSEC_HYBRID_RSA):
            return CreateNapiIpsecVpnConfig(env, sysVpnConfig);
        case static_cast<int32_t>(VpnType::L2TP_IPSEC_PSK):
        case static_cast<int32_t>(VpnType::L2TP_IPSEC_RSA):
            return CreateNapiL2tpVpnConfig(env, sysVpnConfig);
        default:
            return NapiUtils::GetUndefined(env);
    }
}

napi_value CreateNapiSysVpnConfig(napi_env env, sptr<SysVpnConfig> &sysVpnConfig)
{
    napi_value config = NapiUtils::CreateObject(env);
    std::vector<INetAddr> addresses = sysVpnConfig->addresses_;
    if (!addresses.empty()) {
        napi_value linkAddresses = NapiUtils::CreateArray(env, 1);
        napi_value netAddr = NapiUtils::CreateObject(env);
        NapiUtils::SetStringPropertyUtf8(env, netAddr, NET_ADDRESS, addresses[0].address_);
        napi_value linkAddr = NapiUtils::CreateObject(env);
        NapiUtils::SetNamedProperty(env, linkAddr, NET_ADDRESS, netAddr);
        NapiUtils::SetUint32Property(env, linkAddr, NET_PREFIXLENGTH, 1);
        NapiUtils::SetArrayElement(env, linkAddresses, 0, linkAddr);
        NapiUtils::SetNamedProperty(env, config, CONFIG_ADDRESSES, linkAddresses);
    }
    std::vector<std::string> dnsAddresses = sysVpnConfig->dnsAddresses_;
    if (!dnsAddresses.empty()) {
        napi_value dnsArray = NapiUtils::CreateArray(env, 1);
        NapiUtils::SetArrayElement(env, dnsArray, 0, NapiUtils::CreateStringUtf8(env, dnsAddresses[0]));
        NapiUtils::SetNamedProperty(env, config, CONFIG_DNSADDRESSES, dnsArray);
    }
    std::vector<std::string> searchDomains = sysVpnConfig->searchDomains_;
    if (!searchDomains.empty()) {
        napi_value domainsArray = NapiUtils::CreateArray(env, 1);
        NapiUtils::SetArrayElement(env, domainsArray, 0, NapiUtils::CreateStringUtf8(env, searchDomains[0]));
        NapiUtils::SetNamedProperty(env, config, CONFIG_SEARCHDOMAINS, domainsArray);
    }
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_VPN_ID, sysVpnConfig->vpnId_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_VPN_NAME, sysVpnConfig->vpnName_);
    NapiUtils::SetInt32Property(env, config, CONFIG_VPN_TYPE, sysVpnConfig->vpnType_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_USER_NAME, sysVpnConfig->userName_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_PASSWORD, sysVpnConfig->password_);
    NapiUtils::SetBooleanProperty(env, config, CONFIG_SAVE_LOGIN, sysVpnConfig->saveLogin_ == 0 ? false : true);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_FORWARDED_ROUTES, sysVpnConfig->forwardingRoutes_);
    return config;
}

napi_value CreateNapiIpsecVpnConfig(napi_env env, sptr<SysVpnConfig> &sysVpnConfig)
{
    napi_value config = CreateNapiSysVpnConfig(env, sysVpnConfig);
    sptr<IpsecVpnConfig> ipsecVpnConfig = sptr<IpsecVpnConfig>(static_cast<IpsecVpnConfig*>(sysVpnConfig.GetRefPtr()));
    if (!ipsecVpnConfig) {
        return NapiUtils::GetUndefined(env);
    }
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_IPSEC_PRE_SHARE_KEY, ipsecVpnConfig->ipsecPreSharedKey_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_IPSEC_IDENTIFIER, ipsecVpnConfig->ipsecIdentifier_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_SWANCTL_CONF, ipsecVpnConfig->swanctlConf_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_STRONGSWAN_CONF, ipsecVpnConfig->strongswanConf_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_IPSEC_CA_CERT_CONF, ipsecVpnConfig->ipsecCaCertConf_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_IPSEC_PRIVATE_USER_CERT_CONF,
        ipsecVpnConfig->ipsecPrivateUserCertConf_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_IPSEC_PUBLIC_USER_CERT_CONF,
        ipsecVpnConfig->ipsecPublicUserCertConf_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_IPSEC_PRIVATE_SERVER_CERT_CONF,
        ipsecVpnConfig->ipsecPrivateServerCertConf_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_IPSEC_PUBLIC_SERVER_CERT_CONF,
        ipsecVpnConfig->ipsecPublicServerCertConf_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_IPSEC_CA_CERT_FILE_PATH,
        ipsecVpnConfig->ipsecCaCertFilePath_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_IPSEC_PRIVATE_USER_CERT_FILE_PATH,
        ipsecVpnConfig->ipsecPrivateUserCertFilePath_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_IPSEC_PUBLIC_USER_CERT_FILE_PATH,
        ipsecVpnConfig->ipsecPublicUserCertFilePath_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_IPSEC_PRIVATE_SERVER_CERT_FILE_PATH,
        ipsecVpnConfig->ipsecPrivateServerCertFilePath_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_IPSEC_PUBLIC_SERVER_CERT_FILE_PATH,
        ipsecVpnConfig->ipsecPublicServerCertFilePath_);
    return config;
}

napi_value CreateNapiL2tpVpnConfig(napi_env env, sptr<SysVpnConfig> &sysVpnConfig)
{
    napi_value config = CreateNapiSysVpnConfig(env, sysVpnConfig);
    sptr<L2tpVpnConfig> l2tpVpnConfig = sptr<L2tpVpnConfig>(static_cast<L2tpVpnConfig*>(sysVpnConfig.GetRefPtr()));
    if (!l2tpVpnConfig) {
        return NapiUtils::GetUndefined(env);
    }
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_IPSEC_PRE_SHARE_KEY, l2tpVpnConfig->ipsecPreSharedKey_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_IPSEC_IDENTIFIER, l2tpVpnConfig->ipsecIdentifier_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_STRONGSWAN_CONF, l2tpVpnConfig->strongswanConf_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_IPSEC_CA_CERT_CONF, l2tpVpnConfig->ipsecCaCertConf_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_IPSEC_PRIVATE_USER_CERT_CONF,
        l2tpVpnConfig->ipsecPrivateUserCertConf_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_IPSEC_PUBLIC_USER_CERT_CONF,
        l2tpVpnConfig->ipsecPublicUserCertConf_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_IPSEC_PRIVATE_SERVER_CERT_CONF,
        l2tpVpnConfig->ipsecPrivateServerCertConf_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_IPSEC_PUBLIC_SERVER_CERT_CONF,
        l2tpVpnConfig->ipsecPublicServerCertConf_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_IPSEC_CA_CERT_FILE_PATH,
        l2tpVpnConfig->ipsecCaCertFilePath_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_IPSEC_PRIVATE_USER_CERT_FILE_PATH,
        l2tpVpnConfig->ipsecPrivateUserCertFilePath_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_IPSEC_PUBLIC_USER_CERT_FILE_PATH,
        l2tpVpnConfig->ipsecPublicUserCertFilePath_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_IPSEC_PRIVATE_SERVER_CERT_FILE_PATH,
        l2tpVpnConfig->ipsecPrivateServerCertFilePath_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_IPSEC_PUBLIC_SERVER_CERT_FILE_PATH,
        l2tpVpnConfig->ipsecPublicServerCertFilePath_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_IPSEC_CONF, l2tpVpnConfig->ipsecConf_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_IPSEC_SECRETS, l2tpVpnConfig->ipsecSecrets_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_OPTIONS_L2TPD_CLIENT, l2tpVpnConfig->optionsL2tpdClient_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_XL2TPD_CONF, l2tpVpnConfig->xl2tpdConf_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_L2TP_SHARED_KEY, l2tpVpnConfig->l2tpSharedKey_);
    return config;
}
}
} // namespace NetManagerStandard
} // namespace OHOS