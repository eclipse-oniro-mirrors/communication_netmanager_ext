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

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "mock_netfirewall_service_stub_test.h"
#include "net_manager_constants.h"
#include "netmanager_ext_test_security.h"
#include "netfirewall_common.h"
#include "netfirewall_stub.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr int32_t USER_ID = 100;
constexpr int32_t TEST_INT32_NUMBER = 1;

using namespace testing::ext;

std::vector<NetFirewallIpParam> GetIpList(uint8_t num = 1)
{
    const uint8_t mask = 24;
    const int32_t gap = 10;
    const int32_t hexWidth = 4;
    std::vector<NetFirewallIpParam> localParamList;
    NetFirewallIpParam localParam;
    localParam.family = FAMILY_IPV4;
    localParam.type = SINGLE_IP;
    std::string ip;
    std::stringstream ss;
    const std::string addressStart = "192.168.1.";
    for (int32_t i = 0; i < num; i++) {
        ss.str("");
        ss.clear();
        ss << addressStart << (i * gap);
        inet_pton(AF_INET, ss.str().c_str(), &localParam.ipv4.startIp);
        localParam.mask = mask;
        localParamList.emplace_back(std::move(localParam));
    }
    return localParamList;
}

std::vector<NetFirewallPortParam> GetPortList(uint8_t num = 0)
{
    const int32_t offset = 20;
    const uint16_t localStartPort = 10020;
    std::vector<NetFirewallPortParam> localPortParamList;
    NetFirewallPortParam localPortParam;
    localPortParam.startPort = localStartPort;
    localPortParam.endPort = localStartPort;
    for (int32_t i = 0; i < num; i++) {
        localPortParamList.emplace_back(std::move(localPortParam));
        localPortParam.startPort += offset;
        localPortParam.endPort += offset;
    }
    return localPortParamList;
}


sptr<NetFirewallRule> GetNetFirewallRuleSptr(NetFirewallRuleType type, uint8_t number, uint8_t portNum = 0)
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
    rule->appUid = 0;
    switch (type) {
        case NetFirewallRuleType::RULE_IP: {
            rule->localIps = GetIpList(number);
            rule->localPorts = GetPortList(portNum);
            break;
        }
        case NetFirewallRuleType::RULE_DOMAIN: {
            std::vector<NetFirewallDomainParam> domainList;
            NetFirewallDomainParam domain;
            domain.isWildcard = false;
            const std::string tmp = "www.openharmony.cn";
            for (int32_t i = 0; i < number; i++) {
                domain.domain = tmp + std::to_string(i);
                domainList.emplace_back(domain);
            }
            rule->domains = domainList;
            break;
        }
        default:
            break;
    }

    return rule;
}
}
class NetFirewallServiceStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<NetFirewallStub> instance_ = std::make_shared<MockNetFirewallServiceStub>();
    static int32_t SendRemoteRequest(MessageParcel &data, int32_t code);
};

void NetFirewallServiceStubTest::SetUpTestCase() {}

void NetFirewallServiceStubTest::TearDownTestCase() {}

void NetFirewallServiceStubTest::SetUp() {}

void NetFirewallServiceStubTest::TearDown() {}

int32_t NetFirewallServiceStubTest::SendRemoteRequest(MessageParcel &data, int32_t code)
{
    MessageParcel reply;
    MessageOption option;
    return instance_->OnRemoteRequest(static_cast<uint32_t>(code), data, reply, option);
}

HWTEST_F(NetFirewallServiceStubTest, SetNetFirewallPolicy001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    ASSERT_NE(data.WriteInterfaceToken(NetFirewallStub::GetDescriptor()), false);

    if (!data.WriteInt32(TEST_INT32_NUMBER)) {
        return;
    }

    sptr<NetFirewallPolicy> status = new (std::nothrow) NetFirewallPolicy();
    EXPECT_TRUE(status != nullptr);
    if (status == nullptr) {
        return;
    }

    if (!status->Marshalling(data)) {
        return;
    }

    int32_t ret = SendRemoteRequest(data, (INetFirewallService::SET_NET_FIREWALL_STATUS));
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(NetFirewallServiceStubTest, GetNetFirewallPolicy001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    ASSERT_NE(data.WriteInterfaceToken(NetFirewallStub::GetDescriptor()), false);

    if (!data.WriteInt32(TEST_INT32_NUMBER)) {
        return;
    }

    sptr<NetFirewallPolicy> status = new (std::nothrow) NetFirewallPolicy();
    EXPECT_TRUE(status != nullptr);
    if (status == nullptr) {
        return;
    }

    if (!status->Marshalling(data)) {
        return;
    }

    int32_t ret = SendRemoteRequest(data, (INetFirewallService::GET_NET_FIREWALL_STATUS));
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(NetFirewallServiceStubTest, AddNetFirewallRule001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    ASSERT_NE(data.WriteInterfaceToken(NetFirewallStub::GetDescriptor()), false);

    sptr<NetFirewallRule> status = new (std::nothrow) NetFirewallRule();
    EXPECT_TRUE(status != nullptr);
    if (status == nullptr) {
        return;
    }

    if (!status->Marshalling(data)) {
        return;
    }

    int32_t ret = SendRemoteRequest(data, (INetFirewallService::ADD_NET_FIREWALL_RULE));
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(NetFirewallServiceStubTest, UpdateNetFirewallRule001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    ASSERT_NE(data.WriteInterfaceToken(NetFirewallStub::GetDescriptor()), false);

    sptr<NetFirewallRule> status = new (std::nothrow) NetFirewallRule();
    EXPECT_TRUE(status != nullptr);
    if (status == nullptr) {
        return;
    }

    if (!status->Marshalling(data)) {
        return;
    }

    int32_t ret = SendRemoteRequest(data, (INetFirewallService::UPDATE_NET_FIREWALL_RULE));
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(NetFirewallServiceStubTest, DeleteNetFirewallRule001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    ASSERT_NE(data.WriteInterfaceToken(NetFirewallStub::GetDescriptor()), false);

    if (!data.WriteInt32(USER_ID)) {
        return;
    }

    if (!data.WriteInt32(TEST_INT32_NUMBER)) {
        return;
    }

    int32_t ret = SendRemoteRequest(data, (INetFirewallService::DELETE_NET_FIREWALL_RULE));
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(NetFirewallServiceStubTest, GetAllNetFirewallRule001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    ASSERT_NE(data.WriteInterfaceToken(NetFirewallStub::GetDescriptor()), false);

    if (!data.WriteInt32(TEST_INT32_NUMBER)) {
        return;
    }
    sptr<RequestParam> param = new (std::nothrow) RequestParam();
    param->page = 1;
    param->pageSize = 5;
    param->orderType = NetFirewallOrderType::ORDER_ASC;
    if (!param->Marshalling(data)) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetFirewallService::GET_ALL_NET_FIREWALL_RULES);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(NetFirewallServiceStubTest, GetNetFirewallRule001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    ASSERT_NE(data.WriteInterfaceToken(NetFirewallStub::GetDescriptor()), false);

    if (!data.WriteInt32(TEST_INT32_NUMBER)) {
        return;
    }

    if (!data.WriteInt32(TEST_INT32_NUMBER)) {
        return;
    }

    int32_t ret = SendRemoteRequest(data, (INetFirewallService::GET_NET_FIREWALL_RULE));
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(NetFirewallServiceStubTest, GetInterceptRecord001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    ASSERT_NE(data.WriteInterfaceToken(NetFirewallStub::GetDescriptor()), false);

    if (!data.WriteInt32(TEST_INT32_NUMBER)) {
        return;
    }

    sptr<RequestParam> param = new (std::nothrow) RequestParam();
    param->page = 1;
    param->pageSize = 5;
    param->orderType = NetFirewallOrderType::ORDER_ASC;
    if (!param->Marshalling(data)) {
        NETMGR_EXT_LOG_E("proxy Marshalling failed");
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetFirewallService::GET_ALL_INTERCEPT_RECORDS);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(NetFirewallServiceStubTest, OnRemoteRequest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    ASSERT_NE(data.WriteInterfaceToken(NetFirewallStub::GetDescriptor()), false);
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<NetFirewallStub> instance = std::make_shared<MockNetFirewallServiceStub>();
    int32_t ret = instance->OnRemoteRequest(-1, data, reply, option);
    EXPECT_EQ(ret, IPC_STUB_UNKNOW_TRANS_ERR);
}

HWTEST_F(NetFirewallServiceStubTest, OnSetNetFirewallPolicy001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = instance_->OnSetNetFirewallPolicy(data, reply);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_READ_DATA_FAIL);

    int32_t userId = USER_ID;
    data.WriteInt32(userId);
    ret = instance_->OnSetNetFirewallPolicy(data, reply);
    EXPECT_EQ(ret, FIREWALL_ERR_INTERNAL);
}

HWTEST_F(NetFirewallServiceStubTest, OnGetNetFirewallPolicy001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = instance_->OnGetNetFirewallPolicy(data, reply);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_READ_DATA_FAIL);
}

HWTEST_F(NetFirewallServiceStubTest, OnAddNetFirewallRule001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = instance_->OnAddNetFirewallRule(data, reply);
    EXPECT_EQ(ret, FIREWALL_ERR_INTERNAL);

    sptr<NetFirewallRule> rule = GetNetFirewallRuleSptr(NetFirewallRuleType::RULE_IP, MAX_RULE_IP_COUNT + 1);
    rule->Marshalling(data);
    ret = instance_->OnAddNetFirewallRule(data, reply);
    EXPECT_EQ(ret, FIREWALL_ERR_EXCEED_MAX_IP);
}

HWTEST_F(NetFirewallServiceStubTest, OnAddNetFirewallRule002, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    sptr<NetFirewallRule> rule = GetNetFirewallRuleSptr(NetFirewallRuleType::RULE_IP, 1, MAX_RULE_PORT_COUNT + 1);
    rule->Marshalling(data);
    MessageParcel reply;
    int32_t ret = instance_->OnAddNetFirewallRule(data, reply);
    EXPECT_EQ(ret, FIREWALL_ERR_EXCEED_MAX_PORT);
}

HWTEST_F(NetFirewallServiceStubTest, OnAddNetFirewallRule003, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    sptr<NetFirewallRule> rule = GetNetFirewallRuleSptr(NetFirewallRuleType::RULE_DOMAIN, MAX_RULE_DOMAIN_COUNT + 1);
    rule->Marshalling(data);
    MessageParcel reply;
    int32_t ret = instance_->OnAddNetFirewallRule(data, reply);
    EXPECT_EQ(ret, FIREWALL_ERR_EXCEED_MAX_DOMAIN);
}

HWTEST_F(NetFirewallServiceStubTest, OnUpdateNetFirewallRule001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = instance_->OnUpdateNetFirewallRule(data, reply);
    EXPECT_EQ(ret, FIREWALL_ERR_INTERNAL);

    sptr<NetFirewallRule> rule = GetNetFirewallRuleSptr(NetFirewallRuleType::RULE_IP, MAX_RULE_IP_COUNT + 1);
    rule->Marshalling(data);
    ret = instance_->OnUpdateNetFirewallRule(data, reply);
    EXPECT_EQ(ret, FIREWALL_ERR_EXCEED_MAX_IP);
}

HWTEST_F(NetFirewallServiceStubTest, OnUpdateNetFirewallRule002, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    sptr<NetFirewallRule> rule = GetNetFirewallRuleSptr(NetFirewallRuleType::RULE_IP, 1, MAX_RULE_PORT_COUNT + 1);
    rule->Marshalling(data);
    MessageParcel reply;
    int32_t ret = instance_->OnUpdateNetFirewallRule(data, reply);
    EXPECT_EQ(ret, FIREWALL_ERR_EXCEED_MAX_PORT);
}

HWTEST_F(NetFirewallServiceStubTest, OnUpdateNetFirewallRule003, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    sptr<NetFirewallRule> rule = GetNetFirewallRuleSptr(NetFirewallRuleType::RULE_DOMAIN, MAX_RULE_DOMAIN_COUNT + 1);
    rule->Marshalling(data);
    MessageParcel reply;
    int32_t ret = instance_->OnUpdateNetFirewallRule(data, reply);
    EXPECT_EQ(ret, FIREWALL_ERR_EXCEED_MAX_DOMAIN);
}

HWTEST_F(NetFirewallServiceStubTest, OnDeleteNetFirewallRule001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = instance_->OnDeleteNetFirewallRule(data, reply);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_READ_DATA_FAIL);

    int32_t userId = -1;
    data.WriteInt32(userId);
    ret = instance_->OnDeleteNetFirewallRule(data, reply);
    EXPECT_EQ(ret, FIREWALL_ERR_INVALID_PARAMETER);
}

HWTEST_F(NetFirewallServiceStubTest, OnDeleteNetFirewallRule002, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    int32_t userId = USER_ID;
    data.WriteInt32(userId);
    MessageParcel reply;
    int32_t ret = instance_->OnDeleteNetFirewallRule(data, reply);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_READ_DATA_FAIL);
}

HWTEST_F(NetFirewallServiceStubTest, OnDeleteNetFirewallRule003, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    int32_t userId = USER_ID;
    data.WriteInt32(userId);
    int32_t ruleId = -1;
    data.WriteInt32(ruleId);
    MessageParcel reply;
    int32_t ret = instance_->OnDeleteNetFirewallRule(data, reply);
    EXPECT_EQ(ret, FIREWALL_ERR_INVALID_PARAMETER);
}

HWTEST_F(NetFirewallServiceStubTest, OnGetNetFirewallRules001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = instance_->OnGetNetFirewallRules(data, reply);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_READ_DATA_FAIL);

    int32_t userId = -1;
    data.WriteInt32(userId);
    ret = instance_->OnGetNetFirewallRules(data, reply);
    EXPECT_EQ(ret, FIREWALL_ERR_INVALID_PARAMETER);
}

HWTEST_F(NetFirewallServiceStubTest, OnGetNetFirewallRules002, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    int32_t userId = USER_ID;
    data.WriteInt32(userId);
    MessageParcel reply;
    int32_t ret = instance_->OnGetNetFirewallRules(data, reply);
    EXPECT_EQ(ret, FIREWALL_ERR_INTERNAL);
}

HWTEST_F(NetFirewallServiceStubTest, OnGetNetFirewallRule001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = instance_->OnGetNetFirewallRule(data, reply);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_READ_DATA_FAIL);

    int32_t userId = USER_ID;
    data.WriteInt32(userId);
    ret = instance_->OnGetNetFirewallRule(data, reply);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_READ_DATA_FAIL);
}

HWTEST_F(NetFirewallServiceStubTest, OnGetInterceptRecords001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = instance_->OnGetInterceptRecords(data, reply);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_READ_DATA_FAIL);

    int32_t userId = USER_ID;
    data.WriteInt32(userId);
    ret = instance_->OnGetInterceptRecords(data, reply);
    EXPECT_EQ(ret, FIREWALL_ERR_INTERNAL);
}


HWTEST_F(NetFirewallServiceStubTest, OnRegisterInterceptRecordsCallback001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    std::shared_ptr<NetFirewallStub> instance = std::make_shared<MockNetFirewallServiceStub>();
    ASSERT_NE(instance, nullptr);
    int32_t ret = instance->OnRegisterInterceptRecordsCallback(data, reply);
    EXPECT_EQ(ret, FIREWALL_ERR_INVALID_PARAMETER);
}

HWTEST_F(NetFirewallServiceStubTest, OnUnregisterInterceptRecordsCallback001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    std::shared_ptr<NetFirewallStub> instance = std::make_shared<MockNetFirewallServiceStub>();
    ASSERT_NE(instance, nullptr);
    int32_t ret = instance->OnUnregisterInterceptRecordsCallback(data, reply);
    EXPECT_EQ(ret, FIREWALL_ERR_INVALID_PARAMETER);
}
} // namespace NetManagerStandard
} // namespace OHOS
