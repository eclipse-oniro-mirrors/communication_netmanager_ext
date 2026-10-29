/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "eth_eap_state_callback_stub.h"
#include "net_manager_constants.h"
#include "eth_eap_state_info.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
} // namespace

class MockEthEapStateCallbackStubTest : public EthEapStateCallbackStub {
public:
    MockEthEapStateCallbackStubTest() = default;
    ~MockEthEapStateCallbackStubTest() override = default;

    int32_t OnEthEapStateChanged(const EthEapStateInfo &stateInfo) override
    {
        lastState = stateInfo.state;
        lastRetryCount = stateInfo.retryCount;
        lastMessage = stateInfo.message;
        return NETMANAGER_SUCCESS;
    }

    EthEapState lastState = EthEapState::STATE_IDLE;
    int32_t lastRetryCount = 0;
    std::string lastMessage;
};

class EthEapStateCallbackStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void EthEapStateCallbackStubTest::SetUpTestCase() {}

void EthEapStateCallbackStubTest::TearDownTestCase() {}

void EthEapStateCallbackStubTest::SetUp() {}

void EthEapStateCallbackStubTest::TearDown() {}

/**
 * @tc.name: OnRemoteRequestTest001
 * @tc.desc: Test EthEapStateCallbackStub OnRemoteRequest with invalid descriptor.
 * @tc.type: FUNC
 */
HWTEST_F(EthEapStateCallbackStubTest, OnRemoteRequestTest001, TestSize.Level1)
{
    // 未写接口描述符：接口校验失败
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    MockEthEapStateCallbackStubTest stub;
    int32_t ret = stub.OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: OnRemoteRequestTest002
 * @tc.desc: Test EthEapStateCallbackStub OnRemoteRequest with unknown code.
 * @tc.type: FUNC
 */
HWTEST_F(EthEapStateCallbackStubTest, OnRemoteRequestTest002, TestSize.Level1)
{
    // 未知消息码：转交 IPC 基类处理
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    MockEthEapStateCallbackStubTest stub;
    data.WriteInterfaceToken(IEthEapStateCallback::GetDescriptor());
    int32_t ret = stub.OnRemoteRequest(100, data, reply, option);
    EXPECT_EQ(ret, IPC_STUB_UNKNOW_TRANS_ERR);
}

/**
 * @tc.name: OnRemoteRequestTest003
 * @tc.desc: Test EthEapStateCallbackStub OnRemoteRequest normal dispatch.
 * @tc.type: FUNC
 */
HWTEST_F(EthEapStateCallbackStubTest, OnRemoteRequestTest003, TestSize.Level1)
{
    // 正常分发：状态信息反序列化后回调业务接口
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    MockEthEapStateCallbackStubTest stub;
    data.WriteInterfaceToken(IEthEapStateCallback::GetDescriptor());
    EthEapStateInfo info;
    info.state = EthEapState::STATE_AUTHENTICATED;
    info.retryCount = 2;
    info.message = "authenticated";
    ASSERT_TRUE(info.Marshalling(data));
    int32_t ret = stub.OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    EXPECT_EQ(stub.lastState, EthEapState::STATE_AUTHENTICATED);
    EXPECT_EQ(stub.lastRetryCount, 2);
    EXPECT_EQ(stub.lastMessage, "authenticated");
}

/**
 * @tc.name: OnRemoteRequestTest004
 * @tc.desc: Test EthEapStateCallbackStub OnRemoteRequest with missing state info.
 * @tc.type: FUNC
 */
HWTEST_F(EthEapStateCallbackStubTest, OnRemoteRequestTest004, TestSize.Level1)
{
    // 消息体缺失：状态信息反序列化失败
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    MockEthEapStateCallbackStubTest stub;
    data.WriteInterfaceToken(IEthEapStateCallback::GetDescriptor());
    int32_t ret = stub.OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_LOCAL_PTR_NULL);
}

} // namespace NetManagerStandard
} // namespace OHOS
