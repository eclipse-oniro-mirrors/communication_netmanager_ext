/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "mock_networkvpn_service_stub_test.h"
#include "mock_vpn_event_callback_test.h"
#include "sharing_event_callback_stub.h"
#include "net_manager_constants.h"
#include "network_vpn_service_stub.h"
#include "netmanager_ext_test_security.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
class NetworkVpnServiceStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<MockNetworkVpnServiceStub> instance_ = std::make_shared<MockNetworkVpnServiceStub>();
    static int32_t SendRemoteRequest(MessageParcel &data, INetworkVpnServiceIpcCode code);
};

void NetworkVpnServiceStubTest::SetUpTestCase() {}

void NetworkVpnServiceStubTest::TearDownTestCase() {}

void NetworkVpnServiceStubTest::SetUp() {}

void NetworkVpnServiceStubTest::TearDown() {}

int32_t NetworkVpnServiceStubTest::SendRemoteRequest(MessageParcel &data, INetworkVpnServiceIpcCode code)
{
    MessageParcel reply;
    MessageOption option;
    return instance_->OnRemoteRequest(static_cast<uint32_t>(code), data, reply, option);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyPrepareTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnServiceIpcCode::COMMAND_PREPARE);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplySetUpVpnTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    VpnConfig* config = new (std::nothrow) VpnConfig();
    EXPECT_TRUE(config != nullptr);
    if (config == nullptr) {
        return;
    }
    config->isAcceptIPv4_ = false;
    config->isAcceptIPv6_ = false;
    config->isLegacy_ = false;
    config->isMetered_ = false;
    config->isBlocking_ = false;
    if (!config->Marshalling(data)) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnServiceIpcCode::COMMAND_SET_UP_VPN);
    EXPECT_NE(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyVpnappTest001, TestSize.Level1)
{
    MessageParcel data;
    uint32_t maxsize = 2;
    ASSERT_TRUE(data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor()));
    std::shared_ptr<VpnConfig> config = std::make_shared<VpnConfig>();
    ASSERT_TRUE(config != nullptr);
    config->acceptedApplications_ = {"com.qq.reader", "com.tencent.mm", "com.quark.browser"};
    ASSERT_TRUE(config->Marshalling(data));
    ASSERT_FALSE(config->UnmarshallingVectorString(data, config->acceptedApplications_, maxsize));
    delete config;
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyProtectTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnServiceIpcCode::COMMAND_PROTECT);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyDestroyVpnTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnServiceIpcCode::COMMAND_DESTROY_VPN);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyRegisterVpnEventTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    sptr<IVpnEventCallback> callback = new (std::nothrow) MockIVpnEventCallback();
    if (!data.WriteRemoteObject(callback->AsObject())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnServiceIpcCode::COMMAND_REGISTER_VPN_EVENT);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyUnregisterVpnEventTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    sptr<IVpnEventCallback> callback = new (std::nothrow) MockIVpnEventCallback();
    if (!data.WriteRemoteObject(callback->AsObject())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnServiceIpcCode::COMMAND_UNREGISTER_VPN_EVENT);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyCreateVpnConnectionTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnServiceIpcCode::COMMAND_CREATE_VPN_CONNECTION);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyRegisterSharingEventTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    sptr<IVpnEventCallback> callback = new (std::nothrow) MockIVpnEventCallback();
    if (!data.WriteRemoteObject(callback->AsObject())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnServiceIpcCode::COMMAND_REGISTER_VPN_EVENT);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyUnregisterSharingEventTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    sptr<IVpnEventCallback> callback = new (std::nothrow) MockIVpnEventCallback();
    if (!data.WriteRemoteObject(callback->AsObject())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnServiceIpcCode::COMMAND_UNREGISTER_VPN_EVENT);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyFactoryResetVpnTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnServiceIpcCode::COMMAND_FACTORY_RESET_VPN);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyRegisterBundleNameVpnTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    sptr<IVpnEventCallback> callback = new (std::nothrow) MockIVpnEventCallback();
    if (!data.WriteRemoteObject(callback->AsObject())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnServiceIpcCode::COMMAND_REGISTER_BUNDLE_NAME);
    EXPECT_TRUE(ret == NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL || ret == NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyDefaultTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    uint32_t code = static_cast<uint32_t>(INetworkVpnServiceIpcCode::COMMAND_REGISTER_BUNDLE_NAME) +
                    static_cast<uint32_t>(INetworkVpnServiceIpcCode::COMMAND_REGISTER_BUNDLE_NAME);
    int32_t ret = SendRemoteRequest(data, static_cast<INetworkVpnServiceIpcCode>(code));
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
