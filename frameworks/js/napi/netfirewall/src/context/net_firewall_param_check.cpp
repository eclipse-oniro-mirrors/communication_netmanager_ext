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

#include <regex>

#include "napi_utils.h"
#include "net_firewall_param_check.h"
#include "net_manager_constants.h"
#include "netmanager_base_common_utils.h"
#include "netmanager_ext_log.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t MAX_RULE_PORT = 65535;
const std::regex DOMAIN_PATTERN { "^([A-Za-z0-9-\\.]{1,63})(\\.)([A-Za-z0-9-]{1,63}){0,}$" };
const std::regex WILDCARD_DOMAIN_PATTERN { "^(\\*)(\\.)([A-Za-z0-9-]{1,63})(\\.)([A-Za-z0-9-\\.]{1,63}){0,}$" };

int32_t NetFirewallParamCheck::CheckFirewallRulePolicy(napi_env env, napi_value object)
{
    if (!NapiUtils::HasNamedProperty(env, object, NET_FIREWALL_IS_OPEN) ||
        !NapiUtils::HasNamedProperty(env, object, NET_FIREWALL_IN_ACTION) ||
        !NapiUtils::HasNamedProperty(env, object, NET_FIREWALL_OUT_ACTION)) {
        NETMANAGER_EXT_LOGE("params can not be empty.");
        return FIREWALL_ERR_PARAMETER_ERROR;
    }

    if (NapiUtils::HasNamedProperty(env, object, NET_FIREWALL_IS_OPEN)) {
        if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_IS_OPEN)) !=
            napi_boolean) {
            NETMANAGER_EXT_LOGE("params state type invalid");
            return FIREWALL_ERR_PARAMETER_ERROR;
        }
    }

    if (NapiUtils::HasNamedProperty(env, object, NET_FIREWALL_IN_ACTION)) {
        if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_IN_ACTION)) !=
            napi_number) {
            NETMANAGER_EXT_LOGE("params inAction type invalid");
            return FIREWALL_ERR_PARAMETER_ERROR;
        }
        FirewallRuleAction inAction =
            static_cast<FirewallRuleAction>(NapiUtils::GetInt32Property(env, object, NET_FIREWALL_IN_ACTION));
        if (inAction != FirewallRuleAction::RULE_ALLOW && inAction != FirewallRuleAction::RULE_DENY) {
            NETMANAGER_EXT_LOGE("params inAction invalid");
            return FIREWALL_ERR_INVALID_PARAMETER;
        }
    }

    if (NapiUtils::HasNamedProperty(env, object, NET_FIREWALL_OUT_ACTION)) {
        if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_OUT_ACTION)) !=
            napi_number) {
            NETMANAGER_EXT_LOGE("params outAction type invalid");
            return FIREWALL_ERR_PARAMETER_ERROR;
        }
        FirewallRuleAction outAction =
            static_cast<FirewallRuleAction>(NapiUtils::GetInt32Property(env, object, NET_FIREWALL_OUT_ACTION));
        if (outAction != FirewallRuleAction::RULE_ALLOW && outAction != FirewallRuleAction::RULE_DENY) {
            NETMANAGER_EXT_LOGE("params outAction invalid");
            return FIREWALL_ERR_INVALID_PARAMETER;
        }
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallParamCheck::CheckFirewallDns(napi_env env, napi_value object)
{
    if (NapiUtils::HasNamedProperty(env, object, NET_FIREWALL_DNS_PRIMARY)) {
        if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_DNS_PRIMARY)) !=
            napi_string) {
            NETMANAGER_EXT_LOGE("params dns type invalid");
            return FIREWALL_ERR_PARAMETER_ERROR;
        }
        std::string primaryDns =
            NapiUtils::GetStringFromValueUtf8(env, NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_DNS_PRIMARY));
        int32_t ret = CheckIpAddress(primaryDns);
        if (ret != FIREWALL_SUCCESS) {
            return ret;
        }
    }

    if (NapiUtils::HasNamedProperty(env, object, NET_FIREWALL_DNS_STANDY)) {
        if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_DNS_STANDY)) !=
            napi_string) {
            NETMANAGER_EXT_LOGE("params dns type invalid");
            return FIREWALL_ERR_PARAMETER_ERROR;
        }
        std::string standyDns =
            NapiUtils::GetStringFromValueUtf8(env, NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_DNS_STANDY));
        int32_t ret = CheckIpAddress(standyDns);
        if (ret != FIREWALL_SUCCESS) {
            return ret;
        }
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallParamCheck::CheckFirewallAction(napi_env env, napi_value object)
{
    FirewallRuleAction action =
        static_cast<FirewallRuleAction>(NapiUtils::GetInt32Property(env, object, NET_FIREWALL_RULE_ACTION));
    if (action != FirewallRuleAction::RULE_ALLOW && action != FirewallRuleAction::RULE_DENY) {
        NETMANAGER_EXT_LOGE("params action invalid");
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallParamCheck::CheckFirewallDirection(napi_env env, napi_value object)
{
    NetFirewallRuleDirection direction =
        static_cast<NetFirewallRuleDirection>(NapiUtils::GetInt32Property(env, object, NET_FIREWALL_RULE_DIR));
    if (direction != NetFirewallRuleDirection::RULE_IN && direction != NetFirewallRuleDirection::RULE_OUT) {
        NETMANAGER_EXT_LOGE("params direction invalid");
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallParamCheck::CheckIpAddress(const std::string &ipStr)
{
    if (ipStr.size() == 0) {
        return FIREWALL_SUCCESS;
    }
    if (CheckIpV4(ipStr) != FIREWALL_SUCCESS && CheckIpV6(ipStr) != FIREWALL_SUCCESS) {
        NETMANAGER_EXT_LOGE("ip address invalid");
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallParamCheck::CheckIpAddress(const std::string &ipStr, const int32_t family)
{
    if ((family == FAMILY_IPV4 && CheckIpV4(ipStr) != FIREWALL_SUCCESS) ||
        (family == FAMILY_IPV6 && CheckIpV6(ipStr) != FIREWALL_SUCCESS)) {
        NETMANAGER_EXT_LOGE("ip and family invalid");
        return FIREWALL_ERR_INVALID_PARAMETER;
    }

    return FIREWALL_SUCCESS;
}

bool NetFirewallParamCheck::CheckIpAddress(const std::string &startIp, const std::string &endIp, const int32_t family)
{
    int32_t ret = 0;
    int32_t endRet = 0;
    if (family == FAMILY_IPV6) {
        in6_addr in6_addr1;
        in6_addr in6_addr2;
        // Convert IPv6 addresses to binary form
        ret = inet_pton(AF_INET6, startIp.c_str(), &in6_addr1);
        endRet = inet_pton(AF_INET6, endIp.c_str(), &in6_addr2);
        if (ret <= 0 || endRet <= 0) {
            NETMANAGER_EXT_LOGE("CheckIpAddress ipv6: startIp or endIp is invalid");
            return false;
        }
        // Comparing IPv6 addresses in binary form
        ret = memcmp(&in6_addr1, &in6_addr2, sizeof(in6_addr1));
        if (ret > 0) {
            NETMANAGER_EXT_LOGE("CheckIpAddress ipv6: start Ip is larger than endIp");
            return false;
        }
        return true;
    }

    in_addr inAddr1;
    in_addr inAddr2;
    ret = inet_pton(AF_INET, startIp.c_str(), &inAddr1);
    endRet = inet_pton(AF_INET, endIp.c_str(), &inAddr2);
    if (ret <= 0 || endRet <= 0) {
        NETMANAGER_EXT_LOGE("CheckIpAddress ipv4: startIp or endIp is invalid");
        return false;
    }
    ret = memcmp(&inAddr1, &inAddr2, sizeof(struct in_addr));
    if (ret > 0) {
        NETMANAGER_EXT_LOGE("CheckIpAddress ipv4: start Ip is larger than endIp");
        return false;
    }
    return true;
}

int32_t NetFirewallParamCheck::CheckIpV4(const std::string &ipV4)
{
    in_addr v4Addr;
    int32_t ret = inet_pton(AF_INET, ipV4.c_str(), &v4Addr);
    if (ret <= 0) {
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallParamCheck::CheckIpV6(const std::string &ipV6)
{
    in6_addr v6Addr;
    int32_t ret = inet_pton(AF_INET6, ipV6.c_str(), &v6Addr);
    if (ret <= 0) {
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallParamCheck::CheckSingeIp(napi_env env, napi_value valAttr, int32_t family)
{
    // Address must be filled if a single IP is used
    if (!NapiUtils::HasNamedProperty(env, valAttr, NET_FIREWALL_IP_ADDRESS)) {
        NETMANAGER_EXT_LOGE("params ip type1 but no address");
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, valAttr, NET_FIREWALL_IP_ADDRESS)) !=
        napi_string) {
        return FIREWALL_ERR_PARAMETER_ERROR;
    }
    std::string address =
        NapiUtils::GetStringFromValueUtf8(env, NapiUtils::GetNamedProperty(env, valAttr, NET_FIREWALL_IP_ADDRESS));
    int32_t ret = CheckIpAddress(address, family);
    if (ret != FIREWALL_SUCCESS) {
        return ret;
    }

    if (NapiUtils::HasNamedProperty(env, valAttr, NET_FIREWALL_IP_MASK)) {
        if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, valAttr, NET_FIREWALL_IP_MASK)) !=
            napi_number) {
            return FIREWALL_ERR_PARAMETER_ERROR;
        }
        int32_t mask = NapiUtils::GetInt32Property(env, valAttr, NET_FIREWALL_IP_MASK);
        int32_t maskMax = family == FAMILY_IPV4 ? IPV4_MASK_MAX : IPV6_MASK_MAX;
        if (mask <= 0 || mask > maskMax) {
            NETMANAGER_EXT_LOGE("params mask=%{public}d is invalid", mask);
            return FIREWALL_ERR_INVALID_PARAMETER;
        }
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallParamCheck::CheckMultipleIp(napi_env env, napi_value valAttr, int32_t family)
{
    // Starting address must be filled if the multiple IPs are used
    if (!NapiUtils::HasNamedProperty(env, valAttr, NET_FIREWALL_IP_START) ||
        !NapiUtils::HasNamedProperty(env, valAttr, NET_FIREWALL_IP_END)) {
        NETMANAGER_EXT_LOGE("params ip type2 but no startIp or endIp");
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, valAttr, NET_FIREWALL_IP_START)) != napi_string ||
        NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, valAttr, NET_FIREWALL_IP_END)) != napi_string) {
        return FIREWALL_ERR_PARAMETER_ERROR;
    }
    std::string startIp =
        NapiUtils::GetStringFromValueUtf8(env, NapiUtils::GetNamedProperty(env, valAttr, NET_FIREWALL_IP_START));
    std::string endIp =
        NapiUtils::GetStringFromValueUtf8(env, NapiUtils::GetNamedProperty(env, valAttr, NET_FIREWALL_IP_END));
    if (CheckIpAddress(startIp, endIp, family)) {
        return FIREWALL_SUCCESS;
    }
    return FIREWALL_ERR_INVALID_PARAMETER;
}

int32_t NetFirewallParamCheck::CheckIpList(napi_env env, napi_value object)
{
    if (!NapiUtils::IsArray(env, object)) {
        NETMANAGER_EXT_LOGE("ParseIpList get params failed");
        return FIREWALL_ERR_PARAMETER_ERROR;
    }
    uint32_t len = NapiUtils::GetArrayLength(env, object);
    for (size_t j = 0; j < len; j++) {
        napi_value valAttr = NapiUtils::GetArrayElement(env, object, j);
        if (NapiUtils::GetValueType(env, valAttr) != napi_object) {
            NETMANAGER_EXT_LOGI("valAttr is not napi_number");
            continue;
        }
        int32_t family = FAMILY_IPV4;
        if (NapiUtils::HasNamedProperty(env, valAttr, NET_FIREWALL_IP_FAMILY)) {
            if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, valAttr, NET_FIREWALL_IP_FAMILY)) !=
                napi_number) {
                return FIREWALL_ERR_PARAMETER_ERROR;
            }
            family = NapiUtils::GetInt32Property(env, valAttr, NET_FIREWALL_IP_FAMILY);
            if (family != FAMILY_IPV4 && family != FAMILY_IPV6) {
                NETMANAGER_EXT_LOGE("family param invalid.");
                return FIREWALL_ERR_INVALID_PARAMETER;
            }
        }

        int32_t type = SINGLE_IP;
        if (NapiUtils::HasNamedProperty(env, valAttr, NET_FIREWALL_IP_TYPE)) {
            if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, valAttr, NET_FIREWALL_IP_TYPE)) !=
                napi_number) {
                return FIREWALL_ERR_PARAMETER_ERROR;
            }
            type = NapiUtils::GetInt32Property(env, valAttr, NET_FIREWALL_IP_TYPE);
            if (type != SINGLE_IP && type != MULTIPLE_IP) {
                NETMANAGER_EXT_LOGE("params ip type invalid");
                return FIREWALL_ERR_INVALID_PARAMETER;
            }
        }
        int32_t ret = type == MULTIPLE_IP ? CheckMultipleIp(env, valAttr, family) : CheckSingeIp(env, valAttr, family);
        if (ret != FIREWALL_SUCCESS) {
            return ret;
        }
    }

    return FIREWALL_SUCCESS;
}

int32_t NetFirewallParamCheck::CheckPortList(napi_env env, napi_value object)
{
    if (!NapiUtils::IsArray(env, object)) {
        NETMANAGER_EXT_LOGE("ParsePortList get params failed");
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    uint32_t len = NapiUtils::GetArrayLength(env, object);
    for (size_t j = 0; j < len; j++) {
        napi_value valAttr = NapiUtils::GetArrayElement(env, object, j);
        if (NapiUtils::GetValueType(env, valAttr) != napi_object) {
            NETMANAGER_EXT_LOGI("valAttr is not napi_number");
            continue;
        }
        int32_t startPort = 0;
        if (NapiUtils::HasNamedProperty(env, valAttr, NET_FIREWALL_PORT_START)) {
            if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, valAttr, NET_FIREWALL_PORT_START)) !=
                napi_number) {
                return FIREWALL_ERR_PARAMETER_ERROR;
            }
            startPort = NapiUtils::GetInt32Property(env, valAttr, NET_FIREWALL_PORT_START);
            if (startPort > MAX_RULE_PORT || startPort < 0) {
                NETMANAGER_EXT_LOGE("start port is more then %{public}d", MAX_RULE_PORT);
                return FIREWALL_ERR_INVALID_PARAMETER;
            }
        }
        if (NapiUtils::HasNamedProperty(env, valAttr, NET_FIREWALL_PORT_END)) {
            int32_t endPort = 0;
            if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, valAttr, NET_FIREWALL_PORT_END)) !=
                napi_number) {
                return FIREWALL_ERR_PARAMETER_ERROR;
            }
            endPort = NapiUtils::GetInt32Property(env, valAttr, NET_FIREWALL_PORT_END);
            if (endPort > MAX_RULE_PORT || endPort < 0 || endPort < startPort) {
                NETMANAGER_EXT_LOGE("end port is more then %{public}d", MAX_RULE_PORT);
                return FIREWALL_ERR_INVALID_PARAMETER;
            }
        }
    }

    return FIREWALL_SUCCESS;
}

int32_t NetFirewallParamCheck::CheckDomainList(napi_env env, napi_value object)
{
    if (!NapiUtils::IsArray(env, object)) {
        NETMANAGER_EXT_LOGE("ParseDomainList get params failed");
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    uint32_t len = NapiUtils::GetArrayLength(env, object);
    for (size_t j = 0; j < len; j++) {
        napi_value valAttr = NapiUtils::GetArrayElement(env, object, j);
        if (NapiUtils::GetValueType(env, valAttr) != napi_object) {
            NETMANAGER_EXT_LOGI("valAttr is not napi_number");
            continue;
        }
        bool isWildCard = false;
        if (NapiUtils::HasNamedProperty(env, valAttr, NET_FIREWALL_DOMAIN_IS_WILDCARD)) {
            napi_value value = NapiUtils::GetNamedProperty(env, valAttr, NET_FIREWALL_DOMAIN_IS_WILDCARD);
            if (NapiUtils::GetValueType(env, value) != napi_boolean) {
                return FIREWALL_ERR_PARAMETER_ERROR;
            }
            isWildCard = NapiUtils::GetBooleanValue(env, value);
        }
        if (!NapiUtils::HasNamedProperty(env, valAttr, NET_FIREWALL_DOMAIN)) {
            continue;
        }
        std::string domain = "";
        if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, valAttr, NET_FIREWALL_DOMAIN)) !=
            napi_string) {
            return FIREWALL_ERR_PARAMETER_ERROR;
        }
        domain = NapiUtils::GetStringFromValueUtf8(env, NapiUtils::GetNamedProperty(env, valAttr, NET_FIREWALL_DOMAIN));
        if (domain.empty() || domain.size() > MAX_RULE_DOMAIN_NAME_LEN) {
            NETMANAGER_EXT_LOGE("domain is empty or length more than %{public}d", MAX_RULE_DOMAIN_NAME_LEN);
            return FIREWALL_ERR_INVALID_PARAMETER;
        }
        std::regex pattern = isWildCard ? WILDCARD_DOMAIN_PATTERN : DOMAIN_PATTERN;
        bool isValid = std::regex_match(domain, pattern);
        if (!isValid) {
            NETMANAGER_EXT_LOGE("invalid domain address");
            return FIREWALL_ERR_INVALID_PARAMETER;
        }
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallParamCheck::CheckUpdateFirewallRule(napi_env env, napi_value object)
{
    if (!NapiUtils::HasNamedProperty(env, object, NET_FIREWALL_RULE_ID)) {
        NETMANAGER_EXT_LOGE("params type invalid");
        return FIREWALL_ERR_PARAMETER_ERROR;
    }
    if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_RULE_ID)) != napi_number) {
        NETMANAGER_EXT_LOGE("params type invalid");
        return FIREWALL_ERR_PARAMETER_ERROR;
    }
    int32_t ruleId = NapiUtils::GetInt32Property(env, object, NET_FIREWALL_RULE_ID);
    if (ruleId < 0 || ruleId > INT32_MAX) {
        NETMANAGER_EXT_LOGE("invalid ruleId");
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    return CheckFirewallRule(env, object);
}

int32_t NetFirewallParamCheck::CheckAddFirewallRule(napi_env env, napi_value object)
{
    return CheckFirewallRule(env, object);
}

int32_t NetFirewallParamCheck::CheckFirewallRule(napi_env env, napi_value object)
{
    if (!NapiUtils::HasNamedProperty(env, object, NET_FIREWALL_RULE_NAME) ||
        !NapiUtils::HasNamedProperty(env, object, NET_FIREWALL_RULE_DIR) ||
        !NapiUtils::HasNamedProperty(env, object, NET_FIREWALL_RULE_ACTION) ||
        !NapiUtils::HasNamedProperty(env, object, NET_FIREWALL_RULE_TYPE) ||
        !NapiUtils::HasNamedProperty(env, object, NET_FIREWALL_IS_ENABLED) ||
        !NapiUtils::HasNamedProperty(env, object, NET_FIREWALL_USER_ID)) {
        NETMANAGER_EXT_LOGE("params can not be empty.");
        return FIREWALL_ERR_PARAMETER_ERROR;
    }
    NetFirewallRuleType ruleType =
        static_cast<NetFirewallRuleType>(NapiUtils::GetInt32Property(env, object, NET_FIREWALL_RULE_TYPE));
    if (ruleType != NetFirewallRuleType::RULE_IP && ruleType != NetFirewallRuleType::RULE_DOMAIN &&
        ruleType != NetFirewallRuleType::RULE_DNS) {
        NETMANAGER_EXT_LOGE("params ruleType invalid");
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    std::vector<std::string> numberParam;
    numberParam.emplace_back(NET_FIREWALL_RULE_ID);
    numberParam.emplace_back(NET_FIREWALL_RULE_DIR);
    numberParam.emplace_back(NET_FIREWALL_RULE_ACTION);
    numberParam.emplace_back(NET_FIREWALL_APP_ID);
    numberParam.emplace_back(NET_FIREWALL_USER_ID);
    if (ruleType == NetFirewallRuleType::RULE_IP) {
        numberParam.emplace_back(NET_FIREWALL_PROTOCOL);
    }
    int32_t ret = FIREWALL_SUCCESS;
    for (std::string &propertyName : numberParam) {
        ret = CheckRuleNumberParam(env, object, propertyName);
        if (ret != FIREWALL_SUCCESS) {
            NETMANAGER_EXT_LOGE("CheckRuleNumberParam %{public}s is error=%{public}d", propertyName.c_str(), ret);
            return ret;
        }
    }
    ret = CheckRuleName(env, object);
    if (ret != FIREWALL_SUCCESS) {
        return ret;
    }
    if (NapiUtils::HasNamedProperty(env, object, NET_FIREWALL_IS_ENABLED)) {
        if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_IS_ENABLED)) !=
            napi_boolean) {
            NETMANAGER_EXT_LOGE("params enable type invalid");
            return FIREWALL_ERR_PARAMETER_ERROR;
        }
    }
    return CheckRuleObjectParams(env, object, ruleType);
}

int32_t NetFirewallParamCheck::CheckRuleObjectParams(napi_env env, napi_value object, const NetFirewallRuleType &type)
{
    std::vector<std::string> objectParam;
    switch (type) {
        case NetFirewallRuleType::RULE_IP: {
            objectParam.emplace_back(NET_FIREWALL_LOCAL_IP);
            objectParam.emplace_back(NET_FIREWALL_REMOTE_IP);
            objectParam.emplace_back(NET_FIREWALL_LOCAL_PORT);
            objectParam.emplace_back(NET_FIREWALL_REMOTE_PORT);
            break;
        }
        case NetFirewallRuleType::RULE_DOMAIN:
            objectParam.emplace_back(NET_FIREWALL_RULE_DOMAIN);
            break;
        case NetFirewallRuleType::RULE_DNS:
            objectParam.emplace_back(NET_FIREWALL_DNS);
            break;
        default:
            break;
    }
    int32_t ret;
    for (const std::string &propertyName : objectParam) {
        ret = CheckRuleObjectParamValue(env, object, propertyName);
        if (ret != FIREWALL_SUCCESS) {
            NETMANAGER_EXT_LOGE("CheckRuleObjectParams %{public}s is error=%{public}d", propertyName.c_str(), ret);
            return ret;
        }
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallParamCheck::CheckRuleObjectParamValue(napi_env env, napi_value object,
    const std::string &propertyName)
{
    if (!NapiUtils::HasNamedProperty(env, object, propertyName)) {
        return FIREWALL_SUCCESS;
    }
    napi_value value = NapiUtils::GetNamedProperty(env, object, propertyName);
    if (NapiUtils::GetValueType(env, value) != napi_object) {
        NETMANAGER_EXT_LOGE("CheckRuleObjectParamValue property is not object");
        return FIREWALL_ERR_PARAMETER_ERROR;
    }
    if (propertyName == NET_FIREWALL_LOCAL_IP || propertyName == NET_FIREWALL_REMOTE_IP) {
        return CheckIpList(env, value);
    }
    if (propertyName == NET_FIREWALL_LOCAL_PORT || propertyName == NET_FIREWALL_REMOTE_PORT) {
        return CheckPortList(env, value);
    }
    if (propertyName == NET_FIREWALL_RULE_DOMAIN) {
        return CheckDomainList(env, value);
    }
    if (propertyName == NET_FIREWALL_DNS) {
        return CheckFirewallDns(env, value);
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallParamCheck::CheckRuleName(napi_env env, napi_value object)
{
    if (NapiUtils::HasNamedProperty(env, object, NET_FIREWALL_RULE_NAME)) {
        if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_RULE_NAME)) !=
            napi_string) {
            NETMANAGER_EXT_LOGE("params ruleName type invalid");
            return FIREWALL_ERR_PARAMETER_ERROR;
        }
        std::string ruleName =
            NapiUtils::GetStringFromValueUtf8(env, NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_RULE_NAME));
        if (ruleName.size() == 0 || ruleName.size() > MAX_RULE_NAME_LEN) {
            NETMANAGER_EXT_LOGE("ruleName=[%{public}s] is too long", ruleName.c_str());
            return FIREWALL_ERR_INVALID_PARAMETER;
        }
    }

    if (NapiUtils::HasNamedProperty(env, object, NET_FIREWALL_RULE_DESC)) {
        if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_RULE_DESC)) !=
            napi_string) {
            NETMANAGER_EXT_LOGE("params ruleDescription type invalid");
            return FIREWALL_ERR_PARAMETER_ERROR;
        }
        std::string ruleDescription =
            NapiUtils::GetStringFromValueUtf8(env, NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_RULE_DESC));
        if (ruleDescription.size() > MAX_RULE_DESCRIPTION_LEN) {
            NETMANAGER_EXT_LOGE("ruleDescription size=%{public}d is too long", ruleDescription.size());
            return FIREWALL_ERR_INVALID_PARAMETER;
        }
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallParamCheck::CheckRuleNumberParam(napi_env env, napi_value object, const std::string &propertyName)
{
    if (!NapiUtils::HasNamedProperty(env, object, propertyName)) {
        return FIREWALL_SUCCESS;
    }
    if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, object, propertyName)) != napi_number) {
        NETMANAGER_EXT_LOGE("CheckRuleNumberParam property is not number");
        return FIREWALL_ERR_PARAMETER_ERROR;
    }
    if (propertyName == NET_FIREWALL_RULE_DIR) {
        return CheckFirewallDirection(env, object);
    }
    if (propertyName == NET_FIREWALL_RULE_ACTION) {
        return CheckFirewallAction(env, object);
    }
    int32_t value = NapiUtils::GetInt32Property(env, object, propertyName);
    if (propertyName == NET_FIREWALL_APP_ID) {
        if (value < 0 || value > INT32_MAX) {
            NETMANAGER_EXT_LOGE("invalid appUid");
            return FIREWALL_ERR_INVALID_PARAMETER;
        }
    }
    if (propertyName == NET_FIREWALL_PROTOCOL) {
        NetworkProtocol ruleProtocol = static_cast<NetworkProtocol>(value);
        if (ruleProtocol != NetworkProtocol::ICMP && ruleProtocol != NetworkProtocol::TCP &&
            ruleProtocol != NetworkProtocol::UDP && ruleProtocol != NetworkProtocol::ICMPV6) {
            NETMANAGER_EXT_LOGE("params protocol invalid");
            return FIREWALL_ERR_INVALID_PARAMETER;
        }
    }
    if (propertyName == NET_FIREWALL_USER_ID) {
        return value < 0 ? FIREWALL_ERR_INVALID_PARAMETER : FIREWALL_SUCCESS;
    }
    return FIREWALL_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS