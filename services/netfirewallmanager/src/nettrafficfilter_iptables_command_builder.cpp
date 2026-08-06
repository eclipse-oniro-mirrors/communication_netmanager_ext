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

#include "nettrafficfilter_iptables_command_builder.h"
#include "netmgr_ext_log_wrapper.h"
#include "netsys_controller.h"
#include <arpa/inet.h>
#include <sstream>
#include <securec.h>

namespace OHOS {
namespace NetManagerStandard {
const std::string DNAT_TARGET = "DNAT";
constexpr uint32_t UID_UNSPEC = static_cast<uint32_t>(-1);
constexpr uint32_t PORT_MAX = 65535;
constexpr uint32_t DEFAULT_IPTABLE_LEN = 128;

static const char* GetTcpFlagName(uint8_t bit)
{
    switch (bit) {
        case 0x01: return "SYN";
        case 0x02: return "ACK";
        case 0x04: return "FIN";
        case 0x08: return "RST";
        case 0x10: return "PSH";
        case 0x20: return "URG";
        default: return "";
    }
}

static const char* GetConntrackStateName(uint8_t bit)
{
    switch (bit) {
        case 0x01: return "NEW";
        case 0x02: return "ESTABLISHED";
        case 0x04: return "RELATED";
        case 0x08: return "INVALID";
        case 0x10: return "UNTRACKED";
        default: return "";
    }
}

constexpr uint8_t BIT_FLAG_START = 0x01;
constexpr uint8_t BIT_SHIFT_STEP = 1;

static constexpr const char* FILTER_TABLE_APPEND = "-t filter -A ";
static constexpr const char* TARGET_JUMP_PREFIX = " -j ";
static constexpr const char* NFQUEUE_ACTION_PREFIX = "NFQUEUE --queue-num ";
static constexpr const char* RETURN_TARGET = "RETURN";
static constexpr const char* TCP_FLAGS_MATCH_PREFIX = " -m tcp --tcp-flags ";
static constexpr const char* CONNTRACK_MATCH_PREFIX = " -m conntrack --ctstate ";
static constexpr const char* MAC_MATCH_PREFIX = " -m mac";
static constexpr const char* MAC_SOURCE_PREFIX = " --mac-source ";
static constexpr const char* PROTO_TCP_FLAG = " -p tcp";
static constexpr const char* PROTO_PREFIX = " -p ";
static constexpr const char* OWNER_MATCH_PREFIX = " -m owner --uid-owner ";
static constexpr const char* UID_RANGE_SEPARATOR = "-";

static std::string BuildNfqueueAction(int32_t queueNum)
{
    return std::string(NFQUEUE_ACTION_PREFIX) + std::to_string(queueNum);
}

static std::string FormatBitMask(uint8_t mask, const char* (*getBitName)(uint8_t))
{
    std::string result;
    for (uint8_t bit = BIT_FLAG_START; bit != 0; bit <<= BIT_SHIFT_STEP) {
        if (mask & bit) {
            if (!result.empty()) {
                result += ",";
            }
            result += getBitName(bit);
        }
    }
    return result;
}

static std::vector<TrafficFilterPortMatch> SplitPortMatch(
    const TrafficFilterPortMatch& portMatch, uint32_t maxCount)
{
    std::vector<TrafficFilterPortMatch> result;
    if (portMatch.type_ != static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_MULTI) ||
        portMatch.multi_.portCount_ <= maxCount) {
        result.push_back(portMatch);
        return result;
    }
    uint32_t count = portMatch.multi_.portCount_;
    for (uint32_t i = 0; i < count; i += maxCount) {
        TrafficFilterPortMatch split;
        split.type_ = portMatch.type_;
        split.invert_ = portMatch.invert_;
        uint32_t end = std::min(i + maxCount, count);
        split.multi_.portCount_ = end - i;
        for (uint32_t j = 0; j < split.multi_.portCount_; j++) {
            split.multi_.ports_[j] = portMatch.multi_.ports_[i + j];
        }
        result.push_back(split);
    }
    return result;
}

static bool IsMultiIPMatch(const TrafficFilterIPMatch& ipMatch)
{
    return ipMatch.type_ == static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_MULTI);
}

static std::vector<TrafficFilterIPMatch> ExpandMultiIPMatch(const TrafficFilterIPMatch& ipMatch)
{
    std::vector<TrafficFilterIPMatch> result;
    if (!IsMultiIPMatch(ipMatch)) {
        result.push_back(ipMatch);
        return result;
    }
    for (uint32_t i = 0; i < ipMatch.multi_.ipCount_; ++i) {
        TrafficFilterIPMatch single;
        single.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE);
        single.invert_ = false;
        single.single_ = ipMatch.multi_.ips_[i];
        result.push_back(single);
    }
    return result;
}

void NetTrafficFilterIptablesCommandBuilder::AppendMatchConditions(
    std::ostringstream& cmd, const TrafficFilterRedirectRule& rule)
{
    if (rule.protocol_ == NETTRAFFICFILTER_PROTO_TCP) {
        cmd << " -p tcp";
    }
    std::string srcIpMatch = FormatIPMatch(rule.srcIp_, true);
    if (!srcIpMatch.empty()) {
        cmd << srcIpMatch;
    }
    std::string dstIpMatch = FormatIPMatch(rule.dstIp_, false);
    if (!dstIpMatch.empty()) {
        cmd << dstIpMatch;
    }
    std::string srcPortMatch = FormatPortMatch(rule.srcPort_, true);
    if (!srcPortMatch.empty()) {
        cmd << srcPortMatch;
    }
    std::string dstPortMatch = FormatPortMatch(rule.dstPort_, false);
    if (!dstPortMatch.empty()) {
        cmd << dstPortMatch;
    }
    std::string inIfMatch = FormatInterfaceMatch(rule.inInterface_, true);
    if (!inIfMatch.empty()) {
        cmd << inIfMatch;
    }
    std::string outIfMatch = FormatInterfaceMatch(rule.outInterface_, false);
    if (!outIfMatch.empty()) {
        cmd << outIfMatch;
    }
    if (rule.uidStart_ != UID_UNSPEC && rule.uidEnd_ != UID_UNSPEC) {
        if (rule.uidStart_ == rule.uidEnd_) {
            cmd << " -m owner --uid-owner " << rule.uidStart_;
        } else {
            cmd << " -m owner --uid-owner " << rule.uidStart_ << "-" << rule.uidEnd_;
        }
    }
}

bool NetTrafficFilterIptablesCommandBuilder::AppendRedirectTarget(
    std::ostringstream& cmd, const TrafficFilterRedirectRule& rule)
{
    std::string proxyDest = FormatNatAddressWithPort(rule.proxyIp_, rule.proxyPort_);
    if (proxyDest.empty()) {
        NETMGR_EXT_LOG_E("AppendRedirectTarget failed: invalid proxy destination");
        return false;
    }
    cmd << " -j " << DNAT_TARGET << " --to-destination " << proxyDest;
    return true;
}

std::string NetTrafficFilterIptablesCommandBuilder::BuildRedirectCommandBase(
    const TrafficFilterRedirectRule& rule, const std::string& chainName,
    const std::string& action, const std::string& position)
{
    NETMGR_EXT_LOG_I("BuildRedirectCommandBase started: chainName=%{public}s, action=%{public}s",
        chainName.c_str(), action.c_str());
    std::ostringstream cmd;
    cmd << "-t nat " << action << " " << chainName;
    if (!position.empty()) {
        cmd << " " << position;
    }
    AppendMatchConditions(cmd, rule);
    if (!AppendRedirectTarget(cmd, rule)) {
        NETMGR_EXT_LOG_E("BuildRedirectCommandBase failed: invalid proxy destination");
        return "";
    }
    std::string result = cmd.str();
    NETMGR_EXT_LOG_I("BuildRedirectCommandBase completed, command: %{private}s", result.c_str());
    return result;
}

std::string NetTrafficFilterIptablesCommandBuilder::BuildRedirectCommandWithPosition(
    const TrafficFilterRedirectRule& rule, const std::string& chainName, uint32_t position)
{
    return BuildRedirectCommandBase(rule, chainName, "-I", std::to_string(position));
}

std::string NetTrafficFilterIptablesCommandBuilder::BuildFlushChainCommand(const std::string& chainName,
    const IptablesName tableName)
{
    const char* tblName = (tableName == IptablesName::FILTER) ? "filter" : "nat";
    std::string result;
    result.reserve(DEFAULT_IPTABLE_LEN);
    result.append("-t ")
        .append(tblName)
        .append(" -F ")
        .append(chainName);
    return result;
}

std::string NetTrafficFilterIptablesCommandBuilder::BuildCreateChainCommand(const std::string& chainName,
    const IptablesName tableName)
{
    const char* tblName = (tableName == IptablesName::FILTER) ? "filter" : "nat";
    std::string result;
    result.reserve(DEFAULT_IPTABLE_LEN);
    result.append("-t ")
        .append(tblName)
        .append(" -N ")
        .append(chainName);
    return result;
}

std::string NetTrafficFilterIptablesCommandBuilder::BuildDeleteChainCommand(const std::string& chainName,
    const IptablesName tableName)
{
    const char* tblName = (tableName == IptablesName::FILTER) ? "filter" : "nat";
    std::string result;
    result.reserve(DEFAULT_IPTABLE_LEN);
    result.append("-t ")
        .append(tblName)
        .append(" -X ")
        .append(chainName);
    return result;
}

std::string NetTrafficFilterIptablesCommandBuilder::BuildInsertJumpToChainCommand(
    const std::string& fromHook, const std::string& chainName, const IptablesName tableName)
{
    return BuildInsertJumpToChainCommand(fromHook, chainName, 1, tableName);
}

std::string NetTrafficFilterIptablesCommandBuilder::BuildInsertJumpToChainCommand(
    const std::string& fromHook, const std::string& chainName, uint32_t position, const IptablesName tableName)
{
    if (fromHook.empty() || chainName.empty() || position == 0) {
        return "";
    }
    const char* tblName = (tableName == IptablesName::FILTER) ? "filter" : "nat";
    std::string result;
    result.reserve(DEFAULT_IPTABLE_LEN);
    result.append("-t ")
        .append(tblName)
        .append(" -I ")
        .append(fromHook)
        .append(" ")
        .append(std::to_string(position))
        .append(" -j ")
        .append(chainName);
    return result;
}

std::string NetTrafficFilterIptablesCommandBuilder::BuildAppendRedirectCommand(
    const TrafficFilterRedirectRule& rule, const std::string& chainName)
{
    return BuildRedirectCommandBase(rule, chainName, "-A", "");
}

std::string NetTrafficFilterIptablesCommandBuilder::BuildAppendPauseRuleCommand(const std::string& chainName)
{
    return "-t nat -A " + chainName + " -j RETURN -m comment --comment NETFIREWALL_PAUSE_MARKER";
}

std::string NetTrafficFilterIptablesCommandBuilder::BuildDeleteJumpCommand(
    const std::string& fromHook, const std::string& chainName, const IptablesName tableName)
{
    const char* tblName = (tableName == IptablesName::FILTER) ? "filter" : "nat";
    std::string result;
    result.reserve(DEFAULT_IPTABLE_LEN);
    result.append("-t ")
        .append(tblName)
        .append(" -D ")
        .append(fromHook)
        .append(" -j ")
        .append(chainName);
    return result;
}

std::string NetTrafficFilterIptablesCommandBuilder::BuildInsertPauseRuleCommand(const std::string& chainName)
{
    return "-t nat -I " + chainName + " 1 -j RETURN -m comment --comment NETFIREWALL_PAUSE_MARKER";
}

std::string NetTrafficFilterIptablesCommandBuilder::GenerateChainName(int32_t uid, uint32_t groupId)
{
    std::string chainName = "TR_" + std::to_string(uid) + "_GRP_" + std::to_string(groupId);
    NETMGR_EXT_LOG_I("GenerateChainName result: %{public}s", chainName.c_str());
    return chainName;
}

std::string NetTrafficFilterIptablesCommandBuilder::FormatIPMatch(const TrafficFilterIPMatch& ipMatch, bool isSource)
{
    if (ipMatch.type_ == static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY)) {
        return "";
    }

    std::ostringstream oss;
    std::string prefix = ipMatch.invert_ ? " !" : "";
    std::string ipStr;
    std::string direction = isSource ? " -s " : " -d ";

    switch (ipMatch.type_) {
        case static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE):
            ipStr = FormatIPAddress(ipMatch.single_);
            oss << prefix << direction << ipStr;
            break;
        case static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_CIDR):
            ipStr = FormatIPAddress(ipMatch.cidr_.base_);
            oss << prefix << direction << ipStr << "/" << static_cast<int>(ipMatch.cidr_.prefixLen_);
            break;
        case static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_RANGE): {
            std::string startIp = FormatIPAddress(ipMatch.range_.start_);
            std::string endIp = FormatIPAddress(ipMatch.range_.end_);
            oss << " -m iprange ";
            if (ipMatch.invert_) {
                oss << "! ";
            }
            if (isSource) {
                oss << "--src-range " << startIp << "-" << endIp;
            } else {
                oss << "--dst-range " << startIp << "-" << endIp;
            }
            break;
        }
        case static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_MULTI): {
            std::string ipList;
            for (uint32_t i = 0; i < ipMatch.multi_.ipCount_; i++) {
                if (i > 0) {
                    ipList += ",";
                }
                ipList += FormatIPAddress(ipMatch.multi_.ips_[i]);
            }
            oss << prefix << direction << ipList;
            break;
        }
        default:
            break;
    }

    return oss.str();
}

std::string NetTrafficFilterIptablesCommandBuilder::FormatPortMatch(
    const TrafficFilterPortMatch& portMatch, bool isSource)
{
    if (portMatch.type_ == static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY)) {
        return "";
    }
    std::ostringstream oss;
    const std::string tcpPortOpt = isSource ? "--sport " : "--dport ";
    const std::string multiPortOpt = isSource ? "--sports " : "--dports ";
    switch (portMatch.type_) {
        case static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_SINGLE): {
            oss << " -m tcp ";
            if (portMatch.invert_) {
                oss << "! ";
            }
            oss << tcpPortOpt << portMatch.single_;
            break;
        }
        case static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_RANGE): {
            if (!portMatch.invert_ && portMatch.range_.startPort_ == 0 && portMatch.range_.endPort_ == PORT_MAX) {
                return "";
            }
            oss << " -m tcp ";
            if (portMatch.invert_) {
                oss << "! ";
            }
            oss << tcpPortOpt << portMatch.range_.startPort_ << ":" << portMatch.range_.endPort_;
            break;
        }
        case static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_MULTI): {
            oss << " -m multiport ";
            if (portMatch.invert_) {
                oss << "! ";
            }
            oss << multiPortOpt;
            for (uint32_t i = 0; i < portMatch.multi_.portCount_; i++) {
                if (i > 0) {
                    oss << ",";
                }
                oss << portMatch.multi_.ports_[i];
            }
            break;
        }
        default:
            break;
    }
    return oss.str();
}

std::string NetTrafficFilterIptablesCommandBuilder::FormatInterfaceMatch(
    const TrafficFilterInterfaceMatch& ifMatch, bool isIncoming)
{
    if (!ifMatch.enabled_ || ifMatch.ifName_.empty()) {
        return "";
    }
    std::ostringstream oss;
    std::string interfaceFlag = isIncoming ? "-i" : "-o";
    oss << " ";
    if (ifMatch.invert_) {
        oss << "! ";
    }
    oss << interfaceFlag << " " << ifMatch.ifName_;
    return oss.str();
}

std::string NetTrafficFilterIptablesCommandBuilder::FormatIPAddress(const TrafficFilterIPAddress& ipAddr)
{
    if (ipAddr.family_ == static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4)) {
        char buf[INET_ADDRSTRLEN] = {0};
        if (inet_ntop(AF_INET, ipAddr.addr_, buf, sizeof(buf)) == nullptr) {
            NETMGR_EXT_LOG_E("FormatIPAddress failed to format IPv4 address");
            return "";
        }
        return std::string(buf);
    }
    if (ipAddr.family_ == static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V6)) {
        char buf[INET6_ADDRSTRLEN] = {0};
        if (inet_ntop(AF_INET6, ipAddr.addr_, buf, sizeof(buf)) == nullptr) {
            NETMGR_EXT_LOG_E("FormatIPAddress failed to format IPv6 address");
            return "";
        }
        return std::string(buf);
    }
    NETMGR_EXT_LOG_E("FormatIPAddress failed: invalid family=%{public}d", ipAddr.family_);
    return "";
}

std::string NetTrafficFilterIptablesCommandBuilder::GetHookPointName(TrafficFilterHookPoint hookPoint)
{
    switch (hookPoint) {
        case TrafficFilterHookPoint::HOOK_PREROUTING:
            return "PREROUTING";
        case TrafficFilterHookPoint::HOOK_INPUT:
            return "INPUT";
        case TrafficFilterHookPoint::HOOK_OUTPUT:
            return "OUTPUT";
        case TrafficFilterHookPoint::HOOK_POSTROUTING:
            return "POSTROUTING";
        case TrafficFilterHookPoint::HOOK_FORWARD:
            return "FORWARD";
        default:
            NETMGR_EXT_LOG_W("Unknown hook point: %{public}d", static_cast<int32_t>(hookPoint));
            return "";
    }
}

std::string NetTrafficFilterIptablesCommandBuilder::FormatNatAddressWithPort(
    const TrafficFilterIPAddress& ipAddr, uint16_t port)
{
    std::string ip = FormatIPAddress(ipAddr);
    if (ip.empty()) {
        NETMGR_EXT_LOG_E("FormatNatAddressWithPort failed: empty ip");
        return "";
    }
    if (ipAddr.family_ == static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V6)) {
        return "[" + ip + "]:" + std::to_string(port);
    }
    if (ipAddr.family_ == static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4)) {
        return ip + ":" + std::to_string(port);
    }
    NETMGR_EXT_LOG_E("FormatNatAddressWithPort failed: invalid family=%{public}d", ipAddr.family_);
    return "";
}

int32_t NetTrafficFilterIptablesCommandBuilder::ExecuteIptablesCommand(
    const std::string& command, TrafficFilterIPFamily family)
{
    if (command.empty()) {
        NETMGR_EXT_LOG_E("empty iptables command");
        return -1;
    }
    std::string respond;
    NetsysNative::IptablesType ipType = NetsysNative::IptablesType::IPTYPE_IPV4;
    switch (family) {
        case TrafficFilterIPFamily::IP_FAMILY_UNSPEC:
            ipType = NetsysNative::IptablesType::IPTYPE_IPV4V6;
            break;
        case TrafficFilterIPFamily::IP_FAMILY_V4:
            ipType = NetsysNative::IptablesType::IPTYPE_IPV4;
            break;
        case TrafficFilterIPFamily::IP_FAMILY_V6:
            ipType = NetsysNative::IptablesType::IPTYPE_IPV6;
            break;
        case TrafficFilterIPFamily::IP_FAMILY_V4V6:
            ipType = NetsysNative::IptablesType::IPTYPE_IPV4V6;
            break;
        default:
            NETMGR_EXT_LOG_E("invalid ipType");
            return -1;
    }
    int32_t ret = NetsysController::GetInstance().SetIptablesCommandForRes(
        command, respond, ipType);
    if (ret != 0) {
        NETMGR_EXT_LOG_E("Failed to execute iptables command: %{private}s, error: %{public}s",
            command.c_str(), respond.c_str());
        return -1;
    }
    NETMGR_EXT_LOG_I("Executed iptables command: %{private}s", command.c_str());
    return 0;
}

std::string NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(
    const TrafficFilterPacketRule& rule, const std::string& chainName, int32_t queueNum)
{
    return BuildPacketFilterCommand(rule, rule.srcIp_, rule.dstIp_, rule.srcPort_, rule.dstPort_, chainName,
        BuildNfqueueAction(queueNum));
}

std::string NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(
    const TrafficFilterPacketRule& rule, const TrafficFilterPortMatch& srcPort,
    const TrafficFilterPortMatch& dstPort, const std::string& chainName, int32_t queueNum)
{
    return BuildPacketFilterCommand(rule, rule.srcIp_, rule.dstIp_, srcPort, dstPort, chainName,
        BuildNfqueueAction(queueNum));
}

std::string NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(
    const TrafficFilterPacketRule& rule, const TrafficFilterIPMatch& srcIp,
    const TrafficFilterIPMatch& dstIp, const TrafficFilterPortMatch& srcPort,
    const TrafficFilterPortMatch& dstPort, const std::string& chainName,
    const std::string& action)
{
    std::string cmd;
    cmd.reserve(DEFAULT_IPTABLE_LEN);
    cmd.append(FILTER_TABLE_APPEND).append(chainName);
    AppendPacketMatchConditions(cmd, rule, srcIp, dstIp, srcPort, dstPort);
    cmd.append(TARGET_JUMP_PREFIX).append(action);
    return cmd;
}

void NetTrafficFilterIptablesCommandBuilder::AppendPacketMatchConditions(std::string& cmd,
    const TrafficFilterPacketRule& rule, const TrafficFilterIPMatch& srcIp, const TrafficFilterIPMatch& dstIp,
    const TrafficFilterPortMatch& srcPort, const TrafficFilterPortMatch& dstPort)
{
    bool needTcp = (rule.protocol_ == NETTRAFFICFILTER_PROTO_TCP) ||
                   (rule.protocol_ == NETTRAFFICFILTER_PROTO_ANY && rule.tcpFlagsMatch_.enable_);
    if (needTcp) {
        cmd.append(PROTO_TCP_FLAG);
    } else if (rule.protocol_ != NETTRAFFICFILTER_PROTO_ANY) {
        cmd.append(PROTO_PREFIX).append(std::to_string(rule.protocol_));
    }

    auto appendIfNotEmpty = [&](const std::string& match) {
        if (!match.empty()) {
            cmd.append(match);
        }
    };

    appendIfNotEmpty(FormatIPMatch(srcIp, true));
    appendIfNotEmpty(FormatIPMatch(dstIp, false));
    appendIfNotEmpty(FormatPortMatch(srcPort, true));
    appendIfNotEmpty(FormatPortMatch(dstPort, false));
    appendIfNotEmpty(FormatInterfaceMatch(rule.inInterface_, true));
    appendIfNotEmpty(FormatInterfaceMatch(rule.outInterface_, false));
    appendIfNotEmpty(FormatMacMatch(rule.macMatch_));
    appendIfNotEmpty(FormatTcpFlagsMatch(rule.tcpFlagsMatch_));
    appendIfNotEmpty(FormatConntrackMatch(rule.conntrackMatch_));

    if (rule.uidStart_ != UID_UNSPEC && rule.uidEnd_ != UID_UNSPEC &&
        rule.hookPoint_ == static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT)) {
        cmd.append(OWNER_MATCH_PREFIX).append(std::to_string(rule.uidStart_));
        if (rule.uidStart_ != rule.uidEnd_) {
            cmd.append(UID_RANGE_SEPARATOR).append(std::to_string(rule.uidEnd_));
        }
    }
}

std::string NetTrafficFilterIptablesCommandBuilder::FormatMacMatch(const TrafficFilterMACMatch& macMatch)
{
    if (!macMatch.enable_ || macMatch.srcMac_.empty()) {
        return "";
    }
    std::string result;
    result.reserve(DEFAULT_IPTABLE_LEN);
    result.append(MAC_MATCH_PREFIX);
    if (macMatch.invert_) {
        result.append(" !");
    }
    result.append(MAC_SOURCE_PREFIX).append(macMatch.srcMac_);
    return result;
}

std::string NetTrafficFilterIptablesCommandBuilder::FormatTcpFlagsMatch(const TrafficFilterTCPFlagsMatch& tcpFlags)
{
    if (!tcpFlags.enable_ || tcpFlags.flagMask_ == 0) {
        return "";
    }
    std::string maskStr = FormatBitMask(tcpFlags.flagMask_, GetTcpFlagName);
    uint8_t comp = tcpFlags.flagComp_ & tcpFlags.flagMask_;
    std::string compStr = FormatBitMask(comp, GetTcpFlagName);
    if (compStr.empty()) {
        compStr = "NONE";
    }
    std::string result;
    result.reserve(DEFAULT_IPTABLE_LEN);
    result.append(TCP_FLAGS_MATCH_PREFIX).append(maskStr).append(" ").append(compStr);
    return result;
}

std::string NetTrafficFilterIptablesCommandBuilder::FormatConntrackMatch(const TrafficFilterConntrackMatch& ctMatch)
{
    if (!ctMatch.enable_ || ctMatch.stateMask_ == 0) {
        return "";
    }
    std::string states = FormatBitMask(ctMatch.stateMask_, GetConntrackStateName);
    if (states.empty()) {
        return "";
    }
    std::string result;
    result.reserve(DEFAULT_IPTABLE_LEN);
    result.append(CONNTRACK_MATCH_PREFIX).append(states);
    return result;
}

static void AppendPacketFilterForAllPorts(std::vector<std::string>& commands,
    const TrafficFilterPacketRule& rule, const TrafficFilterIPMatch& srcIp,
    const TrafficFilterIPMatch& dstIp, const std::vector<TrafficFilterPortMatch>& srcPortSplits,
    const std::vector<TrafficFilterPortMatch>& dstPortSplits, const std::string& chainName,
    const std::string& action)
{
    for (const auto& srcPort : srcPortSplits) {
        for (const auto& dstPort : dstPortSplits) {
            commands.push_back(NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(
                rule, srcIp, dstIp, srcPort, dstPort, chainName, action));
        }
    }
}

static std::vector<TrafficFilterIPMatch> GetBlockIPOptions(const TrafficFilterIPMatch& ipMatch)
{
    return (IsMultiIPMatch(ipMatch) && ipMatch.invert_) ? ExpandMultiIPMatch(ipMatch)
                                                        : std::vector<TrafficFilterIPMatch>{};
}

static std::vector<TrafficFilterIPMatch> GetTargetIPOptions(const TrafficFilterIPMatch& ipMatch)
{
    if (IsMultiIPMatch(ipMatch) && !ipMatch.invert_) {
        return ExpandMultiIPMatch(ipMatch);
    }
    if (IsMultiIPMatch(ipMatch) && ipMatch.invert_) {
        return std::vector<TrafficFilterIPMatch>{TrafficFilterIPMatch()};
    }
    return std::vector<TrafficFilterIPMatch>{ipMatch};
}

static TrafficFilterIPMatch GetDefaultIPMatch(const TrafficFilterIPMatch& ipMatch)
{
    return IsMultiIPMatch(ipMatch) ? TrafficFilterIPMatch() : ipMatch;
}

std::vector<std::string> NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommands(
    const TrafficFilterPacketRule& rule, const std::string& chainName, int32_t queueNum)
{
    constexpr uint32_t MAX_MULTIPORT = 15;
    std::vector<std::string> commands;
    auto srcPortSplits = SplitPortMatch(rule.srcPort_, MAX_MULTIPORT);
    auto dstPortSplits = SplitPortMatch(rule.dstPort_, MAX_MULTIPORT);
    auto srcBlock = GetBlockIPOptions(rule.srcIp_);
    auto dstBlock = GetBlockIPOptions(rule.dstIp_);
    auto srcTarget = GetTargetIPOptions(rule.srcIp_);
    auto dstTarget = GetTargetIPOptions(rule.dstIp_);
    bool srcNonInvMulti = IsMultiIPMatch(rule.srcIp_) && !rule.srcIp_.invert_;
    bool dstNonInvMulti = IsMultiIPMatch(rule.dstIp_) && !rule.dstIp_.invert_;

    auto appendAll = [&](const std::vector<TrafficFilterIPMatch>& srcIps,
                         const std::vector<TrafficFilterIPMatch>& dstIps,
                         const std::string& action) {
        for (const auto& srcIp : srcIps) {
            for (const auto& dstIp : dstIps) {
                AppendPacketFilterForAllPorts(commands, rule, srcIp, dstIp,
                    srcPortSplits, dstPortSplits, chainName, action);
            }
        }
    };

    appendAll(srcBlock, dstTarget, RETURN_TARGET);
    appendAll(srcTarget, dstBlock, RETURN_TARGET);
    appendAll(srcTarget, dstTarget, BuildNfqueueAction(queueNum));

    if (srcNonInvMulti || dstNonInvMulti) {
        appendAll(std::vector<TrafficFilterIPMatch>{GetDefaultIPMatch(rule.srcIp_)},
                  std::vector<TrafficFilterIPMatch>{GetDefaultIPMatch(rule.dstIp_)}, RETURN_TARGET);
    }

    return commands;
}
} // namespace NetManagerStandard
} // namespace OHOS
