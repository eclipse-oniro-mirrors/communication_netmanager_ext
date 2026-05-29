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

#include "gtest/gtest-message.h"
#include "gtest/gtest-test-part.h"
#include "gtest/hwext/gtest-ext.h"
#include "gtest/hwext/gtest-tag.h"

#include "nettrafficfilter_redirector_context.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
const uint16_t RULE_PORT = 8080;
const uint32_t TEST_NFQUEUE_LEN = 1024;
const uint32_t TEST_UID = 1000;
const uint32_t TEST_UID_END = 2000;
const int32_t ADDR_BIT1 = 1;
const int32_t ADDR_BIT2 = 2;
const int32_t ADDR_BIT3 = 3;
const uint8_t ADDR1 = 127;

TrafficFilterRedirectRule CreateTestRule(uint32_t priority, TrafficFilterHookPoint hookPoint,
    uint8_t protocol = 6)
{
    TrafficFilterRedirectRule rule;
    rule.priority_ = priority;
    rule.hookPoint_ = static_cast<int32_t>(hookPoint);
    rule.protocol_ = protocol;
    rule.uidStart_ = TEST_UID;
    rule.uidEnd_ = TEST_UID_END;
    rule.proxyIp_.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4);
    rule.proxyIp_.addr_[0] = ADDR1;
    rule.proxyIp_.addr_[ADDR_BIT1] = 0;
    rule.proxyIp_.addr_[ADDR_BIT2] = 0;
    rule.proxyIp_.addr_[ADDR_BIT3] = 1;
    rule.proxyPort_ = RULE_PORT;
    return rule;
}
}

class NetTrafficFilterRedirectorContextTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

HWTEST_F(NetTrafficFilterRedirectorContextTest, Constructor001, TestSize.Level1)
{
    std::string redirectorId = "test_redirector_001";
    std::string bundleName = "com.example.test";
    uint32_t groupId = 1001;
    uint32_t priority = 100;

    NetTrafficFilterRedirectorContext context(redirectorId, bundleName, groupId, priority);

    EXPECT_EQ(context.GetRedirectorId(), redirectorId);
    EXPECT_EQ(context.GetBundleName(), bundleName);
    EXPECT_EQ(context.GetGroupId(), groupId);
    EXPECT_EQ(context.GetPriority(), priority);
    EXPECT_FALSE(context.IsPaused());
    EXPECT_EQ(context.GetCallingUid(), -1);
    EXPECT_EQ(context.GetCallingPid(), -1);
}

HWTEST_F(NetTrafficFilterRedirectorContextTest, HasRules001, TestSize.Level0)
{
    NetTrafficFilterRedirectorContext context("test_id", "com.test.app", 1001, 100);

    EXPECT_FALSE(context.HasRules());
}

HWTEST_F(NetTrafficFilterRedirectorContextTest, HasRules002, TestSize.Level1)
{
    NetTrafficFilterRedirectorContext context("test_id", "com.test.app", 1001, 100);

    TrafficFilterRedirectRule rule = CreateTestRule(100, TrafficFilterHookPoint::HOOK_PREROUTING);
    context.AddRuleWithPriority(rule);

    EXPECT_TRUE(context.HasRules());
}

HWTEST_F(NetTrafficFilterRedirectorContextTest, HasRules003, TestSize.Level1)
{
    NetTrafficFilterRedirectorContext context("test_id", "com.test.app", 1001, 100);

    TrafficFilterRedirectRule rule1 = CreateTestRule(100, TrafficFilterHookPoint::HOOK_PREROUTING);
    TrafficFilterRedirectRule rule2 = CreateTestRule(200, TrafficFilterHookPoint::HOOK_OUTPUT);
    context.AddRuleWithPriority(rule1);
    context.AddRuleWithPriority(rule2);

    EXPECT_TRUE(context.HasRules());

    context.ClearRules();

    EXPECT_FALSE(context.HasRules());
}

HWTEST_F(NetTrafficFilterRedirectorContextTest, GetUsedHookPoints001, TestSize.Level0)
{
    NetTrafficFilterRedirectorContext context("test_id", "com.test.app", 1001, 100);

    std::set<TrafficFilterHookPoint> usedHookPoints = context.GetUsedHookPoints();

    EXPECT_TRUE(usedHookPoints.empty());
}

HWTEST_F(NetTrafficFilterRedirectorContextTest, GetUsedHookPoints002, TestSize.Level1)
{
    NetTrafficFilterRedirectorContext context("test_id", "com.test.app", 1001, 100);

    TrafficFilterRedirectRule rule1 = CreateTestRule(100, TrafficFilterHookPoint::HOOK_PREROUTING);
    context.AddRuleWithPriority(rule1);

    std::set<TrafficFilterHookPoint> usedHookPoints = context.GetUsedHookPoints();

    EXPECT_EQ(usedHookPoints.size(), 1);
    EXPECT_NE(usedHookPoints.find(TrafficFilterHookPoint::HOOK_PREROUTING), usedHookPoints.end());
}

HWTEST_F(NetTrafficFilterRedirectorContextTest, GetUsedHookPoints003, TestSize.Level1)
{
    NetTrafficFilterRedirectorContext context("test_id", "com.test.app", 1001, 100);

    TrafficFilterRedirectRule rule1 = CreateTestRule(100, TrafficFilterHookPoint::HOOK_PREROUTING);
    TrafficFilterRedirectRule rule2 = CreateTestRule(200, TrafficFilterHookPoint::HOOK_PREROUTING);
    TrafficFilterRedirectRule rule3 = CreateTestRule(150, TrafficFilterHookPoint::HOOK_OUTPUT);
    context.AddRuleWithPriority(rule1);
    context.AddRuleWithPriority(rule2);
    context.AddRuleWithPriority(rule3);

    std::set<TrafficFilterHookPoint> usedHookPoints = context.GetUsedHookPoints();

    EXPECT_EQ(usedHookPoints.size(), 2);
    EXPECT_NE(usedHookPoints.find(TrafficFilterHookPoint::HOOK_PREROUTING), usedHookPoints.end());
    EXPECT_NE(usedHookPoints.find(TrafficFilterHookPoint::HOOK_OUTPUT), usedHookPoints.end());
}

HWTEST_F(NetTrafficFilterRedirectorContextTest, AddRuleWithPriority001, TestSize.Level1)
{
    NetTrafficFilterRedirectorContext context("test_id", "com.test.app", 1001, 100);

    TrafficFilterRedirectRule rule = CreateTestRule(100, TrafficFilterHookPoint::HOOK_PREROUTING);

    int32_t ret = context.AddRuleWithPriority(rule);

    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetTrafficFilterRedirectorContextTest, AddRuleWithPriority002, TestSize.Level1)
{
    NetTrafficFilterRedirectorContext context("test_id", "com.test.app", 1001, 100);

    TrafficFilterRedirectRule rule1 = CreateTestRule(200, TrafficFilterHookPoint::HOOK_PREROUTING);
    TrafficFilterRedirectRule rule2 = CreateTestRule(100, TrafficFilterHookPoint::HOOK_PREROUTING);
    TrafficFilterRedirectRule rule3 = CreateTestRule(150, TrafficFilterHookPoint::HOOK_PREROUTING);

    context.AddRuleWithPriority(rule1);
    context.AddRuleWithPriority(rule2);
    context.AddRuleWithPriority(rule3);

    std::vector<TrafficFilterRedirectRule> sortedRules = context.GetSortedRules();

    EXPECT_EQ(sortedRules.size(), 3);
    EXPECT_EQ(sortedRules[0].priority_, 100);
    EXPECT_EQ(sortedRules[1].priority_, 150);
    EXPECT_EQ(sortedRules[2].priority_, 200);
}

HWTEST_F(NetTrafficFilterRedirectorContextTest, AddRuleWithPriority003, TestSize.Level1)
{
    NetTrafficFilterRedirectorContext context("test_id", "com.test.app", 1001, 100);

    for (int i = 10; i > 0; i--) {
        TrafficFilterRedirectRule rule = CreateTestRule(i * 10, TrafficFilterHookPoint::HOOK_PREROUTING);
        context.AddRuleWithPriority(rule);
    }

    std::vector<TrafficFilterRedirectRule> sortedRules = context.GetSortedRules();

    EXPECT_EQ(sortedRules.size(), 10);
    for (size_t i = 0; i < sortedRules.size() - 1; i++) {
        EXPECT_LT(sortedRules[i].priority_, sortedRules[i + 1].priority_);
    }
}

HWTEST_F(NetTrafficFilterRedirectorContextTest, AddRuleWithPriority004, TestSize.Level1)
{
    NetTrafficFilterRedirectorContext context("test_id", "com.test.app", 1001, 100);

    TrafficFilterRedirectRule rule1 = CreateTestRule(100, TrafficFilterHookPoint::HOOK_PREROUTING, 6);
    TrafficFilterRedirectRule rule2 = CreateTestRule(200, TrafficFilterHookPoint::HOOK_OUTPUT, 17);
    TrafficFilterRedirectRule rule3 = CreateTestRule(150, TrafficFilterHookPoint::HOOK_OUTPUT, 6);

    context.AddRuleWithPriority(rule1);
    context.AddRuleWithPriority(rule2);
    context.AddRuleWithPriority(rule3);

    std::vector<TrafficFilterRedirectRule> sortedRules = context.GetSortedRules();

    EXPECT_EQ(sortedRules.size(), 3);
    EXPECT_EQ(sortedRules[0].priority_, 100);
    EXPECT_EQ(sortedRules[1].priority_, 150);
    EXPECT_EQ(sortedRules[2].priority_, 200);
}

HWTEST_F(NetTrafficFilterRedirectorContextTest, ClearRules001, TestSize.Level0)
{
    NetTrafficFilterRedirectorContext context("test_id", "com.test.app", 1001, 100);

    int32_t ret = context.ClearRules();

    EXPECT_EQ(ret, 0);
    EXPECT_FALSE(context.HasRules());
}

HWTEST_F(NetTrafficFilterRedirectorContextTest, ClearRules002, TestSize.Level1)
{
    NetTrafficFilterRedirectorContext context("test_id", "com.test.app", 1001, 100);

    TrafficFilterRedirectRule rule1 = CreateTestRule(100, TrafficFilterHookPoint::HOOK_PREROUTING);
    TrafficFilterRedirectRule rule2 = CreateTestRule(200, TrafficFilterHookPoint::HOOK_OUTPUT);
    context.AddRuleWithPriority(rule1);
    context.AddRuleWithPriority(rule2);

    int32_t ret = context.ClearRules();

    EXPECT_EQ(ret, 0);
    EXPECT_FALSE(context.HasRules());

    std::vector<TrafficFilterRedirectRule> rules = context.GetRules();
    EXPECT_TRUE(rules.empty());
}

HWTEST_F(NetTrafficFilterRedirectorContextTest, ClearRules003, TestSize.Level1)
{
    NetTrafficFilterRedirectorContext context("test_id", "com.test.app", 1001, 100);

    for (int i = 0; i < 10; i++) {
        TrafficFilterRedirectRule rule = CreateTestRule(i * 100, TrafficFilterHookPoint::HOOK_PREROUTING);
        context.AddRuleWithPriority(rule);
    }

    context.ClearRules();

    std::set<TrafficFilterHookPoint> usedHookPoints = context.GetUsedHookPoints();
    EXPECT_TRUE(usedHookPoints.empty());
}

HWTEST_F(NetTrafficFilterRedirectorContextTest, ClearRules004, TestSize.Level1)
{
    NetTrafficFilterRedirectorContext context("test_id", "com.test.app", 1001, 100);

    TrafficFilterRedirectRule rule1 = CreateTestRule(100, TrafficFilterHookPoint::HOOK_PREROUTING);
    context.AddRuleWithPriority(rule1);

    context.ClearRules();
    context.ClearRules();

    EXPECT_FALSE(context.HasRules());
}

HWTEST_F(NetTrafficFilterRedirectorContextTest, GetRules001, TestSize.Level0)
{
    NetTrafficFilterRedirectorContext context("test_id", "com.test.app", 1001, 100);

    std::vector<TrafficFilterRedirectRule> rules = context.GetRules();

    EXPECT_TRUE(rules.empty());
}

HWTEST_F(NetTrafficFilterRedirectorContextTest, GetRules002, TestSize.Level1)
{
    NetTrafficFilterRedirectorContext context("test_id", "com.test.app", 1001, 100);

    TrafficFilterRedirectRule rule1 = CreateTestRule(200, TrafficFilterHookPoint::HOOK_PREROUTING);
    TrafficFilterRedirectRule rule2 = CreateTestRule(100, TrafficFilterHookPoint::HOOK_OUTPUT);
    context.AddRuleWithPriority(rule1);
    context.AddRuleWithPriority(rule2);

    std::vector<TrafficFilterRedirectRule> rules = context.GetRules();

    EXPECT_EQ(rules.size(), 2);
}

HWTEST_F(NetTrafficFilterRedirectorContextTest, GetSortedRules001, TestSize.Level0)
{
    NetTrafficFilterRedirectorContext context("test_id", "com.test.app", 1001, 100);

    std::vector<TrafficFilterRedirectRule> sortedRules = context.GetSortedRules();

    EXPECT_TRUE(sortedRules.empty());
}

HWTEST_F(NetTrafficFilterRedirectorContextTest, GetSortedRules002, TestSize.Level1)
{
    NetTrafficFilterRedirectorContext context("test_id", "com.test.app", 1001, 100);

    TrafficFilterRedirectRule rule1 = CreateTestRule(300, TrafficFilterHookPoint::HOOK_PREROUTING);
    TrafficFilterRedirectRule rule2 = CreateTestRule(100, TrafficFilterHookPoint::HOOK_OUTPUT);
    TrafficFilterRedirectRule rule3 = CreateTestRule(200, TrafficFilterHookPoint::HOOK_PREROUTING);

    context.AddRuleWithPriority(rule1);
    context.AddRuleWithPriority(rule2);
    context.AddRuleWithPriority(rule3);

    std::vector<TrafficFilterRedirectRule> sortedRules = context.GetSortedRules();

    EXPECT_EQ(sortedRules.size(), 3);
    EXPECT_EQ(sortedRules[0].priority_, 100);
    EXPECT_EQ(sortedRules[1].priority_, 200);
    EXPECT_EQ(sortedRules[2].priority_, 300);
}

HWTEST_F(NetTrafficFilterRedirectorContextTest, GetSortedRules003, TestSize.Level1)
{
    NetTrafficFilterRedirectorContext context("test_id", "com.test.app", 1001, 100);

    std::vector<TrafficFilterRedirectRule> rules = context.GetRules();
    std::vector<TrafficFilterRedirectRule> sortedRules1 = context.GetSortedRules();
    std::vector<TrafficFilterRedirectRule> sortedRules2 = context.GetSortedRules();

    EXPECT_EQ(sortedRules1.size(), sortedRules2.size());
}

HWTEST_F(NetTrafficFilterRedirectorContextTest, GetSortedRules004, TestSize.Level2)
{
    NetTrafficFilterRedirectorContext context("test_id", "com.test.app", 1001, 100);

    for (int i = 0; i < 100; i++) {
        TrafficFilterRedirectRule rule = CreateTestRule(1000 - i * 10, TrafficFilterHookPoint::HOOK_PREROUTING);
        context.AddRuleWithPriority(rule);
    }

    std::vector<TrafficFilterRedirectRule> sortedRules = context.GetSortedRules();

    EXPECT_EQ(sortedRules.size(), 100);
    for (size_t i = 0; i < sortedRules.size() - 1; i++) {
        EXPECT_LT(sortedRules[i].priority_, sortedRules[i + 1].priority_);
    }
}

HWTEST_F(NetTrafficFilterRedirectorContextTest, RestoreRules001, TestSize.Level0)
{
    NetTrafficFilterRedirectorContext context("test_id", "com.test.app", 1001, 100);

    std::vector<TrafficFilterRedirectRule> rules;

    int32_t ret = context.RestoreRules(rules);

    EXPECT_EQ(ret, 0);
    EXPECT_FALSE(context.HasRules());
}

HWTEST_F(NetTrafficFilterRedirectorContextTest, RestoreRules002, TestSize.Level1)
{
    NetTrafficFilterRedirectorContext context("test_id", "com.test.app", 1001, 100);

    std::vector<TrafficFilterRedirectRule> rules;
    rules.push_back(CreateTestRule(200, TrafficFilterHookPoint::HOOK_PREROUTING));
    rules.push_back(CreateTestRule(100, TrafficFilterHookPoint::HOOK_OUTPUT));
    rules.push_back(CreateTestRule(150, TrafficFilterHookPoint::HOOK_PREROUTING));

    int32_t ret = context.RestoreRules(rules);

    EXPECT_EQ(ret, 0);
    EXPECT_TRUE(context.HasRules());

    std::vector<TrafficFilterRedirectRule> sortedRules = context.GetSortedRules();
    EXPECT_EQ(sortedRules.size(), 3);
    EXPECT_EQ(sortedRules[0].priority_, 100);
    EXPECT_EQ(sortedRules[1].priority_, 150);
    EXPECT_EQ(sortedRules[2].priority_, 200);
}

HWTEST_F(NetTrafficFilterRedirectorContextTest, RestoreRules003, TestSize.Level1)
{
    NetTrafficFilterRedirectorContext context("test_id", "com.test.app", 1001, 100);

    TrafficFilterRedirectRule rule1 = CreateTestRule(100, TrafficFilterHookPoint::HOOK_PREROUTING);
    TrafficFilterRedirectRule rule2 = CreateTestRule(200, TrafficFilterHookPoint::HOOK_OUTPUT);
    context.AddRuleWithPriority(rule1);
    context.AddRuleWithPriority(rule2);

    std::vector<TrafficFilterRedirectRule> newRules;
    newRules.push_back(CreateTestRule(300, TrafficFilterHookPoint::HOOK_PREROUTING));
    newRules.push_back(CreateTestRule(250, TrafficFilterHookPoint::HOOK_PREROUTING));

    context.RestoreRules(newRules);

    std::vector<TrafficFilterRedirectRule> sortedRules = context.GetSortedRules();
    EXPECT_EQ(sortedRules.size(), 2);
    EXPECT_EQ(sortedRules[0].priority_, 250);
    EXPECT_EQ(sortedRules[1].priority_, 300);
}

HWTEST_F(NetTrafficFilterRedirectorContextTest, RestoreRules004, TestSize.Level2)
{
    NetTrafficFilterRedirectorContext context("test_id", "com.test.app", 1001, 100);

    std::vector<TrafficFilterRedirectRule> rules;
    for (int i = 0; i < 50; i++) {
        rules.push_back(CreateTestRule(500 - i * 5, TrafficFilterHookPoint::HOOK_PREROUTING));
    }

    context.RestoreRules(rules);

    std::vector<TrafficFilterRedirectRule> sortedRules = context.GetSortedRules();
    EXPECT_EQ(sortedRules.size(), 50);
    for (size_t i = 0; i < sortedRules.size() - 1; i++) {
        EXPECT_LT(sortedRules[i].priority_, sortedRules[i + 1].priority_);
    }
}

HWTEST_F(NetTrafficFilterRedirectorContextTest, SetCallingInfo001, TestSize.Level1)
{
    NetTrafficFilterRedirectorContext context("test_id", "com.test.app", 1001, 100);

    context.SetCallingInfo(98765, 54321);

    EXPECT_EQ(context.GetCallingUid(), 98765);
    EXPECT_EQ(context.GetCallingPid(), 54321);
}

HWTEST_F(NetTrafficFilterRedirectorContextTest, SetPaused001, TestSize.Level1)
{
    NetTrafficFilterRedirectorContext context("test_id", "com.test.app", 1001, 100);

    EXPECT_FALSE(context.IsPaused());

    context.SetPaused(true);

    EXPECT_TRUE(context.IsPaused());

    context.SetPaused(false);

    EXPECT_FALSE(context.IsPaused());
}

HWTEST_F(NetTrafficFilterRedirectorContextTest, EdgeCase_SamePriority001, TestSize.Level1)
{
    NetTrafficFilterRedirectorContext context("test_id", "com.test.app", 1001, 100);

    TrafficFilterRedirectRule rule1 = CreateTestRule(100, TrafficFilterHookPoint::HOOK_PREROUTING, 6);
    TrafficFilterRedirectRule rule2 = CreateTestRule(100, TrafficFilterHookPoint::HOOK_OUTPUT, 17);
    TrafficFilterRedirectRule rule3 = CreateTestRule(100, TrafficFilterHookPoint::HOOK_FORWARD, 0);

    context.AddRuleWithPriority(rule1);
    context.AddRuleWithPriority(rule2);
    context.AddRuleWithPriority(rule3);

    std::vector<TrafficFilterRedirectRule> sortedRules = context.GetSortedRules();

    EXPECT_EQ(sortedRules.size(), 3);
    EXPECT_EQ(sortedRules[0].priority_, 100);
    EXPECT_EQ(sortedRules[1].priority_, 100);
    EXPECT_EQ(sortedRules[2].priority_, 100);
}

HWTEST_F(NetTrafficFilterRedirectorContextTest, EdgeCase_LargePriority001, TestSize.Level1)
{
    NetTrafficFilterRedirectorContext context("test_id", "com.test.app", 1001, 100);

    TrafficFilterRedirectRule rule1 = CreateTestRule(1, TrafficFilterHookPoint::HOOK_PREROUTING);
    TrafficFilterRedirectRule rule2 = CreateTestRule(10000, TrafficFilterHookPoint::HOOK_OUTPUT);
    TrafficFilterRedirectRule rule3 = CreateTestRule(5000, TrafficFilterHookPoint::HOOK_PREROUTING);

    context.AddRuleWithPriority(rule1);
    context.AddRuleWithPriority(rule2);
    context.AddRuleWithPriority(rule3);

    std::vector<TrafficFilterRedirectRule> sortedRules = context.GetSortedRules();

    EXPECT_EQ(sortedRules.size(), 3);
    EXPECT_EQ(sortedRules[0].priority_, 1);
    EXPECT_EQ(sortedRules[1].priority_, 5000);
    EXPECT_EQ(sortedRules[2].priority_, 10000);
}
} // namespace NetManagerStandard
} // namespace OHOS
