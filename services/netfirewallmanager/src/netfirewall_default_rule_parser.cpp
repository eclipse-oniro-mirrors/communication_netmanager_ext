/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include <fstream>

#include "netmgr_ext_log_wrapper.h"
#include "netfirewall_default_rule_parser.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr const char *DEFAULT_RULES = "default_rules";

void NetFirewallDefaultRuleParser::ConvertFirewallRuleToConfig(sptr<NetFirewallRule> &rule, const cJSON * const mem)
{
    cJSON *ruleName = cJSON_GetObjectItem(mem, NET_FIREWALL_RULE_NAME.c_str());
    if (ruleName != nullptr && cJSON_IsString(ruleName)) {
        rule->ruleName = cJSON_GetStringValue(ruleName);
        NETMGR_EXT_LOG_D("ruleName = %{public}s", rule->ruleName.c_str());
    }
    cJSON *ruleDescription = cJSON_GetObjectItem(mem, NET_FIREWALL_RULE_DESC.c_str());
    if (ruleDescription != nullptr && cJSON_IsString(ruleDescription)) {
        rule->ruleDescription = cJSON_GetStringValue(ruleDescription);
        NETMGR_EXT_LOG_D("ruleDescription = %{public}s", rule->ruleDescription.c_str());
    }
    cJSON *ruleDirection = cJSON_GetObjectItem(mem, NET_FIREWALL_RULE_DIR.c_str());
    if (ruleDirection != nullptr && cJSON_IsNumber(ruleDirection)) {
        rule->ruleDirection = static_cast<NetFirewallRuleDirection>(cJSON_GetNumberValue(ruleDirection));
        NETMGR_EXT_LOG_D("mtu = %{public}d", rule->ruleDirection);
    }
    cJSON *ruleAction = cJSON_GetObjectItem(mem, NET_FIREWALL_RULE_ACTION.c_str());
    if (ruleAction != nullptr && cJSON_IsNumber(ruleAction)) {
        rule->ruleAction = static_cast<FirewallRuleAction>(cJSON_GetNumberValue(ruleAction));
        NETMGR_EXT_LOG_D("ruleAction = %{public}d", rule->ruleAction);
    }
    cJSON *ruleType = cJSON_GetObjectItem(mem, NET_FIREWALL_RULE_TYPE.c_str());
    if (ruleType != nullptr && cJSON_IsNumber(ruleType)) {
        rule->ruleType = static_cast<NetFirewallRuleType>(cJSON_GetNumberValue(ruleType));
        NETMGR_EXT_LOG_D("ruleType = %{public}d", rule->ruleType);
    }
    cJSON *isEnabled = cJSON_GetObjectItem(mem, NET_FIREWALL_IS_ENABLED.c_str());
    if (isEnabled != nullptr && cJSON_IsBool(isEnabled)) {
        rule->isEnabled = cJSON_IsTrue(isEnabled);
        NETMGR_EXT_LOG_D("isEnabled = %{public}d", rule->isEnabled);
    }
    cJSON *appUid = cJSON_GetObjectItem(mem, NET_FIREWALL_APP_ID.c_str());
    if (appUid != nullptr && cJSON_IsNumber(appUid)) {
        rule->appUid = cJSON_GetNumberValue(appUid);
        NETMGR_EXT_LOG_D("appUid = %{public}d", rule->appUid);
    }
    ParseListObject(rule, mem);
    cJSON *protocol = cJSON_GetObjectItem(mem, NET_FIREWALL_PROTOCOL.c_str());
    if (protocol != nullptr && cJSON_IsNumber(protocol)) {
        rule->protocol = static_cast<NetworkProtocol>(cJSON_GetNumberValue(protocol));
        NETMGR_EXT_LOG_D("protocol = %{public}d", rule->protocol);
    }
    ParseDnsObject(rule->dns, mem, NET_FIREWALL_DNS);
    cJSON *userId = cJSON_GetObjectItem(mem, NET_FIREWALL_USER_ID.c_str());
    if (userId != nullptr && cJSON_IsNumber(userId)) {
        rule->userId = cJSON_GetNumberValue(userId);
        NETMGR_EXT_LOG_D("userId = %{public}d", rule->userId);
    }
}

void NetFirewallDefaultRuleParser::ParseListObject(sptr<NetFirewallRule> &rule, const cJSON * const mem)
{
    ParseIpList(rule->localIps, mem, NET_FIREWALL_LOCAL_IP);
    ParseIpList(rule->remoteIps, mem, NET_FIREWALL_REMOTE_IP);
    ParsePortList(rule->localPorts, mem, NET_FIREWALL_LOCAL_PORT);
    ParsePortList(rule->remotePorts, mem, NET_FIREWALL_REMOTE_PORT);
    ParseDomainList(rule->domains, mem, NET_FIREWALL_RULE_DOMAIN);
}

void NetFirewallDefaultRuleParser::ParseIpList(std::vector<NetFirewallIpParam> &ipParamlist, const cJSON * const mem,
    const std::string jsonKey)
{
    cJSON *ips = cJSON_GetObjectItem(mem, jsonKey.c_str());
    if (ips != nullptr && cJSON_IsArray(ips)) {
        uint32_t itemSize = cJSON_GetArraySize(ips);
        for (uint32_t i = 0; i < itemSize; i++) {
            cJSON *item = cJSON_GetArrayItem(ips, i);
            if (cJSON_IsObject(item)) {
                NetFirewallIpParam ipParam;
                ConvertIpParamToConfig(ipParam, item);
                ipParamlist.emplace_back(std::move(ipParam));
            }
        }
    }
}

void NetFirewallDefaultRuleParser::ParsePortList(std::vector<NetFirewallPortParam> &portParamlist,
    const cJSON * const mem, const std::string jsonKey)
{
    cJSON *prot = cJSON_GetObjectItem(mem, jsonKey.c_str());
    if (prot != nullptr && cJSON_IsArray(prot)) {
        uint32_t itemSize = cJSON_GetArraySize(prot);
        for (uint32_t i = 0; i < itemSize; i++) {
            cJSON *item = cJSON_GetArrayItem(prot, i);
            if (cJSON_IsObject(item)) {
                NetFirewallPortParam portParam;
                ConvertPortParamToConfig(portParam, item);
                portParamlist.emplace_back(std::move(portParam));
            }
        }
    }
}

void NetFirewallDefaultRuleParser::ParseDomainList(std::vector<NetFirewallDomainParam> &domainParamlist,
    const cJSON * const mem, const std::string jsonKey)
{
    cJSON *domain = cJSON_GetObjectItem(mem, jsonKey.c_str());
    if (domain != nullptr && cJSON_IsArray(domain)) {
        uint32_t itemSize = cJSON_GetArraySize(domain);
        for (uint32_t i = 0; i < itemSize; i++) {
            cJSON *item = cJSON_GetArrayItem(domain, i);
            if (cJSON_IsObject(item)) {
                NetFirewallDomainParam domainParam;
                ConvertDomainParamToConfig(domainParam, item);
                domainParamlist.emplace_back(std::move(domainParam));
            }
        }
    }
}

void NetFirewallDefaultRuleParser::ParseDnsObject(NetFirewallDnsParam &dns, const cJSON * const mem,
    const std::string jsonKey)
{
    cJSON *obj = cJSON_GetObjectItem(mem, jsonKey.c_str());
    if (obj != nullptr && cJSON_IsObject(obj)) {
        NetFirewallDnsParam dnsParam;
        ConvertDnsParamToConfig(dnsParam, obj);
        dns = dnsParam;
    }
}

void NetFirewallDefaultRuleParser::ConvertIpParamToConfig(NetFirewallIpParam &rule, const cJSON * const mem)
{
    cJSON *family = cJSON_GetObjectItem(mem, NET_FIREWALL_IP_FAMILY.c_str());
    if (family != nullptr && cJSON_IsNumber(family)) {
        rule.family = static_cast<uint8_t>(cJSON_GetNumberValue(family));
        NETMGR_EXT_LOG_D("family = %{public}d", rule.family);
    }
    cJSON *type = cJSON_GetObjectItem(mem, NET_FIREWALL_IP_TYPE.c_str());
    if (type != nullptr && cJSON_IsNumber(type)) {
        rule.type = static_cast<uint8_t>(cJSON_GetNumberValue(type));
        NETMGR_EXT_LOG_D("type = %{public}d", rule.type);
    }
    cJSON *address = cJSON_GetObjectItem(mem, NET_FIREWALL_IP_ADDRESS.c_str());
    if (address != nullptr && cJSON_IsString(address)) {
        rule.address = cJSON_GetStringValue(address);
        NETMGR_EXT_LOG_D("address = %{public}s", rule.address.c_str());
    }
    cJSON *mask = cJSON_GetObjectItem(mem, NET_FIREWALL_IP_MASK.c_str());
    if (mask != nullptr && cJSON_IsNumber(mask)) {
        rule.mask = static_cast<uint8_t>(cJSON_GetNumberValue(mask));
        NETMGR_EXT_LOG_D("mask = %{public}d", rule.mask);
    }
    cJSON *startIp = cJSON_GetObjectItem(mem, NET_FIREWALL_IP_START.c_str());
    if (startIp != nullptr && cJSON_IsString(startIp)) {
        rule.startIp = cJSON_GetStringValue(startIp);
        NETMGR_EXT_LOG_D("startIp = %{public}s", rule.startIp.c_str());
    }
    cJSON *endIp = cJSON_GetObjectItem(mem, NET_FIREWALL_IP_END.c_str());
    if (endIp != nullptr && cJSON_IsString(endIp)) {
        rule.endIp = cJSON_GetStringValue(endIp);
        NETMGR_EXT_LOG_D("endIp = %{public}s", rule.endIp.c_str());
    }
}

void NetFirewallDefaultRuleParser::ConvertPortParamToConfig(NetFirewallPortParam &rule, const cJSON * const mem)
{
    cJSON *startPort = cJSON_GetObjectItem(mem, NET_FIREWALL_PORT_START.c_str());
    if (startPort != nullptr && cJSON_IsNumber(startPort)) {
        rule.startPort = static_cast<uint16_t>(cJSON_GetNumberValue(startPort));
        NETMGR_EXT_LOG_D("startPort = %{public}d", rule.startPort);
    }
    cJSON *endPort = cJSON_GetObjectItem(mem, NET_FIREWALL_PORT_END.c_str());
    if (endPort != nullptr && cJSON_IsNumber(endPort)) {
        rule.endPort = static_cast<uint16_t>(cJSON_GetNumberValue(endPort));
        NETMGR_EXT_LOG_D("endPort = %{public}d", rule.endPort);
    }
}

void NetFirewallDefaultRuleParser::ConvertDomainParamToConfig(NetFirewallDomainParam &rule, const cJSON * const mem)
{
    cJSON *isWildcard = cJSON_GetObjectItem(mem, NET_FIREWALL_DOMAIN_IS_WILDCARD.c_str());
    if (isWildcard != nullptr && cJSON_IsBool(isWildcard)) {
        rule.isWildcard = cJSON_IsTrue(isWildcard);
        NETMGR_EXT_LOG_D("isWildcard = %{public}d", rule.isWildcard);
    }
    cJSON *domain = cJSON_GetObjectItem(mem, NET_FIREWALL_DOMAIN.c_str());
    if (domain != nullptr && cJSON_IsString(domain)) {
        rule.domain = cJSON_GetStringValue(domain);
        NETMGR_EXT_LOG_D("domain = %{public}s", rule.domain.c_str());
    }
}

void NetFirewallDefaultRuleParser::ConvertDnsParamToConfig(NetFirewallDnsParam &rule, const cJSON * const mem)
{
    cJSON *primaryDns = cJSON_GetObjectItem(mem, NET_FIREWALL_DNS_PRIMARY.c_str());
    if (primaryDns != nullptr && cJSON_IsString(primaryDns)) {
        rule.primaryDns = cJSON_GetStringValue(primaryDns);
        NETMGR_EXT_LOG_D("primaryDns = %{public}s", rule.primaryDns.c_str());
    }
    cJSON *standbyDns = cJSON_GetObjectItem(mem, NET_FIREWALL_DNS_STANDY.c_str());
    if (standbyDns != nullptr && cJSON_IsString(standbyDns)) {
        rule.standbyDns = cJSON_GetStringValue(standbyDns);
        NETMGR_EXT_LOG_D("standbyDns = %{public}s", rule.standbyDns.c_str());
    }
}

bool NetFirewallDefaultRuleParser::GetDefaultRules(std::vector<sptr<NetFirewallRule>> &ruleList)
{
    const auto &jsonString = ReadJsonFile(DEFAULT_RULE_FILE);
    if (jsonString.length() == 0) {
        NETMGR_EXT_LOG_E("ReadJsonFile config file is return empty!");
        return false;
    }

    NETMGR_EXT_LOG_D("Parse json %{public}s,", jsonString.c_str());
    cJSON *doc = cJSON_Parse(jsonString.c_str());
    cJSON *defaultRules = cJSON_GetObjectItem(doc, DEFAULT_RULES);
    if (defaultRules != nullptr && cJSON_IsArray(defaultRules)) {
        uint32_t itemSize = cJSON_GetArraySize(defaultRules);
        for (uint32_t i = 0; i < itemSize; i++) {
            cJSON *item = cJSON_GetArrayItem(defaultRules, i);
            if (cJSON_IsObject(item)) {
                sptr<NetFirewallRule> rule = new (std::nothrow) NetFirewallRule();
                ConvertFirewallRuleToConfig(rule, item);
                ruleList.emplace_back(std::move(rule));
            }
        }
    } else {
        NETMGR_EXT_LOG_E("Parse json error");
        cJSON_Delete(doc);
        return false;
    }
    cJSON_Delete(doc);
    return true;
}

std::string NetFirewallDefaultRuleParser::ReadJsonFile(const std::string &filePath)
{
    std::ifstream infile;
    std::string strLine;
    std::string strAll;
    infile.open(filePath);
    if (!infile.is_open()) {
        NETMGR_EXT_LOG_E("ReadJsonFile filePath failed");
        return strAll;
    }
    while (getline(infile, strLine)) {
        strAll.append(strLine);
    }
    infile.close();
    return strAll;
}
} // namespace NetManagerStandard
} // namespace OHOS
