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
#include "net_manager_constants.h"

#define private public
#include "wearable_distributed_net_service.h"
#undef private

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr int32_t TCP_PORT_ID = 8080;
constexpr int32_t UDP_PORT_ID = 8081;
constexpr int32_t SA_ID_TEST = 1;
using namespace testing::ext;
} // namesapce

class WearableDistributedNetServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();

    void SetUp();
    void TearDown();
};

void WearableDistributedNetServiceTest::SetUpTestCase() {}

void WearableDistributedNetServiceTest::TearDownTestCase() {}

void WearableDistributedNetServiceTest::SetUp() {}

void WearableDistributedNetServiceTest::TearDown() {}

HWTEST_F(WearableDistributedNetServiceTest, OnStart, TestSize.Level1)
{
    WearableDistributedNetService wearableDistributedNetService(SA_ID_TEST, true);
    wearableDistributedNetService.state_ = WearableDistributedNetService::ServiceRunningState::STATE_RUNNING;
    wearableDistributedNetService.OnStart();
    wearableDistributedNetService.state_ = WearableDistributedNetService::ServiceRunningState::STATE_STOPPED;
    EXPECT_EQ(wearableDistributedNetService.state_, WearableDistributedNetService::ServiceRunningState::STATE_STOPPED);
}

HWTEST_F(WearableDistributedNetServiceTest, Init, TestSize.Level1)
{
    WearableDistributedNetService wearableDistributedNetService(SA_ID_TEST, true);
    bool initResult = wearableDistributedNetService.Init();
    EXPECT_FALSE(initResult);
    wearableDistributedNetService.OnStop();
}

HWTEST_F(WearableDistributedNetServiceTest, OnStop, TestSize.Level1)
{
    WearableDistributedNetService wearableDistributedNetService(SA_ID_TEST, true);
    wearableDistributedNetService.OnStop();
    EXPECT_EQ(wearableDistributedNetService.state_, WearableDistributedNetService::ServiceRunningState::STATE_STOPPED);
}

HWTEST_F(WearableDistributedNetServiceTest, SetupWearableDistributedNet001, TestSize.Level1)
{
    WearableDistributedNetService wearableDistributedNetService(SA_ID_TEST, true);
    auto ret = wearableDistributedNetService.SetupWearableDistributedNet(TCP_PORT_ID, UDP_PORT_ID, false);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
    wearableDistributedNetService.TearDownWearableDistributedNet();
    ret = wearableDistributedNetService.SetupWearableDistributedNet(TCP_PORT_ID, UDP_PORT_ID, true);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
    wearableDistributedNetService.TearDownWearableDistributedNet();
}

HWTEST_F(WearableDistributedNetServiceTest, SetupWearableDistributedNet002, TestSize.Level1)
{
    WearableDistributedNetService wearableDistributedNetService(SA_ID_TEST, true);
    auto ret = wearableDistributedNetService.SetupWearableDistributedNet(TCP_PORT_ID, UDP_PORT_ID, false);
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
    wearableDistributedNetService.TearDownWearableDistributedNet();
    ret = wearableDistributedNetService.SetupWearableDistributedNet(TCP_PORT_ID, UDP_PORT_ID, true);
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
    wearableDistributedNetService.TearDownWearableDistributedNet();
}

HWTEST_F(WearableDistributedNetServiceTest, TearDownWearableDistributedNet001, TestSize.Level1)
{
    WearableDistributedNetService wearableDistributedNetService(SA_ID_TEST, true);
    wearableDistributedNetService.SetupWearableDistributedNet(TCP_PORT_ID, UDP_PORT_ID, false);
    auto ret = wearableDistributedNetService.TearDownWearableDistributedNet();
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(WearableDistributedNetServiceTest, TearDownWearableDistributedNet002, TestSize.Level1)
{
    WearableDistributedNetService wearableDistributedNetService(SA_ID_TEST, true);
    wearableDistributedNetService.SetupWearableDistributedNet(TCP_PORT_ID, UDP_PORT_ID, false);
    auto ret = wearableDistributedNetService.TearDownWearableDistributedNet();
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}
} // namespace NetManagerStandard
} // namespace OHOS
