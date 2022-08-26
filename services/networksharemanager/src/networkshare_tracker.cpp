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

#include "networkshare_tracker.h"

#include <system_ability_definition.h>

#include "netsys_controller.h"
#include "netmgr_ext_log_wrapper.h"
#include "networkshare_constants.h"
#include "net_manager_constants.h"
#include "networkshare_state_common.h"

namespace OHOS {
namespace NetManagerStandard {
static constexpr const char *WIFI_AP_DEFAULT_IFACE_NAME = "wlan0";
static constexpr const char *BLUETOOTH_DEFAULT_IFACE_NAME = "bt-pan";

NetworkShareTracker::NetsysCallback::NetsysCallback(NetworkShareTracker &netShareTracker)
    : netShareTracker_(netShareTracker)
{
}

int32_t NetworkShareTracker::NetsysCallback::OnInterfaceAddressUpdated(const std::string &, const std::string &, int,
                                                                       int)
{
    return 0;
}

int32_t NetworkShareTracker::NetsysCallback::OnInterfaceAddressRemoved(const std::string &, const std::string &, int,
                                                                       int)
{
    return 0;
}

int32_t NetworkShareTracker::NetsysCallback::OnInterfaceAdded(const std::string &iface)
{
    netShareTracker_.InterfaceAdded(iface);
    return 0;
}

int32_t NetworkShareTracker::NetsysCallback::OnInterfaceRemoved(const std::string &iface)
{
    netShareTracker_.InterfaceRemoved(iface);
    return 0;
}

int32_t NetworkShareTracker::NetsysCallback::OnInterfaceChanged(const std::string &iface, bool up)
{
    netShareTracker_.InterfaceStatusChanged(iface, up);
    return 0;
}

int32_t NetworkShareTracker::NetsysCallback::OnInterfaceLinkStateChanged(const std::string &iface, bool up)
{
    netShareTracker_.InterfaceStatusChanged(iface, up);
    return 0;
}

int32_t NetworkShareTracker::NetsysCallback::OnRouteChanged(bool, const std::string &, const std::string &,
                                                            const std::string &)
{
    return 0;
}

int32_t NetworkShareTracker::NetsysCallback::OnDhcpSuccess(NetsysControllerCallback::DhcpResult &dhcpResult)
{
    return 0;
}

int32_t NetworkShareTracker::NetsysCallback::OnBandwidthReachedLimit(const std::string &limitName,
                                                                     const std::string &iface)
{
    return 0;
}

NetworkShareTracker::ManagerEventHandler::ManagerEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner)
    : AppExecFwk::EventHandler(runner)
{
}

void NetworkShareTracker::ManagerEventHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    EHandlerEventType eventId = static_cast<EHandlerEventType>(event->GetInnerEventId());
    if (eventId == EHandlerEventType::EVENT_HANDLER_MSG_FIR) {
        NETMGR_EXT_LOG_I("EVENT_HANDLER_MSG_FIR");
    }
    NETMGR_EXT_LOG_W("because eventId is unkonwn.");
}

void NetworkShareTracker::MainSmUpstreamCallback::OnUpstreamStateChanged(int msgName, int param1)
{
    MessageUpstreamInfo temp;
    temp.cmd_ = param1;
    temp.upstreamInfo_ = nullptr;
    NETMGR_EXT_LOG_I("NOTIFY TO Main SM EVENT_UPSTREAM_CALLBACK with one param.");
    NetworkShareTracker::GetInstance().GetMainStateMachine()->MainSmEventHandle(EVENT_UPSTREAM_CALLBACK, temp);
}

void NetworkShareTracker::MainSmUpstreamCallback::OnUpstreamStateChanged(int msgName, int param1, int param2,
                                                                         const std::any &messageObj)
{
    std::shared_ptr<UpstreamNetworkInfo> upstreamInfo = std::any_cast<std::shared_ptr<UpstreamNetworkInfo>>(messageObj);
    if (upstreamInfo != nullptr) {
        NetworkShareTracker::GetInstance().SendSharingUpstreamChange(upstreamInfo->netHandle_);
    }
    MessageUpstreamInfo temp;
    temp.cmd_ = param1;
    temp.upstreamInfo_ = upstreamInfo;
    NETMGR_EXT_LOG_I("NOTIFY TO Main SM EVENT_UPSTREAM_CALLBACK with two param.");
    NetworkShareTracker::GetInstance().GetMainStateMachine()->MainSmEventHandle(EVENT_UPSTREAM_CALLBACK, temp);
}

void NetworkShareTracker::SubSmUpstreamCallback::OnUpdateInterfaceState(
    const std::shared_ptr<NetworkShareSubStateMachine> &paraSubStateMachine, int state, int lastError)
{
    NetworkShareTracker::GetInstance().HandleSubSmUpdateInterfaceState(paraSubStateMachine, state, lastError);
}

NetworkShareTracker::NetSharingSubSmState::NetSharingSubSmState(
    const std::shared_ptr<NetworkShareSubStateMachine> &subStateMachine, bool isNcm)
    : subStateMachine_(subStateMachine),
      lastState_(SUB_SM_STATE_AVAILABLE),
      lastError_(NETWORKSHARE_ERROR_NO_ERROR),
      isNcm_(isNcm)
{
}

void NetworkShareTracker::WifiShareHotspotEventCallback::OnHotspotStateChanged(int state)
{
    NETMGR_EXT_LOG_I("Receive Hotspot state changed event, state[%{public}d]", state);
    Wifi::ApState curState = static_cast<Wifi::ApState>(state);
    switch (curState) {
        case Wifi::ApState::AP_STATE_STARTING:
            break;
        case Wifi::ApState::AP_STATE_STARTED: {
            NetworkShareTracker::GetInstance().EnableWifiSubStateMachine();
            break;
        }
        case Wifi::ApState::AP_STATE_CLOSING:
            break;
        case Wifi::ApState::AP_STATE_CLOSED:
        default: {
            NetworkShareTracker::GetInstance().DisableWifiSubStateMachine();
            break;
        }
    }
}

void NetworkShareTracker::WifiShareHotspotEventCallback::OnHotspotStaJoin(const Wifi::StationInfo &info)
{
    NETMGR_EXT_LOG_I("Receive Hotspot join event.");
}

void NetworkShareTracker::WifiShareHotspotEventCallback::OnHotspotStaLeave(const Wifi::StationInfo &info)
{
    NETMGR_EXT_LOG_I("Receive Hotspot leave event.");
}

OHOS::sptr<OHOS::IRemoteObject> NetworkShareTracker::WifiShareHotspotEventCallback::AsObject()
{
    return nullptr;
}

void NetworkShareTracker::SharingPanObserver::OnConnectionStateChanged(const Bluetooth::BluetoothRemoteDevice &device,
                                                                       int state)
{
    NETMGR_EXT_LOG_I("Recieve bt-pan state changed event, state[%{public}d].", state);
    Bluetooth::BTConnectState curState = static_cast<Bluetooth::BTConnectState>(state);
    switch (curState) {
        case Bluetooth::BTConnectState::CONNECTING:
            break;
        case Bluetooth::BTConnectState::CONNECTED: {
            NetworkShareTracker::GetInstance().EnableBluetoothSubStateMachine();
            break;
        }
        case Bluetooth::BTConnectState::DISCONNECTING:
            break;
        case Bluetooth::BTConnectState::DISCONNECTED:
        default: {
            NetworkShareTracker::GetInstance().DisableBluetoothSubStateMachine();
            break;
        }
    }
}

NetworkShareTracker &NetworkShareTracker::GetInstance()
{
    static NetworkShareTracker instance;
    return instance;
}

bool NetworkShareTracker::Init()
{
    configuration_ = std::make_shared<NetworkShareConfiguration>();

    upstreamNetworkMonitor_ = DelayedSingleton<NetworkShareUpstreamMonitor>::GetInstance();
    monitorRunner_ = AppExecFwk::EventRunner::Create("network_share_monitor");
    if (monitorRunner_ == nullptr) {
        NETMGR_EXT_LOG_E("Init create monitorRunner error");
        return false;
    }
    monitorHandler_ =
        std::make_shared<NetworkShareUpstreamMonitor::MonitorEventHandler>(upstreamNetworkMonitor_, monitorRunner_);
    upstreamNetworkMonitor_->SetOptionData(EVENT_UPSTREAM_CALLBACK, monitorHandler_);
    upstreamNetworkMonitor_->ListenDefaultNetwork();
    std::shared_ptr<MainSmUpstreamCallback> upstreamCallback = std::make_shared<MainSmUpstreamCallback>();
    if (upstreamCallback == nullptr) {
        NETMGR_EXT_LOG_E("Init create upstreamMonitor error");
        return false;
    }
    upstreamNetworkMonitor_->RegisterUpstreamChangedCallback(upstreamCallback);

    mainStateMachine_ = std::make_shared<NetworkShareMainStateMachine>(upstreamNetworkMonitor_);
    if (mainStateMachine_ == nullptr) {
        NETMGR_EXT_LOG_E("Init create mainStateMachine error");
        return false;
    }

    wifiHotspotPtr_ = Wifi::WifiHotspot::GetInstance(WIFI_HOTSPOT_ABILITY_ID);
    if (wifiHotspotPtr_ == nullptr) {
        NETMGR_EXT_LOG_E("Init create wifiHotspot error");
        return false;
    }
    netsysCallback_ = new (std::nothrow) NetsysCallback(*this);
    if (netsysCallback_ == nullptr) {
        NETMGR_EXT_LOG_E("Init create netsysCallback error");
        return false;
    }
    NetsysController::GetInstance().RegisterCallback(netsysCallback_);

    RegisterWifiApCallback();
    RegisterBtPanCallback();

    eventRunner_ = AppExecFwk::EventRunner::Create("network_share_tracker");
    if (eventRunner_ == nullptr) {
        NETMGR_EXT_LOG_E("create eventRunner error");
        return false;
    }
    eventHandler_ = std::make_shared<NetworkShareTracker::ManagerEventHandler>(eventRunner_);
    if (eventHandler_ == nullptr) {
        NETMGR_EXT_LOG_E("create eventHandler error");
        return false;
    }

    isNetworkSharing_ = false;
    NETMGR_EXT_LOG_I("Tracker Init sucessful.");
    return true;
}

void NetworkShareTracker::RegisterWifiApCallback()
{
    sptr<WifiShareHotspotEventCallback> wifiHotspotCallback =
        sptr<WifiShareHotspotEventCallback>(new (std::nothrow) WifiShareHotspotEventCallback());
    if (wifiHotspotPtr_ != nullptr) {
        int32_t ret = wifiHotspotPtr_->RegisterCallBack(wifiHotspotCallback);
        if (ret != Wifi::ErrCode::WIFI_OPT_SUCCESS) {
            NETMGR_EXT_LOG_E("Register wifi hotspot callback error[%{public}d].", ret);
        }
    }
}

void NetworkShareTracker::RegisterBtPanCallback()
{
    Bluetooth::Pan *profile_ = Bluetooth::Pan::GetProfile();
    if (profile_ != nullptr) {
        panObserver_ = std::make_shared<SharingPanObserver>();
        if (panObserver_ != nullptr) {
            profile_->RegisterObserver(panObserver_.get());
        }
    }
}

void NetworkShareTracker::Uninit()
{
    Bluetooth::Pan *profile_ = Bluetooth::Pan::GetProfile();
    if (profile_ == nullptr || panObserver_ == nullptr) {
        NETMGR_EXT_LOG_E("bt-pan profile or observer is null.");
        return;
    }
    profile_->DeregisterObserver(panObserver_.get());
    NETMGR_EXT_LOG_I("Uninit successful.");
}

std::shared_ptr<NetworkShareMainStateMachine> &NetworkShareTracker::GetMainStateMachine()
{
    return mainStateMachine_;
}

void NetworkShareTracker::HandleSubSmUpdateInterfaceState(const std::shared_ptr<NetworkShareSubStateMachine> &who,
                                                          int32_t state, int32_t lastError)
{
    if (who == nullptr) {
        NETMGR_EXT_LOG_E("subsm is null.");
        return;
    }
    std::map<std::string, std::shared_ptr<NetSharingSubSmState>>::iterator iter =
        subStateMachineMap_.find(who->GetInterfaceName());
    if (iter != subStateMachineMap_.end()) {
        std::shared_ptr<NetSharingSubSmState> shareState = iter->second;
        if (shareState->lastState_ != state) {
            NETMGR_EXT_LOG_I("iface=%{public}s state is change from[%{public}d] to[%{public}d].", (iter->first).c_str(),
                             shareState->lastState_, state);
            shareState->lastState_ = state;
            shareState->lastError_ = lastError;
            SendIfaceSharingStateChange(who->GetNetShareType(), iter->first, SubSmStateToExportState(state));
        } else {
            shareState->lastError_ = lastError;
            NETMGR_EXT_LOG_I("iface=%{public}s state is not change.", (iter->first).c_str());
        }
    } else {
        NETMGR_EXT_LOG_W("iface=%{public}s is not find", (who->GetInterfaceName()).c_str());
    }

    if (lastError == NETWORKSHARE_ERROR_INTERNAL_ERROR) {
        MessageIfaceActive temp;
        temp.value_ = 0;
        temp.subsm_ = who;
        NETMGR_EXT_LOG_W("NOTIFY TO Main SM CMD_CLEAR_ERROR.");
        mainStateMachine_->MainSmEventHandle(CMD_CLEAR_ERROR, temp);
    }
    int which;
    switch (state) {
        case SUB_SM_STATE_AVAILABLE:
        case SUB_SM_STATE_UNAVAILABLE:
            which = EVENT_IFACE_SM_STATE_INACTIVE;
            break;
        case SUB_SM_STATE_SHARED:
            which = EVENT_IFACE_SM_STATE_ACTIVE;
            break;
        default:
            NETMGR_EXT_LOG_E("Unknown interface state=%{public}d", state);
            return;
    }
    MessageIfaceActive temp;
    temp.value_ = state;
    temp.subsm_ = who;
    NETMGR_EXT_LOG_W("NOTIFY TO Main SM EVENT[%{public}d].", which);
    mainStateMachine_->MainSmEventHandle(which, temp);
    SendGlobalSharingStateChange();
}

int32_t NetworkShareTracker::IsNetworkSharingSupported()
{
    return configuration_->IsNetworkSharingSupported() ? NETWORKSHARE_IS_SUPPORTED : NETWORKSHARE_IS_UNSUPPORTED;
}

int32_t NetworkShareTracker::IsSharing()
{
    bool isSharing = false;
    std::lock_guard lock(mutex_);
    std::map<std::string, std::shared_ptr<NetSharingSubSmState>>::iterator iter = subStateMachineMap_.begin();
    for (; iter != subStateMachineMap_.end(); iter++) {
        std::shared_ptr<NetSharingSubSmState> shareState = iter->second;
        if (shareState->lastState_ == SUB_SM_STATE_SHARED) {
            isSharing = true;
            break;
        }
    }
    return isSharing ? NETWORKSHARE_IS_SHARING : NETWORKSHARE_IS_UNSHARING;
}

int32_t NetworkShareTracker::StartNetworkSharing(const SharingIfaceType &type)
{
    auto fit = find(clientRequestsVector_.begin(), clientRequestsVector_.end(), type);
    if (fit != clientRequestsVector_.end()) {
        int ret = EnableNetSharingInternal(type, false);
        if (ret != NETWORKSHARE_SUCCESS) {
            NETMGR_EXT_LOG_E("stop current [%{public}d] sharing error [%{public}ul]", static_cast<int32_t>(type), ret);
            return ret;
        }
    } else {
        clientRequestsVector_.push_back(type);
    }

    return EnableNetSharingInternal(type, true);
}

int32_t NetworkShareTracker::StopNetworkSharing(const SharingIfaceType &type)
{
    auto fit = find(clientRequestsVector_.begin(), clientRequestsVector_.end(), type);
    if (fit != clientRequestsVector_.end()) {
        clientRequestsVector_.erase(fit);
    }

    return EnableNetSharingInternal(type, false);
}

std::vector<std::string> NetworkShareTracker::GetSharableRegexs(SharingIfaceType type)
{
    switch (type) {
        case SharingIfaceType::SHARING_WIFI: {
            return configuration_->GetWifiIfaceRegexs();
        }
        case SharingIfaceType::SHARING_USB: {
            return configuration_->GetUsbIfaceRegexs();
        }
        case SharingIfaceType::SHARING_BLUETOOTH: {
            return configuration_->GetBluetoothIfaceRegexs();
        }
        default: {
            NETMGR_EXT_LOG_E("type[%{public}d] is unkonwn.", type);
            return {};
        }
    }
}

bool NetworkShareTracker::IsInterfaceMatchType(const std::string &iface, const SharingIfaceType &type)
{
    if (type == SharingIfaceType::SHARING_WIFI && configuration_->IsWifiIface(iface)) {
        return true;
    }
    if (type == SharingIfaceType::SHARING_USB && configuration_->IsUsbIface(iface)) {
        return true;
    }
    if (type == SharingIfaceType::SHARING_BLUETOOTH && configuration_->IsBluetoothIface(iface)) {
        return true;
    }
    return false;
}

int32_t NetworkShareTracker::GetSharingState(SharingIfaceType type, SharingIfaceState &state)
{
    bool isFindType = false;
    state = SharingIfaceState::SHARING_NIC_CAN_SERVER;
    std::lock_guard lock(mutex_);
    for (auto &iter : subStateMachineMap_) {
        if (IsInterfaceMatchType(iter.first, type)) {
            std::shared_ptr<NetSharingSubSmState> subsmState = iter.second;
            if (subsmState == nullptr) {
                NETMGR_EXT_LOG_W("subsmState is null.");
                continue;
            }
            if (subsmState->lastState_ == SUB_SM_STATE_UNAVAILABLE) {
                state = SharingIfaceState::SHARING_NIC_ERROR;
                isFindType = true;
                break;
            }
            if (subsmState->lastState_ == SUB_SM_STATE_AVAILABLE) {
                state = SharingIfaceState::SHARING_NIC_CAN_SERVER;
                isFindType = true;
                break;
            }
            if (subsmState->lastState_ == SUB_SM_STATE_SHARED) {
                state = SharingIfaceState::SHARING_NIC_SERVING;
                isFindType = true;
                break;
            }
            NETMGR_EXT_LOG_E("lastState_=%{public}d is unknown data.", subsmState->lastState_);
        } else {
            NETMGR_EXT_LOG_W("iface=%{public}s is not match type[%{public}d]", iter.first.c_str(), type);
        }
    }
    if (!isFindType) {
        NETMGR_EXT_LOG_W("type=%{public}d is not find, used default value.", type);
    }
    return NETWORKSHARE_SUCCESS;
}

std::vector<std::string> NetworkShareTracker::GetNetSharingIfaces(const SharingIfaceState &state)
{
    std::vector<std::string> ifaces;
    std::lock_guard lock(mutex_);
    for_each(subStateMachineMap_.begin(), subStateMachineMap_.end(),
             [&](std::map<std::string, std::shared_ptr<NetSharingSubSmState>>::reference iter) {
                 std::shared_ptr<NetSharingSubSmState> subsmState = iter.second;
                 if (subsmState == nullptr) {
                     NETMGR_EXT_LOG_W("iface=%{public}s subsmState is null.", (iter.first).c_str());
                 } else {
                     NETMGR_EXT_LOG_I("iface=%{public}s, state=%{public}d", (iter.first).c_str(),
                                      subsmState->lastState_);
                     if ((state == SharingIfaceState::SHARING_NIC_ERROR) &&
                         (subsmState->lastState_ == SUB_SM_STATE_UNAVAILABLE)) {
                         ifaces.push_back(iter.first);
                     } else if ((state == SharingIfaceState::SHARING_NIC_CAN_SERVER) &&
                                (subsmState->lastState_ == SUB_SM_STATE_AVAILABLE)) {
                         ifaces.push_back(iter.first);
                     } else if ((state == SharingIfaceState::SHARING_NIC_SERVING) &&
                                (subsmState->lastState_ == SUB_SM_STATE_SHARED)) {
                         ifaces.push_back(iter.first);
                     }
                 }
             });

    return ifaces;
}

int32_t NetworkShareTracker::RegisterSharingEvent(sptr<ISharingEventCallback> callback)
{
    if (callback == nullptr) {
        NETMGR_EXT_LOG_E("callback is null.");
        return NETWORKSHARE_ERROR;
    }
    sharingEventCallback_ = callback;
    return NETWORKSHARE_SUCCESS;
}

int32_t NetworkShareTracker::UnregisterSharingEvent(sptr<ISharingEventCallback> callback)
{
    sharingEventCallback_ = nullptr;
    return NETWORKSHARE_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS
