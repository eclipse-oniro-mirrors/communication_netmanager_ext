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

#include <gtest/gtest.h>
#include <arpa/inet.h>

#define private public
#define protected public

#include "nettrafficfilter_redirect_manager.h"
#include "nettrafficfilter_iptables_command_builder.h"
#include "net_manager_constants.h"

#define private public
#define protected public

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;

constexpr uint32_t TEST_GROUP_ID = 1001;
constexpr uint32_t TEST_PRIORITY = 100;
constexpr uint32_t TEST_UID = 1000;
constexpr uint32_t TEST_UID_END = 1050;
constexpr int32_t TEST_PID = 1000;
constexpr uint16_t TEST_PROXY_PORT = 8080;
constexpr uint32_t INVALID_GROUP_ID = 0;
constexpr uint32_t INVALID_PRIORITY_LOW = 0;
constexpr uint32_t INVALID_PRIORITY_HIGH = 10001;
constexpr uint32_t TEST_PACKET_LEN = 65535;
constexpr uint32_t TEST_NFQUEUE_LEN = 1024;
constexpr int32_t ADDR_BIT1 = 1;
constexpr int32_t ADDR_BIT2 = 2;
constexpr int32_t ADDR_BIT3 = 3;
constexpr uint8_t ADDR1 = 127;

TrafficFilterRedirectRule CreateTestRule(uint32_t priority = TEST_PRIORITY)
{
    TrafficFilterRedirectRule rule;
    rule.priority_ = priority;
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_PREROUTING);
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;

    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);

    rule.uidStart_ = static_cast<uint32_t>(-1);
    rule.uidEnd_ = static_cast<uint32_t>(-1);

    rule.proxyIp_.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4);
    rule.proxyIp_.addr_[0] = ADDR1;
    rule.proxyIp_.addr_[ADDR_BIT1] = 0;
    rule.proxyIp_.addr_[ADDR_BIT2] = 0;
    rule.proxyIp_.addr_[ADDR_BIT3] = 1;
    rule.proxyPort_ = TEST_PROXY_PORT;

    return rule;
}

TrafficFilterRedirectRule CreateTestRuleWithUidMatch(uint32_t priority = TEST_PRIORITY)
{
    TrafficFilterRedirectRule rule = CreateTestRule(priority);
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    rule.uidStart_ = TEST_UID;
    rule.uidEnd_ = TEST_UID_END;
    return rule;
}
} // namespace

class NetTrafficFilterRedirectManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static inline NetTrafficFilterRedirectManager* instance_ = nullptr;
};

void NetTrafficFilterRedirectManagerTest::SetUpTestCase()
{
    instance_ = &NetTrafficFilterRedirectManager::GetInstance();
}

void NetTrafficFilterRedirectManagerTest::TearDownTestCase()
{
    instance_ = nullptr;
}

void NetTrafficFilterRedirectManagerTest::SetUp() {}

void NetTrafficFilterRedirectManagerTest::TearDown() {}

HWTEST_F(NetTrafficFilterRedirectManagerTest, CreateRedirector001, TestSize.Level1)
{
    std::string bundleName = "";
    uint32_t groupId = TEST_GROUP_ID;
    uint32_t priority = TEST_PRIORITY;
    std::string redirectorId;

    int32_t ret = instance_->CreateRedirector(bundleName, groupId, priority, redirectorId);
    EXPECT_EQ(ret, TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, CreateRedirector002, TestSize.Level1)
{
    std::string bundleName = "com.example.test";
    uint32_t groupId = INVALID_GROUP_ID;
    uint32_t priority = TEST_PRIORITY;
    std::string redirectorId;

    int32_t ret = instance_->CreateRedirector(bundleName, groupId, priority, redirectorId);
    EXPECT_EQ(ret, TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, CreateRedirector003, TestSize.Level1)
{
    std::string bundleName = "com.example.test";
    uint32_t groupId = TEST_GROUP_ID;
    uint32_t priority = INVALID_PRIORITY_LOW;
    std::string redirectorId;

    int32_t ret = instance_->CreateRedirector(bundleName, groupId, priority, redirectorId);
    EXPECT_EQ(ret, TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, CreateRedirector004, TestSize.Level1)
{
    std::string bundleName = "com.example.test";
    uint32_t groupId = TEST_GROUP_ID;
    uint32_t priority = INVALID_PRIORITY_HIGH;
    std::string redirectorId;

    int32_t ret = instance_->CreateRedirector(bundleName, groupId, priority, redirectorId);
    EXPECT_EQ(ret, TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, CreateRedirector005, TestSize.Level1)
{
    std::string bundleName = "com.example.test";
    uint32_t groupId = TEST_GROUP_ID;
    uint32_t priority = TEST_PRIORITY;
    std::string redirectorId;

    int32_t ret = instance_->CreateRedirector(bundleName, groupId, priority, redirectorId);
    EXPECT_EQ(ret, TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, CreateRedirector006, TestSize.Level1)
{
    std::string bundleName = "com.example.test";
    uint32_t groupId = TEST_GROUP_ID;
    uint32_t priority = TEST_PRIORITY;
    std::string redirectorId;

    int32_t ret = instance_->CreateRedirector(bundleName, groupId, priority, redirectorId);
    EXPECT_EQ(ret, TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, CreateRedirector007, TestSize.Level1)
{
    std::string bundleName = "com.example.test";
    uint32_t groupId = TEST_GROUP_ID;
    uint32_t priority = TEST_PRIORITY;
    std::string redirectorId;

    int32_t ret = instance_->CreateRedirector(bundleName, groupId, priority, redirectorId);
    EXPECT_EQ(ret, TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, CreateRedirector008, TestSize.Level1)
{
    std::string bundleName(256, 'a'); // Exceeds 255 char limit
    uint32_t groupId = TEST_GROUP_ID;
    uint32_t priority = TEST_PRIORITY;
    std::string redirectorId;

    int32_t ret = instance_->CreateRedirector(bundleName, groupId, priority, redirectorId);
    EXPECT_EQ(ret, TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, DestroyRedirector001, TestSize.Level1)
{
    std::string redirectorId = "non_existent_id";
    int32_t ret = instance_->DestroyRedirector(redirectorId);
    EXPECT_EQ(ret, TRAFFICFILTER_ERROR_NOT_FOUND);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, DestroyRedirectorsByBundleName001, TestSize.Level1)
{
    std::string bundleName = "com.example.test";

    int32_t ret = instance_->DestroyRedirectorsByBundleName(bundleName);
    EXPECT_EQ(ret, -1); // No redirectors found
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, AddRedirectRule001, TestSize.Level1)
{
    std::string redirectorId = "non_existent_id";
    TrafficFilterRedirectRule rule = CreateTestRule();

    int32_t addRet = instance_->AddRedirectRule(redirectorId, &rule);
    EXPECT_EQ(addRet, TRAFFICFILTER_ERROR_NOT_FOUND);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, AddRedirectRule002, TestSize.Level1)
{
    std::string bundleName = "com.example.test";
    uint32_t groupId = TEST_GROUP_ID;
    uint32_t priority = TEST_PRIORITY;
    std::string redirectorId;

    int32_t createRet = instance_->CreateRedirector(bundleName, groupId, priority, redirectorId);
    int32_t addRet = instance_->AddRedirectRule(redirectorId, nullptr);
    EXPECT_EQ(addRet, TRAFFICFILTER_ERROR_INVALID_PARAM);

    // Cleanup
    instance_->DestroyRedirector(redirectorId);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, AddRedirectRule003, TestSize.Level1)
{
    std::string bundleName = "com.example.test";
    uint32_t groupId = TEST_GROUP_ID;
    uint32_t priority = TEST_PRIORITY;
    std::string redirectorId;

    int32_t createRet = instance_->CreateRedirector(bundleName, groupId, priority, redirectorId);
    TrafficFilterRedirectRule rule = CreateTestRule();
    rule.priority_ = INVALID_PRIORITY_LOW;

    int32_t addRet = instance_->AddRedirectRule(redirectorId, &rule);
    EXPECT_EQ(addRet, TRAFFICFILTER_ERROR_INVALID_PARAM);

    // Cleanup
    instance_->DestroyRedirector(redirectorId);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, AddRedirectRule004, TestSize.Level1)
{
    std::string bundleName = "com.example.test";
    uint32_t groupId = TEST_GROUP_ID;
    uint32_t priority = TEST_PRIORITY;
    std::string redirectorId;

    int32_t createRet = instance_->CreateRedirector(bundleName, groupId, priority, redirectorId);
    TrafficFilterRedirectRule rule = CreateTestRule();
    rule.protocol_ = 17; // UDP - invalid for current implementation

    int32_t addRet = instance_->AddRedirectRule(redirectorId, &rule);
    EXPECT_EQ(addRet, TRAFFICFILTER_ERROR_INVALID_PARAM);

    // Cleanup
    instance_->DestroyRedirector(redirectorId);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, AddRedirectRule005, TestSize.Level1)
{
    std::string bundleName = "com.example.test";
    uint32_t groupId = TEST_GROUP_ID;
    uint32_t priority = TEST_PRIORITY;
    std::string redirectorId;

    int32_t createRet = instance_->CreateRedirector(bundleName, groupId, priority, redirectorId);
    TrafficFilterRedirectRule rule = CreateTestRule();
    rule.hookPoint_ = 99; // Invalid hook point

    int32_t addRet = instance_->AddRedirectRule(redirectorId, &rule);
    EXPECT_EQ(addRet, TRAFFICFILTER_ERROR_INVALID_PARAM);

    // Cleanup
    instance_->DestroyRedirector(redirectorId);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, AddRedirectRule006, TestSize.Level1)
{
    std::string bundleName = "com.example.test";
    uint32_t groupId = TEST_GROUP_ID;
    uint32_t priority = TEST_PRIORITY;
    std::string redirectorId;

    int32_t createRet = instance_->CreateRedirector(bundleName, groupId, priority, redirectorId);
    TrafficFilterRedirectRule rule = CreateTestRule();
    rule.proxyPort_ = 0; // Invalid proxy port

    int32_t addRet = instance_->AddRedirectRule(redirectorId, &rule);
    EXPECT_EQ(addRet, TRAFFICFILTER_ERROR_INVALID_PARAM);

    // Cleanup
    instance_->DestroyRedirector(redirectorId);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, AddRedirectRule007, TestSize.Level1)
{
    std::string bundleName = "com.example.test";
    uint32_t groupId = TEST_GROUP_ID;
    uint32_t priority = TEST_PRIORITY;
    std::string redirectorId;

    int32_t createRet = instance_->CreateRedirector(bundleName, groupId, priority, redirectorId);
    TrafficFilterRedirectRule rule = CreateTestRuleWithUidMatch();
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_PREROUTING); // Not OUTPUT

    int32_t addRet = instance_->AddRedirectRule(redirectorId, &rule);
    EXPECT_EQ(addRet, TRAFFICFILTER_ERROR_INVALID_PARAM);

    // Cleanup
    instance_->DestroyRedirector(redirectorId);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, AddRedirectRule008, TestSize.Level1)
{
    std::string bundleName = "com.example.test";
    uint32_t groupId = TEST_GROUP_ID;
    uint32_t priority = TEST_PRIORITY;
    std::string redirectorId;

    int32_t createRet = instance_->CreateRedirector(bundleName, groupId, priority, redirectorId);
    TrafficFilterRedirectRule rule = CreateTestRule();
    rule.uidStart_ = 2000;
    rule.uidEnd_ = 1000; // Start > End, invalid range

    int32_t addRet = instance_->AddRedirectRule(redirectorId, &rule);
    EXPECT_EQ(addRet, TRAFFICFILTER_ERROR_INVALID_PARAM);

    // Cleanup
    instance_->DestroyRedirector(redirectorId);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ClearRedirectRule001, TestSize.Level1)
{
    std::string redirectorId = "non_existent_id";

    int32_t clearRet = instance_->ClearRedirectRule(redirectorId);
    EXPECT_EQ(clearRet, -1); // Redirector not found
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, PauseAllRedirectors001, TestSize.Level1)
{
    int32_t ret = instance_->PauseAllRedirectors();
    EXPECT_EQ(ret, TRAFFICFILTER_OK); // No redirectors exists
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ResumeAllRedirectors001, TestSize.Level1)
{
    int32_t ret = instance_->ResumeAllRedirectors();
    EXPECT_EQ(ret, TRAFFICFILTER_OK); // No redirectors exists
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, PauseRedirectorsByBundleName001, TestSize.Level1)
{
    std::string bundleName = "non.existent.bundle";

    int32_t ret = instance_->PauseRedirectorsByBundleName(bundleName);
    EXPECT_EQ(ret, TRAFFICFILTER_OK); // No redirectors found
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ResumeRedirectorsByBundleName001, TestSize.Level1)
{
    std::string bundleName = "non.existent.bundle";

    int32_t ret = instance_->ResumeRedirectorsByBundleName(bundleName);
    EXPECT_EQ(ret, TRAFFICFILTER_OK); // No redirectors found
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, GlobalEnableTrafficFilter001, TestSize.Level1)
{
    int32_t ret = instance_->GlobalEnableTrafficFilter();
    EXPECT_EQ(ret, TRAFFICFILTER_OK); // Already enabled by default
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, GlobalDisableTrafficFilter001, TestSize.Level1)
{
    int32_t ret = instance_->GlobalDisableTrafficFilter();
    EXPECT_EQ(ret, TRAFFICFILTER_OK);

    int32_t ret2 = instance_->GlobalDisableTrafficFilter();
    EXPECT_EQ(ret2, TRAFFICFILTER_OK); // Already disabled

    // Re-enable for cleanup
    instance_->GlobalEnableTrafficFilter();
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, GetTrafficFilterGlobalStatus001, TestSize.Level1)
{
    bool isEnabled = false;
    int32_t ret = instance_->GetTrafficFilterGlobalStatus(isEnabled);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
    EXPECT_TRUE(isEnabled); // Default should be enabled
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, GetTrafficFilterGlobalStatus002, TestSize.Level1)
{
    instance_->GlobalDisableTrafficFilter();

    bool isEnabled = true;
    int32_t ret = instance_->GetTrafficFilterGlobalStatus(isEnabled);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
    EXPECT_FALSE(isEnabled);

    // Re-enable for cleanup
    instance_->GlobalEnableTrafficFilter();
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ValidateCidrIPMatch001, TestSize.Level1)
{
    TrafficFilterIPMatch ipMatch;
    ipMatch.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_CIDR);
    ipMatch.cidr_.base_.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4);
    ipMatch.cidr_.prefixLen_ = 33; // Invalid: > 32 for IPv4

    bool isValid = instance_->ValidateCidrIPMatch(ipMatch);
    EXPECT_FALSE(isValid);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ValidateCidrIPMatch002, TestSize.Level1)
{
    TrafficFilterIPMatch ipMatch;
    ipMatch.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_CIDR);
    ipMatch.cidr_.base_.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4);
    ipMatch.cidr_.prefixLen_ = 24; // Valid for IPv4

    bool isValid = instance_->ValidateCidrIPMatch(ipMatch);
    EXPECT_TRUE(isValid);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ValidateRangeIPMatch001, TestSize.Level1)
{
    TrafficFilterIPMatch ipMatch;
    ipMatch.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_RANGE);
    ipMatch.range_.start_.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4);
    ipMatch.range_.end_.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V6); // Family mismatch

    bool isValid = instance_->ValidateRangeIPMatch(ipMatch);
    EXPECT_FALSE(isValid);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ValidateRangeIPMatch002, TestSize.Level1)
{
    TrafficFilterIPMatch ipMatch;
    ipMatch.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_RANGE);
    ipMatch.range_.start_.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4);
    ipMatch.range_.end_.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4); // Same family

    bool isValid = instance_->ValidateRangeIPMatch(ipMatch);
    EXPECT_TRUE(isValid);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ValidateMultiIPMatch001, TestSize.Level1)
{
    TrafficFilterIPMatch ipMatch;
    ipMatch.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_MULTI);
    ipMatch.multi_.ipCount_ = 0; // Empty

    bool isValid = instance_->ValidateMultiIPMatch(ipMatch);
    EXPECT_FALSE(isValid);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ValidateMultiIPMatch002, TestSize.Level1)
{
    TrafficFilterIPMatch ipMatch;
    ipMatch.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_MULTI);
    ipMatch.multi_.ipCount_ = 17; // Exceeds MAX_RULE_IP_COUNT typically 16

    bool isValid = instance_->ValidateMultiIPMatch(ipMatch);
    EXPECT_FALSE(isValid);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ValidateMultiIPMatch003, TestSize.Level1)
{
    TrafficFilterIPMatch ipMatch;
    ipMatch.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_MULTI);
    ipMatch.multi_.ipCount_ = 2;
    ipMatch.multi_.ips_[0].family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4);
    ipMatch.multi_.ips_[1].family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V6); // Family mismatch

    bool isValid = instance_->ValidateMultiIPMatch(ipMatch);
    EXPECT_FALSE(isValid);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ValidateMultiIPMatch004, TestSize.Level1)
{
    TrafficFilterIPMatch ipMatch;
    ipMatch.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_MULTI);
    ipMatch.multi_.ipCount_ = 2;
    ipMatch.multi_.ips_[0].family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4);
    ipMatch.multi_.ips_[1].family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4); // Same family

    bool isValid = instance_->ValidateMultiIPMatch(ipMatch);
    EXPECT_TRUE(isValid);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ValidateIPMatch001, TestSize.Level1)
{
    TrafficFilterIPMatch ipMatch;
    ipMatch.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);

    bool isValid = instance_->ValidateIPMatch(ipMatch);
    EXPECT_TRUE(isValid);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ValidateIPMatch002, TestSize.Level1)
{
    TrafficFilterIPMatch ipMatch;
    ipMatch.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE);
    ipMatch.single_.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4);

    bool isValid = instance_->ValidateIPMatch(ipMatch);
    EXPECT_TRUE(isValid);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ValidateIPMatch003, TestSize.Level1)
{
    TrafficFilterIPMatch ipMatch;
    ipMatch.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE);
    ipMatch.single_.family_ = 99; // Invalid family

    bool isValid = instance_->ValidateIPMatch(ipMatch);
    EXPECT_FALSE(isValid);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ValidatePortMatch001, TestSize.Level1)
{
    TrafficFilterPortMatch portMatch;
    portMatch.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);

    bool isValid = instance_->ValidatePortMatch(portMatch);
    EXPECT_TRUE(isValid);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ValidatePortMatch002, TestSize.Level1)
{
    TrafficFilterPortMatch portMatch;
    portMatch.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_MULTI);
    portMatch.multi_.portCount_ = 0; // Empty

    bool isValid = instance_->ValidatePortMatch(portMatch);
    EXPECT_FALSE(isValid);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ValidatePortMatch003, TestSize.Level1)
{
    TrafficFilterPortMatch portMatch;
    portMatch.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_MULTI);
    portMatch.multi_.portCount_ = 65; // Exceeds typical limit

    bool isValid = instance_->ValidatePortMatch(portMatch);
    EXPECT_FALSE(isValid);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ValidateInterfaceMatch001, TestSize.Level1)
{
    TrafficFilterInterfaceMatch interfaceMatch;
    interfaceMatch.enabled_ = true;
    interfaceMatch.ifName_ = ""; // Empty but enabled

    bool isValid = instance_->ValidateInterfaceMatch(interfaceMatch);
    EXPECT_FALSE(isValid);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ValidateInterfaceMatch002, TestSize.Level1)
{
    TrafficFilterInterfaceMatch interfaceMatch;
    interfaceMatch.enabled_ = true;
    interfaceMatch.ifName_ = "eth0"; // Valid

    bool isValid = instance_->ValidateInterfaceMatch(interfaceMatch);
    EXPECT_TRUE(isValid);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ValidateInterfaceMatch003, TestSize.Level1)
{
    TrafficFilterInterfaceMatch interfaceMatch;
    interfaceMatch.enabled_ = false; // Disabled, so empty name is OK
    interfaceMatch.ifName_ = "";

    bool isValid = instance_->ValidateInterfaceMatch(interfaceMatch);
    EXPECT_TRUE(isValid);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ValidateIPFamilyConsistency001, TestSize.Level1)
{
    TrafficFilterIPMatch srcIp;
    TrafficFilterIPMatch dstIp;
    srcIp.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE);
    dstIp.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE);
    srcIp.single_.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4);
    dstIp.single_.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V6); // Mismatch

    bool isValid = instance_->ValidateIPFamilyConsistency(srcIp, dstIp);
    EXPECT_FALSE(isValid);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ValidateIPFamilyConsistency002, TestSize.Level1)
{
    TrafficFilterIPMatch srcIp;
    TrafficFilterIPMatch dstIp;
    srcIp.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE);
    dstIp.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE);
    srcIp.single_.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4);
    dstIp.single_.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4); // Same family

    bool isValid = instance_->ValidateIPFamilyConsistency(srcIp, dstIp);
    EXPECT_TRUE(isValid);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ValidateIPFamilyConsistency003, TestSize.Level1)
{
    TrafficFilterIPMatch srcIp;
    TrafficFilterIPMatch dstIp;
    srcIp.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    dstIp.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY); // Both ANY

    bool isValid = instance_->ValidateIPFamilyConsistency(srcIp, dstIp);
    EXPECT_TRUE(isValid);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, DetermineRuleFamily001, TestSize.Level1)
{
    TrafficFilterRedirectRule rule = CreateTestRule();
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE);
    rule.srcIp_.single_.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE);
    rule.dstIp_.single_.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4);

    TrafficFilterIPFamily family = instance_->DetermineRuleFamily(rule);
    EXPECT_EQ(static_cast<int32_t>(family), static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4));
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, DetermineRuleFamily002, TestSize.Level1)
{
    TrafficFilterRedirectRule rule = CreateTestRule();
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE);
    rule.srcIp_.single_.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE);
    rule.dstIp_.single_.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V6); // Mismatch

    TrafficFilterIPFamily family = instance_->DetermineRuleFamily(rule);
    EXPECT_EQ(static_cast<int32_t>(family), static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_UNSPEC));
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, GetIPFamilyFromMatch001, TestSize.Level1)
{
    TrafficFilterIPMatch ipMatch;
    ipMatch.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE);
    ipMatch.single_.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4);

    TrafficFilterIPFamily family = instance_->GetIPFamilyFromMatch(ipMatch);
    EXPECT_EQ(static_cast<int32_t>(family), static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4));
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, GetIPFamilyFromMatch002, TestSize.Level1)
{
    TrafficFilterIPMatch ipMatch;
    ipMatch.type_ = 99; // Invalid type

    TrafficFilterIPFamily family = instance_->GetIPFamilyFromMatch(ipMatch);
    EXPECT_EQ(static_cast<int32_t>(family), static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_UNSPEC));
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, GetIPFamilyFromMatch003, TestSize.Level1)
{
    TrafficFilterIPMatch ipMatch;
    ipMatch.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_MULTI);
    ipMatch.multi_.ipCount_ = 0; // Empty

    TrafficFilterIPFamily family = instance_->GetIPFamilyFromMatch(ipMatch);
    EXPECT_EQ(static_cast<int32_t>(family), static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_UNSPEC));
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, GenerateRedirectorId001, TestSize.Level1)
{
    std::string id1 = instance_->GenerateRedirectorId();
    std::string id2 = instance_->GenerateRedirectorId();

    EXPECT_FALSE(id1.empty());
    EXPECT_FALSE(id2.empty());
    EXPECT_NE(id1, id2);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, MatchTcpConnection001, TestSize.Level1)
{
    TcpNetPortStatesInfo tcpInfo;
    tcpInfo.tcpLocalIp_ = "192.168.1.100";
    tcpInfo.tcpLocalPort_ = 54321;
    tcpInfo.tcpRemoteIp_ = "93.184.216.34";
    tcpInfo.tcpRemotePort_ = 443;
    tcpInfo.tcpUid_ = 1000;
    tcpInfo.tcpPid_ = 12345;

    bool isMatch = instance_->MatchTcpConnection(tcpInfo, "192.168.1.100", 54321,
                                                "93.184.216.34", 443);
    EXPECT_TRUE(isMatch);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, MatchTcpConnection002, TestSize.Level1)
{
    TcpNetPortStatesInfo tcpInfo;
    tcpInfo.tcpLocalIp_ = "192.168.1.100";
    tcpInfo.tcpLocalPort_ = 54321;
    tcpInfo.tcpRemoteIp_ = "93.184.216.34";
    tcpInfo.tcpRemotePort_ = 443;

    bool isMatch = instance_->MatchTcpConnection(tcpInfo, "93.184.216.34", 443,
                                                "192.168.1.100", 54321); // Reversed
    EXPECT_TRUE(isMatch);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, MatchTcpConnection003, TestSize.Level1)
{
    TcpNetPortStatesInfo tcpInfo;
    tcpInfo.tcpLocalIp_ = "192.168.1.100";
    tcpInfo.tcpLocalPort_ = 54321;
    tcpInfo.tcpRemoteIp_ = "93.184.216.34";
    tcpInfo.tcpRemotePort_ = 443;

    bool isMatch = instance_->MatchTcpConnection(tcpInfo, "192.168.1.100", 54321,
                                                "93.184.216.34", 80); // Wrong port
    EXPECT_FALSE(isMatch);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, MatchUdpConnection001, TestSize.Level1)
{
    UdpNetPortStatesInfo udpInfo;
    udpInfo.udpLocalIp_ = "192.168.1.100";
    udpInfo.udpLocalPort_ = 12345;
    udpInfo.udpUid_ = 1000;
    udpInfo.udpPid_ = 12345;

    bool isMatch = instance_->MatchUdpConnection(udpInfo, "192.168.1.100", 12345,
                                                "8.8.8.8", 53);
    EXPECT_TRUE(isMatch);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, MatchUdpConnection002, TestSize.Level1)
{
    UdpNetPortStatesInfo udpInfo;
    udpInfo.udpLocalIp_ = "192.168.1.100";
    udpInfo.udpLocalPort_ = 12345;

    bool isMatch = instance_->MatchUdpConnection(udpInfo, "8.8.8.8", 53,
                                                "192.168.1.100", 12345); // Reversed
    EXPECT_TRUE(isMatch);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ValidateProxyFamilyConsistency001, TestSize.Level1)
{
    TrafficFilterRedirectRule rule = CreateTestRule();
    rule.proxyIp_.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4);
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE);
    rule.srcIp_.single_.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE);
    rule.dstIp_.single_.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4);

    bool isValid = instance_->ValidateProxyFamilyConsistency(rule);
    EXPECT_TRUE(isValid);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ValidateProxyFamilyConsistency002, TestSize.Level1)
{
    TrafficFilterRedirectRule rule = CreateTestRule();
    rule.proxyIp_.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V6); // Mismatch
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE);
    rule.srcIp_.single_.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE);
    rule.dstIp_.single_.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4);

    bool isValid = instance_->ValidateProxyFamilyConsistency(rule);
    EXPECT_FALSE(isValid);
}
} // namespace NetManagerStandard
} // namespace OHOS
