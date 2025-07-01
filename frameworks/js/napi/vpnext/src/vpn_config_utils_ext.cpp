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

#include "vpn_config_utils_ext.h"

#include "napi_utils.h"
#include "inet_addr.h"
#include "net_manager_constants.h"
#include "netmanager_base_common_utils.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
namespace VpnConfigUtilsExt {
bool ParseSysVpnConfig(napi_env env, napi_value *params, sptr<SysVpnConfig> &vpnConfig)
{
    if (params == nullptr) {
        NETMGR_EXT_LOG_E("ParseSysVpnConfig failed, params is null");
        return false;
    }
    int vpnType = -1;
    GetInt32FromJsOptionItem(env, params[0], CONFIG_VPN_TYPE, vpnType);
    switch (vpnType) {
        case VpnType::IKEV2_IPSEC_MSCHAPv2:
        case VpnType::IKEV2_IPSEC_PSK:
        case VpnType::IKEV2_IPSEC_RSA: {
            vpnConfig = CreateAndParseIpsecVpnConf(env, params[0]);
            if (vpnConfig == nullptr) {
                NETMGR_EXT_LOG_E("CreateAndParseIpsecVpnConf failed, vpnConfig is null");
                return false;
            }
            break;
        }
        case VpnType::L2TP:
        case VpnType::L2TP_IPSEC_PSK:
        case VpnType::L2TP_IPSEC_RSA: {
            vpnConfig = CreateAndParseL2tpVpnConf(env, params[0]);
            if (vpnConfig == nullptr) {
                NETMGR_EXT_LOG_E("CreateAndParseL2tpVpnConf failed, vpnConfig is null");
                return false;
            }
            break;
        }
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

bool ParseSystemVpnParams(napi_env env, napi_value config, sptr<SysVpnConfig> sysVpnConfig)
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
    if (sysVpnConfig->vpnType_ == VpnType::IKEV2_IPSEC_MSCHAPv2 ||
        sysVpnConfig->vpnType_ == VpnType::IKEV2_IPSEC_RSA ||
        sysVpnConfig->vpnType_ == VpnType::L2TP_IPSEC_RSA) {
        GetStringFromJsOptionItem(env, config, CONFIG_PK12_PASSWORD, sysVpnConfig->pkcs12Password_);
        GetU8VectorFromJsOptionItem(env, config, CONFIG_PK12_FILE_DATA, sysVpnConfig->pkcs12FileData_);
    }
    return true;
}

sptr<IpsecVpnConfig> CreateAndParseIpsecVpnConf(napi_env env, napi_value config)
{
    sptr<IpsecVpnConfig> ipsecVpnConfig = new (std::nothrow) IpsecVpnConfig();
    if (ipsecVpnConfig == nullptr) {
        NETMGR_EXT_LOG_E("create ipsecVpnConfig failed, is null");
        return nullptr;
    }
    if (!ParseSystemVpnParams(env, config, ipsecVpnConfig)) {
        NETMGR_EXT_LOG_E("ParseSystemVpnParams failed");
        return nullptr;
    }
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
    return ipsecVpnConfig;
}

sptr<L2tpVpnConfig> CreateAndParseL2tpVpnConf(napi_env env, napi_value config)
{
    sptr<L2tpVpnConfig> l2tpVpnConfig = new (std::nothrow) L2tpVpnConfig();
    if (l2tpVpnConfig == nullptr) {
        NETMGR_EXT_LOG_E("l2tpVpnConfig is null");
        return nullptr;
    }
    if (!ParseSystemVpnParams(env, config, l2tpVpnConfig)) {
        NETMGR_EXT_LOG_E("ParseSystemVpnParams failed");
        return nullptr;
    }

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
    return l2tpVpnConfig;
}

sptr<OpenvpnConfig> CreateAndParseOpenvpnConf(napi_env env, napi_value config)
{
    sptr<OpenvpnConfig> openvpnConfig = new (std::nothrow) OpenvpnConfig();
    if (openvpnConfig == nullptr) {
        NETMGR_EXT_LOG_E("openvpnConfig is null");
        return nullptr;
    }
    if (!ParseSystemVpnParams(env, config, openvpnConfig)) {
        NETMGR_EXT_LOG_E("ParseSystemVpnParams failed");
        return nullptr;
    }
    GetStringFromJsOptionItem(env, config, CONFIG_OVPN_PORT, openvpnConfig->ovpnPort_);
    GetInt32FromJsOptionItem(env, config, CONFIG_OPENVPN_PROTOCOL, openvpnConfig->ovpnProtocol_);
    GetStringFromJsOptionItem(env, config, CONFIG_OPENVPN_CFG, openvpnConfig->ovpnConfig_);
    GetInt32FromJsOptionItem(env, config, CONFIG_OPENVPN_AUTH_TYPE, openvpnConfig->ovpnAuthType_);
    GetStringFromJsOptionItem(env, config, CONFIG_ASKPASS, openvpnConfig->askpass_);
    GetStringFromJsOptionItem(env, config, CONFIG_OPENVPN_CFG_FILE_PATH, openvpnConfig->ovpnConfigFilePath_);
    GetStringFromJsOptionItem(env, config, CONFIG_OPENVPN_CA_CERT_FILE_PATH, openvpnConfig->ovpnCaCertFilePath_);
    GetStringFromJsOptionItem(env, config, CONFIG_OPENVPN_USER_CERT_FILE_PATH, openvpnConfig->ovpnUserCertFilePath_);
    GetStringFromJsOptionItem(env, config, CONFIG_OPENVPN_PRIVATE_KEY_FILE_PATH,
        openvpnConfig->ovpnPrivateKeyFilePath_);
    return openvpnConfig;
}

bool GetU8VectorFromJsOptionItem(const napi_env env, const napi_value config, const std::string &key,
    std::vector<uint8_t> &value)
{
    bool hasProperty = NapiUtils::HasNamedProperty(env, config, key);
    if (!hasProperty) {
        NETMGR_EXT_LOG_E("JsObjectToU8Vector, Js to U8Vector no property: %{public}s", key.c_str());
        return false;
    }
    napi_value array = NapiUtils::GetNamedProperty(env, config, key);

    bool isTypedArray = false;
    if (napi_is_typedarray(env, array, &isTypedArray) != napi_ok || !isTypedArray) {
        NETMGR_EXT_LOG_E("JsObjectToU8Vector, property is not typedarray: %{public}s", key.c_str());
        return false;
    }

    size_t length = 0;
    size_t offset = 0;
    napi_typedarray_type type;
    napi_value buffer = nullptr;
    NAPI_CALL_BASE(env, napi_get_typedarray_info(env, array, &type, &length, nullptr, &buffer, &offset), {});
    if (type != napi_uint8_array || buffer == nullptr) {
        NETMGR_EXT_LOG_E("JsObjectToU8Vector, %{public}s, buffer is nullptr: %{public}d",
            key.c_str(), (int)(buffer == nullptr));
        return false;
    }

    size_t total = 0;
    uint8_t *data = nullptr;
    NAPI_CALL_BASE(env, napi_get_arraybuffer_info(env, buffer, reinterpret_cast<void **>(&data), &total), {});
    length = std::min<size_t>(length, total - offset);
    value.resize(length);
    int retCode = memcpy_s(value.data(), value.size(), &data[offset], length);
    if (retCode != EOK) {
        NETMGR_EXT_LOG_E("JsObjectToU8Vector, memcpy_s return fail: %{public}d", retCode);
        return false;
    }
    return true;
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
            NETMGR_EXT_LOG_W("invalid ip address, might be a domain");
            GetUint8FromJsOptionItem(env, netAddress, NET_PORT, iNetAddr.port_);
            return true;
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
        NETMGR_EXT_LOG_E("CreateNapiVpnConfig failed, param is null");
        return NapiUtils::GetUndefined(env);
    }
    switch (sysVpnConfig->vpnType_) {
        case VpnType::IKEV2_IPSEC_MSCHAPv2:
        case VpnType::IKEV2_IPSEC_PSK:
        case VpnType::IKEV2_IPSEC_RSA:
        case VpnType::IPSEC_XAUTH_PSK:
        case VpnType::IPSEC_XAUTH_RSA:
        case VpnType::IPSEC_HYBRID_RSA:
            return CreateNapiIpsecVpnConfig(env, sysVpnConfig);
        case VpnType::OPENVPN:
            return CreateNapiOpenvpnConfig(env, sysVpnConfig);
        case VpnType::L2TP:
        case VpnType::L2TP_IPSEC_PSK:
        case VpnType::L2TP_IPSEC_RSA:
            return CreateNapiL2tpVpnConfig(env, sysVpnConfig);
        default:
            NETMGR_EXT_LOG_E("CreateNapiVpnConfig failed, invalid type %{public}d", sysVpnConfig->vpnType_);
            return NapiUtils::GetUndefined(env);
    }
}

napi_value CreateNapiSysVpnConfig(napi_env env, sptr<SysVpnConfig> &sysVpnConfig)
{
    if (sysVpnConfig == nullptr) {
        NETMGR_EXT_LOG_E("CreateNapiSysVpnConfig failed, param is null");
        return NapiUtils::GetUndefined(env);
    }
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
    if (sysVpnConfig == nullptr) {
        NETMGR_EXT_LOG_E("CreateNapiIpsecVpnConfig failed, param is null");
        return NapiUtils::GetUndefined(env);
    }
    napi_value config = CreateNapiSysVpnConfig(env, sysVpnConfig);

    IpsecVpnConfig *ipsecVpnConfig = static_cast<IpsecVpnConfig *>(sysVpnConfig.GetRefPtr());
    if (ipsecVpnConfig == nullptr) {
        NETMGR_EXT_LOG_E("CreateNapiIpsecVpnConfig failed, ipsecVpnConfig is null");
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
    ipsecVpnConfig = nullptr;
    return config;
}

napi_value CreateNapiL2tpVpnConfig(napi_env env, sptr<SysVpnConfig> &sysVpnConfig)
{
    if (sysVpnConfig == nullptr) {
        NETMGR_EXT_LOG_E("CreateNapiL2tpVpnConfig failed, param is null");
        return NapiUtils::GetUndefined(env);
    }
    napi_value config = CreateNapiSysVpnConfig(env, sysVpnConfig);

    L2tpVpnConfig *l2tpVpnConfig = static_cast<L2tpVpnConfig *>(sysVpnConfig.GetRefPtr());
    if (l2tpVpnConfig == nullptr) {
        NETMGR_EXT_LOG_E("CreateNapiL2tpVpnConfig failed, l2tpVpnConfig is null");
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
    l2tpVpnConfig = nullptr;
    return config;
}

napi_value CreateNapiOpenvpnConfig(napi_env env, sptr<SysVpnConfig> sysVpnConfig)
{
    if (sysVpnConfig == nullptr) {
        NETMGR_EXT_LOG_E("CreateNapiOpenvpnConfig failed, param is null");
        return NapiUtils::GetUndefined(env);
    }
    napi_value config = CreateNapiSysVpnConfig(env, sysVpnConfig);

    OpenvpnConfig *openvpnConfig = static_cast<OpenvpnConfig *>(sysVpnConfig.GetRefPtr());
    if (openvpnConfig == nullptr) {
        NETMGR_EXT_LOG_E("CreateNapiOpenvpnConfig failed, openvpnConfig is null");
        return NapiUtils::GetUndefined(env);
    }
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_OVPN_PORT, openvpnConfig->ovpnPort_);
    NapiUtils::SetInt32Property(env, config, CONFIG_OPENVPN_PROTOCOL, openvpnConfig->ovpnProtocol_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_OPENVPN_CFG, openvpnConfig->ovpnConfig_);
    NapiUtils::SetInt32Property(env, config, CONFIG_OPENVPN_AUTH_TYPE, openvpnConfig->ovpnAuthType_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_ASKPASS, openvpnConfig->askpass_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_OPENVPN_CFG_FILE_PATH, openvpnConfig->ovpnConfigFilePath_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_OPENVPN_CA_CERT_FILE_PATH,
        openvpnConfig->ovpnCaCertFilePath_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_OPENVPN_USER_CERT_FILE_PATH,
        openvpnConfig->ovpnUserCertFilePath_);
    NapiUtils::SetStringPropertyUtf8(env, config, CONFIG_OPENVPN_PRIVATE_KEY_FILE_PATH,
        openvpnConfig->ovpnPrivateKeyFilePath_);
    openvpnConfig = nullptr;
    return config;
}
}
} // namespace NetManagerStandard
} // namespace OHOS
