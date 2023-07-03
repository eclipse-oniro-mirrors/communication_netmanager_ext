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
#include "netmanager_ext_log.h"
#include "route.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
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
} // namespace

SetUpContext::SetUpContext(napi_env env, EventManager *manager) : BaseContext(env, manager) {}

void SetUpContext::ParseParams(napi_value *params, size_t paramsCount)
{
    if (!CheckParamsType(GetEnv(), params, paramsCount)) {
        NETMANAGER_EXT_LOGE("params type failed");
        SetNeedThrowException(true);
        SetErrorCode(NETMANAGER_EXT_ERR_PARAMETER_ERROR);
        return;
    }
    if (!ParseVpnConfig(params)) {
        NETMANAGER_EXT_LOGE("params failed");
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
        NETMANAGER_EXT_LOGE("vpnConfig is nullptr");
        return false;
    }
    if (!ParseAddrRouteParams(params[0]) || !ParseChoiceableParams(params[0])) {
        return false;
    }
    return true;
}

static struct INetAddr ParseAddress(napi_env env, napi_value address)
{
    struct INetAddr iNetAddr;
    napi_value netAddress = NapiUtils::GetNamedProperty(env, address, NET_ADDRESS);
    iNetAddr.address_ = NapiUtils::GetStringPropertyUtf8(env, netAddress, NET_ADDRESS);
    if (NapiUtils::HasNamedProperty(env, netAddress, NET_FAMILY)) {
        iNetAddr.family_ = static_cast<uint8_t>(NapiUtils::GetUint32Property(env, netAddress, NET_FAMILY));
    }
    if (NapiUtils::HasNamedProperty(env, netAddress, NET_PORT)) {
        iNetAddr.port_ = static_cast<uint8_t>(NapiUtils::GetUint32Property(env, netAddress, NET_PORT));
    }
    return iNetAddr;
}

static struct INetAddr ParseDestination(napi_env env, napi_value destination)
{
    struct INetAddr iNetAddr;
    napi_value netAddress = NapiUtils::GetNamedProperty(env, destination, NET_ADDRESS);
    iNetAddr.address_ = NapiUtils::GetStringPropertyUtf8(env, netAddress, NET_ADDRESS);
    if (NapiUtils::HasNamedProperty(env, netAddress, NET_FAMILY)) {
        iNetAddr.family_ = static_cast<uint8_t>(NapiUtils::GetUint32Property(env, netAddress, NET_FAMILY));
    }
    if (NapiUtils::HasNamedProperty(env, netAddress, NET_PORT)) {
        iNetAddr.port_ = static_cast<uint8_t>(NapiUtils::GetUint32Property(env, netAddress, NET_PORT));
    }
    iNetAddr.prefixlen_ = static_cast<uint8_t>(NapiUtils::GetUint32Property(env, destination, NET_PREFIXLENGTH));
    return iNetAddr;
}

static struct INetAddr ParseGateway(napi_env env, napi_value gateway)
{
    struct INetAddr iNetAddr;
    iNetAddr.address_ = NapiUtils::GetStringPropertyUtf8(env, gateway, NET_ADDRESS);
    if (NapiUtils::HasNamedProperty(env, gateway, NET_FAMILY)) {
        iNetAddr.family_ = static_cast<uint8_t>(NapiUtils::GetUint32Property(env, gateway, NET_FAMILY));
    }
    if (NapiUtils::HasNamedProperty(env, gateway, NET_PORT)) {
        iNetAddr.port_ = static_cast<uint8_t>(NapiUtils::GetUint32Property(env, gateway, NET_PORT));
    }
    return iNetAddr;
}

static struct Route ParseRoute(napi_env env, napi_value jsRoute)
{
    struct Route route;
    if (NapiUtils::HasNamedProperty(env, jsRoute, NET_INTERFACE)) {
        route.iface_ = NapiUtils::GetStringPropertyUtf8(env, jsRoute, NET_INTERFACE);
    }

    route.destination_ = ParseDestination(env, NapiUtils::GetNamedProperty(env, jsRoute, NET_DESTINATION));
    route.gateway_ = ParseGateway(env, NapiUtils::GetNamedProperty(env, jsRoute, NET_GATEWAY));

    if (NapiUtils::HasNamedProperty(env, jsRoute, NET_HAS_GATEWAY)) {
        route.hasGateway_ = NapiUtils::GetBooleanProperty(env, jsRoute, NET_HAS_GATEWAY);
    }
    if (NapiUtils::HasNamedProperty(env, jsRoute, NET_ISDEFAULTROUTE)) {
        route.isDefaultRoute_ = NapiUtils::GetBooleanProperty(env, jsRoute, NET_ISDEFAULTROUTE);
    }
    return route;
}

bool SetUpContext::ParseAddrRouteParams(napi_value config)
{
    // parse addresses.
    if (NapiUtils::HasNamedProperty(GetEnv(), config, CONFIG_ADDRESSES)) {
        napi_value addresses = NapiUtils::GetNamedProperty(GetEnv(), config, CONFIG_ADDRESSES);
        if (!NapiUtils::IsArray(GetEnv(), addresses)) {
            NETMANAGER_EXT_LOGE("addresses is not array");
            return false;
        }
        uint32_t addressesLength = NapiUtils::GetArrayLength(GetEnv(), addresses);
        for (uint32_t i = 0; i < addressesLength; ++i) { // set length limit.
            napi_value address = NapiUtils::GetArrayElement(GetEnv(), addresses, i);
            INetAddr iNetAddr = ParseAddress(GetEnv(), address);
            iNetAddr.prefixlen_ =
                static_cast<uint8_t>(NapiUtils::GetUint32Property(GetEnv(), address, NET_PREFIXLENGTH));
            vpnConfig_->addresses_.emplace_back(iNetAddr);
        }
    }

    // parse routes.
    if (NapiUtils::HasNamedProperty(GetEnv(), config, CONFIG_ROUTES)) {
        napi_value routes = NapiUtils::GetNamedProperty(GetEnv(), config, CONFIG_ROUTES);
        if (!NapiUtils::IsArray(GetEnv(), routes)) {
            NETMANAGER_EXT_LOGE("routes is not array");
            return false;
        }
        uint32_t routesLength = NapiUtils::GetArrayLength(GetEnv(), routes);
        for (uint32_t idx = 0; idx < routesLength; ++idx) { // set length limit.
            napi_value route = NapiUtils::GetArrayElement(GetEnv(), routes, idx);
            Route routeInfo = ParseRoute(GetEnv(), route);
            vpnConfig_->routes_.emplace_back(routeInfo);
        }
    }
    return true;
}

static std::vector<std::string> ParseArrayString(napi_env env, napi_value array)
{
    if (!NapiUtils::IsArray(env, array)) {
        return {};
    }
    std::vector<std::string> arrayString;
    uint32_t arrayLength = NapiUtils::GetArrayLength(env, array);
    for (uint32_t i = 0; i < arrayLength; ++i) {
        arrayString.push_back(NapiUtils::GetStringFromValueUtf8(env, NapiUtils::GetArrayElement(env, array, i)));
    }
    return arrayString;
}

bool SetUpContext::ParseChoiceableParams(napi_value config)
{
    NETMANAGER_EXT_LOGI("choiceable");
    if (NapiUtils::HasNamedProperty(GetEnv(), config, CONFIG_DNSADDRESSES)) {
        vpnConfig_->dnsAddresses_ =
            ParseArrayString(GetEnv(), NapiUtils::GetNamedProperty(GetEnv(), config, CONFIG_DNSADDRESSES));
    }
    if (NapiUtils::HasNamedProperty(GetEnv(), config, CONFIG_SEARCHDOMAINS)) {
        vpnConfig_->searchDomains_ =
            ParseArrayString(GetEnv(), NapiUtils::GetNamedProperty(GetEnv(), config, CONFIG_SEARCHDOMAINS));
    }
    if (NapiUtils::HasNamedProperty(GetEnv(), config, CONFIG_MTU)) {
        vpnConfig_->mtu_ = NapiUtils::GetInt32Property(GetEnv(), config, CONFIG_MTU);
    }
    if (NapiUtils::HasNamedProperty(GetEnv(), config, CONFIG_ISIPV4ACCEPTED)) {
        vpnConfig_->isAcceptIPv4_ = NapiUtils::GetBooleanProperty(GetEnv(), config, CONFIG_ISIPV4ACCEPTED);
    }
    if (NapiUtils::HasNamedProperty(GetEnv(), config, CONFIG_ISIPV4ACCEPTED)) {
        vpnConfig_->isAcceptIPv6_ = NapiUtils::GetBooleanProperty(GetEnv(), config, CONFIG_ISIPV6ACCEPTED);
    }
    if (NapiUtils::HasNamedProperty(GetEnv(), config, CONFIG_ISLEGACY)) {
        vpnConfig_->isLegacy_ = NapiUtils::GetBooleanProperty(GetEnv(), config, CONFIG_ISLEGACY);
    }
    if (NapiUtils::HasNamedProperty(GetEnv(), config, CONFIG_ISMETERED)) {
        vpnConfig_->isMetered_ = NapiUtils::GetBooleanProperty(GetEnv(), config, CONFIG_ISMETERED);
    }
    if (NapiUtils::HasNamedProperty(GetEnv(), config, CONFIG_ISBLOCKING)) {
        vpnConfig_->isBlocking_ = NapiUtils::GetBooleanProperty(GetEnv(), config, CONFIG_ISBLOCKING);
    }
    if (NapiUtils::HasNamedProperty(GetEnv(), config, CONFIG_TRUSTEDAPPLICATIONS)) {
        vpnConfig_->acceptedApplications_ =
            ParseArrayString(GetEnv(), NapiUtils::GetNamedProperty(GetEnv(), config, CONFIG_TRUSTEDAPPLICATIONS));
    }
    if (NapiUtils::HasNamedProperty(GetEnv(), config, CONFIG_BLOCKEDAPPLICATIONS)) {
        vpnConfig_->refusedApplications_ =
            ParseArrayString(GetEnv(), NapiUtils::GetNamedProperty(GetEnv(), config, CONFIG_BLOCKEDAPPLICATIONS));
    }
    return true;
}
} // namespace NetManagerStandard
} // namespace OHOS