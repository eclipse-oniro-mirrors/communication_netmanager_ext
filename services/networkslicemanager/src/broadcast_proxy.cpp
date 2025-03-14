/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "broadcast_proxy.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "netsys_controller.h"
#include "sim_state_type.h"
#include "networkslicemsgcenter.h"
 
namespace OHOS {
namespace NetManagerStandard {
static const int32_t DEFAULT_VALUE = -1;

broadcast_proxy::broadcast_proxy()
{
    InitBroadCastHandleMap();
    Subscribe();
}
 
broadcast_proxy::~broadcast_proxy()
{
    UnSubscribe();
}
 
void broadcast_proxy::InitBroadCastHandleMap()
{
    systemEventHandleMap_ = {
        {EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON, &broadcast_proxy::HandleScreenOnEvent},
        {EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF, &broadcast_proxy::HandleScreenOffEvent},
        {EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_CONN_STATE, &broadcast_proxy::HandleWifiConnEvent},
        {EventFwk::CommonEventSupport::COMMON_EVENT_SIM_STATE_CHANGED, &broadcast_proxy::HandleSimStateEvent},
        {EventFwk::CommonEventSupport::COMMON_EVENT_AIRPLANE_MODE_CHANGED, &broadcast_proxy::HandleAirPlaneModeEvent},
        {EventFwk::CommonEventSupport::COMMON_EVENT_NETWORK_STATE_CHANGED, &broadcast_proxy::HandleNetWorkStateChanged},
        {EventFwk::CommonEventSupport::COMMON_EVENT_CONNECTIVITY_CHANGE, &broadcast_proxy::HandleConnectivityChanged},
    };
}
 
void broadcast_proxy::Subscribe()
{
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        NETMGR_EXT_LOG_E("samgrProxy is nullptr");
        return;
    }
    statusChangeListener_ = new (std::nothrow) SystemAbilityListener();
    if (statusChangeListener_ == nullptr) {
        NETMGR_EXT_LOG_E("statusChangeListener_ is nullptr");
        return;
    }
    samgrProxy->SubscribeSystemAbility(COMMON_EVENT_SERVICE_ID, statusChangeListener_);
    samgrProxy->SubscribeSystemAbility(APP_MGR_SERVICE_ID, statusChangeListener_);
    samgrProxy->SubscribeSystemAbility(COMM_VPN_MANAGER_SYS_ABILITY_ID, statusChangeListener_);
    samgrProxy->SubscribeSystemAbility(WIFI_DEVICE_SYS_ABILITY_ID, statusChangeListener_);
    samgrProxy->SubscribeSystemAbility(COMM_NETSYS_NATIVE_SYS_ABILITY_ID, statusChangeListener_);
}
 
void broadcast_proxy::UnSubscribe()
{
    NETMGR_EXT_LOG_E("UnSubscribe enter");
    if (statusChangeListener_ != nullptr) {
        auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgrProxy != nullptr) {
            samgrProxy->UnSubscribeSystemAbility(COMMON_EVENT_SERVICE_ID, statusChangeListener_);
            samgrProxy->UnSubscribeSystemAbility(APP_MGR_SERVICE_ID, statusChangeListener_);
            samgrProxy->UnSubscribeSystemAbility(COMM_VPN_MANAGER_SYS_ABILITY_ID, statusChangeListener_);
            samgrProxy->UnSubscribeSystemAbility(WIFI_DEVICE_SYS_ABILITY_ID, statusChangeListener_);
            samgrProxy->UnSubscribeSystemAbility(COMM_NETSYS_NATIVE_SYS_ABILITY_ID, statusChangeListener_);
        }
        statusChangeListener_ = nullptr;
    }
}
 
void broadcast_proxy::SubscribeCommonEvent()
{
    EventFwk::MatchingSkills matchSkills;
    for (const auto &e : systemEventHandleMap_) {
        matchSkills.AddEvent(e.first);
    }
    EventFwk::CommonEventSubscribeInfo subcriberInfo(matchSkills);
    if (subscriber_ == nullptr) {
        subscriber_ = std::make_shared<BroadcastEventSubscriber>(subcriberInfo);
    }
    if (!EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_)) {
        NETMGR_EXT_LOG_E("system event register fail.");
    }
}

void broadcast_proxy::UnSubscribeCommonEvent()
{
    if (subscriber_ == nullptr) {
        return;
    }
    if (!EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber_)) {
        NETMGR_EXT_LOG_E("system event unregister fail.");
    }
    subscriber_ = nullptr;
}
 
void broadcast_proxy::SubscribeApplicationState()
{
    if (appAwareObserver_ == nullptr) {
        appAwareObserver_ = new (std::nothrow) AppAwareObserver();
    }
    if (appAwareObserver_ == nullptr) {
        NETMGR_EXT_LOG_E("failed to create appAwareObserver_ instance");
        return;
    }
    sptr<ISystemAbilityManager> samgrClient = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrClient == nullptr) {
        NETMGR_EXT_LOG_E("GetSystemAbilityManager failed!");
        return;
    }
 
    sptr<AppExecFwk::IAppMgr> iAppMgr =
        iface_cast<AppExecFwk::IAppMgr>(samgrClient->GetSystemAbility(APP_MGR_SERVICE_ID));
    if (iAppMgr == nullptr) {
        NETMGR_EXT_LOG_E("Subscribe:iAppMgr SA not ready, wait for the SA Added callback");
        return;
    }
    iAppMgr->RegisterApplicationStateObserver(appAwareObserver_);
}
 
void broadcast_proxy::UnSubscribeApplicationState()
{
    if (appAwareObserver_ == nullptr) {
        return;
    }
    sptr<ISystemAbilityManager> samgrClient = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrClient == nullptr) {
        NETMGR_EXT_LOG_E("GetSystemAbilityManager failed!");
        return;
    }
 
    sptr<AppExecFwk::IAppMgr> iAppMgr =
        iface_cast<AppExecFwk::IAppMgr>(samgrClient->GetSystemAbility(APP_MGR_SERVICE_ID));
    if (iAppMgr != nullptr) {
        iAppMgr->UnregisterApplicationStateObserver(appAwareObserver_);
    }
    appAwareObserver_ = nullptr;
}

void broadcast_proxy::SystemAbilityListener::RegisterVpnEventCallback()
{
    NETMGR_EXT_LOG_E("RegisterVpnEventCallback start.");
    if (vpnEventObserver_ == nullptr) {
        vpnEventObserver_ = new (std::nothrow) broadcast_proxy::VpnEventObserver();
    }
    if (NetManagerStandard::NetworkVpnClient::GetInstance().RegisterVpnEvent(vpnEventObserver_) != 0) {
        NETMGR_EXT_LOG_E("vpn event observer register failed");
    }
    NETMGR_EXT_LOG_E("RegisterVpnEventCallback success.");
}

void broadcast_proxy::SystemAbilityListener::RegisterDnsResultCallback()
{
    NETMGR_EXT_LOG_E("RegisterDnsResultCallback start.");
    if (dnsResultCallback_ == nullptr) {
        dnsResultCallback_ = std::make_unique<DnsResultCallback>().release();
        NetsysController::GetInstance().RegisterDnsResultCallback(dnsResultCallback_, 0);
    }
    NETMGR_EXT_LOG_E("RegisterVpnEventCallback success.");
}

void broadcast_proxy::VpnEventObserver::OnVpnStateChanged(const bool &isConnected)
{
    NETMGR_EXT_LOG_E("vpn state changed. cur state: %{public}d", isConnected);
    Singleton<NetworkSliceMsgCenter>::GetInstance().Publish(EVENT_VPN_MODE_CHANGED, isConnected);
}
 
void broadcast_proxy::HandleSystemEvent(const EventFwk::CommonEventData& eventData)
{
    auto action = eventData.GetWant().GetAction();
    auto it = systemEventHandleMap_.find(action);
    if (it == systemEventHandleMap_.end()) {
        return;
    }
    auto handleFunc = it->second;
    if (handleFunc != nullptr) {
        (this->*handleFunc)(eventData);
    }
}
 
void broadcast_proxy::HandleScreenOnEvent(const EventFwk::CommonEventData& eventData)
{
    NETMGR_EXT_LOG_E("HandleScreenOnEvent");
    Singleton<NetworkSliceMsgCenter>::GetInstance().Publish(EVENT_SCREEN_ON);
}
 
void broadcast_proxy::HandleScreenOffEvent(const EventFwk::CommonEventData& eventData)
{
    NETMGR_EXT_LOG_E("HandleScreenOffEvent");
    Singleton<NetworkSliceMsgCenter>::GetInstance().Publish(EVENT_SCREEN_OFF);
}

void broadcast_proxy::HandleWifiConnEvent(const EventFwk::CommonEventData& eventData)
{
    int32_t state = eventData.GetCode();
    if (state == WifiConnected || state == WifiDisconnected) {
        Singleton<NetworkSliceMsgCenter>::GetInstance().Publish(EVENT_WIFI_CONN_CHANGED, state);
    }
    NETMGR_EXT_LOG_E("HandleWifiConnEvent, state:%{public}d", state);
}
 
void broadcast_proxy::HandleAirPlaneModeEvent(const EventFwk::CommonEventData& eventData)
{
    int32_t state = eventData.GetCode();
    NETMGR_EXT_LOG_E("HandleAirPlaneModeEvent, status:%{public}d", state);
    Singleton<NetworkSliceMsgCenter>::GetInstance().Publish(EVENT_AIR_MODE_CHANGED, state);
}

void broadcast_proxy::HandleNetWorkStateChanged(const EventFwk::CommonEventData &eventData)
{
    NETMGR_EXT_LOG_I("Networkslice HandleNetWorkStateChanged");
    auto want = eventData.GetWant();
    if (want.GetAction() != EventFwk::CommonEventSupport::COMMON_EVENT_NETWORK_STATE_CHANGED) {
        NETMGR_EXT_LOG_E("Common Event NetWorkStateChanged Error");
        return;
    }
    auto networkStandard = std::make_shared<NetworkStandard>();
    networkStandard->slotId = want.GetIntParam("slotId", -1);
    networkStandard->networkState = want.GetStringParam("networkState");
    NETMGR_EXT_LOG_E("Networkslice slot = %{public}d, nwState : %{public}s",
        networkStandard->slotId, networkStandard->networkState.c_str());
    Singleton<NetworkSliceMsgCenter>::GetInstance().Publish(EVENT_NETWORK_STATE_CHANGED, networkStandard);
}
 
void broadcast_proxy::HandleConnectivityChanged(const EventFwk::CommonEventData &eventData)
{
    NETMGR_EXT_LOG_E("HandleConnectivityChanged");
    Singleton<NetworkSliceMsgCenter>::GetInstance().Publish(EVENT_CONNECTIVITY_CHANGE);
}

void broadcast_proxy::BroadcastEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData& data)
{
    if (!broadcastFfrtQueue_) {
        NETMGR_EXT_LOG_E("broadcastFfrtQueue is null");
        return;
    }
    auto taskFunc = [data]() { DelayedSingleton<broadcast_proxy>::GetInstance()->HandleSystemEvent(data); };
    broadcastFfrtQueue_->submit(taskFunc);
}
 
void broadcast_proxy::AppAwareObserver::OnForegroundApplicationChanged(const AppExecFwk::AppStateData& appStateData)
{
    NETMGR_EXT_LOG_I("AppAwareObserver::OnForegroundApplicationChanged ");
    int32_t uid = appStateData.uid;
    int32_t pid = appStateData.pid;
    int32_t state = appStateData.state;
    std::string bundleName = appStateData.bundleName;
    bool isFocused = appStateData.isFocused;
    NETMGR_EXT_LOG_I("uid:%{public}d, pid:%{public}d, state:%{public}d, isFocused:%{public}d, %{public}s",
        uid, pid, state, isFocused, bundleName.c_str());
    if ((state == static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_FOREGROUND) && !isFocused) ||
        (state == static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_BACKGROUND))) {
        return;
    }
    if (state == static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_FOREGROUND) &&
        lastAppStateData.bundleName != bundleName) {
        if (lastAppStateData.bundleName != "") {
            lastAppStateData.state = static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_BACKGROUND);
            NETMGR_EXT_LOG_I("publish lastAppStateData, bundleName: %{public}s, %{public}d",
                lastAppStateData.bundleName.c_str(), lastAppStateData.state);
            auto callback = std::make_shared<AppExecFwk::AppStateData>(lastAppStateData);
        }
        lastAppStateData = appStateData;
    }
    NETMGR_EXT_LOG_I("NgrExt publish bundleName: %{public}s, %{public}d", bundleName.c_str(), state);
    std::shared_ptr<AppExecFwk::AppStateData> msg = std::make_shared<AppExecFwk::AppStateData>(appStateData);
    Singleton<NetworkSliceMsgCenter>::GetInstance().Publish(EVENT_FOREGROUND_APP_CHANGED, msg);
}

void broadcast_proxy::HandleSimStateEvent(const EventFwk::CommonEventData& eventData)
{
    NETMGR_EXT_LOG_I("HandleSimStateEvent start");
    auto want = eventData.GetWant();
    if (want.GetAction() != EventFwk::CommonEventSupport::COMMON_EVENT_SIM_STATE_CHANGED) {
        NETMGR_EXT_LOG_E("Common Event CellDataStateChanged Dispached Error");
        return;
    }
    auto simState = std::make_shared<SimState>();
    simState->slotId = want.GetIntParam("slotId", DEFAULT_VALUE);
    simState->simStatus = want.GetIntParam("state", DEFAULT_VALUE);
    NETMGR_EXT_LOG_I("Netmgr HandleSimStateEvent: slotId:%{public}d, simStatus:%{public}d",
        simState->slotId, simState->simStatus);
    if (simState->simStatus == (int)OHOS::Telephony::SimState::SIM_STATE_LOADED) {
        Singleton<NetworkSliceMsgCenter>::GetInstance().Publish(EVENT_HANDLE_SIM_STATE_CHANGED, simState);
    }
}

void broadcast_proxy::SystemAbilityListener::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    NETMGR_EXT_LOG_E("OnAddSystemAbility: systemAbilityId:%{public}d", systemAbilityId);
    switch (systemAbilityId) {
        case COMMON_EVENT_SERVICE_ID:
            DelayedSingleton<broadcast_proxy>::GetInstance()->SubscribeCommonEvent();
            break;
        case APP_MGR_SERVICE_ID:
            DelayedSingleton<broadcast_proxy>::GetInstance()->SubscribeApplicationState();
            break;
        case COMM_VPN_MANAGER_SYS_ABILITY_ID:
            RegisterVpnEventCallback();
            break;
        case COMM_NETSYS_NATIVE_SYS_ABILITY_ID:
            RegisterDnsResultCallback();
            break;
        default:
            break;
    }
}

void broadcast_proxy::SystemAbilityListener::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    NETMGR_EXT_LOG_I("OnRemoveSystemAbility: systemAbilityId:%{public}d", systemAbilityId);
}

}
}
