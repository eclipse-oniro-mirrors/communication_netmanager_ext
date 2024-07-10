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
#include "gtest/hwext/gtest-tag.h"
#include "netmanager_ext_test_security.h"
#include "net_manager_constants.h"

#define private public
#define protected public

#include "netfirewall_rule_native_helper.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
}
class NetFirewallRuleNativeHelperTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();

    void SetUp();
    void TearDown();
};

void NetFirewallRuleNativeHelperTest::SetUpTestCase() {}

void NetFirewallRuleNativeHelperTest::TearDownTestCase() {}

void NetFirewallRuleNativeHelperTest::SetUp() {}

void NetFirewallRuleNativeHelperTest::TearDown() {}

/**
 * @tc.name: SetFirewallRulesInner001
 * @tc.desc: Test NetFirewallRuleNativeHelperTest SetFirewallRulesInner.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallRuleNativeHelperTest, SetFirewallRulesInner001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    std::vector<sptr<NetFirewallBaseRule>> rules;
    const int32_t USER_ID = 100;
    const int32_t RULE_NUM = 301;
    const int32_t MAX_IP_VALUE = 256;
    NetFirewallIpParam param;
    param.family = FAMILY_IPV4;
    param.type = SINGLE_IP;
    const std::string tmp = "192.168.";
    for (int32_t i = 0; i < RULE_NUM; i++) {
        sptr<NetFirewallIpRule> rule = new (std::nothrow) NetFirewallIpRule();
        ASSERT_NE(rule, nullptr);
        rule->userId = USER_ID;
        rule->ruleDirection = NetFirewallRuleDirection::RULE_OUT;
        rule->ruleAction = FirewallRuleAction::RULE_DENY;
        rule->protocol = NetworkProtocol::ICMP;

        inet_pton(AF_INET, (tmp + std::to_string(i / MAX_IP_VALUE) + "." + std::to_string(i % MAX_IP_VALUE)).c_str(),
            &param.ipv4.startIp);
        rule->remoteIps.emplace_back(param);
        rules.emplace_back(rule);
    }
    int32_t ret =
        NetFirewallRuleNativeHelper::GetInstance().SetFirewallRulesInner(NetFirewallRuleType::RULE_IP, rules, true);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
