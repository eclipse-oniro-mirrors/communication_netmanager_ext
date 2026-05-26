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
#include <arpa/inet.h>
#include <sstream>
#include <securec.h>

namespace OHOS {
namespace NetManagerStandard {
const std::string DNAT_TARGET = "DNAT";
constexpr uint32_t UID_UNSPEC = static_cast<uint32_t>(-1);

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

std::string NetTrafficFilterIptablesCommandBuilder::BuildFlushChainCommand(const std::string& chainName)
{
    return "-t nat -F " + chainName;
}

std::string NetTrafficFilterIptablesCommandBuilder::BuildCreateChainCommand(const std::string& chainName)
{
    return "-t nat -N " + chainName;
}

std::string NetTrafficFilterIptablesCommandBuilder::BuildDeleteChainCommand(const std::string& chainName)
{
    return "-t nat -X " + chainName;
}

std::string NetTrafficFilterIptablesCommandBuilder::BuildInsertJumpToChainCommand(
    const std::string& fromHook, const std::string& chainName)
{
    return BuildInsertJumpToChainCommand(fromHook, chainName, 1);
}

std::string NetTrafficFilterIptablesCommandBuilder::BuildInsertJumpToChainCommand(
    const std::string& fromHook, const std::string& chainName, uint32_t position)
{
    if (fromHook.empty() || chainName.empty() || position == 0) {
        return "";
    }
    return "-t nat -I " + fromHook + " " + std::to_string(position) + " -j " + chainName;
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
    const std::string& fromHook, const std::string& chainName)
{
    return "-t nat -D " + fromHook + " -j " + chainName;
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
            if (!portMatch.invert_ && portMatch.range_.startPort_ == 0 && portMatch.range_.endPort_ == 65535) {
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

} // namespace NetManagerStandard
} // namespace OHOS
