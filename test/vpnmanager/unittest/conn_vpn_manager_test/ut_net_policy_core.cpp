/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <thread>

#include <gtest/gtest.h>

#include "net_mgr_log_wrapper.h"
#include "net_policy_callback_test.h"
#include "net_policy_client.h"
#include "net_policy_constants.h"
#define private public
#include "net_policy_core.h"
#include "net_policy_firewall.h"
#include "net_policy_inner_define.h"
#include "net_policy_rule.h"
#include "net_policy_traffic.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr const char *NET_POLICY_WORK_TEST_THREAD = "NET_POLICY_WORK_TEST_THREAD";
constexpr const char *NET_POLICY_STUB_QUEUE = "NET_POLICY_STUB_QUEUE";
constexpr const char *TEST_EVENT_ACTION = "TEST_ACTION";
constexpr const char *EVENT_PARAM_DELETED_UID = "DeletedUid";
std::shared_ptr<NetPolicyCore> g_netPolicyCore;
std::shared_ptr<AppExecFwk::EventRunner> g_runner;
std::shared_ptr<NetPolicyEventHandler> g_handler;
ffrt::queue ffrtQueue_ = NET_POLICY_STUB_QUEUE;
} // namespace

class UtNetPolicyCore : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void UtNetPolicyCore::SetUpTestCase()
{
    g_runner = AppExecFwk::EventRunner::Create(NET_POLICY_WORK_TEST_THREAD);
    g_netPolicyCore = DelayedSingleton<NetPolicyCore>::GetInstance();
    std::make_shared<NetPolicyEventHandler>(g_netPolicyCore, ffrtQueue_);
    g_netPolicyCore->Init(g_handler);
}

void UtNetPolicyCore::TearDownTestCase()
{
    g_netPolicyCore.reset();
}

void UtNetPolicyCore::SetUp() {}

void UtNetPolicyCore::TearDown() {}

/**
 * @tc.name: CreateCore001
 * @tc.desc: Test NetPolicyCore CreateCore.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyCore, CreateCore001, TestSize.Level1)
{
    auto netPolicyTraffic = g_netPolicyCore->CreateCore<NetPolicyTraffic>();
    auto netPolicyFirewall = g_netPolicyCore->CreateCore<NetPolicyFirewall>();
    auto netPolicyRule = g_netPolicyCore->CreateCore<NetPolicyRule>();
    ASSERT_TRUE(netPolicyTraffic != nullptr && netPolicyFirewall != nullptr && g_netPolicyCore != nullptr);
}

/**
 * @tc.name: HandleEvent001
 * @tc.desc: Test NetPolicyCore HandleEvent.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyCore, HandleEvent001, TestSize.Level1)
{
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get();
    auto eventId = static_cast<int32_t>(event->GetInnerEventId());
    auto eventData = event->GetSharedObject<PolicyEvent>();
    ASSERT_NE(g_netPolicyCore, nullptr);
    g_netPolicyCore->HandleEvent(eventId, eventData);
}

/**
 * @tc.name: HandleEvent002
 * @tc.desc: Test NetPolicyCore HandleEvent.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyCore, HandleEvent002, TestSize.Level1)
{
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get();
    auto eventId = static_cast<int32_t>(event->GetInnerEventId());
    auto eventData = event->GetSharedObject<PolicyEvent>();
    event.reset();
    ASSERT_NE(g_netPolicyCore, nullptr);
    g_netPolicyCore->HandleEvent(eventId, eventData);
}

/**
 * @tc.name: SendEvent001
 * @tc.desc: Test NetPolicyCore HandleEvent.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyCore, SendEvent001, TestSize.Level1)
{
    int32_t testEventId = 1154;
    int32_t testDelayTime = 100;
    std::shared_ptr<PolicyEvent> event = nullptr;
    ASSERT_NE(g_netPolicyCore, nullptr);
    g_netPolicyCore->SendEvent(testEventId, event, testDelayTime);
}

/**
 * @tc.name: OnReceiveEvent001
 * @tc.desc: Test NetPolicyCore OnReceiveEvent.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyCore, OnReceiveEvent001, TestSize.Level1)
{
    EventFwk::CommonEventData eventData;
    eventData.SetCode(SAVE_MODE);
    EventFwk::Want want;
    want.SetAction(COMMON_EVENT_POWER_SAVE_MODE_CHANGED);
    eventData.SetWant(want);
    ASSERT_NE(g_netPolicyCore, nullptr);
    g_netPolicyCore->subscriber_->OnReceiveEvent(eventData);
}

/**
 * @tc.name: OnReceiveEvent002
 * @tc.desc: Test NetPolicyCore OnReceiveEvent.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyCore, OnReceiveEvent002, TestSize.Level1)
{
    EventFwk::CommonEventData eventData;
    eventData.SetCode(LOWPOWER_MODE);
    EventFwk::Want want;
    want.SetAction(COMMON_EVENT_POWER_SAVE_MODE_CHANGED);
    eventData.SetWant(want);
    ASSERT_NE(g_netPolicyCore, nullptr);
    g_netPolicyCore->subscriber_->OnReceiveEvent(eventData);
}

/**
 * @tc.name: OnReceiveEvent003
 * @tc.desc: Test NetPolicyCore OnReceiveEvent.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyCore, OnReceiveEvent003, TestSize.Level1)
{
    EventFwk::CommonEventData eventData;
    eventData.SetCode(NORMAL_MODE);
    EventFwk::Want want;
    want.SetAction(COMMON_EVENT_POWER_SAVE_MODE_CHANGED);
    eventData.SetWant(want);
    ASSERT_NE(g_netPolicyCore, nullptr);
    g_netPolicyCore->subscriber_->OnReceiveEvent(eventData);
}

/**
 * @tc.name: OnReceiveEvent004
 * @tc.desc: Test NetPolicyCore OnReceiveEvent.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyCore, OnReceiveEvent004, TestSize.Level1)
{
    EventFwk::CommonEventData eventData;
    eventData.SetCode(EXTREME_MODE);
    EventFwk::Want want;
    want.SetAction(COMMON_EVENT_DEVICE_IDLE_MODE_CHANGED);
    eventData.SetWant(want);
    ASSERT_NE(g_netPolicyCore, nullptr);
    g_netPolicyCore->subscriber_->OnReceiveEvent(eventData);
}

/**
 * @tc.name: OnReceiveEvent005
 * @tc.desc: Test NetPolicyCore OnReceiveEvent.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyCore, OnReceiveEvent005, TestSize.Level1)
{
    EventFwk::CommonEventData eventData;
    eventData.SetCode(NORMAL_MODE);
    EventFwk::Want want;
    want.SetAction(COMMON_EVENT_DEVICE_IDLE_MODE_CHANGED);
    eventData.SetWant(want);
    ASSERT_NE(g_netPolicyCore, nullptr);
    g_netPolicyCore->subscriber_->OnReceiveEvent(eventData);
}

/**
 * @tc.name: OnReceiveEvent006
 * @tc.desc: Test NetPolicyCore OnReceiveEvent.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyCore, OnReceiveEvent006, TestSize.Level1)
{
    std::string testUid = "111";
    EventFwk::CommonEventData eventData;
    eventData.SetCode(NORMAL_MODE);
    EventFwk::Want want;
    want.SetParam(EVENT_PARAM_DELETED_UID, testUid);
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_UID_REMOVED);
    eventData.SetWant(want);
    ASSERT_NE(g_netPolicyCore, nullptr);
    g_netPolicyCore->subscriber_->OnReceiveEvent(eventData);
}

/**
 * @tc.name: OnReceiveEvent007
 * @tc.desc: Test NetPolicyCore OnReceiveEvent.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyCore, OnReceiveEvent007, TestSize.Level1)
{
    EventFwk::CommonEventData eventData;
    eventData.SetCode(NORMAL_MODE);
    EventFwk::Want want;
    want.SetAction(TEST_EVENT_ACTION);
    eventData.SetWant(want);
    ASSERT_NE(g_netPolicyCore, nullptr);
    g_netPolicyCore->subscriber_->OnReceiveEvent(eventData);
}
} // namespace NetManagerStandard
} // namespace OHOS
