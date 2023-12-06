/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "setup_context.h"

#include <new>
#include <string>
#include <vector>

#include "inet_addr.h"
#include "napi_utils.h"
#include "net_manager_constants.h"
#include "netmanager_base_common_utils.h"
#include "netmgr_ext_log_wrapper.h"
#include "route.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr int32_t NET_MASK_MAX_LENGTH = 32;
constexpr int32_t IPV6_NET_PREFIX_MAX_LENGTH = 128;
constexpr int32_t PARAM_JUST_OPTIONS = 1;
constexpr int32_t PARAM_OPTIONS_AND_CALLBACK = 2;
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
bool CheckParamsType(napi_env env, napi_value *params, size_t paramsCount)
{
    switch (paramsCount) {
        case PARAM_JUST_OPTIONS:
            return (NapiUtils::GetValueType(env, params[0]) == napi_object);
        case PARAM_OPTIONS_AND_CALLBACK:
            return ((NapiUtils::GetValueType(env, params[0]) == napi_object) &&
                    (NapiUtils::GetValueType(env, params[1]) == napi_function));
        default:
            return false;
    }
}

bool GetStringFromJsMandatoryItem(napi_env env, napi_value object, const std::string &key, std::string &value)
{
    if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, object, key)) != napi_string) {
        NETMGR_EXT_LOG_E("param [%{public}s] type is mismatch", key.c_str());
        return false;
    }
    value = NapiUtils::GetStringPropertyUtf8(env, object, key);
    NETMGR_EXT_LOG_I("%{public}s: %{public}s", key.c_str(), value.c_str());
    return (value.empty()) ? false : true;
}

void GetStringFromJsOptionItem(napi_env env, napi_value object, const std::string &key, std::string &value)
{
    if (NapiUtils::HasNamedProperty(env, object, key)) {
        if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, object, key)) == napi_string) {
            value = NapiUtils::GetStringPropertyUtf8(env, object, key);
            NETMGR_EXT_LOG_I("%{public}s: %{public}s", key.c_str(), value.c_str());
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
            NETMGR_EXT_LOG_I("%{public}s: %{public}d", key.c_str(), value);
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
            NETMGR_EXT_LOG_I("%{public}s: %{public}d", key.c_str(), value);
        } else {
            NETMGR_EXT_LOG_E("param [%{public}s] type is mismatch", key.c_str());
        }
    }
}
} // namespace

SetUpContext::SetUpContext(napi_env env, EventManager *manager) : BaseContext(env, manager) {}

void SetUpContext::ParseParams(napi_value *params, size_t paramsCount)
{
    if (!CheckParamsType(GetEnv(), params, paramsCount)) {
        NETMGR_EXT_LOG_E("params type is mismatch");
        SetNeedThrowException(true);
        SetErrorCode(NETMANAGER_EXT_ERR_PARAMETER_ERROR);
        return;
    }
    if (!ParseVpnConfig(params)) {
        NETMGR_EXT_LOG_E("parse vpn config from js failed");
        SetNeedThrowException(true);
        SetErrorCode(NETMANAGER_EXT_ERR_PARAMETER_ERROR);
        return;
    }
    if (paramsCount == PARAM_OPTIONS_AND_CALLBACK) {
        SetParseOK(SetCallback(params[1]) == napi_ok);
        return;
    }
    SetParseOK(true);
}

bool SetUpContext::ParseVpnConfig(napi_value *params)
{
    vpnConfig_ = new (std::nothrow) VpnConfig();
    if (vpnConfig_ == nullptr) {
        NETMGR_EXT_LOG_E("vpnConfig is nullptr");
        return false;
    }
    if (!ParseAddrRouteParams(params[0]) || !ParseChoiceableParams(params[0])) {
        return false;
    }
    return true;
}

static bool ParseAddress(napi_env env, napi_value address, struct INetAddr &iNetAddr)
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
        NETMGR_EXT_LOG_E("param [%{public}s] type is mismatch", NET_PREFIXLENGTH);
        return false;
    }
    if (!isIpv6) {
        iNetAddr.prefixlen_ = static_cast<uint8_t>(NapiUtils::GetUint32Property(env, address, NET_PREFIXLENGTH));
    } else {
        iNetAddr.prefixlen_ = CommonUtils::Ipv6PrefixLen(iNetAddr.address_);
    }

    NETMGR_EXT_LOG_I("isIpv6:%{public}d, %{public}s: %{public}d", isIpv6, NET_PREFIXLENGTH, iNetAddr.prefixlen_);

    uint32_t prefix = iNetAddr.prefixlen_;
    if (prefix == 0 || prefix >= (isIpv6 ? IPV6_NET_PREFIX_MAX_LENGTH : NET_MASK_MAX_LENGTH)) {
        NETMGR_EXT_LOG_E("prefix: %{public}d error", prefix);
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

static bool ParseDestination(napi_env env, napi_value jsRoute, struct INetAddr &iNetAddr)
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

static bool ParseGateway(napi_env env, napi_value jsRoute, struct INetAddr &iNetAddr)
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

static bool ParseRoute(napi_env env, napi_value jsRoute, Route &route)
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

bool SetUpContext::ParseAddrRouteParams(napi_value config)
{
    // parse addresses.
    if (NapiUtils::HasNamedProperty(GetEnv(), config, CONFIG_ADDRESSES)) {
        napi_value addrArray = NapiUtils::GetNamedProperty(GetEnv(), config, CONFIG_ADDRESSES);
        if (!NapiUtils::IsArray(GetEnv(), addrArray)) {
            NETMGR_EXT_LOG_E("addresses is not array");
            return false;
        }
        uint32_t addrLength = NapiUtils::GetArrayLength(GetEnv(), addrArray);
        for (uint32_t i = 0; i < addrLength; ++i) { // set length limit.
            INetAddr iNetAddr;
            if (!ParseAddress(GetEnv(), NapiUtils::GetArrayElement(GetEnv(), addrArray, i), iNetAddr)) {
                NETMGR_EXT_LOG_E("ParseAddress failed");
                return false;
            }
            vpnConfig_->addresses_.emplace_back(iNetAddr);
            bool isIpv6 = CommonUtils::IsValidIPV6(iNetAddr.address_);
            vpnConfig_->isAcceptIPv4_ = !isIpv6;
            vpnConfig_->isAcceptIPv6_ = isIpv6;
        }
    }

    // parse routes.
    if (NapiUtils::HasNamedProperty(GetEnv(), config, CONFIG_ROUTES)) {
        napi_value routes = NapiUtils::GetNamedProperty(GetEnv(), config, CONFIG_ROUTES);
        if (!NapiUtils::IsArray(GetEnv(), routes)) {
            NETMGR_EXT_LOG_E("routes is not array");
            return false;
        }
        uint32_t routesLength = NapiUtils::GetArrayLength(GetEnv(), routes);
        for (uint32_t idx = 0; idx < routesLength; ++idx) { // set length limit.
            struct Route routeInfo;
            if (!ParseRoute(GetEnv(), NapiUtils::GetArrayElement(GetEnv(), routes, idx), routeInfo)) {
                NETMGR_EXT_LOG_E("ParseRoute failed");
                return false;
            }
            vpnConfig_->routes_.emplace_back(routeInfo);
        }
    }
    return true;
}

static bool ParseOptionArrayString(napi_env env, napi_value config, const std::string &key,
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
            NETMGR_EXT_LOG_D("%{public}s: %{public}s", key.c_str(), item.c_str());
            vector.push_back(item);
        }
    }
    return true;
}

bool SetUpContext::ParseChoiceableParams(napi_value config)
{
    ParseOptionArrayString(GetEnv(), config, CONFIG_DNSADDRESSES, vpnConfig_->dnsAddresses_);
    ParseOptionArrayString(GetEnv(), config, CONFIG_SEARCHDOMAINS, vpnConfig_->searchDomains_);

    if (NapiUtils::HasNamedProperty(GetEnv(), config, CONFIG_MTU)) {
        if (NapiUtils::GetValueType(GetEnv(), NapiUtils::GetNamedProperty(GetEnv(), config, CONFIG_MTU)) ==
            napi_number) {
            vpnConfig_->mtu_ = NapiUtils::GetInt32Property(GetEnv(), config, CONFIG_MTU);
            NETMGR_EXT_LOG_I("%{public}s: %{public}d", CONFIG_MTU, vpnConfig_->mtu_);
        } else {
            NETMGR_EXT_LOG_E("param [%{public}s] type is mismatch", CONFIG_MTU);
        }
    }

    GetBoolFromJsOptionItem(GetEnv(), config, CONFIG_ISIPV4ACCEPTED, vpnConfig_->isAcceptIPv4_);
    GetBoolFromJsOptionItem(GetEnv(), config, CONFIG_ISIPV6ACCEPTED, vpnConfig_->isAcceptIPv6_);
    GetBoolFromJsOptionItem(GetEnv(), config, CONFIG_ISLEGACY, vpnConfig_->isLegacy_);
    GetBoolFromJsOptionItem(GetEnv(), config, CONFIG_ISMETERED, vpnConfig_->isMetered_);
    GetBoolFromJsOptionItem(GetEnv(), config, CONFIG_ISBLOCKING, vpnConfig_->isBlocking_);

    ParseOptionArrayString(GetEnv(), config, CONFIG_TRUSTEDAPPLICATIONS, vpnConfig_->acceptedApplications_);
    ParseOptionArrayString(GetEnv(), config, CONFIG_BLOCKEDAPPLICATIONS, vpnConfig_->refusedApplications_);
    return true;
}
} // namespace NetManagerStandard
} // namespace OHOS
