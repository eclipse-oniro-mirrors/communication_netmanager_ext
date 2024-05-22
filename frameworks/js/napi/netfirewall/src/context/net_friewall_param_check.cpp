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

#include "napi_utils.h"
#include "net_firewall_param_check.h"
#include "net_manager_constants.h"
#include "netmanager_base_common_utils.h"
#include "netmanager_ext_log.h"
#include <regex>

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t MAX_RULE_NAME_LEN = 128;
constexpr int32_t MAX_RULE_DESCRIPTION_LEN = 256;
constexpr int32_t MAX_RULE_IP_COUNT = 20;
constexpr int32_t MAX_RULE_PORT_COUNT = 10;
constexpr int32_t MAX_RULE_DOMAIN_COUNT = 100;
constexpr int32_t MAX_RULE_DOMAIN_NAME_LEN = 253;
constexpr int32_t MAX_RULE_PORT = 65535;
const std::regex DOMAIN_PATTERN { "(https?://"
    ")?([a-zA-Z0-9-]|\\*)+(\\.([a-zA-Z0-9-])+)*\\.(com|net|org|edu|gov|mil|cn|hk|tw|jp|de|uk|fr|au|ca|br|ru|it|es|in|"
    "online|shop|vip|club|xyz|top|icu|work|website|tech|asia|xin|co|mobi|info)" };

int32_t NetFirewallParamCheck::CheckFirewallRuleStatus(napi_env env, napi_value object)
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
        if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_IN_ACTION)) ==
            napi_number) {
            FirewallRuleAction inAction =
                static_cast<FirewallRuleAction>(NapiUtils::GetInt32Property(env, object, NET_FIREWALL_IN_ACTION));
            if (inAction != FirewallRuleAction::RULE_ALLOW && inAction != FirewallRuleAction::RULE_DENY) {
                NETMANAGER_EXT_LOGE("params inAction invalid");
                return FIREWALL_ERR_INVALID_PARAMETER;
            }
        } else {
            NETMANAGER_EXT_LOGE("params inAction type invalid");
            return FIREWALL_ERR_PARAMETER_ERROR;
        }
    }

    if (NapiUtils::HasNamedProperty(env, object, NET_FIREWALL_OUT_ACTION)) {
        if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_OUT_ACTION)) ==
            napi_number) {
            FirewallRuleAction outAction =
                static_cast<FirewallRuleAction>(NapiUtils::GetInt32Property(env, object, NET_FIREWALL_OUT_ACTION));
            if (outAction != FirewallRuleAction::RULE_ALLOW && outAction != FirewallRuleAction::RULE_DENY) {
                NETMANAGER_EXT_LOGE("params outAction invalid");
                return FIREWALL_ERR_INVALID_PARAMETER;
            }
        } else {
            NETMANAGER_EXT_LOGE("params outAction type invalid");
            return FIREWALL_ERR_PARAMETER_ERROR;
        }
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallParamCheck::CheckFirewallDns(napi_env env, napi_value object)
{
    if (NapiUtils::HasNamedProperty(env, object, NET_FIREWALL_DNS_PRIMARY)) {
        if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_DNS_PRIMARY)) ==
            napi_string) {
            std::string primaryDns = NapiUtils::GetStringFromValueUtf8(env,
                NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_DNS_PRIMARY));
            int32_t ret = CheckIpAddress(primaryDns);
            if (ret != FIREWALL_SUCCESS) {
                return ret;
            }
        } else {
            NETMANAGER_EXT_LOGE("params dns type invalid");
            return FIREWALL_ERR_PARAMETER_ERROR;
        }
    }

    if (NapiUtils::HasNamedProperty(env, object, NET_FIREWALL_DNS_STANDY)) {
        if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_DNS_STANDY)) ==
            napi_string) {
            std::string standyDns = NapiUtils::GetStringFromValueUtf8(env,
                NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_DNS_STANDY));
            int32_t ret = CheckIpAddress(standyDns);
            if (ret != FIREWALL_SUCCESS) {
                return ret;
            }
        } else {
            NETMANAGER_EXT_LOGE("params dns type invalid");
            return FIREWALL_ERR_PARAMETER_ERROR;
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
    if (family == FAMILY_IPV6) {
        in6_addr in6_addr1, in6_addr2;
        // 将IPv6地址转换为二进制形式
        ret = inet_pton(AF_INET6, startIp.c_str(), &in6_addr1);
        if (ret <= 0) {
            NETMANAGER_EXT_LOGE("CheckIpAddress ipv6: startIp is invalid");
            return false;
        }
        ret = inet_pton(AF_INET6, endIp.c_str(), &in6_addr2);
        if (ret <= 0) {
            NETMANAGER_EXT_LOGE("CheckIpAddress ipv6: endIp is invalid");
            return false;
        }
        // 比较二进制形式的IPv6地址
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
    if (ret <= 0) {
        NETMANAGER_EXT_LOGE("CheckIpAddress ipv4: startIp is invalid");
        return false;
    }
    ret = inet_pton(AF_INET, endIp.c_str(), &inAddr2);
    if (ret <= 0) {
        NETMANAGER_EXT_LOGE("CheckIpAddress ipv4: endIp is invalid");
        return false;
    }
    ret = memcmp(&startIp, &endIp, sizeof(struct in_addr));
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
    // 单ip时，address必须有
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
    // 多ip时，起始地址必须有
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
    if (len > MAX_RULE_IP_COUNT || len > MAX_RULE_IP_COUNT) {
        NETMANAGER_EXT_LOGE("ip invalid, size is too long.");
        return FIREWALL_ERR_EXCEED_MAX_IP;
    }
    for (size_t j = 0; j < len; j++) {
        napi_value valAttr = NapiUtils::GetArrayElement(env, object, j);
        if (NapiUtils::GetValueType(env, valAttr) != napi_object) {
            NETMANAGER_EXT_LOGI("valAttr is not napi_number");
            continue;
        }
        int32_t family = FAMILY_IPV4;
        if (NapiUtils::HasNamedProperty(env, valAttr, NET_FIREWALL_IP_FAMILY)) {
            if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, valAttr, NET_FIREWALL_IP_FAMILY)) ==
                napi_number) {
                family = NapiUtils::GetInt32Property(env, valAttr, NET_FIREWALL_IP_FAMILY);
            } else {
                return FIREWALL_ERR_PARAMETER_ERROR;
            }
            if (family != FAMILY_IPV4 && family != FAMILY_IPV6) {
                NETMANAGER_EXT_LOGE("family param invalid.");
                return FIREWALL_ERR_INVALID_PARAMETER;
            }
        }

        int32_t type = SINGLE_IP;
        if (NapiUtils::HasNamedProperty(env, valAttr, NET_FIREWALL_IP_TYPE)) {
            if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, valAttr, NET_FIREWALL_IP_TYPE)) ==
                napi_number) {
                type = NapiUtils::GetInt32Property(env, valAttr, NET_FIREWALL_IP_TYPE);
            } else {
                return FIREWALL_ERR_PARAMETER_ERROR;
            }
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
    if (len > MAX_RULE_PORT_COUNT) {
        NETMANAGER_EXT_LOGE("port invalid, size is too long.");
        return FIREWALL_ERR_EXCEED_MAX_PORT;
    }

    for (size_t j = 0; j < len; j++) {
        napi_value valAttr = NapiUtils::GetArrayElement(env, object, j);
        if (NapiUtils::GetValueType(env, valAttr) != napi_object) {
            NETMANAGER_EXT_LOGI("valAttr is not napi_number");
            continue;
        }
        int32_t startPort = 0;
        if (NapiUtils::HasNamedProperty(env, valAttr, NET_FIREWALL_PORT_START)) {
            if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, valAttr, NET_FIREWALL_PORT_START)) ==
                napi_number) {
                startPort = NapiUtils::GetInt32Property(env, valAttr, NET_FIREWALL_PORT_START);
            } else {
                return FIREWALL_ERR_PARAMETER_ERROR;
            }
            if (startPort > MAX_RULE_PORT || startPort < 0) {
                NETMANAGER_EXT_LOGE("start port is more then %{public}d", MAX_RULE_PORT);
                return FIREWALL_ERR_INVALID_PARAMETER;
            }
        }
        if (NapiUtils::HasNamedProperty(env, valAttr, NET_FIREWALL_PORT_END)) {
            int32_t endPort = 0;
            if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, valAttr, NET_FIREWALL_PORT_END)) ==
                napi_number) {
                endPort = NapiUtils::GetInt32Property(env, valAttr, NET_FIREWALL_PORT_END);
            } else {
                return FIREWALL_ERR_PARAMETER_ERROR;
            }
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
    if (len > MAX_RULE_DOMAIN_COUNT) {
        NETMANAGER_EXT_LOGE("domain invalid, size is too long.");
        return FIREWALL_ERR_EXCEED_MAX_DOMAIN;
    }

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
            } else {
                isWildCard = NapiUtils::GetBooleanValue(env, value);
            }
        }

        if (!NapiUtils::HasNamedProperty(env, valAttr, NET_FIREWALL_DOMAIN)) {
            continue;
        }
        std::string domain = "";
        if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, valAttr, NET_FIREWALL_DOMAIN)) ==
            napi_string) {
            domain =
                NapiUtils::GetStringFromValueUtf8(env, NapiUtils::GetNamedProperty(env, valAttr, NET_FIREWALL_DOMAIN));
        } else {
            return FIREWALL_ERR_PARAMETER_ERROR;
        }
        if (domain.empty() || domain.size() > MAX_RULE_DOMAIN_NAME_LEN) {
            NETMANAGER_EXT_LOGE("domain is empty or length more than %{public}d", MAX_RULE_DOMAIN_NAME_LEN);
            return FIREWALL_ERR_INVALID_PARAMETER;
        }
        bool isValid = isWildCard ? std::regex_match(domain, DOMAIN_PATTERN) : CommonUtils::IsValidDomain(domain);
        if (!isValid) {
            NETMANAGER_EXT_LOGE("invalid domain address");
            return FIREWALL_ERR_INVALID_PARAMETER;
        }
    }

    return FIREWALL_SUCCESS;
}

int32_t NetFirewallParamCheck::CheckUpdateFirewallRule(napi_env env, napi_value object)
{
    if (NapiUtils::HasNamedProperty(env, object, NET_FIREWALL_RULE_ID)) {
        if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_RULE_ID)) ==
            napi_number) {
            int32_t ruleId = NapiUtils::GetInt32Property(env, object, NET_FIREWALL_RULE_ID);
            if (ruleId < 0 || ruleId > INT32_MAX) {
                NETMANAGER_EXT_LOGE("invalid ruleId");
                return FIREWALL_ERR_INVALID_PARAMETER;
            }
        } else {
            NETMANAGER_EXT_LOGE("params type invalid");
            return FIREWALL_ERR_PARAMETER_ERROR;
        }
    } else {
        NETMANAGER_EXT_LOGE("params type invalid");
        return FIREWALL_ERR_PARAMETER_ERROR;
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
    if (type == NetFirewallRuleType::RULE_IP) {
        objectParam.emplace_back(NET_FIREWALL_LOCAL_IP);
        objectParam.emplace_back(NET_FIREWALL_REMOTE_IP);
        objectParam.emplace_back(NET_FIREWALL_LOCAL_PORT);
        objectParam.emplace_back(NET_FIREWALL_REMOTE_PORT);
    } else if (type == NetFirewallRuleType::RULE_DOMAIN) {
        objectParam.emplace_back(NET_FIREWALL_RULE_DOMAIN);
    } else {
        objectParam.emplace_back(NET_FIREWALL_DNS);
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
    if (propertyName.find("Ips") != std::string::npos) {
        return CheckIpList(env, value);
    }
    if (propertyName.find("Ports") != std::string::npos) {
        return CheckPortList(env, value);
    }
    if (propertyName.find("domains") != std::string::npos) {
        return CheckDomainList(env, value);
    }
    if (propertyName.find("dns") != std::string::npos) {
        return CheckFirewallDns(env, value);
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallParamCheck::CheckRuleName(napi_env env, napi_value object)
{
    if (NapiUtils::HasNamedProperty(env, object, NET_FIREWALL_RULE_NAME)) {
        if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_RULE_NAME)) ==
            napi_string) {
            std::string ruleName = NapiUtils::GetStringFromValueUtf8(env,
                NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_RULE_NAME));
            if (ruleName.size() == 0 || ruleName.size() > MAX_RULE_NAME_LEN) {
                NETMANAGER_EXT_LOGE("ruleName=[%{public}s] is too long", ruleName.c_str());
                return FIREWALL_ERR_INVALID_PARAMETER;
            }
        } else {
            NETMANAGER_EXT_LOGE("params ruleName type invalid");
            return FIREWALL_ERR_PARAMETER_ERROR;
        }
    }

    if (NapiUtils::HasNamedProperty(env, object, NET_FIREWALL_RULE_DESC)) {
        if (NapiUtils::GetValueType(env, NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_RULE_DESC)) ==
            napi_string) {
            std::string ruleDescription = NapiUtils::GetStringFromValueUtf8(env,
                NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_RULE_DESC));
            if (ruleDescription.size() > MAX_RULE_DESCRIPTION_LEN) {
                NETMANAGER_EXT_LOGE("ruleDescription size=%{public}d is too long", ruleDescription.size());
                return FIREWALL_ERR_INVALID_PARAMETER;
            }
        } else {
            NETMANAGER_EXT_LOGE("params ruleDescription type invalid");
            return FIREWALL_ERR_PARAMETER_ERROR;
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