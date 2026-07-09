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

#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <cstring>

#define private public
#define protected public

#include "nettrafficfilter_iptables_command_builder.h"
#include "netfirewall_common.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;

constexpr uint32_t UID_UNSPEC = static_cast<uint32_t>(-1);

void SetupIPv4Address(TrafficFilterIPAddress& ipAddr, const char* ipv4Str)
{
    ipAddr.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4);
    inet_pton(AF_INET, ipv4Str, ipAddr.addr_);
}

void SetupIPv6Address(TrafficFilterIPAddress& ipAddr, const char* ipv6Str)
{
    ipAddr.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V6);
    inet_pton(AF_INET6, ipv6Str, ipAddr.addr_);
}

} // namespace

class NetTrafficFilterIptablesCommandBuilderTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildCreateChainCommand001, TestSize.Level1)
{
    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildCreateChainCommand(chainName);
    EXPECT_EQ(result, "-t nat -N " + chainName);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildDeleteChainCommand001, TestSize.Level1)
{
    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildDeleteChainCommand(chainName);
    EXPECT_EQ(result, "-t nat -X " + chainName);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildFlushChainCommand001, TestSize.Level1)
{
    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildFlushChainCommand(chainName);
    EXPECT_EQ(result, "-t nat -F " + chainName);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, GetHookPointName001, TestSize.Level1)
{
    EXPECT_EQ(NetTrafficFilterIptablesCommandBuilder::GetHookPointName(
        TrafficFilterHookPoint::HOOK_PREROUTING), "PREROUTING");
    EXPECT_EQ(NetTrafficFilterIptablesCommandBuilder::GetHookPointName(
        TrafficFilterHookPoint::HOOK_INPUT), "INPUT");
    EXPECT_EQ(NetTrafficFilterIptablesCommandBuilder::GetHookPointName(
        TrafficFilterHookPoint::HOOK_OUTPUT), "OUTPUT");
    EXPECT_EQ(NetTrafficFilterIptablesCommandBuilder::GetHookPointName(
        TrafficFilterHookPoint::HOOK_POSTROUTING), "POSTROUTING");
    EXPECT_EQ(NetTrafficFilterIptablesCommandBuilder::GetHookPointName(
        TrafficFilterHookPoint::HOOK_FORWARD), "FORWARD");
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, GetHookPointName002, TestSize.Level1)
{
    TrafficFilterHookPoint invalidHook = static_cast<TrafficFilterHookPoint>(999);
    EXPECT_EQ(NetTrafficFilterIptablesCommandBuilder::GetHookPointName(invalidHook), "");
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, GenerateChainName001, TestSize.Level1)
{
    std::string result = NetTrafficFilterIptablesCommandBuilder::GenerateChainName(10042, 1001);
    EXPECT_EQ(result, "TR_10042_GRP_1001");
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildInsertJumpToChainCommand001, TestSize.Level1)
{
    std::string fromHook = "PREROUTING";
    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildInsertJumpToChainCommand(fromHook, chainName);
    EXPECT_EQ(result, "-t nat -I " + fromHook + " 1 -j " + chainName);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildInsertJumpToChainCommandWithPosition, TestSize.Level1)
{
    std::string fromHook = "PREROUTING";
    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildInsertJumpToChainCommand(
        fromHook, chainName, 2);
    EXPECT_EQ(result, "-t nat -I " + fromHook + " 2 -j " + chainName);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildInsertJumpToChainCommandEmptyParams, TestSize.Level1)
{
    EXPECT_EQ(NetTrafficFilterIptablesCommandBuilder::BuildInsertJumpToChainCommand("", "CHAIN", 1), "");
    EXPECT_EQ(NetTrafficFilterIptablesCommandBuilder::BuildInsertJumpToChainCommand("PREROUTING", "", 1), "");
    EXPECT_EQ(NetTrafficFilterIptablesCommandBuilder::BuildInsertJumpToChainCommand("PREROUTING", "CHAIN", 0), "");
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildDeleteJumpCommand001, TestSize.Level1)
{
    std::string fromHook = "PREROUTING";
    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildDeleteJumpCommand(fromHook, chainName);
    EXPECT_EQ(result, "-t nat -D " + fromHook + " -j " + chainName);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildAppendPauseRuleCommand001, TestSize.Level1)
{
    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildAppendPauseRuleCommand(chainName);
    EXPECT_EQ(result, "-t nat -A " + chainName + " -j RETURN -m comment --comment NETFIREWALL_PAUSE_MARKER");
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildInsertPauseRuleCommand001, TestSize.Level1)
{
    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildInsertPauseRuleCommand(chainName);
    EXPECT_EQ(result, "-t nat -I " + chainName + " 1 -j RETURN -m comment --comment NETFIREWALL_PAUSE_MARKER");
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildAppendRedirectCommandWithTCPPacket, TestSize.Level1)
{
    TrafficFilterRedirectRule rule;
    rule.priority_ = 100;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_PREROUTING);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    SetupIPv4Address(rule.proxyIp_, "192.168.1.100");
    rule.proxyPort_ = 8080;
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildAppendRedirectCommand(rule, chainName);
    EXPECT_TRUE(result.find("-p tcp") != std::string::npos);
    EXPECT_TRUE(result.find("-j DNAT") != std::string::npos);
    EXPECT_TRUE(result.find("--to-destination 192.168.1.100:8080") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildAppendRedirectCommandIPv6Proxy, TestSize.Level1)
{
    TrafficFilterRedirectRule rule;
    rule.priority_ = 100;
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    SetupIPv6Address(rule.proxyIp_, "2001:db8::1");
    rule.proxyPort_ = 8080;
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;

    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildAppendRedirectCommand(rule, chainName);
    EXPECT_TRUE(result.find("--to-destination [2001:db8::1]:8080") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildRedirectCommandWithInvalidProxy, TestSize.Level1)
{
    TrafficFilterRedirectRule rule;
    rule.priority_ = 100;
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.proxyIp_.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_UNSPEC);
    rule.proxyPort_ = 8080;
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;

    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildAppendRedirectCommand(rule, chainName);
    EXPECT_EQ(result, "");
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildRedirectCommandWithSingleUID, TestSize.Level1)
{
    TrafficFilterRedirectRule rule;
    rule.priority_ = 100;
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    SetupIPv4Address(rule.proxyIp_, "192.168.1.100");
    rule.proxyPort_ = 8080;
    rule.uidStart_ = 10042;
    rule.uidEnd_ = 10042;

    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildAppendRedirectCommand(rule, chainName);
    EXPECT_TRUE(result.find("-m owner --uid-owner 10042") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildRedirectCommandWithUIDRange, TestSize.Level1)
{
    TrafficFilterRedirectRule rule;
    rule.priority_ = 100;
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    SetupIPv4Address(rule.proxyIp_, "192.168.1.100");
    rule.proxyPort_ = 8080;
    rule.uidStart_ = 10042;
    rule.uidEnd_ = 10100;

    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildAppendRedirectCommand(rule, chainName);
    EXPECT_TRUE(result.find("-m owner --uid-owner 10042-10100") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildRedirectCommandWithSingleSourceIP, TestSize.Level1)
{
    TrafficFilterRedirectRule rule;
    rule.priority_ = 100;
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE);
    SetupIPv4Address(rule.srcIp_.single_, "192.168.1.10");
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    SetupIPv4Address(rule.proxyIp_, "192.168.1.100");
    rule.proxyPort_ = 8080;
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;

    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildAppendRedirectCommand(rule, chainName);
    EXPECT_TRUE(result.find("-s 192.168.1.10") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildRedirectCommandWithInvertedSourceIP, TestSize.Level1)
{
    TrafficFilterRedirectRule rule;
    rule.priority_ = 100;
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE);
    SetupIPv4Address(rule.srcIp_.single_, "192.168.1.10");
    rule.srcIp_.invert_ = true;
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    SetupIPv4Address(rule.proxyIp_, "192.168.1.100");
    rule.proxyPort_ = 8080;
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;

    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildAppendRedirectCommand(rule, chainName);
    EXPECT_TRUE(result.find(" ! -s 192.168.1.10") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildRedirectCommandWithCIDRIp, TestSize.Level1)
{
    TrafficFilterRedirectRule rule;
    rule.priority_ = 100;
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_CIDR);
    SetupIPv4Address(rule.dstIp_.cidr_.base_, "192.168.1.0");
    rule.dstIp_.cidr_.prefixLen_ = 24;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    SetupIPv4Address(rule.proxyIp_, "192.168.1.100");
    rule.proxyPort_ = 8080;
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;

    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildAppendRedirectCommand(rule, chainName);
    EXPECT_TRUE(result.find("-d 192.168.1.0/24") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildRedirectCommandWithIPRange, TestSize.Level1)
{
    TrafficFilterRedirectRule rule;
    rule.priority_ = 100;
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_RANGE);
    SetupIPv4Address(rule.srcIp_.range_.start_, "192.168.1.1");
    SetupIPv4Address(rule.srcIp_.range_.end_, "192.168.1.100");
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    SetupIPv4Address(rule.proxyIp_, "192.168.1.100");
    rule.proxyPort_ = 8080;
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;

    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildAppendRedirectCommand(rule, chainName);
    EXPECT_TRUE(result.find("-m iprange --src-range 192.168.1.1-192.168.1.100") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildRedirectCommandWithInvertedIPRange, TestSize.Level1)
{
    TrafficFilterRedirectRule rule;
    rule.priority_ = 100;
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_RANGE);
    SetupIPv4Address(rule.dstIp_.range_.start_, "192.168.1.1");
    SetupIPv4Address(rule.dstIp_.range_.end_, "192.168.1.100");
    rule.dstIp_.invert_ = true;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    SetupIPv4Address(rule.proxyIp_, "192.168.1.100");
    rule.proxyPort_ = 8080;
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;

    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildAppendRedirectCommand(rule, chainName);
    EXPECT_TRUE(result.find("-m iprange ! --dst-range 192.168.1.1-192.168.1.100") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildRedirectCommandWithMultiIP, TestSize.Level1)
{
    TrafficFilterRedirectRule rule;
    rule.priority_ = 100;
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_MULTI);
    rule.srcIp_.multi_.ipCount_ = 3;
    SetupIPv4Address(rule.srcIp_.multi_.ips_[0], "192.168.1.10");
    SetupIPv4Address(rule.srcIp_.multi_.ips_[1], "192.168.1.20");
    SetupIPv4Address(rule.srcIp_.multi_.ips_[2], "192.168.1.30");
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    SetupIPv4Address(rule.proxyIp_, "192.168.1.100");
    rule.proxyPort_ = 8080;
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;

    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildAppendRedirectCommand(rule, chainName);
    EXPECT_TRUE(result.find("-s 192.168.1.10,192.168.1.20,192.168.1.30") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildRedirectCommandWithSingleSourcePort, TestSize.Level1)
{
    TrafficFilterRedirectRule rule;
    rule.priority_ = 100;
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_SINGLE);
    rule.srcPort_.single_ = 8080;
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    SetupIPv4Address(rule.proxyIp_, "192.168.1.100");
    rule.proxyPort_ = 8080;
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildAppendRedirectCommand(rule, chainName);
    EXPECT_TRUE(result.find("-m tcp --sport 8080") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildRedirectCommandWithInvertedSourcePort, TestSize.Level1)
{
    TrafficFilterRedirectRule rule;
    rule.priority_ = 100;
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_SINGLE);
    rule.srcPort_.single_ = 8080;
    rule.srcPort_.invert_ = true;
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    SetupIPv4Address(rule.proxyIp_, "192.168.1.100");
    rule.proxyPort_ = 8080;
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildAppendRedirectCommand(rule, chainName);
    EXPECT_TRUE(result.find("-m tcp ! --sport 8080") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildRedirectCommandWithPortRange, TestSize.Level1)
{
    TrafficFilterRedirectRule rule;
    rule.priority_ = 100;
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_RANGE);
    rule.dstPort_.range_.startPort_ = 80;
    rule.dstPort_.range_.endPort_ = 443;
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    SetupIPv4Address(rule.proxyIp_, "192.168.1.100");
    rule.proxyPort_ = 8080;
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildAppendRedirectCommand(rule, chainName);
    EXPECT_TRUE(result.find("-m tcp --dport 80:443") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildRedirectCommandWithFullPortRange, TestSize.Level1)
{
    TrafficFilterRedirectRule rule;
    rule.priority_ = 100;
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_RANGE);
    rule.dstPort_.range_.startPort_ = 0;
    rule.dstPort_.range_.endPort_ = 65535;
    rule.dstPort_.invert_ = false;
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    SetupIPv4Address(rule.proxyIp_, "192.168.1.100");
    rule.proxyPort_ = 8080;
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildAppendRedirectCommand(rule, chainName);
    EXPECT_TRUE(result.find("--dport") == std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildRedirectCommandWithInvertedPortRange, TestSize.Level1)
{
    TrafficFilterRedirectRule rule;
    rule.priority_ = 100;
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_RANGE);
    rule.dstPort_.range_.startPort_ = 80;
    rule.dstPort_.range_.endPort_ = 443;
    rule.dstPort_.invert_ = true;
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    SetupIPv4Address(rule.proxyIp_, "192.168.1.100");
    rule.proxyPort_ = 8080;
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildAppendRedirectCommand(rule, chainName);
    EXPECT_TRUE(result.find("-m tcp ! --dport 80:443") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildRedirectCommandWithMultiPort, TestSize.Level1)
{
    TrafficFilterRedirectRule rule;
    rule.priority_ = 100;
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_MULTI);
    rule.dstPort_.multi_.portCount_ = 3;
    rule.dstPort_.multi_.ports_[0] = 80;
    rule.dstPort_.multi_.ports_[1] = 443;
    rule.dstPort_.multi_.ports_[2] = 8080;
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    SetupIPv4Address(rule.proxyIp_, "192.168.1.100");
    rule.proxyPort_ = 8080;
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildAppendRedirectCommand(rule, chainName);
    EXPECT_TRUE(result.find("-m multiport --dports 80,443,8080") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildRedirectCommandWithInvertedMultiPort, TestSize.Level1)
{
    TrafficFilterRedirectRule rule;
    rule.priority_ = 100;
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_MULTI);
    rule.srcPort_.multi_.portCount_ = 2;
    rule.srcPort_.multi_.ports_[0] = 8000;
    rule.srcPort_.multi_.ports_[1] = 9000;
    rule.srcPort_.invert_ = true;
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    SetupIPv4Address(rule.proxyIp_, "192.168.1.100");
    rule.proxyPort_ = 8080;
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildAppendRedirectCommand(rule, chainName);
    EXPECT_TRUE(result.find("-m multiport ! --sports 8000,9000") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildRedirectCommandWithInInterface, TestSize.Level1)
{
    TrafficFilterRedirectRule rule;
    rule.priority_ = 100;
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.inInterface_.enabled_ = true;
    rule.inInterface_.ifName_ = "eth0";
    SetupIPv4Address(rule.proxyIp_, "192.168.1.100");
    rule.proxyPort_ = 8080;
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildAppendRedirectCommand(rule, chainName);
    EXPECT_TRUE(result.find("-i eth0") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildRedirectCommandWithInvertedInInterface, TestSize.Level1)
{
    TrafficFilterRedirectRule rule;
    rule.priority_ = 100;
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.inInterface_.enabled_ = true;
    rule.inInterface_.ifName_ = "eth0";
    rule.inInterface_.invert_ = true;
    SetupIPv4Address(rule.proxyIp_, "192.168.1.100");
    rule.proxyPort_ = 8080;
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildAppendRedirectCommand(rule, chainName);
    EXPECT_TRUE(result.find("! -i eth0") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildRedirectCommandWithOutInterface, TestSize.Level1)
{
    TrafficFilterRedirectRule rule;
    rule.priority_ = 100;
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = true;
    rule.outInterface_.ifName_ = "wlan0";
    SetupIPv4Address(rule.proxyIp_, "192.168.1.100");
    rule.proxyPort_ = 8080;
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;

    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildAppendRedirectCommand(rule, chainName);
    EXPECT_TRUE(result.find("-o wlan0") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildRedirectCommandWithInvertedOutInterface, TestSize.Level1)
{
    TrafficFilterRedirectRule rule;
    rule.priority_ = 100;
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = true;
    rule.outInterface_.ifName_ = "wlan0";
    rule.outInterface_.invert_ = true;
    SetupIPv4Address(rule.proxyIp_, "192.168.1.100");
    rule.proxyPort_ = 8080;
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;

    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildAppendRedirectCommand(rule, chainName);
    EXPECT_TRUE(result.find("! -o wlan0") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildRedirectCommandWithPosition, TestSize.Level1)
{
    TrafficFilterRedirectRule rule;
    rule.priority_ = 100;
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    SetupIPv4Address(rule.proxyIp_, "192.168.1.100");
    rule.proxyPort_ = 8080;
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildRedirectCommandWithPosition(rule, chainName, 5);
    EXPECT_TRUE(result.find("-I " + chainName + " 5") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildComplexRedirectRule, TestSize.Level1)
{
    TrafficFilterRedirectRule rule;
    rule.priority_ = 100;
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_CIDR);
    SetupIPv4Address(rule.srcIp_.cidr_.base_, "192.168.1.0");
    rule.srcIp_.cidr_.prefixLen_ = 24;
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_CIDR);
    SetupIPv4Address(rule.dstIp_.cidr_.base_, "10.0.0.0");
    rule.dstIp_.cidr_.prefixLen_ = 8;
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_MULTI);
    rule.dstPort_.multi_.portCount_ = 2;
    rule.dstPort_.multi_.ports_[0] = 80;
    rule.dstPort_.multi_.ports_[1] = 443;
    rule.inInterface_.enabled_ = true;
    rule.inInterface_.ifName_ = "eth0";
    rule.outInterface_.enabled_ = false;
    SetupIPv4Address(rule.proxyIp_, "192.168.1.100");
    rule.proxyPort_ = 8080;
    rule.uidStart_ = 10042;
    rule.uidEnd_ = 10100;

    std::string chainName = "TRF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildAppendRedirectCommand(rule, chainName);
    EXPECT_TRUE(result.find("-p tcp") != std::string::npos);
    EXPECT_TRUE(result.find("-s 192.168.1.0/24") != std::string::npos);
    EXPECT_TRUE(result.find("-d 10.0.0.0/8") != std::string::npos);
    EXPECT_TRUE(result.find("-m multiport --dports 80,443") != std::string::npos);
    EXPECT_TRUE(result.find("-i eth0") != std::string::npos);
    EXPECT_TRUE(result.find("-m owner --uid-owner 10042-10100") != std::string::npos);
    EXPECT_TRUE(result.find("-j DNAT --to-destination 192.168.1.100:8080") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterFlushChainCommand001, TestSize.Level1)
{
    std::string chainName = "PF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildFlushChainCommand(
        chainName, IptablesName::FILTER);
    EXPECT_EQ(result, "-t filter -F " + chainName);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCreateChainCommand001, TestSize.Level1)
{
    std::string chainName = "PF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildCreateChainCommand(
        chainName, IptablesName::FILTER);
    EXPECT_EQ(result, "-t filter -N " + chainName);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterDeleteChainCommand001, TestSize.Level1)
{
    std::string chainName = "PF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildDeleteChainCommand(
        chainName, IptablesName::FILTER);
    EXPECT_EQ(result, "-t filter -X " + chainName);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterDeleteJumpCommand001, TestSize.Level1)
{
    std::string fromHook = "OUTPUT";
    std::string chainName = "PF_com_example_app_1001";
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildDeleteJumpCommand(
        fromHook, chainName, IptablesName::FILTER);
    EXPECT_EQ(result, "-t filter -D " + fromHook + " -j " + chainName);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterDeleteJumpCommandEmptyHook, TestSize.Level1)
{
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildDeleteJumpCommand(
        "", "CHAIN", IptablesName::FILTER);
    EXPECT_EQ(result, "-t filter -D  -j CHAIN");
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterDeleteJumpCommandEmptyChain, TestSize.Level1)
{
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildDeleteJumpCommand(
        "OUTPUT", "", IptablesName::FILTER);
    EXPECT_EQ(result, "-t filter -D OUTPUT -j ");
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandAnyMatch, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.priority_ = 100;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "PF_com_example_app_1001";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_TRUE(result.find("-t filter -A " + chainName) != std::string::npos);
    EXPECT_TRUE(result.find("-p tcp") != std::string::npos);
    EXPECT_TRUE(result.find("-j NFQUEUE --queue-num 5") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandAnyProtocol, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.priority_ = 100;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_ANY;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "PF_com_example_app_1001";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_TRUE(result.find("-p tcp") == std::string::npos);
    EXPECT_TRUE(result.find("-j NFQUEUE --queue-num 5") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandWithUDP, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.priority_ = 100;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_UDP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "PF_com_example_app_1001";
    int32_t queueNum = 3;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_TRUE(result.find("-p 17") != std::string::npos);
    EXPECT_TRUE(result.find("-j NFQUEUE --queue-num 3") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandWithSingleSourceIP, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.priority_ = 100;
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE);
    SetupIPv4Address(rule.srcIp_.single_, "192.168.1.10");
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "PF_com_example_app_1001";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_TRUE(result.find("-s 192.168.1.10") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandWithSingleUID, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.priority_ = 100;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = 10042;
    rule.uidEnd_ = 10042;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "PF_com_example_app_1001";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_TRUE(result.find("-m owner --uid-owner 10042") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandWithUIDRange, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.priority_ = 100;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = 10042;
    rule.uidEnd_ = 10100;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "PF_com_example_app_1001";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_TRUE(result.find("-m owner --uid-owner 10042-10100") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandWithInInterface, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.priority_ = 100;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_INPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = true;
    rule.inInterface_.ifName_ = "eth0";
    rule.outInterface_.enabled_ = false;

    std::string chainName = "PF_com_example_app_1001";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_TRUE(result.find("-i eth0") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandWithOutInterface, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.priority_ = 100;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = true;
    rule.outInterface_.ifName_ = "wlan0";

    std::string chainName = "PF_com_example_app_1001";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_TRUE(result.find("-o wlan0") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandsWithSplitPortsAny, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.priority_ = 100;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "PF_com_example_app_1001";
    int32_t queueNum = 5;
    auto commands = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommands(
        rule, chainName, queueNum);
    EXPECT_EQ(commands.size(), 1);
    EXPECT_TRUE(commands[0].find("-t filter -A " + chainName) != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandsWithSplitPortMulti, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.priority_ = 100;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_MULTI);
    rule.srcPort_.multi_.portCount_ = 3;
    rule.srcPort_.multi_.ports_[0] = 80;
    rule.srcPort_.multi_.ports_[1] = 443;
    rule.srcPort_.multi_.ports_[2] = 8080;
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "PF_com_example_app_1001";
    int32_t queueNum = 5;
    auto commands = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommands(
        rule, chainName, queueNum);
    EXPECT_EQ(commands.size(), 1);
    EXPECT_TRUE(commands[0].find("-m multiport --sports 80,443,8080") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandWithMACMatch, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;
    rule.macMatch_.enable_ = true;
    rule.macMatch_.srcMac_ = "aa:bb:cc:dd:ee:ff";

    std::string chainName = "PF_chain";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_TRUE(result.find("-m mac --mac-source aa:bb:cc:dd:ee:ff") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandWithMACMatchInverted, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;
    rule.macMatch_.enable_ = true;
    rule.macMatch_.invert_ = true;
    rule.macMatch_.srcMac_ = "aa:bb:cc:dd:ee:ff";

    std::string chainName = "PF_chain";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_FALSE(result.find("! -m mac --mac-source aa:bb:cc:dd:ee:ff") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandWithTCPFlags, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_ANY;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;
    rule.tcpFlagsMatch_.enable_ = true;
    rule.tcpFlagsMatch_.flagMask_ = 0x01;
    rule.tcpFlagsMatch_.flagComp_ = 0x01;

    std::string chainName = "PF_chain";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_TRUE(result.find("-p tcp") != std::string::npos);
    EXPECT_TRUE(result.find("--tcp-flags") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandWithTCPFlagsAllSet, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;
    rule.tcpFlagsMatch_.enable_ = true;
    rule.tcpFlagsMatch_.flagMask_ = 0x3F;
    rule.tcpFlagsMatch_.flagComp_ = 0x2F;

    std::string chainName = "PF_chain";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_TRUE(result.find("--tcp-flags SYN,ACK,FIN,RST,PSH,URG") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandWithConntrack, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;
    rule.conntrackMatch_.enable_ = true;
    rule.conntrackMatch_.stateMask_ = 0x03;

    std::string chainName = "PF_chain";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_TRUE(result.find("-m conntrack --ctstate NEW,ESTABLISHED") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandWithConntrackAllStates, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;
    rule.conntrackMatch_.enable_ = true;
    rule.conntrackMatch_.stateMask_ = 0x1F;

    std::string chainName = "PF_chain";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_TRUE(result.find("--ctstate") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandWithCIDRIP, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_CIDR);
    SetupIPv4Address(rule.dstIp_.cidr_.base_, "10.0.0.0");
    rule.dstIp_.cidr_.prefixLen_ = 8;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "PF_chain";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_TRUE(result.find("-d 10.0.0.0/8") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandWithIPRange, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_RANGE);
    SetupIPv4Address(rule.srcIp_.range_.start_, "192.168.1.1");
    SetupIPv4Address(rule.srcIp_.range_.end_, "192.168.1.100");
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "PF_chain";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_TRUE(result.find("-m iprange --src-range") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandWithMultiIP, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_MULTI);
    rule.srcIp_.multi_.ipCount_ = 2;
    SetupIPv4Address(rule.srcIp_.multi_.ips_[0], "192.168.1.10");
    SetupIPv4Address(rule.srcIp_.multi_.ips_[1], "192.168.1.20");
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "PF_chain";
    int32_t queueNum = 5;
    std::vector<std::string> results = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommands(
        rule, chainName, queueNum);
    EXPECT_EQ(results.size(), 3);
    EXPECT_TRUE(results[0].find("-s 192.168.1.10") != std::string::npos);
    EXPECT_TRUE(results[0].find("-j NFQUEUE --queue-num 5") != std::string::npos);
    EXPECT_TRUE(results[1].find("-s 192.168.1.20") != std::string::npos);
    EXPECT_TRUE(results[1].find("-j NFQUEUE --queue-num 5") != std::string::npos);
    EXPECT_TRUE(results[2].find("-j RETURN") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandWithInvertedSrcIP, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE);
    SetupIPv4Address(rule.srcIp_.single_, "192.168.1.10");
    rule.srcIp_.invert_ = true;
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "PF_chain";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_TRUE(result.find("! -s 192.168.1.10") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandWithInputHookAndUid, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_INPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = 10042;
    rule.uidEnd_ = 10042;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "PF_chain";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_TRUE(result.find("-j NFQUEUE") != std::string::npos);
    EXPECT_FALSE(result.find("--uid-owner") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandWithPortRange, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_RANGE);
    rule.dstPort_.range_.startPort_ = 80;
    rule.dstPort_.range_.endPort_ = 443;
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "PF_chain";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_TRUE(result.find("--dport 80:443") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandWithFullPortRange, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_RANGE);
    rule.dstPort_.range_.startPort_ = 0;
    rule.dstPort_.range_.endPort_ = 65535;
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "PF_chain";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_TRUE(result.find("--dport") == std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandWithMultiPortDst, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_MULTI);
    rule.dstPort_.multi_.portCount_ = 3;
    rule.dstPort_.multi_.ports_[0] = 80;
    rule.dstPort_.multi_.ports_[1] = 443;
    rule.dstPort_.multi_.ports_[2] = 8080;
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "PF_chain";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_TRUE(result.find("-m multiport --dports 80,443,8080") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandWithInvertedSrcPort, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_SINGLE);
    rule.srcPort_.single_ = 8080;
    rule.srcPort_.invert_ = true;
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "PF_chain";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_TRUE(result.find("! --sport 8080") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandWithInvertedDstPortRange, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_RANGE);
    rule.dstPort_.range_.startPort_ = 80;
    rule.dstPort_.range_.endPort_ = 443;
    rule.dstPort_.invert_ = true;
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "PF_chain";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_TRUE(result.find("! --dport 80:443") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandWithInvertedMultiPort, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_MULTI);
    rule.srcPort_.multi_.portCount_ = 2;
    rule.srcPort_.multi_.ports_[0] = 8000;
    rule.srcPort_.multi_.ports_[1] = 9000;
    rule.srcPort_.invert_ = true;
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "PF_chain";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_TRUE(result.find("-m multiport ! --sports 8000,9000") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandsInvertedSrcMultiIP, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.priority_ = 100;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_MULTI);
    rule.srcIp_.invert_ = true;
    rule.srcIp_.multi_.ipCount_ = 2;
    SetupIPv4Address(rule.srcIp_.multi_.ips_[0], "192.168.1.10");
    SetupIPv4Address(rule.srcIp_.multi_.ips_[1], "192.168.1.20");
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "PF_chain";
    int32_t queueNum = 5;
    auto commands = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommands(rule, chainName, queueNum);
    EXPECT_EQ(commands.size(), 3);
    EXPECT_TRUE(commands[0].find("-s 192.168.1.10") != std::string::npos);
    EXPECT_TRUE(commands[0].find("-j RETURN") != std::string::npos);
    EXPECT_TRUE(commands[1].find("-s 192.168.1.20") != std::string::npos);
    EXPECT_TRUE(commands[2].find("-j NFQUEUE") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandsBothNonInvertedMultiIP, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.priority_ = 100;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_MULTI);
    rule.srcIp_.multi_.ipCount_ = 2;
    SetupIPv4Address(rule.srcIp_.multi_.ips_[0], "192.168.1.10");
    SetupIPv4Address(rule.srcIp_.multi_.ips_[1], "192.168.1.20");
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_MULTI);
    rule.dstIp_.multi_.ipCount_ = 2;
    SetupIPv4Address(rule.dstIp_.multi_.ips_[0], "10.0.0.1");
    SetupIPv4Address(rule.dstIp_.multi_.ips_[1], "10.0.0.2");
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "PF_chain";
    int32_t queueNum = 5;
    auto commands = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommands(rule, chainName, queueNum);
    EXPECT_EQ(commands.size(), 5);
    EXPECT_TRUE(commands[4].find("-j RETURN") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandMacMatchDisabled, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;
    rule.macMatch_.enable_ = false;

    std::string chainName = "PF_chain";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_FALSE(result.find("-m mac") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandTcpFlagsDisabled, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;
    rule.tcpFlagsMatch_.enable_ = false;

    std::string chainName = "PF_chain";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_FALSE(result.find("--tcp-flags") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandTcpFlagsCompNone, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;
    rule.tcpFlagsMatch_.enable_ = true;
    rule.tcpFlagsMatch_.flagMask_ = 0x01;
    rule.tcpFlagsMatch_.flagComp_ = 0x00;

    std::string chainName = "PF_chain";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_TRUE(result.find("--tcp-flags SYN NONE") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandConntrackDisabled, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;
    rule.conntrackMatch_.enable_ = false;

    std::string chainName = "PF_chain";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_FALSE(result.find("--ctstate") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandAnyProtocolWithTcpFlags, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_ANY;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;
    rule.tcpFlagsMatch_.enable_ = true;
    rule.tcpFlagsMatch_.flagMask_ = 0x01;
    rule.tcpFlagsMatch_.flagComp_ = 0x01;

    std::string chainName = "PF_chain";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_TRUE(result.find("-p tcp") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandNumericProtocol, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_UDP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = UID_UNSPEC;
    rule.uidEnd_ = UID_UNSPEC;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "PF_chain";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_TRUE(result.find("-p 17") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, BuildPacketFilterCommandUidRange, TestSize.Level1)
{
    TrafficFilterPacketRule rule;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = 10042;
    rule.uidEnd_ = 10100;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;

    std::string chainName = "PF_chain";
    int32_t queueNum = 5;
    std::string result = NetTrafficFilterIptablesCommandBuilder::BuildPacketFilterCommand(rule, chainName, queueNum);
    EXPECT_TRUE(result.find("--uid-owner 10042-10100") != std::string::npos);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, ExecuteIptablesCommandEmpty, TestSize.Level1)
{
    int32_t ret = NetTrafficFilterIptablesCommandBuilder::ExecuteIptablesCommand(
        "", TrafficFilterIPFamily::IP_FAMILY_V4);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(NetTrafficFilterIptablesCommandBuilderTest, ExecuteIptablesCommandInvalidFamily, TestSize.Level1)
{
    int32_t ret = NetTrafficFilterIptablesCommandBuilder::ExecuteIptablesCommand(
        "-t filter -L", static_cast<TrafficFilterIPFamily>(999));
    EXPECT_EQ(ret, -1);
}

} // namespace NetManagerStandard
} // namespace OHOS
