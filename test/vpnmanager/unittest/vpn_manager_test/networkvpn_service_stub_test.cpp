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
#include "networkvpn_service_stub.h"
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
    static inline std::shared_ptr<NetworkVpnServiceStub> instance_ = std::make_shared<MockNetworkVpnServiceStub>();
    static int32_t SendRemoteRequest(MessageParcel &data, INetworkVpnService::MessageCode code);
};

void NetworkVpnServiceStubTest::SetUpTestCase() {}

void NetworkVpnServiceStubTest::TearDownTestCase() {}

void NetworkVpnServiceStubTest::SetUp() {}

void NetworkVpnServiceStubTest::TearDown() {}

int32_t NetworkVpnServiceStubTest::SendRemoteRequest(MessageParcel &data, INetworkVpnService::MessageCode code)
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
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_PREPARE);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplySetUpVpnTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
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
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_START_VPN);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyProtectTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_PROTECT);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyDestroyVpnTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_STOP_VPN);
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
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_REGISTER_EVENT_CALLBACK);
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
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_UNREGISTER_EVENT_CALLBACK);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyCreateVpnConnectionTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_CREATE_VPN_CONNECTION);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyCreateVpnConnectionTest002, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_CREATE_VPN_CONNECTION_EXT);
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
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_REGISTER_EVENT_CALLBACK);
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
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_UNREGISTER_EVENT_CALLBACK);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyFactoryResetVpnTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_FACTORYRESET_VPN);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
