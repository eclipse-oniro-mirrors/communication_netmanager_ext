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
#include <map>
#include <mutex>
#include <set>
#include <vector>
#include <gtest/gtest.h>
#include "system_ability_definition.h"
#include "networkslice_loop_manager.h"
#include "networkslice_loop_manager.h"
#include "networkslicemsgcenter.h"
#include "networkslice_event.h"
#include "broadcast_proxy.h"
 
namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
constexpr int NUMBER5000 = 5000;
class BroadcastProxyTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};
 
class TestHandler : public AppExecFwk::EventHandler {
public:
    TestHandler(): AppExecFwk::EventHandler(
        DelayedSingleton<NetworkSlice_Loop_Manager>::GetInstance()->getLoop(OLLIE_MESSAGE_THREAD)) {}
    virtual ~TestHandler() = default;
 
    std::unordered_map<uint32_t, bool> eventMap;
 
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event) override
    {
        std::unique_lock<std::mutex> lck(mutex_);
        uint32_t eventId = event->GetInnerEventId();
        eventMap[eventId] = true;
        cv_.notify_one();
    }
 
    bool ConsumeEvent(NetworkSliceEvent event)
    {
        std::unique_lock<std::mutex> lck(mutex_);
        if (HasInnerEvent(static_cast<uint32_t>(event)) || eventMap.find(event) != eventMap.end()) {
            return true;
        }
        cv_.wait_for(lck, std::chrono::milliseconds(NUMBER5000));
        return HasInnerEvent(static_cast<uint32_t>(event)) || eventMap.find(event) != eventMap.end();
    }
 
private:
    std::mutex mutex_;
    std::condition_variable cv_;
};
 
class VpnEventObserverTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};
 
/**
* @tc.name: BroadcastProxyInitTest001
* @tc.desc: broadcast_proxy init
* @tc.type: FUNC
* @tc.require: issue
*/
HWTEST_F(BroadcastProxyTest, BroadcastProxyInitTest001, TestSize.Level1)
{
    NETMGR_EXT_LOG_I("BroadcastProxyInitTest001 enter");
    auto broadcastProxy = std::make_shared<broadcast_proxy>();
    EXPECT_NE(broadcastProxy, nullptr);
}
 
/**
* @tc.name: SystemAbilityCustomizedListenerTest001
* @tc.desc: SystemAbilityCustomizedListener Test
* @tc.type: FUNC
* @tc.require: issue
*/
HWTEST_F(BroadcastProxyTest, SystemAbilityCustomizedListenerTest001, TestSize.Level1)
{
    NETMGR_EXT_LOG_I("SystemAbilityCustomizedListenerTest001 enter");
    auto broadcastProxy = std::make_shared<broadcast_proxy>();
    if (broadcastProxy == nullptr) {
        return;
    }
    broadcastProxy->statusChangeListener_->OnAddSystemAbility(-1, "");
    broadcastProxy->statusChangeListener_->OnRemoveSystemAbility(-1, "");
    broadcastProxy->statusChangeListener_->OnAddSystemAbility(COMMON_EVENT_SERVICE_ID, "");
    broadcastProxy->statusChangeListener_->OnAddSystemAbility(APP_MGR_SERVICE_ID, "");
    broadcastProxy->statusChangeListener_->OnAddSystemAbility(COMM_VPN_MANAGER_SYS_ABILITY_ID, "");
    broadcastProxy->statusChangeListener_->OnAddSystemAbility(WIFI_DEVICE_SYS_ABILITY_ID, "");
    broadcastProxy->statusChangeListener_->OnAddSystemAbility(COMM_NETSYS_NATIVE_SYS_ABILITY_ID, "");
    EXPECT_TRUE(broadcastProxy != nullptr);
}
 
/**
  * @tc.name: NetManagerExtNetAppAwareObserverCallback
  * @tc.desc: TEST NetManagerExtAppAwareObserverCallback1
  * @tc.type: FUNC
  * @tc.require: issue
 */
HWTEST_F(BroadcastProxyTest, NetManagerAppAwareObserverCallback1, testing::ext::TestSize.Level1)
{
    sptr<broadcast_proxy::AppAwareObserver> appAwareObserver = new (std::nothrow) broadcast_proxy::AppAwareObserver();
    AppExecFwk::AppStateData appStateData;
    EXPECT_NE(appAwareObserver, NULL);
    if (!appAwareObserver) {
        return;
    }
    appAwareObserver->OnForegroundApplicationChanged(appStateData);
}
 
/**
  * @tc.name: NetManagerExtNetAppAwareObserverCallback
  * @tc.desc: TEST NetManagerExtAppAwareObserverCallback2
  * @tc.type: FUNC
  * @tc.require: issue
 */
HWTEST_F(BroadcastProxyTest, NetManagerAppAwareObserverCallback2, testing::ext::TestSize.Level1)
{
    sptr<broadcast_proxy::AppAwareObserver> appAwareObserver = new (std::nothrow) broadcast_proxy::AppAwareObserver();
    AppExecFwk::AppStateData appStateData;
    appStateData.uid = 1;
    appStateData.pid = 1;
    appStateData.state = 2;
    appStateData.bundleName = "browser";
    appStateData.isFocused = false;
    EXPECT_NE(appAwareObserver, NULL);
    if (!appAwareObserver) {
        return;
    }
    appAwareObserver->OnForegroundApplicationChanged(appStateData);
}
 
/**
* @tc.name: NETMANAGER_EXT_SCREEN_ON
* @tc.desc: recv NETMANAGER_EXT_SCREEN_ON event
* @tc.type: FUNC
* @tc.require: issue
*/
HWTEST_F(BroadcastProxyTest, NETMANAGER_EXT_SCREEN_ON, TestSize.Level1)
{
    auto broadcastProxy = std::make_shared<broadcast_proxy>();
    if (broadcastProxy == nullptr) {
        return;
    }
    NETMGR_EXT_LOG_I("NETMANAGER_EXT_SCREEN_ON enter");
    NetworkSliceEvent event = EVENT_SCREEN_ON;
    std::string commonEventSupport = EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON;
    auto handler = std::make_shared<TestHandler>();
    Singleton<NetworkSliceMsgCenter>::GetInstance().registHandler(MODULE_NETWORKSLICE, handler);
    Singleton<NetworkSliceMsgCenter>::GetInstance().Subscribe(event, MODULE_NETWORKSLICE);
    EventFwk::MatchingSkills matchSkills;
    matchSkills.AddEvent(commonEventSupport);
    EventFwk::CommonEventSubscribeInfo subcriberInfo(matchSkills);
    std::shared_ptr<broadcast_proxy::BroadcastEventSubscriber> subscriber =
        std::make_shared<broadcast_proxy::BroadcastEventSubscriber>(subcriberInfo);
    AAFwk::Want want;
    want.SetAction(commonEventSupport);
    EventFwk::CommonEventData data(want);
    subscriber->OnReceiveEvent(data);
    Singleton<NetworkSliceMsgCenter>::GetInstance().unRegistHandler(MODULE_NETWORKSLICE, handler);
    EXPECT_FALSE(handler == nullptr);
}
 
/**
* @tc.name: NETMANAGER_EXT_SCREEN_OFF
* @tc.desc: recv NETMANAGER_EXT_SCREEN_OFF event
* @tc.type: FUNC
* @tc.require: issue
*/
HWTEST_F(BroadcastProxyTest, NETMANAGER_EXT_SCREEN_OFF, TestSize.Level1)
{
    auto broadcastProxy = std::make_shared<broadcast_proxy>();
    if (broadcastProxy == nullptr) {
        return;
    }
    NETMGR_EXT_LOG_I("NETMANAGER_EXT_SCREEN_OFF enter");
    NetworkSliceEvent event = EVENT_SCREEN_OFF;
    std::string commonEventSupport = EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF;
    auto handler = std::make_shared<TestHandler>();
    Singleton<NetworkSliceMsgCenter>::GetInstance().registHandler(MODULE_NETWORKSLICE, handler);
    Singleton<NetworkSliceMsgCenter>::GetInstance().Subscribe(event, MODULE_NETWORKSLICE);
    EventFwk::MatchingSkills matchSkills;
    matchSkills.AddEvent(commonEventSupport);
    EventFwk::CommonEventSubscribeInfo subcriberInfo(matchSkills);
    std::shared_ptr<broadcast_proxy::BroadcastEventSubscriber> subscriber =
        std::make_shared<broadcast_proxy::BroadcastEventSubscriber>(subcriberInfo);
    AAFwk::Want want;
    want.SetAction(commonEventSupport);
    EventFwk::CommonEventData data(want);
    subscriber->OnReceiveEvent(data);
    Singleton<NetworkSliceMsgCenter>::GetInstance().unRegistHandler(MODULE_NETWORKSLICE, handler);
    EXPECT_FALSE(handler == nullptr);
}
 
/**
* @tc.name: NETMANAGER_EXT_WIFI_CONN_STATE
* @tc.desc: recv NETMANAGER_EXT_WIFI_CONN_STATE event
* @tc.type: FUNC
* @tc.require: issue
*/
HWTEST_F(BroadcastProxyTest, NETMANAGER_EXT_WIFI_CONN_STATE, TestSize.Level1)
{
    auto broadcastProxy = std::make_shared<broadcast_proxy>();
    if (broadcastProxy == nullptr) {
        return;
    }
    NETMGR_EXT_LOG_I("NETMANAGER_EXT_WIFI_CONN_STATE enter");
    NetworkSliceEvent event = EVENT_WIFI_CONN_CHANGED;
    std::string commonEventSupport = EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_CONN_STATE;
    auto handler = std::make_shared<TestHandler>();
    Singleton<NetworkSliceMsgCenter>::GetInstance().registHandler(MODULE_NETWORKSLICE, handler);
    Singleton<NetworkSliceMsgCenter>::GetInstance().Subscribe(event, MODULE_NETWORKSLICE);
    EventFwk::MatchingSkills matchSkills;
    matchSkills.AddEvent(commonEventSupport);
    EventFwk::CommonEventSubscribeInfo subcriberInfo(matchSkills);
    std::shared_ptr<broadcast_proxy::BroadcastEventSubscriber> subscriber =
        std::make_shared<broadcast_proxy::BroadcastEventSubscriber>(subcriberInfo);
    AAFwk::Want want;
    want.SetAction(commonEventSupport);
    EventFwk::CommonEventData data(want);
    subscriber->OnReceiveEvent(data);
    Singleton<NetworkSliceMsgCenter>::GetInstance().unRegistHandler(MODULE_NETWORKSLICE, handler);
    EXPECT_FALSE(handler == nullptr);
}
 
/**
* @tc.name: NETMANAGER_EXT_SIM_STATE
* @tc.desc: recv NETMANAGER_EXT_SIM_STATE event
* @tc.type: FUNC
* @tc.require: issue
*/
HWTEST_F(BroadcastProxyTest, NETMANAGER_EXT_SIM_STATE, TestSize.Level1)
{
    auto broadcastProxy = std::make_shared<broadcast_proxy>();
    if (broadcastProxy == nullptr) {
        return;
    }
    NETMGR_EXT_LOG_I("NETMANAGER_EXT_SIM_STATE enter");
    NetworkSliceEvent event = EVENT_HANDLE_SIM_STATE_CHANGED;
    std::string commonEventSupport = EventFwk::CommonEventSupport::COMMON_EVENT_SIM_STATE_CHANGED;
    auto handler = std::make_shared<TestHandler>();
    Singleton<NetworkSliceMsgCenter>::GetInstance().registHandler(MODULE_NETWORKSLICE, handler);
    Singleton<NetworkSliceMsgCenter>::GetInstance().Subscribe(event, MODULE_NETWORKSLICE);
    EventFwk::MatchingSkills matchSkills;
    matchSkills.AddEvent(commonEventSupport);
    EventFwk::CommonEventSubscribeInfo subcriberInfo(matchSkills);
    std::shared_ptr<broadcast_proxy::BroadcastEventSubscriber> subscriber =
        std::make_shared<broadcast_proxy::BroadcastEventSubscriber>(subcriberInfo);
    AAFwk::Want want;
    want.SetAction(commonEventSupport);
    EventFwk::CommonEventData data(want);
    subscriber->OnReceiveEvent(data);
    Singleton<NetworkSliceMsgCenter>::GetInstance().unRegistHandler(MODULE_NETWORKSLICE, handler);
    EXPECT_FALSE(handler == nullptr);
}
 
/**
* @tc.name: NETMANAGER_EXT_AIRPLANE_MODE_CHANGED
* @tc.desc: recv NETMANAGER_EXT_AIRPLANE_MODE_CHANGED event
* @tc.type: FUNC
* @tc.require: issue
*/
HWTEST_F(BroadcastProxyTest, NETMANAGER_EXT_AIRPLANE_MODE_CHANGED, TestSize.Level1)
{
    auto broadcastProxy = std::make_shared<broadcast_proxy>();
    if (broadcastProxy == nullptr) {
        return;
    }
    NETMGR_EXT_LOG_I("NETMANAGER_EXT_AIRPLANE_MODE_CHANGED enter");
    NetworkSliceEvent event = EVENT_AIR_MODE_CHANGED;
    std::string commonEventSupport = EventFwk::CommonEventSupport::COMMON_EVENT_AIRPLANE_MODE_CHANGED;
    auto handler = std::make_shared<TestHandler>();
    Singleton<NetworkSliceMsgCenter>::GetInstance().registHandler(MODULE_NETWORKSLICE, handler);
    Singleton<NetworkSliceMsgCenter>::GetInstance().Subscribe(event, MODULE_NETWORKSLICE);
    EventFwk::MatchingSkills matchSkills;
    matchSkills.AddEvent(commonEventSupport);
    EventFwk::CommonEventSubscribeInfo subcriberInfo(matchSkills);
    std::shared_ptr<broadcast_proxy::BroadcastEventSubscriber> subscriber =
        std::make_shared<broadcast_proxy::BroadcastEventSubscriber>(subcriberInfo);
    AAFwk::Want want;
    want.SetAction(commonEventSupport);
    EventFwk::CommonEventData data(want);
    subscriber->OnReceiveEvent(data);
    Singleton<NetworkSliceMsgCenter>::GetInstance().unRegistHandler(MODULE_NETWORKSLICE, handler);
    EXPECT_FALSE(handler == nullptr);
}
 
/**
* @tc.name: NETMANAGER_EXT_NETWORK_STATE_CHANGED
* @tc.desc: recv NETMANAGER_EXT_NETWORK_STATE_CHANGED event
* @tc.type: FUNC
* @tc.require: issue
*/
HWTEST_F(BroadcastProxyTest, NETMANAGER_EXT_NETWORK_STATE_CHANGED, TestSize.Level1)
{
    auto broadcastProxy = std::make_shared<broadcast_proxy>();
    if (broadcastProxy == nullptr) {
        return;
    }
    NETMGR_EXT_LOG_I("NETMANAGER_EXT_NETWORK_STATE_CHANGED enter");
    NetworkSliceEvent event = EVENT_NETWORK_STATE_CHANGED;
    std::string commonEventSupport = EventFwk::CommonEventSupport::COMMON_EVENT_NETWORK_STATE_CHANGED;
    auto handler = std::make_shared<TestHandler>();
    Singleton<NetworkSliceMsgCenter>::GetInstance().registHandler(MODULE_NETWORKSLICE, handler);
    Singleton<NetworkSliceMsgCenter>::GetInstance().Subscribe(event, MODULE_NETWORKSLICE);
    EventFwk::MatchingSkills matchSkills;
    matchSkills.AddEvent(commonEventSupport);
    EventFwk::CommonEventSubscribeInfo subcriberInfo(matchSkills);
    std::shared_ptr<broadcast_proxy::BroadcastEventSubscriber> subscriber =
        std::make_shared<broadcast_proxy::BroadcastEventSubscriber>(subcriberInfo);
    AAFwk::Want want;
    want.SetAction(commonEventSupport);
    EventFwk::CommonEventData data(want);
    subscriber->OnReceiveEvent(data);
    Singleton<NetworkSliceMsgCenter>::GetInstance().unRegistHandler(MODULE_NETWORKSLICE, handler);
    EXPECT_FALSE(handler == nullptr);
}
 
/**
* @tc.name: NETMANAGER_EXT_CONNECTIVITY_CHANGE
* @tc.desc: recv NETMANAGER_EXT_CONNECTIVITY_CHANGE event
* @tc.type: FUNC
* @tc.require: issue
*/
HWTEST_F(BroadcastProxyTest, NETMANAGER_EXT_CONNECTIVITY_CHANGE, TestSize.Level1)
{
    auto broadcastProxy = std::make_shared<broadcast_proxy>();
    if (broadcastProxy == nullptr) {
        return;
    }
    NETMGR_EXT_LOG_I("NETMANAGER_EXT_CONNECTIVITY_CHANGE enter");
    NetworkSliceEvent event = EVENT_CONNECTIVITY_CHANGE;
    std::string commonEventSupport = EventFwk::CommonEventSupport::COMMON_EVENT_CONNECTIVITY_CHANGE;
    auto handler = std::make_shared<TestHandler>();
    Singleton<NetworkSliceMsgCenter>::GetInstance().registHandler(MODULE_NETWORKSLICE, handler);
    Singleton<NetworkSliceMsgCenter>::GetInstance().Subscribe(event, MODULE_NETWORKSLICE);
    EventFwk::MatchingSkills matchSkills;
    matchSkills.AddEvent(commonEventSupport);
    EventFwk::CommonEventSubscribeInfo subcriberInfo(matchSkills);
    std::shared_ptr<broadcast_proxy::BroadcastEventSubscriber> subscriber =
        std::make_shared<broadcast_proxy::BroadcastEventSubscriber>(subcriberInfo);
    AAFwk::Want want;
    want.SetAction(commonEventSupport);
    EventFwk::CommonEventData data(want);
    subscriber->OnReceiveEvent(data);
    Singleton<NetworkSliceMsgCenter>::GetInstance().unRegistHandler(MODULE_NETWORKSLICE, handler);
    EXPECT_FALSE(handler == nullptr);
}
 
/**
 * @tc.name  : VpnEventObserverTest_001
 * @tc.desc  : Test when VPN is connected then OnVpnStateChanged should publish EVENT_VPN_MODE_CHANGED event
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(VpnEventObserverTest, VpnEventObserverTest_001, TestSize.Level1)
{
    bool isConnected = true;
    broadcast_proxy::VpnEventObserver observer;
    observer.OnVpnStateChanged(isConnected);
    broadcast_proxy proxy_instance;
    EXPECT_TRUE(proxy_instance.subscriber_ == nullptr);
}

}
}
