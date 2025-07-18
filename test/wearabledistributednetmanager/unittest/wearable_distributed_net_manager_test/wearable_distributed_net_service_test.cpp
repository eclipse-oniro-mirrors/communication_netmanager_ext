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
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "net_manager_constants.h"
#include "token_setproc.h"

#define private public
#include "wearable_distributed_net_agent.h"
#include "wearable_distributed_net_service.h"
#undef private

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr int32_t TCP_PORT_ID = 8080;
constexpr int32_t UDP_PORT_ID = 8081;
constexpr int32_t SA_ID_TEST = COMM_WEARABLE_DISTRIBUTED_NET_ABILITY_ID;
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

HWTEST_F(WearableDistributedNetServiceTest, OnStop, TestSize.Level1)
{
    WearableDistributedNetService wearableDistributedNetService(SA_ID_TEST, true);
    wearableDistributedNetService.OnStop();
    EXPECT_EQ(wearableDistributedNetService.registerToService_,
        WearableDistributedNetService::ServiceRunningState::STATE_STOPPED);
}

HWTEST_F(WearableDistributedNetServiceTest, SetupWearableDistributedNet, TestSize.Level1)
{
    WearableDistributedNetService wearableDistributedNetService(SA_ID_TEST, true);
    auto ret = wearableDistributedNetService.SetupWearableDistributedNet(TCP_PORT_ID, UDP_PORT_ID, false);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
    wearableDistributedNetService.TearDownWearableDistributedNet();
    ret = wearableDistributedNetService.SetupWearableDistributedNet(TCP_PORT_ID, UDP_PORT_ID, true);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
    wearableDistributedNetService.TearDownWearableDistributedNet();
}

HWTEST_F(WearableDistributedNetServiceTest, TearDownWearableDistributedNet, TestSize.Level1)
{
    WearableDistributedNetService wearableDistributedNetService(SA_ID_TEST, true);
    wearableDistributedNetService.SetupWearableDistributedNet(TCP_PORT_ID, UDP_PORT_ID, false);
    auto ret = wearableDistributedNetService.TearDownWearableDistributedNet();
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(WearableDistributedNetServiceTest, UpdateNetScore, TestSize.Level1)
{
    bool isCharging = true;
    WearableDistributedNetManagement::GetInstance().UpdateNetScore(isCharging);
    EXPECT_EQ(WearableDistributedNetAgent::GetInstance().score_, NET_SCORE_WITH_CHARGE_STATE);
}

HWTEST_F(WearableDistributedNetServiceTest, OnReceiveEvent001, TestSize.Level1)
{
    WearableDistributedNetService wearableDistributedNetService(SA_ID_TEST, true);
    EventFwk::CommonEventData eventData;
    AAFwk::Want want = eventData.GetWant();
    want.SetParam(PowerMgr::BatteryInfo::COMMON_EVENT_KEY_CHARGE_STATE,
        static_cast<int32_t>(ChargeState::CHARGE_STATE_DISABLE));
    want.SetParam(PowerMgr::BatteryInfo::COMMON_EVENT_KEY_CHARGE_STATE,
        static_cast<int32_t>(ChargeState::CHARGE_STATE_NONE));
    eventData.SetWant(want);
    wearableDistributedNetService.SubscribeCommonEvent();
    wearableDistributedNetService.subscriber_->OnReceiveEvent(eventData);
    EXPECT_EQ(WearableDistributedNetAgent::GetInstance().score_, NET_SCORE_WITH_UNCHARGE_STATE);
}

HWTEST_F(WearableDistributedNetServiceTest, OnReceiveEvent002, TestSize.Level1)
{
    WearableDistributedNetService wearableDistributedNetService(SA_ID_TEST, true);
    EventFwk::CommonEventData eventData;
    AAFwk::Want want = eventData.GetWant();
    want.SetParam(PowerMgr::BatteryInfo::COMMON_EVENT_KEY_CHARGE_STATE,
        static_cast<int32_t>(ChargeState::CHARGE_STATE_ENABLE));
    want.SetParam(PowerMgr::BatteryInfo::COMMON_EVENT_KEY_CHARGE_STATE,
        static_cast<int32_t>(ChargeState::CHARGE_STATE_FULL));
    eventData.SetWant(want);
    wearableDistributedNetService.SubscribeCommonEvent();
    wearableDistributedNetService.subscriber_->OnReceiveEvent(eventData);
    EXPECT_EQ(WearableDistributedNetAgent::GetInstance().score_, NET_SCORE_WITH_CHARGE_STATE);
}

HWTEST_F(WearableDistributedNetServiceTest, OnReceiveEvent003, TestSize.Level1)
{
    WearableDistributedNetService wearableDistributedNetService(SA_ID_TEST, true);
    EventFwk::CommonEventData eventData;
    AAFwk::Want want = eventData.GetWant();
    eventData.SetWant(want);
    wearableDistributedNetService.SubscribeCommonEvent();
    wearableDistributedNetService.subscriber_->OnReceiveEvent(eventData);
    EXPECT_EQ(WearableDistributedNetAgent::GetInstance().score_, NET_SCORE_WITH_CHARGE_STATE);
}

HWTEST_F(WearableDistributedNetServiceTest, UpdateMeteredStatus001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    bool isMetered = false;
    WearableDistributedNetService wearableDistributedNetService(SA_ID_TEST, true);
    wearableDistributedNetService.SetupWearableDistributedNet(TCP_PORT_ID, UDP_PORT_ID, false);
    auto ret = wearableDistributedNetService.UpdateMeteredStatus(isMetered);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(WearableDistributedNetServiceTest, UpdateMeteredStatus002, TestSize.Level1)
{
    bool isMetered = false;
    WearableDistributedNetService wearableDistributedNetService(SA_ID_TEST, true);
    auto ret = wearableDistributedNetService.UpdateMeteredStatus(isMetered);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(WearableDistributedNetServiceTest, OnStop002, TestSize.Level1)
{
    WearableDistributedNetService wearableDistributedNetService(SA_ID_TEST, true);
    wearableDistributedNetService.SubscribeCommonEvent();
    wearableDistributedNetService.state_ = WearableDistributedNetService::ServiceRunningState::STATE_RUNNING;
    wearableDistributedNetService.OnStop();
    wearableDistributedNetService.state_ = WearableDistributedNetService::ServiceRunningState::STATE_STOPPED;
    EXPECT_EQ(wearableDistributedNetService.state_, WearableDistributedNetService::ServiceRunningState::STATE_STOPPED);
}

HWTEST_F(WearableDistributedNetServiceTest, SetupWearableDistributedNet002, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    WearableDistributedNetService wearableDistributedNetService(SA_ID_TEST, true);
    auto ret = wearableDistributedNetService.SetupWearableDistributedNet(TCP_PORT_ID, UDP_PORT_ID, false);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    wearableDistributedNetService.TearDownWearableDistributedNet();
    ret = wearableDistributedNetService.SetupWearableDistributedNet(TCP_PORT_ID, UDP_PORT_ID, true);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    wearableDistributedNetService.TearDownWearableDistributedNet();
}

HWTEST_F(WearableDistributedNetServiceTest, EnableWearableDistributedNet, TestSize.Level1)
{
    bool enableFlag = false;
    WearableDistributedNetService wearableDistributedNetService(SA_ID_TEST, true);
    int32_t result = wearableDistributedNetService.EnableWearableDistributedNet(enableFlag);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
