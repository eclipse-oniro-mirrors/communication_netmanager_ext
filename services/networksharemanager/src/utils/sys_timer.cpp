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

#include "sys_timer.h"
#include "common_timer_errors.h"

namespace OHOS {
namespace NetManagerStandard {

SysTimer::SysTimer()
{}

SysTimer::~SysTimer()
{}

SysTimer::SysTimer(bool repeat, uint64_t interval, bool isNoWakeUp, bool isIdle)
{
    this->repeat = repeat;
    this->interval = interval;
    this->type = TIMER_TYPE_REALTIME;
    if (!isNoWakeUp) {
        this->type = TIMER_TYPE_WAKEUP + TIMER_TYPE_REALTIME;
    }
    if (isIdle) {
        this->type = TIMER_TYPE_IDLE;
    }
}

void SysTimer::OnTrigger()
{
    if (callBack_ != nullptr) {
        callBack_();
    }
}

void SysTimer::SetCallbackInfo(const std::function<void()> &callBack)
{
    this->callBack_ = callBack;
}

void SysTimer::SetType(const int &type)
{
    this->type = type;
}

void SysTimer::SetRepeat(bool repeat)
{
    this->repeat = repeat;
}

void SysTimer::SetInterval(const uint64_t &interval)
{
    this->interval = interval;
}

void SysTimer::SetWantAgent(std::shared_ptr<OHOS::AbilityRuntime::WantAgent::WantAgent> wantAgent)
{
    this->wantAgent = wantAgent;
}
} // namespace NetManagerStandard
} // namespace OHOS