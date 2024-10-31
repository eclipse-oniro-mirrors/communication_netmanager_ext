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

#include "netfirewall_rule_manager.h"
#include "net_manager_constants.h"
#include "singleton.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr int32_t USER_ID = 100;

sptr<NetFirewallRule> GetNetFirewallRuleSptr(NetFirewallRuleType type, int32_t appUid = 0)
{
    sptr<NetFirewallRule> rule = (std::make_unique<NetFirewallRule>()).release();
    if (!rule) {
        return rule;
    }
    rule->ruleId = 1;
    rule->userId = USER_ID;
    rule->ruleName = "testRule";
    rule->ruleDescription = "testRuleDes";
    rule->ruleDirection = NetFirewallRuleDirection::RULE_OUT;
    rule->ruleAction = FirewallRuleAction::RULE_ALLOW;
    rule->ruleType = type;
    rule->isEnabled = true;
    rule->appUid = appUid;
    switch (type) {
        case NetFirewallRuleType::RULE_IP: {
            const uint8_t mask = 24;
            std::vector<NetFirewallIpParam> localParamList;
            NetFirewallIpParam localParam;
            localParam.family = FAMILY_IPV4;
            localParam.type = SINGLE_IP;
            localParam.mask = mask;
            inet_pton(AF_INET, "192.168.9.6", &localParam.ipv4.startIp);
            localParamList.emplace_back(std::move(localParam));
            rule->remoteIps = localParamList;
            break;
        }
        case NetFirewallRuleType::RULE_DOMAIN: {
            std::vector<NetFirewallDomainParam> domainList;
            NetFirewallDomainParam domain;
            domain.isWildcard = false;
            domain.domain = "www.openharmony.netfirewallrulemanagertest.cn";
            domainList.emplace_back(domain);
            rule->domains = domainList;
            break;
        }
        case NetFirewallRuleType::RULE_DNS: {
            NetFirewallDnsParam dns;
            dns.primaryDns = "192.168.9.1";
            rule->dns = dns;
            break;
        }
        default:
            break;
    }

    return rule;
}
}

class NetFirewallRuleManagerTest : public testing::Test {
public:
    static void SetUpTestCase(){};
    static void TearDownTestCase(){};
    void SetUp(){};
    void TearDown(){};
    static inline auto instance_ = DelayedSingleton<NetFirewallRuleManager>::GetInstance();
};

HWTEST_F(NetFirewallRuleManagerTest, UpdateNetFirewallRule001, TestSize.Level1)
{
    sptr<NetFirewallRule> rule = nullptr;
    int32_t ret = instance_->UpdateNetFirewallRule(rule);
    EXPECT_EQ(ret, FIREWALL_ERR_PARAMETER_ERROR);
    rule = GetNetFirewallRuleSptr(NetFirewallRuleType::RULE_IP);
    rule->userId++;
    ret = instance_->UpdateNetFirewallRule(rule);
    EXPECT_EQ(ret, FIREWALL_ERR_NO_USER);
}

HWTEST_F(NetFirewallRuleManagerTest, UpdateNetFirewallRule002, TestSize.Level1)
{
    sptr<NetFirewallRule> rule = GetNetFirewallRuleSptr(NetFirewallRuleType::RULE_IP);
    int32_t ruleId;
    rule->isEnabled = false;
    int32_t ret = instance_->AddNetFirewallRule(rule, ruleId);
    EXPECT_NE(ruleId, 0);
    rule->ruleDescription = "UpdateNetFirewallRule002";
    rule->ruleId = ruleId;
    ret = instance_->UpdateNetFirewallRule(rule);
    instance_->DeleteNetFirewallRule(rule->userId, ruleId);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(NetFirewallRuleManagerTest, DeleteNetFirewallRule001, TestSize.Level1)
{
    int32_t userId = USER_ID;
    int32_t ruleId = 100;
    int32_t ret = instance_->DeleteNetFirewallRule(userId + 1, ruleId);
    EXPECT_EQ(ret, FIREWALL_ERR_NO_USER);

    ret = instance_->DeleteNetFirewallRule(userId, ruleId);
    EXPECT_EQ(ret, FIREWALL_ERR_NO_RULE);

    sptr<NetFirewallRule> rule = GetNetFirewallRuleSptr(NetFirewallRuleType::RULE_IP);
    rule->isEnabled = false;
    ret = instance_->AddNetFirewallRule(rule, ruleId);
    EXPECT_NE(ruleId, 100);
    ret = instance_->DeleteNetFirewallRule(rule->userId, ruleId);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(NetFirewallRuleManagerTest, GetEnabledNetFirewallRules001, TestSize.Level1)
{
    std::vector<NetFirewallRule> list;
    int32_t userId = USER_ID + 1;
    int32_t ret = instance_->GetEnabledNetFirewallRules(userId, list);
    EXPECT_EQ(ret, FIREWALL_ERR_NO_USER);
}

HWTEST_F(NetFirewallRuleManagerTest, GetNetFirewallRules001, TestSize.Level1)
{
    int32_t userId = USER_ID;
    sptr<RequestParam> param = nullptr;
    sptr<FirewallRulePage> info = nullptr;
    int32_t ret = instance_->GetNetFirewallRules(userId, param, info);
    EXPECT_EQ(ret, FIREWALL_ERR_PARAMETER_ERROR);
    param = new (std::nothrow) RequestParam();
    param->page = 1;
    param->pageSize = 5;
    param->orderType = NetFirewallOrderType::ORDER_ASC;
    param->orderField = NetFirewallOrderField::ORDER_BY_RULE_NAME;
    ret = instance_->GetNetFirewallRules(userId, param, info);
    EXPECT_EQ(ret, FIREWALL_ERR_INTERNAL);
}
}
}