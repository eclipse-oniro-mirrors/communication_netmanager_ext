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
constexpr int32_t USER_ID = 100;
constexpr int32_t TEST_INT32_NUMBER = 1;

using namespace testing::ext;
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
    if (!data.WriteInterfaceToken(NetFirewallStub::GetDescriptor())) {
        return;
    }

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
    if (!data.WriteInterfaceToken(NetFirewallStub::GetDescriptor())) {
        return;
    }

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
    if (!data.WriteInterfaceToken(NetFirewallStub::GetDescriptor())) {
        return;
    }

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
    if (!data.WriteInterfaceToken(NetFirewallStub::GetDescriptor())) {
        return;
    }

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
    if (!data.WriteInterfaceToken(NetFirewallStub::GetDescriptor())) {
        return;
    }

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
    if (!data.WriteInterfaceToken(NetFirewallStub::GetDescriptor())) {
        return;
    }

    if (!data.WriteInt32(TEST_INT32_NUMBER)) {
        return;
    }
    sptr<RequestParam> param = new (std::nothrow) RequestParam();
    param->page = 0;
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
    if (!data.WriteInterfaceToken(NetFirewallStub::GetDescriptor())) {
        return;
    }

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
    if (!data.WriteInterfaceToken(NetFirewallStub::GetDescriptor())) {
        return;
    }

    if (!data.WriteInt32(TEST_INT32_NUMBER)) {
        return;
    }

    sptr<RequestParam> param = new (std::nothrow) RequestParam();
    param->page = 0;
    param->pageSize = 5;
    param->orderType = NetFirewallOrderType::ORDER_ASC;
    if (!param->Marshalling(data)) {
        NETMGR_EXT_LOG_E("proxy Marshalling failed");
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetFirewallService::GET_ALL_INTERCEPT_RECORDS);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
