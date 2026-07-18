/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include "net_firewall_param_check.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <regex>

#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "wrapper.rs.h"

namespace OHOS {
namespace NetManagerAni {
using namespace NetManagerStandard;

namespace {
constexpr int32_t MAX_RULE_PORT = 65535;

inline const std::regex DOMAIN_PATTERN { "^([a-zA-Z0-9][-a-zA-Z0-9]{0,62}(\\.[a-zA-Z0-9][-a-zA-Z0-9]{0,62})+\\.?)$" };
inline const std::regex WILDCARD_DOMAIN_PATTERN {
    "^(([a-zA-Z0-9][-a-zA-Z0-9]{0,62}|\\*)(\\.[a-zA-Z0-9][-a-zA-Z0-9]{0,62})+\\.?)$"
};
inline const std::regex INTERFACE_NAME_PATTERN { "^[a-zA-Z][a-zA-Z0-9_-]{0,14}$" };
} //namesapce

int32_t NetFirewallParamCheck::CheckFirewallRulePolicy(const NetFirewallPolicyAni &policyAni)
{
    FirewallRuleAction inAction = static_cast<FirewallRuleAction>(policyAni.in_action);
    if (inAction != FirewallRuleAction::RULE_ALLOW && inAction != FirewallRuleAction::RULE_DENY) {
        NETMANAGER_EXT_LOGE("params inAction invalid");
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    FirewallRuleAction outAction = static_cast<FirewallRuleAction>(policyAni.out_action);
    if (outAction != FirewallRuleAction::RULE_ALLOW && outAction != FirewallRuleAction::RULE_DENY) {
        NETMANAGER_EXT_LOGE("params outAction invalid");
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallParamCheck::CheckUpdateFirewallRule(const NetFirewallRuleAni &ruleAni)
{
    if (ruleAni.id < 0 || ruleAni.id > INT32_MAX) {
        NETMANAGER_EXT_LOGE("invalid ruleId");
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    return CheckFirewallRule(ruleAni);
}

int32_t NetFirewallParamCheck::CheckRuleBaseParam(const NetFirewallRuleAni &ruleAni, NetFirewallRuleType ruleType)
{
    NetFirewallRuleDirection direction = static_cast<NetFirewallRuleDirection>(ruleAni.direction);
    if (direction != NetFirewallRuleDirection::RULE_IN && direction != NetFirewallRuleDirection::RULE_OUT) {
        NETMANAGER_EXT_LOGE("params direction invalid");
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    FirewallRuleAction action = static_cast<FirewallRuleAction>(ruleAni.action);
    if (action != FirewallRuleAction::RULE_ALLOW && action != FirewallRuleAction::RULE_DENY) {
        NETMANAGER_EXT_LOGE("params action invalid");
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    if (ruleAni.app_uid < 0) {
        NETMANAGER_EXT_LOGE("invalid appUid");
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    if (ruleAni.user_id < 0) {
        NETMANAGER_EXT_LOGE("invalid userId");
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    if (ruleType == NetFirewallRuleType::RULE_IP && ruleAni.protocol != 0) {
        NetworkProtocol protocol = static_cast<NetworkProtocol>(ruleAni.protocol);
        if (protocol != NetworkProtocol::ICMP && protocol != NetworkProtocol::TCP &&
            protocol != NetworkProtocol::UDP && protocol != NetworkProtocol::ICMPV6 &&
            protocol != NetworkProtocol::SAT_EXPAK) {
            NETMANAGER_EXT_LOGE("params protocol invalid");
            return FIREWALL_ERR_INVALID_PARAMETER;
        }
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallParamCheck::CheckRuleStringParam(const NetFirewallRuleAni &ruleAni)
{
    std::string ruleName(ruleAni.name);
    if (ruleName.empty() || ruleName.size() > MAX_RULE_NAME_LEN) {
        NETMANAGER_EXT_LOGE("ruleName is empty or too long");
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    std::string ruleDescription(ruleAni.description);
    if (ruleDescription.size() > MAX_RULE_DESCRIPTION_LEN) {
        NETMANAGER_EXT_LOGE("ruleDescription is too long");
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    std::string interfaceName(ruleAni.iface);
    if (!interfaceName.empty()) {
        NETMANAGER_EXT_LOGI("CheckRuleStringParam interfaceName:%{public}s", interfaceName.c_str());
        return CheckInterfaceName(interfaceName);
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallParamCheck::CheckRuleObjectParams(const NetFirewallRuleAni &ruleAni, NetFirewallRuleType ruleType)
{
    if (ruleType == NetFirewallRuleType::RULE_IP) {
        int32_t ret = CheckIpList(ruleAni.local_ips);
        if (ret != FIREWALL_SUCCESS) {
            return ret;
        }
        ret = CheckIpList(ruleAni.remote_ips);
        if (ret != FIREWALL_SUCCESS) {
            return ret;
        }
        ret = CheckPortList(ruleAni.local_ports);
        if (ret != FIREWALL_SUCCESS) {
            return ret;
        }
        return CheckPortList(ruleAni.remote_ports);
    }
    if (ruleType == NetFirewallRuleType::RULE_DOMAIN) {
        return CheckDomainList(ruleAni.domains);
    }
    return CheckFirewallDns(ruleAni.dns);
}

int32_t NetFirewallParamCheck::CheckFirewallRule(const NetFirewallRuleAni &ruleAni)
{
    NetFirewallRuleType ruleType = static_cast<NetFirewallRuleType>(ruleAni.type_);
    if (ruleType != NetFirewallRuleType::RULE_IP && ruleType != NetFirewallRuleType::RULE_DOMAIN &&
        ruleType != NetFirewallRuleType::RULE_DNS) {
        NETMANAGER_EXT_LOGE("params ruleType invalid");
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    int32_t ret = CheckRuleBaseParam(ruleAni, ruleType);
    if (ret != FIREWALL_SUCCESS) {
        return ret;
    }
    ret = CheckRuleStringParam(ruleAni);
    if (ret != FIREWALL_SUCCESS) {
        return ret;
    }
    return CheckRuleObjectParams(ruleAni, ruleType);
}

int32_t NetFirewallParamCheck::CheckFirewallDns(const NetFirewallDnsParamAni &dnsAni)
{
    std::string primaryDns(dnsAni.primary_dns);
    if (primaryDns.empty()) {
        NETMANAGER_EXT_LOGE("primary dns is null, params invalid");
        return FIREWALL_ERR_PARAMETER_ERROR;
    }
    int32_t ret = CheckIpAddress(primaryDns);
    if (ret != FIREWALL_SUCCESS) {
        return ret;
    }
    return CheckIpAddress(std::string(dnsAni.standby_dns));
}

int32_t NetFirewallParamCheck::CheckIpList(const rust::Vec<NetFirewallIpParamsAni> &ipsAni)
{
    for (const auto &ip : ipsAni) {
        int32_t family = ip.family == 0 ? FAMILY_IPV4 : ip.family;
        if (family != FAMILY_IPV4 && family != FAMILY_IPV6) {
            NETMANAGER_EXT_LOGE("family param invalid.");
            return FIREWALL_ERR_INVALID_PARAMETER;
        }
        int32_t type = ip.ip_type == 0 ? SINGLE_IP : ip.ip_type;
        if (type != SINGLE_IP && type != MULTIPLE_IP) {
            NETMANAGER_EXT_LOGE("params ip type invalid");
            return FIREWALL_ERR_INVALID_PARAMETER;
        }
        int32_t ret = type == MULTIPLE_IP ? CheckMultipleIp(ip, family) : CheckSingeIp(ip, family);
        if (ret != FIREWALL_SUCCESS) {
            return ret;
        }
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallParamCheck::CheckPortList(const rust::Vec<NetFirewallPortParamAni> &portsAni)
{
    for (const auto &port : portsAni) {
        if (port.start_port != 0 && (port.start_port < 1 || port.start_port > MAX_RULE_PORT)) {
            NETMANAGER_EXT_LOGE("start port is more then %{public}d", MAX_RULE_PORT);
            return FIREWALL_ERR_INVALID_PARAMETER;
        }
        if (port.end_port != 0 && (port.end_port > MAX_RULE_PORT || port.end_port < port.start_port)) {
            NETMANAGER_EXT_LOGE("end port is more then %{public}d", MAX_RULE_PORT);
            return FIREWALL_ERR_INVALID_PARAMETER;
        }
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallParamCheck::CheckDomainList(const rust::Vec<NetFirewallDomainParamAni> &domainsAni)
{
    for (const auto &domain : domainsAni) {
        std::string domainStr(domain.domain);
        size_t maxSize = domain.is_wildcard ? MAX_FUZZY_DOMAIN_NAME_LEN : MAX_EXACT_DOMAIN_NAME_LEN;
        if (domainStr.empty() || domainStr.size() > maxSize) {
            NETMANAGER_EXT_LOGE("domain is empty or length more than %{public}zu", maxSize);
            return FIREWALL_ERR_INVALID_PARAMETER;
        }
        std::regex pattern = domain.is_wildcard ? WILDCARD_DOMAIN_PATTERN : DOMAIN_PATTERN;
        if (!std::regex_match(domainStr, pattern)) {
            NETMANAGER_EXT_LOGE("invalid domain address");
            return FIREWALL_ERR_INVALID_PARAMETER;
        }
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
    if (family == FAMILY_IPV6) {
        in6_addr start;
        in6_addr end;
        if (inet_pton(AF_INET6, startIp.c_str(), &start) <= 0 || inet_pton(AF_INET6, endIp.c_str(), &end) <= 0) {
            NETMANAGER_EXT_LOGE("CheckIpAddress ipv6: startIp or endIp is invalid");
            return false;
        }
        if (memcmp(&start, &end, sizeof(start)) > 0) {
            NETMANAGER_EXT_LOGE("CheckIpAddress ipv6: start Ip is larger than endIp");
            return false;
        }
        return true;
    }
    in_addr start;
    in_addr end;
    if (inet_pton(AF_INET, startIp.c_str(), &start) <= 0 || inet_pton(AF_INET, endIp.c_str(), &end) <= 0) {
        NETMANAGER_EXT_LOGE("CheckIpAddress ipv4: startIp or endIp is invalid");
        return false;
    }
    if (memcmp(&start, &end, sizeof(start)) > 0) {
        NETMANAGER_EXT_LOGE("CheckIpAddress ipv4: start Ip is larger than endIp");
        return false;
    }
    return true;
}

int32_t NetFirewallParamCheck::CheckIpV4(const std::string &ipV4)
{
    in_addr v4Addr;
    if (inet_pton(AF_INET, ipV4.c_str(), &v4Addr) <= 0) {
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallParamCheck::CheckIpV6(const std::string &ipV6)
{
    in6_addr v6Addr;
    if (inet_pton(AF_INET6, ipV6.c_str(), &v6Addr) <= 0) {
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallParamCheck::CheckSingeIp(const NetFirewallIpParamsAni &ipAni, int32_t family)
{
    int32_t ret = CheckIpAddress(std::string(ipAni.address), family);
    if (ret != FIREWALL_SUCCESS) {
        return ret;
    }
    if (ipAni.mask != 0) {
        int32_t maskMax = family == FAMILY_IPV4 ? IPV4_MASK_MAX : IPV6_MASK_MAX;
        if (ipAni.mask < 0 || ipAni.mask > maskMax) {
            NETMANAGER_EXT_LOGE("params mask is invalid");
            return FIREWALL_ERR_INVALID_PARAMETER;
        }
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallParamCheck::CheckMultipleIp(const NetFirewallIpParamsAni &ipAni, int32_t family)
{
    if (CheckIpAddress(std::string(ipAni.start_ip), std::string(ipAni.end_ip), family)) {
        return FIREWALL_SUCCESS;
    }
    return FIREWALL_ERR_INVALID_PARAMETER;
}

int32_t NetFirewallParamCheck::CheckInterfaceName(const std::string &interfaceName)
{
    if (interfaceName.empty()) {
        NETMANAGER_EXT_LOGE("interface name is empty");
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    if (interfaceName.size() > MAX_INTERFACE_NAME_LEN) {
        NETMANAGER_EXT_LOGE("interface name length more than %{public}d", MAX_INTERFACE_NAME_LEN);
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    if (!std::regex_match(interfaceName, INTERFACE_NAME_PATTERN)) {
        NETMANAGER_EXT_LOGE("interface name format invalid: %{public}s", interfaceName.c_str());
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    return FIREWALL_SUCCESS;
}
} // namespace NetManagerAni
} // namespace OHOS
