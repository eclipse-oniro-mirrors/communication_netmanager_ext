/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include <net/if.h>
#include <netinet/in.h>
#include <regex>
#include <securec.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include "battery_srv_client.h"
#include "net_manager_constants.h"
#include "net_manager_ext_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "netsys_controller.h"
#include "network_sharing.h"
#include "networkshare_constants.h"
#include "networkshare_state_common.h"
#ifdef SHARE_NOTIFICATION_ENABLE
#include "networkshare_notification.h"
#endif
#include "system_ability_definition.h"
#include "sys_timer.h"
#ifdef USB_MODOULE
#include "usb_errors.h"
#include "usb_srv_client.h"
#include "usb_srv_support.h"
#endif
#ifdef SHARE_TRAFFIC_LIMIT_ENABLE
#include "networkshare_trafficlimit.h"
#endif

#define IFACENAME_LEN 20

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr const char *BLUETOOTH_DEFAULT_IFACE_NAME = "bt-pan";
#ifdef WIFI_MODOULE
constexpr const char *WIFI_AP_DEFAULT_IFACE_NAME = "wlan0";
constexpr const char *ERROR_MSG_ENABLE_WIFI = "Enable Wifi Iface failed";
constexpr const char *ERROR_MSG_DISABLE_WIFI = "Disable Wifi Iface failed";
constexpr const int IDLE_SOFTAP_STOP_INTERVAL_MS = 10 * 60 * 1000;
#endif
#ifdef BLUETOOTH_MODOULE
constexpr const char *ERROR_MSG_ENABLE_BTPAN = "Enable BlueTooth Iface failed";
constexpr const char *ERROR_MSG_DISABLE_BTPAN = "Disable BlueTooth Iface failed";
#endif
constexpr int32_t BYTE_TRANSFORM_KB = 1024;
constexpr int32_t MAX_CALLBACK_COUNT = 100;
}
constexpr const SharingIfaceType SHARE_VALID_INTERFACES[3] = {SharingIfaceType::SHARING_WIFI,
    SharingIfaceType::SHARING_USB, SharingIfaceType::SHARING_BLUETOOTH};

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
    NetworkShareTracker::GetInstance().InterfaceAdded(iface);
    return 0;
}

int32_t NetworkShareTracker::NetsysCallback::OnInterfaceRemoved(const std::string &iface)
{
    NetworkShareTracker::GetInstance().InterfaceRemoved(iface);
    return 0;
}

int32_t NetworkShareTracker::NetsysCallback::OnInterfaceChanged(const std::string &iface, bool up)
{
    NetworkShareTracker::GetInstance().InterfaceStatusChanged(iface, up);
    return 0;
}

int32_t NetworkShareTracker::NetsysCallback::OnInterfaceLinkStateChanged(const std::string &iface, bool up)
{
    NetworkShareTracker::GetInstance().InterfaceStatusChanged(iface, up);
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
    if (event == nullptr) {
        NETMGR_EXT_LOG_I("event is null");
        return;
    }
    EHandlerEventType eventId = static_cast<EHandlerEventType>(event->GetInnerEventId());
    if (eventId == EHandlerEventType::EVENT_HANDLER_MSG_FIR) {
        NETMGR_EXT_LOG_I("EVENT_HANDLER_MSG_FIR");
        return;
    }
    NETMGR_EXT_LOG_W("because eventId is unkonwn.");
}

void NetworkShareTracker::MainSmUpstreamCallback::OnUpstreamStateChanged(int32_t msgName, int32_t param1)
{
    (void)msgName;
    MessageUpstreamInfo temp;
    temp.cmd_ = param1;
    temp.upstreamInfo_ = nullptr;
    NETMGR_EXT_LOG_I("NOTIFY TO Main SM EVENT_UPSTREAM_CALLBACK with one param.");
    NetworkShareTracker::GetInstance().GetMainStateMachine()->MainSmEventHandle(EVENT_UPSTREAM_CALLBACK, temp);
}

void NetworkShareTracker::MainSmUpstreamCallback::OnUpstreamStateChanged(int32_t msgName, int32_t param1,
                                                                         int32_t param2, const std::any &messageObj)
{
    (void)msgName;
    (void)param2;
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
      lastError_(NETMANAGER_EXT_SUCCESS),
      isNcm_(isNcm)
{
}

#ifdef BLUETOOTH_MODOULE
void NetworkShareTracker::SharingPanObserver::OnConnectionStateChanged(const Bluetooth::BluetoothRemoteDevice &device,
                                                                       int state, int cause)
{
    NETMGR_EXT_LOG_I("Recieve bt-pan state changed event, state[%{public}d].", state);
    Bluetooth::BTConnectState curState = static_cast<Bluetooth::BTConnectState>(state);
    NetworkShareTracker::GetInstance().SetBluetoothState(curState);
    switch (curState) {
        case Bluetooth::BTConnectState::CONNECTING:
            break;
        case Bluetooth::BTConnectState::CONNECTED: {
            NetworkShareTracker::GetInstance().EnableBluetoothSubStateMachine();
            break;
        }
        case Bluetooth::BTConnectState::DISCONNECTING:
            break;
        case Bluetooth::BTConnectState::DISCONNECTED: {
            NetworkShareTracker::GetInstance().StopSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME,
                                                                   SharingIfaceType::SHARING_BLUETOOTH);
            break;
        }
        default:
            break;
    }
}
#endif

#ifdef WIFI_MODOULE
void NetworkShareTracker::WifiHotspotCallback::OnHotspotStateChanged(int state)
{
    NETMGR_EXT_LOG_I("Receive Hotspot state changed event, state[%{public}d]", state);
    Wifi::ApState curState = static_cast<Wifi::ApState>(state);
    NetworkShareTracker::GetInstance().SetWifiState(curState);
    switch (curState) {
        case Wifi::ApState::AP_STATE_STARTING:
            break;
        case Wifi::ApState::AP_STATE_STARTED:
            NetworkShareTracker::GetInstance().HandleHotSpotStarted();
            break;
        case Wifi::ApState::AP_STATE_CLOSING:
            break;
        case Wifi::ApState::AP_STATE_CLOSED:
            NetworkShareTracker::GetInstance().HandleHotSpotClosed();
            break;
        default:
            break;
    }
}

void NetworkShareTracker::WifiHotspotCallback::OnHotspotStaJoin(const Wifi::StationInfo &info)
{
    NetworkShareTracker::GetInstance().HandleHotSpotStaJoin();
}

void NetworkShareTracker::WifiHotspotCallback::OnHotspotStaLeave(const Wifi::StationInfo &info)
{
    NetworkShareTracker::GetInstance().HandleHotSpotStaLeave();
}
#endif

NetworkShareTracker &NetworkShareTracker::GetInstance()
{
    static NetworkShareTracker instance;
    return instance;
}

void NetworkShareTracker::RecoverSharingType()
{
    NETMGR_EXT_LOG_I("NetworkShareTracker::RecoverSharingType in");
    std::set<uint32_t> sharingTypeIsOn;
    int32_t ret = NetsysController::GetInstance().GetNetworkSharingType(sharingTypeIsOn);
    if (ret == NETMANAGER_EXT_SUCCESS) {
        clientRequestsVector_.clear();
        for (auto mem : sharingTypeIsOn) {
            clientRequestsVector_.push_back(static_cast<SharingIfaceType>(mem));
            NETMGR_EXT_LOG_D("clientRequestsVector_.push_back = [%{public}u]", mem);
            EnableNetSharingInternal(static_cast<SharingIfaceType>(mem), false);
            EnableNetSharingInternal(static_cast<SharingIfaceType>(mem), true);
        }
        NETMGR_EXT_LOG_I("now clientRequestsVector_.size() = [%{public}zu], ret = [%{public}d]",
                         clientRequestsVector_.size(), ret);
    }
}

bool NetworkShareTracker::Init()
{
    configuration_ = std::make_shared<NetworkShareConfiguration>();
    networkShareTrackerFfrtQueue_ =  std::make_shared<ffrt::queue>("networkShareTrackerFfrtQueue_");
    if (!networkShareTrackerFfrtQueue_) {
        NETMGR_EXT_LOG_E("ffrt create failed!");
        return false;
    }

    std::shared_ptr<NetworkShareUpstreamMonitor> upstreamNetworkMonitor =
        DelayedSingleton<NetworkShareUpstreamMonitor>::GetInstance();
    upstreamNetworkMonitor->SetOptionData(EVENT_UPSTREAM_CALLBACK);
    upstreamNetworkMonitor->RegisterUpstreamChangedCallback(std::make_shared<MainSmUpstreamCallback>());
    mainStateMachine_ = std::make_shared<NetworkShareMainStateMachine>(upstreamNetworkMonitor);

    netsysCallback_ = sptr<NetsysCallback>::MakeSptr();
    NetsysController::GetInstance().RegisterCallback(netsysCallback_);

#ifdef WIFI_MODOULE
    RegisterWifiApCallback();
#endif
    RegisterBtPanCallback();

    isNetworkSharing_ = false;
    isInit = true;
    NETMGR_EXT_LOG_I("Tracker Init sucessful.");

    RecoverSharingType();

    return true;
}

void NetworkShareTracker::OnChangeSharingState(const SharingIfaceType &type, bool state)
{
    auto fit = find(clientRequestsVector_.begin(), clientRequestsVector_.end(), type);
    if (state && fit == clientRequestsVector_.end()) {
        clientRequestsVector_.push_back(type);
    }
    if (!state && fit != clientRequestsVector_.end()) {
        clientRequestsVector_.erase(fit);
    }
    SendGlobalSharingStateChange();
    NETMGR_EXT_LOG_I("Hotspot OnChangeSharing, clientRequestsVector_ [%{public}zu]", clientRequestsVector_.size());
}

#ifdef WIFI_MODOULE
void NetworkShareTracker::RegisterWifiApCallback()
{
    NETMGR_EXT_LOG_I("RegisterWifiApCallback");
    if (wifiHotspotCallback_ == nullptr) {
        wifiHotspotCallback_ = sptr<WifiHotspotCallback>::MakeSptr();
    }
    auto WifiHostInstance = Wifi::WifiHotspot::GetInstance(WIFI_HOTSPOT_ABILITY_ID);
    if (WifiHostInstance == nullptr) {
        NETMGR_EXT_LOG_E("WifiHostInstance is nullptr");
        return;
    }
    static std::vector<std::string> event = {
        EVENT_HOTSPOT_STATE_CHANGE,
        EVENT_HOTSPOT_STA_JOIN,
        EVENT_HOTSPOT_STA_LEAVE,
    };
    int ret = WifiHostInstance->RegisterCallBack(wifiHotspotCallback_, event);
    if (ret != Wifi::WIFI_OPT_SUCCESS) {
        NETMGR_EXT_LOG_E("NetworkShareTracker::RegisterWifiApCallback failed:%{public}d", ret);
    }
}
#endif

void NetworkShareTracker::RegisterBtPanCallback()
{
#ifdef BLUETOOTH_MODOULE
    Bluetooth::Pan *profile = Bluetooth::Pan::GetProfile();
    if (profile == nullptr) {
        return;
    }
    panObserver_ = std::make_shared<SharingPanObserver>();
    if (panObserver_ != nullptr) {
        profile->RegisterObserver(panObserver_);
    }
#endif
}

void NetworkShareTracker::Uninit()
{
    isInit = false;
#ifdef BLUETOOTH_MODOULE
    Bluetooth::Pan *profile = Bluetooth::Pan::GetProfile();
    if (profile == nullptr || panObserver_ == nullptr) {
        NETMGR_EXT_LOG_E("bt-pan profile or observer is null.");
        return;
    }
    profile->DeregisterObserver(panObserver_);
#endif
    NETMGR_EXT_LOG_I("Uninit successful.");
}

std::shared_ptr<NetworkShareMainStateMachine> &NetworkShareTracker::GetMainStateMachine()
{
    return mainStateMachine_;
}

#ifdef WIFI_MODOULE
void NetworkShareTracker::SetWifiState(const Wifi::ApState &state)
{
    curWifiState_ = state;
}
#endif

#ifdef BLUETOOTH_MODOULE
void NetworkShareTracker::SetBluetoothState(const Bluetooth::BTConnectState &state)
{
    curBluetoothState_ = state;
}
#endif

void NetworkShareTracker::HandleSubSmUpdateInterfaceState(const std::shared_ptr<NetworkShareSubStateMachine> &who,
                                                          int32_t state, int32_t lastError)
{
    if (who == nullptr) {
        NETMGR_EXT_LOG_E("subsm is null.");
        return;
    }
    std::shared_ptr<NetSharingSubSmState> shareState = nullptr;
    std::string ifaceName;
    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        auto iter = subStateMachineMap_.find(who->GetInterfaceName());
        if (iter != subStateMachineMap_.end()) {
            ifaceName = iter->first;
            shareState = iter->second;
        }
    }
    if (shareState != nullptr) {
        NETMGR_EXT_LOG_I("iface=%{public}s state is change from[%{public}d] to[%{public}d].", ifaceName.c_str(),
                         shareState->lastState_, state);
        shareState->lastError_ = lastError;
        if (shareState->lastState_ != state) {
            shareState->lastState_ = state;
            SendIfaceSharingStateChange(who->GetNetShareType(), ifaceName, SubSmStateToExportState(state));
        }
    } else {
        NETMGR_EXT_LOG_W("iface=%{public}s is not find", (who->GetInterfaceName()).c_str());
    }

    if (lastError == NETWORKSHARE_ERROR_INTERNAL_ERROR) {
        SendMainSMEvent(who, CMD_CLEAR_ERROR, 0);
    }
    int32_t which = 0;
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
    SendMainSMEvent(who, which, state);
}

void NetworkShareTracker::SendMainSMEvent(const std::shared_ptr<NetworkShareSubStateMachine> &subSM, int32_t event,
                                          int32_t state)
{
    if (mainStateMachine_ == nullptr) {
        NETMGR_EXT_LOG_I("MainSM is null");
        return;
    }
    NETMGR_EXT_LOG_I("Notify to Main SM event[%{public}d].", event);
    MessageIfaceActive message;
    message.value_ = state;
    message.subsm_ = subSM;
    mainStateMachine_->MainSmEventHandle(event, message);
}

int32_t NetworkShareTracker::IsNetworkSharingSupported(int32_t &supported)
{
    if (configuration_ == nullptr) {
        supported = NETWORKSHARE_IS_UNSUPPORTED;
        return NETWORKSHARE_ERROR_IFACE_CFG_ERROR;
    }
    if (configuration_->IsNetworkSharingSupported()) {
        supported = NETWORKSHARE_IS_SUPPORTED;
    } else {
        supported = NETWORKSHARE_IS_UNSUPPORTED;
    }
    NETMGR_EXT_LOG_I("NetworkShareTracker supported is %{public}d", supported);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkShareTracker::IsSharing(int32_t &sharingStatus)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    for (auto &iter : subStateMachineMap_) {
        std::shared_ptr<NetSharingSubSmState> shareState = iter.second;
        if (shareState == nullptr) {
            continue;
        }
        if (shareState->lastState_ == SUB_SM_STATE_SHARED) {
            NETMGR_EXT_LOG_I("NetworkShareTracker is sharing.");
            sharingStatus = NETWORKSHARE_IS_SHARING;
            return NETMANAGER_EXT_SUCCESS;
        }
    }
    NETMGR_EXT_LOG_I("NetworkShareTracker is unsharing.");
    sharingStatus = NETWORKSHARE_IS_UNSHARING;
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkShareTracker::StartNetworkSharing(const SharingIfaceType &type)
{
    NETMGR_EXT_LOG_I("NetworkShare start sharing,clientRequestsVector_.size = %{public}zu.",
                     clientRequestsVector_.size());
    auto fit = find(clientRequestsVector_.begin(), clientRequestsVector_.end(), type);
    if (fit != clientRequestsVector_.end()) {
        NETMGR_EXT_LOG_I("type[%{public}d] is sharing, will close", type);
        int32_t ret = EnableNetSharingInternal(type, false);
        if (ret != NETMANAGER_EXT_SUCCESS) {
            NETMGR_EXT_LOG_E("stop current [%{public}d] sharing error [%{public}ul]", static_cast<int32_t>(type), ret);
            return ret;
        }
    } else {
        clientRequestsVector_.push_back(type);
    }
#ifdef SHARE_TRAFFIC_LIMIT_ENABLE
    NetworkShareTrafficLimit::GetInstance().StartHandleSharingLimitEvent();
#endif
    return EnableNetSharingInternal(type, true);
}

int32_t NetworkShareTracker::StopNetworkSharing(const SharingIfaceType &type)
{
    NETMGR_EXT_LOG_I("NetworkShare stop sharing,clientRequestsVector_.size = %{public}zu.",
                     clientRequestsVector_.size());
    auto fit = find(clientRequestsVector_.begin(), clientRequestsVector_.end(), type);
    if (fit != clientRequestsVector_.end()) {
        clientRequestsVector_.erase(fit);
    }
#ifdef SHARE_TRAFFIC_LIMIT_ENABLE
    NetworkShareTrafficLimit::GetInstance().EndHandleSharingLimitEvent();
#endif
    return EnableNetSharingInternal(type, false);
}

int32_t NetworkShareTracker::GetSharableRegexs(SharingIfaceType type, std::vector<std::string> &ifaceRegexs)
{
    if (configuration_ == nullptr) {
        NETMGR_EXT_LOG_E("configuration_ is null.");
        return NETWORKSHARE_ERROR_IFACE_CFG_ERROR;
    }
    NETMGR_EXT_LOG_I("NetworkSharing GetSharableRegexs type is %{public}d", type);
    switch (type) {
        case SharingIfaceType::SHARING_WIFI: {
            ifaceRegexs = configuration_->GetWifiIfaceRegexs();
            return NETMANAGER_EXT_SUCCESS;
        }
        case SharingIfaceType::SHARING_USB: {
            ifaceRegexs = configuration_->GetUsbIfaceRegexs();
            return NETMANAGER_EXT_SUCCESS;
        }
        case SharingIfaceType::SHARING_BLUETOOTH: {
            ifaceRegexs = configuration_->GetBluetoothIfaceRegexs();
            return NETMANAGER_EXT_SUCCESS;
        }
        default: {
            NETMGR_EXT_LOG_E("type[%{public}d] is unkonwn.", type);
            return NETWORKSHARE_ERROR_UNKNOWN_TYPE;
        }
    }
}

bool NetworkShareTracker::IsInterfaceMatchType(const std::string &iface, const SharingIfaceType &type)
{
    if (configuration_ == nullptr) {
        return false;
    }
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

int32_t NetworkShareTracker::GetSharingState(const SharingIfaceType type, SharingIfaceState &state)
{
    NETMGR_EXT_LOG_I("NetworkSharing GetSharingState type is %{public}d", type);
    if (type != SharingIfaceType::SHARING_WIFI &&
        type != SharingIfaceType::SHARING_USB &&
        type != SharingIfaceType::SHARING_BLUETOOTH) {
        return NETWORKSHARE_ERROR_UNKNOWN_TYPE;
    }
    bool isFindType = false;
    state = SharingIfaceState::SHARING_NIC_CAN_SERVER;
    std::lock_guard<ffrt::mutex> lock(mutex_);
    for (const auto &iter : subStateMachineMap_) {
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
            NETMGR_EXT_LOG_W("lastState_=%{public}d is unknown data.", subsmState->lastState_);
        } else {
            NETMGR_EXT_LOG_W("iface=%{public}s is not match type[%{public}d]", iter.first.c_str(), type);
        }
    }
    if (!isFindType) {
        NETMGR_EXT_LOG_E("type=%{public}d is not find, used default value.", type);
    }
    NETMGR_EXT_LOG_I("type=%{public}d is find, isFindType is %{public}d.", type, isFindType);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkShareTracker::GetNetSharingIfaces(const SharingIfaceState &state, std::vector<std::string> &ifaces)
{
    NETMGR_EXT_LOG_I("NetworkSharing GetNetSharingIfaces type is %{public}d", state);
    if (state != SharingIfaceState::SHARING_NIC_ERROR &&
        state != SharingIfaceState::SHARING_NIC_CAN_SERVER &&
        state != SharingIfaceState::SHARING_NIC_SERVING) {
        return NETWORKSHARE_ERROR_UNKNOWN_TYPE;
    }
    std::lock_guard<ffrt::mutex> lock(mutex_);
    for_each(subStateMachineMap_.begin(), subStateMachineMap_.end(), [&](auto iter) {
        std::shared_ptr<NetSharingSubSmState> subsmState = iter.second;
        if (subsmState == nullptr) {
            NETMGR_EXT_LOG_W("iface=%{public}s subsmState is null.", (iter.first).c_str());
            return;
        }
        NETMGR_EXT_LOG_I("iface=%{public}s, state=%{public}d", (iter.first).c_str(), subsmState->lastState_);
        if ((state == SharingIfaceState::SHARING_NIC_ERROR && subsmState->lastState_ == SUB_SM_STATE_UNAVAILABLE) ||
            (state == SharingIfaceState::SHARING_NIC_CAN_SERVER && subsmState->lastState_ == SUB_SM_STATE_AVAILABLE) ||
            (state == SharingIfaceState::SHARING_NIC_SERVING && subsmState->lastState_ == SUB_SM_STATE_SHARED)) {
            ifaces.push_back(iter.first);
        }
    });
    NETMGR_EXT_LOG_I("GetNetSharingIfaces ifaces.size is %{public}zu", ifaces.size());
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkShareTracker::RegisterSharingEvent(sptr<ISharingEventCallback> callback)
{
    if (callback == nullptr) {
        NETMGR_EXT_LOG_E("callback is null.");
        return NETMANAGER_EXT_ERR_LOCAL_PTR_NULL;
    }
    std::lock_guard<ffrt::mutex> lock(callbackMutex_);
    if (sharingEventCallback_.size() >= MAX_CALLBACK_COUNT) {
        NETMGR_EXT_LOG_E("callback above max count, return error.");
        return NETWORKSHARE_ERROR_ISSHARING_CALLBACK_ERROR;
    }
    for (auto iter = sharingEventCallback_.begin(); iter != sharingEventCallback_.end(); ++iter) {
        if (callback->AsObject().GetRefPtr() == (*iter)->AsObject().GetRefPtr()) {
            NETMGR_EXT_LOG_I("SharingEventCallback find same callback");
            return NETMANAGER_EXT_SUCCESS;
        }
    }
    sharingEventCallback_.push_back(callback);
    NETMGR_EXT_LOG_I("RegisterSharingEvent is successful, sharingEventCallback_.size = %{public}zu",
                     sharingEventCallback_.size());
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkShareTracker::UnregisterSharingEvent(sptr<ISharingEventCallback> callback)
{
    std::lock_guard<ffrt::mutex> lock(callbackMutex_);
    for (auto iter = sharingEventCallback_.begin(); iter != sharingEventCallback_.end(); ++iter) {
        if (callback->AsObject().GetRefPtr() == (*iter)->AsObject().GetRefPtr()) {
            sharingEventCallback_.erase(iter);
            break;
        }
    }
    NETMGR_EXT_LOG_I("UnregisterSharingEvent is successful, sharingEventCallback_.size = %{public}zu",
                     sharingEventCallback_.size());
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkShareTracker::GetSharedSubSMTraffic(const TrafficType &type, int32_t &kbByte)
{
    int64_t bytes = 0;
    NETMGR_EXT_LOG_I("GetSharedSubSMTraffic start, type is %{public}d", type);
    for (auto &subSM : sharedSubSM_) {
        if (subSM == nullptr) {
            continue;
        }
        std::string downIface;
        std::string upIface;
        subSM->GetDownIfaceName(downIface);
        subSM->GetUpIfaceName(upIface);
        nmd::NetworkSharingTraffic traffic;
        NETMGR_EXT_LOG_I("DownIface[%{public}s], upIface[%{public}s].", downIface.c_str(), upIface.c_str());
        int32_t ret = NetsysController::GetInstance().GetNetworkSharingTraffic(downIface, upIface, traffic);
        if (ret != NETMANAGER_SUCCESS) {
            NETMGR_EXT_LOG_E("GetTrafficBytes err, ret[%{public}d].", ret);
            continue;
        }
        switch (type) {
            case TrafficType::TRAFFIC_RX:
                bytes += traffic.receive;
                break;
            case TrafficType::TRAFFIC_TX:
                bytes += traffic.send;
                break;
            case TrafficType::TRAFFIC_ALL:
                bytes += traffic.all;
                break;
            default:
                break;
        }
    }

    kbByte = static_cast<int32_t>(bytes / BYTE_TRANSFORM_KB);
    if (kbByte > std::numeric_limits<int32_t>::max()) {
        NETMGR_EXT_LOG_I("GetBytes [%{public}s] is above max.", std::to_string(kbByte).c_str());
        kbByte = std::numeric_limits<int32_t>::max();
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkShareTracker::EnableNetSharingInternal(const SharingIfaceType &type, bool enable)
{
    NETMGR_EXT_LOG_I("NetSharing type[%{public}d] enable[%{public}d].", type, enable);
    int32_t result = NETMANAGER_EXT_SUCCESS;
    switch (type) {
        case SharingIfaceType::SHARING_WIFI:
            result = SetWifiNetworkSharing(enable);
            break;
        case SharingIfaceType::SHARING_USB:
            result = SetUsbNetworkSharing(enable);
            break;
        case SharingIfaceType::SHARING_BLUETOOTH:
            result = SetBluetoothNetworkSharing(enable);
            break;
        default:
            NETMGR_EXT_LOG_E("Invalid networkshare type.");
            result = NETWORKSHARE_ERROR_UNKNOWN_TYPE;
            break;
    }
    NETMGR_EXT_LOG_I("NetSharing EnableNetSharingInternal result is %{public}d.", result);
    if (result != NETMANAGER_EXT_SUCCESS) {
        auto it = find(clientRequestsVector_.begin(), clientRequestsVector_.end(), type);
        if (it != clientRequestsVector_.end()) {
            clientRequestsVector_.erase(it);
        }
    }

    return result;
}

int32_t NetworkShareTracker::SetWifiNetworkSharing(bool enable)
{
    int32_t result = NETMANAGER_EXT_SUCCESS;
#ifdef WIFI_MODOULE
    if (enable) {
        int32_t ret = EnableHotspot();
        if (ret != Wifi::WIFI_OPT_SUCCESS) {
            NETMGR_EXT_LOG_E("EnableHotspot error[%{public}d].", ret);
            result = NETWORKSHARE_ERROR_WIFI_SHARING;
            NetworkShareHisysEvent::GetInstance().SendFaultEvent(
                SharingIfaceType::SHARING_WIFI, NetworkShareEventOperator::OPERATION_ENABLE_IFACE,
                NetworkShareEventErrorType::ERROR_ENABLE_IFACE, ERROR_MSG_ENABLE_WIFI,
                NetworkShareEventType::SETUP_EVENT);
        } else {
            NETMGR_EXT_LOG_I("EnableHotspot successfull.");
            if (wifiShareCount_ < INT32_MAX) {
                wifiShareCount_++;
            }
            NetworkShareHisysEvent::GetInstance().SendBehaviorEvent(wifiShareCount_, SharingIfaceType::SHARING_WIFI);
            NETMGR_EXT_LOG_E("wifiShareCount_[%{public}d].", wifiShareCount_);
        }
    } else {
        int32_t ret = DisableHotspot();
        if (ret != Wifi::WIFI_OPT_SUCCESS) {
            NetworkShareHisysEvent::GetInstance().SendFaultEvent(
                SharingIfaceType::SHARING_WIFI, NetworkShareEventOperator::OPERATION_DISABLE_IFACE,
                NetworkShareEventErrorType::ERROR_DISABLE_IFACE, ERROR_MSG_DISABLE_WIFI,
                NetworkShareEventType::CANCEL_EVENT);
            NETMGR_EXT_LOG_E("DisableHotspot error[%{public}d].", ret);
            result = NETWORKSHARE_ERROR_WIFI_SHARING;
        } else {
            NETMGR_EXT_LOG_I("DisableHotspot successful.");
        }
    }
#endif
    return result;
}

int32_t NetworkShareTracker::SetUsbNetworkSharing(bool enable)
{
#ifdef USB_MODOULE
    auto &usbSrvClient = USB::UsbSrvClient::GetInstance();
    if (enable) {
        curUsbState_ = UsbShareState::USB_SHARING;
        int32_t ret = usbSrvClient.GetCurrentFunctions(previousFuncs_);
        if (ret != USB::UEC_OK) {
            NETMGR_EXT_LOG_E("GetCurrentFunctions error[%{public}d].", ret);
            return NETWORKSHARE_ERROR_USB_SHARING;
        }
        ret = usbSrvClient.SetCurrentFunctions(USB::UsbSrvSupport::FUNCTION_RNDIS);
        if (ret != USB::UEC_OK) {
            NETMGR_EXT_LOG_E("SetCurrentFunctions error[%{public}d].", ret);
            return NETWORKSHARE_ERROR_USB_SHARING;
        }
        if (usbShareCount_ < INT32_MAX) {
            usbShareCount_++;
        }
        NetworkShareHisysEvent::GetInstance().SendBehaviorEvent(usbShareCount_, SharingIfaceType::SHARING_USB);
    } else {
        if (curUsbState_ != UsbShareState::USB_SHARING) {
            return NETMANAGER_EXT_SUCCESS;
        }
        curUsbState_ = UsbShareState::USB_CLOSING;
        int32_t ret = usbSrvClient.SetCurrentFunctions(previousFuncs_);
        if (ret != USB::UEC_OK) {
            NETMGR_EXT_LOG_E("usb SetCurrentFunctions error[%{public}d].", ret);
            return NETWORKSHARE_ERROR_USB_SHARING;
        }
    }
#endif
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkShareTracker::SetBluetoothNetworkSharing(bool enable)
{
#ifdef BLUETOOTH_MODOULE
    Bluetooth::Pan *profile = Bluetooth::Pan::GetProfile();
    if (profile == nullptr) {
        NETMGR_EXT_LOG_E("SetBluetoothNetworkSharing(%{public}s) profile is null].", enable ? "true" : "false");
        return NETWORKSHARE_ERROR_BT_SHARING;
    }
    if (enable && panObserver_ == nullptr) {
        NETMGR_EXT_LOG_I("register bluetooth pan callback.");
        RegisterBtPanCallback();
    }
    bool ret = profile->SetTethering(enable);
    if (ret) {
        NETMGR_EXT_LOG_I("SetBluetoothNetworkSharing(%{public}s) is success.", enable ? "true" : "false");
        if (enable && bluetoothShareCount_ < INT32_MAX) {
            bluetoothShareCount_++;
        }
        NetworkShareHisysEvent::GetInstance().SendBehaviorEvent(bluetoothShareCount_,
                                                                SharingIfaceType::SHARING_BLUETOOTH);
        return NETMANAGER_EXT_SUCCESS;
    }
    if (enable) {
        NetworkShareHisysEvent::GetInstance().SendFaultEvent(
            SharingIfaceType::SHARING_BLUETOOTH, NetworkShareEventOperator::OPERATION_ENABLE_IFACE,
            NetworkShareEventErrorType::ERROR_ENABLE_IFACE, ERROR_MSG_ENABLE_BTPAN, NetworkShareEventType::SETUP_EVENT);
    } else {
        NetworkShareHisysEvent::GetInstance().SendFaultEvent(
            SharingIfaceType::SHARING_BLUETOOTH, NetworkShareEventOperator::OPERATION_DISABLE_IFACE,
            NetworkShareEventErrorType::ERROR_DISABLE_IFACE, ERROR_MSG_DISABLE_BTPAN,
            NetworkShareEventType::CANCEL_EVENT);
    }

    NETMGR_EXT_LOG_E("SetBluetoothNetworkSharing(%{public}s) is error.", enable ? "true" : "false");
#endif
    return NETWORKSHARE_ERROR_BT_SHARING;
}

int32_t NetworkShareTracker::Sharing(const std::string &iface, int32_t reqState)
{
    std::shared_ptr<NetSharingSubSmState> subSMState = nullptr;
    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        std::map<std::string, std::shared_ptr<NetSharingSubSmState>>::iterator iter = subStateMachineMap_.find(iface);
        if (iter == subStateMachineMap_.end()) {
            NETMGR_EXT_LOG_E("Try to share an unknown iface:%{public}s, ignore.", iface.c_str());
            return NETWORKSHARE_ERROR_UNKNOWN_IFACE;
        }
        subSMState = iter->second;
    }
    if (subSMState == nullptr) {
        NETMGR_EXT_LOG_E("NetSharingSubSmState is null.");
        return NETMANAGER_EXT_ERR_LOCAL_PTR_NULL;
    }
    if (subSMState->lastState_ != SUB_SM_STATE_AVAILABLE) {
        NETMGR_EXT_LOG_E("Try to share an unavailable iface:%{public}s, ignore.", iface.c_str());
        return NETWORKSHARE_ERROR_UNAVAIL_IFACE;
    }

    if (subSMState->subStateMachine_ != nullptr) {
        NETMGR_EXT_LOG_I("NOTIFY TO SUB SM [%{public}s] CMD_NETSHARE_REQUESTED.",
                         subSMState->subStateMachine_->GetInterfaceName().c_str());
        subSMState->subStateMachine_->SubSmEventHandle(CMD_NETSHARE_REQUESTED, reqState);
        return NETMANAGER_EXT_SUCCESS;
    }

    NETMGR_EXT_LOG_E("subStateMachine_ is null.");
    return NETWORKSHARE_ERROR_SHARING_IFACE_ERROR;
}

bool NetworkShareTracker::FindSubStateMachine(const std::string &iface, const SharingIfaceType &interfaceType,
                                              std::shared_ptr<NetworkShareSubStateMachine> &subSM, std::string &findKey)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    std::map<std::string, std::shared_ptr<NetSharingSubSmState>>::iterator iter = subStateMachineMap_.find(iface);
    if (iter != subStateMachineMap_.end()) {
        if (iter->second == nullptr) {
            NETMGR_EXT_LOG_E("NetSharingSubSmState is null.");
            return false;
        }
        if (iter->second->subStateMachine_ == nullptr) {
            NETMGR_EXT_LOG_E("NetSharingSubSm is null.");
            return false;
        }
        subSM = iter->second->subStateMachine_;
        findKey = iter->first;
        NETMGR_EXT_LOG_I("find subSM by iface[%{public}s].", iface.c_str());
        return true;
    }

    for (const auto &it : subStateMachineMap_) {
        if (it.second == nullptr) {
            NETMGR_EXT_LOG_W("NetSharingSubSmState is null.");
            continue;
        }
        if (it.second->subStateMachine_ == nullptr) {
            NETMGR_EXT_LOG_E("NetSharingSubSm is null.");
            continue;
        }
        if (it.second->subStateMachine_->GetNetShareType() == interfaceType) {
            subSM = it.second->subStateMachine_;
            findKey = it.first;
            NETMGR_EXT_LOG_I("find subsm by type[%{public}d].", interfaceType);
            return true;
        }
    }
    return false;
}

void NetworkShareTracker::EnableWifiSubStateMachine()
{
    int32_t ret = CreateSubStateMachine(mApIfaceName_, SharingIfaceType::SHARING_WIFI, false);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("create wifi sub SM failed, error[%{public}d].", ret);
        return;
    }

    ret = Sharing(mApIfaceName_, SUB_SM_STATE_SHARED);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("start wifi sharing failed, error[%{public}d].", ret);
    }
}

void NetworkShareTracker::EnableBluetoothSubStateMachine()
{
    int32_t ret = CreateSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_BLUETOOTH, false);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("create bluetooth sub SM failed, error[%{public}d].", ret);
        return;
    }
    ret = Sharing(BLUETOOTH_DEFAULT_IFACE_NAME, SUB_SM_STATE_SHARED);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("create sub SM failed, error[%{public}d].", ret);
    }
}

bool NetworkShareTracker::UpstreamWanted()
{
    return sharedSubSM_.size() != 0;
}

void NetworkShareTracker::ModifySharedSubStateMachineList(bool isAdd,
                                                          const std::shared_ptr<NetworkShareSubStateMachine> &subSm)
{
    if (isAdd) {
        std::vector<std::shared_ptr<NetworkShareSubStateMachine>>::iterator iter =
            find(sharedSubSM_.begin(), sharedSubSM_.end(), subSm);
        if (iter == sharedSubSM_.end()) {
            NETMGR_EXT_LOG_I("add new shared subSm.");
            sharedSubSM_.push_back(subSm);
        }
    } else {
        std::vector<std::shared_ptr<NetworkShareSubStateMachine>>::iterator iter =
            find(sharedSubSM_.begin(), sharedSubSM_.end(), subSm);
        if (iter != sharedSubSM_.end()) {
            NETMGR_EXT_LOG_I("remove the shared subSm.");
            sharedSubSM_.erase(iter);
        }
    }
}

void NetworkShareTracker::SetUpstreamNetHandle(const std::shared_ptr<UpstreamNetworkInfo> &netinfo)
{
    if (netinfo != nullptr && netinfo->netHandle_ != nullptr) {
        SetDnsForwarders(*(netinfo->netHandle_));
    } else {
        StopDnsProxy();
    }
    NotifyDownstreamsHasNewUpstreamIface(netinfo);
}

void NetworkShareTracker::SetDnsForwarders(const NetHandle &netHandle)
{
    if (mainStateMachine_ == nullptr) {
        NETMGR_EXT_LOG_I("MainSM is null");
        return;
    }
    int32_t ret = NETMANAGER_SUCCESS;
    if (!isStartDnsProxy_) {
        ret = NetsysController::GetInstance().StartDnsProxyListen();
        if (ret != NETSYS_SUCCESS) {
            NETMGR_EXT_LOG_E("StartDnsProxy error, result[%{public}d].", ret);
            mainStateMachine_->SwitcheToErrorState(CMD_SET_DNS_FORWARDERS_ERROR);
            return;
        }
        NETMGR_EXT_LOG_I("StartDnsProxy successful.");
        isStartDnsProxy_ = true;
    }
    int32_t netId = netHandle.GetNetId();
    if (netId < 0) {
        NETMGR_EXT_LOG_E("netId less than 0.");
        mainStateMachine_->SwitcheToErrorState(CMD_SET_DNS_FORWARDERS_ERROR);
        return;
    }
    ret = NetsysController::GetInstance().ShareDnsSet(netId);
    if (ret != NETSYS_SUCCESS) {
        NETMGR_EXT_LOG_E("SetDns error, result[%{public}d].", ret);
        mainStateMachine_->SwitcheToErrorState(CMD_SET_DNS_FORWARDERS_ERROR);
        return;
    }

    netId_ = netId;
    NETMGR_EXT_LOG_I("SetDns netId[%{public}d] success.", netId);
}

void NetworkShareTracker::StopDnsProxy()
{
    if (isStartDnsProxy_) {
        int32_t ret = NetsysController::GetInstance().StopDnsProxyListen();
        if (ret != NETSYS_SUCCESS) {
            NETMGR_EXT_LOG_E("StopDnsProxy error, result[%{public}d].", ret);
        } else {
            NETMGR_EXT_LOG_I("StopDnsProxy success.");
            isStartDnsProxy_ = false;
        }
    }
}

void NetworkShareTracker::NotifyDownstreamsHasNewUpstreamIface(const std::shared_ptr<UpstreamNetworkInfo> &netinfo)
{
    upstreamInfo_ = netinfo;
    for_each(sharedSubSM_.begin(), sharedSubSM_.end(), [netinfo](std::shared_ptr<NetworkShareSubStateMachine> subsm) {
        if (subsm != nullptr) {
            NETMGR_EXT_LOG_I("NOTIFY TO SUB SM [%{public}s] CMD_NETSHARE_CONNECTION_CHANGED.",
                             subsm->GetInterfaceName().c_str());
            subsm->SubSmEventHandle(CMD_NETSHARE_CONNECTION_CHANGED, netinfo);
        }
    });
}

void NetworkShareTracker::GetUpstreamInfo(std::shared_ptr<UpstreamNetworkInfo> &upstreamInfo)
{
    upstreamInfo = upstreamInfo_;
}

int32_t NetworkShareTracker::CreateSubStateMachine(const std::string &iface, const SharingIfaceType &interfaceType,
                                                   bool isNcm)
{
    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        if (subStateMachineMap_.count(iface) != 0) {
            NETMGR_EXT_LOG_W("iface[%{public}s] has added, ignoring", iface.c_str());
            return NETMANAGER_EXT_SUCCESS;
        }
    }

    std::shared_ptr<NetworkShareSubStateMachine> subSm =
        std::make_shared<NetworkShareSubStateMachine>(iface, interfaceType, configuration_);
    std::shared_ptr<SubSmUpstreamCallback> smcallback = std::make_shared<SubSmUpstreamCallback>();
    subSm->RegisterSubSMCallback(smcallback);

    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        std::shared_ptr<NetSharingSubSmState> netShareState = std::make_shared<NetSharingSubSmState>(subSm, isNcm);
        subStateMachineMap_.insert(std::make_pair(iface, netShareState));
    }
    NETMGR_EXT_LOG_I("adding subSM[%{public}s], type[%{public}d], current subSM count[%{public}s]", iface.c_str(),
                     static_cast<SharingIfaceType>(interfaceType), std::to_string(subStateMachineMap_.size()).c_str());
    return NETMANAGER_EXT_SUCCESS;
}

__attribute__((no_sanitize("cfi"))) void NetworkShareTracker::StopSubStateMachine(
    const std::string &iface, const SharingIfaceType &interfaceType)
{
    std::shared_ptr<NetworkShareSubStateMachine> subSM = nullptr;
    std::string findKey;
    if (!FindSubStateMachine(iface, interfaceType, subSM, findKey) || subSM == nullptr) {
        NETMGR_EXT_LOG_W("not find the subSM.");
        return;
    }
    NETMGR_EXT_LOG_I("NOTIFY TO SUB SM [%{public}s] CMD_NETSHARE_UNREQUESTED.", subSM->GetInterfaceName().c_str());
    subSM->SubSmEventHandle(CMD_NETSHARE_UNREQUESTED, 0);

    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        if (subStateMachineMap_.count(findKey) > 0) {
            subStateMachineMap_.erase(findKey);
            NETMGR_EXT_LOG_I("removed iface[%{public}s] subSM, current subSM count[%{public}s].", iface.c_str(),
                             std::to_string(subStateMachineMap_.size()).c_str());
        }
    }
}

bool NetworkShareTracker::InterfaceNameToType(const std::string &iface, SharingIfaceType &type)
{
    if (configuration_ == nullptr) {
        NETMGR_EXT_LOG_E("configuration is null.");
        return false;
    }
    if (configuration_->IsWifiIface(iface)) {
        type = SharingIfaceType::SHARING_WIFI;
        return true;
    }
    if (configuration_->IsUsbIface(iface)) {
        type = SharingIfaceType::SHARING_USB;
        return true;
    }
    if (configuration_->IsBluetoothIface(iface)) {
        type = SharingIfaceType::SHARING_BLUETOOTH;
        return true;
    }
    return false;
}

bool NetworkShareTracker::IsHandleNetlinkEvent(const SharingIfaceType &type, bool up)
{
#ifdef WIFI_MODOULE
    if (type == SharingIfaceType::SHARING_WIFI) {
        return up ? curWifiState_ == Wifi::ApState::AP_STATE_STARTING
                  : curWifiState_ == Wifi::ApState::AP_STATE_CLOSING;
    }
#endif
#ifdef BLUETOOTH_MODOULE
    if (type == SharingIfaceType::SHARING_BLUETOOTH) {
        return up ? curBluetoothState_ == Bluetooth::BTConnectState::CONNECTING
                  : curBluetoothState_ == Bluetooth::BTConnectState::DISCONNECTING;
    }
#endif
#ifdef USB_MODOULE
    if (type == SharingIfaceType::SHARING_USB) {
        return up ? curUsbState_ == UsbShareState::USB_SHARING
                  : curUsbState_ == UsbShareState::USB_CLOSING;
    }
#endif
    return false;
}

void NetworkShareTracker::InterfaceStatusChanged(const std::string &iface, bool up)
{
    if (!isInit) {
        NETMGR_EXT_LOG_E("eventHandler is null.");
        return;
    }
    SharingIfaceType type;
    if (!InterfaceNameToType(iface, type) || !IsHandleNetlinkEvent(type, up)) {
        NETMGR_EXT_LOG_E("iface[%{public}s] is not downsteam or not correct event.", iface.c_str());
        return;
    }
    NETMGR_EXT_LOG_I("interface[%{public}s] for [%{public}s]", iface.c_str(), up ? "up" : "down");
    if (up) {
        if (configuration_ == nullptr) {
            NETMGR_EXT_LOG_E("configuration_ is null");
            return;
        }
        std::string taskName = "InterfaceAdded_task";
        if (configuration_->IsUsbIface(iface)) {
            std::function<void()> sharingUsbFunc =
                [this, iface]() { Sharing(iface, SUB_SM_STATE_SHARED); };
            networkShareTrackerFfrtQueue_->submit(sharingUsbFunc, ffrt::task_attr().name(taskName.c_str()));
        } else {
            std::function<void()> createSubStateMachineFunc =
                [this, iface, type]() { CreateSubStateMachine(iface, type, false); };
            networkShareTrackerFfrtQueue_->submit(createSubStateMachineFunc, ffrt::task_attr().name(taskName.c_str()));
        }
    } else {
        std::string taskName = "InterfaceRemoved_task";
        std::function<void()> stopSubStateMachineFunc =
            [this, iface, type]() { StopSubStateMachine(iface, type); };
        networkShareTrackerFfrtQueue_->submit(stopSubStateMachineFunc, ffrt::task_attr().name(taskName.c_str()));
    }
}

bool NetworkShareTracker::CheckIfUpUsbIface(const std::string &iface)
{
    if (!configuration_->IsUsbIface(iface)) {
        NETMGR_EXT_LOG_I("Iface is not usb, no need to up.");
        return true;
    }
    if (NetsysController::GetInstance().InterfaceSetIpAddress(iface, configuration_->GetUsbRndisIpv4Addr()) != 0) {
        NETMGR_EXT_LOG_E("Failed setting usb ip address");
        return false;
    }
    if (NetsysController::GetInstance().SetInterfaceUp(iface) != 0) {
        NETMGR_EXT_LOG_E("Failed setting usb iface up");
        return false;
    }
    return true;
}

void NetworkShareTracker::InterfaceAdded(const std::string &iface)
{
    if (!CheckValidShareInterface(iface)) {
        NETMGR_EXT_LOG_I("invalid share interface");
        return;
    }
    if (configuration_ == nullptr) {
        NETMGR_EXT_LOG_E("configuration_ is null");
        return;
    }
    if (!isInit) {
        NETMGR_EXT_LOG_E("eventHandler is null.");
        return;
    }
    SharingIfaceType type;
    if (!InterfaceNameToType(iface, type) || !IsHandleNetlinkEvent(type, true)) {
        NETMGR_EXT_LOG_E("iface[%{public}s] is not downsteam or not correct event.", iface.c_str());
        return;
    }
    NETMGR_EXT_LOG_I("iface[%{public}s], type[%{public}d].", iface.c_str(), static_cast<int32_t>(type));
    if (!CheckIfUpUsbIface(iface)) {
        return;
    }
    std::string taskName = "InterfaceAdded_task";
    std::function<void()> createSubStateMachineFunc =
        [this, iface, type]() { CreateSubStateMachine(iface, type, false); };
    networkShareTrackerFfrtQueue_->submit(createSubStateMachineFunc, ffrt::task_attr().name(taskName.c_str()));
}

void NetworkShareTracker::InterfaceRemoved(const std::string &iface)
{
    if (!isInit) {
        NETMGR_EXT_LOG_E("eventHandler is null.");
        return;
    }
    SharingIfaceType type;
    if (!InterfaceNameToType(iface, type) || !IsHandleNetlinkEvent(type, false)) {
        NETMGR_EXT_LOG_E("iface[%{public}s] is not downsteam or not correct event.", iface.c_str());
        return;
    }
    NETMGR_EXT_LOG_I("iface[%{public}s], type[%{public}d].", iface.c_str(), static_cast<int32_t>(type));
    std::string taskName = "InterfaceRemoved_task";
    std::function<void()> stopSubStateMachineFunc =
        [this, iface, type]() { StopSubStateMachine(iface, type); };
    networkShareTrackerFfrtQueue_->submit(stopSubStateMachineFunc, ffrt::task_attr().name(taskName.c_str()));
}

void NetworkShareTracker::SendGlobalSharingStateChange()
{
    uint32_t callbackSize = 0;
    {
        std::lock_guard<ffrt::mutex> lock(callbackMutex_);
        callbackSize = sharingEventCallback_.size();
    }
    if (callbackSize == 0) {
        NETMGR_EXT_LOG_E("sharingEventCallback is empty.");
        return;
    }
    bool isSharing = false;
    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        for (auto &iter : subStateMachineMap_) {
            std::shared_ptr<NetSharingSubSmState> subsmState = iter.second;
            if (subsmState == nullptr) {
                NETMGR_EXT_LOG_W("iface[%{public}s] subsmState is null.", iter.first.c_str());
                continue;
            }
            if (subsmState->lastState_ == SUB_SM_STATE_SHARED) {
                isSharing = true;
                break;
            }
        }
    }
    NETMGR_EXT_LOG_I("send global sharing state change, isNetworkSharing_[%{public}d] isSharing[%{public}d].",
                     isNetworkSharing_, isSharing);
    if (isNetworkSharing_ != isSharing) {
        isNetworkSharing_ = isSharing;
        std::lock_guard<ffrt::mutex> lock(callbackMutex_);
        for_each(sharingEventCallback_.begin(), sharingEventCallback_.end(),
                 [isSharing](sptr<ISharingEventCallback> &callback) {
                     if (callback != nullptr) {
                         callback->OnSharingStateChanged(isSharing);
                     }
                 });
    }
}

void NetworkShareTracker::SendIfaceSharingStateChange(const SharingIfaceType &type, const std::string &iface,
                                                      const SharingIfaceState &state)
{
    std::lock_guard<ffrt::mutex> lock(callbackMutex_);
    if (sharingEventCallback_.size() == 0) {
        NETMGR_EXT_LOG_E("sharingEventCallback is empty.");
        return;
    }
    NETMGR_EXT_LOG_I("send iface sharing state change, type[%{public}d] iface[%{public}s]", type, iface.c_str());
    for_each(sharingEventCallback_.begin(), sharingEventCallback_.end(),
             [type, iface, state](sptr<ISharingEventCallback> &callback) {
                 if (callback != nullptr) {
                     callback->OnInterfaceSharingStateChanged(type, iface, state);
                 }
             });
}

void NetworkShareTracker::SendSharingUpstreamChange(const sptr<NetHandle> &netHandle)
{
    std::lock_guard<ffrt::mutex> lock(callbackMutex_);
    if (sharingEventCallback_.size() == 0 || netHandle == nullptr) {
        NETMGR_EXT_LOG_E("sharingEventCallback is empty.");
        return;
    }
    NETMGR_EXT_LOG_I("send sharing upstream change, netId[%{public}d]", netHandle->GetNetId());
    for_each(sharingEventCallback_.begin(), sharingEventCallback_.end(),
             [netHandle](sptr<ISharingEventCallback> &callback) {
                 if (callback != nullptr) {
                     callback->OnSharingUpstreamChanged(netHandle);
                 }
             });
}

SharingIfaceState NetworkShareTracker::SubSmStateToExportState(int32_t state)
{
    SharingIfaceState newState = SharingIfaceState::SHARING_NIC_CAN_SERVER;
    if (state == SUB_SM_STATE_AVAILABLE) {
        newState = SharingIfaceState::SHARING_NIC_CAN_SERVER;
    } else if (state == SUB_SM_STATE_SHARED) {
        newState = SharingIfaceState::SHARING_NIC_SERVING;
    } else if (state == SUB_SM_STATE_UNAVAILABLE) {
        newState = SharingIfaceState::SHARING_NIC_ERROR;
    } else {
        NETMGR_EXT_LOG_E("SubSmStateToExportState state[%{public}d] is unknown type.", state);
        newState = SharingIfaceState::SHARING_NIC_ERROR;
    }
    return newState;
}

void NetworkShareTracker::RestartResume()
{
    if (clientRequestsVector_.empty()) {
        NETMGR_EXT_LOG_E("RestartResume, no StartDnsProxy.");
        return;
    }

    int32_t ret = NETMANAGER_SUCCESS;

    if (isStartDnsProxy_) {
        StopDnsProxy();

        ret = NetsysController::GetInstance().StartDnsProxyListen();
        if (ret != NETSYS_SUCCESS) {
            NETMGR_EXT_LOG_E("StartDnsProxy error, result[%{public}d].", ret);
            mainStateMachine_->SwitcheToErrorState(CMD_SET_DNS_FORWARDERS_ERROR);
            return;
        }
        isStartDnsProxy_ = true;
        NETMGR_EXT_LOG_I("StartDnsProxy successful.");
    }

    ret = NetsysController::GetInstance().ShareDnsSet(netId_);
    if (ret != NETSYS_SUCCESS) {
        NETMGR_EXT_LOG_E("SetDns error, result[%{public}d].", ret);
        mainStateMachine_->SwitcheToErrorState(CMD_SET_DNS_FORWARDERS_ERROR);
        return;
    }

    NETMGR_EXT_LOG_I("SetDns netId[%{public}d] success.", netId_);

    for (auto &subsm : sharedSubSM_) {
        if (subsm != nullptr) {
            NETMGR_EXT_LOG_I("NOTIFY TO SUB SM [%{public}s] CMD_NETSHARE_CONNECTION_CHANGED.",
                subsm->GetInterfaceName().c_str());
            subsm->HandleConnection();
        }
    }
}

bool NetworkShareTracker::CheckValidShareInterface(const std::string &iface)
{
    bool ret = false;
    uint32_t ifacesize = sizeof(SHARE_VALID_INTERFACES) / sizeof(SHARE_VALID_INTERFACES[0]);

    for (uint32_t i = 0; i < ifacesize; ++i) {
        ret = IsInterfaceMatchType(iface, SHARE_VALID_INTERFACES[i]);
        if (ret) {
            break;
        }
    }
    return ret;
}

void NetworkShareTracker::OnPowerConnected()
{
#ifdef WIFI_MODOULE
    std::lock_guard<ffrt::mutex> lock(apStopTimerMutex_);
    NETMGR_EXT_LOG_I("Power connected and stop idle ap timer");
    powerConnected_ = true;
    NetworkShareTracker::GetInstance().HandleIdleApStopTimer();
#endif
}

void NetworkShareTracker::OnPowerDisConnected()
{
#ifdef WIFI_MODOULE
    std::lock_guard<ffrt::mutex> lock(apStopTimerMutex_);
    NETMGR_EXT_LOG_I("Power disConnected");
    powerConnected_ = false;

    size_t size = 0;
    int32_t ret = GetStationNum(size);
    if (ret != Wifi::WIFI_OPT_SUCCESS) {
        NETMGR_EXT_LOG_E("Get softap station num, ret:%{public}d", ret);
    }
    NETMGR_EXT_LOG_I("Current connected station mum is:%{public}zu", size);
    staConnected_ = size != 0;
    NetworkShareTracker::GetInstance().HandleIdleApStopTimer();
#endif
}

#ifdef WIFI_MODOULE
void NetworkShareTracker::HandleHotSpotStarted()
{
    if (NetworkShareTracker::GetInstance().mApIfaceName_.empty()) {
        std::string ifaceName = WIFI_AP_DEFAULT_IFACE_NAME;
        auto WifiHostInstance = Wifi::WifiHotspot::GetInstance(WIFI_HOTSPOT_ABILITY_ID);
        int32_t ret = -1;
        if (WifiHostInstance != nullptr) {
            ret = WifiHostInstance->GetApIfaceName(ifaceName);
        }
        if (ret != Wifi::WIFI_OPT_SUCCESS) {
            NETMGR_EXT_LOG_E("get AP ifcace name failed! use default value");
        }
        NetworkShareTracker::GetInstance().mApIfaceName_ = ifaceName;
    }
    NetworkShareTracker::GetInstance().OnChangeSharingState(SharingIfaceType::SHARING_WIFI, true);
    NetworkShareTracker::GetInstance().EnableWifiSubStateMachine();
#ifdef SHARE_NOTIFICATION_ENABLE
    NetworkShareNotification::GetInstance().CancelNetworkShareNotification(
        NotificationId::HOTSPOT_IDLE_NOTIFICATION_ID);
#endif
    {
        std::lock_guard<ffrt::mutex> lock(apStopTimerMutex_);
        staConnected_ = false;
        NetworkShareTracker::GetInstance().GetPowerConnected();
        NetworkShareTracker::GetInstance().HandleIdleApStopTimer();
    }
}

void NetworkShareTracker::HandleHotSpotClosed()
{
    {
        std::lock_guard<ffrt::mutex> lock(apStopTimerMutex_);
        staConnected_ = false;
        NetworkShareTracker::GetInstance().HandleIdleApStopTimer();
    }
    NetworkShareTracker::GetInstance().OnChangeSharingState(SharingIfaceType::SHARING_WIFI, false);
    NetworkShareTracker::GetInstance().StopSubStateMachine(NetworkShareTracker::GetInstance().mApIfaceName_,
        SharingIfaceType::SHARING_WIFI);
    NetworkShareTracker::GetInstance().mApIfaceName_ = "";
}

void NetworkShareTracker::HandleHotSpotStaJoin()
{
    std::lock_guard<ffrt::mutex> lock(apStopTimerMutex_);
    NETMGR_EXT_LOG_I("Receive hotspot sta join");
    staConnected_ = true;
    NetworkShareTracker::GetInstance().HandleIdleApStopTimer();
}

void NetworkShareTracker::HandleHotSpotStaLeave()
{
    std::lock_guard<ffrt::mutex> lock(apStopTimerMutex_);
    NETMGR_EXT_LOG_I("Receive hotspot sta leave");
    NetworkShareTracker::GetInstance().GetPowerConnected();

    size_t size = 0;
    int32_t ret = GetStationNum(size);
    if (ret != Wifi::WIFI_OPT_SUCCESS) {
        NETMGR_EXT_LOG_E("Get softap station num, ret:%{public}d", ret);
    }
    NETMGR_EXT_LOG_I("Current connected station mum is:%{public}zu", size);
    staConnected_ = size != 0;
    NetworkShareTracker::GetInstance().HandleIdleApStopTimer();
}

void NetworkShareTracker::GetPowerConnected()
{
#ifdef SUPPORT_BATTERY_MANAGER
    auto batteryPluggedType = PowerMgr::BatterySrvClient::GetInstance().GetPluggedType();
    powerConnected_ = batteryPluggedType != PowerMgr::BatteryPluggedType::PLUGGED_TYPE_NONE;
    NETMGR_EXT_LOG_I("Current power connected:%{public}d", powerConnected_);
#endif
}

void NetworkShareTracker::HandleIdleApStopTimer()
{
    NETMGR_EXT_LOG_I("Current powerConnected:%{public}d, staConnected:%{public}d, apStarted:%{public}d",
        powerConnected_, staConnected_, curWifiState_);
    bool startTimer = !powerConnected_ && !staConnected_ &&
        curWifiState_ == Wifi::ApState::AP_STATE_STARTED;
    if (startTimer) {
        NetworkShareTracker::GetInstance().StartIdleApStopTimer();
    } else {
        NetworkShareTracker::GetInstance().StopIdleApStopTimer();
    }
}

void NetworkShareTracker::StartIdleApStopTimer()
{
    if (idleApStopTimerId_ != 0) {
        NETMGR_EXT_LOG_I("IdleApStopTimer exist, id:%{public}u", idleApStopTimerId_);
        return;
    }

    std::shared_ptr<SysTimer> timer = std::make_shared<SysTimer>(false, 0, false, false);
    timer->SetCallbackInfo([this] {
        NETMGR_EXT_LOG_I("Disable hotspot and push network share notification");
#ifdef SHARE_NOTIFICATION_ENABLE
        NetworkShareNotification::GetInstance().PublishNetworkShareNotification(
            NotificationId::HOTSPOT_IDLE_NOTIFICATION_ID);
#endif
        int32_t ret = DisableHotspot();
        NETMGR_EXT_LOG_E("DisableHotspot ret[%{public}d].", ret);
    });
    idleApStopTimerId_ = MiscServices::TimeServiceClient::GetInstance()->CreateTimer(timer);
    int64_t currentTime = MiscServices::TimeServiceClient::GetInstance()->GetBootTimeMs();
    MiscServices::TimeServiceClient::GetInstance()->StartTimer(idleApStopTimerId_,
        currentTime + IDLE_SOFTAP_STOP_INTERVAL_MS);
    NETMGR_EXT_LOG_I("Start new IdleApStopTimer, id:%{public}u", idleApStopTimerId_);
}

void NetworkShareTracker::StopIdleApStopTimer()
{
    NETMGR_EXT_LOG_I("Stop IdleApStopTimer, id:%{public}u", idleApStopTimerId_);
    if (idleApStopTimerId_ == 0) {
        return;
    }
    MiscServices::TimeServiceClient::GetInstance()->StopTimer(idleApStopTimerId_);
    MiscServices::TimeServiceClient::GetInstance()->DestroyTimer(idleApStopTimerId_);
    idleApStopTimerId_ = 0;
}

int32_t NetworkShareTracker::EnableHotspot()
{
    auto WifiHostInstance = Wifi::WifiHotspot::GetInstance(WIFI_HOTSPOT_ABILITY_ID);
    if (WifiHostInstance == nullptr) {
        return -1;
    }
    return WifiHostInstance->EnableHotspot();
}

int32_t NetworkShareTracker::DisableHotspot()
{
    auto WifiHostInstance = Wifi::WifiHotspot::GetInstance(WIFI_HOTSPOT_ABILITY_ID);
    if (WifiHostInstance == nullptr) {
        return -1;
    }
    return WifiHostInstance->DisableHotspot();
}

int32_t NetworkShareTracker::GetStationNum(size_t &num)
{
    auto WifiHostInstance = Wifi::WifiHotspot::GetInstance(WIFI_HOTSPOT_ABILITY_ID);
    if (WifiHostInstance == nullptr) {
        return -1;
    }
    std::vector<Wifi::StationInfo> stationInfos;
    int32_t ret = WifiHostInstance->GetStationList(stationInfos);
    num = stationInfos.size();
    return ret;
}
#endif
} // namespace NetManagerStandard
} // namespace OHOS
