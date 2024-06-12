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

#include "gtest/gtest-message.h"
#include "gtest/gtest-test-part.h"
#include "gtest/hwext/gtest-ext.h"
#include "gtest/hwext/gtest-tag.h"
#include "http_proxy.h"
#include "inet_addr.h"
#include "net_manager_constants.h"
#include "netmanager_ext_test_security.h"
#include "netmgr_ext_log_wrapper.h"
#include "refbase.h"
#include "singleton.h"


#define private public
#define protected public

#include <string>

#include "i_netfirewall_service.h"
#include "netfirewall_service.h"
#include "netfirewall_client.h"
#include "netfirewall_common.h"
#include "netfirewall_proxy.h"
#include "netsys_controller.h"
#include "system_ability_definition.h"
#include "bundle_constants.h"
#include "netfirewall_database.h"
#include "netfirewall_default_rule_parser.h"
#include "netfirewall_db_helper.h"
#include "netfirewall_hisysevent.h"
#include "netfirewall_intercept_recorder.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
int32_t g_rowId = 0;

constexpr uint32_t APPID_TEST01 = 2034;
constexpr int32_t USER_ID1 = 100;
constexpr int32_t USER_ID2 = 101;
constexpr int32_t OLD_VERSION = 1;
constexpr int32_t NEW_VERSION = 2;
constexpr int32_t MAX_USER_RULE = 1;
constexpr int32_t MAX_IPS = 1;
constexpr int32_t MAX_PORTS = 1;
constexpr int32_t MAX_DOMAINS = 1;
constexpr uint16_t LOCAL_START_PORT = 10020;
constexpr uint16_t LOCAL_END_PORT = 1003;
constexpr uint16_t REMOTE_START_PORT = 1002;
constexpr uint16_t REMOTE_END_PORT = 10030;

std::vector<NetFirewallIpParam> GetIpList(const std::string &addressStart)
{
    const uint8_t mask = 24;
    std::vector<NetFirewallIpParam> localParamList;
    NetFirewallIpParam localParam;
    localParam.family = 1;
    localParam.type = 0;
    localParam.mask = mask;
    localParam.startIp = "192.168.16.7";
    localParam.endIp = "19.168.1.3";
    for (int i = 0; i < MAX_IPS; i++) {
        localParam.address = addressStart + std::to_string(i);
        localParamList.push_back(localParam);
    }
    return localParamList;
}

sptr<NetFirewallRule> GetNetFirewallRuleSptr()
{
    sptr<NetFirewallRule> rule = (std::make_unique<NetFirewallRule>()).release();
    if (!rule) {
        return rule;
    }
    rule->ruleId = 1;
    rule->userId = USER_ID1;
    rule->ruleName = "rule test";
    rule->ruleDescription = "AddNetFirewallRule 001";
    rule->ruleDirection = NetFirewallRuleDirection::RULE_OUT;
    rule->ruleAction = FirewallRuleAction::RULE_ALLOW;
    rule->isEnabled = true;
    rule->appUid = APPID_TEST01;

    rule->localIps = GetIpList("192.168.10.");
    rule->remoteIps = GetIpList("192.168.2.");
    std::vector<NetFirewallPortParam> localPortParamList;
    NetFirewallPortParam localPortParam;
    localPortParam.startPort = LOCAL_START_PORT;
    localPortParam.endPort = LOCAL_END_PORT;
    for (int i = 0; i < MAX_PORTS; i++) {
        localPortParamList.push_back(localPortParam);
    }
    rule->localPorts = localPortParamList;
    std::vector<NetFirewallPortParam> remotePortParamList;
    NetFirewallPortParam remotePortParam;
    remotePortParam.startPort = REMOTE_START_PORT;
    remotePortParam.endPort = REMOTE_END_PORT;
    for (int i = 0; i < MAX_PORTS; i++) {
        remotePortParamList.push_back(remotePortParam);
    }
    rule->remotePorts = remotePortParamList;
    std::vector<NetFirewallDomainParam> domainList;
    NetFirewallDomainParam domain;
    domain.isWildcard = 1;
    domain.domain = "www.openharmony.cn";
    for (int i = 0; i < MAX_DOMAINS; i++) {
        domainList.push_back(domain);
    }
    rule->domains = domainList;
    rule->dns.primaryDns = "192.168.1.245";
    rule->dns.standbyDns = "192.168.1.1";

    return rule;
}

sptr<NetFirewallRule> GetNetFirewallRuleSptrTypeDns()
{
    sptr<NetFirewallRule> rule = (std::make_unique<NetFirewallRule>()).release();
    if (!rule) {
        return rule;
    }
    int ruleId = 2;
    rule->ruleId = ruleId;
    rule->userId = USER_ID1;
    rule->ruleName = "rule test";
    rule->ruleDescription = "AddNetFirewallRule 001";
    rule->ruleDirection = NetFirewallRuleDirection::RULE_OUT;
    rule->ruleAction = FirewallRuleAction::RULE_ALLOW;
    rule->isEnabled = true;
    rule->appUid = APPID_TEST01;

    rule->localIps = GetIpList("192.168.10.");
    rule->remoteIps = GetIpList("192.168.2.");
    rule->ruleType = NetFirewallRuleType::RULE_IP;
    std::vector<NetFirewallPortParam> localPortParamList;
    NetFirewallPortParam localPortParam;
    localPortParam.startPort = LOCAL_START_PORT;
    localPortParam.endPort = LOCAL_END_PORT;
    for (int i = 0; i < MAX_PORTS; i++) {
        localPortParamList.push_back(localPortParam);
    }
    rule->localPorts = localPortParamList;
    std::vector<NetFirewallPortParam> remotePortParamList;
    NetFirewallPortParam remotePortParam;
    remotePortParam.startPort = REMOTE_START_PORT;
    remotePortParam.endPort = REMOTE_END_PORT;
    for (int i = 0; i < MAX_PORTS; i++) {
        remotePortParamList.push_back(remotePortParam);
    }
    rule->remotePorts = remotePortParamList;
    std::vector<NetFirewallDomainParam> domainList;
    NetFirewallDomainParam domain;
    domain.isWildcard = 1;
    domain.domain = "www.openharmony.cn";
    for (int i = 0; i < MAX_DOMAINS; i++) {
        domainList.push_back(domain);
    }
    rule->domains = domainList;
    rule->dns.primaryDns = "192.168.1.245";
    rule->dns.standbyDns = "192.168.1.1";

    return rule;
}
}

class NetFirewallServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();

    void SetUp();
    void TearDown();
    bool PublishChangedEvent(const std::string &action, int32_t code) const;
    static inline auto instance_ = DelayedSingleton<NetFirewallService>::GetInstance();
};

void NetFirewallServiceTest::SetUpTestCase() {}

void NetFirewallServiceTest::TearDownTestCase() {}

void NetFirewallServiceTest::SetUp() {}

void NetFirewallServiceTest::TearDown() {}

/**
 * @tc.name: OnStart
 * @tc.desc: Test NetFirewallServiceTest OnStart.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, OnStart, TestSize.Level1)
{
    instance_->state_ = NetFirewallService::ServiceRunningState::STATE_RUNNING;
    instance_->OnStart();
    instance_->state_ = NetFirewallService::ServiceRunningState::STATE_NOT_START;
    EXPECT_EQ(instance_->state_, NetFirewallService::ServiceRunningState::STATE_NOT_START);
}

/**
 * @tc.name: OnInit001
 * @tc.desc: Test NetFirewallServiceTest OnInit.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, OnInit001, TestSize.Level1)
{
    int32_t ret = instance_->OnInit();
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

/**
 * @tc.name: OnStop
 * @tc.desc: Test NetFirewallServiceTest OnStop.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, OnStop, TestSize.Level1)
{
    instance_->OnStop();
    EXPECT_EQ(instance_->state_, NetFirewallService::ServiceRunningState::STATE_NOT_START);
}

/**
 * @tc.name: Dump
 * @tc.desc: Test NetFirewallServiceTest Dump.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, Dump, TestSize.Level1)
{
    int32_t fd = 1;
    std::vector<std::u16string> args = {};
    EXPECT_EQ(instance_->Dump(fd, args), FIREWALL_SUCCESS);
}

/**
 * @tc.name: GetDumpMessage
 * @tc.desc: Test NetFirewallServiceTest GetDumpMessage.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, GetDumpMessage, TestSize.Level1)
{
    std::string message;
    instance_->GetDumpMessage(message);
    EXPECT_EQ(message.empty(), false);
}

/**
 * @tc.name: OnAddSystemAbility001
 * @tc.desc: Test NetFirewallServiceTest OnAddSystemAbility.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, OnAddSystemAbility001, TestSize.Level1)
{
    std::string deviceId = "dev1";
    instance_->OnRemoveSystemAbility(COMM_NETSYS_NATIVE_SYS_ABILITY_ID, deviceId);
    EXPECT_TRUE(instance_->hasSaRemoved_);

    instance_->OnAddSystemAbility(COMM_NETSYS_NATIVE_SYS_ABILITY_ID, deviceId);
    EXPECT_FALSE(instance_->hasSaRemoved_);
}

/**
 * @tc.name: AddDefaultNetFirewallRule001
 * @tc.desc: Test NetFirewallServiceTest AddDefaultNetFirewallRule.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, AddDefaultNetFirewallRule001, TestSize.Level1)
{
    int ret = instance_->AddDefaultNetFirewallRule(instance_->GetCurrentAccountId());
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

/**
 * @tc.name: AddDefaultNetFirewallRule002
 * @tc.desc: Test NetFirewallServiceTest AddDefaultNetFirewallRule.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, AddDefaultNetFirewallRule002, TestSize.Level1)
{
    int ret = instance_->AddDefaultNetFirewallRule(instance_->GetCurrentAccountId());
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

/**
 * @tc.name: DeleteNetFirewallRuleByUserId001
 * @tc.desc: Test NetFirewallServiceTest DeleteNetFirewallRuleByUserId.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, DeleteNetFirewallRuleByUserId001, TestSize.Level1)
{
    NETMGR_EXT_LOG_I("delete userid id = %{public}d ", instance_->GetCurrentAccountId());
    int ret = NetFirewallRuleManager::GetInstance()->DeleteNetFirewallRuleByUserId(instance_->GetCurrentAccountId());
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}


/**
 * @tc.name: SetNetFirewallPolicy001
 * @tc.desc: Test NetFirewallServiceTest SetNetFirewallPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, SetNetFirewallPolicy001, TestSize.Level1)
{
    int32_t userId = USER_ID1;
    DelayedSingleton<NetFirewallService>::GetInstance()->GetCurrentAccountId();
    sptr<NetFirewallPolicy> status = new (std::nothrow) NetFirewallPolicy;
    status->isOpen = true;
    status->inAction = FirewallRuleAction::RULE_DENY;
    status->outAction = FirewallRuleAction::RULE_ALLOW;
    int ret = DelayedSingleton<NetFirewallService>::GetInstance()->SetNetFirewallPolicy(userId, status);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

/**
 * @tc.name: SetNetFirewallPolicy002
 * @tc.desc: Test NetFirewallServiceTest SetNetFirewallPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, SetNetFirewallPolicy002, TestSize.Level1)
{
    int32_t userId = 101;
    sptr<NetFirewallPolicy> status = new (std::nothrow) NetFirewallPolicy;
    status->isOpen = false;
    status->inAction = FirewallRuleAction::RULE_DENY;
    status->outAction = FirewallRuleAction::RULE_ALLOW;
    int ret = DelayedSingleton<NetFirewallService>::GetInstance()->SetNetFirewallPolicy(userId, status);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

/**
 * @tc.name: ClearCurrentNetFirewallPreferences001
 * @tc.desc: Test NetFirewallServiceTest ClearCurrentFirewallPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, ClearCurrentNetFirewallPreferences001, TestSize.Level1)
{
    int32_t userId = 101;
    int ret = NetFirewallPolicyManager::GetInstance()->ClearCurrentFirewallPolicy();
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}


/**
 * @tc.name: GetNetFirewallPolicy001
 * @tc.desc: Test NetFirewallServiceTest GetNetFirewallPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, GetNetFirewallPolicy001, TestSize.Level1)
{
    int32_t userId = USER_ID1;
    sptr<NetFirewallPolicy> status = new (std::nothrow) NetFirewallPolicy;
    int ret = DelayedSingleton<NetFirewallService>::GetInstance()->GetNetFirewallPolicy(userId, status);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
    EXPECT_TRUE(status->isOpen);
}

/**
 * @tc.name: AddNetFirewallRule001
 * @tc.desc: Test NetFirewallServiceTest AddNetFirewallRule.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, AddNetFirewallRule001, TestSize.Level1)
{
    sptr<NetFirewallRule> rule = GetNetFirewallRuleSptr();
    int32_t ruleId = 0;
    int ret = DelayedSingleton<NetFirewallService>::GetInstance()->AddNetFirewallRule(rule, ruleId);
    g_rowId = ruleId;
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

/**
 * @tc.name: DeleteNetFirewallRuleByAppId001
 * @tc.desc: Test NetFirewallServiceTest DeleteNetFirewallRuleByAppId.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, DeleteNetFirewallRuleByAppId001, TestSize.Level1)
{
    NETMGR_EXT_LOG_I("delete appid id = %{public}d ", APPID_TEST01);
    int ret = NetFirewallRuleManager::GetInstance()->DeleteNetFirewallRuleByAppId(APPID_TEST01);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}


/**
 * @tc.name: AddNetFirewallRule002
 * @tc.desc: Test NetFirewallServiceTest AddNetFirewallRule.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, AddNetFirewallRule002, TestSize.Level1)
{
    sptr<NetFirewallRule> rule = GetNetFirewallRuleSptr();
    int32_t ruleId = 0;
    rule->userId = 102;
    int ret = DelayedSingleton<NetFirewallService>::GetInstance()->AddNetFirewallRule(rule, ruleId);
    NETMGR_EXT_LOG_I("db row id = %{public}d ", ruleId);
    g_rowId = ruleId;
    EXPECT_EQ(ret, FIREWALL_ERR_NO_RULE);
}

/**
 * @tc.name: AddNetFirewallRule003
 * @tc.desc: Test NetFirewallServiceTest AddNetFirewallRule.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, AddNetFirewallRule003, TestSize.Level1)
{
    sptr<NetFirewallRule> rule = GetNetFirewallRuleSptr();
    int32_t ruleId = 0;
    int ret = 0;
    int userId = instance_->GetCurrentAccountId();

    ret = instance_->OnInit();

    ret = instance_->AddDefaultNetFirewallRule(instance_->GetCurrentAccountId());

    rule->userId = userId;
    uint64_t startTime = 0;
    uint64_t endTime = 0;
    for (int i = 0; i < MAX_USER_RULE; i++) {
        startTime = GetCurrentMilliseconds();
        rule->ruleName = "ruleTest_" + std::to_string(i + 1);
        ret = DelayedSingleton<NetFirewallService>::GetInstance()->AddNetFirewallRule(rule, ruleId);
        endTime = GetCurrentMilliseconds();
        if (ret != FIREWALL_SUCCESS) {
            std::cout << "add user " << userId << " to db row failed! error code = " << ret << std::endl;
            break;
        }
        std::cout << "add user " << userId << " to db row id " << ruleId << ", use time : " << endTime - startTime <<
            std::endl;
        g_rowId = ruleId;
    }

    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

/**
 * @tc.name: AddNetFirewallRule004
 * @tc.desc: Test NetFirewallServiceTest AddNetFirewallRule.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, AddNetFirewallRule004, TestSize.Level1)
{
    sptr<NetFirewallRule> rule = GetNetFirewallRuleSptr();
    int32_t ruleId = 0;
    int userId = instance_->GetCurrentAccountId();
    rule->userId = ++userId;
    int ret = DelayedSingleton<NetFirewallService>::GetInstance()->AddNetFirewallRule(rule, ruleId);
    NETMGR_EXT_LOG_I("db row id = %{public}d ", ruleId);
    if (ruleId > 0) {
        g_rowId = ruleId;
    }
    EXPECT_EQ(ret, FIREWALL_ERR_NO_RULE);
}

/**
 * @tc.name: AddNetFirewallRule005
 * @tc.desc: Test NetFirewallServiceTest AddNetFirewallRule.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, AddNetFirewallRule005, TestSize.Level1)
{
    sptr<NetFirewallRule> rule = GetNetFirewallRuleSptrTypeDns();
    int32_t ruleId = 0;
    int userId = instance_->GetCurrentAccountId();
    rule->userId = ++userId;
    int ret = DelayedSingleton<NetFirewallService>::GetInstance()->AddNetFirewallRule(rule, ruleId);
    NETMGR_EXT_LOG_I("db row id = %{public}d ", ruleId);
    if (ruleId > 0) {
        g_rowId = ruleId;
    }
    EXPECT_EQ(ret, FIREWALL_ERR_NO_RULE);
}

/**
 * @tc.name: UpdateNetFirewallRule001
 * @tc.desc: Test NetFirewallServiceTest UpdateNetFirewallRule.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, UpdateNetFirewallRule001, TestSize.Level1)
{
    sptr<NetFirewallRule> rule = GetNetFirewallRuleSptr();
    rule->userId = USER_ID1;
    rule->ruleId = g_rowId;
    NETMGR_EXT_LOG_I("update row id = %{public}d ", g_rowId);
    int ret = DelayedSingleton<NetFirewallService>::GetInstance()->UpdateNetFirewallRule(rule);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

/**
 * @tc.name: UpdateNetFirewallRule002
 * @tc.desc: Test NetFirewallServiceTest UpdateNetFirewallRule.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, UpdateNetFirewallRule002, TestSize.Level1)
{
    sptr<NetFirewallRule> rule = GetNetFirewallRuleSptrTypeDns();
    rule->userId = USER_ID1;
    rule->ruleId = g_rowId;
    NETMGR_EXT_LOG_I("update row id = %{public}d ", g_rowId);
    int ret = DelayedSingleton<NetFirewallService>::GetInstance()->UpdateNetFirewallRule(rule);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

/**
 * @tc.name: DeleteNetFirewallRule001
 * @tc.desc: Test NetFirewallServiceTest DeleteNetFirewallRule.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, DeleteNetFirewallRule001, TestSize.Level1)
{
    int32_t ruleId = g_rowId;
    NETMGR_EXT_LOG_I("delete row id = %{public}d ", g_rowId);
    int ret = DelayedSingleton<NetFirewallService>::GetInstance()->DeleteNetFirewallRule(USER_ID1, ruleId);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

/**
 * @tc.name: GetAllNetFirewallRule001
 * @tc.desc: Test NetFirewallServiceTest GetNetFirewallRules.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, GetAllNetFirewallRule001, TestSize.Level1)
{
    int32_t userId = USER_ID1;
    sptr<RequestParam> param = new (std::nothrow) RequestParam();
    param->page = 0;
    param->pageSize = 5;
    param->orderType = NetFirewallOrderType::ORDER_ASC;
    param->orderField = NetFirewallOrderField::ORDER_BY_RULE_NAME;
    sptr<FirewallRulePage> info = new (std::nothrow) FirewallRulePage();
    uint64_t startTime = GetCurrentMilliseconds();
    int ret = DelayedSingleton<NetFirewallService>::GetInstance()->GetNetFirewallRules(userId, param, info);
    uint64_t endTime = GetCurrentMilliseconds();
    std::cout << "GetNetFirewallRules " << userId << "running time : " << endTime - startTime << std::endl;
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

/**
 * @tc.name: GetAllNetFirewallRule002
 * @tc.desc: Test NetFirewallServiceTest GetNetFirewallRules.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, GetAllNetFirewallRule002, TestSize.Level1)
{
    int32_t userId = 102;
    sptr<RequestParam> param = new (std::nothrow) RequestParam();
    param->page = 0;
    param->pageSize = 5;
    param->orderType = NetFirewallOrderType::ORDER_ASC;
    param->orderField = NetFirewallOrderField::ORDER_BY_RULE_NAME;
    sptr<FirewallRulePage> info = new (std::nothrow) FirewallRulePage();
    int ret = DelayedSingleton<NetFirewallService>::GetInstance()->GetNetFirewallRules(userId, param, info);

    EXPECT_EQ(ret, FIREWALL_ERR_NO_USER);
}

/**
 * @tc.name: GetNetFirewallRule001
 * @tc.desc: Test NetFirewallServiceTest GetNetFirewallRule.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, GetNetFirewallRule001, TestSize.Level1)
{
    int32_t ruleId = 1;
    int32_t userId = USER_ID1;
    NetFirewallRuleManager::GetInstance()->GetAllRuleConstraint(userId);
    ruleId = NetFirewallRuleManager::GetInstance()->allUserRule_;
    uint64_t startTime = GetCurrentMilliseconds();
    sptr<NetFirewallRule> rule = new (std::nothrow) NetFirewallRule();
    int ret = DelayedSingleton<NetFirewallService>::GetInstance()->GetNetFirewallRule(userId, ruleId, rule);
    uint64_t endTime = GetCurrentMilliseconds();
    std::cout << "GetNetFirewallRule " << userId << "running time : " << endTime - startTime << std::endl;

    if (rule->ruleId == 0) {
        EXPECT_EQ(ret, FIREWALL_ERR_NO_RULE);
    } else {
        EXPECT_EQ(ret, FIREWALL_SUCCESS);
    }
}

/**
 * @tc.name: OnInit002
 * @tc.desc: Test NetFirewallServiceTest OnInit.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, OnInit002, TestSize.Level1)
{
    int32_t ret = instance_->OnInit();
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

/**
 * @tc.name: OnReceiveEvent
 * @tc.desc: Test NetFirewallServiceTest OnReceiveEvent.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, OnReceiveEvent, TestSize.Level1)
{
    int32_t ret = PublishChangedEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_ADDED, USER_ID1);
    EXPECT_EQ(ret, true);
    ret = PublishChangedEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED, USER_ID1);
    EXPECT_EQ(ret, true);
    ret = PublishChangedEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED, USER_ID2);
    EXPECT_EQ(ret, true);
    ret = PublishChangedEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED, USER_ID1);

    EXPECT_EQ(ret, true);
}

bool NetFirewallServiceTest::PublishChangedEvent(const std::string &action, int32_t code) const
{
    Want want;
    want.SetAction(action);
    want.SetParam(AppExecFwk::Constants::UID, USER_ID1);
    CommonEventData data;
    data.SetWant(want);
    data.SetCode(code);
    if (instance_->subscriber_ != nullptr) {
        instance_->subscriber_->OnReceiveEvent(data);
    }
    return true;
}

/**
 * @tc.name: OnIntercept
 * @tc.desc: Test NetFirewallServiceTest OnIntercept.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, OnIntercept, TestSize.Level1)
{
    sptr<InterceptRecord> record = (std::make_unique<InterceptRecord>()).release();
    const uint32_t time = 10025152;
    const uint16_t sourcePort = 10000;
    const uint16_t destPort = 20000;
    const uint32_t uid = 10085;
    record->time = time;
    record->localIp = "192.168.1.2";
    record->remoteIp = "192.168.1.3";
    record->localPort = sourcePort;
    record->remotePort = destPort;
    record->protocol = 1;
    record->appUid = uid;
    int32_t ret = NetFirewallInterceptRecorder::GetInstance()->callback_->OnIntercept(record);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

/**
 * @tc.name: GetInterceptRecord001
 * @tc.desc: Test NetFirewallServiceTest GetInterceptRecords.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, GetInterceptRecord001, TestSize.Level1)
{
    sptr<RequestParam> param = new (std::nothrow) RequestParam();
    param->page = 0;
    param->pageSize = 5;
    param->orderType = NetFirewallOrderType::ORDER_ASC;
    param->orderField = NetFirewallOrderField::ORDER_BY_RECORD_TIME;
    sptr<InterceptRecordPage> info = new (std::nothrow) InterceptRecordPage();
    int ret = instance_->GetInterceptRecords(instance_->GetCurrentAccountId(), param, info);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

/**
 * @tc.name: OnCreate
 * @tc.desc: Test NetFirewallServiceTest OnCreate001.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, OnCreate001, TestSize.Level1)
{
    auto dbCallBack = new (std::nothrow) NetFirewallDataBaseCallBack();
    std::string firewallDatabaseName = FIREWALL_DB_PATH + FIREWALL_DB_NAME;
    int32_t errCode = OHOS::NativeRdb::E_OK;
    OHOS::NativeRdb::RdbStoreConfig config(firewallDatabaseName);
    config.SetSecurityLevel(NativeRdb::SecurityLevel::S1);
    NetFirewallDataBaseCallBack sqliteOpenHelperCallback;
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store_ =
        OHOS::NativeRdb::RdbHelper::GetRdbStore(config, OLD_VERSION, sqliteOpenHelperCallback, errCode);
    int32_t ret = dbCallBack->OnCreate(*(store_));
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

/**
 * @tc.name: OnUpgrade
 * @tc.desc: Test NetFirewallServiceTest OnUpgrade001.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, OnUpgrade001, TestSize.Level1)
{
    auto dbCallBack = new (std::nothrow) NetFirewallDataBaseCallBack();
    std::string firewallDatabaseName = FIREWALL_DB_PATH + FIREWALL_DB_NAME;
    int32_t errCode = OHOS::NativeRdb::E_OK;
    OHOS::NativeRdb::RdbStoreConfig config(firewallDatabaseName);
    config.SetSecurityLevel(NativeRdb::SecurityLevel::S1);
    NetFirewallDataBaseCallBack sqliteOpenHelperCallback;
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store_ =
        OHOS::NativeRdb::RdbHelper::GetRdbStore(config, NEW_VERSION, sqliteOpenHelperCallback, errCode);
    int32_t ret = dbCallBack->OnUpgrade(*(store_), OLD_VERSION, NEW_VERSION);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

/**
 * @tc.name: OnDowngrade
 * @tc.desc: Test NetFirewallServiceTest OnDowngrade001.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, OnDowngrade001, TestSize.Level1)
{
    auto dbCallBack = new (std::nothrow) NetFirewallDataBaseCallBack();
    std::string firewallDatabaseName = FIREWALL_DB_PATH + FIREWALL_DB_NAME;
    int32_t errCode = OHOS::NativeRdb::E_OK;
    OHOS::NativeRdb::RdbStoreConfig config(firewallDatabaseName);
    config.SetSecurityLevel(NativeRdb::SecurityLevel::S1);
    NetFirewallDataBaseCallBack sqliteOpenHelperCallback;
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store_ =
        OHOS::NativeRdb::RdbHelper::GetRdbStore(config, NEW_VERSION, sqliteOpenHelperCallback, errCode);
    int32_t ret = dbCallBack->OnDowngrade(*(store_), OLD_VERSION, NEW_VERSION);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

/**
 * @tc.name: RollBack
 * @tc.desc: Test NetFirewallServiceTest RollBack001.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, RollBack001, TestSize.Level1)
{
    auto dbInstance_ = NetFirewallDataBase::GetInstance();
    int32_t ret = dbInstance_->RollBack();
    EXPECT_EQ(ret != FIREWALL_SUCCESS, true);
}

/**
 * @tc.name: QueryEnabledFirewallRules
 * @tc.desc: Test NetFirewallDbHelper QueryEnabledFirewallRules001.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, QueryEnabledFirewallRules001, TestSize.Level1)
{
    auto _netFileDbHelper = NetFirewallDbHelper::GetInstance();
    std::vector<NetFirewallRule> rules;
    int32_t userId = USER_ID1;
    int32_t appUip = APPID_TEST01;
    int32_t ret = _netFileDbHelper->QueryEnabledFirewallRules(userId, appUip, rules);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

/**
 * @tc.name: ConvertIpParamToConfig
 * @tc.desc: Test NetFirewallDefaultRuleParser ConvertIpParamToConfig001.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, ConvertIpParamToConfig001, TestSize.Level1)
{
    NetFirewallIpParam rule;
    std::string jsonString = "\"family\": 1,\"type\": 1,\"address\": \"192.168.1.2\",\"mask\": 32,\"startIp\": "
        "\"192.168.1.1\",\"endIp\": \"192.168.1.255\"";
    cJSON *mem = cJSON_Parse(jsonString.c_str());
    NetFirewallDefaultRuleParser::ConvertIpParamToConfig(rule, mem);
}

/**
 * @tc.name: ConvertDomainParamToConfig
 * @tc.desc: Test NetFirewallDefaultRuleParser ConvertDomainParamToConfig001.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, ConvertDomainParamToConfig001, TestSize.Level1)
{
    NetFirewallDomainParam rule;
    std::string jsonString = "\"isWildcard\": false,\"domain\": \"www.openharmony.cn\"";
    cJSON *mem = cJSON_Parse(jsonString.c_str());
    NetFirewallDefaultRuleParser::ConvertDomainParamToConfig(rule, mem);
}

/**
 * @tc.name: ConvertDnsParamToConfig
 * @tc.desc: Test NetFirewallDefaultRuleParser ConvertDnsParamToConfig001.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, ConvertDnsParamToConfig001, TestSize.Level1)
{
    NetFirewallDnsParam rule;
    std::string jsonString = "\"primaryDns\": \"192.168.1.1\",\"standbyDns\": \"192.168.1.1\"";
    cJSON *mem = cJSON_Parse(jsonString.c_str());
    NetFirewallDefaultRuleParser::ConvertDnsParamToConfig(rule, mem);
}

/**
 * @tc.name: AddFirewallDomainRule
 * @tc.desc: Test NetFirewallDbHelper AddFirewallDomainRule001.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, AddFirewallDomainRule001, TestSize.Level1)
{
    auto _netFileDbHelper = NetFirewallDbHelper::GetInstance();

    NativeRdb::ValuesBucket values;

    NetFirewallRule rule;
    rule.ruleId = 1;
    rule.userId = USER_ID1;
    std::vector<NetFirewallDomainParam> domainList;
    NetFirewallDomainParam domain;
    domain.isWildcard = 1;
    domain.domain = "www.openharmony.cn";
    for (int i = 0; i < MAX_DOMAINS; i++) {
        domainList.push_back(domain);
    }
    rule.domains = domainList;

    int32_t ruleId = _netFileDbHelper->AddFirewallDomainRule(values, rule, USER_ID1);
    EXPECT_GT(ruleId, 0);
}

/**
 * @tc.name: AddFirewallPortRule
 * @tc.desc: Test NetFirewallDbHelper AddFirewallPortRule001.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, AddFirewallPortRule001, TestSize.Level1)
{
    auto _netFileDbHelper = NetFirewallDbHelper::GetInstance();

    NativeRdb::ValuesBucket values;

    NetFirewallRule rule;
    int32_t outRowId = 1;
    rule.ruleId = 1;
    rule.userId = USER_ID1;
    rule.appUid = APPID_TEST01;

    rule.localIps = GetIpList("192.168.10.");
    rule.remoteIps = GetIpList("192.168.2.");
    std::vector<NetFirewallPortParam> localPortParamList;
    NetFirewallPortParam localPortParam;
    localPortParam.startPort = LOCAL_START_PORT;
    localPortParam.endPort = LOCAL_END_PORT;
    for (int i = 0; i < MAX_PORTS; i++) {
        localPortParamList.push_back(localPortParam);
    }
    rule.localPorts = localPortParamList;
    std::vector<NetFirewallPortParam> remotePortParamList;
    NetFirewallPortParam remotePortParam;
    remotePortParam.startPort = REMOTE_START_PORT;
    remotePortParam.endPort = REMOTE_END_PORT;
    for (int i = 0; i < MAX_PORTS; i++) {
        remotePortParamList.push_back(remotePortParam);
    }
    rule.remotePorts = remotePortParamList;

    int32_t ret = _netFileDbHelper->AddFirewallPortRule(values, rule, USER_ID1, LocationType::SRC_LOCATION);
    EXPECT_EQ(ret, outRowId);
}

/**
 * @tc.name: AddFirewallIpRule
 * @tc.desc: Test NetFirewallDbHelper AddFirewallIpRule001.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, AddFirewallIpRule001, TestSize.Level1)
{
    auto _netFileDbHelper = NetFirewallDbHelper::GetInstance();

    NativeRdb::ValuesBucket values;
    int64_t outRowId = 1;
    NetFirewallRule rule;
    rule.ruleId = 1;
    rule.userId = USER_ID1;
    rule.appUid = APPID_TEST01;

    rule.localIps = GetIpList("192.168.10.");
    rule.remoteIps = GetIpList("192.168.2.");
    std::vector<NetFirewallPortParam> localPortParamList;
    NetFirewallPortParam localPortParam;
    localPortParam.startPort = LOCAL_START_PORT;
    localPortParam.endPort = LOCAL_END_PORT;
    for (int i = 0; i < MAX_PORTS; i++) {
        localPortParamList.push_back(localPortParam);
    }
    rule.localPorts = localPortParamList;
    std::vector<NetFirewallPortParam> remotePortParamList;
    NetFirewallPortParam remotePortParam;
    remotePortParam.startPort = REMOTE_START_PORT;
    remotePortParam.endPort = REMOTE_END_PORT;
    for (int i = 0; i < MAX_PORTS; i++) {
        remotePortParamList.push_back(remotePortParam);
    }
    rule.remotePorts = remotePortParamList;

    int32_t ret = _netFileDbHelper->AddFirewallIpRule(values, rule, USER_ID1, LocationType::SRC_LOCATION);
    EXPECT_EQ(ret, outRowId);
}

/**
 * @tc.name: QueryAllFirewallRuleRecord
 * @tc.desc: Test NetFirewallDbHelper QueryAllFirewallRuleRecord001.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, QueryAllFirewallRuleRecord001, TestSize.Level1)
{
    auto _netFileDbHelper = NetFirewallDbHelper::GetInstance();

    std::vector<NetFirewallRule> rules;
    int32_t ret = _netFileDbHelper->QueryAllFirewallRuleRecord(rules);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
    EXPECT_GT(rules.size(), 0);
}

/**
 * @tc.name: SendInitDefaultRequestReport
 * @tc.desc: Test NetFirewallDbHelper SendInitDefaultRequestReport001.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, SendInitDefaultRequestReport001, TestSize.Level1)
{
    int32_t errorCode = 0;
    NetFirewallHisysEvent::SendInitDefaultRequestReport(USER_ID1, errorCode);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: SendNetFirewallRuleFault
 * @tc.desc: Test NetFirewallDbHelper SendNetFirewallRuleFault001.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, SendNetFirewallRuleFault001, TestSize.Level1)
{
    int32_t errorCode = 0;
    NetFirewallEvent event;
    event.userId = USER_ID1;
    event.errorType = errorCode;
    std::string info = "info";
    std::string eventName = "eventName";
    auto _netFileHisysEvent = NetFirewallHisysEvent::GetInstance();
    _netFileHisysEvent.SendNetFirewallRuleFault(event, info, eventName);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: SendNetFirewallFault
 * @tc.desc: Test NetFirewallDbHelper SendNetFirewallFault001.
 * @tc.type: FUNC
 */
HWTEST_F(NetFirewallServiceTest, SendNetFirewallFault001, TestSize.Level1)
{
    int32_t errorCode = 0;
    std::string eventName = "eventName";
    NetFirewallEvent event;
    event.userId = USER_ID1;
    event.errorType = errorCode;
    auto _netFileHisysEvent = NetFirewallHisysEvent::GetInstance();
    _netFileHisysEvent.SendNetFirewallFault(event, eventName);
    EXPECT_TRUE(true);
}
} // namespace NetManagerStandard
} // namespace OHOS
