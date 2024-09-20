/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <thread>

#define private public
#define protected public
#include "firewall_rule.h"
#undef private
#undef protected
#include "net_mgr_log_wrapper.h"
#include "net_policy_inner_define.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
} // namespace

class UtFirewallRule : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void UtFirewallRule::SetUpTestCase() {}

void UtFirewallRule::TearDownTestCase() {}

void UtFirewallRule::SetUp() {}

void UtFirewallRule::TearDown() {}

/**
 * @tc.name: CreateFirewallRule
 * @tc.desc: Test FirewallRule CreateFirewallRule.
 * @tc.type: FUNC
 */
HWTEST_F(UtFirewallRule, CreateFirewallRule, TestSize.Level1)
{
    auto rulePtr = FirewallRule::CreateFirewallRule(FIREWALL_CHAIN_DEVICE_IDLE);
    EXPECT_NE(rulePtr, nullptr);
    rulePtr = FirewallRule::CreateFirewallRule(FIREWALL_CHAIN_NONE);
    EXPECT_EQ(rulePtr, nullptr);
}

/**
 * @tc.name: GetPolicyByUid001
 * @tc.desc: Test FirewallRule FirewallRuleCon.
 * @tc.type: FUNC
 */
HWTEST_F(UtFirewallRule, FirewallRuleCon, TestSize.Level1)
{
    FirewallRule rule(FIREWALL_CHAIN_DEVICE_IDLE);
    EXPECT_EQ(rule.chainType_, FIREWALL_CHAIN_DEVICE_IDLE);
    EXPECT_NE(rule.netsys_, nullptr);
    uint32_t uid = 1;
    rule.SetUidFirewallRule(uid, false);
    rule.SetUidFirewallRule(uid, true);
    rule.EnableFirewall(true);
}

/**
 * @tc.name: SetAllowedList
 * @tc.desc: Test FirewallRule SetAllowedList.
 * @tc.type: FUNC
 */
HWTEST_F(UtFirewallRule, SetAllowedList, TestSize.Level1)
{
    NETMGR_LOG_E("SetAllowedList enter");
    FirewallRule rule(FIREWALL_CHAIN_DEVICE_IDLE);
    rule.ClearAllowedList();
    EXPECT_EQ(rule.GetAllowedList().size(), static_cast<uint32_t>(0));
    uint32_t uid = 1;
    std::vector<uint32_t> uidsVec;
    uidsVec.push_back(uid);
    rule.SetAllowedList(uidsVec, FIREWALL_RULE_ALLOW);
    EXPECT_EQ(rule.GetAllowedList().size(), static_cast<uint32_t>(1));
    EXPECT_EQ(rule.GetAllowedList()[0], uid);
    uint32_t uidCount = 5;
    std::set<uint32_t> uids;
    for (size_t i = 0; i < uidCount; i++) {
        uids.insert(i);
    }
    rule.SetAllowedList(uids);
    EXPECT_EQ(rule.GetAllowedList().size(), uidCount);
    rule.SetAllowedList(uidsVec, FIREWALL_RULE_DENY);
    uint32_t expectCount = uidCount - 1;
    EXPECT_EQ(rule.GetAllowedList().size(), expectCount);
    uid = 5;
    rule.SetAllowedList(uidsVec, FIREWALL_RULE_DENY);
    EXPECT_EQ(rule.GetAllowedList().size(), expectCount);
    uid = 2;
    rule.RemoveFromAllowedList(uid);
    EXPECT_EQ(rule.GetAllowedList().size(), --expectCount);
    uid = 0;
    rule.RemoveFromAllowedList(uid);
    EXPECT_EQ(rule.GetAllowedList().size(), --expectCount);
}

/**
 * @tc.name: SetDeniedList
 * @tc.desc: Test FirewallRule SetDeniedList.
 * @tc.type: FUNC
 */
HWTEST_F(UtFirewallRule, SetDeniedList, TestSize.Level1)
{
    FirewallRule rule(FIREWALL_CHAIN_DEVICE_IDLE);
    rule.ClearDeniedList();
    EXPECT_EQ(rule.GetDeniedList().size(), static_cast<uint32_t>(0));
    uint32_t uid = 1;
    rule.SetDeniedList(uid, FIREWALL_RULE_DENY);
    EXPECT_EQ(rule.GetDeniedList().size(), static_cast<uint32_t>(1));
    EXPECT_EQ(rule.GetDeniedList()[0], uid);
    rule.ClearDeniedList();
    uint32_t uidCount = 5;
    std::vector<uint32_t> uids;
    for (size_t i = 0; i < uidCount; i++) {
        uids.emplace_back(i);
    }
    rule.ClearDeniedList();
    rule.SetDeniedList(uids);
    EXPECT_EQ(rule.GetDeniedList().size(), uidCount);
    rule.SetDeniedList(uid, FIREWALL_RULE_ALLOW);
    uint32_t expectCount = uidCount - 1;
    EXPECT_EQ(rule.GetDeniedList().size(), expectCount);
    uid = 5;
    rule.SetDeniedList(uid, FIREWALL_RULE_ALLOW);
    EXPECT_EQ(rule.GetDeniedList().size(), expectCount);
    uid = 2;
    rule.RemoveFromDeniedList(uid);
    EXPECT_EQ(rule.GetDeniedList().size(), --expectCount);
    uid = 0;
    rule.RemoveFromDeniedList(uid);
    EXPECT_EQ(rule.GetDeniedList().size(), --expectCount);
}
} // namespace NetManagerStandard
} // namespace OHOS