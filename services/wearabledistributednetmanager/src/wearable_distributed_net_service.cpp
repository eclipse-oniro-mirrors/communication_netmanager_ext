/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "netmanager_base_permission.h"
#include "wearable_distributed_net_service.h"

namespace OHOS {
namespace NetManagerStandard {
REGISTER_SYSTEM_ABILITY_BY_ID(WearableDistributedNetService, COMM_WEARABLE_DISTRIBUTED_NET_ABILITY_ID, true);

WearableDistributedNetService::WearableDistributedNetService(int32_t saId, bool runOnCreate)
    : SystemAbility(saId, runOnCreate) {}

WearableDistributedNetService::~WearableDistributedNetService() = default;

void WearableDistributedNetService::OnStart()
{
    NETMGR_EXT_LOG_I("WearableDistributedNetService::OnStart begin");
    if (state_ == STATE_RUNNING) {
        NETMGR_EXT_LOG_D("WearableDistributedNetService the state is already running");
        return;
    }
    if (!Init()) {
        NETMGR_EXT_LOG_E("WearableDistributedNetService init failed");
        return;
    }
    state_ = STATE_RUNNING;
    NETMGR_EXT_LOG_I("WearableDistributedNetService::OnStart end");
}

void WearableDistributedNetService::OnStop()
{
    state_ = STATE_STOPPED;
    registerToService_ = false;
    if (subscriber_ != nullptr) {
        OHOS::EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber_);
        subscriber_ = nullptr;
    }
}

int32_t WearableDistributedNetService::SetupWearableDistributedNet(int32_t tcpPortId, int32_t udpPortId, bool isMetered)
{
    NETMGR_EXT_LOG_I("Wearable Distributed Net Service Setup Net");
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        NETMGR_EXT_LOG_E("Wearable Distributed Net Service Setup Net no permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return WearableDistributedNetManagement::GetInstance()
        .StartWearableDistributedNetwork(tcpPortId, udpPortId, isMetered);
}

int32_t WearableDistributedNetService::TearDownWearableDistributedNet()
{
    NETMGR_EXT_LOG_I("Wearable Distributed Net Service TearDown Net");
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        NETMGR_EXT_LOG_E("Wearable Distributed Net Service TearDown Net no permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return WearableDistributedNetManagement::GetInstance().StopWearableDistributedNetwork();
}

int32_t WearableDistributedNetService::UpdateMeteredStatus(bool isMetered)
{
    NETMGR_EXT_LOG_I("Update wearable distributed net metered status");
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        NETMGR_EXT_LOG_E("Update wearable distributed net metered status no permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return WearableDistributedNetManagement::GetInstance().UpdateMeteredStatus(isMetered);
}

bool WearableDistributedNetService::Init()
{
    NETMGR_EXT_LOG_I("Wearable Distributed Net Service Init");
    AddSystemAbilityListener(NET_MANAGER_SYS_ABILITY_ID);
    if (!registerToService_) {
        if (!Publish(this)) {
            NETMGR_EXT_LOG_E("Wearable Distributed Net Service Register to sa manager failed");
            return false;
        }
        registerToService_ = true;
    }
    AddSystemAbilityListener(POWER_MANAGER_BATT_SERVICE_ID);
    AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
    if (!SubscribeCommonEvent()) {
        return false;
    }
    return true;
}

void WearableDistributedNetService::UpdateNetScore(const bool isCharging)
{
    WearableDistributedNetManagement::GetInstance().UpdateNetScore(isCharging);
}

bool WearableDistributedNetService::SubscribeCommonEvent()
{
    NETMGR_EXT_LOG_I("Wearable Distributed Net subscribe power event.");
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(Event::COMMON_EVENT_POWER_CONNECTED);
    matchingSkills.AddEvent(Event::COMMON_EVENT_POWER_DISCONNECTED);
    matchingSkills.AddEvent(Event::COMMON_EVENT_BATTERY_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // 1 means CORE_EVENT_PRIORITY
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<ReceiveMessage>(subscribeInfo, *this);
    if (subscriber_ == nullptr) {
        return false;
    }
    return EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_);
}

void WearableDistributedNetService::ReceiveMessage::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    const auto &want = eventData.GetWant();
    const auto &chargingState = want.GetIntParam(PowerMgr::BatteryInfo::COMMON_EVENT_KEY_CHARGE_STATE,
                                                 static_cast<int32_t>(ChargeState::CHARGE_STATE_BUTT));

    NETMGR_EXT_LOG_I("Wearable Distributed Net receive power message: chargingState = %{public}d", chargingState);
    if (chargingState == static_cast<int32_t>(ChargeState::CHARGE_STATE_DISABLE) ||
        chargingState == static_cast<int32_t>(ChargeState::CHARGE_STATE_NONE)) {
        NETMGR_EXT_LOG_I("Wearable Distributed Net receive power disconnected message");
        WearableDistributedNetService_.UpdateNetScore(false);
    } else if (chargingState == static_cast<int32_t>(ChargeState::CHARGE_STATE_ENABLE) ||
        chargingState == static_cast<int32_t>(ChargeState::CHARGE_STATE_FULL)) {
        NETMGR_EXT_LOG_I("Wearable Distributed Net receive power connected message");
        WearableDistributedNetService_.UpdateNetScore(true);
    }
}
} // namespace NetManagerStandard
} // namespace OHOS
