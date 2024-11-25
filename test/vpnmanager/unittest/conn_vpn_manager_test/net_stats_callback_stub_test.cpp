/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "common_net_stats_callback_test.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr uint64_t OUTOFFRANGECODE = 100;
} // namespace

using namespace testing::ext;
class NetStatsCallbackStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<NetStatsCallbackStub> instance_ = std::make_shared<NetStatsCallbackTestCb>();
};

void NetStatsCallbackStubTest::SetUpTestCase() {}

void NetStatsCallbackStubTest::TearDownTestCase() {}

void NetStatsCallbackStubTest::SetUp() {}

void NetStatsCallbackStubTest::TearDown() {}

/**
 * @tc.name: OnRemoteRequestTest001
 * @tc.desc: Test NetStatsCallbackStub OnRemoteRequest.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsCallbackStubTest, OnRemoteRequestTest001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(OUTOFFRANGECODE, data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_ERR_DESCRIPTOR_MISMATCH);
}

/**
 * @tc.name: OnRemoteRequestTest002
 * @tc.desc: Test NetStatsCallbackStub OnRemoteRequest.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsCallbackStubTest, OnRemoteRequestTest002, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetStatsCallbackStub::GetDescriptor())) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(OUTOFFRANGECODE, data, reply, option);
    EXPECT_EQ(ret, IPC_STUB_UNKNOW_TRANS_ERR);
}

/**
 * @tc.name: OnRemoteRequestTest003
 * @tc.desc: Test NetStatsCallbackStub OnRemoteRequest.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsCallbackStubTest, OnRemoteRequestTest003, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetStatsCallbackStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString("test")) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(StatsCallBackInterfaceCode::NET_STATS_IFACE_CHANGED),
                                             data, reply, option);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: OnNetIfaceStatsChangedTest001
 * @tc.desc: Test NetStatsCallbackStub OnNetIfaceStatsChanged.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsCallbackStubTest, OnNetIfaceStatsChangedTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetStatsCallbackStub::GetDescriptor())) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(StatsCallBackInterfaceCode::NET_STATS_IFACE_CHANGED),
                                             data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_ERR_READ_DATA_FAIL);
}

/**
 * @tc.name: OnNetIfaceStatsChangedTest002
 * @tc.desc: Test NetStatsCallbackStub OnNetIfaceStatsChanged.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsCallbackStubTest, OnNetIfaceStatsChangedTest002, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetStatsCallbackStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString("test")) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(StatsCallBackInterfaceCode::NET_STATS_IFACE_CHANGED),
                                             data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnNetUidStatsChangedTest001
 * @tc.desc: Test NetStatsCallbackStub OnNetUidStatsChanged.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsCallbackStubTest, OnNetUidStatsChangedTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetStatsCallbackStub::GetDescriptor())) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(StatsCallBackInterfaceCode::NET_STATS_UID_CHANGED),
                                             data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_ERR_READ_DATA_FAIL);
}

/**
 * @tc.name: OnNetUidStatsChangedTest002
 * @tc.desc: Test NetStatsCallbackStub OnNetUidStatsChanged.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsCallbackStubTest, OnNetUidStatsChangedTest002, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetStatsCallbackStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString("test")) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(StatsCallBackInterfaceCode::NET_STATS_UID_CHANGED),
                                             data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_ERR_READ_DATA_FAIL);
}

/**
 * @tc.name: OnNetUidStatsChangedTest003
 * @tc.desc: Test NetStatsCallbackStub OnNetUidStatsChanged.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsCallbackStubTest, OnNetUidStatsChangedTest003, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetStatsCallbackStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString("test")) {
        return;
    }
    if (!data.WriteUint32(1)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(StatsCallBackInterfaceCode::NET_STATS_UID_CHANGED),
                                             data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS