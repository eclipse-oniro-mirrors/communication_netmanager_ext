/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include <map>
#include <mutex>
#include <set>
#include <vector>
#include <gtest/gtest.h>
#include "networkslicecommconfig.h"
#include "allowednssaiconfig.h"
#include "networksliceutil.h"
#include "urspconfig.h"
#include "nrunsolicitedmsgparser.h"
#include "networkslicemanager.h"
#include "networkslice_service.h"
#include "networkslice_client.h"
 
namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
const std::string ATTR_TRAFFICDESCRIPTORTYPE = "trafficDescriptorComponentTypeIdentifier";
const std::string ATTR_TRAFFICDESCRIPTORVALUE = "trafficDescriptorComponentValue";
constexpr int TRAFFIC_DESCRIPTOR_COMPONENT_MATCH_ALL = 0x01;
constexpr int TRAFFIC_DESCRIPTOR_COMPONENT_OS_ID_OS_APP_ID = 0x08;
constexpr int TRAFFIC_DESCRIPTOR_COMPONENT_IPV4_ADDR = 0x10;
constexpr int TRAFFIC_DESCRIPTOR_COMPONENT_IPV6_ADDR = 0x21;
constexpr int TRAFFIC_DESCRIPTOR_COMPONENT_PROTOCOL_ID = 0x30;
constexpr int TRAFFIC_DESCRIPTOR_COMPONENT_SINGLE_REMOTE_PORT = 0x50;
constexpr int TRAFFIC_DESCRIPTOR_COMPONENT_REMOTE_PORT_RANGE = 0x51;
constexpr int TRAFFIC_DESCRIPTOR_COMPONENT_DNN = 0x88;
constexpr int TRAFFIC_DESCRIPTOR_COMPONENT_CONNECTION_CAPABILITY = 0x90;
constexpr int TRAFFIC_DESCRIPTOR_COMPONENT_FQDN = 0x91;
constexpr int TRAFFIC_DESCRIPTOR_COMPONENT_OS_APP_ID = 0xA0;
class UrspconfigTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};
 
void UrspconfigTest::SetUpTestCase() {}
 
void UrspconfigTest::TearDownTestCase() {}
 
void UrspconfigTest::SetUp() {}
 
void UrspconfigTest::TearDown() {}
 
constexpr int DECODE_FAIL_UNKNOWN_IDENTIFIER = 1;
constexpr int DECODE_SUCCESS = 0;
constexpr int DECODE_FAIL_OTHER = -1;
 
constexpr int PROTOCOL_PDU_SESSION_TYPE_IPV4 = 1;
constexpr int PROTOCOL_PDU_SESSION_TYPE_IPV6 = 2;
constexpr int PROTOCOL_PDU_SESSION_TYPE_IPV4V6 = 3;
constexpr int PROTOCOL_PDU_SESSION_TYPE_UNSTRUCTURED = 4;
constexpr int PROTOCOL_PDU_SESSION_TYPE_ETHERNET = 5;
constexpr int PROTOCOL_PDU_SESSION_TYPE_RESERVED = 7;
 
constexpr int HAL_PDU_SESSION_TYPE_IP = 0;
constexpr int HAL_PDU_SESSION_TYPE_IPV6 = 1;
constexpr int HAL_PDU_SESSION_TYPE_IPV4V6 = 2;
constexpr int HAL_PDU_SESSION_TYPE_NON_IP = 4;
constexpr int HAL_PDU_SESSION_TYPE_UNSTRUCTURED = 5;
 
HWTEST_F(UrspconfigTest, DecodeUrspRules001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUrspRules001");
    std::vector<uint8_t> buffer = {0x0D, 0x02, 0x00, 0X02, 0x30, 0x16, // ProtocolId
        0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03}; // SscMode
    int inputLen = buffer.size();
    int startIndex = 0;
    UrspRule urspRule;
    std::vector<UrspRule> urspRules;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeUrspRules(inputLen, startIndex, buffer, urspRules), true);
}
 
HWTEST_F(UrspconfigTest, DecodeUrspRules002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUrspRules002");
    std::vector<uint8_t> buffer = {0x00, 0x0E, 0x02, 0x00, 0X02, 0x30, 0x16, // ProtocolId
        0x00, 0x07, 0x00, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03}; // SscMode
    int inputLen = buffer.size();
    int startIndex = 0;
    UrspRule urspRule;
    std::vector<UrspRule> urspRules;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeUrspRules(inputLen, startIndex, buffer, urspRules), true);
}
 
HWTEST_F(UrspconfigTest, DecodeUrspRules003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUrspRules003");
    std::vector<uint8_t> buffer = {0x0E, 0x00, 0x02, 0x02, 0X00, 0x30, 0x16, // ProtocolId
        0x07, 0x00, 0x05, 0x03, 0x02, 0x00, 0x01, 0x03}; // SscMode
    int inputLen = buffer.size();
    int startIndex = 0;
    UrspRule urspRule;
    std::vector<UrspRule> urspRules;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeUrspRules(inputLen, startIndex, buffer, urspRules), false);
}
 
HWTEST_F(UrspconfigTest, DecodeUrspByVersion001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUrspByVersion001");
    std::vector<uint8_t> buffer = {0x0D, 0x02, 0x00, 0X02, 0x30, 0x16, // ProtocolId
        0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03}; // SscMode
    int inputLen = buffer.size();
    int startIndex = 0;
    short version = 1510;
    UrspRule urspRule;
    std::vector<UrspRule> urspRules;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeUrspByVersion(inputLen, startIndex, buffer, urspRules, version), true);
}
 
HWTEST_F(UrspconfigTest, DecodeUrspByVersion002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUrspByVersion002");
    std::vector<uint8_t> buffer = {0x0D, 0x02, 0x00, 0X02, 0x30, 0x16, // ProtocolId
        0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03}; // SscMode
    int inputLen = buffer.size();
    int startIndex = 14;
    short version = 1510;
    UrspRule urspRule;
    std::vector<UrspRule> urspRules;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeUrspByVersion(inputLen, startIndex, buffer, urspRules, version), false);
}
 
HWTEST_F(UrspconfigTest, DecodeUrspByVersion003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUrspByVersion003");
    std::vector<uint8_t> buffer = {0x0D, 0x02, 0x02};
    int inputLen = buffer.size();
    int startIndex = 0;
    short version = 1510;
    UrspRule urspRule;
    std::vector<UrspRule> urspRules;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeUrspByVersion(inputLen, startIndex, buffer, urspRules, version), false);
}
 
HWTEST_F(UrspconfigTest, DecodeUrspByVersion004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUrspByVersion004");
    std::vector<uint8_t> buffer = {0x04, 0x02, 0x02, 0X00, 0x30};
    int inputLen = buffer.size();
    int startIndex = 0;
    short version = 1510;
    UrspRule urspRule;
    std::vector<UrspRule> urspRules;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeUrspByVersion(inputLen, startIndex, buffer, urspRules, version), false);
}
 
HWTEST_F(UrspconfigTest, DecodeUrspRule001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUrspRule001");
    std::vector<uint8_t> buffer = {0x02, 0x00, 0X02, 0x30, 0x16, // ProtocolId
        0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03}; // SscMode
    int inputLen = buffer.size();
    int startIndex = 0;
    short version = 1510;
    UrspRule urspRule;
    std::vector<UrspRule> urspRules;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeUrspRule(inputLen, startIndex, buffer, urspRules, version), true);
}
 
HWTEST_F(UrspconfigTest, DecodeUrspRule002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUrspRule002");
    std::vector<uint8_t> buffer = {0x02};
    int inputLen = buffer.size();
    int startIndex = 0;
    short version = 1510;
    UrspRule urspRule;
    std::vector<UrspRule> urspRules;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeUrspRule(inputLen, startIndex, buffer, urspRules, version), false);
}
 
HWTEST_F(UrspconfigTest, DecodeUrspRuleExtra001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUrspRuleExtra001");
    std::vector<uint8_t> buffer = {0x02, 0x00, 0X02, 0x30, 0x16, // ProtocolId
        0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03}; // SscMode
    int inputLen = buffer.size();
    int startIndex = 1;
    short version = 1510;
    UrspRule urspRule;
    RouteSelectionDescriptor routeRule;
    std::vector<UrspRule> urspRules;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeUrspRuleExtra(
        inputLen, startIndex, buffer, urspRule, urspRules, version), true);
}
 
HWTEST_F(UrspconfigTest, DecodeUrspRuleExtra002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUrspRuleExtra002");
    std::vector<uint8_t> buffer = {0x00, 0X20, 0x30, 0x16,
        0x05, 0x03, 0x02, 0x00, 0x01, 0x03};
    int inputLen = buffer.size();
    int startIndex = 0;
    short version = 1510;
    UrspRule urspRule;
    RouteSelectionDescriptor routeRule;
    std::vector<UrspRule> urspRules;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeUrspRuleExtra(
        inputLen, startIndex, buffer, urspRule, urspRules, version), false);
}
 
HWTEST_F(UrspconfigTest, DecodeUrspRuleExtra003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUrspRuleExtra003");
    std::vector<uint8_t> buffer = {0x02, 0x00, 0X01, 0x30};
    int inputLen = buffer.size();
    int startIndex = 1;
    short version = 1510;
    UrspRule urspRule;
    RouteSelectionDescriptor routeRule;
    std::vector<UrspRule> urspRules;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeUrspRuleExtra(
        inputLen, startIndex, buffer, urspRule, urspRules, version), false);
}
 
HWTEST_F(UrspconfigTest, DecodeUrspRuleExtra004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUrspRuleExtra004");
    std::vector<uint8_t> buffer = {0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x91, 0x05, 0x04, 0x46, 0x71, 0x64, 0x6E};
    int inputLen = buffer.size() + 1;
    int startIndex = 5;
    short version = 1510;
    UrspRule urspRule;
    RouteSelectionDescriptor routeRule;
    std::vector<UrspRule> urspRules;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeUrspRuleExtra(
        inputLen, startIndex, buffer, urspRule, urspRules, version), false);
}
 
HWTEST_F(UrspconfigTest, DecodeUrspRuleExtra005, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUrspRuleExtra005");
    std::vector<uint8_t> buffer = {0x02, 0x00, 0x02, 0x91, 0x05, 0x04, 0x46, 0x71, 0x64, 0x6E}; // Fqdn
    int inputLen = buffer.size();
    int startIndex = 1;
    short version = 1510;
    UrspRule urspRule;
    RouteSelectionDescriptor routeRule;
    std::vector<UrspRule> urspRules;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeUrspRuleExtra(
        inputLen, startIndex, buffer, urspRule, urspRules, version), true);
}
 
HWTEST_F(UrspconfigTest, DecodeUrspRuleExtra006, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUrspRuleExtra006");
    std::vector<uint8_t> buffer = {0x02, 0x00, 0X00};
    int inputLen = buffer.size();
    int startIndex = 1;
    short version = 1510;
    UrspRule urspRule;
    RouteSelectionDescriptor routeRule;
    std::vector<UrspRule> urspRules;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeUrspRuleExtra(
        inputLen, startIndex, buffer, urspRule, urspRules, version), false);
}
 
HWTEST_F(UrspconfigTest, DecodeUrspRuleExtra007, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUrspRuleExtra007");
    std::vector<uint8_t> buffer = {0x00, 0X02, 0x30, 0x16,
        0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03};
    int inputLen = buffer.size();
    int startIndex = 0;
    short version = 1510;
    UrspRule urspRule;
    RouteSelectionDescriptor routeRule;
    std::vector<UrspRule> urspRules;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeUrspRuleExtra(
        inputLen, startIndex, buffer, urspRule, urspRules, version), false);
}
 
HWTEST_F(UrspconfigTest, DecodeUrspRuleExtra008, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUrspRuleExtra008");
    std::vector<uint8_t> buffer = {0x02, 0x00, 0X02, 0x30, 0x16, // ProtocolId
        0x00, 0x07, 0x11, 0x03, 0x00, 0x02, 0x03, 0x01, 0x03}; // SscMode
    int inputLen = buffer.size();
    int startIndex = 1;
    short version = 1510;
    UrspRule urspRule;
    RouteSelectionDescriptor routeRule;
    std::vector<UrspRule> urspRules;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeUrspRuleExtra(
        inputLen, startIndex, buffer, urspRule, urspRules, version), false);
}
 
HWTEST_F(UrspconfigTest, DecodeUrspRuleExtra009, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUrspRuleExtra009");
    std::vector<uint8_t> buffer = {0x02, 0x00, 0X02, 0x30, 0x16, // ProtocolId
        0x00, 0x05, 0x04, 0x03, 0x00, 0x01, 0x00}; // SscMode
    int inputLen = buffer.size();
    int startIndex = 1;
    short version = 1510;
    UrspRule urspRule;
    RouteSelectionDescriptor routeRule;
    std::vector<UrspRule> urspRules;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeUrspRuleExtra(
        inputLen, startIndex, buffer, urspRule, urspRules, version), true);
}
 
HWTEST_F(UrspconfigTest, DecodeTrafficDescriptor001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeTrafficDescriptor001");
    std::vector<uint8_t> buffer = {0x10, 0xC0, 0xA0, 0x01, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, // ipv4
        0x30, 0x16}; // protocolIds
    int inputLen = buffer.size();
    int startIndex = 0;
    TrafficDescriptor trafficDescriptor;
    UrspConfig::GetInstance().DecodeTrafficDescriptor(inputLen, startIndex, buffer, trafficDescriptor);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
    EXPECT_EQ(trafficDescriptor.ipv4Addrs[0].getIpv4Addr(), 3231711491);
    EXPECT_EQ(trafficDescriptor.ipv4Addrs[0].getIpv4Mask(), 4294967295);
    EXPECT_EQ(trafficDescriptor.protocolIds[0], 22);
}
 
HWTEST_F(UrspconfigTest, DecodeTrafficDescriptor002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeTrafficDescriptor002");
    std::vector<uint8_t> buffer = {};
    int inputLen = buffer.size() + 1;
    int startIndex = 0;
    TrafficDescriptor trafficDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeTrafficDescriptor(
        inputLen, startIndex, buffer, trafficDescriptor), DECODE_FAIL_OTHER);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
}
 
HWTEST_F(UrspconfigTest, DecodeTrafficDescriptor003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeTrafficDescriptor003");
    std::vector<uint8_t> buffer = {0x10, 0xC0, 0xA0, 0x01, 0x03};
    int inputLen = buffer.size();
    int startIndex = 0;
    TrafficDescriptor trafficDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeIpv4Addr(startIndex, buffer, trafficDescriptor), DECODE_FAIL_OTHER);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
}
 
HWTEST_F(UrspconfigTest, DecodeTrafficDescriptor004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeTrafficDescriptor004");
    std::vector<uint8_t> buffer = {0x01, 0xC0, 0xA0, 0x01, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, // ipv4
        0x30, 0x16}; // protocolIds
    int inputLen = buffer.size();
    int startIndex = 0;
    TrafficDescriptor trafficDescriptor;
    EXPECT_NE(UrspConfig::GetInstance().DecodeTrafficDescriptor(
        inputLen, startIndex, buffer, trafficDescriptor), DECODE_FAIL_OTHER);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
}
 
HWTEST_F(UrspconfigTest, DecodeTrafficDescriptor005, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeTrafficDescriptor005");
    std::vector<uint8_t> buffer = {0x08, 0xC0, 0xA0, 0x01, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, // ipv4
        0x30, 0x16}; // protocolIds
    int inputLen = buffer.size();
    int startIndex = 0;
    TrafficDescriptor trafficDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeTrafficDescriptor(
        inputLen, startIndex, buffer, trafficDescriptor), DECODE_FAIL_OTHER);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
}
 
HWTEST_F(UrspconfigTest, DecodeTrafficDescriptor006, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeTrafficDescriptor006");
    std::vector<uint8_t> buffer = {0x21, 0xC0, 0xA0, 0x01, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, // ipv4
        0x30, 0x16}; // protocolIds
    int inputLen = buffer.size();
    int startIndex = 0;
    TrafficDescriptor trafficDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeTrafficDescriptor(
        inputLen, startIndex, buffer, trafficDescriptor), DECODE_FAIL_OTHER);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
}
 
HWTEST_F(UrspconfigTest, DecodeOsIdOsAppId001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeOsIdOsAppId001");
    std::vector<uint8_t> buffer = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x08, 0x0E, 0x63, 0x6F, 0x6D, 0x2E, 0x74, 0x65, 0x6E, 0x63, 0x65, 0x6E, 0x74, 0x2E, 0x6D, 0x6D};
    int startIndex = 0;
    TrafficDescriptor trafficDescriptor;
    UrspConfig::GetInstance().DecodeOsIdOsAppId(startIndex, buffer, trafficDescriptor, false);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
    EXPECT_TRUE(trafficDescriptor.osAppIds[0].getOsId() == "01020304050607080102030405060708");
    EXPECT_TRUE(trafficDescriptor.osAppIds[0].getAppId() == "com.tencent.mm");
}
 
HWTEST_F(UrspconfigTest, DecodeOsIdOsAppId002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeOsIdOsAppId002");
    std::vector<uint8_t> buffer = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x07, 0x08, 0x0E, 0x63, 0x6F, 0x6D, 0x2E, 0x74, 0x65, 0x6E, 0x63, 0x65, 0x6E, 0x74, 0x2E, 0x6D, 0x6D};
    int startIndex = 20;
    TrafficDescriptor trafficDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeOsIdOsAppId(
        startIndex, buffer, trafficDescriptor, false), DECODE_FAIL_OTHER);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
}
 
HWTEST_F(UrspconfigTest, DecodeOsIdOsAppId003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeOsIdOsAppId003");
    std::vector<uint8_t> buffer = {0x63, 0x6F, 0x6D, 0x2E, 0x74, 0x65, 0x6E, 0x63, 0x65, 0x6E, 0x74, 0x2E, 0x6D, 0x6D};
    int startIndex = 1;
    TrafficDescriptor trafficDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeOsIdOsAppId(
        startIndex, buffer, trafficDescriptor, true), DECODE_FAIL_OTHER);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
}
 
HWTEST_F(UrspconfigTest, DecodeIpv4Addr001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeIpv4Addr001");
    std::vector<uint8_t> buffer = {0xC0, 0xA0, 0x01, 0x03, 0xFF, 0xFF, 0xFF, 0xFF};
    int startIndex = 0;
    TrafficDescriptor trafficDescriptor;
    UrspConfig::GetInstance().DecodeIpv4Addr(startIndex, buffer, trafficDescriptor);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
    EXPECT_EQ(trafficDescriptor.ipv4Addrs[0].getIpv4Addr(), 3231711491);
    EXPECT_EQ(trafficDescriptor.ipv4Addrs[0].getIpv4Mask(), 4294967295);
}
 
HWTEST_F(UrspconfigTest, DecodeIpv4Addr002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeIpv4Addr002");
    std::vector<uint8_t> buffer = {0xC0, 0xA0, 0x01, 0x03, 0xFF, 0xFF, 0xFF, 0xFF};
    int startIndex = 7;
    TrafficDescriptor trafficDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeIpv4Addr(startIndex, buffer, trafficDescriptor), DECODE_FAIL_OTHER);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
}
 
HWTEST_F(UrspconfigTest, DecodeProtocolId001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeProtocolId001");
    std::vector<uint8_t> buffer = {0x16};
    int startIndex = 0;
    TrafficDescriptor trafficDescriptor;
    UrspConfig::GetInstance().DecodeProtocolId(startIndex, buffer, trafficDescriptor);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
    EXPECT_EQ(trafficDescriptor.protocolIds[0], 22);
}
 
HWTEST_F(UrspconfigTest, DecodeProtocolId002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeProtocolId002");
    std::vector<uint8_t> buffer = {0x16};
    int startIndex = 1;
    TrafficDescriptor trafficDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeProtocolId(startIndex, buffer, trafficDescriptor), DECODE_FAIL_OTHER);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
}
 
HWTEST_F(UrspconfigTest, DecodeIpv6Addr001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeIpv6Addr001");
    std::vector<uint8_t> buffer = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x04};
    std::string ipv6addr;
    int startIndex = 0;
    TrafficDescriptor trafficDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeIpv6Addr(startIndex, buffer, trafficDescriptor), DECODE_SUCCESS);
    EXPECT_EQ(trafficDescriptor.ipv6Addrs[0].getIpv6PrefixLen(), 4);
    EXPECT_EQ(transIpv6AddrToStr(trafficDescriptor.ipv6Addrs[0].getIpv6Addr()),
        "0102:0304:0506:0708:0102:0304:0506:0708");
}
 
HWTEST_F(UrspconfigTest, DecodeIpv6Addr002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeIpv6Addr002");
    std::vector<uint8_t> buffer = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x01, 0x02, 0x03, 0x04,
        0x05, 0x06, 0x07, 0x08, 0x04};
    std::string ipv6addr;
    int startIndex = 1;
    TrafficDescriptor trafficDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeIpv6Addr(startIndex, buffer, trafficDescriptor), DECODE_FAIL_OTHER);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
}
 
HWTEST_F(UrspconfigTest, DecodeTrafficDescriptorExtra001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeTrafficDescriptorExtra001");
    std::vector<uint8_t> buffer = {0x50, 0x16, 0x15}; // SingleRemotePort
    int inputLen = buffer.size();
    int startIndex = 0;
    int initBufferRemaining = buffer.size() - startIndex;
    int type = static_cast<int>(buffer[startIndex++]);
    TrafficDescriptor trafficDescriptor;
    UrspConfig::GetInstance().DecodeTrafficDescriptorExtra(
        inputLen, startIndex, type, buffer, trafficDescriptor, initBufferRemaining);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
    EXPECT_EQ(trafficDescriptor.singleRemotePorts[0], 5653);
}
 
HWTEST_F(UrspconfigTest, DecodeTrafficDescriptorExtra002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeTrafficDescriptorExtra002");
    std::vector<uint8_t> buffer = {0x51, 0x16, 0x15, 0x18, 0x17}; // RemotePortRange
    int inputLen = buffer.size();
    int startIndex = 0;
    int initBufferRemaining = buffer.size() - startIndex;
    int type = static_cast<int>(buffer[startIndex++]);
    TrafficDescriptor trafficDescriptor;
    UrspConfig::GetInstance().DecodeTrafficDescriptorExtra(
        inputLen, startIndex, type, buffer, trafficDescriptor, initBufferRemaining);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
    EXPECT_EQ(trafficDescriptor.remotePortRanges[0].getPortRangeLowLimit(), 5653);
    EXPECT_EQ(trafficDescriptor.remotePortRanges[0].getPortRangeHighLimit(), 6167);
}
 
HWTEST_F(UrspconfigTest, DecodeTrafficDescriptorExtra003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeTrafficDescriptorExtra003");
    std::vector<uint8_t> buffer = {0x88, 0x04, 0x03, 0x64, 0x6E, 0x6E};
    int inputLen = buffer.size();
    int startIndex = 0;
    int initBufferRemaining = buffer.size() - startIndex;
    int type = static_cast<int>(buffer[startIndex++]);
    TrafficDescriptor trafficDescriptor;
    UrspConfig::GetInstance().DecodeTrafficDescriptorExtra(
        inputLen, startIndex, type, buffer, trafficDescriptor, initBufferRemaining);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
    EXPECT_TRUE(trafficDescriptor.dnns[0] == "dnn");
}
 
HWTEST_F(UrspconfigTest, DecodeTrafficDescriptorExtra004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeTrafficDescriptorExtra004");
    std::vector<uint8_t> buffer = {0x91, 0x05, 0x04, 0x46, 0x71, 0x64, 0x6E};
    int inputLen = buffer.size();
    int startIndex = 0;
    int initBufferRemaining = buffer.size() - startIndex;
    int type = static_cast<int>(buffer[startIndex++]);
    TrafficDescriptor trafficDescriptor;
    UrspConfig::GetInstance().SetUrspVersion(NetworkSliceCommConfig::URSP_VERSION_1520);
    EXPECT_EQ(UrspConfig::GetInstance().DecodeTrafficDescriptorExtra(
        inputLen, startIndex, type, buffer, trafficDescriptor, initBufferRemaining), DECODE_SUCCESS);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
    UrspConfig::GetInstance().SetUrspVersion(NetworkSliceCommConfig::URSP_VERSION_1510);
}
 
HWTEST_F(UrspconfigTest, DecodeTrafficDescriptorExtra005, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeTrafficDescriptorExtra005");
    std::vector<uint8_t> buffer = {0x90, 0x04, 0x01, 0x02, 0x03, 0x04};
    int inputLen = buffer.size();
    int startIndex = 0;
    int initBufferRemaining = buffer.size() - startIndex;
    int type = static_cast<int>(buffer[startIndex++]);
    TrafficDescriptor trafficDescriptor;
    UrspConfig::GetInstance().DecodeTrafficDescriptorExtra(
        inputLen, startIndex, type, buffer, trafficDescriptor, initBufferRemaining);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
    EXPECT_EQ(trafficDescriptor.connectionCapabilities[0], 1);
    EXPECT_EQ(trafficDescriptor.connectionCapabilities[1], 2);
    EXPECT_EQ(trafficDescriptor.connectionCapabilities[2], 3);
    EXPECT_EQ(trafficDescriptor.connectionCapabilities[3], 4);
}
 
HWTEST_F(UrspconfigTest, DecodeTrafficDescriptorExtra006, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeTrafficDescriptorExtra006");
    std::vector<uint8_t> buffer = {0xA0, 0x0E, 0x63, 0x6F, 0x6D, 0x2E, 0x74, 0x65, 0x6E, 0x63, 0x65,
        0x6E, 0x74, 0x2E, 0x6D, 0x6D};
    int inputLen = buffer.size();
    int startIndex = 0;
    int initBufferRemaining = buffer.size() - startIndex;
    int type = static_cast<int>(buffer[startIndex++]);
    TrafficDescriptor trafficDescriptor;
    UrspConfig::GetInstance().DecodeTrafficDescriptorExtra(
        inputLen, startIndex, type, buffer, trafficDescriptor, initBufferRemaining);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
    EXPECT_TRUE(trafficDescriptor.osAppIds[0].getAppId() == "com.tencent.mm");
}
 
HWTEST_F(UrspconfigTest, DecodeTrafficDescriptorExtra007, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeTrafficDescriptorExtra007");
    std::vector<uint8_t> buffer = {0x52, 0x15};
    int inputLen = buffer.size();
    int startIndex = 0;
    int initBufferRemaining = buffer.size() - startIndex;
    int type = static_cast<int>(buffer[startIndex++]);
    TrafficDescriptor trafficDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeTrafficDescriptorExtra(
        inputLen, startIndex, type, buffer, trafficDescriptor, initBufferRemaining), DECODE_SUCCESS);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
}
 
HWTEST_F(UrspconfigTest, DecodeSingleRemotePort001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeSingleRemotePort001");
    std::vector<uint8_t> buffer = {0x16, 0x15};
    int startIndex = 0;
    TrafficDescriptor trafficDescriptor;
    UrspConfig::GetInstance().DecodeSingleRemotePort(startIndex, buffer, trafficDescriptor);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
    EXPECT_EQ(trafficDescriptor.singleRemotePorts[0], 5653);
}
 
HWTEST_F(UrspconfigTest, DecodeSingleRemotePort002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeSingleRemotePort002");
    std::vector<uint8_t> buffer = {0x16, 0x15};
    int startIndex = 2;
    TrafficDescriptor trafficDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeSingleRemotePort(
        startIndex, buffer, trafficDescriptor), DECODE_FAIL_OTHER);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
}
 
HWTEST_F(UrspconfigTest, DecodeRemotePortRange001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeRemotePortRange001");
    std::vector<uint8_t> buffer = {0x16, 0x15, 0x18, 0x17};
    int startIndex = 0;
    TrafficDescriptor trafficDescriptor;
    UrspConfig::GetInstance().DecodeRemotePortRange(startIndex, buffer, trafficDescriptor);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
    EXPECT_EQ(trafficDescriptor.remotePortRanges[0].getPortRangeLowLimit(), 5653);
    EXPECT_EQ(trafficDescriptor.remotePortRanges[0].getPortRangeHighLimit(), 6167);
}
 
HWTEST_F(UrspconfigTest, DecodeRemotePortRange002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeRemotePortRange002");
    std::vector<uint8_t> buffer = {0x16, 0x15, 0x18, 0x17};
    int startIndex = 3;
    TrafficDescriptor trafficDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeRemotePortRange(
        startIndex, buffer, trafficDescriptor), DECODE_FAIL_OTHER);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
}
 
HWTEST_F(UrspconfigTest, DecodeTrafficDescriptorDnn001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeTrafficDescriptorDnn001");
    std::vector<uint8_t> buffer = {0x04, 0x03, 0x64, 0x6E, 0x6E};
    int startIndex = 0;
    TrafficDescriptor trafficDescriptor;
    UrspConfig::GetInstance().DecodeTrafficDescriptorDnn(startIndex, buffer, trafficDescriptor);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
    EXPECT_TRUE(trafficDescriptor.dnns[0] == "dnn");
}
 
HWTEST_F(UrspconfigTest, DecodeTrafficDescriptorDnn002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeTrafficDescriptorDnn002");
    std::vector<uint8_t> buffer = {0x03, 0x64, 0x6E, 0x6E};
    int startIndex = 4;
    TrafficDescriptor trafficDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeTrafficDescriptorDnn(
        startIndex, buffer, trafficDescriptor), DECODE_FAIL_OTHER);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
}
 
HWTEST_F(UrspconfigTest, DecodeTrafficDescriptorDnn003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeTrafficDescriptorDnn003");
    std::vector<uint8_t> buffer = {0x03, 0x64, 0x6E, 0x6E};
    int startIndex = 1;
    TrafficDescriptor trafficDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeTrafficDescriptorDnn(
        startIndex, buffer, trafficDescriptor), DECODE_FAIL_OTHER);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
}
 
HWTEST_F(UrspconfigTest, DecodeTrafficDescriptorDnn004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeTrafficDescriptorDnn004");
    std::vector<uint8_t> buffer = {0x03, 0x64, 0x6E, 0x00};
    int startIndex = 3;
    TrafficDescriptor trafficDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeTrafficDescriptorDnn(
        startIndex, buffer, trafficDescriptor), DECODE_FAIL_OTHER);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
}
 
HWTEST_F(UrspconfigTest, DecodeSubDnns001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeSubDnns001");
    std::vector<uint8_t> buffer = {0x03, 0x64, 0x6E, 0x6E};
    int startIndex = 0;
    uint8_t stringLen = 4;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeSubDnns(startIndex, buffer, stringLen), "dnn");
}
 
HWTEST_F(UrspconfigTest, DecodeSubDnns002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeSubDnns002");
    std::vector<uint8_t> buffer = {0x03, 0x64, 0x6E, 0x6E};
    int startIndex = 2;
    uint8_t stringLen = 3;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeSubDnns(startIndex, buffer, stringLen), "");
}
 
HWTEST_F(UrspconfigTest, DecodeFqdn001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeFqdn001");
    std::vector<uint8_t> buffer = {0x05, 0x04, 0x46, 0x71, 0x64, 0x6E};
    int startIndex = 0;
    TrafficDescriptor trafficDescriptor;
    UrspConfig::GetInstance().SetUrspVersion(NetworkSliceCommConfig::URSP_VERSION_1520);
    EXPECT_EQ(UrspConfig::GetInstance().DecodeFqdn(startIndex, buffer, trafficDescriptor), DECODE_SUCCESS);
    UrspConfig::GetInstance().SetUrspVersion(NetworkSliceCommConfig::URSP_VERSION_1510);
}
 
HWTEST_F(UrspconfigTest, DecodeFqdn002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeFqdn002");
    std::vector<uint8_t> buffer = {0x05, 0x04, 0x46, 0x71, 0x64, 0x6E};
    int startIndex = 0;
    TrafficDescriptor trafficDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeFqdn(startIndex, buffer, trafficDescriptor),
        DECODE_FAIL_UNKNOWN_IDENTIFIER);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
}
 
HWTEST_F(UrspconfigTest, DecodeFqdn003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeFqdn003");
    std::vector<uint8_t> buffer = {0x04, 0x46, 0x71, 0x64, 0x6E};
    int startIndex = 5;
    TrafficDescriptor trafficDescriptor;
    UrspConfig::GetInstance().SetUrspVersion(NetworkSliceCommConfig::URSP_VERSION_1520);
    EXPECT_EQ(UrspConfig::GetInstance().DecodeFqdn(startIndex, buffer, trafficDescriptor), DECODE_FAIL_OTHER);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
}
 
HWTEST_F(UrspconfigTest, DecodeFqdn004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeFqdn004");
    std::vector<uint8_t> buffer = {0x06, 0x46, 0x71, 0x64, 0x6E};
    int startIndex = 0;
    TrafficDescriptor trafficDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeFqdn(startIndex, buffer, trafficDescriptor), DECODE_FAIL_OTHER);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
}
 
HWTEST_F(UrspconfigTest, DecodeFqdn005, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeFqdn005");
    std::vector<uint8_t> buffer = {0x05, 0x04, 0x46, 0x71, 0x64, 0x00};
    int startIndex = 5;
    TrafficDescriptor trafficDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeFqdn(startIndex, buffer, trafficDescriptor), DECODE_FAIL_OTHER);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
    UrspConfig::GetInstance().SetUrspVersion(NetworkSliceCommConfig::URSP_VERSION_1510);
}
 
HWTEST_F(UrspconfigTest, DecodeConnectionCapabilities001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeConnectionCapabilities001");
    std::vector<uint8_t> buffer = {0x04, 0x01, 0x02, 0x03, 0x04};
    int startIndex = 0;
    TrafficDescriptor trafficDescriptor;
    UrspConfig::GetInstance().DecodeConnectionCapabilities(startIndex, buffer, trafficDescriptor);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
    EXPECT_EQ(trafficDescriptor.connectionCapabilities[0], 1);
    EXPECT_EQ(trafficDescriptor.connectionCapabilities[1], 2);
    EXPECT_EQ(trafficDescriptor.connectionCapabilities[2], 3);
    EXPECT_EQ(trafficDescriptor.connectionCapabilities[3], 4);
}
 
HWTEST_F(UrspconfigTest, DecodeConnectionCapabilities002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeConnectionCapabilities002");
    std::vector<uint8_t> buffer = {0x04, 0x01, 0x02, 0x03, 0x04};
    int startIndex = 5;
    TrafficDescriptor trafficDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeConnectionCapabilities(
        startIndex, buffer, trafficDescriptor), DECODE_FAIL_OTHER);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
}
 
HWTEST_F(UrspconfigTest, DecodeConnectionCapabilities003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeConnectionCapabilities003");
    std::vector<uint8_t> buffer = {0x04, 0x01, 0x02, 0x03, 0x04};
    int startIndex = 4;
    TrafficDescriptor trafficDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeConnectionCapabilities(
        startIndex, buffer, trafficDescriptor), DECODE_FAIL_OTHER);
    UrspConfig::GetInstance().DumptrafficDescriptor(trafficDescriptor);
}
 
HWTEST_F(UrspconfigTest, DecodeRouteRuleList001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeRouteRuleList001");
    std::vector<uint8_t> buffer = {0x05, 0x03, 0x00, 0x02, 0x01, 0x03}; // SscMode
    int inputLen = buffer.size();
    int startIndex = 0;
    short version = 1510;
    UrspRule urspRule;
    RouteSelectionDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeRouteRuleList(
        inputLen, startIndex, buffer, urspRule, version), DECODE_SUCCESS);
}
 
HWTEST_F(UrspconfigTest, DecodeRouteRuleList002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeRouteRuleList002");
    std::vector<uint8_t> buffer = {0x01, 0x03};
    int inputLen = buffer.size();
    int startIndex = 2;
    short version = 1510;
    UrspRule urspRule;
    RouteSelectionDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeRouteRuleList(
        inputLen, startIndex, buffer, urspRule, version), DECODE_FAIL_OTHER);
}
 
HWTEST_F(UrspconfigTest, DecodeRouteRuleList003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeRouteRuleList003");
    std::vector<uint8_t> buffer = {0x11, 0x03, 0x02, 0x00, 0x03, 0x01, 0x03};
    int inputLen = buffer.size();
    int startIndex = 0;
    short version = 1510;
    UrspRule urspRule;
    RouteSelectionDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeRouteRuleList(
        inputLen, startIndex, buffer, urspRule, version), DECODE_FAIL_OTHER);
}
 
HWTEST_F(UrspconfigTest, DecodeRouteRuleList004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeRouteRuleList004");
    std::vector<uint8_t> buffer = {0x02, 0x02, 0x01};
    int inputLen = buffer.size();
    int startIndex = 0;
    short version = 1510;
    UrspRule urspRule;
    RouteSelectionDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeRouteRuleList(
        inputLen, startIndex, buffer, urspRule, version), DECODE_FAIL_OTHER);
}
 
HWTEST_F(UrspconfigTest, DecodeRouteRuleList005, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeRouteRuleList005");
    std::vector<uint8_t> buffer = {0x03, 0x03, 0x00, 0x02};
    int inputLen = buffer.size();
    int startIndex = 0;
    short version = 1510;
    UrspRule urspRule;
    RouteSelectionDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeRouteRuleList(
        inputLen, startIndex, buffer, urspRule, version), DECODE_FAIL_OTHER);
}
 
HWTEST_F(UrspconfigTest, DecodeRouteRuleList006, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeRouteRuleList006");
    std::vector<uint8_t> buffer = {0x04, 0x03, 0x00, 0x01, 0x02};
    int inputLen = buffer.size();
    int startIndex = 0;
    short version = 1510;
    UrspRule urspRule;
    RouteSelectionDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeRouteRuleList(
        inputLen, startIndex, buffer, urspRule, version), DECODE_FAIL_OTHER);
}
 
HWTEST_F(UrspconfigTest, DecodeRouteRuleList007, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeRouteRuleList007");
    std::vector<uint8_t> buffer = {0x04, 0x03, 0x00, 0x01, 0x00};
    int inputLen = buffer.size();
    int startIndex = 0;
    short version = 1510;
    UrspRule urspRule;
    RouteSelectionDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeRouteRuleList(inputLen, startIndex, buffer, urspRule, version),
        DECODE_FAIL_UNKNOWN_IDENTIFIER);
}
 
HWTEST_F(UrspconfigTest, GetSubLenByversion001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetSubLenByversion001");
    std::vector<uint8_t> buffer = {0x01, 0x03};
    int startIndex = 0;
    short version = 1510;
    EXPECT_EQ(UrspConfig::GetInstance().GetSubLenByversion(startIndex, buffer, version), 1);
}
 
HWTEST_F(UrspconfigTest, GetSubLenByversion002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetSubLenByversion002");
    std::vector<uint8_t> buffer = {0x01, 0x03};
    int startIndex = 2;
    short version = 1510;
    EXPECT_EQ(UrspConfig::GetInstance().GetSubLenByversion(startIndex, buffer, version), DECODE_FAIL_OTHER);
}
 
HWTEST_F(UrspconfigTest, GetSubLenByversion003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetSubLenByversion003");
    std::vector<uint8_t> buffer = {0x01, 0x03};
    int startIndex = 2;
    short version = 1520;
    EXPECT_EQ(UrspConfig::GetInstance().GetSubLenByversion(startIndex, buffer, version), DECODE_FAIL_OTHER);
}
 
HWTEST_F(UrspconfigTest, GetSubLenByversion004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetSubLenByversion004");
    std::vector<uint8_t> buffer = {0x01, 0x03};
    int startIndex = 0;
    short version = 1599;
    EXPECT_EQ(UrspConfig::GetInstance().GetSubLenByversion(startIndex, buffer, version), DECODE_FAIL_OTHER);
}
 
HWTEST_F(UrspconfigTest, GetLenBytesByversion001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetLenBytesByversion001");
    short version = 1510;
    EXPECT_EQ(UrspConfig::GetInstance().GetLenBytesByversion(version), NetworkSliceCommConfig::LEN_BYTE);
}
 
HWTEST_F(UrspconfigTest, GetLenBytesByversion002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetLenBytesByversion002");
    short version = 1520;
    EXPECT_EQ(UrspConfig::GetInstance().GetLenBytesByversion(version), NetworkSliceCommConfig::LEN_SHORT);
}
 
HWTEST_F(UrspconfigTest, DecodeRouteRule001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeRouteRule001");
    std::vector<uint8_t> buffer = {0x01, 0x03};
    int startIndex = 0;
    int inputLen = buffer.size();
    RouteSelectionDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeRouteRule(startIndex, inputLen, buffer, routeRule), DECODE_SUCCESS);
}
 
HWTEST_F(UrspconfigTest, DecodeRouteRule002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeRouteRule002");
    std::vector<uint8_t> buffer = {0x01};
    int startIndex = 1;
    int inputLen = buffer.size();
    RouteSelectionDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeRouteRule(startIndex, inputLen, buffer, routeRule), DECODE_FAIL_OTHER);
}
 
HWTEST_F(UrspconfigTest, DecodeRouteRule003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeRouteRule003");
    std::vector<uint8_t> buffer = {0x00};
    int startIndex = 0;
    int inputLen = buffer.size();
    RouteSelectionDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeRouteRule(
        startIndex, inputLen, buffer, routeRule), DECODE_FAIL_UNKNOWN_IDENTIFIER);
}
 
HWTEST_F(UrspconfigTest, DecodeRouteRule004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeRouteRule004");
    std::vector<uint8_t> buffer = {0x04, 0x03};
    int startIndex = 0;
    int inputLen = buffer.size();
    RouteSelectionDescriptor routeRule;
    EXPECT_NE(UrspConfig::GetInstance().DecodeRouteRule(startIndex, inputLen, buffer, routeRule), DECODE_SUCCESS);
}
 
HWTEST_F(UrspconfigTest, DecodeRouteRule005, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeRouteRule005");
    std::vector<uint8_t> buffer = {0x08, 0x03};
    int startIndex = 0;
    int inputLen = buffer.size();
    RouteSelectionDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeRouteRule(startIndex, inputLen, buffer, routeRule), DECODE_SUCCESS);
}
 
HWTEST_F(UrspconfigTest, DecodeRouteRule006, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeRouteRule006");
    std::vector<uint8_t> buffer = {0x10, 0x03};
    int startIndex = 0;
    int inputLen = buffer.size();
    RouteSelectionDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeRouteRule(startIndex, inputLen, buffer, routeRule), DECODE_SUCCESS);
}
 
HWTEST_F(UrspconfigTest, DecodeRouteRule007, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeRouteRule007");
    std::vector<uint8_t> buffer = {0x10, 0x10};
    int startIndex = 0;
    int inputLen = buffer.size();
    RouteSelectionDescriptor routeRule;
    EXPECT_NE(UrspConfig::GetInstance().DecodeRouteRule(startIndex, inputLen, buffer, routeRule), DECODE_SUCCESS);
}
 
HWTEST_F(UrspconfigTest, DecodeRouteRule008, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeRouteRule008");
    std::vector<uint8_t> buffer = {0x10, 0x10, 0x01, 0x01};
    int startIndex = 0;
    int inputLen = buffer.size();
    RouteSelectionDescriptor routeRule;
    EXPECT_NE(UrspConfig::GetInstance().DecodeRouteRule(startIndex, inputLen, buffer, routeRule), DECODE_SUCCESS);
}
 
HWTEST_F(UrspconfigTest, DecodeRouteRule009, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeRouteRule009");
    std::vector<uint8_t> buffer = {0x11, 0x03, 0x01, 0x01};
    int startIndex = 0;
    int inputLen = buffer.size();
    RouteSelectionDescriptor routeRule;
    EXPECT_NE(UrspConfig::GetInstance().DecodeRouteRule(startIndex, inputLen, buffer, routeRule), DECODE_SUCCESS);
}
 
HWTEST_F(UrspconfigTest, DecodeSscMode001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeSscMode001");
    std::vector<uint8_t> buffer = {0x03};
    int startIndex = 0;
    RouteSelectionDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeSscMode(startIndex, buffer, routeRule), DECODE_SUCCESS);
}
 
HWTEST_F(UrspconfigTest, DecodeSscMode002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeSscMode002");
    std::vector<uint8_t> buffer = {};
    int startIndex = 0;
    RouteSelectionDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeSscMode(startIndex, buffer, routeRule), DECODE_FAIL_OTHER);
}
 
HWTEST_F(UrspconfigTest, DecodeSnssai001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeSnssai001");
    std::vector<uint8_t> buffer = {0x01, 0x03};
    int startIndex = 0;
    RouteSelectionDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeSnssai(startIndex, buffer, routeRule), DECODE_SUCCESS);
}
 
HWTEST_F(UrspconfigTest, DecodeSnssai002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeSnssai002");
    std::vector<uint8_t> buffer = {};
    int startIndex = 0;
    RouteSelectionDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeSnssai(startIndex, buffer, routeRule), DECODE_FAIL_OTHER);
}
 
HWTEST_F(UrspconfigTest, DecodeSnssai003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeSnssai003");
    std::vector<uint8_t> buffer = {0x01, 0x03};
    int startIndex = 1;
    RouteSelectionDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeSnssai(startIndex, buffer, routeRule), DECODE_FAIL_OTHER);
}
 
HWTEST_F(UrspconfigTest, DecodeSnssai004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeSnssai004");
    std::vector<uint8_t> buffer = {0x03, 0x03, 0x00, 0x00};
    int startIndex = 0;
    RouteSelectionDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeSnssai(startIndex, buffer, routeRule), DECODE_FAIL_OTHER);
}
 
HWTEST_F(UrspconfigTest, DecodeDnn001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeDnn001");
    std::vector<uint8_t> buffer = {0x04, 0x03, 0x64, 0x6E, 0x6E};
    int startIndex = 0;
    RouteSelectionDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeDnn(startIndex, buffer, routeRule), DECODE_SUCCESS);
}
 
HWTEST_F(UrspconfigTest, DecodeDnn002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeDnn002");
    std::vector<uint8_t> buffer = {0x03, 0x01, 0x02, 0x03, 0x04};
    int startIndex = 5;
    RouteSelectionDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeDnn(startIndex, buffer, routeRule), DECODE_FAIL_OTHER);
}
 
HWTEST_F(UrspconfigTest, DecodeDnn003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeDnn003");
    std::vector<uint8_t> buffer = {0x06, 0x01, 0x02, 0x03, 0x04};
    int startIndex = 0;
    RouteSelectionDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeDnn(startIndex, buffer, routeRule), DECODE_FAIL_OTHER);
}

HWTEST_F(UrspconfigTest, DecodeDnn004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeDnn004");
    std::vector<uint8_t> buffer = {0x04, 0x01, 0x02, 0x03, 0x00};
    int startIndex = 4;
    RouteSelectionDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().DecodeDnn(startIndex, buffer, routeRule), DECODE_FAIL_OTHER);
}
 
HWTEST_F(UrspconfigTest, DecodePduSessionType001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodePduSessionType001");
    std::vector<uint8_t> buffer = {0x03};
    int startIndex = 0;
    RouteSelectionDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().DecodePduSessionType(startIndex, buffer, routeRule), DECODE_SUCCESS);
}
 
HWTEST_F(UrspconfigTest, DecodePduSessionType002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodePduSessionType002");
    std::vector<uint8_t> buffer = {};
    int startIndex = 0;
    RouteSelectionDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().DecodePduSessionType(startIndex, buffer, routeRule), DECODE_FAIL_OTHER);
}
 
HWTEST_F(UrspconfigTest, TransferPduSessionTypeToHal001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("TransferPduSessionTypeToHal001");
    int pduSessionType = PROTOCOL_PDU_SESSION_TYPE_IPV4;
    EXPECT_EQ(UrspConfig::GetInstance().TransferPduSessionTypeToHal(pduSessionType), HAL_PDU_SESSION_TYPE_IP);
}
 
HWTEST_F(UrspconfigTest, TransferPduSessionTypeToHal002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("TransferPduSessionTypeToHal002");
    int pduSessionType = PROTOCOL_PDU_SESSION_TYPE_IPV6;
    EXPECT_EQ(UrspConfig::GetInstance().TransferPduSessionTypeToHal(pduSessionType), HAL_PDU_SESSION_TYPE_IPV6);
}
 
HWTEST_F(UrspconfigTest, TransferPduSessionTypeToHal003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("TransferPduSessionTypeToHal003");
    int pduSessionType = PROTOCOL_PDU_SESSION_TYPE_IPV4V6;
    EXPECT_EQ(UrspConfig::GetInstance().TransferPduSessionTypeToHal(pduSessionType), HAL_PDU_SESSION_TYPE_IPV4V6);
}
 
HWTEST_F(UrspconfigTest, TransferPduSessionTypeToHal004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("TransferPduSessionTypeToHal004");
    int pduSessionType = PROTOCOL_PDU_SESSION_TYPE_UNSTRUCTURED;
    EXPECT_EQ(UrspConfig::GetInstance().TransferPduSessionTypeToHal(pduSessionType), HAL_PDU_SESSION_TYPE_UNSTRUCTURED);
}
 
HWTEST_F(UrspconfigTest, TransferPduSessionTypeToHal005, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("TransferPduSessionTypeToHal005");
    int pduSessionType = PROTOCOL_PDU_SESSION_TYPE_ETHERNET;
    EXPECT_EQ(UrspConfig::GetInstance().TransferPduSessionTypeToHal(pduSessionType), HAL_PDU_SESSION_TYPE_NON_IP);
}
 
HWTEST_F(UrspconfigTest, TransferPduSessionTypeToHal006, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("TransferPduSessionTypeToHal006");
    int pduSessionType = PROTOCOL_PDU_SESSION_TYPE_RESERVED;
    EXPECT_EQ(UrspConfig::GetInstance().TransferPduSessionTypeToHal(pduSessionType), HAL_PDU_SESSION_TYPE_IPV4V6);
}
 
HWTEST_F(UrspconfigTest, TransferPduSessionTypeToHal007, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("TransferPduSessionTypeToHal007");
    int pduSessionType = 6;
    EXPECT_EQ(UrspConfig::GetInstance().TransferPduSessionTypeToHal(pduSessionType), HAL_PDU_SESSION_TYPE_IPV4V6);
}
 
HWTEST_F(UrspconfigTest, DecodePreferredAccessType001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodePreferredAccessType001");
    std::vector<uint8_t> buffer = {0x03};
    int startIndex = 0;
    RouteSelectionDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().DecodePreferredAccessType(startIndex, buffer, routeRule), DECODE_SUCCESS);
}
 
HWTEST_F(UrspconfigTest, DecodePreferredAccessType002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodePreferredAccessType002");
    std::vector<uint8_t> buffer = {};
    int startIndex = 0;
    RouteSelectionDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().DecodePreferredAccessType(startIndex, buffer, routeRule), DECODE_FAIL_OTHER);
}
 
HWTEST_F(UrspconfigTest, DecodePreferredAccessType003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodePreferredAccessType003");
    std::vector<uint8_t> buffer = {0x10};
    int startIndex = 0;
    RouteSelectionDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().DecodePreferredAccessType(
        startIndex, buffer, routeRule), DECODE_FAIL_UNKNOWN_IDENTIFIER);
}
 
HWTEST_F(UrspconfigTest, GetImsRsdList001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetImsRsdList001");
    UrspConfig::GetInstance().GetImsRsdList();
}
 
HWTEST_F(UrspconfigTest, ConvertRsdList2BufferArray001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("ConvertRsdList2BufferArray001");
    std::unordered_map<int, std::vector<RouteSelectionDescriptor>> rsdsMap;
    UrspConfig::GetInstance().ConvertRsdList2BufferArray(rsdsMap);
}
 
HWTEST_F(UrspconfigTest, ConvertRsdList2BufferArray002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("ConvertRsdList2BufferArray002");
    std::unordered_map<int, std::vector<RouteSelectionDescriptor>> rsdsMap;
    RouteSelectionDescriptor map1;
    map1.routePrecedence = 1;
    rsdsMap[1].push_back(map1);
    UrspConfig::GetInstance().ConvertRsdList2BufferArray(rsdsMap);
}
 
HWTEST_F(UrspconfigTest, PutRsdListInfo001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("PutRsdListInfo001");
    std::vector<uint8_t> buffer;
    std::vector<RouteSelectionDescriptor> rsdList;
    UrspConfig::GetInstance().PutRsdListInfo(buffer, rsdList);
}
 
HWTEST_F(UrspconfigTest, PutDnnsInfo001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("PutDnnsInfo001");
    std::vector<uint8_t> buffer;
    RouteSelectionDescriptor rsd;
    UrspConfig::GetInstance().PutDnnsInfo(buffer, rsd);
}
 
HWTEST_F(UrspconfigTest, PutDnnsInfo002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("PutDnnsInfo002");
    std::vector<uint8_t> buffer;
    RouteSelectionDescriptor rsd;
    rsd.dnns = {"01", "02", "03"};
    UrspConfig::GetInstance().PutDnnsInfo(buffer, rsd);
}
 
HWTEST_F(UrspconfigTest, PutNssaisInfo001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("PutNssaisInfo001");
    std::vector<uint8_t> buffer;
    RouteSelectionDescriptor rsd;
    UrspConfig::GetInstance().PutNssaisInfo(buffer, rsd);
}
 
HWTEST_F(UrspconfigTest, PutNssaisInfo002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("PutNssaisInfo002");
    std::vector<uint8_t> buffer;
    RouteSelectionDescriptor rsd;
    Snssai snssai1;
    snssai1.setSd(1);
    rsd.snssais.push_back(snssai1);
    UrspConfig::GetInstance().PutNssaisInfo(buffer, rsd);
}
 
HWTEST_F(UrspconfigTest, SetBitOpt001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("SetBitOpt001");
    int num = 0;
    int position = 0;
    UrspConfig::GetInstance().SetBitOpt(num, position);
}
 
HWTEST_F(UrspconfigTest, NotifyImsaDelRsdInfo001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("NotifyImsaDelRsdInfo001");
    UrspConfig::GetInstance().NotifyImsaDelRsdInfo();
}
 
HWTEST_F(UrspconfigTest, FindAvailableRouteRule001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("FindAvailableRouteRule001");
    std::vector<RouteSelectionDescriptor> routeSelectionDescriptors;
    SelectedRouteDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().FindAvailableRouteRule(routeSelectionDescriptors, routeRule), false);
}
 
HWTEST_F(UrspconfigTest, FindAvailableRouteRule002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("FindAvailableRouteRule002");
    std::vector<RouteSelectionDescriptor> routeSelectionDescriptors;
    RouteSelectionDescriptor routeSelectionDescriptor;
    Snssai snssai;
    snssai.setSnssaiLen(4);
    snssai.setSnssai("1234");
    routeSelectionDescriptor.snssais.push_back(snssai);
    routeSelectionDescriptor.dnns = {"dnn"};
    routeSelectionDescriptors.push_back(routeSelectionDescriptor);
    SelectedRouteDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().FindAvailableRouteRule(routeSelectionDescriptors, routeRule), false);
}
 
HWTEST_F(UrspconfigTest, FindAvailableRouteRule003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("FindAvailableRouteRule003");
    std::vector<RouteSelectionDescriptor> routeSelectionDescriptors;
    RouteSelectionDescriptor routeSelectionDescriptor;
    Snssai snssai;
    snssai.setSnssaiLen(4);
    snssai.setSnssai("1234");
    routeSelectionDescriptor.snssais.push_back(snssai);
    routeSelectionDescriptors.push_back(routeSelectionDescriptor);
    SelectedRouteDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().FindAvailableRouteRule(routeSelectionDescriptors, routeRule), false);
}
 
HWTEST_F(UrspconfigTest, FindAvailableRouteRule004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("FindAvailableRouteRule004");
    std::vector<RouteSelectionDescriptor> routeSelectionDescriptors;
    RouteSelectionDescriptor routeSelectionDescriptor;
    routeSelectionDescriptor.dnns = {"dnn"};
    routeSelectionDescriptors.push_back(routeSelectionDescriptor);
    SelectedRouteDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().FindAvailableRouteRule(routeSelectionDescriptors, routeRule), true);
}
 
HWTEST_F(UrspconfigTest, FindAvailableSnssai002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("FindAvailableSnssai002");
    RouteSelectionDescriptor routeSelectionDescriptor;
    SelectedRouteDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().FindAvailableSnssai(routeSelectionDescriptor, routeRule), false);
}
 
HWTEST_F(UrspconfigTest, FindAvailableDnn001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("FindAvailableDnn001");
    RouteSelectionDescriptor routeSelectionDescriptor;
    routeSelectionDescriptor.dnns = {"dnn"};
    SelectedRouteDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().FindAvailableDnn(routeSelectionDescriptor, routeRule), true);
}
 
HWTEST_F(UrspconfigTest, FindAvailableDnn002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("FindAvailableDnn002");
    RouteSelectionDescriptor routeSelectionDescriptor;
    SelectedRouteDescriptor routeRule;
    EXPECT_EQ(UrspConfig::GetInstance().FindAvailableDnn(routeSelectionDescriptor, routeRule), false);
}
 
HWTEST_F(UrspconfigTest, FillTrafficDescriptor001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("FillTrafficDescriptor001");
    TrafficDescriptor urspTrafficDescriptor;
    urspTrafficDescriptor.isMatchAll = true;
    AppDescriptor appDescriptor;
    SelectedRouteDescriptor routeRule;
    UrspConfig::GetInstance().FillTrafficDescriptor(urspTrafficDescriptor, appDescriptor, routeRule);
    EXPECT_EQ(routeRule.getRouteBitmap(), (1));
}
 
HWTEST_F(UrspconfigTest, FillTrafficDescriptor002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("FillTrafficDescriptor002");
    TrafficDescriptor urspTrafficDescriptor;
    urspTrafficDescriptor.isMatchAll = false;
    OsAppId OsAppIds;
    OsAppIds.setOsId("01020304050607080102030405060708");
    OsAppIds.setAppId("com.tencent.mm");
    std::vector<OsAppId> osAppIds;
    osAppIds.push_back(OsAppIds);
    std::string osId = "01020304050607080102030405060708";
    std::string appId = "com.tencent.mm";
    Ipv4Addr ipv4;
    ipv4.setIpv4Addr(3231711491);
    ipv4.setIpv4Mask(4294967295);
    std::vector<Ipv4Addr> ipv4Addrs;
    ipv4Addrs.push_back(ipv4);
    Ipv6Addr ipv6;
    ipv6.setIpv6Addr({0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08});
    ipv6.setIpv6PrefixLen(0x04);
    std::vector<Ipv6Addr> ipv6Addrs;
    ipv6Addrs.push_back(ipv6);
    AppDescriptor appDescriptor;
    urspTrafficDescriptor.osAppIds = osAppIds;
    urspTrafficDescriptor.protocolIds.push_back(22);
    urspTrafficDescriptor.dnns.push_back("dnn");
    urspTrafficDescriptor.fqdns.push_back("fqdn");
    urspTrafficDescriptor.connectionCapabilities.push_back(1234);
    appDescriptor.setOsAppId(osId, appId);
    appDescriptor.mIpv4Addr = 3231711491;
    appDescriptor.mIpv6Addr = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    SelectedRouteDescriptor routeRule;
    UrspConfig::GetInstance().FillTrafficDescriptor(urspTrafficDescriptor, appDescriptor, routeRule);
    EXPECT_EQ(routeRule.getRouteBitmap(), (14));
}
 
HWTEST_F(UrspconfigTest, FillIpv4Addrs001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("FillIpv4Addrs001");
    TrafficDescriptor urspTrafficDescriptor;
    SelectedRouteDescriptor routeRule;
    UrspConfig::GetInstance().FillIpv4Addrs(urspTrafficDescriptor, routeRule);
}
 
HWTEST_F(UrspconfigTest, FillIpv6Addrs001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("FillIpv6Addrs001");
    TrafficDescriptor urspTrafficDescriptor;
    SelectedRouteDescriptor routeRule;
    UrspConfig::GetInstance().FillIpv6Addrs(urspTrafficDescriptor, routeRule);
}
 
HWTEST_F(UrspconfigTest, FillRemotePorts001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("FillRemotePorts001");
    RemotePortRange RemotePortRanges;
    RemotePortRanges.setPortRangeLowLimit(5653);
    RemotePortRanges.setPortRangeHighLimit(6167);
    std::vector<RemotePortRange> remotePortRanges;
    remotePortRanges.push_back(RemotePortRanges);
    TrafficDescriptor urspTrafficDescriptor;
    urspTrafficDescriptor.remotePortRanges = remotePortRanges;
    AppDescriptor appDescriptor;
    appDescriptor.mRemotePort = 5654;
    SelectedRouteDescriptor routeRule;
    UrspConfig::GetInstance().FillRemotePorts(urspTrafficDescriptor, routeRule);
}
 
HWTEST_F(UrspconfigTest, isTrafficDescriptorMatch001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isTrafficDescriptorMatch001");
    TrafficDescriptor urspTrafficDescriptor;
    urspTrafficDescriptor.isMatchAll = true;
    AppDescriptor appDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().isTrafficDescriptorMatch(urspTrafficDescriptor, appDescriptor), true);
}
 
HWTEST_F(UrspconfigTest, isTrafficDescriptorMatch002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isTrafficDescriptorMatch002");
    OsAppId OsAppIds;
    OsAppIds.setOsId("01020304050607080102030405060708");
    OsAppIds.setAppId("com.tencent.mm");
    std::vector<OsAppId> osAppIds;
    osAppIds.push_back(OsAppIds);
    TrafficDescriptor urspTrafficDescriptor;
    urspTrafficDescriptor.osAppIds = osAppIds;
    AppDescriptor appDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().isTrafficDescriptorMatch(urspTrafficDescriptor, appDescriptor), false);
}
 
HWTEST_F(UrspconfigTest, isTrafficDescriptorMatch003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isTrafficDescriptorMatch003");
    Ipv4Addr ipv4;
    ipv4.setIpv4Addr(3231711491);
    ipv4.setIpv4Mask(4294967295);
    std::vector<Ipv4Addr> ipv4Addrs;
    ipv4Addrs.push_back(ipv4);
    TrafficDescriptor urspTrafficDescriptor;
    urspTrafficDescriptor.ipv4Addrs = ipv4Addrs;
    AppDescriptor appDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().isTrafficDescriptorMatch(urspTrafficDescriptor, appDescriptor), false);
}
 
HWTEST_F(UrspconfigTest, isTrafficDescriptorMatch004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isTrafficDescriptorMatch004");
    Ipv6Addr ipv6;
    ipv6.setIpv6Addr({0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08});
    ipv6.setIpv6PrefixLen(0x04);
    std::vector<Ipv6Addr> ipv6Addrs;
    ipv6Addrs.push_back(ipv6);
    TrafficDescriptor urspTrafficDescriptor;
    urspTrafficDescriptor.ipv6Addrs = ipv6Addrs;
    AppDescriptor appDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().isTrafficDescriptorMatch(urspTrafficDescriptor, appDescriptor), false);
}
 
HWTEST_F(UrspconfigTest, isTrafficDescriptorMatch005, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isTrafficDescriptorMatch005");
    TrafficDescriptor urspTrafficDescriptor;
    urspTrafficDescriptor.protocolIds.push_back(22);
    AppDescriptor appDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().isTrafficDescriptorMatch(urspTrafficDescriptor, appDescriptor), false);
}
 
HWTEST_F(UrspconfigTest, isTrafficDescriptorMatch006, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isTrafficDescriptorMatch006");
    TrafficDescriptor urspTrafficDescriptor;
    urspTrafficDescriptor.dnns.push_back("dnn");
    AppDescriptor appDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().isTrafficDescriptorMatch(urspTrafficDescriptor, appDescriptor), false);
}
 
HWTEST_F(UrspconfigTest, isTrafficDescriptorMatch007, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isTrafficDescriptorMatch007");
    TrafficDescriptor urspTrafficDescriptor;
    urspTrafficDescriptor.fqdns.push_back("fqdn");
    AppDescriptor appDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().isTrafficDescriptorMatch(urspTrafficDescriptor, appDescriptor), false);
}
 
HWTEST_F(UrspconfigTest, isTrafficDescriptorMatch008, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isTrafficDescriptorMatch008");
    TrafficDescriptor urspTrafficDescriptor;
    urspTrafficDescriptor.connectionCapabilities.push_back(1234);
    AppDescriptor appDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().isTrafficDescriptorMatch(urspTrafficDescriptor, appDescriptor), false);
}
 
HWTEST_F(UrspconfigTest, isTrafficDescriptorMatch009, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isTrafficDescriptorMatch009");
    RemotePortRange RemotePortRanges;
    RemotePortRanges.setPortRangeLowLimit(5653);
    RemotePortRanges.setPortRangeHighLimit(6167);
    std::vector<RemotePortRange> remotePortRanges;
    remotePortRanges.push_back(RemotePortRanges);
    TrafficDescriptor urspTrafficDescriptor;
    urspTrafficDescriptor.remotePortRanges = remotePortRanges;
    AppDescriptor appDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().isTrafficDescriptorMatch(urspTrafficDescriptor, appDescriptor), false);
}
 
HWTEST_F(UrspconfigTest, isTrafficDescriptorMatch010, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isTrafficDescriptorMatch010");
    TrafficDescriptor urspTrafficDescriptor;
    AppDescriptor appDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().isTrafficDescriptorMatch(urspTrafficDescriptor, appDescriptor), true);
}

HWTEST_F(UrspconfigTest, isIpThreeTuplesInWhiteList002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isIpThreeTuplesInWhiteList002");
    std::string plmn;
    AppDescriptor appDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().isIpThreeTuplesInWhiteList(plmn, appDescriptor), false);
}
 
HWTEST_F(UrspconfigTest, isIpThreeTuplesInTrafficDescriptor001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isIpThreeTuplesInTrafficDescriptor001");
    Ipv4Addr ipv4;
    ipv4.setIpv4Addr(3231711491);
    ipv4.setIpv4Mask(4294967295);
    std::vector<Ipv4Addr> ipv4Addrs;
    ipv4Addrs.push_back(ipv4);
    TrafficDescriptor urspTrafficDescriptor;
    urspTrafficDescriptor.ipv4Addrs = ipv4Addrs;
    AppDescriptor appDescriptor;
    appDescriptor.mIpv4Addr = 3231711491;
    EXPECT_EQ(UrspConfig::GetInstance().isIpThreeTuplesInTrafficDescriptor(urspTrafficDescriptor, appDescriptor), true);
}
 
HWTEST_F(UrspconfigTest, isIpThreeTuplesInTrafficDescriptor002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isIpThreeTuplesInTrafficDescriptor002");
    TrafficDescriptor urspTrafficDescriptor;
    AppDescriptor appDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().isIpThreeTuplesInTrafficDescriptor(
        urspTrafficDescriptor, appDescriptor), false);
}
 
HWTEST_F(UrspconfigTest, isIpThreeTuplesInTrafficDescriptor003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isIpThreeTuplesInTrafficDescriptor003");
    Ipv6Addr ipv6;
    ipv6.setIpv6Addr({0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08});
    ipv6.setIpv6PrefixLen(0x04);
    std::vector<Ipv6Addr> ipv6Addrs;
    ipv6Addrs.push_back(ipv6);
    TrafficDescriptor urspTrafficDescriptor;
    urspTrafficDescriptor.ipv6Addrs = ipv6Addrs;
    AppDescriptor appDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().isIpThreeTuplesInTrafficDescriptor(
        urspTrafficDescriptor, appDescriptor), false);
}
 
HWTEST_F(UrspconfigTest, isIpThreeTuplesInTrafficDescriptor004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isIpThreeTuplesInTrafficDescriptor004");
    Ipv4Addr ipv4;
    ipv4.setIpv4Addr(3231711491);
    ipv4.setIpv4Mask(4294967295);
    std::vector<Ipv4Addr> ipv4Addrs;
    ipv4Addrs.push_back(ipv4);
    TrafficDescriptor urspTrafficDescriptor;
    urspTrafficDescriptor.ipv4Addrs = ipv4Addrs;
    AppDescriptor appDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().isIpThreeTuplesInTrafficDescriptor(
        urspTrafficDescriptor, appDescriptor), false);
}
 
HWTEST_F(UrspconfigTest, isIpThreeTuplesInTrafficDescriptor005, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isIpThreeTuplesInTrafficDescriptor005");
    TrafficDescriptor urspTrafficDescriptor;
    urspTrafficDescriptor.protocolIds.push_back(22);
    AppDescriptor appDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().isIpThreeTuplesInTrafficDescriptor(
        urspTrafficDescriptor, appDescriptor), false);
}
 
HWTEST_F(UrspconfigTest, isIpThreeTuplesInTrafficDescriptor006, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isIpThreeTuplesInTrafficDescriptor006");
    RemotePortRange RemotePortRanges;
    RemotePortRanges.setPortRangeLowLimit(5653);
    RemotePortRanges.setPortRangeHighLimit(6167);
    std::vector<RemotePortRange> remotePortRanges;
    remotePortRanges.push_back(RemotePortRanges);
    TrafficDescriptor urspTrafficDescriptor;
    urspTrafficDescriptor.remotePortRanges = remotePortRanges;
    AppDescriptor appDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().isIpThreeTuplesInTrafficDescriptor(
        urspTrafficDescriptor, appDescriptor), false);
}
 
HWTEST_F(UrspconfigTest, isOsAppIdMatch001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isOsAppIdMatch001");
    TrafficDescriptor urspTrafficDescriptor;
    AppDescriptor appDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().isOsAppIdMatch(urspTrafficDescriptor, appDescriptor), true);
}
 
HWTEST_F(UrspconfigTest, isOsAppIdMatch002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isOsAppIdMatch002");
    OsAppId OsAppIds;
    OsAppIds.setOsId("01020304050607080102030405060708");
    OsAppIds.setAppId("com.tencent.mm");
    std::vector<OsAppId> osAppIds;
    osAppIds.push_back(OsAppIds);
    TrafficDescriptor urspTrafficDescriptor;
    urspTrafficDescriptor.osAppIds = osAppIds;
    AppDescriptor appDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().isOsAppIdMatch(urspTrafficDescriptor, appDescriptor), false);
}
 
HWTEST_F(UrspconfigTest, isOsAppIdMatch003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isOsAppIdMatch003");
    OsAppId OsAppIds;
    OsAppIds.setOsId("01020304050607080102030405060708");
    OsAppIds.setAppId("com.tencent.mm");
    std::vector<OsAppId> osAppIds;
    osAppIds.push_back(OsAppIds);
    std::string osId = "01020304050607080102030405060708";
    std::string appId = "com.tencent.mm";
    TrafficDescriptor urspTrafficDescriptor;
    urspTrafficDescriptor.osAppIds = osAppIds;
    AppDescriptor appDescriptor;
    appDescriptor.setOsAppId(osId, appId);
    EXPECT_EQ(UrspConfig::GetInstance().isOsAppIdMatch(urspTrafficDescriptor, appDescriptor), true);
}
 
HWTEST_F(UrspconfigTest, isRemotePortMatch001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isRemotePortMatch001");
    TrafficDescriptor urspTrafficDescriptor;
    AppDescriptor appDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().isRemotePortMatch(urspTrafficDescriptor, appDescriptor), true);
}
 
HWTEST_F(UrspconfigTest, isRemotePortMatch002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isRemotePortMatch002");
    RemotePortRange RemotePortRanges;
    RemotePortRanges.setPortRangeLowLimit(5653);
    RemotePortRanges.setPortRangeHighLimit(6167);
    std::vector<RemotePortRange> remotePortRanges;
    remotePortRanges.push_back(RemotePortRanges);
    TrafficDescriptor urspTrafficDescriptor;
    urspTrafficDescriptor.remotePortRanges = remotePortRanges;
    AppDescriptor appDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().isRemotePortMatch(urspTrafficDescriptor, appDescriptor), false);
}
 
HWTEST_F(UrspconfigTest, isRemotePortMatch003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isRemotePortMatch003");
    RemotePortRange RemotePortRanges;
    RemotePortRanges.setPortRangeLowLimit(5653);
    RemotePortRanges.setPortRangeHighLimit(6167);
    std::vector<RemotePortRange> remotePortRanges;
    remotePortRanges.push_back(RemotePortRanges);
    TrafficDescriptor urspTrafficDescriptor;
    urspTrafficDescriptor.remotePortRanges = remotePortRanges;
    AppDescriptor appDescriptor;
    appDescriptor.mRemotePort = 5654;
    EXPECT_EQ(UrspConfig::GetInstance().isRemotePortMatch(urspTrafficDescriptor, appDescriptor), true);
}
 
HWTEST_F(UrspconfigTest, isIpv4AddrMatch001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isIpv4AddrMatch001");
    TrafficDescriptor urspTrafficDescriptor;
    AppDescriptor appDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().isIpv4AddrMatch(urspTrafficDescriptor, appDescriptor), true);
}
 
HWTEST_F(UrspconfigTest, isIpv4AddrMatch002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isIpv4AddrMatch002");
    Ipv4Addr ipv4;
    ipv4.setIpv4Addr(3231711491);
    ipv4.setIpv4Mask(4294967295);
    std::vector<Ipv4Addr> ipv4Addrs;
    ipv4Addrs.push_back(ipv4);
    TrafficDescriptor urspTrafficDescriptor;
    urspTrafficDescriptor.ipv4Addrs = ipv4Addrs;
    AppDescriptor appDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().isIpv4AddrMatch(urspTrafficDescriptor, appDescriptor), false);
}
 
HWTEST_F(UrspconfigTest, isIpv4AddrMatch003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isIpv4AddrMatch003");
    Ipv4Addr ipv4;
    ipv4.setIpv4Addr(3231711491);
    ipv4.setIpv4Mask(4294967295);
    std::vector<Ipv4Addr> ipv4Addrs;
    ipv4Addrs.push_back(ipv4);
    TrafficDescriptor urspTrafficDescriptor;
    urspTrafficDescriptor.ipv4Addrs = ipv4Addrs;
    AppDescriptor appDescriptor;
    appDescriptor.mIpv4Addr = 3231711491;
    EXPECT_EQ(UrspConfig::GetInstance().isIpv4AddrMatch(urspTrafficDescriptor, appDescriptor), true);
}
 
HWTEST_F(UrspconfigTest, isIpv6AddrMatch001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isIpv6AddrMatch001");
    TrafficDescriptor urspTrafficDescriptor;
    AppDescriptor appDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().isIpv6AddrMatch(urspTrafficDescriptor, appDescriptor), true);
}
 
HWTEST_F(UrspconfigTest, isIpv6AddrMatch002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isIpv6AddrMatch002");
    Ipv6Addr ipv6;
    ipv6.setIpv6Addr({0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08});
    ipv6.setIpv6PrefixLen(0x04);
    std::vector<Ipv6Addr> ipv6Addrs;
    ipv6Addrs.push_back(ipv6);
    TrafficDescriptor urspTrafficDescriptor;
    urspTrafficDescriptor.ipv6Addrs = ipv6Addrs;
    AppDescriptor appDescriptor;
    EXPECT_EQ(UrspConfig::GetInstance().isIpv6AddrMatch(urspTrafficDescriptor, appDescriptor), false);
}
 
HWTEST_F(UrspconfigTest, isIpv6AddrMatch003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isIpv6AddrMatch003");
    Ipv6Addr ipv6;
    ipv6.setIpv6Addr({0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08});
    ipv6.setIpv6PrefixLen(0x04);
 
    std::vector<Ipv6Addr> ipv6Addrs;
    ipv6Addrs.push_back(ipv6);
 
    TrafficDescriptor urspTrafficDescriptor;
    urspTrafficDescriptor.ipv6Addrs = ipv6Addrs;
 
    AppDescriptor appDescriptor;
    appDescriptor.mIpv6Addr = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x07, 0x08};
    EXPECT_EQ(UrspConfig::GetInstance().isIpv6AddrMatch(urspTrafficDescriptor, appDescriptor), true);
}
 
HWTEST_F(UrspconfigTest, DumpUePolicyMap001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DumpUePolicyMap001");
    UrspConfig::GetInstance().DumpUePolicyMap();
}

HWTEST_F(UrspconfigTest, ParseTrafficDescriptor001, testing::ext::TestSize.Level1)
{
    TrafficDescriptor trafficDescriptor;
    xmlNodePtr trafficDescriptorNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar *>("trafficDescriptor"));
    xmlSetProp(trafficDescriptorNode, reinterpret_cast<const xmlChar *>(ATTR_TRAFFICDESCRIPTORTYPE.c_str()),
        reinterpret_cast<const xmlChar *>(std::to_string(TRAFFIC_DESCRIPTOR_COMPONENT_OS_ID_OS_APP_ID).c_str()));
    xmlSetProp(trafficDescriptorNode, reinterpret_cast<const xmlChar *>(ATTR_TRAFFICDESCRIPTORVALUE.c_str()),
        reinterpret_cast<const xmlChar *>("1,2"));
    UrspConfig::GetInstance().ParseTrafficDescriptor(trafficDescriptorNode, trafficDescriptor);
    EXPECT_EQ(trafficDescriptor.osAppIds[0].getOsId(), "1");
    EXPECT_EQ(trafficDescriptor.osAppIds[0].getAppId(), "2");
}

HWTEST_F(UrspconfigTest, ParseTrafficDescriptor002, testing::ext::TestSize.Level1)
{
    TrafficDescriptor trafficDescriptor;
    xmlNodePtr trafficDescriptorNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar *>("trafficDescriptor"));
    xmlSetProp(trafficDescriptorNode, reinterpret_cast<const xmlChar *>(ATTR_TRAFFICDESCRIPTORTYPE.c_str()),
        reinterpret_cast<const xmlChar *>(std::to_string(TRAFFIC_DESCRIPTOR_COMPONENT_IPV4_ADDR).c_str()));
    xmlSetProp(trafficDescriptorNode, reinterpret_cast<const xmlChar *>(ATTR_TRAFFICDESCRIPTORVALUE.c_str()),
        reinterpret_cast<const xmlChar *>("1.1.1.1,1.1.1.1"));
    UrspConfig::GetInstance().ParseTrafficDescriptor(trafficDescriptorNode, trafficDescriptor);
    EXPECT_EQ(trafficDescriptor.ipv4Addrs[0].getIpv4Addr(), 16843009);
    EXPECT_EQ(trafficDescriptor.ipv4Addrs[0].getIpv4Mask(), 16843009);
}

HWTEST_F(UrspconfigTest, ParseTrafficDescriptor003, testing::ext::TestSize.Level1)
{
    TrafficDescriptor trafficDescriptor;
    xmlNodePtr trafficDescriptorNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar *>("trafficDescriptor"));
    xmlSetProp(trafficDescriptorNode, reinterpret_cast<const xmlChar *>(ATTR_TRAFFICDESCRIPTORTYPE.c_str()),
        reinterpret_cast<const xmlChar *>(std::to_string(TRAFFIC_DESCRIPTOR_COMPONENT_IPV6_ADDR).c_str()));
    xmlSetProp(trafficDescriptorNode, reinterpret_cast<const xmlChar *>(ATTR_TRAFFICDESCRIPTORVALUE.c_str()),
        reinterpret_cast<const xmlChar *>("2001:0db8:85a3::8a2e:0370:7334,64"));
    UrspConfig::GetInstance().ParseTrafficDescriptor(trafficDescriptorNode, trafficDescriptor);
    EXPECT_EQ(trafficDescriptor.ipv6Addrs[0].getIpv6PrefixLen(), 64);
}

HWTEST_F(UrspconfigTest, ParseTrafficDescriptor004, testing::ext::TestSize.Level1)
{
    TrafficDescriptor trafficDescriptor;
    xmlNodePtr trafficDescriptorNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar *>("trafficDescriptor"));
    xmlSetProp(trafficDescriptorNode, reinterpret_cast<const xmlChar *>(ATTR_TRAFFICDESCRIPTORTYPE.c_str()),
        reinterpret_cast<const xmlChar *>(std::to_string(TRAFFIC_DESCRIPTOR_COMPONENT_PROTOCOL_ID).c_str()));
    xmlSetProp(trafficDescriptorNode, reinterpret_cast<const xmlChar *>(ATTR_TRAFFICDESCRIPTORVALUE.c_str()),
        reinterpret_cast<const xmlChar *>("2"));
    UrspConfig::GetInstance().ParseTrafficDescriptor(trafficDescriptorNode, trafficDescriptor);
    EXPECT_EQ(trafficDescriptor.protocolIds[0], 2);
}

HWTEST_F(UrspconfigTest, ParseTrafficDescriptor005, testing::ext::TestSize.Level1)
{
    TrafficDescriptor trafficDescriptor;
    xmlNodePtr trafficDescriptorNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar *>("trafficDescriptor"));
    xmlSetProp(trafficDescriptorNode, reinterpret_cast<const xmlChar *>(ATTR_TRAFFICDESCRIPTORTYPE.c_str()),
        reinterpret_cast<const xmlChar *>(std::to_string(TRAFFIC_DESCRIPTOR_COMPONENT_SINGLE_REMOTE_PORT).c_str()));
    xmlSetProp(trafficDescriptorNode, reinterpret_cast<const xmlChar *>(ATTR_TRAFFICDESCRIPTORVALUE.c_str()),
        reinterpret_cast<const xmlChar *>("2"));
    UrspConfig::GetInstance().ParseTrafficDescriptor(trafficDescriptorNode, trafficDescriptor);
    EXPECT_EQ(trafficDescriptor.singleRemotePorts[0], 2);
}

HWTEST_F(UrspconfigTest, ParseTrafficDescriptor006, testing::ext::TestSize.Level1)
{
    TrafficDescriptor trafficDescriptor;
    xmlNodePtr trafficDescriptorNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar *>("trafficDescriptor"));
    xmlSetProp(trafficDescriptorNode, reinterpret_cast<const xmlChar *>(ATTR_TRAFFICDESCRIPTORTYPE.c_str()),
        reinterpret_cast<const xmlChar *>(std::to_string(TRAFFIC_DESCRIPTOR_COMPONENT_REMOTE_PORT_RANGE).c_str()));
    xmlSetProp(trafficDescriptorNode, reinterpret_cast<const xmlChar *>(ATTR_TRAFFICDESCRIPTORVALUE.c_str()),
        reinterpret_cast<const xmlChar *>("2,4"));
    UrspConfig::GetInstance().ParseTrafficDescriptor(trafficDescriptorNode, trafficDescriptor);
    EXPECT_EQ(trafficDescriptor.remotePortRanges[0].getPortRangeLowLimit(), 2);
    EXPECT_EQ(trafficDescriptor.remotePortRanges[0].getPortRangeHighLimit(), 4);
}

HWTEST_F(UrspconfigTest, ParseTrafficDescriptor007, testing::ext::TestSize.Level1)
{
    TrafficDescriptor trafficDescriptor;
    xmlNodePtr trafficDescriptorNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar *>("trafficDescriptor"));
    xmlSetProp(trafficDescriptorNode, reinterpret_cast<const xmlChar *>(ATTR_TRAFFICDESCRIPTORTYPE.c_str()),
        reinterpret_cast<const xmlChar *>(std::to_string(TRAFFIC_DESCRIPTOR_COMPONENT_DNN).c_str()));
    xmlSetProp(trafficDescriptorNode, reinterpret_cast<const xmlChar *>(ATTR_TRAFFICDESCRIPTORVALUE.c_str()),
        reinterpret_cast<const xmlChar *>("ims"));
    UrspConfig::GetInstance().ParseTrafficDescriptor(trafficDescriptorNode, trafficDescriptor);
    EXPECT_EQ(trafficDescriptor.dnns[0], "ims");
}

HWTEST_F(UrspconfigTest, ParseTrafficDescriptor008, testing::ext::TestSize.Level1)
{
    TrafficDescriptor trafficDescriptor;
    xmlNodePtr trafficDescriptorNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar *>("trafficDescriptor"));
    xmlSetProp(trafficDescriptorNode, reinterpret_cast<const xmlChar *>(ATTR_TRAFFICDESCRIPTORTYPE.c_str()),
        reinterpret_cast<const xmlChar *>(std::to_string(TRAFFIC_DESCRIPTOR_COMPONENT_FQDN).c_str()));
    xmlSetProp(trafficDescriptorNode, reinterpret_cast<const xmlChar *>(ATTR_TRAFFICDESCRIPTORVALUE.c_str()),
        reinterpret_cast<const xmlChar *>("fqdn"));
    UrspConfig::GetInstance().ParseTrafficDescriptor(trafficDescriptorNode, trafficDescriptor);
    EXPECT_EQ(trafficDescriptor.fqdns[0], "fqdn");
}

HWTEST_F(UrspconfigTest, ParseTrafficDescriptor009, testing::ext::TestSize.Level1)
{
    TrafficDescriptor trafficDescriptor;
    xmlNodePtr trafficDescriptorNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar *>("trafficDescriptor"));
    xmlSetProp(trafficDescriptorNode, reinterpret_cast<const xmlChar *>(ATTR_TRAFFICDESCRIPTORTYPE.c_str()),
        reinterpret_cast<const xmlChar *>(std::to_string(TRAFFIC_DESCRIPTOR_COMPONENT_CONNECTION_CAPABILITY).c_str()));
    xmlSetProp(trafficDescriptorNode, reinterpret_cast<const xmlChar *>(ATTR_TRAFFICDESCRIPTORVALUE.c_str()),
        reinterpret_cast<const xmlChar *>("1"));
    UrspConfig::GetInstance().ParseTrafficDescriptor(trafficDescriptorNode, trafficDescriptor);
    EXPECT_EQ(trafficDescriptor.connectionCapabilities[0], 1);
}

HWTEST_F(UrspconfigTest, ParseTrafficDescriptor010, testing::ext::TestSize.Level1)
{
    TrafficDescriptor trafficDescriptor;
    xmlNodePtr trafficDescriptorNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar *>("trafficDescriptor"));
    xmlSetProp(trafficDescriptorNode, reinterpret_cast<const xmlChar *>(ATTR_TRAFFICDESCRIPTORTYPE.c_str()),
        reinterpret_cast<const xmlChar *>("3"));
    UrspConfig::GetInstance().ParseTrafficDescriptor(trafficDescriptorNode, trafficDescriptor);
}
 
HWTEST_F(UrspconfigTest, FillTrafficDescriptorWhiteList001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<TrafficDescriptorWhiteList> whiteList = std::make_shared<TrafficDescriptorWhiteList>();
    OsAppId osAppid;
    osAppid.setOsId("1234");
    osAppid.setAppId("app1");
    UrspRule urspRule;
    urspRule.trafficDescriptor.osAppIds.push_back(osAppid);
    urspRule.trafficDescriptor.dnns = { "dn1", "dn2" };
    urspRule.trafficDescriptor.fqdns = { "fqdn1", "fqdn2" };
    urspRule.trafficDescriptor.connectionCapabilities = { 1, 2 };
    std::vector<UrspRule> urspRules;
    urspRules.push_back(urspRule);
    UrspConfig::GetInstance().FillTrafficDescriptorWhiteList(whiteList, urspRules);
    EXPECT_EQ(whiteList->osAppIds, "1234#app1");
    EXPECT_EQ(whiteList->dnns, "dn1,dn2");
    EXPECT_EQ(whiteList->fqdns, "fqdn1,fqdn2");
    EXPECT_EQ(whiteList->cct, "1,2");
}
} // NetManagerStandard
} // OHOS