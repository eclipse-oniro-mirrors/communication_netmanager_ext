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

    if (rule->hook_point != OH_TRAFFICFILTER_HOOK_PREROUTING &&
        rule->hook_point != OH_TRAFFICFILTER_HOOK_OUTPUT) {
        NETMGR_EXT_LOG_E("Invalid hook_point: %{public}d (must be PREROUTING=3 or OUTPUT=1)",
            rule->hook_point);
        return false;
    }

    return true;
}

static bool ValidateIPMatchFields(const OH_TrafficFilter_RedirectRule* rule)
{
    if (rule->src_ip.type < OH_TRAFFICFILTER_IP_MATCH_ANY ||
        rule->src_ip.type > OH_TRAFFICFILTER_IP_MATCH_MULTI) {
        NETMGR_EXT_LOG_E("Invalid src_ip type: %{public}d", rule->src_ip.type);
        return false;
    }
    if (rule->dst_ip.type < OH_TRAFFICFILTER_IP_MATCH_ANY ||
        rule->dst_ip.type > OH_TRAFFICFILTER_IP_MATCH_MULTI) {
        NETMGR_EXT_LOG_E("Invalid dst_ip type: %{public}d", rule->dst_ip.type);
        return false;
    }
    if (rule->src_ip.type == OH_TRAFFICFILTER_IP_MATCH_MULTI) {
        if (rule->src_ip.value.multi.ipCount == 0 ||
            rule->src_ip.value.multi.ipCount > NETTRAFFICFILTER_MAX_MULTI_IP_COUNT) {
            NETMGR_EXT_LOG_E("Invalid src_ip multi ipCount: %{public}u (valid: 1-%{public}u)",
                rule->src_ip.value.multi.ipCount, OH_TRAFFICFILTER_MAX_MULTI_IP_COUNT);
            return false;
        }
    }
    if (rule->dst_ip.type == OH_TRAFFICFILTER_IP_MATCH_MULTI) {
        if (rule->dst_ip.value.multi.ipCount == 0 ||
            rule->dst_ip.value.multi.ipCount > NETTRAFFICFILTER_MAX_MULTI_IP_COUNT) {
            NETMGR_EXT_LOG_E("Invalid dst_ip multi ipCount: %{public}u (valid: 1-%{public}u)",
                rule->dst_ip.value.multi.ipCount, OH_TRAFFICFILTER_MAX_MULTI_IP_COUNT);
            return false;
        }
    }
    return true;
}

static bool ValidatePortMatchFields(const OH_TrafficFilter_RedirectRule* rule)
{
    if (rule->src_port.type < OH_TRAFFICFILTER_PORT_MATCH_ANY ||
        rule->src_port.type > OH_TRAFFICFILTER_PORT_MATCH_MULTI) {
        NETMGR_EXT_LOG_E("Invalid src_port type: %{public}d", rule->src_port.type);
        return false;
    }
    if (rule->dst_port.type < OH_TRAFFICFILTER_PORT_MATCH_ANY ||
        rule->dst_port.type > OH_TRAFFICFILTER_PORT_MATCH_MULTI) {
        NETMGR_EXT_LOG_E("Invalid dst_port type: %{public}d", rule->dst_port.type);
        return false;
    }
    if (rule->src_port.type == OH_TRAFFICFILTER_PORT_MATCH_RANGE) {
        if (rule->src_port.value.range.startPort > rule->src_port.value.range.endPort) {
            NETMGR_EXT_LOG_E("Invalid src_port range: start(%{public}u) > end(%{public}u)",
                rule->src_port.value.range.startPort, rule->src_port.value.range.endPort);
            return false;
        }
    }
    if (rule->dst_port.type == OH_TRAFFICFILTER_PORT_MATCH_RANGE) {
        if (rule->dst_port.value.range.startPort > rule->dst_port.value.range.endPort) {
            NETMGR_EXT_LOG_E("Invalid dst_port range: start(%{public}u) > end(%{public}u)",
                rule->dst_port.value.range.startPort, rule->dst_port.value.range.endPort);
            return false;
        }
    }
    if (rule->src_port.type == OH_TRAFFICFILTER_PORT_MATCH_MULTI) {
        if (rule->src_port.value.multi.portCount == 0 ||
            rule->src_port.value.multi.portCount > NETTRAFFICFILTER_MAX_MULTI_PORT_COUNT) {
            NETMGR_EXT_LOG_E("Invalid src_port multi portCount: %{public}u (valid: 1-%{public}u)",
                rule->src_port.value.multi.portCount, NETTRAFFICFILTER_MAX_MULTI_PORT_COUNT);
            return false;
        }
    }
    if (rule->dst_port.type == OH_TRAFFICFILTER_PORT_MATCH_MULTI) {
        if (rule->dst_port.value.multi.portCount == 0 ||
            rule->dst_port.value.multi.portCount > NETTRAFFICFILTER_MAX_MULTI_PORT_COUNT) {
            NETMGR_EXT_LOG_E("Invalid dst_port multi portCount: %{public}u (valid: 1-%{public}u)",
                rule->dst_port.value.multi.portCount, NETTRAFFICFILTER_MAX_MULTI_PORT_COUNT);
            return false;
        }
    }
    return true;
}

static bool ValidateRuleAttributes(const OH_TrafficFilter_RedirectRule* rule)
{
    if (rule->uid_start > rule->uid_end) {
        NETMGR_EXT_LOG_E("Invalid UID range: start(%{public}u) > end(%{public}u)",
            rule->uid_start, rule->uid_end);
        return false;
    }

    bool hasValidProxyIpFamily = (rule->proxy_ip.family == OH_TRAFFICFILTER_IP_FAMILY_V4 ||
        rule->proxy_ip.family == OH_TRAFFICFILTER_IP_FAMILY_V6);
    if (!hasValidProxyIpFamily) {
        NETMGR_EXT_LOG_E("Invalid proxy_ip family: %{public}d", rule->proxy_ip.family);
        return false;
    }
    if (rule->proxy_port == 0) {
        NETMGR_EXT_LOG_E("Invalid proxy_port");
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
    ipcRule.hookPoint_ = static_cast<int32_t>(cRule->hook_point);
    ipcRule.protocol_ = cRule->protocol;

    if (!ConvertCIPMatchToIPCMATCH(cRule->src_ip, ipcRule.srcIp_)) {
        return false;
    }

    if (!ConvertCPortMatchToPortMatch(cRule->src_port, ipcRule.srcPort_)) {
        return false;
    }

    if (!ConvertCIPMatchToIPCMATCH(cRule->dst_ip, ipcRule.dstIp_)) {
        return false;
    }

    if (!ConvertCPortMatchToPortMatch(cRule->dst_port, ipcRule.dstPort_)) {
        return false;
    }

    if (!ConvertCInterfaceMatchToInterfaceMatch(cRule->in_interface, ipcRule.inInterface_)) {
        return false;
    }

    if (!ConvertCInterfaceMatchToInterfaceMatch(cRule->out_interface, ipcRule.outInterface_)) {
        return false;
    }

    ipcRule.uidStart_ = cRule->uid_start;
    ipcRule.uidEnd_ = cRule->uid_end;

    if (!ConvertCIPAddressToIPC(cRule->proxy_ip, ipcRule.proxyIp_)) {
        return false;
    }

    ipcRule.proxyPort_ = cRule->proxy_port;

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

    NETMGR_EXT_LOG_I("AddRedirectRule: priority=%{public}u, hook_point=%{public}d",
        rule->priority, rule->hook_point);
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
    if (!ConvertTrafficFilterIpToString(connectionInfo->src_ip, srcIp)) {
        NETMGR_EXT_LOG_E("QueryProcess: convert src_ip failed");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    if (!ConvertTrafficFilterIpToString(connectionInfo->dst_ip, dstIp)) {
        NETMGR_EXT_LOG_E("QueryProcess: convert dst_ip failed");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    uint32_t uid = 0;
    uint32_t pid = 0;
    int32_t ret = NetFirewallClient::GetInstance().QueryProcess(
        srcIp, connectionInfo->src_port, dstIp, connectionInfo->dst_port,
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