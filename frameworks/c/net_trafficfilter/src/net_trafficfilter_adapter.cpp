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

#include <arpa/inet.h>
#include <cstddef>
#include <cstring>
#include <string>
#include "net_trafficfilter_adapter.h"
#include "netfirewall_client.h"
#include "netfirewall_common.h"
#include "netmgr_ext_log_wrapper.h"

using namespace OHOS::NetManagerStandard;

namespace OHOS {
namespace NetManagerStandard {

static bool ConvertCIPAddressToIPC(const OH_TrafficFilter_IPAddress& cAddr, TrafficFilterIPAddress& ipcAddr)
{
    ipcAddr.family_ = static_cast<int32_t>((cAddr.family == OH_TRAFFICFILTER_IP_FAMILY_V4) ?
        TrafficFilterIPFamily::IP_FAMILY_V4 : TrafficFilterIPFamily::IP_FAMILY_V6);
    for (int i = 0; i < NETTRAFFICFILTER_IP_ADDRLEN; i++) {
        ipcAddr.addr_[i] = cAddr.addr[i];
    }
    return true;
}

static bool ConvertCIPMatchToIPCMATCH(const OH_TrafficFilter_IPMatch& cMatch, TrafficFilterIPMatch& ipcMatch)
{
    if (cMatch.type == OH_TRAFFICFILTER_IP_MATCH_ANY) {
        ipcMatch.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
        ipcMatch.invert_ = false;
        return true;
    }

    ipcMatch.type_ = static_cast<int32_t>(cMatch.type);
    ipcMatch.invert_ = cMatch.invert;

    switch (cMatch.type) {
        case OH_TRAFFICFILTER_IP_MATCH_SINGLE:
            ConvertCIPAddressToIPC(cMatch.value.single, ipcMatch.single_);
            break;
        case OH_TRAFFICFILTER_IP_MATCH_CIDR:
            ConvertCIPAddressToIPC(cMatch.value.cidr.base, ipcMatch.cidr_.base_);
            ipcMatch.cidr_.prefixLen_ = cMatch.value.cidr.prefixLen;
            break;
        case OH_TRAFFICFILTER_IP_MATCH_RANGE:
            ConvertCIPAddressToIPC(cMatch.value.range.start, ipcMatch.range_.start_);
            ConvertCIPAddressToIPC(cMatch.value.range.end, ipcMatch.range_.end_);
            break;
        case OH_TRAFFICFILTER_IP_MATCH_MULTI:
            if (cMatch.value.multi.ipCount == 0 ||
                cMatch.value.multi.ipCount > NETTRAFFICFILTER_MAX_MULTI_IP_COUNT) {
                NETMGR_EXT_LOG_E("Invalid IP multi count: %{public}u (valid: 1-%{public}u)",
                    cMatch.value.multi.ipCount, NETTRAFFICFILTER_MAX_MULTI_IP_COUNT);
                return false;
            }
            ipcMatch.multi_.ipCount_ = cMatch.value.multi.ipCount;
            for (uint32_t i = 0; i < cMatch.value.multi.ipCount; i++) {
                ConvertCIPAddressToIPC(cMatch.value.multi.ips[i], ipcMatch.multi_.ips_[i]);
            }
            break;
        default:
            return false;
    }

    return true;
}

static bool ConvertCPortMatchToPortMatch(const OH_TrafficFilter_PortMatch& cMatch, TrafficFilterPortMatch& ipcMatch)
{
    if (cMatch.type == OH_TRAFFICFILTER_PORT_MATCH_ANY) {
        ipcMatch.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
        ipcMatch.invert_ = false;
        return true;
    }

    ipcMatch.type_ = static_cast<int32_t>(cMatch.type);
    ipcMatch.invert_ = cMatch.invert;

    switch (cMatch.type) {
        case OH_TRAFFICFILTER_PORT_MATCH_SINGLE:
            ipcMatch.single_ = cMatch.value.single;
            break;
        case OH_TRAFFICFILTER_PORT_MATCH_RANGE:
            ipcMatch.range_.startPort_ = cMatch.value.range.startPort;
            ipcMatch.range_.endPort_ = cMatch.value.range.endPort;
            break;
        case OH_TRAFFICFILTER_PORT_MATCH_MULTI:
            if (cMatch.value.multi.portCount == 0 ||
                cMatch.value.multi.portCount > NETTRAFFICFILTER_MAX_MULTI_PORT_COUNT) {
                NETMGR_EXT_LOG_E("Invalid port multi count: %{public}u (valid: 1-%{public}u)",
                    cMatch.value.multi.portCount, NETTRAFFICFILTER_MAX_MULTI_PORT_COUNT);
                return false;
            }
            ipcMatch.multi_.portCount_ = cMatch.value.multi.portCount;
            for (uint32_t i = 0; i < cMatch.value.multi.portCount; i++) {
                ipcMatch.multi_.ports_[i] = cMatch.value.multi.ports[i];
            }
            break;
        default:
            return false;
    }

    return true;
}

static bool ConvertCInterfaceMatchToInterfaceMatch(
    const OH_TrafficFilter_InterfaceMatch& cMatch, TrafficFilterInterfaceMatch& ipcMatch)
{
    ipcMatch.enabled_ = cMatch.enabled;
    ipcMatch.invert_ = cMatch.invert;
    ipcMatch.isPrefix_ = cMatch.isPrefix;
    ipcMatch.ifName_.assign(cMatch.ifName, strnlen(cMatch.ifName, OH_TRAFFICFILTER_IFNAMSIZ));
    return true;
}

bool ConvertTrafficFilterIpToString(const OH_TrafficFilter_IPAddress& ip, std::string& ipStr)
{
    ipStr.clear();
    OH_TrafficFilter_IPFamily family = ip.family;
    if (static_cast<int32_t>(family) == 0) {
        family = OH_TRAFFICFILTER_IP_FAMILY_V4;
    }
    char buf[INET6_ADDRSTRLEN] = {0};
    if (family == OH_TRAFFICFILTER_IP_FAMILY_V4) {
        if (inet_ntop(AF_INET, ip.addr, buf, sizeof(buf)) == nullptr) {
            NETMGR_EXT_LOG_E("ConvertTrafficFilterIpToString: convert IPv4 failed");
            return false;
        }
        ipStr = buf;
        return true;
    }
    if (family == OH_TRAFFICFILTER_IP_FAMILY_V6) {
        if (inet_ntop(AF_INET6, ip.addr, buf, sizeof(buf)) == nullptr) {
            NETMGR_EXT_LOG_E("ConvertTrafficFilterIpToString: convert IPv6 failed");
            return false;
        }
        ipStr = buf;
        return true;
    }
    NETMGR_EXT_LOG_E("ConvertTrafficFilterIpToString: invalid family=%{public}d",
        static_cast<int32_t>(ip.family));
    return false;
}

static bool IsValidIPAddress(const OH_TrafficFilter_IPAddress& ip)
{
    OH_TrafficFilter_IPFamily family = ip.family;
    if (static_cast<int32_t>(family) == 0) {
        family = OH_TRAFFICFILTER_IP_FAMILY_V4;
    }
    char buf[INET6_ADDRSTRLEN] = {0};
    if (family == OH_TRAFFICFILTER_IP_FAMILY_V4) {
        if (inet_ntop(AF_INET, ip.addr, buf, sizeof(buf)) == nullptr) {
            NETMGR_EXT_LOG_E("IsValidIPAddress: invalid IPv4 bytes");
            return false;
        }
        for (int i = IPV4_ADDR_LEN; i < OH_TRAFFICFILTER_IP_ADDRLEN; i++) {
            if (ip.addr[i] != 0) {
                NETMGR_EXT_LOG_E("IsValidIPAddress: IPv4 has non-zero high bytes at index %{public}d", i);
                return false;
            }
        }
        return true;
    }
    if (family == OH_TRAFFICFILTER_IP_FAMILY_V6) {
        if (inet_ntop(AF_INET6, ip.addr, buf, sizeof(buf)) == nullptr) {
            NETMGR_EXT_LOG_E("IsValidIPAddress: invalid IPv6 bytes");
            return false;
        }
        return true;
    }
    NETMGR_EXT_LOG_E("IsValidIPAddress: invalid family=%{public}d", static_cast<int32_t>(ip.family));
    return false;
}

static bool IsAllZeroIP(const OH_TrafficFilter_IPAddress& ip)
{
    for (int i = 0; i < OH_TRAFFICFILTER_IP_ADDRLEN; i++) {
        if (ip.addr[i] != 0) {
            return false;
        }
    }
    return true;
}

static bool CompareIPBytes(const OH_TrafficFilter_IPAddress& start, const OH_TrafficFilter_IPAddress& end)
{
    for (int i = 0; i < OH_TRAFFICFILTER_IP_ADDRLEN; i++) {
        if (start.addr[i] != end.addr[i]) {
            return start.addr[i] < end.addr[i];
        }
    }
    return true;
}

static bool ValidateSingleIP(const OH_TrafficFilter_IPAddress& ip)
{
    if (!IsValidIPAddress(ip)) {
        NETMGR_EXT_LOG_E("ValidateSingleIP: invalid IP bytes");
        return false;
    }
    return true;
}

static bool ValidateCidrMatch(const OH_TrafficFilter_IPCidr& cidr)
{
    if (!IsValidIPAddress(cidr.base)) {
        NETMGR_EXT_LOG_E("ValidateCidrMatch: invalid CIDR base IP bytes");
        return false;
    }
    OH_TrafficFilter_IPFamily family = cidr.base.family;
    if (static_cast<int32_t>(family) == 0) {
        family = OH_TRAFFICFILTER_IP_FAMILY_V4;
    }
    uint8_t maxPrefix = (family == OH_TRAFFICFILTER_IP_FAMILY_V6) ? IPV6_PREFIX_MAX : IPV4_PREFIX_MAX;
    if (cidr.prefixLen > maxPrefix) {
        NETMGR_EXT_LOG_E("ValidateCidrMatch: invalid prefixLen=%{public}u (max=%{public}u)",
            cidr.prefixLen, maxPrefix);
        return false;
    }
    return true;
}

static bool ValidateRangeMatch(const OH_TrafficFilter_IPRange& range)
{
    if (!IsValidIPAddress(range.start)) {
        NETMGR_EXT_LOG_E("ValidateRangeMatch: invalid range start IP bytes");
        return false;
    }
    if (!IsValidIPAddress(range.end)) {
        NETMGR_EXT_LOG_E("ValidateRangeMatch: invalid range end IP bytes");
        return false;
    }
    if (range.start.family != range.end.family) {
        NETMGR_EXT_LOG_E("ValidateRangeMatch: range family mismatch, start=%{public}d, end=%{public}d",
            static_cast<int32_t>(range.start.family), static_cast<int32_t>(range.end.family));
        return false;
    }
    if (!CompareIPBytes(range.start, range.end)) {
        NETMGR_EXT_LOG_E("ValidateRangeMatch: range start > end");
        return false;
    }
    return true;
}

static bool ValidateMultiMatch(const OH_TrafficFilter_IPMulti& multi)
{
    for (uint32_t i = 0; i < multi.ipCount; i++) {
        if (!IsValidIPAddress(multi.ips[i])) {
            NETMGR_EXT_LOG_E("ValidateMultiMatch: invalid multi IP[%{public}u] bytes", i);
            return false;
        }
        if (i > 0 && multi.ips[i].family != multi.ips[0].family) {
            NETMGR_EXT_LOG_E("ValidateMultiMatch: multi IP family mismatch at index %{public}u", i);
            return false;
        }
    }
    return true;
}

static bool ValidateIPMatchValue(const OH_TrafficFilter_IPMatch& ipMatch)
{
    switch (ipMatch.type) {
        case OH_TRAFFICFILTER_IP_MATCH_ANY:
            return true;
        case OH_TRAFFICFILTER_IP_MATCH_SINGLE:
            return ValidateSingleIP(ipMatch.value.single);
        case OH_TRAFFICFILTER_IP_MATCH_CIDR:
            return ValidateCidrMatch(ipMatch.value.cidr);
        case OH_TRAFFICFILTER_IP_MATCH_RANGE:
            return ValidateRangeMatch(ipMatch.value.range);
        case OH_TRAFFICFILTER_IP_MATCH_MULTI:
            return ValidateMultiMatch(ipMatch.value.multi);
        default:
            NETMGR_EXT_LOG_E("ValidateIPMatchValue: invalid match type: %{public}d",
                static_cast<int32_t>(ipMatch.type));
            return false;
    }
}

static bool IsFieldInSize(uint32_t structSize, size_t fieldOffset, size_t fieldSize)
{
    return structSize >= fieldOffset + fieldSize;
}

static void FillProcessInfoBySize(OH_TrafficFilter_ProcessInfo* processInfo,
    uint32_t pid, uint32_t uid)
{
    uint32_t structSize = processInfo->size;
    if (IsFieldInSize(structSize, offsetof(OH_TrafficFilter_ProcessInfo, pid),
        sizeof(processInfo->pid))) {
        processInfo->pid = pid;
    }
    if (IsFieldInSize(structSize, offsetof(OH_TrafficFilter_ProcessInfo, uid),
        sizeof(processInfo->uid))) {
        processInfo->uid = uid;
    }
}

static bool ValidateBasicRuleFields(const OH_TrafficFilter_RedirectRule* rule)
{
    if (rule == nullptr) {
        return false;
    }

    if (rule->priority < OH_TRAFFICFILTER_MIN_PRIORITY ||
        rule->priority > OH_TRAFFICFILTER_MAX_PRIORITY) {
        NETMGR_EXT_LOG_E("Invalid priority: %{public}u (valid range: %{public}u-%{public}u)",
            rule->priority, OH_TRAFFICFILTER_MIN_PRIORITY, OH_TRAFFICFILTER_MAX_PRIORITY);
        return false;
    }

    if (rule->protocol != OH_TRAFFICFILTER_PROTO_TCP) {
        NETMGR_EXT_LOG_E("Invalid protocol: %{public}u (must be TCP=6)", rule->protocol);
        return false;
    }

    if (rule->hookPoint != OH_TRAFFICFILTER_HOOK_PREROUTING &&
        rule->hookPoint != OH_TRAFFICFILTER_HOOK_OUTPUT) {
        NETMGR_EXT_LOG_E("Invalid hookPoint: %{public}d (must be PREROUTING=3 or OUTPUT=1)",
            rule->hookPoint);
        return false;
    }

    return true;
}

static bool ValidateIPMatchFields(const OH_TrafficFilter_RedirectRule* rule)
{
    if (rule->srcIp.type < OH_TRAFFICFILTER_IP_MATCH_ANY ||
        rule->srcIp.type > OH_TRAFFICFILTER_IP_MATCH_MULTI) {
        NETMGR_EXT_LOG_E("Invalid srcIp type: %{public}d", rule->srcIp.type);
        return false;
    }
    if (rule->dstIp.type < OH_TRAFFICFILTER_IP_MATCH_ANY ||
        rule->dstIp.type > OH_TRAFFICFILTER_IP_MATCH_MULTI) {
        NETMGR_EXT_LOG_E("Invalid dstIp type: %{public}d", rule->dstIp.type);
        return false;
    }
    if (rule->srcIp.type == OH_TRAFFICFILTER_IP_MATCH_MULTI) {
        if (rule->srcIp.value.multi.ipCount == 0 ||
            rule->srcIp.value.multi.ipCount > NETTRAFFICFILTER_MAX_MULTI_IP_COUNT) {
            NETMGR_EXT_LOG_E("Invalid srcIp multi ipCount: %{public}u (valid: 1-%{public}u)",
                rule->srcIp.value.multi.ipCount, OH_TRAFFICFILTER_MAX_MULTI_IP_COUNT);
            return false;
        }
    }
    if (rule->dstIp.type == OH_TRAFFICFILTER_IP_MATCH_MULTI) {
        if (rule->dstIp.value.multi.ipCount == 0 ||
            rule->dstIp.value.multi.ipCount > NETTRAFFICFILTER_MAX_MULTI_IP_COUNT) {
            NETMGR_EXT_LOG_E("Invalid dstIp multi ipCount: %{public}u (valid: 1-%{public}u)",
                rule->dstIp.value.multi.ipCount, OH_TRAFFICFILTER_MAX_MULTI_IP_COUNT);
            return false;
        }
    }
    if (!ValidateIPMatchValue(rule->srcIp)) {
        NETMGR_EXT_LOG_E("Invalid srcIp value");
        return false;
    }
    if (!ValidateIPMatchValue(rule->dstIp)) {
        NETMGR_EXT_LOG_E("Invalid dstIp value");
        return false;
    }
    return true;
}

static bool ValidatePortMatchFields(const OH_TrafficFilter_RedirectRule* rule)
{
    if (rule->srcPort.type < OH_TRAFFICFILTER_PORT_MATCH_ANY ||
        rule->srcPort.type > OH_TRAFFICFILTER_PORT_MATCH_MULTI) {
        NETMGR_EXT_LOG_E("Invalid srcPort type: %{public}d", rule->srcPort.type);
        return false;
    }
    if (rule->dstPort.type < OH_TRAFFICFILTER_PORT_MATCH_ANY ||
        rule->dstPort.type > OH_TRAFFICFILTER_PORT_MATCH_MULTI) {
        NETMGR_EXT_LOG_E("Invalid dstPort type: %{public}d", rule->dstPort.type);
        return false;
    }
    if (rule->srcPort.type == OH_TRAFFICFILTER_PORT_MATCH_RANGE) {
        if (rule->srcPort.value.range.startPort > rule->srcPort.value.range.endPort) {
            NETMGR_EXT_LOG_E("Invalid srcPort range: start(%{public}u) > end(%{public}u)",
                rule->srcPort.value.range.startPort, rule->srcPort.value.range.endPort);
            return false;
        }
    }
    if (rule->dstPort.type == OH_TRAFFICFILTER_PORT_MATCH_RANGE) {
        if (rule->dstPort.value.range.startPort > rule->dstPort.value.range.endPort) {
            NETMGR_EXT_LOG_E("Invalid dstPort range: start(%{public}u) > end(%{public}u)",
                rule->dstPort.value.range.startPort, rule->dstPort.value.range.endPort);
            return false;
        }
    }
    if (rule->srcPort.type == OH_TRAFFICFILTER_PORT_MATCH_MULTI) {
        if (rule->srcPort.value.multi.portCount == 0 ||
            rule->srcPort.value.multi.portCount > NETTRAFFICFILTER_MAX_MULTI_PORT_COUNT) {
            NETMGR_EXT_LOG_E("Invalid srcPort multi portCount: %{public}u (valid: 1-%{public}u)",
                rule->srcPort.value.multi.portCount, NETTRAFFICFILTER_MAX_MULTI_PORT_COUNT);
            return false;
        }
    }
    if (rule->dstPort.type == OH_TRAFFICFILTER_PORT_MATCH_MULTI) {
        if (rule->dstPort.value.multi.portCount == 0 ||
            rule->dstPort.value.multi.portCount > NETTRAFFICFILTER_MAX_MULTI_PORT_COUNT) {
            NETMGR_EXT_LOG_E("Invalid dstPort multi portCount: %{public}u (valid: 1-%{public}u)",
                rule->dstPort.value.multi.portCount, NETTRAFFICFILTER_MAX_MULTI_PORT_COUNT);
            return false;
        }
    }
    return true;
}

static bool ValidateRuleAttributes(const OH_TrafficFilter_RedirectRule* rule)
{
    if (rule->uidStart > rule->uidEnd) {
        NETMGR_EXT_LOG_E("Invalid UID range: start(%{public}u) > end(%{public}u)",
            rule->uidStart, rule->uidEnd);
        return false;
    }

    bool hasValidProxyIpFamily = (rule->proxyIp.family == OH_TRAFFICFILTER_IP_FAMILY_V4 ||
        rule->proxyIp.family == OH_TRAFFICFILTER_IP_FAMILY_V6);
    if (!hasValidProxyIpFamily) {
        NETMGR_EXT_LOG_E("Invalid proxyIp family: %{public}d", rule->proxyIp.family);
        return false;
    }
    if (!IsValidIPAddress(rule->proxyIp)) {
        NETMGR_EXT_LOG_E("Invalid proxyIp bytes");
        return false;
    }
    if (IsAllZeroIP(rule->proxyIp)) {
        NETMGR_EXT_LOG_E("Invalid proxyIp: all zero");
        return false;
    }
    if (rule->proxyPort == 0) {
        NETMGR_EXT_LOG_E("Invalid proxyPort");
        return false;
    }

    return true;
}

static bool ValidateRedirectRule(const OH_TrafficFilter_RedirectRule* rule)
{
    if (!ValidateBasicRuleFields(rule)) {
        return false;
    }
    if (!ValidateIPMatchFields(rule)) {
        return false;
    }
    if (!ValidatePortMatchFields(rule)) {
        return false;
    }
    if (!ValidateRuleAttributes(rule)) {
        return false;
    }
    return true;
}

static bool ConvertCRedirectRuleToIPCRule(
    const OH_TrafficFilter_RedirectRule* cRule, TrafficFilterRedirectRule& ipcRule)
{
    if (cRule == nullptr) {
        return false;
    }

    ipcRule.priority_ = cRule->priority;
    ipcRule.hookPoint_ = static_cast<int32_t>(cRule->hookPoint);
    ipcRule.protocol_ = cRule->protocol;

    if (!ConvertCIPMatchToIPCMATCH(cRule->srcIp, ipcRule.srcIp_)) {
        return false;
    }

    if (!ConvertCPortMatchToPortMatch(cRule->srcPort, ipcRule.srcPort_)) {
        return false;
    }

    if (!ConvertCIPMatchToIPCMATCH(cRule->dstIp, ipcRule.dstIp_)) {
        return false;
    }

    if (!ConvertCPortMatchToPortMatch(cRule->dstPort, ipcRule.dstPort_)) {
        return false;
    }

    if (!ConvertCInterfaceMatchToInterfaceMatch(cRule->inInterface, ipcRule.inInterface_)) {
        return false;
    }

    if (!ConvertCInterfaceMatchToInterfaceMatch(cRule->outInterface, ipcRule.outInterface_)) {
        return false;
    }

    ipcRule.uidStart_ = cRule->uidStart;
    ipcRule.uidEnd_ = cRule->uidEnd;

    if (!ConvertCIPAddressToIPC(cRule->proxyIp, ipcRule.proxyIp_)) {
        return false;
    }

    ipcRule.proxyPort_ = cRule->proxyPort;

    return true;
}

RedirectorAdapterManager& RedirectorAdapterManager::GetInstance()
{
    static RedirectorAdapterManager instance;
    return instance;
}

int32_t RedirectorAdapterManager::CreateRedirector(uint32_t group_id, uint32_t priority,
    OH_TrafficFilter_Redirector** redirector)
{
    if (priority < OH_TRAFFICFILTER_MIN_PRIORITY || priority > OH_TRAFFICFILTER_MAX_PRIORITY) {
        NETMGR_EXT_LOG_E("CreateRedirector: invalid priority %{public}u", priority);
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    if (group_id < OH_TRAFFICFILTER_MIN_GROUP_ID || group_id > OH_TRAFFICFILTER_MAX_GROUP_ID) {
        NETMGR_EXT_LOG_E("CreateRedirector: invalid group_id %{public}u", group_id);
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    NETMGR_EXT_LOG_I("CreateRedirector: group_id=%{public}u, priority=%{public}u", group_id, priority);

    std::string redirectorId = "";
    int32_t ret = NetFirewallClient::GetInstance().CreateRedirector(group_id, priority, redirectorId);
    if (ret != 0) {
        NETMGR_EXT_LOG_E("CreateRedirector: NetFirewallClient::CreateRedirector failed, ret=%{public}d", ret);
        return ret;
    }
    if (redirectorId.empty()) {
        NETMGR_EXT_LOG_E("CreateRedirector: redirectorId is empty after creation");
        return OH_TRAFFICFILTER_ERROR_NFQUEUE_ERROR;
    }
    return AddRedirector(redirectorId, redirector);
}

int32_t RedirectorAdapterManager::DestroyRedirector(OH_TrafficFilter_Redirector* redirector)
{
    if (redirector == nullptr) {
        NETMGR_EXT_LOG_E("DestroyRedirector: redirector is NULL");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    NETMGR_EXT_LOG_I("DestroyRedirector");

    std::string redirectorId;
    if (!GetRedirectorId(redirector, redirectorId)) {
        NETMGR_EXT_LOG_E("DestroyRedirector: redirector handle not found in map");
        return OH_TRAFFICFILTER_ERROR_NOT_FOUND;
    }

    int32_t ret = NetFirewallClient::GetInstance().DestroyRedirector(redirectorId);
    if (ret != 0) {
        NETMGR_EXT_LOG_E("DestroyRedirector: NetFirewallClient::DestroyRedirector failed, ret=%{public}d", ret);
    } else {
        NETMGR_EXT_LOG_I("DestroyRedirector: success");
    }

    RemoveRedirector(redirector);
    return ret;
}

int32_t RedirectorAdapterManager::AddRedirectRule(
    OH_TrafficFilter_Redirector* redirector, const OH_TrafficFilter_RedirectRule* rule)
{
    if (redirector == nullptr) {
        NETMGR_EXT_LOG_E("AddRedirectRule: redirector is NULL");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    if (rule == nullptr) {
        NETMGR_EXT_LOG_E("AddRedirectRule: rule is NULL");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    if (rule->size < REDIRECT_RULE_MIN_SIZE) {
        NETMGR_EXT_LOG_E("AddRedirectRule: invalid rule size=%{public}u, min=%{public}u",
            rule->size, REDIRECT_RULE_MIN_SIZE);
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    if (!ValidateRedirectRule(rule)) {
        NETMGR_EXT_LOG_E("AddRedirectRule: rule validation failed");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    NETMGR_EXT_LOG_I("AddRedirectRule: priority=%{public}u, hookPoint=%{public}d",
        rule->priority, rule->hookPoint);
    std::string redirectorId;
    if (!GetRedirectorId(redirector, redirectorId)) {
        NETMGR_EXT_LOG_E("AddRedirectRule: redirector handle not found in map");
        return OH_TRAFFICFILTER_ERROR_NOT_FOUND;
    }
    OHOS::sptr<TrafficFilterRedirectRule> cppRule = new (std::nothrow) TrafficFilterRedirectRule();
    if (cppRule == nullptr) {
        NETMGR_EXT_LOG_E("AddRedirectRule: failed to create TrafficFilterRedirectRule");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    if (!ConvertCRedirectRuleToIPCRule(rule, *cppRule)) {
        NETMGR_EXT_LOG_E("AddRedirectRule: failed to convert C struct to IPC struct");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    int32_t ret = NetFirewallClient::GetInstance().AddRedirectRule(redirectorId, cppRule);
    if (ret != 0) {
        NETMGR_EXT_LOG_E("AddRedirectRule: NetFirewallClient::AddRedirectRule failed, ret=%{public}d", ret);
        return static_cast<int32_t>(ret);
    }

    NETMGR_EXT_LOG_I("AddRedirectRule: success");
    return OH_TRAFFICFILTER_OK;
}

int32_t RedirectorAdapterManager::ClearRedirectRule(OH_TrafficFilter_Redirector* redirector)
{
    if (redirector == nullptr) {
        NETMGR_EXT_LOG_E("ClearRedirectRule: redirector is NULL");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    NETMGR_EXT_LOG_I("ClearRedirectRule");

    std::string redirectorId;
    if (!GetRedirectorId(redirector, redirectorId)) {
        NETMGR_EXT_LOG_E("ClearRedirectRule: redirector handle not found in map");
        return OH_TRAFFICFILTER_ERROR_NOT_FOUND;
    }

    int32_t ret = NetFirewallClient::GetInstance().ClearRedirectRule(redirectorId);
    if (ret != 0) {
        NETMGR_EXT_LOG_E("ClearRedirectRule: NetFirewallClient::ClearRedirectRule failed, ret=%{public}d", ret);
        return static_cast<int32_t>(ret);
    }

    NETMGR_EXT_LOG_I("ClearRedirectRule: success");
    return OH_TRAFFICFILTER_OK;
}

int32_t RedirectorAdapterManager::QueryProcess(const OH_TrafficFilter_ConnectionInfo* connectionInfo,
    OH_TrafficFilter_ProcessInfo* processInfo)
{
    if (connectionInfo == nullptr || processInfo == nullptr) {
        NETMGR_EXT_LOG_E("QueryProcess: connectionInfo or processInfo is null");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    if (connectionInfo->size < CONNECTION_INFO_MIN_SIZE ||
        processInfo->size < PROCESS_INFO_MIN_SIZE) {
        NETMGR_EXT_LOG_E("QueryProcess: invalid struct size, connection=%{public}u, process=%{public}u",
            connectionInfo->size, processInfo->size);
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    if (connectionInfo->protocol != OH_TRAFFICFILTER_PROTO_TCP &&
        connectionInfo->protocol != OH_TRAFFICFILTER_PROTO_UDP) {
        NETMGR_EXT_LOG_E("QueryProcess: invalid protocol=%{public}u", connectionInfo->protocol);
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    std::string srcIp;
    std::string dstIp;
    if (!ConvertTrafficFilterIpToString(connectionInfo->srcIp, srcIp)) {
        NETMGR_EXT_LOG_E("QueryProcess: convert srcIp failed");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    if (!ConvertTrafficFilterIpToString(connectionInfo->dstIp, dstIp)) {
        NETMGR_EXT_LOG_E("QueryProcess: convert dstIp failed");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    uint32_t uid = 0;
    uint32_t pid = 0;
    int32_t ret = NetFirewallClient::GetInstance().QueryProcess(
        srcIp, connectionInfo->srcPort, dstIp, connectionInfo->dstPort,
        connectionInfo->protocol, uid, pid);
    if (ret != OH_TRAFFICFILTER_OK) {
        NETMGR_EXT_LOG_E("QueryProcess: QueryProcess failed, ret=%{public}d", ret);
        return ret;
    }
    FillProcessInfoBySize(processInfo, pid, uid);
    return OH_TRAFFICFILTER_OK;
}

int32_t RedirectorAdapterManager::AddRedirector(
    const std::string& redirectorId, OH_TrafficFilter_Redirector** redirector)
{
    if (redirectorId.empty()) {
        NETMGR_EXT_LOG_E("AddRedirector: redirectorId is empty");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    if (redirector == nullptr) {
        NETMGR_EXT_LOG_E("AddRedirector: redirector output ptr is NULL");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    OH_TrafficFilter_Redirector* handle = new (std::nothrow) OH_TrafficFilter_Redirector();
    if (handle == nullptr) {
        NETMGR_EXT_LOG_E("AddRedirector: failed to allocate handle memory");
        return OH_TRAFFICFILTER_ERROR_NFQUEUE_ERROR;
    }

    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        redirectorIdMap_[handle] = redirectorId;
    }

    *redirector = handle;
    NETMGR_EXT_LOG_I("AddRedirector: success, redirectorId=%{public}s", redirectorId.c_str());
    return OH_TRAFFICFILTER_OK;
}

bool RedirectorAdapterManager::GetRedirectorId(OH_TrafficFilter_Redirector* redirector, std::string& redirectorId)
{
    if (redirector == nullptr) {
        return false;
    }

    std::lock_guard<std::mutex> lock(mapMutex_);
    auto it = redirectorIdMap_.find(redirector);
    if (it == redirectorIdMap_.end()) {
        NETMGR_EXT_LOG_E("GetRedirectorId: redirector handle not found in map");
        return false;
    }

    redirectorId = it->second;
    return true;
}

void RedirectorAdapterManager::RemoveRedirector(OH_TrafficFilter_Redirector* redirector)
{
    if (redirector == nullptr) {
        return;
    }

    std::string redirectorId;
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        auto it = redirectorIdMap_.find(redirector);
        if (it != redirectorIdMap_.end()) {
            redirectorId = it->second;
            redirectorIdMap_.erase(it);
        }
    }

    delete redirector;
    NETMGR_EXT_LOG_I("RemoveRedirector: redirectorId=%{public}s", redirectorId.c_str());
}
}
}