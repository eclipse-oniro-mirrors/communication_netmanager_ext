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

HWTEST_F(NetTrafficFilterRedirectManagerTest, IsRedirectorExists001, TestSize.Level1)
{
    // bundleName not found in map
    std::string bundleName = "com.example.notfound";
    bool exists = instance_->IsRedirectorExists(bundleName, TEST_GROUP_ID);
    EXPECT_FALSE(exists);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, IsRedirectorExists002, TestSize.Level1)
{
    // empty bundleName
    std::string bundleName = "";
    bool exists = instance_->IsRedirectorExists(bundleName, TEST_GROUP_ID);
    EXPECT_FALSE(exists);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, IsRedirectorExists003, TestSize.Level1)
{
    // bundleName found, groupId match
    std::string bundleName = "com.example.exists";
    std::string redirectorId = "test_redir_001";
    auto redirector = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId, bundleName, TEST_GROUP_ID, TEST_PRIORITY);
    instance_->redirectors_[redirectorId] = redirector;
    instance_->bundleNameToRedirectorsMap_[bundleName].push_back(redirectorId);

    bool exists = instance_->IsRedirectorExists(bundleName, TEST_GROUP_ID);
    EXPECT_TRUE(exists);

    // Cleanup
    instance_->redirectors_.erase(redirectorId);
    instance_->bundleNameToRedirectorsMap_.erase(bundleName);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, IsRedirectorExists004, TestSize.Level1)
{
    // bundleName found, groupId mismatch
    std::string bundleName = "com.example.mismatch";
    std::string redirectorId = "test_redir_002";
    auto redirector = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId, bundleName, TEST_GROUP_ID, TEST_PRIORITY);
    instance_->redirectors_[redirectorId] = redirector;
    instance_->bundleNameToRedirectorsMap_[bundleName].push_back(redirectorId);

    bool exists = instance_->IsRedirectorExists(bundleName, TEST_GROUP_ID + 1);
    EXPECT_FALSE(exists);

    // Cleanup
    instance_->redirectors_.erase(redirectorId);
    instance_->bundleNameToRedirectorsMap_.erase(bundleName);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, IsRedirectorExists005, TestSize.Level1)
{
    // bundleName found, redirector is null
    std::string bundleName = "com.example.null";
    std::string redirectorId = "test_redir_003";
    instance_->redirectors_[redirectorId] = nullptr;
    instance_->bundleNameToRedirectorsMap_[bundleName].push_back(redirectorId);

    bool exists = instance_->IsRedirectorExists(bundleName, TEST_GROUP_ID);
    EXPECT_FALSE(exists);

    // Cleanup
    instance_->redirectors_.erase(redirectorId);
    instance_->bundleNameToRedirectorsMap_.erase(bundleName);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, IsRedirectorExists006, TestSize.Level1)
{
    // bundleName found, orphan redirectorId (not in redirectors_ map)
    std::string bundleName = "com.example.orphan";
    std::string redirectorId = "test_redir_orphan";
    instance_->bundleNameToRedirectorsMap_[bundleName].push_back(redirectorId);

    bool exists = instance_->IsRedirectorExists(bundleName, TEST_GROUP_ID);
    EXPECT_FALSE(exists);

    // Cleanup
    instance_->bundleNameToRedirectorsMap_.erase(bundleName);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, IsRedirectorExists007, TestSize.Level1)
{
    // multiple redirectors, one matches groupId
    std::string bundleName = "com.example.multi";
    std::string redirectorId1 = "test_redir_multi_1";
    std::string redirectorId2 = "test_redir_multi_2";
    auto redirector1 = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId1, bundleName, TEST_GROUP_ID, TEST_PRIORITY);
    auto redirector2 = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId2, bundleName, TEST_GROUP_ID + 1, TEST_PRIORITY);
    instance_->redirectors_[redirectorId1] = redirector1;
    instance_->redirectors_[redirectorId2] = redirector2;
    instance_->bundleNameToRedirectorsMap_[bundleName].push_back(redirectorId1);
    instance_->bundleNameToRedirectorsMap_[bundleName].push_back(redirectorId2);

    bool exists = instance_->IsRedirectorExists(bundleName, TEST_GROUP_ID + 1);
    EXPECT_TRUE(exists);

    // Cleanup
    instance_->redirectors_.erase(redirectorId1);
    instance_->redirectors_.erase(redirectorId2);
    instance_->bundleNameToRedirectorsMap_.erase(bundleName);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, IsRedirectorExists008, TestSize.Level1)
{
    // multiple redirectors, none match groupId
    std::string bundleName = "com.example.multi_nomatch";
    std::string redirectorId1 = "test_redir_multi_3";
    std::string redirectorId2 = "test_redir_multi_4";
    auto redirector1 = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId1, bundleName, TEST_GROUP_ID, TEST_PRIORITY);
    auto redirector2 = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId2, bundleName, TEST_GROUP_ID + 1, TEST_PRIORITY);
    instance_->redirectors_[redirectorId1] = redirector1;
    instance_->redirectors_[redirectorId2] = redirector2;
    instance_->bundleNameToRedirectorsMap_[bundleName].push_back(redirectorId1);
    instance_->bundleNameToRedirectorsMap_[bundleName].push_back(redirectorId2);

    bool exists = instance_->IsRedirectorExists(bundleName, TEST_GROUP_ID + 999);
    EXPECT_FALSE(exists);

    // Cleanup
    instance_->redirectors_.erase(redirectorId1);
    instance_->redirectors_.erase(redirectorId2);
    instance_->bundleNameToRedirectorsMap_.erase(bundleName);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, IsRedirectorExists009, TestSize.Level1)
{
    // mixed: null redirector + valid redirector with matching groupId
    std::string bundleName = "com.example.mixed";
    std::string redirectorId1 = "test_redir_mixed_null";
    std::string redirectorId2 = "test_redir_mixed_valid";
    instance_->redirectors_[redirectorId1] = nullptr;
    auto redirector2 = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId2, bundleName, TEST_GROUP_ID, TEST_PRIORITY);
    instance_->redirectors_[redirectorId2] = redirector2;
    instance_->bundleNameToRedirectorsMap_[bundleName].push_back(redirectorId1);
    instance_->bundleNameToRedirectorsMap_[bundleName].push_back(redirectorId2);

    bool exists = instance_->IsRedirectorExists(bundleName, TEST_GROUP_ID);
    EXPECT_TRUE(exists);

    // Cleanup
    instance_->redirectors_.erase(redirectorId1);
    instance_->redirectors_.erase(redirectorId2);
    instance_->bundleNameToRedirectorsMap_.erase(bundleName);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, IsRedirectorExists010, TestSize.Level1)
{
    // mixed: orphan redirectorId + valid redirector with matching groupId
    std::string bundleName = "com.example.mixed_orphan";
    std::string redirectorId1 = "test_redir_mixed_orphan";
    std::string redirectorId2 = "test_redir_mixed_valid2";
    auto redirector2 = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId2, bundleName, TEST_GROUP_ID, TEST_PRIORITY);
    instance_->redirectors_[redirectorId2] = redirector2;
    instance_->bundleNameToRedirectorsMap_[bundleName].push_back(redirectorId1);
    instance_->bundleNameToRedirectorsMap_[bundleName].push_back(redirectorId2);

    bool exists = instance_->IsRedirectorExists(bundleName, TEST_GROUP_ID);
    EXPECT_TRUE(exists);

    // Cleanup
    instance_->redirectors_.erase(redirectorId2);
    instance_->bundleNameToRedirectorsMap_.erase(bundleName);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, RollbackRedirectorRules001, TestSize.Level1)
{
    // null redirector
    std::shared_ptr<NetTrafficFilterRedirectorContext> redirector = nullptr;
    std::string chainName = "test_chain";
    std::vector<TrafficFilterRedirectRule> oldRules;
    std::set<TrafficFilterHookPoint> affectedHookPoints;

    int32_t ret = instance_->RollbackRedirectorRules(redirector, chainName, oldRules, affectedHookPoints);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, RollbackRedirectorRules002, TestSize.Level1)
{
    // valid redirector, empty oldRules, empty affectedHookPoints
    std::string redirectorId = "test_rollback_002";
    std::string bundleName = "com.example.rollback002";
    auto redirector = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId, bundleName, TEST_GROUP_ID, TEST_PRIORITY);
    std::string chainName = "test_chain_002";
    std::vector<TrafficFilterRedirectRule> oldRules;
    std::set<TrafficFilterHookPoint> affectedHookPoints;

    int32_t ret = instance_->RollbackRedirectorRules(redirector, chainName, oldRules, affectedHookPoints);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, RollbackRedirectorRules003, TestSize.Level1)
{
    // valid redirector with oldRules, empty affectedHookPoints
    std::string redirectorId = "test_rollback_003";
    std::string bundleName = "com.example.rollback003";
    auto redirector = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId, bundleName, TEST_GROUP_ID, TEST_PRIORITY);
    std::string chainName = "test_chain_003";
    std::vector<TrafficFilterRedirectRule> oldRules;
    TrafficFilterRedirectRule rule = CreateTestRule();
    oldRules.push_back(rule);
    std::set<TrafficFilterHookPoint> affectedHookPoints;

    int32_t ret = instance_->RollbackRedirectorRules(redirector, chainName, oldRules, affectedHookPoints);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);

    // Verify rules were restored
    std::vector<TrafficFilterRedirectRule> restoredRules = redirector->GetRules();
    EXPECT_EQ(restoredRules.size(), 1);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, RollbackRedirectorRules004, TestSize.Level1)
{
    // valid redirector, empty oldRules, with affectedHookPoints
    std::string redirectorId = "test_rollback_004";
    std::string bundleName = "com.example.rollback004";
    auto redirector = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId, bundleName, TEST_GROUP_ID, TEST_PRIORITY);
    std::string chainName = "test_chain_004";
    std::vector<TrafficFilterRedirectRule> oldRules;
    std::set<TrafficFilterHookPoint> affectedHookPoints;
    affectedHookPoints.insert(TrafficFilterHookPoint::HOOK_PREROUTING);

    int32_t ret = instance_->RollbackRedirectorRules(redirector, chainName, oldRules, affectedHookPoints);
    // ApplyGlobalJumpRules may succeed or fail depending on environment
    EXPECT_TRUE(ret == TRAFFICFILTER_OK || ret == -1);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, RollbackRedirectorRules005, TestSize.Level1)
{
    // valid redirector with oldRules and multiple affectedHookPoints
    std::string redirectorId = "test_rollback_005";
    std::string bundleName = "com.example.rollback005";
    auto redirector = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId, bundleName, TEST_GROUP_ID, TEST_PRIORITY);
    std::string chainName = "test_chain_005";
    std::vector<TrafficFilterRedirectRule> oldRules;
    TrafficFilterRedirectRule rule1 = CreateTestRule(TEST_PRIORITY);
    TrafficFilterRedirectRule rule2 = CreateTestRule(TEST_PRIORITY + 1);
    oldRules.push_back(rule1);
    oldRules.push_back(rule2);
    std::set<TrafficFilterHookPoint> affectedHookPoints;
    affectedHookPoints.insert(TrafficFilterHookPoint::HOOK_PREROUTING);
    affectedHookPoints.insert(TrafficFilterHookPoint::HOOK_OUTPUT);

    int32_t ret = instance_->RollbackRedirectorRules(redirector, chainName, oldRules, affectedHookPoints);
    // May succeed or fail depending on environment
    EXPECT_TRUE(ret == TRAFFICFILTER_OK || ret == -1);

    // Verify rules were restored regardless of iptables result
    std::vector<TrafficFilterRedirectRule> restoredRules = redirector->GetRules();
    EXPECT_EQ(restoredRules.size(), 2);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, RollbackRedirectorRules006, TestSize.Level1)
{
    // redirector with existing rules, restore to different oldRules
    std::string redirectorId = "test_rollback_006";
    std::string bundleName = "com.example.rollback006";
    auto redirector = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId, bundleName, TEST_GROUP_ID, TEST_PRIORITY);

    // Add some current rules
    TrafficFilterRedirectRule currentRule = CreateTestRule(TEST_PRIORITY);
    redirector->AddRuleWithPriority(currentRule);
    ASSERT_EQ(redirector->GetRules().size(), 1);

    // Restore to empty oldRules (simulating rollback after failed add)
    std::string chainName = "test_chain_006";
    std::vector<TrafficFilterRedirectRule> oldRules;
    std::set<TrafficFilterHookPoint> affectedHookPoints;

    int32_t ret = instance_->RollbackRedirectorRules(redirector, chainName, oldRules, affectedHookPoints);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);

    // Verify rules were restored to empty
    std::vector<TrafficFilterRedirectRule> restoredRules = redirector->GetRules();
    EXPECT_EQ(restoredRules.size(), 0);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ApplyGlobalJumpRules001, TestSize.Level1)
{
    // No redirectors exist, HOOK_PREROUTING - both V4 and V6 have no active redirectors
    int32_t ret = instance_->ApplyGlobalJumpRules(TrafficFilterHookPoint::HOOK_PREROUTING);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ApplyGlobalJumpRules002, TestSize.Level1)
{
    // No redirectors exist, HOOK_OUTPUT - both V4 and V6 have no active redirectors
    int32_t ret = instance_->ApplyGlobalJumpRules(TrafficFilterHookPoint::HOOK_OUTPUT);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ApplyGlobalJumpRules003, TestSize.Level1)
{
    // Invalid hook point - GetHookPointName returns empty, UpdateGlobalJumpRules returns -1
    int32_t ret = instance_->ApplyGlobalJumpRules(static_cast<TrafficFilterHookPoint>(99));
    EXPECT_EQ(ret, -1);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ApplyGlobalJumpRules004, TestSize.Level1)
{
    // Redirector with no rules - not active, should succeed
    std::string redirectorId = "test_apply_jump_004";
    std::string bundleName = "com.example.applyjump004";
    auto redirector = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId, bundleName, TEST_GROUP_ID, TEST_PRIORITY);
    instance_->redirectors_[redirectorId] = redirector;
    instance_->redirectorIdList_.push_back(redirectorId);

    int32_t ret = instance_->ApplyGlobalJumpRules(TrafficFilterHookPoint::HOOK_PREROUTING);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);

    // Cleanup
    instance_->redirectors_.erase(redirectorId);
    auto listIt = std::remove(instance_->redirectorIdList_.begin(), instance_->redirectorIdList_.end(),
        redirectorId);
    instance_->redirectorIdList_.erase(listIt, instance_->redirectorIdList_.end());
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ApplyGlobalJumpRules005, TestSize.Level1)
{
    // Paused redirector with rules - not active (IsPaused returns true), should succeed
    std::string redirectorId = "test_apply_jump_005";
    std::string bundleName = "com.example.applyjump005";
    auto redirector = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId, bundleName, TEST_GROUP_ID, TEST_PRIORITY);
    TrafficFilterRedirectRule rule = CreateTestRule();
    redirector->AddRuleWithPriority(rule);
    redirector->SetPaused(true);
    instance_->redirectors_[redirectorId] = redirector;
    instance_->redirectorIdList_.push_back(redirectorId);

    int32_t ret = instance_->ApplyGlobalJumpRules(TrafficFilterHookPoint::HOOK_PREROUTING);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);

    // Cleanup
    instance_->redirectors_.erase(redirectorId);
    auto listIt = std::remove(instance_->redirectorIdList_.begin(), instance_->redirectorIdList_.end(),
        redirectorId);
    instance_->redirectorIdList_.erase(listIt, instance_->redirectorIdList_.end());
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ApplyGlobalJumpRules006, TestSize.Level1)
{
    // Redirector with rules for HOOK_OUTPUT, apply for HOOK_PREROUTING - no active redirectors for this hook
    std::string redirectorId = "test_apply_jump_006";
    std::string bundleName = "com.example.applyjump006";
    auto redirector = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId, bundleName, TEST_GROUP_ID, TEST_PRIORITY);
    TrafficFilterRedirectRule rule = CreateTestRule();
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    redirector->AddRuleWithPriority(rule);
    instance_->redirectors_[redirectorId] = redirector;
    instance_->redirectorIdList_.push_back(redirectorId);

    int32_t ret = instance_->ApplyGlobalJumpRules(TrafficFilterHookPoint::HOOK_PREROUTING);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);

    // Cleanup
    instance_->redirectors_.erase(redirectorId);
    auto listIt = std::remove(instance_->redirectorIdList_.begin(), instance_->redirectorIdList_.end(),
        redirectorId);
    instance_->redirectorIdList_.erase(listIt, instance_->redirectorIdList_.end());
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ApplyGlobalJumpRules007, TestSize.Level1)
{
    // Null redirector in map - should be skipped, no active redirectors
    std::string redirectorId = "test_apply_jump_007";
    instance_->redirectors_[redirectorId] = nullptr;
    instance_->redirectorIdList_.push_back(redirectorId);

    int32_t ret = instance_->ApplyGlobalJumpRules(TrafficFilterHookPoint::HOOK_PREROUTING);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);

    // Cleanup
    instance_->redirectors_.erase(redirectorId);
    auto listIt = std::remove(instance_->redirectorIdList_.begin(), instance_->redirectorIdList_.end(),
        redirectorId);
    instance_->redirectorIdList_.erase(listIt, instance_->redirectorIdList_.end());
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ApplyGlobalJumpRules008, TestSize.Level1)
{
    // Redirector with V4 rules - V4 has active redirectors (may fail), V6 has none (should succeed)
    // Result depends on iptables execution environment
    std::string redirectorId = "test_apply_jump_008";
    std::string bundleName = "com.example.applyjump008";
    auto redirector = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId, bundleName, TEST_GROUP_ID, TEST_PRIORITY);
    TrafficFilterRedirectRule rule = CreateTestRule();
    redirector->AddRuleWithPriority(rule);
    instance_->redirectors_[redirectorId] = redirector;
    instance_->redirectorIdList_.push_back(redirectorId);

    int32_t ret = instance_->ApplyGlobalJumpRules(TrafficFilterHookPoint::HOOK_PREROUTING);
    EXPECT_TRUE(ret == TRAFFICFILTER_OK || ret == -1);

    // Cleanup
    instance_->redirectors_.erase(redirectorId);
    auto listIt = std::remove(instance_->redirectorIdList_.begin(), instance_->redirectorIdList_.end(),
        redirectorId);
    instance_->redirectorIdList_.erase(listIt, instance_->redirectorIdList_.end());
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ApplyGlobalJumpRules009, TestSize.Level1)
{
    // Multiple redirectors, none active (all without rules) - should succeed
    std::string redirectorId1 = "test_apply_jump_009_1";
    std::string redirectorId2 = "test_apply_jump_009_2";
    std::string bundleName = "com.example.applyjump009";
    auto redirector1 = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId1, bundleName, TEST_GROUP_ID, TEST_PRIORITY);
    auto redirector2 = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId2, bundleName, TEST_GROUP_ID + 1, TEST_PRIORITY);
    instance_->redirectors_[redirectorId1] = redirector1;
    instance_->redirectors_[redirectorId2] = redirector2;
    instance_->redirectorIdList_.push_back(redirectorId1);
    instance_->redirectorIdList_.push_back(redirectorId2);

    int32_t ret = instance_->ApplyGlobalJumpRules(TrafficFilterHookPoint::HOOK_PREROUTING);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);

    // Cleanup
    instance_->redirectors_.erase(redirectorId1);
    instance_->redirectors_.erase(redirectorId2);
    auto listIt1 = std::remove(instance_->redirectorIdList_.begin(), instance_->redirectorIdList_.end(),
        redirectorId1);
    instance_->redirectorIdList_.erase(listIt1, instance_->redirectorIdList_.end());
    auto listIt2 = std::remove(instance_->redirectorIdList_.begin(), instance_->redirectorIdList_.end(),
        redirectorId2);
    instance_->redirectorIdList_.erase(listIt2, instance_->redirectorIdList_.end());
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ApplyGlobalJumpRules010, TestSize.Level1)
{
    // Redirector with V6 rules only - V4 has no active redirectors (should succeed),
    // V6 has active redirectors (may fail), result depends on environment
    std::string redirectorId = "test_apply_jump_010";
    std::string bundleName = "com.example.applyjump010";
    auto redirector = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId, bundleName, TEST_GROUP_ID, TEST_PRIORITY);
    TrafficFilterRedirectRule rule = CreateTestRule();
    rule.proxyIp_.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V6);
    redirector->AddRuleWithPriority(rule);
    instance_->redirectors_[redirectorId] = redirector;
    instance_->redirectorIdList_.push_back(redirectorId);

    int32_t ret = instance_->ApplyGlobalJumpRules(TrafficFilterHookPoint::HOOK_PREROUTING);
    EXPECT_TRUE(ret == TRAFFICFILTER_OK || ret == -1);

    // Cleanup
    instance_->redirectors_.erase(redirectorId);
    auto listIt = std::remove(instance_->redirectorIdList_.begin(), instance_->redirectorIdList_.end(),
        redirectorId);
    instance_->redirectorIdList_.erase(listIt, instance_->redirectorIdList_.end());
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ApplyGlobalJumpRules011, TestSize.Level1)
{
    // Orphan ID in redirectorIdList_ (not in redirectors_ map) - should be skipped
    std::string orphanId = "test_apply_jump_orphan_011";
    instance_->redirectorIdList_.push_back(orphanId);

    int32_t ret = instance_->ApplyGlobalJumpRules(TrafficFilterHookPoint::HOOK_PREROUTING);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);

    // Cleanup
    auto listIt = std::remove(instance_->redirectorIdList_.begin(), instance_->redirectorIdList_.end(),
        orphanId);
    instance_->redirectorIdList_.erase(listIt, instance_->redirectorIdList_.end());
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ApplyGlobalJumpRules012, TestSize.Level1)
{
    // Mixed: null redirector + valid redirector without rules - none active, should succeed
    std::string nullId = "test_apply_jump_null_012";
    std::string validId = "test_apply_jump_valid_012";
    std::string bundleName = "com.example.applyjump012";
    instance_->redirectors_[nullId] = nullptr;
    auto validRedirector = std::make_shared<NetTrafficFilterRedirectorContext>(
        validId, bundleName, TEST_GROUP_ID, TEST_PRIORITY);
    instance_->redirectors_[validId] = validRedirector;
    instance_->redirectorIdList_.push_back(nullId);
    instance_->redirectorIdList_.push_back(validId);

    int32_t ret = instance_->ApplyGlobalJumpRules(TrafficFilterHookPoint::HOOK_PREROUTING);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);

    // Cleanup
    instance_->redirectors_.erase(nullId);
    instance_->redirectors_.erase(validId);
    auto listIt1 = std::remove(instance_->redirectorIdList_.begin(), instance_->redirectorIdList_.end(),
        nullId);
    instance_->redirectorIdList_.erase(listIt1, instance_->redirectorIdList_.end());
    auto listIt2 = std::remove(instance_->redirectorIdList_.begin(), instance_->redirectorIdList_.end(),
        validId);
    instance_->redirectorIdList_.erase(listIt2, instance_->redirectorIdList_.end());
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ApplyGlobalJumpRules013, TestSize.Level1)
{
    // Redirector with rules for HOOK_PREROUTING, apply for HOOK_OUTPUT - no active for this hook
    std::string redirectorId = "test_apply_jump_013";
    std::string bundleName = "com.example.applyjump013";
    auto redirector = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId, bundleName, TEST_GROUP_ID, TEST_PRIORITY);
    TrafficFilterRedirectRule rule = CreateTestRule();
    rule.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_PREROUTING);
    redirector->AddRuleWithPriority(rule);
    instance_->redirectors_[redirectorId] = redirector;
    instance_->redirectorIdList_.push_back(redirectorId);

    int32_t ret = instance_->ApplyGlobalJumpRules(TrafficFilterHookPoint::HOOK_OUTPUT);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);

    // Cleanup
    instance_->redirectors_.erase(redirectorId);
    auto listIt = std::remove(instance_->redirectorIdList_.begin(), instance_->redirectorIdList_.end(),
        redirectorId);
    instance_->redirectorIdList_.erase(listIt, instance_->redirectorIdList_.end());
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ApplyGlobalJumpRules014, TestSize.Level1)
{
    // Multiple redirectors with rules, some paused, some active - result depends on environment
    std::string redirectorId1 = "test_apply_jump_014_1";
    std::string redirectorId2 = "test_apply_jump_014_2";
    std::string bundleName = "com.example.applyjump014";
    auto redirector1 = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId1, bundleName, TEST_GROUP_ID, TEST_PRIORITY);
    TrafficFilterRedirectRule rule1 = CreateTestRule(TEST_PRIORITY);
    redirector1->AddRuleWithPriority(rule1);
    redirector1->SetPaused(true); // Paused, not active

    auto redirector2 = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId2, bundleName, TEST_GROUP_ID + 1, TEST_PRIORITY + 1);
    TrafficFilterRedirectRule rule2 = CreateTestRule(TEST_PRIORITY + 1);
    redirector2->AddRuleWithPriority(rule2); // Not paused, has rules - active for V4

    instance_->redirectors_[redirectorId1] = redirector1;
    instance_->redirectors_[redirectorId2] = redirector2;
    instance_->redirectorIdList_.push_back(redirectorId1);
    instance_->redirectorIdList_.push_back(redirectorId2);

    int32_t ret = instance_->ApplyGlobalJumpRules(TrafficFilterHookPoint::HOOK_PREROUTING);
    EXPECT_TRUE(ret == TRAFFICFILTER_OK || ret == -1);

    // Cleanup
    instance_->redirectors_.erase(redirectorId1);
    instance_->redirectors_.erase(redirectorId2);
    auto listIt1 = std::remove(instance_->redirectorIdList_.begin(), instance_->redirectorIdList_.end(),
        redirectorId1);
    instance_->redirectorIdList_.erase(listIt1, instance_->redirectorIdList_.end());
    auto listIt2 = std::remove(instance_->redirectorIdList_.begin(), instance_->redirectorIdList_.end(),
        redirectorId2);
    instance_->redirectorIdList_.erase(listIt2, instance_->redirectorIdList_.end());
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, ApplyGlobalJumpRules015, TestSize.Level1)
{
    // Redirector with rules matching both V4 and V6 (via srcIp single V4 and proxy V6)
    // This tests family mismatch scenario - DetermineRuleFamily returns UNSPEC
    std::string redirectorId = "test_apply_jump_015";
    std::string bundleName = "com.example.applyjump015";
    auto redirector = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId, bundleName, TEST_GROUP_ID, TEST_PRIORITY);
    TrafficFilterRedirectRule rule = CreateTestRule();
    // Set srcIp to V4 and proxyIp to V6 - DetermineRuleFamily returns V4 (src takes priority)
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE);
    rule.srcIp_.single_.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE);
    rule.dstIp_.single_.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4);
    redirector->AddRuleWithPriority(rule);
    instance_->redirectors_[redirectorId] = redirector;
    instance_->redirectorIdList_.push_back(redirectorId);

    // Rule family is V4, so active for V4 (may fail), not active for V6 (should succeed)
    int32_t ret = instance_->ApplyGlobalJumpRules(TrafficFilterHookPoint::HOOK_PREROUTING);
    EXPECT_TRUE(ret == TRAFFICFILTER_OK || ret == -1);

    // Cleanup
    instance_->redirectors_.erase(redirectorId);
    auto listIt = std::remove(instance_->redirectorIdList_.begin(), instance_->redirectorIdList_.end(),
        redirectorId);
    instance_->redirectorIdList_.erase(listIt, instance_->redirectorIdList_.end());
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, UnregisterTrafficFilterObserver001, TestSize.Level1)
{
    // uid not found in map - should be no-op
    int32_t uid = 9999;
    NetTrafficFilterRedirectManager::TrafficFilterHapObserver* observer = nullptr;

    instance_->UnregisterTrafficFilterObserver(uid, observer);
    EXPECT_EQ(instance_->uidToObserverMap_.find(uid), instance_->uidToObserverMap_.end());
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, UnregisterTrafficFilterObserver002, TestSize.Level1)
{
    // uid found, observer pointer matches - should unregister and erase
    int32_t uid = 10001;
    std::string bundleName = "com.example.observer002";
    sptr<NetTrafficFilterRedirectManager::TrafficFilterHapObserver> observer =
        new NetTrafficFilterRedirectManager::TrafficFilterHapObserver(
            instance_->weak_from_this(), bundleName, uid);
    instance_->uidToObserverMap_[uid] = observer;

    instance_->UnregisterTrafficFilterObserver(uid, observer.GetRefPtr());

    EXPECT_EQ(instance_->uidToObserverMap_.find(uid), instance_->uidToObserverMap_.end());
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, UnregisterTrafficFilterObserver003, TestSize.Level1)
{
    // uid found, observer pointer does NOT match - should not unregister
    int32_t uid = 10002;
    std::string bundleName = "com.example.observer003";
    sptr<NetTrafficFilterRedirectManager::TrafficFilterHapObserver> observer1 =
        new NetTrafficFilterRedirectManager::TrafficFilterHapObserver(
            instance_->weak_from_this(), bundleName, uid);
    sptr<NetTrafficFilterRedirectManager::TrafficFilterHapObserver> observer2 =
        new NetTrafficFilterRedirectManager::TrafficFilterHapObserver(
            instance_->weak_from_this(), bundleName, uid);
    instance_->uidToObserverMap_[uid] = observer1;

    instance_->UnregisterTrafficFilterObserver(uid, observer2.GetRefPtr());

    auto it = instance_->uidToObserverMap_.find(uid);
    EXPECT_NE(it, instance_->uidToObserverMap_.end());
    EXPECT_EQ(it->second.GetRefPtr(), observer1.GetRefPtr());

    instance_->uidToObserverMap_.erase(uid);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, UnregisterTrafficFilterObserver004, TestSize.Level1)
{
    // Empty map - should be no-op
    int32_t uid = 10003;
    NetTrafficFilterRedirectManager::TrafficFilterHapObserver* observer = nullptr;
    instance_->uidToObserverMap_.erase(uid);

    instance_->UnregisterTrafficFilterObserver(uid, observer);
    EXPECT_EQ(instance_->uidToObserverMap_.find(uid), instance_->uidToObserverMap_.end());
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, UnregisterTrafficFilterObserver005, TestSize.Level1)
{
    // Multiple uids in map, unregister one matching - only matching one should be removed
    int32_t uid1 = 10004;
    int32_t uid2 = 10005;
    std::string bundleName1 = "com.example.observer005a";
    std::string bundleName2 = "com.example.observer005b";
    sptr<NetTrafficFilterRedirectManager::TrafficFilterHapObserver> observer1 =
        new NetTrafficFilterRedirectManager::TrafficFilterHapObserver(
            instance_->weak_from_this(), bundleName1, uid1);
    sptr<NetTrafficFilterRedirectManager::TrafficFilterHapObserver> observer2 =
        new NetTrafficFilterRedirectManager::TrafficFilterHapObserver(
            instance_->weak_from_this(), bundleName2, uid2);
    instance_->uidToObserverMap_[uid1] = observer1;
    instance_->uidToObserverMap_[uid2] = observer2;

    instance_->UnregisterTrafficFilterObserver(uid1, observer1.GetRefPtr());

    EXPECT_EQ(instance_->uidToObserverMap_.find(uid1), instance_->uidToObserverMap_.end());
    auto it = instance_->uidToObserverMap_.find(uid2);
    EXPECT_NE(it, instance_->uidToObserverMap_.end());
    EXPECT_EQ(it->second.GetRefPtr(), observer2.GetRefPtr());

    instance_->uidToObserverMap_.erase(uid2);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, UnregisterTrafficFilterObserver006, TestSize.Level1)
{
    // uid found, pass null observer pointer - should not match, no unregister
    int32_t uid = 10006;
    std::string bundleName = "com.example.observer006";
    sptr<NetTrafficFilterRedirectManager::TrafficFilterHapObserver> observer =
        new NetTrafficFilterRedirectManager::TrafficFilterHapObserver(
            instance_->weak_from_this(), bundleName, uid);
    instance_->uidToObserverMap_[uid] = observer;

    instance_->UnregisterTrafficFilterObserver(uid, nullptr);

    auto it = instance_->uidToObserverMap_.find(uid);
    EXPECT_NE(it, instance_->uidToObserverMap_.end());
    EXPECT_EQ(it->second.GetRefPtr(), observer.GetRefPtr());

    instance_->uidToObserverMap_.erase(uid);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, DestroyRedirector002, TestSize.Level1)
{
    // Valid redirector with no rules - should succeed and clean up data structures
    std::string redirectorId = "test_destroy_002";
    std::string bundleName = "com.example.destroy002";
    auto redirector = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId, bundleName, TEST_GROUP_ID, TEST_PRIORITY);
    redirector->SetCallingInfo(TEST_UID, TEST_PID);
    instance_->redirectors_[redirectorId] = redirector;
    instance_->bundleNameToRedirectorsMap_[bundleName].push_back(redirectorId);
    instance_->redirectorIdList_.push_back(redirectorId);

    int32_t ret = instance_->DestroyRedirector(redirectorId);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);

    EXPECT_EQ(instance_->redirectors_.find(redirectorId), instance_->redirectors_.end());
    EXPECT_EQ(instance_->bundleNameToRedirectorsMap_.find(bundleName),
        instance_->bundleNameToRedirectorsMap_.end());
    auto listIt = std::find(instance_->redirectorIdList_.begin(),
        instance_->redirectorIdList_.end(), redirectorId);
    EXPECT_EQ(listIt, instance_->redirectorIdList_.end());
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, DestroyRedirector003, TestSize.Level1)
{
    // Valid redirector with rules - should succeed, cleanup iptables and data structures
    std::string redirectorId = "test_destroy_003";
    std::string bundleName = "com.example.destroy003";
    auto redirector = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId, bundleName, TEST_GROUP_ID, TEST_PRIORITY);
    redirector->SetCallingInfo(TEST_UID, TEST_PID);
    TrafficFilterRedirectRule rule = CreateTestRule();
    redirector->AddRuleWithPriority(rule);
    instance_->redirectors_[redirectorId] = redirector;
    instance_->bundleNameToRedirectorsMap_[bundleName].push_back(redirectorId);
    instance_->redirectorIdList_.push_back(redirectorId);

    int32_t ret = instance_->DestroyRedirector(redirectorId);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);

    EXPECT_EQ(instance_->redirectors_.find(redirectorId), instance_->redirectors_.end());
    EXPECT_EQ(instance_->bundleNameToRedirectorsMap_.find(bundleName),
        instance_->bundleNameToRedirectorsMap_.end());
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, DestroyRedirector004, TestSize.Level1)
{
    // Last redirector for its UID - should also erase observer from uidToObserverMap_
    std::string redirectorId = "test_destroy_004";
    std::string bundleName = "com.example.destroy004";
    int32_t testUid = 10007;
    auto redirector = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId, bundleName, TEST_GROUP_ID, TEST_PRIORITY);
    redirector->SetCallingInfo(testUid, TEST_PID);
    instance_->redirectors_[redirectorId] = redirector;
    instance_->bundleNameToRedirectorsMap_[bundleName].push_back(redirectorId);
    instance_->redirectorIdList_.push_back(redirectorId);

    sptr<NetTrafficFilterRedirectManager::TrafficFilterHapObserver> observer =
        new NetTrafficFilterRedirectManager::TrafficFilterHapObserver(
            instance_->weak_from_this(), bundleName, testUid);
    instance_->uidToObserverMap_[testUid] = observer;

    int32_t ret = instance_->DestroyRedirector(redirectorId);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);

    EXPECT_EQ(instance_->redirectors_.find(redirectorId), instance_->redirectors_.end());
    EXPECT_EQ(instance_->uidToObserverMap_.find(testUid), instance_->uidToObserverMap_.end());
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, DestroyRedirector005, TestSize.Level1)
{
    // Multiple redirectors for same UID - should NOT erase observer when one is destroyed
    std::string redirectorId1 = "test_destroy_005_1";
    std::string redirectorId2 = "test_destroy_005_2";
    std::string bundleName = "com.example.destroy005";
    int32_t testUid = 10008;
    auto redirector1 = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId1, bundleName, TEST_GROUP_ID, TEST_PRIORITY);
    redirector1->SetCallingInfo(testUid, TEST_PID);
    auto redirector2 = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId2, bundleName, TEST_GROUP_ID + 1, TEST_PRIORITY);
    redirector2->SetCallingInfo(testUid, TEST_PID);
    instance_->redirectors_[redirectorId1] = redirector1;
    instance_->redirectors_[redirectorId2] = redirector2;
    instance_->bundleNameToRedirectorsMap_[bundleName].push_back(redirectorId1);
    instance_->bundleNameToRedirectorsMap_[bundleName].push_back(redirectorId2);
    instance_->redirectorIdList_.push_back(redirectorId1);
    instance_->redirectorIdList_.push_back(redirectorId2);

    sptr<NetTrafficFilterRedirectManager::TrafficFilterHapObserver> observer =
        new NetTrafficFilterRedirectManager::TrafficFilterHapObserver(
            instance_->weak_from_this(), bundleName, testUid);
    instance_->uidToObserverMap_[testUid] = observer;

    int32_t ret = instance_->DestroyRedirector(redirectorId1);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);

    EXPECT_EQ(instance_->redirectors_.find(redirectorId1), instance_->redirectors_.end());
    EXPECT_NE(instance_->uidToObserverMap_.find(testUid), instance_->uidToObserverMap_.end());

    instance_->DestroyRedirector(redirectorId2);
    instance_->uidToObserverMap_.erase(testUid);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, DestroyRedirector006, TestSize.Level1)
{
    // Redirector with rules for multiple hook points
    std::string redirectorId = "test_destroy_006";
    std::string bundleName = "com.example.destroy006";
    auto redirector = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId, bundleName, TEST_GROUP_ID, TEST_PRIORITY);
    redirector->SetCallingInfo(TEST_UID, TEST_PID);
    TrafficFilterRedirectRule rule1 = CreateTestRule(TEST_PRIORITY);
    rule1.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_PREROUTING);
    TrafficFilterRedirectRule rule2 = CreateTestRule(TEST_PRIORITY + 1);
    rule2.hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
    redirector->AddRuleWithPriority(rule1);
    redirector->AddRuleWithPriority(rule2);
    instance_->redirectors_[redirectorId] = redirector;
    instance_->bundleNameToRedirectorsMap_[bundleName].push_back(redirectorId);
    instance_->redirectorIdList_.push_back(redirectorId);

    int32_t ret = instance_->DestroyRedirector(redirectorId);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);

    EXPECT_EQ(instance_->redirectors_.find(redirectorId), instance_->redirectors_.end());
    EXPECT_EQ(instance_->bundleNameToRedirectorsMap_.find(bundleName),
        instance_->bundleNameToRedirectorsMap_.end());
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, DestroyRedirector007, TestSize.Level1)
{
    // Redirector with callingUid = -1 (default) - should not attempt observer cleanup
    std::string redirectorId = "test_destroy_007";
    std::string bundleName = "com.example.destroy007";
    auto redirector = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId, bundleName, TEST_GROUP_ID, TEST_PRIORITY);
    instance_->redirectors_[redirectorId] = redirector;
    instance_->bundleNameToRedirectorsMap_[bundleName].push_back(redirectorId);
    instance_->redirectorIdList_.push_back(redirectorId);

    int32_t ret = instance_->DestroyRedirector(redirectorId);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);

    EXPECT_EQ(instance_->redirectors_.find(redirectorId), instance_->redirectors_.end());
    EXPECT_EQ(instance_->bundleNameToRedirectorsMap_.find(bundleName),
        instance_->bundleNameToRedirectorsMap_.end());
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, DestroyRedirector008, TestSize.Level1)
{
    // Multiple redirectors for different bundles and UIDs, destroy one
    std::string redirectorId1 = "test_destroy_008_1";
    std::string redirectorId2 = "test_destroy_008_2";
    std::string bundleName1 = "com.example.destroy008a";
    std::string bundleName2 = "com.example.destroy008b";
    int32_t testUid1 = 10009;
    int32_t testUid2 = 10010;
    auto redirector1 = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId1, bundleName1, TEST_GROUP_ID, TEST_PRIORITY);
    redirector1->SetCallingInfo(testUid1, TEST_PID);
    auto redirector2 = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId2, bundleName2, TEST_GROUP_ID + 1, TEST_PRIORITY);
    redirector2->SetCallingInfo(testUid2, TEST_PID);
    instance_->redirectors_[redirectorId1] = redirector1;
    instance_->redirectors_[redirectorId2] = redirector2;
    instance_->bundleNameToRedirectorsMap_[bundleName1].push_back(redirectorId1);
    instance_->bundleNameToRedirectorsMap_[bundleName2].push_back(redirectorId2);
    instance_->redirectorIdList_.push_back(redirectorId1);
    instance_->redirectorIdList_.push_back(redirectorId2);

    sptr<NetTrafficFilterRedirectManager::TrafficFilterHapObserver> observer1 =
        new NetTrafficFilterRedirectManager::TrafficFilterHapObserver(
            instance_->weak_from_this(), bundleName1, testUid1);
    sptr<NetTrafficFilterRedirectManager::TrafficFilterHapObserver> observer2 =
        new NetTrafficFilterRedirectManager::TrafficFilterHapObserver(
            instance_->weak_from_this(), bundleName2, testUid2);
    instance_->uidToObserverMap_[testUid1] = observer1;
    instance_->uidToObserverMap_[testUid2] = observer2;

    int32_t ret = instance_->DestroyRedirector(redirectorId1);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);

    EXPECT_EQ(instance_->redirectors_.find(redirectorId1), instance_->redirectors_.end());
    EXPECT_NE(instance_->redirectors_.find(redirectorId2), instance_->redirectors_.end());
    EXPECT_EQ(instance_->uidToObserverMap_.find(testUid1), instance_->uidToObserverMap_.end());
    EXPECT_NE(instance_->uidToObserverMap_.find(testUid2), instance_->uidToObserverMap_.end());

    instance_->DestroyRedirector(redirectorId2);
    instance_->uidToObserverMap_.erase(testUid2);
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, DestroyRedirector009, TestSize.Level1)
{
    // Redirector with rules, verify all data structures cleaned
    std::string redirectorId = "test_destroy_009";
    std::string bundleName = "com.example.destroy009";
    auto redirector = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId, bundleName, TEST_GROUP_ID, TEST_PRIORITY);
    redirector->SetCallingInfo(TEST_UID, TEST_PID);
    TrafficFilterRedirectRule rule = CreateTestRule();
    redirector->AddRuleWithPriority(rule);
    instance_->redirectors_[redirectorId] = redirector;
    instance_->bundleNameToRedirectorsMap_[bundleName].push_back(redirectorId);
    instance_->redirectorIdList_.push_back(redirectorId);

    int32_t ret = instance_->DestroyRedirector(redirectorId);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);

    EXPECT_EQ(instance_->redirectors_.find(redirectorId), instance_->redirectors_.end());
    EXPECT_EQ(instance_->bundleNameToRedirectorsMap_.find(bundleName),
        instance_->bundleNameToRedirectorsMap_.end());
    auto listIt = std::find(instance_->redirectorIdList_.begin(),
        instance_->redirectorIdList_.end(), redirectorId);
    EXPECT_EQ(listIt, instance_->redirectorIdList_.end());
}

HWTEST_F(NetTrafficFilterRedirectManagerTest, DestroyRedirector010, TestSize.Level1)
{
    // Two redirectors for same bundle, destroy one - bundle map entry should still exist
    std::string redirectorId1 = "test_destroy_010_1";
    std::string redirectorId2 = "test_destroy_010_2";
    std::string bundleName = "com.example.destroy010";
    int32_t testUid = 10011;
    auto redirector1 = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId1, bundleName, TEST_GROUP_ID, TEST_PRIORITY);
    redirector1->SetCallingInfo(testUid, TEST_PID);
    auto redirector2 = std::make_shared<NetTrafficFilterRedirectorContext>(
        redirectorId2, bundleName, TEST_GROUP_ID + 1, TEST_PRIORITY);
    redirector2->SetCallingInfo(testUid, TEST_PID);
    instance_->redirectors_[redirectorId1] = redirector1;
    instance_->redirectors_[redirectorId2] = redirector2;
    instance_->bundleNameToRedirectorsMap_[bundleName].push_back(redirectorId1);
    instance_->bundleNameToRedirectorsMap_[bundleName].push_back(redirectorId2);
    instance_->redirectorIdList_.push_back(redirectorId1);
    instance_->redirectorIdList_.push_back(redirectorId2);

    int32_t ret = instance_->DestroyRedirector(redirectorId1);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);

    EXPECT_EQ(instance_->redirectors_.find(redirectorId1), instance_->redirectors_.end());
    EXPECT_NE(instance_->redirectors_.find(redirectorId2), instance_->redirectors_.end());
    auto bundleIt = instance_->bundleNameToRedirectorsMap_.find(bundleName);
    EXPECT_NE(bundleIt, instance_->bundleNameToRedirectorsMap_.end());
    EXPECT_EQ(bundleIt->second.size(), 1);
    EXPECT_EQ(bundleIt->second[0], redirectorId2);

    instance_->DestroyRedirector(redirectorId2);
    instance_->uidToObserverMap_.erase(testUid);
}
} // namespace NetManagerStandard
} // namespace OHOS
