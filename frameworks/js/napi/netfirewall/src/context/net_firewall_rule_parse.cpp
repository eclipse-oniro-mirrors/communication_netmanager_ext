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

#include "net_firewall_rule_parse.h"
#include "napi_utils.h"
#include "netmanager_ext_log.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {
void NetFirewallRuleParse::ParseIpList(napi_env env, napi_value params, std::vector<NetFirewallIpParam> &list)
{
    if (!NapiUtils::IsArray(env, params)) {
        NETMANAGER_EXT_LOGE("ParseIpList get params failed");
        return;
    }
    uint32_t len = NapiUtils::GetArrayLength(env, params);
    std::vector<NetFirewallIpParam>().swap(list);
    for (size_t j = 0; j < len; j++) {
        napi_value valAttr = NapiUtils::GetArrayElement(env, params, j);
        if (NapiUtils::GetValueType(env, valAttr) != napi_object) {
            NETMANAGER_EXT_LOGI("valAttr is not napi_number");
            continue;
        }
        NetFirewallIpParam param;
        param.family = static_cast<uint8_t>(NapiUtils::GetUint32Property(env, valAttr, NET_FIREWALL_IP_FAMILY));
        // Default IPv4
        if (param.family == 0) {
            param.family = FAMILY_IPV4;
        }
        param.type = static_cast<uint8_t>(NapiUtils::GetUint32Property(env, valAttr, NET_FIREWALL_IP_TYPE));
        // Default single IP
        if (param.type == 0) {
            param.type = SINGLE_IP;
        }
        std::string startIp;
        std::string endIp;
        if (param.type == SINGLE_IP) {
            param.mask = static_cast<uint8_t>(NapiUtils::GetUint32Property(env, valAttr, NET_FIREWALL_IP_MASK));
            startIp = NapiUtils::GetStringPropertyUtf8(env, valAttr, NET_FIREWALL_IP_ADDRESS);
            // The default IPv4 mask is 32, The IPv6 prefix is 64
            if (param.mask == 0 && param.type == SINGLE_IP) {
                param.mask = param.family == FAMILY_IPV4 ? IPV4_MASK_MAX : IPV6_MASK_MAX;
            }
        } else {
            startIp = NapiUtils::GetStringPropertyUtf8(env, valAttr, NET_FIREWALL_IP_START);
            endIp = NapiUtils::GetStringPropertyUtf8(env, valAttr, NET_FIREWALL_IP_END);
        }
        if (param.family == FAMILY_IPV4) {
            inet_pton(AF_INET, startIp.c_str(), &param.ipv4.startIp);
            if (param.type == MULTIPLE_IP) {
                inet_pton(AF_INET, endIp.c_str(), &param.ipv4.endIp);
            }
        } else {
            inet_pton(AF_INET6, startIp.c_str(), &param.ipv6.startIp);
            if (param.type == MULTIPLE_IP) {
                inet_pton(AF_INET6, endIp.c_str(), &param.ipv6.endIp);
            }
        }
        list.emplace_back(param);
    }
}

void NetFirewallRuleParse::ParsePortList(napi_env env, napi_value params, std::vector<NetFirewallPortParam> &list)
{
    if (!NapiUtils::IsArray(env, params)) {
        NETMANAGER_EXT_LOGE("ParsePortList get params failed");
        return;
    }
    uint32_t len = NapiUtils::GetArrayLength(env, params);
    std::vector<NetFirewallPortParam>().swap(list);
    for (size_t j = 0; j < len; j++) {
        napi_value valAttr = NapiUtils::GetArrayElement(env, params, j);
        if (NapiUtils::GetValueType(env, valAttr) != napi_object) {
            NETMANAGER_EXT_LOGI("valAttr is not napi_number");
            continue;
        }
        NetFirewallPortParam param;
        param.startPort = static_cast<uint16_t>(NapiUtils::GetUint32Property(env, valAttr, NET_FIREWALL_PORT_START));
        param.endPort = static_cast<uint16_t>(NapiUtils::GetUint32Property(env, valAttr, NET_FIREWALL_PORT_END));
        list.emplace_back(param);
    }
}

void NetFirewallRuleParse::ParseDomainList(napi_env env, napi_value params, std::vector<NetFirewallDomainParam> &list)
{
    if (!NapiUtils::IsArray(env, params)) {
        NETMANAGER_EXT_LOGE("ParseDomainList get params failed");
        return;
    }
    uint32_t len = NapiUtils::GetArrayLength(env, params);
    std::vector<NetFirewallDomainParam>().swap(list);
    for (size_t j = 0; j < len; j++) {
        napi_value valAttr = NapiUtils::GetArrayElement(env, params, j);
        if (NapiUtils::GetValueType(env, valAttr) != napi_object) {
            NETMANAGER_EXT_LOGI("valAttr is not napi_number");
            continue;
        }
        NetFirewallDomainParam param;
        param.isWildcard = NapiUtils::GetBooleanProperty(env, valAttr, NET_FIREWALL_DOMAIN_IS_WILDCARD);
        param.domain = NapiUtils::GetStringPropertyUtf8(env, valAttr, NET_FIREWALL_DOMAIN);
        list.emplace_back(param);
    }
}

void NetFirewallRuleParse::ParseRuleParams(napi_env env, napi_value object, const sptr<NetFirewallRule> &rule)
{
    rule->ruleId = NapiUtils::GetInt32Property(env, object, NET_FIREWALL_RULE_ID);
    rule->ruleName = NapiUtils::GetStringPropertyUtf8(env, object, NET_FIREWALL_RULE_NAME);
    rule->ruleDescription = NapiUtils::GetStringPropertyUtf8(env, object, NET_FIREWALL_RULE_DESC);
    rule->ruleDirection =
        static_cast<NetFirewallRuleDirection>(NapiUtils::GetInt32Property(env, object, NET_FIREWALL_RULE_DIR));
    rule->ruleAction =
        static_cast<FirewallRuleAction>(NapiUtils::GetInt32Property(env, object, NET_FIREWALL_RULE_ACTION));
    rule->ruleType = static_cast<NetFirewallRuleType>(NapiUtils::GetInt32Property(env, object, NET_FIREWALL_RULE_TYPE));
    rule->isEnabled = NapiUtils::GetBooleanProperty(env, object, NET_FIREWALL_IS_ENABLED);
    rule->appUid = NapiUtils::GetInt32Property(env, object, NET_FIREWALL_APP_ID);
    if (rule->ruleType == NetFirewallRuleType::RULE_IP) {
        ParseIpList(env, NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_LOCAL_IP), rule->localIps);
        ParseIpList(env, NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_REMOTE_IP), rule->remoteIps);
        rule->protocol = static_cast<NetworkProtocol>(NapiUtils::GetInt32Property(env, object, NET_FIREWALL_PROTOCOL));
        ParsePortList(env, NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_LOCAL_PORT), rule->localPorts);
        ParsePortList(env, NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_REMOTE_PORT), rule->remotePorts);
    } else if (rule->ruleType == NetFirewallRuleType::RULE_DOMAIN) {
        ParseDomainList(env, NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_RULE_DOMAIN), rule->domains);
    } else {
        napi_value dns = NapiUtils::GetNamedProperty(env, object, NET_FIREWALL_DNS);
        rule->dns.primaryDns = NapiUtils::GetStringPropertyUtf8(env, dns, NET_FIREWALL_DNS_PRIMARY);
        rule->dns.standbyDns = NapiUtils::GetStringPropertyUtf8(env, dns, NET_FIREWALL_DNS_STANDY);
    }
    rule->userId = NapiUtils::GetInt32Property(env, object, NET_FIREWALL_USER_ID);
}

void NetFirewallRuleParse::SetIpList(napi_env env, napi_value object, const std::string &propertyName,
    const std::vector<NetFirewallIpParam> &list)
{
    uint32_t index = 0;
    napi_value ipList = NapiUtils::CreateArray(env, list.size());
    for (const auto &iface : list) {
        napi_value element = NapiUtils::CreateObject(env);
        NapiUtils::SetUint32Property(env, element, NET_FIREWALL_IP_FAMILY, static_cast<uint32_t>(iface.family));
        NapiUtils::SetUint32Property(env, element, NET_FIREWALL_IP_TYPE, static_cast<uint32_t>(iface.type));
        if (iface.type == SINGLE_IP) {
            NapiUtils::SetUint32Property(env, element, NET_FIREWALL_IP_MASK, static_cast<uint32_t>(iface.mask));
            NapiUtils::SetStringPropertyUtf8(env, element, NET_FIREWALL_IP_ADDRESS, iface.GetStartIp());
        } else {
            NapiUtils::SetStringPropertyUtf8(env, element, NET_FIREWALL_IP_START, iface.GetStartIp());
            NapiUtils::SetStringPropertyUtf8(env, element, NET_FIREWALL_IP_END, iface.GetEndIp());
        }
        NapiUtils::SetArrayElement(env, ipList, index++, element);
    }
    NapiUtils::SetNamedProperty(env, object, propertyName, ipList);
}

void NetFirewallRuleParse::SetPortList(napi_env env, napi_value object, const std::string &propertyName,
    const std::vector<NetFirewallPortParam> &list)
{
    uint32_t index = 0;
    napi_value portList = NapiUtils::CreateArray(env, list.size());
    for (const auto &iface : list) {
        napi_value element = NapiUtils::CreateObject(env);
        NapiUtils::SetUint32Property(env, element, NET_FIREWALL_PORT_START, static_cast<uint32_t>(iface.startPort));
        NapiUtils::SetUint32Property(env, element, NET_FIREWALL_PORT_END, static_cast<uint32_t>(iface.endPort));

        NapiUtils::SetArrayElement(env, portList, index++, element);
    }
    NapiUtils::SetNamedProperty(env, object, propertyName, portList);
}

void NetFirewallRuleParse::SetDomainList(napi_env env, napi_value object, const std::string &propertyName,
    const std::vector<NetFirewallDomainParam> &list)
{
    uint32_t index = 0;
    napi_value domenList = NapiUtils::CreateArray(env, list.size());
    for (const auto &iface : list) {
        napi_value element = NapiUtils::CreateObject(env);
        NapiUtils::SetBooleanProperty(env, element, NET_FIREWALL_DOMAIN_IS_WILDCARD, iface.isWildcard);
        NapiUtils::SetStringPropertyUtf8(env, element, NET_FIREWALL_DOMAIN, iface.domain);

        NapiUtils::SetArrayElement(env, domenList, index++, element);
    }
    NapiUtils::SetNamedProperty(env, object, propertyName, domenList);
}

void NetFirewallRuleParse::SetRuleParams(napi_env env, napi_value object, const NetFirewallRule &rule)
{
    NapiUtils::SetInt32Property(env, object, NET_FIREWALL_RULE_ID, rule.ruleId);
    NapiUtils::SetStringPropertyUtf8(env, object, NET_FIREWALL_RULE_NAME, rule.ruleName);
    NapiUtils::SetStringPropertyUtf8(env, object, NET_FIREWALL_RULE_DESC, rule.ruleDescription);
    NapiUtils::SetInt32Property(env, object, NET_FIREWALL_RULE_DIR, static_cast<int32_t>(rule.ruleDirection));
    NapiUtils::SetInt32Property(env, object, NET_FIREWALL_RULE_ACTION, static_cast<int32_t>(rule.ruleAction));
    NapiUtils::SetInt32Property(env, object, NET_FIREWALL_RULE_TYPE, static_cast<int32_t>(rule.ruleType));
    NapiUtils::SetBooleanProperty(env, object, NET_FIREWALL_IS_ENABLED, rule.isEnabled);
    NapiUtils::SetInt32Property(env, object, NET_FIREWALL_APP_ID, rule.appUid);
    if (rule.ruleType == NetFirewallRuleType::RULE_IP) {
        SetIpList(env, object, NET_FIREWALL_LOCAL_IP, rule.localIps);
        SetIpList(env, object, NET_FIREWALL_REMOTE_IP, rule.remoteIps);
        NapiUtils::SetInt32Property(env, object, NET_FIREWALL_PROTOCOL, static_cast<int32_t>(rule.protocol));
        SetPortList(env, object, NET_FIREWALL_LOCAL_PORT, rule.localPorts);
        SetPortList(env, object, NET_FIREWALL_REMOTE_PORT, rule.remotePorts);
    } else if (rule.ruleType == NetFirewallRuleType::RULE_DOMAIN) {
        SetDomainList(env, object, NET_FIREWALL_RULE_DOMAIN, rule.domains);
    } else {
        napi_value dns = NapiUtils::CreateObject(env);
        NapiUtils::SetStringPropertyUtf8(env, dns, NET_FIREWALL_DNS_PRIMARY, rule.dns.primaryDns);
        NapiUtils::SetStringPropertyUtf8(env, dns, NET_FIREWALL_DNS_STANDY, rule.dns.standbyDns);
        NapiUtils::SetNamedProperty(env, object, NET_FIREWALL_DNS, dns);
    }
    NapiUtils::SetInt32Property(env, object, NET_FIREWALL_USER_ID, rule.userId);
}

int32_t NetFirewallRuleParse::ParsePageParam(napi_env env, napi_value object, const sptr<RequestParam> &param,
    bool isRule)
{
    if (!NapiUtils::HasNamedProperty(env, object, NET_FIREWALL_PAGE) ||
        !NapiUtils::HasNamedProperty(env, object, NET_FIREWALL_PAGE_SIZE) ||
        !NapiUtils::HasNamedProperty(env, object, NET_FIREWALL_ORDER_FIELD) ||
        !NapiUtils::HasNamedProperty(env, object, NET_FIREWALL_ORDER_TYPE)) {
        NETMANAGER_EXT_LOGE("params can not be empty.");
        return FIREWALL_ERR_PARAMETER_ERROR;
    }
    param->page = NapiUtils::GetInt32Property(env, object, NET_FIREWALL_PAGE);
    if (param->page < 1 || param->page > FIREWALL_USER_MAX_RULE) {
        NETMANAGER_EXT_LOGE("ParsePageParam page[%{public}d] is error", param->page);
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    param->pageSize = NapiUtils::GetInt32Property(env, object, NET_FIREWALL_PAGE_SIZE);
    if (param->pageSize < 1 || param->pageSize > MAX_PAGE_SIZE) {
        NETMANAGER_EXT_LOGE("ParsePageParam pageSize[%{public}d] is error", param->pageSize);
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    param->orderField =
        static_cast<NetFirewallOrderField>(NapiUtils::GetInt32Property(env, object, NET_FIREWALL_ORDER_FIELD));
    if ((isRule && param->orderField != NetFirewallOrderField::ORDER_BY_RULE_NAME) ||
        (!isRule && param->orderField != NetFirewallOrderField::ORDER_BY_RECORD_TIME)) {
        NETMANAGER_EXT_LOGE("params orderField invalid");
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    param->orderType =
        static_cast<NetFirewallOrderType>(NapiUtils::GetInt32Property(env, object, NET_FIREWALL_ORDER_TYPE));
    if (param->orderType != NetFirewallOrderType::ORDER_ASC && param->orderType != NetFirewallOrderType::ORDER_DESC) {
        NETMANAGER_EXT_LOGE("params orderType invalid");
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    return FIREWALL_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS