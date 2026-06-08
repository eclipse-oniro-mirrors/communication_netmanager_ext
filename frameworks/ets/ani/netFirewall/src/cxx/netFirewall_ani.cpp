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

#include "netFirewall_ani.h"

#include <arpa/inet.h>
#include <netinet/in.h>

#include "cxx.h"
#include "net_manager_constants.h"
#include "netfirewall_client.h"
#include "netfirewall_common.h"
#include "singleton.h"
#include "wrapper.rs.h"

namespace OHOS {
namespace NetManagerStandard {

// ==================== Conversion Helpers ====================

static void ConvertFFIToCppPolicy(const NetFirewallPolicyFFI &ffi, sptr<NetFirewallPolicy> &policy)
{
    if (policy == nullptr) {
        policy = new (std::nothrow) NetFirewallPolicy();
    }
    if (policy == nullptr) {
        return;
    }
    policy->isOpen = ffi.is_open;
    policy->inAction = static_cast<FirewallRuleAction>(ffi.in_action);
    policy->outAction = static_cast<FirewallRuleAction>(ffi.out_action);
}

static void ConvertCppToFFIPolicy(const NetFirewallPolicy &policy, NetFirewallPolicyFFI &ffi)
{
    ffi.is_open = policy.isOpen;
    ffi.in_action = static_cast<int32_t>(policy.inAction);
    ffi.out_action = static_cast<int32_t>(policy.outAction);
}

static void ConvertFFIToCppRequestParam(const RequestParamFFI &ffi, sptr<RequestParam> &param)
{
    if (param == nullptr) {
        param = new (std::nothrow) RequestParam();
    }
    if (param == nullptr) {
        return;
    }
    param->page = ffi.page;
    param->pageSize = ffi.page_size;
    param->orderField = static_cast<NetFirewallOrderField>(ffi.order_field);
    param->orderType = static_cast<NetFirewallOrderType>(ffi.order_type);
}

static void ConvertFFIIpParamToCpp(const NetFirewallIpParamFFI &ffi, NetFirewallIpParam &ip)
{
    ip.family = ffi.family;
    ip.type = ffi.ip_type;
    ip.mask = ffi.mask;

    std::string startIp = ffi.start_ip.empty() ? std::string() : std::string(ffi.start_ip);
    std::string endIp = ffi.end_ip.empty() ? std::string() : std::string(ffi.end_ip);

    if (ip.family == FAMILY_IPV6) {
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
}

static void ConvertCppIpParamToFFI(const NetFirewallIpParam &ip, NetFirewallIpParamFFI &ffi)
{
    ffi.family = ip.family;
    ffi.ip_type = ip.type;
    ffi.mask = ip.mask;
    ffi.start_ip = rust::String(ip.GetStartIp().c_str());
    ffi.end_ip = rust::String(ip.GetEndIp().c_str());
}

static void ConvertFFIPortParamToCpp(const NetFirewallPortParamFFI &ffi, NetFirewallPortParam &port)
{
    port.startPort = ffi.start_port;
    port.endPort = ffi.end_port;
}

static void ConvertCppPortParamToFFI(const NetFirewallPortParam &port, NetFirewallPortParamFFI &ffi)
{
    ffi.start_port = port.startPort;
    ffi.end_port = port.endPort;
}

static void ConvertFFIDomainParamToCpp(const NetFirewallDomainParamFFI &ffi, NetFirewallDomainParam &domain)
{
    domain.isWildcard = ffi.is_wildcard;
    domain.domain = ffi.domain.empty() ? std::string() : std::string(ffi.domain);
}

static void ConvertCppDomainParamToFFI(const NetFirewallDomainParam &domain, NetFirewallDomainParamFFI &ffi)
{
    ffi.is_wildcard = domain.isWildcard;
    ffi.domain = rust::String(domain.domain.c_str());
}

static void ConvertFFIDnsParamToCpp(const NetFirewallDnsParamFFI &ffi, NetFirewallDnsParam &dns)
{
    dns.primaryDns = ffi.primary_dns.empty() ? std::string() : std::string(ffi.primary_dns);
    dns.standbyDns = ffi.standby_dns.empty() ? std::string() : std::string(ffi.standby_dns);
}

static void ConvertCppDnsParamToFFI(const NetFirewallDnsParam &dns, NetFirewallDnsParamFFI &ffi)
{
    ffi.has_dns = !dns.primaryDns.empty() || !dns.standbyDns.empty();
    ffi.primary_dns = rust::String(dns.primaryDns.c_str());
    ffi.standby_dns = rust::String(dns.standbyDns.c_str());
}

template<typename CppType, typename FFIType>
static void ConvertFFIVectorToCpp(const rust::Vec<FFIType> &src, std::vector<CppType> &dst,
                                  void (*convert)(const FFIType &, CppType &))
{
    dst.clear();
    for (const auto &item : src) {
        CppType cppItem;
        convert(item, cppItem);
        dst.push_back(std::move(cppItem));
    }
}

static void ConvertFFIRuleToCpp(const NetFirewallRuleFFI &ffi, sptr<NetFirewallRule> &rule)
{
    if (rule == nullptr) {
        rule = new (std::nothrow) NetFirewallRule();
    }
    if (rule == nullptr) {
        return;
    }
    rule->ruleId = ffi.rule_id;
    rule->ruleName = std::string(ffi.rule_name);
    rule->ruleDescription = std::string(ffi.rule_description);
    rule->ruleDirection = static_cast<NetFirewallRuleDirection>(ffi.rule_direction);
    rule->ruleAction = static_cast<FirewallRuleAction>(ffi.rule_action);
    rule->ruleType = static_cast<NetFirewallRuleType>(ffi.rule_type);
    rule->isEnabled = ffi.is_enabled;
    rule->appUid = ffi.app_uid;
    rule->protocol = static_cast<NetworkProtocol>(ffi.protocol);
    rule->userId = ffi.user_id;

    ConvertFFIVectorToCpp(ffi.local_ips, rule->localIps, ConvertFFIIpParamToCpp);
    ConvertFFIVectorToCpp(ffi.remote_ips, rule->remoteIps, ConvertFFIIpParamToCpp);
    ConvertFFIVectorToCpp(ffi.local_ports, rule->localPorts, ConvertFFIPortParamToCpp);
    ConvertFFIVectorToCpp(ffi.remote_ports, rule->remotePorts, ConvertFFIPortParamToCpp);
    ConvertFFIVectorToCpp(ffi.domains, rule->domains, ConvertFFIDomainParamToCpp);

    if (ffi.dns.has_dns) {
        ConvertFFIDnsParamToCpp(ffi.dns, rule->dns);
    }
}

static void ConvertCppRuleToFFI(const NetFirewallRule &rule, NetFirewallRuleResultFFI &ffi)
{
    ffi.rule_id = rule.ruleId;
    ffi.rule_name = rust::String(rule.ruleName.c_str());
    ffi.rule_description = rust::String(rule.ruleDescription.c_str());
    ffi.rule_direction = static_cast<int32_t>(rule.ruleDirection);
    ffi.rule_action = static_cast<int32_t>(rule.ruleAction);
    ffi.rule_type = static_cast<int32_t>(rule.ruleType);
    ffi.is_enabled = rule.isEnabled;
    ffi.app_uid = rule.appUid;
    ffi.protocol = static_cast<uint8_t>(rule.protocol);
    ffi.user_id = rule.userId;

    ffi.local_ips.clear();
    for (const auto &ip : rule.localIps) {
        NetFirewallIpParamFFI ffiIp;
        ConvertCppIpParamToFFI(ip, ffiIp);
        ffi.local_ips.push_back(std::move(ffiIp));
    }

    ffi.remote_ips.clear();
    for (const auto &ip : rule.remoteIps) {
        NetFirewallIpParamFFI ffiIp;
        ConvertCppIpParamToFFI(ip, ffiIp);
        ffi.remote_ips.push_back(std::move(ffiIp));
    }

    ffi.local_ports.clear();
    for (const auto &port : rule.localPorts) {
        NetFirewallPortParamFFI ffiPort;
        ConvertCppPortParamToFFI(port, ffiPort);
        ffi.local_ports.push_back(std::move(ffiPort));
    }

    ffi.remote_ports.clear();
    for (const auto &port : rule.remotePorts) {
        NetFirewallPortParamFFI ffiPort;
        ConvertCppPortParamToFFI(port, ffiPort);
        ffi.remote_ports.push_back(std::move(ffiPort));
    }

    ffi.domains.clear();
    for (const auto &domain : rule.domains) {
        NetFirewallDomainParamFFI ffiDomain;
        ConvertCppDomainParamToFFI(domain, ffiDomain);
        ffi.domains.push_back(std::move(ffiDomain));
    }

    ConvertCppDnsParamToFFI(rule.dns, ffi.dns);
}

static void ConvertCppRecordToFFI(const InterceptRecord &record, InterceptedRecordFFI &ffi)
{
    ffi.time = static_cast<int64_t>(record.time);
    ffi.local_ip = rust::String(record.localIp.c_str());
    ffi.remote_ip = rust::String(record.remoteIp.c_str());
    ffi.local_port = static_cast<int32_t>(record.localPort);
    ffi.remote_port = static_cast<int32_t>(record.remotePort);
    ffi.protocol = static_cast<int32_t>(record.protocol);
    ffi.app_uid = record.appUid;
    ffi.domain = rust::String(record.domain.c_str());
}

// ==================== FFI Function Implementations ====================

int32_t FFISetNetFirewallPolicy(int32_t userId, const NetFirewallPolicyFFI &ffiPolicy)
{
    if (userId < 0) {
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    sptr<NetFirewallPolicy> policy = new (std::nothrow) NetFirewallPolicy();
    if (policy == nullptr) {
        return FIREWALL_ERR_INTERNAL;
    }
    ConvertFFIToCppPolicy(ffiPolicy, policy);
    int32_t ret = DelayedSingleton<NetFirewallClient>::GetInstance()->SetNetFirewallPolicy(userId, policy);
    // sptr reference counting handles cleanup automatically when policy goes out of scope
    return ret;
}

int32_t FFIGetNetFirewallPolicy(int32_t userId, NetFirewallPolicyFFI &ffiPolicy)
{
    if (userId < 0) {
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    sptr<NetFirewallPolicy> policy = new (std::nothrow) NetFirewallPolicy();
    if (policy == nullptr) {
        return FIREWALL_ERR_INTERNAL;
    }
    int32_t ret = DelayedSingleton<NetFirewallClient>::GetInstance()->GetNetFirewallPolicy(userId, policy);
    if (ret == FIREWALL_SUCCESS) {
        ConvertCppToFFIPolicy(*policy, ffiPolicy);
    }
    // sptr reference counting handles cleanup automatically when policy goes out of scope
    return ret;
}

// FFIAddNetFirewallRule: the int32_t &result out-parameter is backed by a Rust &mut i32
// allocated on the caller's stack. cxx::bridge guarantees the reference is valid for the
// duration of the call, so no additional null/initialization check is needed here.
int32_t FFIAddNetFirewallRule(const NetFirewallRuleFFI &ffiRule, int32_t &result)
{
    sptr<NetFirewallRule> rule = new (std::nothrow) NetFirewallRule();
    if (rule == nullptr) {
        return FIREWALL_ERR_INTERNAL;
    }
    ConvertFFIRuleToCpp(ffiRule, rule);
    int32_t ret = DelayedSingleton<NetFirewallClient>::GetInstance()->AddNetFirewallRule(rule, result);
    // sptr reference counting handles cleanup automatically when rule goes out of scope
    return ret;
}

int32_t FFIUpdateNetFirewallRule(const NetFirewallRuleFFI &ffiRule)
{
    sptr<NetFirewallRule> rule = new (std::nothrow) NetFirewallRule();
    if (rule == nullptr) {
        return FIREWALL_ERR_INTERNAL;
    }
    ConvertFFIRuleToCpp(ffiRule, rule);
    int32_t ret = DelayedSingleton<NetFirewallClient>::GetInstance()->UpdateNetFirewallRule(rule);
    // sptr reference counting handles cleanup automatically when rule goes out of scope
    return ret;
}

int32_t FFIRemoveNetFirewallRule(int32_t userId, int32_t ruleId)
{
    if (userId < 0 || ruleId < 0) {
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    int32_t ret = DelayedSingleton<NetFirewallClient>::GetInstance()->DeleteNetFirewallRule(userId, ruleId);
    return ret;
}

int32_t FFIGetNetFirewallRules(int32_t userId, const RequestParamFFI &ffiParam, FirewallRulePageFFI &ffiPage)
{
    if (userId < 0 || ffiParam.page < 0 || ffiParam.page_size <= 0) {
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    sptr<RequestParam> param = new (std::nothrow) RequestParam();
    if (param == nullptr) {
        return FIREWALL_ERR_INTERNAL;
    }
    ConvertFFIToCppRequestParam(ffiParam, param);

    sptr<FirewallRulePage> pageInfo = new (std::nothrow) FirewallRulePage();
    if (pageInfo == nullptr) {
        return FIREWALL_ERR_INTERNAL;
    }

    int32_t ret = DelayedSingleton<NetFirewallClient>::GetInstance()->GetNetFirewallRules(
        userId, param, pageInfo);
    if (ret == FIREWALL_SUCCESS && pageInfo != nullptr) {
        ffiPage.page = pageInfo->page;
        ffiPage.page_size = pageInfo->pageSize;
        ffiPage.total_page = pageInfo->totalPage;
        ffiPage.data.clear();
        for (const auto &rule : pageInfo->data) {
            NetFirewallRuleResultFFI ffiRule;
            ConvertCppRuleToFFI(rule, ffiRule);
            ffiPage.data.push_back(std::move(ffiRule));
        }
    }
    // sptr reference counting handles cleanup automatically when param/pageInfo go out of scope
    return ret;
}

int32_t FFIGetNetFirewallRule(int32_t userId, int32_t ruleId, NetFirewallRuleResultFFI &ffiRule)
{
    if (userId < 0 || ruleId < 0) {
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    sptr<NetFirewallRule> rule = new (std::nothrow) NetFirewallRule();
    if (rule == nullptr) {
        return FIREWALL_ERR_INTERNAL;
    }

    int32_t ret = DelayedSingleton<NetFirewallClient>::GetInstance()->GetNetFirewallRule(userId, ruleId, rule);
    if (ret == FIREWALL_SUCCESS && rule != nullptr) {
        ConvertCppRuleToFFI(*rule, ffiRule);
    }
    // sptr reference counting handles cleanup automatically when rule goes out of scope
    return ret;
}

int32_t FFIGetInterceptedRecords(int32_t userId, const RequestParamFFI &ffiParam, InterceptRecordPageFFI &ffiPage)
{
    if (userId < 0 || ffiParam.page < 0 || ffiParam.page_size <= 0) {
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    sptr<RequestParam> param = new (std::nothrow) RequestParam();
    if (param == nullptr) {
        return FIREWALL_ERR_INTERNAL;
    }
    ConvertFFIToCppRequestParam(ffiParam, param);

    sptr<InterceptRecordPage> pageInfo = new (std::nothrow) InterceptRecordPage();
    if (pageInfo == nullptr) {
        return FIREWALL_ERR_INTERNAL;
    }

    int32_t ret = DelayedSingleton<NetFirewallClient>::GetInstance()->GetInterceptRecords(
        userId, param, pageInfo);
    if (ret == FIREWALL_SUCCESS && pageInfo != nullptr) {
        ffiPage.page = pageInfo->page;
        ffiPage.page_size = pageInfo->pageSize;
        ffiPage.total_page = pageInfo->totalPage;
        ffiPage.data.clear();
        for (const auto &record : pageInfo->data) {
            InterceptedRecordFFI ffiRecord;
            ConvertCppRecordToFFI(record, ffiRecord);
            ffiPage.data.push_back(std::move(ffiRecord));
        }
    }
    // sptr reference counting handles cleanup automatically when param/pageInfo go out of scope
    return ret;
}

} // namespace NetManagerStandard
} // namespace OHOS
