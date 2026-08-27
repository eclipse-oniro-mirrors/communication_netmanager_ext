/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#define private public
#define protected public

#include "nettrafficfilter_packetrule_manager.h"
#include "netfirewall_common.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;

constexpr uint32_t TEST_PRIORITY = 100;
constexpr int32_t TEST_HOOK_OUTPUT =
    static_cast<int32_t>(TrafficFilterHookPoint::HOOK_OUTPUT);
constexpr int32_t TEST_HOOK_INPUT =
    static_cast<int32_t>(TrafficFilterHookPoint::HOOK_INPUT);
constexpr int32_t TEST_HOOK_FORWARD =
    static_cast<int32_t>(TrafficFilterHookPoint::HOOK_FORWARD);
constexpr int32_t TEST_HOOK_PREROUTING =
    static_cast<int32_t>(TrafficFilterHookPoint::HOOK_PREROUTING);
constexpr int32_t TEST_HOOK_POSTROUTING =
    static_cast<int32_t>(TrafficFilterHookPoint::HOOK_POSTROUTING);
constexpr size_t TEST_HOOK_POINT_COUNT = 3;

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

sptr<TrafficFilterPacketRule> CreateValidRule(int32_t hookPoint)
{
    sptr<TrafficFilterPacketRule> rule = new (std::nothrow) TrafficFilterPacketRule();
    if (rule == nullptr) {
        return nullptr;
    }
    rule->priority_ = TEST_PRIORITY;
    rule->hookPoint_ = hookPoint;
    rule->srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule->dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule->srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule->dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule->uidStart_ = static_cast<uint32_t>(-1);
    rule->uidEnd_ = static_cast<uint32_t>(-1);
    return rule;
}
} // namespace

class NetTrafficFilterPacketRuleManagerTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp()
    {
        auto& mgr = NetTrafficFilterPacketRuleManager::GetInstance();
        mgr.queueNumToRules_.clear();
        mgr.queueNumToRuleCtx_.clear();
    }
    void TearDown() {}
};

HWTEST_F(NetTrafficFilterPacketRuleManagerTest, GetInstance001, TestSize.Level1)
{
    auto& instance = NetTrafficFilterPacketRuleManager::GetInstance();
    EXPECT_NE(&instance, nullptr);
}

HWTEST_F(NetTrafficFilterPacketRuleManagerTest, AddPacketRuleNullptr, TestSize.Level1)
{
    auto& instance = NetTrafficFilterPacketRuleManager::GetInstance();
    int32_t ret = instance.AddPacketRule("com.example:5", nullptr);
    EXPECT_EQ(ret, TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(NetTrafficFilterPacketRuleManagerTest, AddPacketRuleInvalidSrcIp, TestSize.Level1)
{
    auto& instance = NetTrafficFilterPacketRuleManager::GetInstance();
    sptr<TrafficFilterPacketRule> rule = CreateValidRule(TEST_HOOK_OUTPUT);
    ASSERT_NE(rule, nullptr);
    rule->srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY) + 100;

    int32_t ret = instance.AddPacketRule("com.example:5", rule);
    EXPECT_EQ(ret, TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(NetTrafficFilterPacketRuleManagerTest, AddPacketRuleInvalidDstIp, TestSize.Level1)
{
    auto& instance = NetTrafficFilterPacketRuleManager::GetInstance();
    sptr<TrafficFilterPacketRule> rule = CreateValidRule(TEST_HOOK_OUTPUT);
    ASSERT_NE(rule, nullptr);
    rule->dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY) + 100;

    int32_t ret = instance.AddPacketRule("com.example:5", rule);
    EXPECT_EQ(ret, TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(NetTrafficFilterPacketRuleManagerTest, AddPacketRuleInvalidSrcPort, TestSize.Level1)
{
    auto& instance = NetTrafficFilterPacketRuleManager::GetInstance();
    sptr<TrafficFilterPacketRule> rule = CreateValidRule(TEST_HOOK_OUTPUT);
    ASSERT_NE(rule, nullptr);
    rule->srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY) + 100;

    int32_t ret = instance.AddPacketRule("com.example:5", rule);
    EXPECT_EQ(ret, TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(NetTrafficFilterPacketRuleManagerTest, AddPacketRuleInvalidDstPort, TestSize.Level1)
{
    auto& instance = NetTrafficFilterPacketRuleManager::GetInstance();
    sptr<TrafficFilterPacketRule> rule = CreateValidRule(TEST_HOOK_OUTPUT);
    ASSERT_NE(rule, nullptr);
    rule->dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY) + 100;

    int32_t ret = instance.AddPacketRule("com.example:5", rule);
    EXPECT_EQ(ret, TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(NetTrafficFilterPacketRuleManagerTest, AddPacketRuleInvalidControllerId, TestSize.Level1)
{
    auto& instance = NetTrafficFilterPacketRuleManager::GetInstance();
    sptr<TrafficFilterPacketRule> rule = CreateValidRule(TEST_HOOK_OUTPUT);
    ASSERT_NE(rule, nullptr);

    int32_t ret = instance.AddPacketRule("invalid", rule);
    EXPECT_EQ(ret, TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(NetTrafficFilterPacketRuleManagerTest, AddPacketRuleInvalidHookPoint, TestSize.Level1)
{
    auto& instance = NetTrafficFilterPacketRuleManager::GetInstance();
    sptr<TrafficFilterPacketRule> rule = CreateValidRule(9999);
    ASSERT_NE(rule, nullptr);

    int32_t ret = instance.AddPacketRule("com.example:5", rule);
    EXPECT_EQ(ret, TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(NetTrafficFilterPacketRuleManagerTest, AddPacketRuleControllerWithQueueNumZero, TestSize.Level1)
{
    auto& instance = NetTrafficFilterPacketRuleManager::GetInstance();
    sptr<TrafficFilterPacketRule> rule = CreateValidRule(TEST_HOOK_OUTPUT);
    ASSERT_NE(rule, nullptr);

    int32_t ret = instance.AddPacketRule("com.example:0", rule);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(NetTrafficFilterPacketRuleManagerTest, ParseAndValidateControllerIdMissingColon, TestSize.Level1)
{
    auto& instance = NetTrafficFilterPacketRuleManager::GetInstance();
    int32_t queueNum = 0;
    bool result = instance.ParseAndValidateControllerId("com.example.app", queueNum);
    EXPECT_FALSE(result);
}

HWTEST_F(NetTrafficFilterPacketRuleManagerTest, ParseAndValidateControllerIdQueueNumZero, TestSize.Level1)
{
    auto& instance = NetTrafficFilterPacketRuleManager::GetInstance();
    int32_t queueNum = 0;
    bool result = instance.ParseAndValidateControllerId("com.example:0", queueNum);
    EXPECT_TRUE(result);
}

HWTEST_F(NetTrafficFilterPacketRuleManagerTest, ParseAndValidateControllerIdValid, TestSize.Level1)
{
    auto& instance = NetTrafficFilterPacketRuleManager::GetInstance();
    int32_t queueNum = 0;
    bool result = instance.ParseAndValidateControllerId("com.example:42", queueNum);
    EXPECT_TRUE(result);
    EXPECT_EQ(queueNum, 42);
}

HWTEST_F(NetTrafficFilterPacketRuleManagerTest, ParseAndValidateControllerIdExtraColon, TestSize.Level1)
{
    auto& instance = NetTrafficFilterPacketRuleManager::GetInstance();
    int32_t queueNum = 0;
    bool result = instance.ParseAndValidateControllerId("com.example:42:1001:extra", queueNum);
    EXPECT_FALSE(result);
    EXPECT_EQ(queueNum, 42);
}

HWTEST_F(NetTrafficFilterPacketRuleManagerTest, ClearPacketRuleInvalidControllerId, TestSize.Level1)
{
    auto& instance = NetTrafficFilterPacketRuleManager::GetInstance();
    QueueInfo info{};
    info.queueNum = 0;
    int32_t ret = instance.ClearPacketRule(info);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
    EXPECT_TRUE(instance.queueNumToRules_.empty());
    EXPECT_TRUE(instance.queueNumToRuleCtx_.empty());
}

HWTEST_F(NetTrafficFilterPacketRuleManagerTest, ClearPacketRuleEmpty, TestSize.Level1)
{
    auto& instance = NetTrafficFilterPacketRuleManager::GetInstance();
    QueueInfo info{};
    int32_t ret = instance.ClearPacketRule(info);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
    EXPECT_TRUE(instance.queueNumToRules_.empty());
    EXPECT_TRUE(instance.queueNumToRuleCtx_.empty());
}

HWTEST_F(NetTrafficFilterPacketRuleManagerTest, ClearPacketRuleQueueNotFound, TestSize.Level1)
{
    auto& instance = NetTrafficFilterPacketRuleManager::GetInstance();
    QueueInfo info{};
    info.queueNum = 42;
    int32_t ret = instance.ClearPacketRule(info);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
    EXPECT_TRUE(instance.queueNumToRules_.empty());
    EXPECT_TRUE(instance.queueNumToRuleCtx_.empty());
}

HWTEST_F(NetTrafficFilterPacketRuleManagerTest, PauseAllRulesEmpty, TestSize.Level1)
{
    auto& instance = NetTrafficFilterPacketRuleManager::GetInstance();
    int32_t ret = instance.PauseAllRules();
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(NetTrafficFilterPacketRuleManagerTest, ResumeAllRulesEmpty, TestSize.Level1)
{
    auto& instance = NetTrafficFilterPacketRuleManager::GetInstance();
    int32_t ret = instance.ResumeAllRules();
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(NetTrafficFilterPacketRuleManagerTest, PauseAllRulesWithCtx, TestSize.Level1)
{
    auto& instance = NetTrafficFilterPacketRuleManager::GetInstance();
    FilterRuleCtx ctx;
    ctx.priority = TEST_PRIORITY;
    ctx.chainNameIn = "chainIn";
    ctx.chainNameOut = "chainOut";
    ctx.chainNameFwd = "chainFwd";
    instance.queueNumToRuleCtx_[1] = ctx;

    int32_t ret = instance.PauseAllRules();
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(NetTrafficFilterPacketRuleManagerTest, ResumeAllRulesWithCtxNoRules, TestSize.Level1)
{
    auto& instance = NetTrafficFilterPacketRuleManager::GetInstance();
    FilterRuleCtx ctx;
    ctx.priority = TEST_PRIORITY;
    ctx.chainNameIn = "chainIn";
    ctx.chainNameOut = "chainOut";
    ctx.chainNameFwd = "chainFwd";
    instance.queueNumToRuleCtx_[1] = ctx;

    int32_t ret = instance.ResumeAllRules();
    EXPECT_EQ(ret, TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(NetTrafficFilterPacketRuleManagerTest, ResumeAllRulesWithRulesAndNeedV6, TestSize.Level1)
{
    auto& instance = NetTrafficFilterPacketRuleManager::GetInstance();
    FilterRuleCtx ctx;
    ctx.priority = TEST_PRIORITY;
    ctx.chainNameIn = "chainIn";
    ctx.chainNameOut = "chainOut";
    ctx.chainNameFwd = "chainFwd";
    instance.queueNumToRuleCtx_[1] = ctx;

    TrafficFilterPacketRule rule;
    rule.hookPoint_ = TEST_HOOK_OUTPUT;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE);
    SetupIPv6Address(rule.dstIp_.single_, "2001:db8::1");
    instance.queueNumToRules_[1].output.push_back(rule);

    int32_t ret = instance.ResumeAllRules();
    EXPECT_EQ(ret, TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(NetTrafficFilterPacketRuleManagerTest, ClearPacketRuleSuccess, TestSize.Level1)
{
    auto& instance = NetTrafficFilterPacketRuleManager::GetInstance();
    FilterRuleCtx ctx;
    ctx.priority = TEST_PRIORITY;
    ctx.chainNameIn = "chainIn";
    ctx.chainNameOut = "chainOut";
    ctx.chainNameFwd = "chainFwd";
    instance.queueNumToRuleCtx_[1] = ctx;

    sptr<TrafficFilterPacketRule> inputRule = CreateValidRule(TEST_HOOK_INPUT);
    ASSERT_NE(inputRule, nullptr);
    instance.queueNumToRules_[1].input.push_back(*inputRule);

    sptr<TrafficFilterPacketRule> outputRule = CreateValidRule(TEST_HOOK_OUTPUT);
    ASSERT_NE(outputRule, nullptr);
    instance.queueNumToRules_[1].output.push_back(*outputRule);

    sptr<TrafficFilterPacketRule> forwardRule = CreateValidRule(TEST_HOOK_FORWARD);
    ASSERT_NE(forwardRule, nullptr);
    instance.queueNumToRules_[1].forward.push_back(*forwardRule);

    QueueInfo info{};
    info.queueNum = 1;
    info.chainNameIn = "chainIn";
    info.chainNameOut = "chainOut";
    info.chainNameFwd = "chainFwd";

    int32_t ret = instance.ClearPacketRule(info);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
    EXPECT_TRUE(instance.queueNumToRules_.find(1) == instance.queueNumToRules_.end());
    EXPECT_TRUE(instance.queueNumToRuleCtx_.find(1) == instance.queueNumToRuleCtx_.end());
}

HWTEST_F(NetTrafficFilterPacketRuleManagerTest, CollectResumeEntriesWithEmptyChain, TestSize.Level1)
{
    auto& instance = NetTrafficFilterPacketRuleManager::GetInstance();
    FilterRuleCtx ctx;
    ctx.priority = TEST_PRIORITY;
    ctx.chainNameIn = "";
    ctx.chainNameOut = "";
    ctx.chainNameFwd = "";
    instance.queueNumToRuleCtx_[1] = ctx;

    auto entries = instance.CollectResumeEntries();
    EXPECT_TRUE(entries.empty());
}

HWTEST_F(NetTrafficFilterPacketRuleManagerTest, CollectResumeEntriesNeedV4, TestSize.Level1)
{
    auto& instance = NetTrafficFilterPacketRuleManager::GetInstance();
    FilterRuleCtx ctx;
    ctx.priority = TEST_PRIORITY;
    ctx.chainNameIn = "chainIn";
    ctx.chainNameOut = "chainOut";
    ctx.chainNameFwd = "chainFwd";
    instance.queueNumToRuleCtx_[1] = ctx;

    TrafficFilterPacketRule rule;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    instance.queueNumToRules_[1].output.push_back(rule);

    auto entries = instance.CollectResumeEntries();
    EXPECT_EQ(entries.size(), TEST_HOOK_POINT_COUNT);
    bool foundV6 = false;
    for (const auto& entry : entries) {
        if (entry.needV6) {
            foundV6 = true;
            break;
        }
    }
    EXPECT_TRUE(foundV6);
}

HWTEST_F(NetTrafficFilterPacketRuleManagerTest, CollectResumeEntriesNeedV6, TestSize.Level1)
{
    auto& instance = NetTrafficFilterPacketRuleManager::GetInstance();
    FilterRuleCtx ctx;
    ctx.priority = TEST_PRIORITY;
    ctx.chainNameIn = "chainIn";
    ctx.chainNameOut = "chainOut";
    ctx.chainNameFwd = "chainFwd";
    instance.queueNumToRuleCtx_[1] = ctx;

    TrafficFilterPacketRule rule;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE);
    SetupIPv6Address(rule.dstIp_.single_, "2001:db8::1");
    instance.queueNumToRules_[1].output.push_back(rule);

    auto entries = instance.CollectResumeEntries();
    bool foundV6 = false;
    for (const auto& entry : entries) {
        if (entry.hookPoint == TEST_HOOK_OUTPUT && entry.needV6) {
            foundV6 = true;
            break;
        }
    }
    EXPECT_TRUE(foundV6);
}

HWTEST_F(NetTrafficFilterPacketRuleManagerTest, ApplyRulesForHookPointFlushFailure, TestSize.Level1)
{
    auto& instance = NetTrafficFilterPacketRuleManager::GetInstance();
    int32_t ret = instance.ApplyRulesForHookPoint(1, TEST_HOOK_OUTPUT, "nonexistent",
        TrafficFilterIPFamily::IP_FAMILY_V4);
    EXPECT_NE(ret, FIREWALL_SUCCESS);
}

HWTEST_F(NetTrafficFilterPacketRuleManagerTest, ExecuteRulesForIpFamilySkipV6OnlyForV4, TestSize.Level1)
{
    auto& instance = NetTrafficFilterPacketRuleManager::GetInstance();
    TrafficFilterPacketRule rule;
    rule.protocol_ = NETTRAFFICFILTER_PROTO_TCP;
    rule.srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule.dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE);
    SetupIPv6Address(rule.dstIp_.single_, "2001:db8::1");
    rule.srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule.uidStart_ = static_cast<uint32_t>(-1);
    rule.uidEnd_ = static_cast<uint32_t>(-1);

    std::vector<TrafficFilterPacketRule> rules = {rule};
    int32_t ret = instance.ExecuteRulesForIpFamily(rules, "chain", 1, TrafficFilterIPFamily::IP_FAMILY_V4);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

} // namespace NetManagerStandard
} // namespace OHOS
