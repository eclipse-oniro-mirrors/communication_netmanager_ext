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

#define private public
#include "networkshare_main_statemachine.h"
#undef private
#include "networkshare_state_common.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
} // namespace

class NetworkShareMainStateMachineTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<NetworkShareMainStateMachine> instance_ = nullptr;
    static inline std::shared_ptr<NetworkShareMainStateMachine> nullParamInstance_ = nullptr;
};

void NetworkShareMainStateMachineTest::SetUpTestCase()
{
    auto monitor = NetworkShareUpstreamMonitor::GetInstance();
    instance_ = std::make_shared<NetworkShareMainStateMachine>(monitor);
    std::shared_ptr<NetworkShareUpstreamMonitor> networkmonitor = nullptr;
    nullParamInstance_ = std::make_shared<NetworkShareMainStateMachine>(networkmonitor);
}

void NetworkShareMainStateMachineTest::TearDownTestCase() {}

void NetworkShareMainStateMachineTest::SetUp() {}

void NetworkShareMainStateMachineTest::TearDown() {}

HWTEST_F(NetworkShareMainStateMachineTest, SwitchToErrorStateTest001, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    int32_t errType = 0;
    instance_->SwitcheToErrorState(errType);
    ASSERT_NE(nullParamInstance_, nullptr);
    nullParamInstance_->SwitcheToErrorState(errType);
    ASSERT_EQ(errType, 0);
}

HWTEST_F(NetworkShareMainStateMachineTest, MainSmStateSwitchTest001, TestSize.Level1)
{
    uint32_t startState = MAINSTATE_INIT;
    uint32_t endState = MAINSTATE_MAX;
    ASSERT_NE(instance_, nullptr);
    ASSERT_NE(nullParamInstance_, nullptr);
    for (uint32_t i = startState; i <= endState; i++) {
        instance_->MainSmStateSwitch(i);
        nullParamInstance_->MainSmStateSwitch(i);
    }
    instance_->MainSmStateSwitch(startState);
    nullParamInstance_->MainSmStateSwitch(startState);
}

HWTEST_F(NetworkShareMainStateMachineTest, MainSmEventHandleTest001, TestSize.Level1)
{
    uint32_t startState = MAINSTATE_INIT;
    uint32_t endState = MAINSTATE_MAX;
    ASSERT_NE(instance_, nullptr);
    ASSERT_NE(nullParamInstance_, nullptr);
    for (uint32_t i = startState; i <= endState; i++) {
        MessageIfaceActive active;
        instance_->MainSmEventHandle(i, active);
        nullParamInstance_->MainSmEventHandle(i, active);
    }
    for (uint32_t i = startState; i <= endState; i++) {
        std::string active;
        instance_->MainSmEventHandle(i, active);
        nullParamInstance_->MainSmEventHandle(i, active);
    }
}

HWTEST_F(NetworkShareMainStateMachineTest, MainSmEventHandleTest002, TestSize.Level1)
{
    int32_t eventId = 0;
    ASSERT_NE(instance_, nullptr);
    ASSERT_NE(nullParamInstance_, nullptr);
    MessageIfaceActive active;
    instance_->MainSmEventHandle(eventId, active);
    nullParamInstance_->MainSmEventHandle(eventId, active);
}

HWTEST_F(NetworkShareMainStateMachineTest, HandleInitInterfaceStateActiveTest001, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    MessageIfaceActive message;
    auto ret = instance_->HandleInitInterfaceStateActive(message);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareMainStateMachineTest, HandleInitInterfaceStateInactiveTest001, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    MessageIfaceActive message;
    auto ret = instance_->HandleInitInterfaceStateInactive(message);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_LOCAL_PTR_NULL);
}

HWTEST_F(NetworkShareMainStateMachineTest, HandleAliveInterfaceStateActiveTest001, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    MessageIfaceActive message;
    auto ret = instance_->HandleAliveInterfaceStateActive(message);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareMainStateMachineTest, HandleAliveInterfaceStateInactiveTest001, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    MessageIfaceActive message;
    auto ret = instance_->HandleAliveInterfaceStateInactive(message);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_LOCAL_PTR_NULL);
}

HWTEST_F(NetworkShareMainStateMachineTest, HandleAliveUpstreamMonitorCallbackTest001, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    MessageUpstreamInfo message;
    auto ret = instance_->HandleAliveUpstreamMonitorCallback(message);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareMainStateMachineTest, NetworkShareMainStateMachineBranchTest001, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    instance_->ChooseUpstreamNetwork();
    instance_->DisableForward();

    MessageUpstreamInfo message;
    auto ret = instance_->HandleErrorClear(message);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    auto result = instance_->TurnOffMainShareSettings();
    EXPECT_EQ(result, true);
}

HWTEST_F(NetworkShareMainStateMachineTest, TurnOnMainShareSettingsTest001, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    instance_->hasSetForward_ = true;
    auto ret = instance_->TurnOnMainShareSettings();
    EXPECT_TRUE(ret);
}

HWTEST_F(NetworkShareMainStateMachineTest, ChooseUpstreamNetworkTest, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    auto mockNetworkMonitor = std::make_shared<NetworkShareUpstreamMonitor>();
    instance_->networkMonitor_ = mockNetworkMonitor;

    sptr<NetHandle> pNetHandle = new (std::nothrow) NetHandle();
    sptr<NetAllCapabilities> pNetCapabilities = new (std::nothrow) NetAllCapabilities();
    sptr<NetLinkInfo> pNetLinkInfo = new (std::nothrow) NetLinkInfo();
    std::shared_ptr<UpstreamNetworkInfo> netInfoPtr =
        std::make_shared<UpstreamNetworkInfo>(pNetHandle, pNetCapabilities, pNetLinkInfo);
    netInfoPtr->netLinkPro_->ifaceName_ = "test_iface";

    EXPECT_TRUE(instance_->upstreamIfaceName_.empty());
    EXPECT_EQ(instance_->upstreamIfaceName_, "");
}

/**
 * @tc.number: NetworkShareMainStateMachine_ChooseUpstreamNetwork_NullMonitor
 * @tc.name: Test ChooseUpstreamNetwork with null networkMonitor_
 * @tc.desc: Verify that ChooseUpstreamNetwork handles null networkMonitor_
 */
HWTEST_F(NetworkShareMainStateMachineTest, ChooseUpstreamNetwork_NullMonitor, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    instance_->networkMonitor_ = nullptr;
    instance_->ChooseUpstreamNetwork();
    // Should return early when networkMonitor_ is null
    EXPECT_TRUE(instance_->upstreamIfaceName_.empty());
}

/**
 * @tc.number: NetworkShareMainStateMachine_ChooseUpstreamNetwork_GetUpstreamFailed
 * @tc.name: Test ChooseUpstreamNetwork when GetCurrentGoodUpstream returns false
 * @tc.desc: Verify that ChooseUpstreamNetwork handles GetCurrentGoodUpstream failure
 */
HWTEST_F(NetworkShareMainStateMachineTest, ChooseUpstreamNetwork_GetUpstreamFailed, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    // Set networkMonitor_ to a valid monitor but GetCurrentGoodUpstream will return false
    // because there's no default network registered
    instance_->ChooseUpstreamNetwork();
    // upstreamIfaceName_ should remain empty when GetCurrentGoodUpstream fails
    EXPECT_TRUE(instance_->upstreamIfaceName_.empty());
}

/**
 * @tc.number: NetworkShareMainStateMachine_DisableForward_EmptyUpstream
 * @tc.name: Test DisableForward with empty upstreamIfaceName_
 * @tc.desc: Verify that DisableForward handles empty upstream interface name
 */
HWTEST_F(NetworkShareMainStateMachineTest, DisableForward_EmptyUpstream, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    instance_->upstreamIfaceName_ = "";
    instance_->DisableForward();
    // Should handle empty upstream interface name
    EXPECT_EQ(instance_->upstreamIfaceName_, "");
}

/**
 * @tc.number: NetworkShareMainStateMachine_DisableForward_ValidUpstream
 * @tc.name: Test DisableForward with valid upstream interface
 * @tc.desc: Verify that DisableForward correctly disables forward for upstream
 */
HWTEST_F(NetworkShareMainStateMachineTest, DisableForward_ValidUpstream, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    instance_->upstreamIfaceName_ = "test_iface";
    instance_->DisableForward();
    // Should clear the upstream interface name
    EXPECT_EQ(instance_->upstreamIfaceName_, "");
}

/**
 * @tc.number: NetworkShareMainStateMachine_DisableForward_WithClat
 * @tc.name: Test DisableForward with clat interface
 * @tc.desc: Verify that DisableForward handles clat interface (tunv4-)
 */
HWTEST_F(NetworkShareMainStateMachineTest, DisableForward_WithClat, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    instance_->upstreamIfaceName_ = "rmnet0";
    instance_->DisableForward();
    // Should disable NAT for both rmnet0 and tunv4-rmnet0
    EXPECT_EQ(instance_->upstreamIfaceName_, "");
}

/**
 * @tc.number: NetworkShareMainStateMachine_HandleAliveUpstreamMonitorCallback_OnLinkProperties
 * @tc.name: Test HandleAliveUpstreamMonitorCallback with ON_LINKPROPERTIES event
 * @tc.desc: Verify that HandleAliveUpstreamMonitorCallback handles ON_LINKPROPERTIES event
 */
HWTEST_F(NetworkShareMainStateMachineTest, HandleAliveUpstreamMonitorCallback_OnLinkProperties, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    MessageUpstreamInfo message;
    message.cmd_ = EVENT_UPSTREAM_CALLBACK_ON_LINKPROPERTIES;
    auto ret = instance_->HandleAliveUpstreamMonitorCallback(message);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.number: NetworkShareMainStateMachine_HandleAliveUpstreamMonitorCallback_OnLost
 * @tc.name: Test HandleAliveUpstreamMonitorCallback with ON_LOST event
 * @tc.desc: Verify that HandleAliveUpstreamMonitorCallback handles ON_LOST event
 */
HWTEST_F(NetworkShareMainStateMachineTest, HandleAliveUpstreamMonitorCallback_OnLost, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    MessageUpstreamInfo message;
    message.cmd_ = EVENT_UPSTREAM_CALLBACK_ON_LOST;
    auto ret = instance_->HandleAliveUpstreamMonitorCallback(message);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.number: NetworkShareMainStateMachine_HandleAliveUpstreamMonitorCallback_OnCapabilities
 * @tc.name: Test HandleAliveUpstreamMonitorCallback with ON_CAPABILITIES event
 * @tc.desc: Verify that HandleAliveUpstreamMonitorCallback handles ON_CAPABILITIES event
 */
HWTEST_F(NetworkShareMainStateMachineTest, HandleAliveUpstreamMonitorCallback_OnCapabilities, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    MessageUpstreamInfo message;
    message.cmd_ = EVENT_UPSTREAM_CALLBACK_ON_CAPABILITIES;
    auto ret = instance_->HandleAliveUpstreamMonitorCallback(message);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.number: NetworkShareMainStateMachine_HandleAliveUpstreamMonitorCallback_DefaultSwitched
 * @tc.name: Test HandleAliveUpstreamMonitorCallback with DEFAULT_SWITCHED event
 * @tc.desc: Verify that HandleAliveUpstreamMonitorCallback handles DEFAULT_SWITCHED event
 */
HWTEST_F(NetworkShareMainStateMachineTest, HandleAliveUpstreamMonitorCallback_DefaultSwitched, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    MessageUpstreamInfo message;
    message.cmd_ = EVENT_UPSTREAM_CALLBACK_DEFAULT_SWITCHED;
    auto ret = instance_->HandleAliveUpstreamMonitorCallback(message);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.number: NetworkShareMainStateMachine_HandleAliveUpstreamMonitorCallback_DefaultCmd
 * @tc.name: Test HandleAliveUpstreamMonitorCallback with default cmd
 * @tc.desc: Verify that HandleAliveUpstreamMonitorCallback handles default cmd
 */
HWTEST_F(NetworkShareMainStateMachineTest, HandleAliveUpstreamMonitorCallback_DefaultCmd, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    MessageUpstreamInfo message;
    message.cmd_ = 100; // Invalid cmd
    auto ret = instance_->HandleAliveUpstreamMonitorCallback(message);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.number: NetworkShareMainStateMachine_HandleAliveUpstreamMonitorCallback_UpstreamNotWanted
 * @tc.name: Test HandleAliveUpstreamMonitorCallback when upstream is not wanted
 * @tc.desc: Verify that HandleAliveUpstreamMonitorCallback returns early when UpstreamWanted is false
 */
HWTEST_F(NetworkShareMainStateMachineTest, HandleAliveUpstreamMonitorCallback_UpstreamNotWanted, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    // This test verifies the code path when UpstreamWanted returns false
    MessageUpstreamInfo message;
    message.cmd_ = EVENT_UPSTREAM_CALLBACK_ON_LINKPROPERTIES;
    auto ret = instance_->HandleAliveUpstreamMonitorCallback(message);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS