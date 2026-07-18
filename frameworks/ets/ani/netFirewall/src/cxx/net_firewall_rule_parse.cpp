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

#include "net_firewall_rule_parse.h"

#include <arpa/inet.h>
#include <netinet/in.h>

#include <string>

#include "netfirewall_common.h"
#include "netfirewall_parcel.h"
#include "wrapper.rs.h"

namespace OHOS {
namespace NetManagerAni {
using namespace NetManagerStandard;

void NetFirewallRuleParse::ParseIpList(const rust::Vec<NetFirewallIpParamsAni> &ipsAni,
    std::vector<NetFirewallIpParam> &ips)
{
    ips.clear();
    for (const auto &ipAni : ipsAni) {
        NetFirewallIpParam ip;
        int32_t family = ipAni.family == 0 ? FAMILY_IPV4 : ipAni.family;
        ip.family = static_cast<uint8_t>(family);
        ip.type = static_cast<uint8_t>(ipAni.ip_type == 0 ? SINGLE_IP : ipAni.ip_type);
        if (ip.type == SINGLE_IP) {
            ip.mask = ipAni.mask == 0
                ? static_cast<uint8_t>(family == FAMILY_IPV4 ? IPV4_MASK_MAX : IPV6_MASK_MAX)
                : static_cast<uint8_t>(ipAni.mask);
        } else {
            ip.mask = 0;
        }
        std::string startIp;
        std::string endIp;
        if (ip.type == SINGLE_IP) {
            startIp = std::string(ipAni.address);
        } else {
            startIp = std::string(ipAni.start_ip);
            endIp = std::string(ipAni.end_ip);
        }
        if (family == FAMILY_IPV6) {
            if (inet_pton(AF_INET6, startIp.c_str(), &ip.ipv6.startIp) <= 0) {
                ip.ipv6.startIp = IN6ADDR_ANY_INIT;
            }
            if (inet_pton(AF_INET6, endIp.c_str(), &ip.ipv6.endIp) <= 0) {
                ip.ipv6.endIp = IN6ADDR_ANY_INIT;
            }
        } else {
            if (inet_pton(AF_INET, startIp.c_str(), &ip.ipv4.startIp) <= 0) {
                ip.ipv4.startIp.s_addr = INADDR_ANY;
            }
            if (inet_pton(AF_INET, endIp.c_str(), &ip.ipv4.endIp) <= 0) {
                ip.ipv4.endIp.s_addr = INADDR_ANY;
            }
        }
        ips.emplace_back(ip);
    }
}

void NetFirewallRuleParse::ParsePortList(const rust::Vec<NetFirewallPortParamAni> &portsAni,
    std::vector<NetFirewallPortParam> &ports)
{
    ports.clear();
    for (const auto &portAni : portsAni) {
        NetFirewallPortParam port;
        port.startPort = static_cast<uint16_t>(portAni.start_port);
        port.endPort = static_cast<uint16_t>(portAni.end_port);
        ports.emplace_back(port);
    }
}

void NetFirewallRuleParse::ParseDomainList(const rust::Vec<NetFirewallDomainParamAni> &domainsAni,
    std::vector<NetFirewallDomainParam> &domains)
{
    domains.clear();
    for (const auto &domainAni : domainsAni) {
        NetFirewallDomainParam domain;
        domain.isWildcard = domainAni.is_wildcard;
        domain.domain = domainAni.domain.empty() ? std::string() : std::string(domainAni.domain);
        domains.emplace_back(domain);
    }
}

void NetFirewallRuleParse::ParseDns(const NetFirewallDnsParamAni &dnsAni, NetFirewallDnsParam &dns)
{
    dns.primaryDns = dnsAni.primary_dns.empty() ? std::string() : std::string(dnsAni.primary_dns);
    dns.standbyDns = dnsAni.standby_dns.empty() ? std::string() : std::string(dnsAni.standby_dns);
}

void NetFirewallRuleParse::ParseRuleParams(const NetFirewallRuleAni &ruleAni, const sptr<NetFirewallRule> &rule)
{
    if (rule == nullptr) {
        return;
    }
    rule->ruleId = ruleAni.id;
    rule->ruleName = ruleAni.name.empty() ? std::string() : std::string(ruleAni.name);
    rule->ruleDescription = ruleAni.description.empty() ? std::string() : std::string(ruleAni.description);
    rule->ruleDirection = static_cast<NetFirewallRuleDirection>(ruleAni.direction);
    rule->ruleAction = static_cast<FirewallRuleAction>(ruleAni.action);
    rule->ruleType = static_cast<NetFirewallRuleType>(ruleAni.type_);
    rule->isEnabled = ruleAni.is_enabled;
    rule->appUid = ruleAni.app_uid;
    rule->protocol = static_cast<NetworkProtocol>(ruleAni.protocol);
    rule->userId = ruleAni.user_id;

    ParseIpList(ruleAni.local_ips, rule->localIps);
    ParseIpList(ruleAni.remote_ips, rule->remoteIps);
    ParsePortList(ruleAni.local_ports, rule->localPorts);
    ParsePortList(ruleAni.remote_ports, rule->remotePorts);
    ParseDomainList(ruleAni.domains, rule->domains);
    if (!ruleAni.dns.primary_dns.empty() || !ruleAni.dns.standby_dns.empty()) {
        ParseDns(ruleAni.dns, rule->dns);
    }
    if (rule->ruleType == NetFirewallRuleType::RULE_IP && !ruleAni.iface.empty()) {
        rule->interface = std::string(ruleAni.iface);
    }
}

void NetFirewallRuleParse::ConvertInterceptRecord(const InterceptRecord &record, InterceptedRecordAni &recordAni)
{
    recordAni.time = static_cast<uint64_t>(record.time);
    recordAni.local_ip = rust::String(record.localIp.c_str());
    recordAni.remote_ip = rust::String(record.remoteIp.c_str());
    recordAni.local_port = static_cast<int32_t>(record.localPort);
    recordAni.remote_port = static_cast<int32_t>(record.remotePort);
    recordAni.protocol = static_cast<int32_t>(record.protocol);
    recordAni.app_uid = record.appUid;
    recordAni.domain = rust::String(record.domain.c_str());
}

void NetFirewallRuleParse::ConvertInterceptRecordPage(const sptr<InterceptRecordPage> &pageInfo,
    InterceptedRecordPageAni &pageAni)
{
    if (pageInfo == nullptr) {
        return;
    }
    pageAni.page = pageInfo->page;
    pageAni.page_size = pageInfo->pageSize;
    pageAni.total_page = pageInfo->totalPage;
    pageAni.data.clear();
    for (const auto &record : pageInfo->data) {
        InterceptedRecordAni recordAni;
        ConvertInterceptRecord(record, recordAni);
        pageAni.data.push_back(std::move(recordAni));
    }
}

static void ConvertIpParam(const NetFirewallIpParam &ip, NetFirewallIpParamsAni &ipAni)
{
    ipAni.family = static_cast<int32_t>(ip.family);
    ipAni.ip_type = static_cast<int32_t>(ip.type);
    if (ip.type == SINGLE_IP) {
        ipAni.mask = static_cast<int32_t>(ip.mask);
        ipAni.address = rust::String(ip.GetStartIp().c_str());
        ipAni.start_ip = rust::String();
        ipAni.end_ip = rust::String();
    } else {
        ipAni.mask = 0;
        ipAni.address = rust::String();
        ipAni.start_ip = rust::String(ip.GetStartIp().c_str());
        ipAni.end_ip = rust::String(ip.GetEndIp().c_str());
    }
}

static void ConvertPortParam(const NetFirewallPortParam &port, NetFirewallPortParamAni &portAni)
{
    portAni.start_port = static_cast<int32_t>(port.startPort);
    portAni.end_port = static_cast<int32_t>(port.endPort);
}

static void ConvertDomainParam(const NetFirewallDomainParam &domain, NetFirewallDomainParamAni &domaniAni)
{
    domaniAni.is_wildcard = domain.isWildcard;
    domaniAni.domain = rust::String(domain.domain.c_str());
}

static void ConvertDnsParam(const NetFirewallDnsParam &dns, NetFirewallDnsParamAni &dnsAni)
{
    dnsAni.primary_dns = rust::String(dns.primaryDns.c_str());
    dnsAni.standby_dns = rust::String(dns.standbyDns.c_str());
}

void NetFirewallRuleParse::ConvertNetFirewallRule(const NetFirewallRule &rule, NetFirewallRuleAni &ruleAni)
{
    ruleAni.id = rule.ruleId;
    ruleAni.name = rust::String(rule.ruleName.c_str());
    ruleAni.description = rust::String(rule.ruleDescription.c_str());
    ruleAni.direction = static_cast<int32_t>(rule.ruleDirection);
    ruleAni.action = static_cast<int32_t>(rule.ruleAction);
    ruleAni.type_ = static_cast<int32_t>(rule.ruleType);
    ruleAni.is_enabled = rule.isEnabled;
    ruleAni.app_uid = rule.appUid;
    ruleAni.protocol = static_cast<int32_t>(rule.protocol);
    ruleAni.user_id = rule.userId;
    ruleAni.local_ips.clear();
    for (const auto &ip : rule.localIps) {
        NetFirewallIpParamsAni ipAni;
        ConvertIpParam(ip, ipAni);
        ruleAni.local_ips.push_back(std::move(ipAni));
    }
    ruleAni.remote_ips.clear();
    for (const auto &ip : rule.remoteIps) {
        NetFirewallIpParamsAni ipAni;
        ConvertIpParam(ip, ipAni);
        ruleAni.remote_ips.push_back(std::move(ipAni));
    }
    ruleAni.local_ports.clear();
    for (const auto &port : rule.localPorts) {
        NetFirewallPortParamAni portAni;
        ConvertPortParam(port, portAni);
        ruleAni.local_ports.push_back(std::move(portAni));
    }
    ruleAni.remote_ports.clear();
    for (const auto &port : rule.remotePorts) {
        NetFirewallPortParamAni portAni;
        ConvertPortParam(port, portAni);
        ruleAni.remote_ports.push_back(std::move(portAni));
    }
    ruleAni.domains.clear();
    for (const auto &domain : rule.domains) {
        NetFirewallDomainParamAni domainAni;
        ConvertDomainParam(domain, domainAni);
        ruleAni.domains.push_back(std::move(domainAni));
    }
    ConvertDnsParam(rule.dns, ruleAni.dns);
    if (rule.ruleType == NetFirewallRuleType::RULE_IP) {
        ruleAni.iface = rust::String(rule.interface.c_str());
    } else {
        ruleAni.iface = rust::String();
    }
}
} // namespace NetManagerAni
} // namespace OHOS
