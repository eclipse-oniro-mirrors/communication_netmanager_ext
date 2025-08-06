/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include <iostream>
#include <memory>
#include "net_eap_callback_stub.h"
 
namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
class MockNetEapPostbackCallbackStubTest : public NetEapPostbackCallbackStub {
public:
    MockNetEapPostbackCallbackStubTest() = default;
    ~MockNetEapPostbackCallbackStubTest() override {}
    int32_t OnEapSupplicantPostback(NetType netType, const sptr<EapData> &eapData) override
    {
        std::cout << std::endl;
        std::cout << "OnEapSupplicantPostback" << std::endl;
        return 0;
    }
};
 
class MockNetRegisterEapCallbackStubTest : public NetRegisterEapCallbackStub {
public:
    MockNetRegisterEapCallbackStubTest() = default;
    ~MockNetRegisterEapCallbackStubTest() override {}
    int32_t OnRegisterCustomEapCallback(const std::string &regCmd) override
    {
        std::cout << std::endl;
        std::cout << "OnRegisterCustomEapCallback" << std::endl;
        return 0;
    }
 
    int32_t OnReplyCustomEapDataEvent(int result, const sptr<EapData> &eapData) override
    {
        std::cout << std::endl;
        std::cout << "OnReplyCustomEapDataEvent" << std::endl;
        return 0;
    }
};
 
class NetEapCallbackStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
 
    static inline std::shared_ptr<NetEapPostbackCallbackStub> eapPostbackPtr =
        std::make_shared<MockNetEapPostbackCallbackStubTest>();
    static inline std::shared_ptr<NetRegisterEapCallbackStub> registerEapPtr =
        std::make_shared<MockNetRegisterEapCallbackStubTest>();
};
 
void NetEapCallbackStubTest::SetUpTestCase() {}
 
void NetEapCallbackStubTest::TearDownTestCase() {}
 
void NetEapCallbackStubTest::SetUp() {}
 
void NetEapCallbackStubTest::TearDown() {}
 
/**
 * @tc.name: OnRemoteRequest001
 * @tc.desc: Test NetEapCallbackStubTest OnRemoteRequest.
 * @tc.type: FUNC
 */
HWTEST_F(NetEapCallbackStubTest, OnRemoteRequest001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int32_t ret = eapPostbackPtr->OnRemoteRequest(100, data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_ERR_DESCRIPTOR_MISMATCH);
 
    data.WriteInterfaceToken(NetEapPostbackCallbackStub::GetDescriptor());
    ret = eapPostbackPtr->OnRemoteRequest(100, data, reply, option);
    EXPECT_EQ(ret, IPC_STUB_UNKNOW_TRANS_ERR);
}
 
/**
 * @tc.name: OnEapSupplicantPostbackTest001
 * @tc.desc: Test NetEapPostbackCallbackStub OnEapSupplicantPostback.
 * @tc.type: FUNC
 */
HWTEST_F(NetEapCallbackStubTest, OnEapSupplicantPostbackTest001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(NetEapPostbackCallbackStub::GetDescriptor());
    data.WriteInt32(1);
    sptr<EapData> eapData = new (std::nothrow) EapData();
    eapData->eapCode = 1;
    eapData->eapType = 13;
    eapData->msgId = 55;
    eapData->bufferLen = 4;
    std::vector<uint8_t> tmp = {0x11, 0x12};
    eapData->eapBuffer = tmp;
    eapData->Marshalling(data);
    int32_t ret = eapPostbackPtr->OnRemoteRequest(static_cast<uint32_t>(NetEapIpcCode::NET_EAP_POSTBACK),
                                             data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}
 
/**
 * @tc.name: OnRegisterCustomEapCallbackTest001
 * @tc.desc: Test NetRegisterEapCallbackStub OnRegisterCustomEapCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetEapCallbackStubTest, OnRegisterCustomEapCallbackTest001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(NetRegisterEapCallbackStub::GetDescriptor());
    std::string regCmd = "2:277:278";
    data.WriteString(regCmd);
    int32_t ret = registerEapPtr->OnRemoteRequest(static_cast<uint32_t>(NetEapIpcCode::NET_REPLY_CUSTOM_EAPDATA),
                                             data, reply, option);
    EXPECT_TRUE(ret == NETMANAGER_SUCCESS || ret == NETMANAGER_ERR_LOCAL_PTR_NULL);
}
 
/**
 * @tc.name: OnReplyCustomEapDataEventTest001
 * @tc.desc: Test NetRegisterEapCallbackStub OnReplyCustomEapDataEvent.
 * @tc.type: FUNC
 */
HWTEST_F(NetEapCallbackStubTest, OnReplyCustomEapDataEventTest001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(NetRegisterEapCallbackStub::GetDescriptor());
    int result = 1;
    std::string strEapData = "55:4:abcd";
 
    data.WriteInt32(result);
    data.WriteString(strEapData);
    int32_t ret = registerEapPtr->OnRemoteRequest(static_cast<uint32_t>(NetEapIpcCode::NET_REPLY_CUSTOM_EAPDATA),
                                             data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}
 
} // namespace NetManagerStandard
} // namespace OHOS