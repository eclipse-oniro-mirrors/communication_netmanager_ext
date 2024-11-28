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

#include "net_push_stats_info.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr int32_t TEST_UID = 20011212;
constexpr int32_t TEST_SIM_ID = 1;
constexpr int32_t TEST_TYPE = 0;
constexpr int32_t TEST_BEGIN_TIME = 179122111;
constexpr int32_t TEST_END_TIME = 179123233;
constexpr int32_t TEST_RX_BYTES = 7894;
constexpr int32_t TEST_TX_BYTES = 8923;
PushStatsInfo GetPushStatsInfo()
{
    PushStatsInfo info;
    info.uid_ = TEST_UID;
    info.simId_ = TEST_SIM_ID;
    info.netBearType_ = TEST_TYPE;
    info.beginTime_ = TEST_BEGIN_TIME;
    info.endTime_ = TEST_END_TIME;
    info.rxBytes_ = TEST_RX_BYTES;
    info.txBytes_ = TEST_TX_BYTES;
    return info;
}
} // namespace

using namespace testing::ext;
class PushStatsInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    uint32_t GetTestTime();
};

void PushStatsInfoTest::SetUpTestCase() {}

void PushStatsInfoTest::TearDownTestCase() {}

void PushStatsInfoTest::SetUp() {}

void PushStatsInfoTest::TearDown() {}

/**
 * @tc.name: MarshallingUnmarshallingTest001
 * @tc.desc: Test NetStatsInfo Marshalling and Unmarshalling.
 * @tc.type: FUNC
 */
HWTEST_F(PushStatsInfoTest, MarshallingUnmarshallingTest001, TestSize.Level1)
{
    Parcel parcel;
    PushStatsInfo info = GetPushStatsInfo();
    EXPECT_TRUE(info.Marshalling(parcel));
    PushStatsInfo result;
    EXPECT_TRUE(PushStatsInfo::Unmarshalling(parcel, result));
    EXPECT_EQ(result.uid_, info.uid_);
    EXPECT_EQ(result.iface_, info.iface_);
    EXPECT_EQ(result.simId_, info.simId_);
    EXPECT_EQ(result.netBearType_, info.netBearType_);
    EXPECT_EQ(result.beginTime_, info.beginTime_);
    EXPECT_EQ(result.endTime_, info.endTime_);
    EXPECT_EQ(result.rxBytes_, info.rxBytes_);
    EXPECT_EQ(result.txBytes_, info.txBytes_);
}

/**
 * @tc.name: MarshallingUnmarshallingTest002
 * @tc.desc: Test NetStatsInfo Marshalling and Unmarshalling.
 * @tc.type: FUNC
 */
HWTEST_F(PushStatsInfoTest, MarshallingUnmarshallingTest002, TestSize.Level1)
{
    Parcel parcel;
    PushStatsInfo info = GetPushStatsInfo();
    EXPECT_TRUE(PushStatsInfo::Marshalling(parcel, info));
    PushStatsInfo result;
    EXPECT_TRUE(PushStatsInfo::Unmarshalling(parcel, result));
    EXPECT_EQ(result.uid_, info.uid_);
    EXPECT_EQ(result.iface_, info.iface_);
    EXPECT_EQ(result.simId_, info.simId_);
    EXPECT_EQ(result.netBearType_, info.netBearType_);
    EXPECT_EQ(result.beginTime_, info.beginTime_);
    EXPECT_EQ(result.endTime_, info.endTime_);
    EXPECT_EQ(result.rxBytes_, info.rxBytes_);
    EXPECT_EQ(result.txBytes_, info.txBytes_);
}
} // namespace NetManagerStandard
} // namespace OHOS