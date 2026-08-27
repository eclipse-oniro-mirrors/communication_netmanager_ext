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

#ifndef NETTRAFFICFILTER_IPTABLES_COMMAND_BUILDER_H
#define NETTRAFFICFILTER_IPTABLES_COMMAND_BUILDER_H

#include <string>
#include "netfirewall_common.h"

namespace OHOS {
namespace NetManagerStandard {
enum IptablesName {
    NAT = 0,
    FILTER
};
class NetTrafficFilterIptablesCommandBuilder {
public:
    static std::string BuildRedirectCommandWithPosition(const TrafficFilterRedirectRule& rule,
        const std::string& chainName, uint32_t position);
    static std::string BuildFlushChainCommand(const std::string& chainName,
        const IptablesName tableName = IptablesName::NAT);
    static std::string BuildCreateChainCommand(const std::string& chainName,
        const IptablesName tableName = IptablesName::NAT);
    static std::string BuildDeleteChainCommand(const std::string& chainName,
        const IptablesName tableName = IptablesName::NAT);
    static std::string BuildInsertJumpToChainCommand(const std::string& fromHook, const std::string& chainName,
        const IptablesName tableName = IptablesName::NAT);
    static std::string BuildInsertJumpToChainCommand(const std::string& fromHook, const std::string& chainName,
        uint32_t position, const IptablesName tableName = IptablesName::NAT);
    static std::string BuildAppendRedirectCommand(const TrafficFilterRedirectRule& rule, const std::string& chainName);
    static std::string BuildAppendPauseRuleCommand(const std::string& chainName);
    static std::string BuildDeleteJumpCommand(const std::string& fromHook, const std::string& chainName,
        const IptablesName tableName = IptablesName::NAT);
    static std::string BuildInsertPauseRuleCommand(const std::string& chainName);
    static std::string GenerateChainName(int32_t uid, uint32_t groupId);
    static std::string GetHookPointName(TrafficFilterHookPoint hookPoint);
    static int32_t ExecuteIptablesCommand(const std::string& command, TrafficFilterIPFamily family);

    static std::string BuildPacketFilterCommand(const TrafficFilterPacketRule& rule,
        const std::string& chainName, int32_t queueNum);
    static std::vector<std::string> BuildPacketFilterCommands(
        const TrafficFilterPacketRule& rule, const std::string& chainName, int32_t queueNum);
    static std::string BuildPacketFilterCommand(const TrafficFilterPacketRule& rule,
        const TrafficFilterIPMatch& srcIp, const TrafficFilterIPMatch& dstIp,
        const TrafficFilterPortMatch& srcPort, const TrafficFilterPortMatch& dstPort,
        const std::string& chainName, const std::string& action);

private:
    static void AppendMatchConditions(std::ostringstream& cmd, const TrafficFilterRedirectRule& rule);
    static bool AppendRedirectTarget(std::ostringstream& cmd, const TrafficFilterRedirectRule& rule);
    static std::string BuildRedirectCommandBase(const TrafficFilterRedirectRule& rule, const std::string& chainName,
        const std::string& action, const std::string& position = "");
    static std::string FormatIPMatch(const TrafficFilterIPMatch& ipMatch, bool isSource);
    static std::string FormatPortMatch(const TrafficFilterPortMatch& portMatch, bool isSource,
        uint8_t protocol = NETTRAFFICFILTER_PROTO_TCP);
    static std::string FormatInterfaceMatch(const TrafficFilterInterfaceMatch& ifMatch, bool isIncoming);
    static std::string FormatIPAddress(const TrafficFilterIPAddress& ipAddr);
    static std::string FormatNatAddressWithPort(const TrafficFilterIPAddress& ipAddr, uint16_t port);
    static std::string BuildPacketFilterCommand(const TrafficFilterPacketRule& rule,
        const TrafficFilterPortMatch& srcPort, const TrafficFilterPortMatch& dstPort,
        const std::string& chainName, int32_t queueNum);
    static std::string FormatMacMatch(const TrafficFilterMACMatch& macMatch);
    static std::string FormatTcpFlagsMatch(const TrafficFilterTCPFlagsMatch& tcpFlags);
    static std::string FormatConntrackMatch(const TrafficFilterConntrackMatch& ctMatch);
    static void AppendPacketMatchConditions(std::string& cmd, const TrafficFilterPacketRule& rule,
        const TrafficFilterIPMatch& srcIp, const TrafficFilterIPMatch& dstIp,
        const TrafficFilterPortMatch& srcPort, const TrafficFilterPortMatch& dstPort);
};

} // namespace NetManagerStandard
} // namespace OHOS

#endif // NETTRAFFICFILTER_IPTABLES_COMMAND_BUILDER_H
