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
#include <cctype>
#include <cstddef>
#include <cstring>
#include <securec.h>
#include <string>
#include "net_trafficfilter_adapter.h"
#include "netfirewall_client.h"
#include "netfirewall_common.h"
#include "netmgr_ext_log_wrapper.h"

#include <poll.h>
#include <sys/socket.h>
#include <linux/netlink.h>

using namespace OHOS::NetManagerStandard;

namespace OHOS {
namespace NetManagerStandard {

static constexpr uint8_t IP_VERSION_V4 = 4;
static constexpr uint8_t IP_VERSION_V6 = 6;
static constexpr uint8_t IP_VERSION_MASK = 0x0F;
static constexpr uint8_t IP_IHL_MASK = 0x0F;
static constexpr uint16_t IPV4_HEADER_MIN_LEN = 20;
static constexpr uint16_t IPV6_HEADER_LEN = 40;
static constexpr uint8_t IPV4_PROTOCOL_OFFSET = 9;
static constexpr uint8_t IPV4_SRC_IP_OFFSET = 12;
static constexpr uint8_t IPV4_DST_IP_OFFSET = 16;
static constexpr uint8_t IPV6_PROTOCOL_OFFSET = 6;
static constexpr uint8_t IPV6_SRC_IP_OFFSET = 8;
static constexpr uint8_t IPV6_DST_IP_OFFSET = 24;
static constexpr uint8_t IPV6_ADDR_LEN = 16;
static constexpr uint8_t TRANSPORT_PORT_LEN = 4;
static constexpr uint8_t TRANSPORT_SRC_PORT_OFFSET = 0;
static constexpr uint8_t TRANSPORT_DST_PORT_OFFSET = 2;
static constexpr uint8_t PORT_BYTE_SHIFT = 8;

static constexpr int32_t NFNL_SUBSYS_QUEUE = 3;
static constexpr int32_t NFQ_MSG_PACKET = 0;
static constexpr int32_t NUMBER_TWO = 2;
static constexpr uint8_t NUMBER_EIGHT = 8;

static constexpr uint8_t TCP_MIN_HEADER_LEN = 20;
static constexpr uint8_t UDP_HEADER_LEN = 8;
static constexpr uint8_t TCP_DATA_OFFSET_MASK = 0xF0;
static constexpr uint8_t TCP_DATA_OFFSET_SHIFT = 4;
static constexpr uint8_t TCP_DATA_OFFSET_UNIT = 4;
static constexpr uint16_t MAX_PACKET_HEADER_SIZE = 256;

static inline uint16_t NfqNlType(uint8_t subsys, uint8_t msg)
{
    return (static_cast<uint16_t>(subsys) << NUMBER_EIGHT) | msg;
}

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

static bool ValidateMacAddress(const std::string& mac)
{
    if (mac.length() != MAC_ADDRESS_LENGTH) {
        return false;
    }
    for (int i = 0; i < MAC_ADDRESS_LENGTH; i++) {
        if (i % MAC_ADDRESS_GROUP_SIZE == MAC_ADDRESS_SEP_INDEX_OFFSET) {
            if (mac[i] != ':') {
                return false;
            }
        } else {
            if (!std::isxdigit(static_cast<unsigned char>(mac[i]))) {
                return false;
            }
        }
    }
    return true;
}
static bool ValidateIPAddress(const OH_TrafficFilter_IPAddress& addr)
{
    OH_TrafficFilter_IPFamily family = addr.family;
    if (family != OH_TRAFFICFILTER_IP_FAMILY_V4 && family != OH_TRAFFICFILTER_IP_FAMILY_V6) {
        return false;
    }

    if (IsAllZeroIP(addr)) {
        return false;
    }

    if (family == OH_TRAFFICFILTER_IP_FAMILY_V4) {
        if ((addr.addr[0] & 0xF0) == 0xE0) {
            return false;
        }
        if (addr.addr[0] == 0x7F) {
            return false;
        }
        bool allOne = true;
        for (int i = 0; i < IPV4_ADDR_LEN; i++) {
            if (addr.addr[i] != 0xFF) {
                allOne = false;
                break;
            }
        }
        if (allOne) {
            return false;
        }
    } else if (family == OH_TRAFFICFILTER_IP_FAMILY_V6) {
        if (addr.addr[0] == 0xFF) {
            return false;
        }
        bool isLoopback = true;
        for (int i = 0; i < OH_TRAFFICFILTER_IP_ADDRLEN - 1; i++) {
            if (addr.addr[i] != 0) {
                isLoopback = false;
                break;
            }
        }
        if (isLoopback && addr.addr[OH_TRAFFICFILTER_IP_ADDRLEN - 1] == 1) {
            return false;
        }
    }

    return true;
}

static bool ValidatePacketRuleIPMatch(const OH_TrafficFilter_IPMatch& ipMatch)
{
    if (ipMatch.type < OH_TRAFFICFILTER_IP_MATCH_ANY || ipMatch.type > OH_TRAFFICFILTER_IP_MATCH_MULTI) {
        NETMGR_EXT_LOG_E("Invalid packet type: %{public}d", ipMatch.type);
        return false;
    }
    if (!ValidateIPMatchValue(ipMatch)) {
        return false;
    }
    switch (ipMatch.type) {
        case OH_TRAFFICFILTER_IP_MATCH_SINGLE:
            return ValidateIPAddress(ipMatch.value.single);
        case OH_TRAFFICFILTER_IP_MATCH_CIDR:
            return ValidateIPAddress(ipMatch.value.cidr.base);
        case OH_TRAFFICFILTER_IP_MATCH_RANGE:
            return ValidateIPAddress(ipMatch.value.range.start) &&
                   ValidateIPAddress(ipMatch.value.range.end);
        case OH_TRAFFICFILTER_IP_MATCH_MULTI: {
            for (uint32_t i = 0; i < ipMatch.value.multi.ipCount; i++) {
                if (!ValidateIPAddress(ipMatch.value.multi.ips[i])) {
                    return false;
                }
            }
            return true;
        }
        default:
            return true;
    }
}

static bool ValidatePacketRulePortMatch(const OH_TrafficFilter_PortMatch& portMatch)
{
    if (portMatch.type < OH_TRAFFICFILTER_PORT_MATCH_ANY || portMatch.type > OH_TRAFFICFILTER_PORT_MATCH_MULTI) {
        NETMGR_EXT_LOG_E("Invalid packet rule type: %{public}d", portMatch.type);
        return false;
    }
    if (portMatch.type == OH_TRAFFICFILTER_PORT_MATCH_RANGE) {
        if (portMatch.value.range.startPort > portMatch.value.range.endPort) {
            return false;
        }
    }
    if (portMatch.type == OH_TRAFFICFILTER_PORT_MATCH_MULTI) {
        if (portMatch.value.multi.portCount == 0 ||
            portMatch.value.multi.portCount > OH_TRAFFICFILTER_MAX_MULTI_PORT_COUNT) {
            return false;
        }
    }
    return true;
}

static bool ValidatePacketRuleBasicFields(const OH_TrafficFilter_FilterRule* rule)
{
    if (rule->priority < OH_TRAFFICFILTER_MIN_PRIORITY || rule->priority > OH_TRAFFICFILTER_MAX_PRIORITY) {
        NETMGR_EXT_LOG_E("Invalid packet rule priority: %{public}u (valid range: %{public}u-%{public}u)",
            rule->priority, OH_TRAFFICFILTER_MIN_PRIORITY, OH_TRAFFICFILTER_MAX_PRIORITY);
        return false;
    }
    if (rule->hookPoint < OH_TRAFFICFILTER_HOOK_INPUT || rule->hookPoint > OH_TRAFFICFILTER_HOOK_POSTROUTING) {
        NETMGR_EXT_LOG_E("Invalid packet rule hookPoint: %{public}d", rule->hookPoint);
        return false;
    }
    if (rule->protocol != OH_TRAFFICFILTER_PROTO_ANY && rule->protocol != OH_TRAFFICFILTER_PROTO_TCP &&
        rule->protocol != OH_TRAFFICFILTER_PROTO_UDP) {
        NETMGR_EXT_LOG_E("Invalid packet rule protocol: %{public}u (only 0,6,17 supported)", rule->protocol);
        return false;
    }
    return true;
}

static bool ValidatePacketRuleInterfaces(const OH_TrafficFilter_FilterRule* rule)
{
    if ((rule->inInterface.enabled && rule->inInterface.ifName[0] == '\0') ||
        (rule->outInterface.enabled && rule->outInterface.ifName[0] == '\0')) {
        NETMGR_EXT_LOG_E("Invalid packet rule: interface enabled but ifName is empty");
        return false;
    }
    if (rule->inInterface.enabled &&
        strnlen(rule->inInterface.ifName, OH_TRAFFICFILTER_IFNAMSIZ) >= OH_TRAFFICFILTER_IFNAMSIZ) {
        return false;
    }
    if (rule->outInterface.enabled &&
        strnlen(rule->outInterface.ifName, OH_TRAFFICFILTER_IFNAMSIZ) >= OH_TRAFFICFILTER_IFNAMSIZ) {
        return false;
    }
    return true;
}

static bool ValidatePacketRuleMacAndTcpFlags(const OH_TrafficFilter_FilterRule* rule)
{
    if (rule->macMatch.enable) {
        std::string mac(rule->macMatch.srcMac, strnlen(rule->macMatch.srcMac, OH_TRAFFICFILTER_MAC_ADDRSTRLEN));
        if (!ValidateMacAddress(mac)) {
            return false;
        }
    }
    if (rule->tcpFlagsMatch.enable && rule->protocol != OH_TRAFFICFILTER_PROTO_TCP) {
        return false;
    }
    if (rule->tcpFlagsMatch.enable && ((rule->tcpFlagsMatch.flagComp & ~rule->tcpFlagsMatch.flagMask) != 0 ||
        (rule->tcpFlagsMatch.flagMask & OH_TRAFFICFILTER_TCP_FLAG_ALL) != rule->tcpFlagsMatch.flagMask)) {
        return false;
    }
    return true;
}

static bool ValidatePacketRule(const OH_TrafficFilter_FilterRule* rule)
{
    if (rule == nullptr) {
        return false;
    }
    if (!ValidatePacketRuleBasicFields(rule)) {
        return false;
    }
    if (!ValidatePacketRuleIPMatch(rule->srcIp) || !ValidatePacketRuleIPMatch(rule->dstIp) ||
        !ValidatePacketRulePortMatch(rule->srcPort) || !ValidatePacketRulePortMatch(rule->dstPort)) {
        return false;
    }
    if (rule->protocol == OH_TRAFFICFILTER_PROTO_ANY &&
        (rule->srcPort.type != OH_TRAFFICFILTER_PORT_MATCH_ANY ||
         rule->dstPort.type != OH_TRAFFICFILTER_PORT_MATCH_ANY)) {
        return false;
    }
    if (rule->uidStart > rule->uidEnd) {
        NETMGR_EXT_LOG_E("Invalid packet rule UID range: start(%{public}u) > end(%{public}u)",
            rule->uidStart, rule->uidEnd);
        return false;
    }
    if (!ValidatePacketRuleInterfaces(rule) || !ValidatePacketRuleMacAndTcpFlags(rule)) {
        return false;
    }
    if (rule->hookPoint == OH_TRAFFICFILTER_HOOK_INPUT && rule->outInterface.enabled) {
        return false;
    }
    if (rule->hookPoint == OH_TRAFFICFILTER_HOOK_OUTPUT && rule->inInterface.enabled) {
        return false;
    }
    if (rule->conntrackMatch.enable && (rule->conntrackMatch.stateMask & ~0x1F) != 0) {
        return false;
    }
    return true;
}

static bool ConvertCMACMatchToIPC(const OH_TrafficFilter_MACMatch& cMatch, TrafficFilterMACMatch& ipcMatch)
{
    if (!cMatch.enable) {
        ipcMatch.enable_ = false;
        return true;
    }
    ipcMatch.enable_ = true;
    ipcMatch.invert_ = cMatch.invert;
    ipcMatch.srcMac_ = std::string(cMatch.srcMac, strnlen(cMatch.srcMac, OH_TRAFFICFILTER_MAC_ADDRSTRLEN));
    return ValidateMacAddress(ipcMatch.srcMac_);
}

static bool ConvertCTcpFlagsMatchToIPC(const OH_TrafficFilter_TCPFlagsMatch& cMatch,
    TrafficFilterTCPFlagsMatch& ipcMatch)
{
    if (!cMatch.enable) {
        ipcMatch.enable_ = false;
        return true;
    }
    ipcMatch.enable_ = true;
    ipcMatch.flagMask_ = cMatch.flagMask;
    ipcMatch.flagComp_ = cMatch.flagComp;
    if ((ipcMatch.flagComp_ & ~ipcMatch.flagMask_) != 0) {
        return false;
    }
    return true;
}

static bool ConvertCConntrackMatchToIPC(const OH_TrafficFilter_ConntrackMatch& cMatch,
    TrafficFilterConntrackMatch& ipcMatch)
{
    if (!cMatch.enable) {
        ipcMatch.enable_ = false;
        return true;
    }
    ipcMatch.enable_ = true;
    ipcMatch.stateMask_ = cMatch.stateMask;
    return true;
}

static bool ConvertCPacketRuleToIPC(const OH_TrafficFilter_FilterRule* cRule, sptr<TrafficFilterPacketRule> ipcRule)
{
    if (cRule == nullptr) {
        return false;
    }

    ipcRule->priority_ = cRule->priority;
    ipcRule->hookPoint_ = static_cast<int32_t>(cRule->hookPoint);
    ipcRule->protocol_ = cRule->protocol;

    if (!ConvertCIPMatchToIPCMATCH(cRule->srcIp, ipcRule->srcIp_)) {
        return false;
    }

    if (!ConvertCPortMatchToPortMatch(cRule->srcPort, ipcRule->srcPort_)) {
        return false;
    }

    if (!ConvertCIPMatchToIPCMATCH(cRule->dstIp, ipcRule->dstIp_)) {
        return false;
    }

    if (!ConvertCPortMatchToPortMatch(cRule->dstPort, ipcRule->dstPort_)) {
        return false;
    }

    if (!ConvertCInterfaceMatchToInterfaceMatch(cRule->inInterface, ipcRule->inInterface_)) {
        return false;
    }

    if (!ConvertCInterfaceMatchToInterfaceMatch(cRule->outInterface, ipcRule->outInterface_)) {
        return false;
    }

    ipcRule->uidStart_ = cRule->uidStart;
    ipcRule->uidEnd_ = cRule->uidEnd;

    if (!ConvertCMACMatchToIPC(cRule->macMatch, ipcRule->macMatch_)) {
        return false;
    }

    if (!ConvertCTcpFlagsMatchToIPC(cRule->tcpFlagsMatch, ipcRule->tcpFlagsMatch_)) {
        return false;
    }

    if (!ConvertCConntrackMatchToIPC(cRule->conntrackMatch, ipcRule->conntrackMatch_)) {
        return false;
    }

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
    ret = AddRedirector(redirectorId, redirector);
    if (ret != OH_TRAFFICFILTER_OK) {
        NETMGR_EXT_LOG_E("CreateRedirector: AddRedirector failed, rollback server redirector, ret=%{public}d", ret);
        NetFirewallClient::GetInstance().DestroyRedirector(redirectorId);
    }
    return ret;
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
            delete redirector;
        }
    }
}

static void ConvertCConfigToIPCCfg(
    const OH_TrafficFilter_Config* cConfig, TrafficFilterConfig& ipcConfig)
{
    if (cConfig == nullptr) {
        return;
    }
    ipcConfig.size_ = cConfig->size;
    ipcConfig.packetCopyMode_ = cConfig->packetCopyMode;
    ipcConfig.packetCopyLen_ = cConfig->packetCopyLen;
    ipcConfig.nfqueueMaxlen_ = cConfig->nfqueueMaxlen;
    ipcConfig.nfqueueFlags_ = cConfig->nfqueueFlags;
}

PacketControllerAdapterManager& PacketControllerAdapterManager::GetInstance()
{
    static PacketControllerAdapterManager instance;
    return instance;
}

bool PacketControllerAdapterManager::GetPacketInfo(OH_TrafficFilter_PacketController* controller,
    PacketInfo& packetInfo)
{
    if (controller == nullptr) {
        return false;
    }

    std::lock_guard<std::mutex> lock(mapMutex_);
    auto it = controllerIdMap_.find(controller);
    if (it == controllerIdMap_.end()) {
        NETMGR_EXT_LOG_E("GetPacketInfo: packetController handle not found in map");
        return false;
    }
    packetInfo = it->second;
    return true;
}

int32_t PacketControllerAdapterManager::CheckConfig(const OH_TrafficFilter_Config* config)
{
    if (config == nullptr) {
        return OH_TRAFFICFILTER_OK;
    }
    if (config->size < PACKET_CONTROLLER_MIN_SIZE) {
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    if (config->packetCopyMode < OH_TRAFFICFILTER_COPY_MODE_META ||
        config->packetCopyMode > OH_TRAFFICFILTER_COPY_MODE_MAXLEN) {
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    if (config->packetCopyLen > PACKET_COPY_LEN_MAX) {
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    if (config->nfqueueMaxlen > NFQUEUE_MAXLEN) {
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    return OH_TRAFFICFILTER_OK;
}

int32_t PacketControllerAdapterManager::CreatePacketController(
    uint32_t group_id,
    uint32_t priority,
    const OH_TrafficFilter_Config* config,
    OH_TrafficFilter_PacketController** controller)
{
    if (group_id < OH_TRAFFICFILTER_MIN_GROUP_ID || group_id > OH_TRAFFICFILTER_MAX_GROUP_ID) {
        NETMGR_EXT_LOG_E("CreatePacketController: invalid group_id=%{public}u", group_id);
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    if (priority < OH_TRAFFICFILTER_MIN_PRIORITY || priority > OH_TRAFFICFILTER_MAX_PRIORITY) {
        NETMGR_EXT_LOG_E("CreatePacketController: invalid priority=%{public}u", priority);
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    OHOS::sptr<TrafficFilterConfig> cppConfig = nullptr;
    uint32_t packetCopyMode = OH_TRAFFICFILTER_COPY_MODE_FULL;
    uint32_t nfqueueFlags = OH_TRAFFICFILTER_NFQUEUE_FLAG_FAIL_OPEN;
    int32_t ret = CheckConfig(config);
    if (ret != OH_TRAFFICFILTER_OK) {
        return ret;
    }
    if (config != nullptr) {
        cppConfig = new (std::nothrow) TrafficFilterConfig();
        if (cppConfig == nullptr) {
            NETMGR_EXT_LOG_E("CreatePacketController: failed to convert C struct to IPC struct");
            return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
        }
        ConvertCConfigToIPCCfg(config, *cppConfig);
        packetCopyMode = cppConfig->packetCopyMode_;
        nfqueueFlags = cppConfig->nfqueueFlags_;
    }
    std::string packetControllerId = "";
    int32_t fd = -1;
    ret = NetFirewallClient::GetInstance().CreatePacketController(group_id, priority,
        cppConfig, packetControllerId, fd);
    if (ret != 0) {
        NETMGR_EXT_LOG_E("CreatePacketController: failed, ret=%{public}d", ret);
        return ret;
    }
    PacketInfo packetInfo {
        .packetControllerId = packetControllerId,
        .fd = fd,
        .packetCopyMode = packetCopyMode,
        .nfqueueFlags = nfqueueFlags
    };
    return AddPacketController(packetInfo, controller);
}

int32_t PacketControllerAdapterManager::AddPacketController(const PacketInfo& packetInfo,
    OH_TrafficFilter_PacketController** controller)
{
    if (packetInfo.packetControllerId.empty()) {
        NETMGR_EXT_LOG_E("AddPacketController: packetControllerId is empty");
        return OH_TRAFFICFILTER_ERROR_NFQUEUE_ERROR;
    }

    OH_TrafficFilter_PacketController* handle = new (std::nothrow) OH_TrafficFilter_PacketController();
    if (handle == nullptr) {
        NETMGR_EXT_LOG_E("AddPacketController: failed to allocate handle memory");
        return OH_TRAFFICFILTER_ERROR_NFQUEUE_ERROR;
    }

    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        controllerIdMap_[handle] = packetInfo;
    }

    *controller = handle;
    return OH_TRAFFICFILTER_OK;
}

int32_t PacketControllerAdapterManager::DestroyPacketController(OH_TrafficFilter_PacketController* controller)
{
    PacketInfo packetInfo;
    if (!GetPacketInfo(controller, packetInfo)) {
        NETMGR_EXT_LOG_E("DestroyPacketController: packetController handle not found in map");
        return OH_TRAFFICFILTER_ERROR_NOT_FOUND;
    }
    UnregisterPacketCallback(controller);
    int32_t ret = NetFirewallClient::GetInstance().DestroyPacketController(packetInfo.packetControllerId);
    if (ret != 0) {
        NETMGR_EXT_LOG_E("DestroyPacketController: failed, ret=%{public}d", ret);
    } else {
        NETMGR_EXT_LOG_I("DestroyPacketController: success");
    }
    RemovePacketController(controller);
    return ret;
}

void PacketControllerAdapterManager::RemovePacketController(OH_TrafficFilter_PacketController* controller)
{
    if (controller == nullptr) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        auto it = controllerIdMap_.find(controller);
        if (it != controllerIdMap_.end()) {
            controllerIdMap_.erase(it);
            delete controller;
        }
    }
}

int32_t PacketControllerAdapterManager::AddPacketRule(OH_TrafficFilter_PacketController* controller,
    const OH_TrafficFilter_FilterRule* rule)
{
    if (controller == nullptr || rule == nullptr) {
        NETMGR_EXT_LOG_E("AddPacketRule: controller or rule is NULL");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    if (rule->size < PACKET_RULE_MIN_SIZE) {
        NETMGR_EXT_LOG_E("AddPacketRule: invalid rule size=%{public}u", rule->size);
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    if (!ValidatePacketRule(rule)) {
        NETMGR_EXT_LOG_E("AddPacketRule: rule validation failed");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    PacketInfo packetInfo;
    if (!GetPacketInfo(controller, packetInfo)) {
        NETMGR_EXT_LOG_E("AddPacketRule: controller handle not found");
        return OH_TRAFFICFILTER_ERROR_NOT_FOUND;
    }
    sptr<TrafficFilterPacketRule> cppRule = new (std::nothrow) TrafficFilterPacketRule();
    if (cppRule == nullptr) {
        NETMGR_EXT_LOG_E("AddPacketRule: failed to create TrafficFilterPacketRule");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    if (!ConvertCPacketRuleToIPC(rule, cppRule)) {
        NETMGR_EXT_LOG_E("AddPacketRule: failed to convert C struct to IPC struct");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    int32_t ret = NetFirewallClient::GetInstance().AddPacketRule(packetInfo.packetControllerId, cppRule);
    if (ret != 0) {
        NETMGR_EXT_LOG_E("AddPacketRule: NetFirewallClient::AddPacketRule failed, ret=%{public}d", ret);
        return static_cast<int32_t>(ret);
    }
    NETMGR_EXT_LOG_I("AddPacketRule: success");
    return OH_TRAFFICFILTER_OK;
}

int32_t PacketControllerAdapterManager::ClearPacketRule(OH_TrafficFilter_PacketController* controller)
{
    if (controller == nullptr) {
        NETMGR_EXT_LOG_E("ClearPacketRule: controller is NULL");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    PacketInfo packetInfo;
    if (!GetPacketInfo(controller, packetInfo)) {
        NETMGR_EXT_LOG_E("ClearPacketRule: controller handle not found");

        return OH_TRAFFICFILTER_ERROR_NOT_FOUND;
    }
    int32_t ret = NetFirewallClient::GetInstance().ClearPacketRule(packetInfo.packetControllerId);
    if (ret != 0) {
        NETMGR_EXT_LOG_E("ClearPacketRule: NetFirewallClient::ClearPacketRule failed, ret=%{public}d", ret);
        return static_cast<int32_t>(ret);
    }
    NETMGR_EXT_LOG_I("ClearPacketRule: success");
    return OH_TRAFFICFILTER_OK;
}

static inline const struct nlattr *NlaGetNext(const struct nlattr *nla, int *remaining)
{
    int aligned = NLA_ALIGN(nla->nla_len);
    *remaining -= aligned;
    return reinterpret_cast<const struct nlattr *>(reinterpret_cast<const char *>(nla) + aligned);
}

static inline int NlaIsValid(const struct nlattr *nla, int remaining)
{
    return remaining >= static_cast<int>(NLA_HDRLEN) &&
           nla->nla_len >= static_cast<int>(NLA_HDRLEN) &&
           nla->nla_len <= remaining;
}

static inline int NlaPayloadLen(const struct nlattr *nla)
{
    return nla->nla_len - NLA_HDRLEN;
}

static inline void *NlaPayload(const struct nlattr *nla)
{
    return reinterpret_cast<void *>(reinterpret_cast<char *>(nla) + NLA_HDRLEN);
}

static void NfqParsePacketHdr(NfqPkt *pkt, const void *data, int dlen)
{
    if (dlen >= static_cast<int>(sizeof(struct NfqPhdr))) {
        const struct NfqPhdr *ph = static_cast<const struct NfqPhdr *>(data);
        pkt->packetId = ntohl(ph->packetId);
        pkt->hwProtocol = ntohs(ph->hwProtocol);
        pkt->hook = ph->hook;
    }
}

static void NfqParseTimestamp(NfqPkt *pkt, const void *data, int dlen)
{
    if (dlen >= static_cast<int>(NUMBER_TWO * sizeof(uint64_t))) {
        const uint64_t *ts = static_cast<const uint64_t *>(data);
        pkt->ts.tv_sec = static_cast<long>(be64toh(ts[0]));
        pkt->ts.tv_usec = static_cast<long>(be64toh(ts[1]));
        pkt->hasTs = 1;
    }
}

static void NfqPktParseAttrs(NfqPkt *pkt, const struct nlattr *start, int remaining)
{
    for (const struct nlattr *nla = start; NlaIsValid(nla, remaining); nla = NlaGetNext(nla, &remaining)) {
        const void *data = NlaPayload(nla);
        int dlen = NlaPayloadLen(nla);

        switch (nla->nla_type) {
            case NFQA_PACKET_HDR:
                NfqParsePacketHdr(pkt, data, dlen);
                break;
            case NFQA_PAYLOAD:
                pkt->payload = data;
                pkt->payloadLen = dlen;
                break;
            case NFQA_MARK:
                if (dlen >= static_cast<int>(sizeof(uint32_t))) {
                    pkt->mark = ntohl(*reinterpret_cast<const uint32_t *>(data));
                }
                break;
            case NFQA_IFINDEX_INDEV:
                if (dlen >= static_cast<int>(sizeof(uint32_t))) {
                    pkt->indev = ntohl(*reinterpret_cast<const uint32_t *>(data));
                }
                break;
            case NFQA_IFINDEX_OUTDEV:
                if (dlen >= static_cast<int>(sizeof(uint32_t))) {
                    pkt->outdev = ntohl(*reinterpret_cast<const uint32_t *>(data));
                }
                break;
            case NFQA_HWADDR:
                if (dlen >= static_cast<int>(sizeof(struct NfqHwaddr))) {
                    const struct NfqHwaddr *hw = static_cast<const struct NfqHwaddr *>(data);
                    pkt->hwAddrlen = ntohs(hw->hwAddrlen);
                    memcpy_s(pkt->hwAddr, sizeof(pkt->hwAddr), hw->hwAddr, NUMBER_EIGHT);
                }
                break;
            case NFQA_TIMESTAMP:
                NfqParseTimestamp(pkt, data, dlen);
                break;
            default:
                break;
        }
    }
}

static uint32_t NfqPktId(const NfqPkt *pkt)
{
    return pkt ? pkt->packetId : 0;
}

static inline uint16_t ParsePort(const uint8_t *transportHeader, uint8_t offset)
{
    return (transportHeader[offset] << PORT_BYTE_SHIFT) | transportHeader[offset + 1];
}

static int NfqPktPayload(const NfqPkt *pkt, const void **data, size_t *len)
{
    if (!pkt || !pkt->payload) {
        return -1;
    }
    *data = pkt->payload;
    *len = pkt->payloadLen;
    return 0;
}

static bool ParseIPv4PacketPayload(uint8_t *payload, uint16_t payloadLen,
    OH_TrafficFilter_PacketDesc &packet)
{
    uint8_t ihl = payload[0] & IP_IHL_MASK;
    uint16_t ipHeaderLen = ihl * IPV4_ADDR_LEN;
    if (payloadLen < ipHeaderLen || ipHeaderLen < IPV4_HEADER_MIN_LEN) {
        return false;
    }
    packet.protocol = payload[IPV4_PROTOCOL_OFFSET];
    packet.srcIp.family = OH_TRAFFICFILTER_IP_FAMILY_V4;
    memset_s(packet.srcIp.addr, OH_TRAFFICFILTER_IP_ADDRLEN, 0, OH_TRAFFICFILTER_IP_ADDRLEN);
    memcpy_s(packet.srcIp.addr, OH_TRAFFICFILTER_IP_ADDRLEN, payload + IPV4_SRC_IP_OFFSET, IPV4_ADDR_LEN);
    packet.dstIp.family = OH_TRAFFICFILTER_IP_FAMILY_V4;
    memset_s(packet.dstIp.addr, OH_TRAFFICFILTER_IP_ADDRLEN, 0, OH_TRAFFICFILTER_IP_ADDRLEN);
    memcpy_s(packet.dstIp.addr, OH_TRAFFICFILTER_IP_ADDRLEN, payload + IPV4_DST_IP_OFFSET, IPV4_ADDR_LEN);
    packet.packetLen = payloadLen;
    if (packet.protocol == OH_TRAFFICFILTER_PROTO_TCP || packet.protocol == OH_TRAFFICFILTER_PROTO_UDP) {
        if (payloadLen < ipHeaderLen + TRANSPORT_PORT_LEN) {
            return false;
        }
        const uint8_t *transportHeader = payload + ipHeaderLen;
        packet.srcPort = ParsePort(transportHeader, TRANSPORT_SRC_PORT_OFFSET);
        packet.dstPort = ParsePort(transportHeader, TRANSPORT_DST_PORT_OFFSET);
    } else {
        packet.srcPort = 0;
        packet.dstPort = 0;
    }
    return true;
}

static bool ParseIPv6PacketPayload(uint8_t *payload, uint16_t payloadLen,
    OH_TrafficFilter_PacketDesc &packet)
{
    if (payloadLen < IPV6_HEADER_LEN) {
        return false;
    }
    packet.protocol = payload[IPV6_PROTOCOL_OFFSET];
    packet.srcIp.family = OH_TRAFFICFILTER_IP_FAMILY_V6;
    memcpy_s(packet.srcIp.addr, OH_TRAFFICFILTER_IP_ADDRLEN, payload + IPV6_SRC_IP_OFFSET, IPV6_ADDR_LEN);
    packet.dstIp.family = OH_TRAFFICFILTER_IP_FAMILY_V6;
    memcpy_s(packet.dstIp.addr, OH_TRAFFICFILTER_IP_ADDRLEN, payload + IPV6_DST_IP_OFFSET, IPV6_ADDR_LEN);
    packet.packetLen = payloadLen;
    if (packet.protocol == OH_TRAFFICFILTER_PROTO_TCP || packet.protocol == OH_TRAFFICFILTER_PROTO_UDP) {
        if (payloadLen < IPV6_HEADER_LEN + TRANSPORT_PORT_LEN) {
            return false;
        }
        const uint8_t *transportHeader = payload + IPV6_HEADER_LEN;
        packet.srcPort = ParsePort(transportHeader, TRANSPORT_SRC_PORT_OFFSET);
        packet.dstPort = ParsePort(transportHeader, TRANSPORT_DST_PORT_OFFSET);
    } else {
        packet.srcPort = 0;
        packet.dstPort = 0;
    }
    return true;
}

static bool ParsePacketPayload(uint8_t *payload, uint16_t payloadLen, OH_TrafficFilter_PacketDesc &packet)
{
    if (payload == nullptr || payloadLen < IPV4_HEADER_MIN_LEN) {
        return false;
    }
    uint8_t ipVersion = (payload[0] >> IP_VERSION_V4) & IP_VERSION_MASK;
    if (ipVersion == IP_VERSION_V4) {
        return ParseIPv4PacketPayload(payload, payloadLen, packet);
    } else if (ipVersion == IP_VERSION_V6) {
        return ParseIPv6PacketPayload(payload, payloadLen, packet);
    } else {
        return false;
    }
}

static bool ParseIPHeaderLength(uint8_t *payload, uint16_t payloadLen, uint16_t &ipHeaderLen, uint8_t &protocol)
{
    uint8_t ipVersion = (payload[0] >> IP_VERSION_V4) & IP_VERSION_MASK;
    if (ipVersion == IP_VERSION_V4) {
        uint8_t ihl = payload[0] & IP_IHL_MASK;
        ipHeaderLen = ihl * IPV4_ADDR_LEN;
        if (ipHeaderLen < IPV4_HEADER_MIN_LEN || payloadLen < ipHeaderLen) {
            return false;
        }
        protocol = payload[IPV4_PROTOCOL_OFFSET];
    } else if (ipVersion == IP_VERSION_V6) {
        ipHeaderLen = IPV6_HEADER_LEN;
        if (payloadLen < ipHeaderLen) {
            return false;
        }
        protocol = payload[IPV6_PROTOCOL_OFFSET];
    } else {
        return false;
    }
    return true;
}

static uint16_t GetTransportHeaderLength(uint8_t *payload, uint16_t payloadLen,
    uint16_t ipHeaderLen, uint8_t protocol)
{
    if (protocol == OH_TRAFFICFILTER_PROTO_TCP) {
        if (payloadLen < ipHeaderLen + TCP_MIN_HEADER_LEN) {
            return 0;
        }
        uint8_t dataOffset = (payload[ipHeaderLen + 12] & TCP_DATA_OFFSET_MASK) >> TCP_DATA_OFFSET_SHIFT;
        uint16_t tcpHeaderLen = dataOffset * TCP_DATA_OFFSET_UNIT;
        return (tcpHeaderLen < TCP_MIN_HEADER_LEN) ? TCP_MIN_HEADER_LEN : tcpHeaderLen;
    } else if (protocol == OH_TRAFFICFILTER_PROTO_UDP) {
        return UDP_HEADER_LEN;
    }
    return 0;
}

static uint16_t ClampHeaderLength(uint16_t totalHeaderLen, uint16_t payloadLen)
{
    if (totalHeaderLen > payloadLen) {
        totalHeaderLen = payloadLen;
    }
    if (totalHeaderLen > MAX_PACKET_HEADER_SIZE) {
        totalHeaderLen = MAX_PACKET_HEADER_SIZE;
    }
    return totalHeaderLen;
}

static bool AllocateAndCopyHeader(uint8_t *payload, uint16_t headerLen, uint8_t **headerBuffer)
{
    if (payload == nullptr || headerBuffer == nullptr || headerLen == 0 || headerLen > MAX_PACKET_HEADER_SIZE) {
        return false;
    }
    *headerBuffer = new (std::nothrow) uint8_t[headerLen];
    if (*headerBuffer == nullptr) {
        return false;
    }
    if (memcpy_s(*headerBuffer, headerLen, payload, headerLen) != 0) {
        delete[] *headerBuffer;
        *headerBuffer = nullptr;
        return false;
    }
    return true;
}

static bool ExtractPacketHeader(uint8_t *payload, uint16_t payloadLen, uint8_t **headerBuffer, uint16_t *headerLen)
{
    if (payload == nullptr || payloadLen < IPV4_HEADER_MIN_LEN || headerBuffer == nullptr || headerLen == nullptr) {
        return false;
    }
    uint16_t ipHeaderLen = 0;
    uint8_t protocol = 0;
    if (!ParseIPHeaderLength(payload, payloadLen, ipHeaderLen, protocol)) {
        return false;
    }
    uint16_t transportHeaderLen = GetTransportHeaderLength(payload, payloadLen, ipHeaderLen, protocol);
    uint16_t totalHeaderLen = ClampHeaderLength(ipHeaderLen + transportHeaderLen, payloadLen);
    if (!AllocateAndCopyHeader(payload, totalHeaderLen, headerBuffer)) {
        return false;
    }
    *headerLen = totalHeaderLen;
    return true;
}

static void FreePacketHeader(uint8_t *headerBuffer)
{
    if (headerBuffer != nullptr) {
        delete[] headerBuffer;
    }
}

static void DetectPacketIdGap(OH_TrafficFilter_PacketController *controller, uint32_t packetId)
{
    if (controller->isFirstPacket) {
        controller->isFirstPacket = false;
        controller->lastPacketId = packetId;
    } else {
        int32_t diff = static_cast<int32_t>(packetId - controller->lastPacketId);
        if (diff != 1 && !(controller->lastPacketId == 0xFFFFFFFFU && packetId == 0)) {
            if (controller->nfqueueFlags == OH_TRAFFICFILTER_NFQUEUE_FLAG_FAIL_OPEN) {
                NETMGR_EXT_LOG_I("Packet ID gap detected: last=%{public}u, current=%{public}u ,packets is accepted",
                    controller->lastPacketId, packetId);
            } else {
                NETMGR_EXT_LOG_I("Packet ID gap detected: last=%{public}u, current=%{public}u ,packets is dropped",
                    controller->lastPacketId, packetId);
            }
        }
        controller->lastPacketId = packetId;
    }
}

static int32_t HandlePacketMessage(OH_TrafficFilter_PacketController *controller, struct nlmsghdr *nlh)
{
    struct NfqNfg *nfg = static_cast<struct NfqNfg *>(NLMSG_DATA(nlh));
    uint16_t queueNum = ntohs(nfg->resId);
    NfqPkt pkt;
    memset_s(&pkt, sizeof(pkt), 0, sizeof(pkt));
    int hdrLen = NLMSG_LENGTH(sizeof(struct NfqNfg));
    int attrRemaining = static_cast<int>(nlh->nlmsg_len) - hdrLen;
    const struct nlattr *attrStart =
        reinterpret_cast<const struct nlattr *>(reinterpret_cast<const char *>(nfg) + sizeof(struct NfqNfg));
    NfqPktParseAttrs(&pkt, attrStart, attrRemaining);

    uint32_t packetId = NfqPktId(&pkt);
    DetectPacketIdGap(controller, packetId);
    const void *payload = nullptr;
    size_t payloadLen = 0;
    if (NfqPktPayload(&pkt, &payload, &payloadLen) < 0 || payload == nullptr) {
        NETMGR_EXT_LOG_W("NFQA_PAYLOAD missing or truncated, accepting standard packet");
        PacketControllerAdapterManager::GetInstance().SendVerdict(queueNum, packetId, 0, 0);
        return OH_TRAFFICFILTER_OK;
    }
    OH_TrafficFilter_PacketDesc packet = {};
    uint8_t *headerBuffer = nullptr;
    uint16_t headerLen = 0;
    if (controller->packetCopyMode == OH_TRAFFICFILTER_COPY_MODE_HEADER) {
        if (!ExtractPacketHeader(static_cast<uint8_t*>(const_cast<void*>(payload)),
            static_cast<uint16_t>(payloadLen), &headerBuffer, &headerLen)) {
            PacketControllerAdapterManager::GetInstance().SendVerdict(queueNum, packetId, 0, 0);
            return OH_TRAFFICFILTER_OK;
        }
        packet.data = headerBuffer;
        packet.packetLen = headerLen;
        payloadLen = headerLen;
    } else {
        packet.data = static_cast<uint8_t*>(const_cast<void*>(payload));
    }
    packet.userData = controller->userData;
    if (!ParsePacketPayload(static_cast<uint8_t*>(const_cast<void*>(payload)),
        static_cast<uint16_t>(payloadLen), packet)) {
        FreePacketHeader(headerBuffer);
        PacketControllerAdapterManager::GetInstance().SendVerdict(queueNum, packetId, 0, 0);
        return OH_TRAFFICFILTER_OK;
    }

    int verdict = controller->callback(&packet, controller->userData) == OH_TRAFFICFILTER_DECISION_ACCEPT? 1 : 0;
    FreePacketHeader(headerBuffer);
    return PacketControllerAdapterManager::GetInstance().SendVerdict(queueNum, packetId, verdict, 0);
}

static bool HandleNetlinkError(struct nlmsghdr *nlh, int &remainingLen)
{
    struct nlmsgerr *err = static_cast<struct nlmsgerr *>(NLMSG_DATA(nlh));
    if (err->error == 0) {
        nlh = NLMSG_NEXT(nlh, remainingLen);
        return true;
    }
    int realErrno = -err->error;
    NETMGR_EXT_LOG_E("FATAL: Kernel rejected us! errno = %{public}d, description = %{public}s",
                     realErrno, strerror(realErrno));
    return false;
}

static void ProcessNetlinkMessage(OH_TrafficFilter_PacketController *controller,
    struct nlmsghdr *nlh, int &remainingLen)
{
    if (nlh->nlmsg_type == NLMSG_ERROR) {
        if (!HandleNetlinkError(nlh, remainingLen)) {
            return;
        }
    } else if (nlh->nlmsg_type == NLMSG_DONE) {
        nlh = NLMSG_NEXT(nlh, remainingLen);
    } else if (nlh->nlmsg_type == NfqNlType(NFNL_SUBSYS_QUEUE, NFQ_MSG_PACKET)) {
        HandlePacketMessage(controller, nlh);
        nlh = NLMSG_NEXT(nlh, remainingLen);
    } else {
        NETMGR_EXT_LOG_W("filtered unexpected netlink message type: %{public}d", nlh->nlmsg_type);
        nlh = NLMSG_NEXT(nlh, remainingLen);
    }
}

static void *PacketWorkerThread(void *arg)
{
    OH_TrafficFilter_PacketController *controller = static_cast<OH_TrafficFilter_PacketController *>(arg);
    if (!controller || controller->fd < 0) {
        return nullptr;
    }
    int fd = controller->fd;
    alignas(NLMSG_ALIGNTO) char buf[65536];
    struct pollfd pfd{ fd, POLLIN, 0 };
    while (controller->running.load()) {
        int ret = poll(&pfd, 1, 500);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }
        if (ret == 0 || !(pfd.revents & POLLIN)) {
            continue;
        }
        ssize_t recvLen = recv(fd, buf, sizeof(buf), 0);
        if (recvLen <= 0) {
            if (recvLen < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                continue;
            }
            break;
        }
        struct nlmsghdr *nlh = reinterpret_cast<struct nlmsghdr *>(buf);
        int remainingLen = static_cast<int>(recvLen);
        while (NLMSG_OK(nlh, remainingLen)) {
            ProcessNetlinkMessage(controller, nlh, remainingLen);
        }
    }
    controller->running.store(false);
    controller->callbackRegistered.store(false);
    return nullptr;
}

int32_t PacketControllerAdapterManager::SendVerdict(int32_t queueNum, uint32_t packetId, int32_t verdict, int32_t mark)
{
    NETMGR_EXT_LOG_D("PacketControllerAdapterManager::SendVerdict");
    int32_t ret = NetFirewallClient::GetInstance().SendVerdict(queueNum, packetId, verdict, mark);
    return ret;
}

int32_t PacketControllerAdapterManager::RegisterPacketCallback(OH_TrafficFilter_PacketController* controller,
    OH_TrafficFilter_PacketCallback callback, void* userData)
{
    if (controller == nullptr || callback == nullptr) {
        NETMGR_EXT_LOG_E("RegisterPacketCallback: invalid parameter");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    PacketInfo packetInfo;
    if (!GetPacketInfo(controller, packetInfo)) {
        NETMGR_EXT_LOG_E("RegisterPacketCallback: controller handle not found");
        return OH_TRAFFICFILTER_ERROR_NOT_FOUND;
    }
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (controller->running.load()) {
        controller->running.store(false);
        controller->callbackRegistered.store(false);
        if (controller->workerThread != 0) {
            pthread_join(controller->workerThread, nullptr);
            controller->workerThread = 0;
        }
    }
    callbackMap_[controller] = {callback, userData};
    controller->callback = callback;
    controller->userData = userData;
    controller->callbackRegistered.store(true);
    controller->running.store(true);
    controller->fd = packetInfo.fd;
    controller->packetCopyMode = packetInfo.packetCopyMode;
    controller->nfqueueFlags = packetInfo.nfqueueFlags;
    int pthreadRet = pthread_create(&controller->workerThread, nullptr, PacketWorkerThread, controller);
    if (pthreadRet != 0) {
        NETMGR_EXT_LOG_E("pthread_create failed: %{public}s", strerror(pthreadRet));
        controller->running.store(false);
        controller->callbackRegistered.store(false);
        controller->workerThread = 0;
        callbackMap_.erase(controller);
        return OH_TRAFFICFILTER_ERROR_NFQUEUE_ERROR;
    }
    NETMGR_EXT_LOG_I("RegisterPacketCallback: success");
    return OH_TRAFFICFILTER_OK;
}

int32_t PacketControllerAdapterManager::UnregisterPacketCallback(OH_TrafficFilter_PacketController* controller)
{
    if (controller == nullptr) {
        NETMGR_EXT_LOG_E("UnregisterPacketCallback: invalid parameter");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    PacketInfo packetInfo;
    if (!GetPacketInfo(controller, packetInfo)) {
        NETMGR_EXT_LOG_E("UnregisterPacketCallback: controller handle not found");
        return OH_TRAFFICFILTER_ERROR_NOT_FOUND;
    }
    std::lock_guard<std::mutex> lock(callbackMutex_);
    callbackMap_.erase(controller);
    if (controller->running.load()) {
        controller->running.store(false);
        controller->callbackRegistered.store(false);
        if (controller->workerThread != 0) {
            pthread_join(controller->workerThread, nullptr);
            controller->workerThread = 0;
        }
    }
    NETMGR_EXT_LOG_I("UnregisterPacketCallback: success");
    return OH_TRAFFICFILTER_OK;
}
}
}