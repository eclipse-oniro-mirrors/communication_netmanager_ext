/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "networkshare_hisysevent.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
constexpr int32_t SHARING_COUNT_VALUE = 5;
constexpr const char *ERROR_MSG = "Test";
class NetworkShareHisysEventTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetworkShareHisysEventTest::SetUpTestCase() {}

void NetworkShareHisysEventTest::TearDownTestCase() {}

void NetworkShareHisysEventTest::SetUp() {}

void NetworkShareHisysEventTest::TearDown() {}

/**
 * @tc.name: NetworkShareHisysEventTest
 * @tc.desc: Test NetworkShareHisysEvent SendFaultEvent01.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareHisysEventTest, SendFaultEvent01, TestSize.Level1)
{
    NetworkShareEventOperator operatorType = NetworkShareEventOperator::OPERATION_START_SA;
    NetworkShareEventErrorType errorCode = NetworkShareEventErrorType::ERROR_START_SA;
    std::string errorMsg = ERROR_MSG;
    NetworkShareEventType eventType = NetworkShareEventType::SETUP_EVENT;
    NetworkShareHisysEvent::GetInstance().SendFaultEvent(operatorType, errorCode, errorMsg, eventType);
    EXPECT_FALSE(errorMsg.empty());
}

/**
 * @tc.name: NetworkShareHisysEventTest
 * @tc.desc: Test NetworkShareHisysEvent SendFaultEvent02.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareHisysEventTest, SendFaultEvent02, TestSize.Level1)
{
    SharingIfaceType sharingType = SharingIfaceType::SHARING_WIFI;
    NetworkShareEventOperator operatorType = NetworkShareEventOperator::OPERATION_ENABLE_IFACE;
    NetworkShareEventErrorType errorCode = NetworkShareEventErrorType::ERROR_ENABLE_IFACE;
    std::string errorMsg = ERROR_MSG;
    NetworkShareEventType eventType = NetworkShareEventType::SETUP_EVENT;
    NetworkShareHisysEvent::GetInstance().SendFaultEvent(sharingType, operatorType, errorCode, errorMsg, eventType);
    int32_t sharingCount = SHARING_COUNT_VALUE;
    NetworkShareHisysEvent::GetInstance().SendBehaviorEvent(sharingCount, sharingType);
    EXPECT_FALSE(errorMsg.empty());
}
} // namespace NetManagerStandard
} // namespace OHOS