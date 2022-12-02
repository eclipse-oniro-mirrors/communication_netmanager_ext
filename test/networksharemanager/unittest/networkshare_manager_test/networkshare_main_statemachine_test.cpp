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

#include "networkshare_main_statemachine.h"
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
    auto monitor = DelayedSingleton<NetworkShareUpstreamMonitor>::GetInstance();
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
} // namespace NetManagerStandard
} // namespace OHOS