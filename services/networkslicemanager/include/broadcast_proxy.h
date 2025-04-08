/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#ifndef BROADCAST_PROXY_H
#define BROADCAST_PROXY_H

#pragma once
#include <memory>
#include "system_ability_status_change_stub.h"
#include "application_state_observer_stub.h"
#include "common_event_subscriber.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "singleton.h"
#include "networkvpn_client.h"
#include "ffrt.h"
#include "networksliceutil.h"
#include "app_mgr_interface.h"
#include "app_mgr_proxy.h"
#include "dns_result_callback.h"
 
namespace OHOS {
namespace NetManagerStandard {
const int32_t SUB_NUM = 3;
enum class CallStatus {
    CALL_STATUS_UNKNOWN = -1,
    CALL_STATUS_ACTIVE = 0,
    CALL_STATUS_HOLDING = 1,
    CALL_STATUS_DIALING = 2,
    CALL_STATUS_ALERTING = 3,
    CALL_STATUS_INCOMING = 4,
    CALL_STATUS_WAITING = 5,
    CALL_STATUS_DISCONNECTED = 6,
    CALL_STATUS_DISCONNECTING = 7,
    CALL_STATUS_IDLE = 8
};
 
struct CellState {
    int32_t slotId;
    int32_t dataState;
    int32_t networkType;
};

const int32_t SLOT_0 = 0;
const int32_t SLOT_1 = 1;
 
struct SimState {
    int32_t slotId;
    int32_t simStatus;
};
 
struct NetworkStandard {
    int32_t slotId;
    std::string networkState;
};
 
class broadcast_proxy : public std::enable_shared_from_this<broadcast_proxy> {
public:
    broadcast_proxy();
    virtual ~broadcast_proxy();
 
private:
    void Subscribe();
    void UnSubscribe();
    using BroadcastFunc = void (broadcast_proxy::*)(const EventFwk::CommonEventData& data);
    void SubscribeCommonEvent();
    void UnSubscribeCommonEvent();
    void SubscribeApplicationState();
    void UnSubscribeApplicationState();
    void InitBroadCastHandleMap();
    void HandleSystemEvent(const EventFwk::CommonEventData& eventData);
    void HandleScreenOnEvent(const EventFwk::CommonEventData& eventData);
    void HandleScreenOffEvent(const EventFwk::CommonEventData& eventData);
    void HandleWifiConnEvent(const EventFwk::CommonEventData& eventData);
    void HandleSimStateEvent(const EventFwk::CommonEventData& eventData);
    void HandleAirPlaneModeEvent(const EventFwk::CommonEventData& eventData);
    void HandleNetWorkStateChanged(const EventFwk::CommonEventData &eventData);
    void HandleConnectivityChanged(const EventFwk::CommonEventData &eventData);
private:
    class BroadcastEventSubscriber : public EventFwk::CommonEventSubscriber {
    public:
        BroadcastEventSubscriber(const EventFwk::CommonEventSubscribeInfo &info)
            : EventFwk::CommonEventSubscriber(info),
              broadcastFfrtQueue_(std::make_shared<ffrt::queue>("BoosterNetBroadCast")) {}
        ~BroadcastEventSubscriber() = default;
        void OnReceiveEvent(const EventFwk::CommonEventData &data) override;
    private:
        std::shared_ptr<ffrt::queue> broadcastFfrtQueue_ = nullptr;
    };
 
    class AppAwareObserver : public AppExecFwk::ApplicationStateObserverStub {
    public:
        AppAwareObserver() = default;
        ~AppAwareObserver() = default;
 
        void OnForegroundApplicationChanged(const AppExecFwk::AppStateData& appStateData) override;
    private:
        AppExecFwk::AppStateData lastAppStateData;
    };
    class VpnEventObserver : public NetManagerStandard::VpnSetUpEventCallback {
    public:
        VpnEventObserver() = default;
        ~VpnEventObserver() = default;
        int32_t OnVpnStateChanged(bool &isConnected) override;
    };

    class SystemAbilityListener : public SystemAbilityStatusChangeStub {
    public:
        SystemAbilityListener() = default;
        ~SystemAbilityListener() = default;
        void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
        void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    private:
        void RegisterVpnEventCallback();
        void RegisterDnsResultCallback();
 
    private:
        sptr<VpnEventObserver> vpnEventObserver_ = nullptr;
        sptr<DnsResultCallback> dnsResultCallback_ = nullptr;
    };

    sptr<ISystemAbilityStatusChange> statusChangeListener_ = nullptr;
    std::map<std::string, BroadcastFunc> systemEventHandleMap_;
    std::shared_ptr<BroadcastEventSubscriber> subscriber_ = nullptr;
    sptr<AppAwareObserver> appAwareObserver_ = nullptr;
    int32_t WifiConnected = 4;
    int32_t WifiDisconnected = 6;
};
} // namespace NetManagerStandard
} // namespace OHOS

#endif  // BROADCAST_PROXY_H
