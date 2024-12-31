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
#ifdef GTEST_API_
#define private public
#define protected public
#endif
#include "sys_timer.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
} // namespace

class SysTimerTest : public testing::Test {
public:
    void SetUp() override
    {
        timer_ = std::make_shared<SysTimer>();
    }

protected:
    std::shared_ptr<SysTimer> timer_;
};

HWTEST_F(SysTimerTest, SetCallbackInfoTest, TestSize.Level1)
{
    bool callbackCalled = false;
    timer_->SetCallbackInfo([&callbackCalled]()
                            { callbackCalled = true; });

    timer_->OnTrigger();

    EXPECT_TRUE(callbackCalled);
}

HWTEST_F(SysTimerTest, SetTypeTest, TestSize.Level1)
{
    int type = 1;
    timer_->SetType(type);

    EXPECT_EQ(timer_->type, type);
}

HWTEST_F(SysTimerTest, SetRepeatTest, TestSize.Level1)
{
    bool repeat = true;
    timer_->SetRepeat(repeat);

    EXPECT_EQ(timer_->repeat, repeat);
}

HWTEST_F(SysTimerTest, SetIntervalTest, TestSize.Level1)
{
    uint64_t interval = 5000;
    timer_->SetInterval(interval);

    EXPECT_EQ(timer_->interval, interval);
}

HWTEST_F(SysTimerTest, SetWantAgentTest, TestSize.Level1)
{
    std::shared_ptr<OHOS::AbilityRuntime::WantAgent::WantAgent> wantAgent =
        std::make_shared<OHOS::AbilityRuntime::WantAgent::WantAgent>();
    timer_->SetWantAgent(wantAgent);

    EXPECT_EQ(timer_->wantAgent, wantAgent);
}

HWTEST_F(SysTimerTest, SysTimerTest, TestSize.Level1)
{
    bool repeat = true;
    uint64_t interval = 5000;
    bool isNoWakeUp = false;
    bool isIdle = true;
    SysTimer SysTimer(repeat, interval, isNoWakeUp, isIdle);

    EXPECT_EQ(SysTimer.interval, interval);
}
} // namespace NetManagerStandard
} // namespace OHOS