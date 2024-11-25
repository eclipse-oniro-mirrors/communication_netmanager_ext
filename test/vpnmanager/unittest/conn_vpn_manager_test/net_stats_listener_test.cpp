/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "common_event_support.h"
#include "net_stats_listener.h"
#include "net_stats_constants.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
#define DTEST_LOG std::cout << __func__ << ":" << __LINE__ << ":"
constexpr uint32_t SAVE_MODE = 601;
} // namespace

class NetStatsListenerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static inline std::shared_ptr<NetStatsListener> instance_ = nullptr;
};

void NetStatsListenerTest::SetUpTestCase()
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SHUTDOWN);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    instance_ = std::make_shared<NetStatsListener>(subscribeInfo);
}

void NetStatsListenerTest::TearDownTestCase() {}

void NetStatsListenerTest::SetUp() {}

void NetStatsListenerTest::TearDown() {}

HWTEST_F(NetStatsListenerTest, OnReceiveEvent001, TestSize.Level1)
{
    instance_->RegisterStatsCallback(EventFwk::CommonEventSupport::COMMON_EVENT_SHUTDOWN,
                                     [this](const EventFwk::Want &want) { return 0; });
    EventFwk::Want wantErr;
    wantErr.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_SAVE_MODE_CHANGED);
    EventFwk::CommonEventData eventData;
    eventData.SetCode(SAVE_MODE);
    eventData.SetWant(wantErr);
    ASSERT_NE(instance_, nullptr);
    instance_->OnReceiveEvent(eventData);

    EventFwk::Want wantOk;
    wantOk.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SHUTDOWN);
    eventData.SetCode(SAVE_MODE);
    eventData.SetWant(wantOk);
    ASSERT_NE(instance_, nullptr);
    instance_->OnReceiveEvent(eventData);
}

HWTEST_F(NetStatsListenerTest, OnReceiveEvent002, TestSize.Level1)
{
    EventFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    EventFwk::CommonEventData data;
    data.SetWant(want);
    instance_->OnReceiveEvent(data);
    ASSERT_NE(instance_, nullptr);
}

HWTEST_F(NetStatsListenerTest, OnReceiveEvent003, TestSize.Level1)
{
    EventFwk::Want want;
    want.SetAction("net stats test action");
    EventFwk::CommonEventData data;
    data.SetWant(want);
    instance_->OnReceiveEvent(data);
    ASSERT_NE(instance_, nullptr);
}

HWTEST_F(NetStatsListenerTest, OnReceiveEvent004, TestSize.Level1)
{
    const std::string testAction = "net stats test action";
    NetStatsListener::StatsCallback testCallback = [](const EventFwk::Want &want) { return -1; };
    instance_->RegisterStatsCallback(testAction, testCallback);
    EventFwk::Want want;
    want.SetAction(testAction);
    EventFwk::CommonEventData data;
    data.SetWant(want);
    instance_->OnReceiveEvent(data);
    ASSERT_NE(instance_, nullptr);
}

HWTEST_F(NetStatsListenerTest, RegisterStatsCallback001, TestSize.Level1)
{
    instance_->RegisterStatsCallback(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED, nullptr);
    EventFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    EventFwk::CommonEventData data;
    data.SetWant(want);
    instance_->OnReceiveEvent(data);
    ASSERT_NE(instance_, nullptr);
}
} // namespace NetManagerStandard
} // namespace OHOS
