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

#include "gtest/gtest-message.h"
#include "gtest/gtest-test-part.h"
#include "gtest/hwext/gtest-ext.h"
#include "gtest/hwext/gtest-tag.h"
#include "http_proxy.h"
#include "interface_type.h"
#include "net_manager_constants.h"
#include "netmanager_ext_test_security.h"
#include "netmgr_ext_log_wrapper.h"
#include "refbase.h"

#define private public
#define protected public

#include "netfirewall_client.h"
#include "netfirewall_common.h"
#include "netfirewall_proxy.h"
#include "netfirewall_service.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;

const int32_t USER_ID = 100;
const int32_t PAGE = 0;
const int32_t PAGE_SIZE = 5;
const int32_t TOTAL_PAGE = 10;
const int32_t APP_ID = 2034;
const uint8_t MASK = 24;
const uint16_t LOCAL_START_PORT = 10020;
const uint16_t LOCAL_END_PORT = 1003;
const uint16_t REMOTE_START_PORT = 1002;
const uint16_t REMOTE_END_PORT = 10030;

sptr<NetFirewallRule> GetNetFirewallRuleSptr()
{
    sptr<NetFirewallRule> rule = (std::make_unique<NetFirewallRule>()).release();
    if (!rule) {
        return rule;
    }
    rule->ruleId = 1;
    rule->userId = USER_ID;
    rule->ruleName = "rule test";
    rule->ruleDescription = "AddNetFirewallRule 001";
    rule->ruleDirection = NetFirewallRuleDirection::RULE_OUT;
    rule->ruleAction = FirewallRuleAction::RULE_ALLOW;
    rule->isEnabled = false;
    rule->appUid = APP_ID;

    NetFirewallIpParam localParam;
    localParam.family = 1;
    localParam.type = 1;
    inet_pton(AF_INET, "192.168.10.1", &localParam.ipv4.startIp);
    localParam.mask = MASK;
    rule->localIps.push_back(localParam);

    NetFirewallIpParam remoteParam;
    remoteParam.family = FAMILY_IPV6;
    remoteParam.type = 1;
    inet_pton(AF_INET6, "fe80::6bec:e9b9:a1df:f69d", &localParam.ipv6.startIp);
    remoteParam.mask = MASK;
    rule->remoteIps.push_back(remoteParam);

    NetFirewallPortParam localPortParam;
    localPortParam.startPort = LOCAL_START_PORT;
    localPortParam.endPort = LOCAL_END_PORT;
    rule->localPorts.push_back(localPortParam);

    NetFirewallPortParam remotePortParam;
    remotePortParam.startPort = REMOTE_START_PORT;
    remotePortParam.endPort = REMOTE_END_PORT;
    rule->remotePorts.push_back(remotePortParam);

    NetFirewallDomainParam domain;
    domain.isWildcard = 1;
    domain.domain = "www.openharmony.cn";
    rule->domains.push_back(domain);
    rule->dns.primaryDns = "192.168.1.245";
    rule->dns.standbyDns = "192.168.1.1";

    return rule;
}

sptr<NetFirewallPolicy> GetNetFirewallPolicySptr()
{
    sptr<NetFirewallPolicy> status = (std::make_unique<NetFirewallPolicy>()).release();
    status->isOpen = true;
    status->inAction = FirewallRuleAction::RULE_DENY;
    status->outAction = FirewallRuleAction::RULE_ALLOW;
    return status;
}

sptr<InterceptRecord> GetInterceptRecordSptr()
{
    sptr<InterceptRecord> record = (std::make_unique<InterceptRecord>()).release();
    const uint32_t time = 10025152;
    const uint16_t localPort = 10000;
    const uint16_t remotePort = 20000;
    const uint32_t uid = 10085;
    record->time = time;
    record->localIp = "192.168.1.2";
    record->remoteIp = "192.168.1.3";
    record->localPort = localPort;
    record->remotePort = remotePort;
    record->protocol = 1;
    record->appUid = uid;
    return record;
}

sptr<NetTrafficFilterConfig> GetNetTrafficFilterConfigSptr()
{
    sptr<NetTrafficFilterConfig> config = (std::make_unique<NetTrafficFilterConfig>()).release();
    config->packetCopyLen = 1024;
    config->nfqueueMaxlen = 1024;
    config->nfqueueFlags = 1;
    return config;
}

sptr<TrafficFilterRedirectRule> GetTrafficFilterRedirectRuleSptr()
{
    sptr<TrafficFilterRedirectRule> rule = (std::make_unique<TrafficFilterRedirectRule>()).release();
    rule->priority_ = 100;
    rule->hookPoint_ = static_cast<int32_t>(TrafficFilterHookPoint::HOOK_PREROUTING);
    rule->protocol_ = 6; // TCP

    rule->srcIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY);
    rule->dstIp_.type_ = static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_SINGLE);
    inet_pton(AF_INET, "93.184.216.34", rule->dstIp_.single_.addr_);

    rule->srcPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY);
    rule->dstPort_.type_ = static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_SINGLE);
    rule->dstPort_.single_ = 443;

    rule->uidStart_ = 10042;
    rule->uidEnd_ = 10042;

    rule->proxyIp_.family_ = static_cast<int32_t>(TrafficFilterIPFamily::IP_FAMILY_V4);
    inet_pton(AF_INET, "127.0.0.1", rule->proxyIp_.addr_);
    rule->proxyPort_ = 8080;

    return rule;
}
} // namespace

class MockNetIRemoteObject : public IRemoteObject {
public:
    MockNetIRemoteObject() : IRemoteObject(u"mock_i_remote_object") {}
    ~MockNetIRemoteObject() {}

    int32_t GetObjectRefCount() override
    {
        return 0;
    }

    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        switch (code) {
            case static_cast<uint32_t>(INetFirewallService::ADD_NET_FIREWALL_RULE):
                reply.WriteInt32(1);
                break;
            case static_cast<uint32_t>(INetFirewallService::GET_NET_FIREWALL_STATUS):
                GetNetFirewallPolicySptr()->Marshalling(reply);
                break;
            case static_cast<uint32_t>(INetFirewallService::GET_NET_FIREWALL_RULE):
                GetNetFirewallRuleSptr()->Marshalling(reply);
                break;
            case static_cast<uint32_t>(INetFirewallService::GET_ALL_NET_FIREWALL_RULES): {
                std::vector<NetFirewallRule> ruleList;
                reply.WriteInt32(PAGE);
                reply.WriteInt32(PAGE_SIZE);
                reply.WriteInt32(TOTAL_PAGE);
                sptr<NetFirewallRule> rule = GetNetFirewallRuleSptr();
                if (rule != nullptr) {
                    ruleList.push_back(*rule);
                }
                reply.WriteUint32(ruleList.size());
                for (auto &rule : ruleList) {
                    rule.Marshalling(reply);
                }
                break;
            }
            case static_cast<uint32_t>(INetFirewallService::SET_NET_FIREWALL_STATUS):
            case static_cast<uint32_t>(INetFirewallService::DELETE_NET_FIREWALL_RULE):
            case static_cast<uint32_t>(INetFirewallService::UPDATE_NET_FIREWALL_RULE):
                reply.WriteInt32(1);
                break;
            case static_cast<uint32_t>(INetFirewallService::GET_ALL_INTERCEPT_RECORDS): {
                std::vector<InterceptRecord> recordList;
                reply.WriteInt32(PAGE);
                reply.WriteInt32(PAGE_SIZE);
                reply.WriteInt32(TOTAL_PAGE);
                sptr<InterceptRecord> record = GetInterceptRecordSptr();
                recordList.push_back(*record);
                reply.WriteUint32(recordList.size());
                for (auto rule : recordList) {
                    rule.Marshalling(reply);
                }
                break;
            }
            case static_cast<uint32_t>(INetFirewallService::CREATE_REDIRECTOR):
                reply.WriteString("redirector_test_123");
                break;
            case static_cast<uint32_t>(INetFirewallService::DESTROY_REDIRECTOR):
            case static_cast<uint32_t>(INetFirewallService::ADD_REDIRECT_RULE):
            case static_cast<uint32_t>(INetFirewallService::CLEAR_REDIRECT_RULE):
            case static_cast<uint32_t>(INetFirewallService::GLOBAL_ENABLE_TRAFFIC_FILTER):
            case static_cast<uint32_t>(INetFirewallService::GLOBAL_DISABLE_TRAFFIC_FILTER):
                reply.WriteInt32(1);
                break;
            case static_cast<uint32_t>(INetFirewallService::GET_TRAFFIC_FILTER_GLOBAL_STATUS):
                reply.WriteBool(true);
                break;
            case static_cast<uint32_t>(INetFirewallService::QUERY_PROCESS):
                reply.WriteUint32(10042);
                reply.WriteUint32(12345);
                break;
            default:
                reply.WriteInt32(1);
                break;
        }
        return eCode;
    }

    bool IsProxyObject() const override
    {
        return true;
    }

    bool CheckObjectLegality() const override
    {
        return true;
    }

    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }

    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }

    bool Marshalling(Parcel &parcel) const override
    {
        return true;
    }

    sptr<IRemoteBroker> AsInterface() override
    {
        return nullptr;
    }

    int Dump(int fd, const std::vector<std::u16string> &args) override
    {
        return 0;
    }

    std::u16string GetObjectDescriptor() const
    {
        std::u16string descriptor = std::u16string();
        return descriptor;
    }

    void SetErrorCode(int errorCode)
    {
        eCode = errorCode;
    }

private:
    int eCode = FIREWALL_SUCCESS;
};

class NetFirewallServiceProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline sptr<MockNetIRemoteObject> remoteObj_ = std::make_unique<MockNetIRemoteObject>().release();
    static inline std::shared_ptr<NetFirewallProxy> instance_ = std::make_shared<NetFirewallProxy>(remoteObj_);
};

void NetFirewallServiceProxyTest::SetUpTestCase() {}

void NetFirewallServiceProxyTest::TearDownTestCase() {}

void NetFirewallServiceProxyTest::SetUp() {}

void NetFirewallServiceProxyTest::TearDown() {}

HWTEST_F(NetFirewallServiceProxyTest, SetNetFirewallPolicy001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    int32_t userId = USER_ID;
    sptr<NetFirewallPolicy> status = GetNetFirewallPolicySptr();
    auto ret = instance_->SetNetFirewallPolicy(userId, status);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(NetFirewallServiceProxyTest, AddNetFirewallRule001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    sptr<NetFirewallRule> rule = GetNetFirewallRuleSptr();
    int32_t ruleId = 0;
    auto ret = instance_->AddNetFirewallRule(rule, ruleId);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(NetFirewallServiceProxyTest, GetNetFirewallPolicy001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    int32_t userId = USER_ID;
    sptr<NetFirewallPolicy> status = new (std::nothrow) NetFirewallPolicy;
    auto ret = instance_->GetNetFirewallPolicy(userId, status);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(NetFirewallServiceProxyTest, UpdateNetFirewallRule001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    sptr<NetFirewallRule> rule = GetNetFirewallRuleSptr();
    auto ret = instance_->UpdateNetFirewallRule(rule);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(NetFirewallServiceProxyTest, GetAllNetFirewallRule001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    int32_t userId = USER_ID;
    sptr<RequestParam> param = new (std::nothrow) RequestParam();
    param->page = 1;
    param->pageSize = 5;
    param->orderField = NetFirewallOrderField::ORDER_BY_RULE_NAME;
    param->orderType = NetFirewallOrderType::ORDER_ASC;
    sptr<FirewallRulePage> info = new (std::nothrow) FirewallRulePage();
    auto ret = instance_->GetNetFirewallRules(userId, param, info);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(NetFirewallServiceProxyTest, GetNetFirewallRule001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    int32_t ruleId = 1;
    int32_t userId = USER_ID;
    sptr<NetFirewallRule> rule = new (std::nothrow) NetFirewallRule;
    auto ret = instance_->GetNetFirewallRule(userId, ruleId, rule);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(NetFirewallServiceProxyTest, DeleteNetFirewallRule001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    int32_t userId = USER_ID;
    int32_t ruleId = 1;
    auto ret = instance_->DeleteNetFirewallRule(userId, ruleId);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(NetFirewallServiceProxyTest, GetInterceptRecord001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    int32_t userId = USER_ID;
    sptr<RequestParam> param = new (std::nothrow) RequestParam();
    param->page = 1;
    param->pageSize = 5;
    param->orderField = NetFirewallOrderField::ORDER_BY_RECORD_TIME;
    param->orderType = NetFirewallOrderType::ORDER_ASC;
    sptr<InterceptRecordPage> info = new (std::nothrow) InterceptRecordPage();
    auto ret = instance_->GetInterceptRecords(userId, param, info);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(NetFirewallServiceProxyTest, CreateRedirector001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    uint32_t groupId = 1001;
    uint32_t priority = 100;
    sptr<NetTrafficFilterConfig> config = GetNetTrafficFilterConfigSptr();
    std::string redirectorId;
    auto ret = instance_->CreateRedirector(groupId, priority, config, redirectorId);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
    EXPECT_EQ(redirectorId, "redirector_test_123");
}

HWTEST_F(NetFirewallServiceProxyTest, DestroyRedirector001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    std::string redirectorId = "redirector_test_123";
    auto ret = instance_->DestroyRedirector(redirectorId);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(NetFirewallServiceProxyTest, AddRedirectRule001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    std::string redirectorId = "redirector_test_123";
    sptr<TrafficFilterRedirectRule> rule = GetTrafficFilterRedirectRuleSptr();
    auto ret = instance_->AddRedirectRule(redirectorId, rule);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(NetFirewallServiceProxyTest, ClearRedirectRule001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    std::string redirectorId = "redirector_test_123";
    auto ret = instance_->ClearRedirectRule(redirectorId);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(NetFirewallServiceProxyTest, GlobalEnableTrafficFilter001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    auto ret = instance_->GlobalEnableTrafficFilter();
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(NetFirewallServiceProxyTest, GlobalDisableTrafficFilter001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    auto ret = instance_->GlobalDisableTrafficFilter();
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(NetFirewallServiceProxyTest, GetTrafficFilterGlobalStatus001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    bool isEnabled = false;
    auto ret = instance_->GetTrafficFilterGlobalStatus(isEnabled);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
    EXPECT_TRUE(isEnabled);
}

HWTEST_F(NetFirewallServiceProxyTest, QueryProcess001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    std::string srcIp = "192.168.1.100";
    uint16_t srcPort = 50000;
    std::string dstIp = "93.184.216.34";
    uint16_t dstPort = 443;
    uint8_t protocol = 6; // TCP
    uint32_t uid = 0;
    uint32_t pid = 0;
    auto ret = instance_->QueryProcess(srcIp, srcPort, dstIp, dstPort, protocol, uid, pid);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
    EXPECT_EQ(uid, 10042);
    EXPECT_EQ(pid, 12345);
}

HWTEST_F(NetFirewallServiceProxyTest, QueryProcess002, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    std::string srcIp = "192.168.1.100";
    uint16_t srcPort = 50001;
    std::string dstIp = "8.8.8.8";
    uint16_t dstPort = 53;
    uint8_t protocol = 17; // UDP
    uint32_t uid = 0;
    uint32_t pid = 0;
    auto ret = instance_->QueryProcess(srcIp, srcPort, dstIp, dstPort, protocol, uid, pid);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
    EXPECT_EQ(uid, 10042);
    EXPECT_EQ(pid, 12345);
}
} // namespace NetManagerStandard
} // namespace OHOS