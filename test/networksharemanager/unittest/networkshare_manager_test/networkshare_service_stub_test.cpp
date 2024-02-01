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

#include "mock_isharing_event_callback_test.h"
#include "net_manager_constants.h"
#include "netmanager_ext_test_security.h"
#include "networkshare_service_stub_test.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr int32_t TEST_INT32_NUMBER = 1;
} // namespace

using namespace testing::ext;
class NetworkShareServiceStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<NetworkShareServiceStub> instance_ = std::make_shared<MockNetworkShareServiceStub>();
    static int32_t SendRemoteRequest(MessageParcel &data, TetheringInterfaceCode code);
};

void NetworkShareServiceStubTest::SetUpTestCase() {}

void NetworkShareServiceStubTest::TearDownTestCase() {}

void NetworkShareServiceStubTest::SetUp() {}

void NetworkShareServiceStubTest::TearDown() {}

int32_t NetworkShareServiceStubTest::SendRemoteRequest(MessageParcel &data, TetheringInterfaceCode code)
{
    MessageParcel reply;
    MessageOption option;
    return instance_->OnRemoteRequest(static_cast<uint32_t>(code), data, reply, option);
}

HWTEST_F(NetworkShareServiceStubTest, ReplyIsNetworkSharingSupportedTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkShareServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, TetheringInterfaceCode::CMD_GET_SHARING_SUPPORTED);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareServiceStubTest, ReplyIsSharingTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkShareServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, TetheringInterfaceCode::CMD_GET_IS_SHARING);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareServiceStubTest, ReplyStartNetworkSharingTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkShareServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteInt32(TEST_INT32_NUMBER)) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, TetheringInterfaceCode::CMD_START_NETWORKSHARE);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareServiceStubTest, ReplyStopNetworkSharingTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkShareServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteInt32(TEST_INT32_NUMBER)) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, TetheringInterfaceCode::CMD_STOP_NETWORKSHARE);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareServiceStubTest, ReplyGetSharableRegexsTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkShareServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteInt32(TEST_INT32_NUMBER)) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, TetheringInterfaceCode::CMD_GET_SHARABLE_REGEXS);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareServiceStubTest, ReplyGetSharingStateTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkShareServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteInt32(TEST_INT32_NUMBER)) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, TetheringInterfaceCode::CMD_GET_SHARING_STATE);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareServiceStubTest, ReplyGetNetSharingIfacesTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkShareServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteInt32(TEST_INT32_NUMBER)) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, TetheringInterfaceCode::CMD_GET_SHARING_IFACES);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareServiceStubTest, ReplyRegisterSharingEventTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkShareServiceStub::GetDescriptor())) {
        return;
    }
    sptr<ISharingEventCallback> callback = new (std::nothrow) MockISharingEventCallback();
    if (!data.WriteRemoteObject(callback->AsObject())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, TetheringInterfaceCode::CMD_REGISTER_EVENT_CALLBACK);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareServiceStubTest, ReplyUnregisterSharingEventTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkShareServiceStub::GetDescriptor())) {
        return;
    }
    sptr<ISharingEventCallback> callback = new (std::nothrow) MockISharingEventCallback();
    if (!data.WriteRemoteObject(callback->AsObject())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, TetheringInterfaceCode::CMD_UNREGISTER_EVENT_CALLBACK);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareServiceStubTest, ReplyGetStatsRxBytesTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkShareServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, TetheringInterfaceCode::CMD_GET_RX_BYTES);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareServiceStubTest, ReplyGetStatsTxBytesTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkShareServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, TetheringInterfaceCode::CMD_GET_TX_BYTES);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareServiceStubTest, ReplyGetStatsTotalBytesTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkShareServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, TetheringInterfaceCode::CMD_GET_TOTAL_BYTES);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
