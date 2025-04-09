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

#include "networkshare_service.h"

#include "net_event_report.h"
#include "net_manager_center.h"
#include "net_manager_constants.h"
#include "netmanager_base_permission.h"
#include "netmgr_ext_log_wrapper.h"
#include "networkshare_constants.h"
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"
#include "system_ability_definition.h"
#include "netsys_controller.h"
#include "edm_parameter_utils.h"
#include "ffrt.h"
#ifdef USB_MODOULE
#include "usb_srv_support.h"
#endif

namespace OHOS {
namespace NetManagerStandard {
const std::string NETWORK_TIMER = "NetworkShare::RegisterSharingEvent";
const bool REGISTER_LOCAL_RESULT_NETSHARE =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<NetworkShareService>::GetInstance().get());
constexpr int32_t XCOLLIE_TIMEOUT_DURATION = 30;
constexpr const char *NETWORK_SHARE_POLICY_PARAM = "persist.edm.tethering_disallowed";
inline const std::string IDLE_AP_USER_RESTART_NOTIFICATION = "ohos.event.notification.wifi.TAP_ENABLE_HOTSPOT";

NetworkShareService::NetworkShareService() : SystemAbility(COMM_NET_TETHERING_MANAGER_SYS_ABILITY_ID, true) {}

NetworkShareService::~NetworkShareService(){};

void NetworkShareService::OnStart()
{
    if (state_ == STATE_RUNNING) {
        NETMGR_EXT_LOG_D("OnStart Service state is already running");
        return;
    }
    if (!Init()) {
        NETMGR_EXT_LOG_E("OnStart init failed");
        EventInfo eventInfo;
        eventInfo.operatorType = static_cast<int32_t>(NetworkShareEventOperator::OPERATION_START_SA);
        eventInfo.errorType = static_cast<int32_t>(NetworkShareEventErrorType::ERROR_START_SA);
        eventInfo.errorMsg = "Start Network Share Service failed";
        NetEventReport::SendSetupFaultEvent(eventInfo);
        return;
    }
    state_ = STATE_RUNNING;
    NETMGR_EXT_LOG_I("OnStart successful");
}

void NetworkShareService::OnStop()
{
    NetworkShareTracker::GetInstance().Uninit();
    state_ = STATE_STOPPED;
    registerToService_ = false;
    EdmParameterUtils::GetInstance().UnRegisterEdmParameterChangeEvent(NETWORK_SHARE_POLICY_PARAM);
    NETMGR_EXT_LOG_I("OnStop successful");
}

int32_t NetworkShareService::Dump(int32_t fd, const std::vector<std::u16string> &args)
{
    NETMGR_EXT_LOG_I("Start Dump, fd: %{public}d", fd);
    std::string result;
    GetDumpMessage(result);
    NETMGR_EXT_LOG_I("Dump content: %{public}s", result.c_str());
    int32_t ret = dprintf(fd, "%s\n", result.c_str());
    return ret < 0 ? NETWORKSHARE_ERROR_INTERNAL_ERROR : NETMANAGER_EXT_SUCCESS;
}

bool NetworkShareService::Init()
{
    if (!REGISTER_LOCAL_RESULT_NETSHARE) {
        NETMGR_EXT_LOG_E("Register to local sa manager failed");
        return false;
    }
    if (!registerToService_) {
        if (!Publish(DelayedSingleton<NetworkShareService>::GetInstance().get())) {
            NETMGR_EXT_LOG_E("Register to sa manager failed");
            return false;
        }
        registerToService_ = true;
    }

    EdmParameterUtils::GetInstance().RegisterEdmParameterChangeEvent(NETWORK_SHARE_POLICY_PARAM,
        DisAllowNetworkShareEventCallback, this);
  
    AddSystemAbilityListener(COMM_NETSYS_NATIVE_SYS_ABILITY_ID);
    AddSystemAbilityListener(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID);
    SubscribeCommonEvent();
#ifdef SHARE_NOTIFICATION_ENABLE
    SubscribeWifiShareNtfEvent();
#endif
    return true;
}

void NetworkShareService::GetDumpMessage(std::string &message)
{
    message.append("Net Sharing Info:\n");
    int32_t supported = NETWORKSHARE_IS_UNSUPPORTED;
    NetworkShareTracker::GetInstance().IsNetworkSharingSupported(supported);
    std::string surpportContent = supported == NETWORKSHARE_IS_SUPPORTED ? "surpported" : "not surpported";
    message.append("\tIs Sharing Supported: " + surpportContent + "\n");
    int32_t sharingStatus = NETWORKSHARE_IS_UNSHARING;
    NetworkShareTracker::GetInstance().IsSharing(sharingStatus);
    std::string sharingState = sharingStatus ? "is sharing" : "not sharing";
    message.append("\tSharing State: " + sharingState + "\n");
    if (sharingStatus) {
        std::string sharingType;
        GetSharingType(SharingIfaceType::SHARING_WIFI, "wifi;", sharingType);
        GetSharingType(SharingIfaceType::SHARING_USB, "usb;", sharingType);
        GetSharingType(SharingIfaceType::SHARING_BLUETOOTH, "bluetooth;", sharingType);
        message.append("\tSharing Types: " + sharingType + "\n");
    }

    std::string wifiShareRegexs;
    GetShareRegexsContent(SharingIfaceType::SHARING_WIFI, wifiShareRegexs);
    message.append("\tUsb Regexs: " + wifiShareRegexs + "\n");
    std::string usbShareRegexs;
    GetShareRegexsContent(SharingIfaceType::SHARING_USB, usbShareRegexs);
    message.append("\tWifi Regexs: " + usbShareRegexs + "\n");
    std::string btpanShareRegexs;
    GetShareRegexsContent(SharingIfaceType::SHARING_BLUETOOTH, btpanShareRegexs);
    message.append("\tBluetooth Regexs: " + btpanShareRegexs + "\n");
}

void NetworkShareService::GetSharingType(const SharingIfaceType &type, const std::string &typeContent,
                                         std::string &sharingType)
{
    SharingIfaceState state;
    NetworkShareTracker::GetInstance().GetSharingState(type, state);
    if (state == SharingIfaceState::SHARING_NIC_SERVING) {
        sharingType += typeContent;
    }
}

void NetworkShareService::GetShareRegexsContent(const SharingIfaceType &type, std::string &shareRegexsContent)
{
    std::vector<std::string> regexs;
    NetworkShareTracker::GetInstance().GetSharableRegexs(type, regexs);
    for_each(regexs.begin(), regexs.end(),
             [&shareRegexsContent](const std::string &regex) { shareRegexsContent += regex + ";"; });
}

int32_t NetworkShareService::IsNetworkSharingSupported(int32_t &supported)
{
    NETMGR_EXT_LOG_I("NetworkSharing IsNetworkSharingSupported");
    if (!NetManagerPermission::IsSystemCaller()) {
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return NetworkShareTracker::GetInstance().IsNetworkSharingSupported(supported);
}

int32_t NetworkShareService::IsSharing(int32_t &sharingStatus)
{
    NETMGR_EXT_LOG_I("NetworkSharing IsSharing");
    if (!NetManagerPermission::IsSystemCaller()) {
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return NetworkShareTracker::GetInstance().IsSharing(sharingStatus);
}

int32_t NetworkShareService::StartNetworkSharing(int32_t typeInt)
{
    SharingIfaceType type = SharingIfaceType(typeInt);
    if (EdmParameterUtils::GetInstance().CheckBoolEdmParameter(NETWORK_SHARE_POLICY_PARAM, "false")) {
        NETMGR_EXT_LOG_E("NetworkSharing start sharing, check EDM param true");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    NETMGR_EXT_LOG_I("NetworkSharing start sharing,type is %{public}d", type);
    if (!NetManagerPermission::IsSystemCaller()) {
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    int32_t ret = NetworkShareTracker::GetInstance().StartNetworkSharing(type);
    if (ret == NETMANAGER_EXT_SUCCESS) {
        ret = NetsysController::GetInstance().UpdateNetworkSharingType(static_cast<uint32_t>(type), true);
    }
    SetConfigureForShare(true);
    return ret;
}

int32_t NetworkShareService::StopNetworkSharing(int32_t typeInt)
{
    SharingIfaceType type = SharingIfaceType(typeInt);
    SetConfigureForShare(false);
    NETMGR_EXT_LOG_I("NetworkSharing stop sharing,type is %{public}d", type);
    if (!NetManagerPermission::IsSystemCaller()) {
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    int32_t ret = NetworkShareTracker::GetInstance().StopNetworkSharing(type);
    if (ret == NETMANAGER_EXT_SUCCESS) {
        ret = NetsysController::GetInstance().UpdateNetworkSharingType(static_cast<uint32_t>(type), false);
    }

    return ret;
}

int32_t NetworkShareService::RegisterSharingEvent(const sptr<ISharingEventCallback>& callback)
{
    NETMGR_EXT_LOG_I("NetworkSharing Register Sharing Event.");
    int id = HiviewDFX::XCollie::GetInstance().SetTimer(NETWORK_TIMER, XCOLLIE_TIMEOUT_DURATION, nullptr, nullptr,
                                                        HiviewDFX::XCOLLIE_FLAG_LOG);
    if (!NetManagerPermission::IsSystemCaller()) {
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    auto ret = NetworkShareTracker::GetInstance().RegisterSharingEvent(callback);
    HiviewDFX::XCollie::GetInstance().CancelTimer(id);
    return ret;
}

int32_t NetworkShareService::UnregisterSharingEvent(const sptr<ISharingEventCallback>& callback)
{
    NETMGR_EXT_LOG_I("NetworkSharing UnRegister Sharing Event.");
    if (!NetManagerPermission::IsSystemCaller()) {
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return NetworkShareTracker::GetInstance().UnregisterSharingEvent(callback);
}

int32_t NetworkShareService::GetSharableRegexs(int32_t typeInt, std::vector<std::string> &ifaceRegexs)
{
    SharingIfaceType type = SharingIfaceType(typeInt);
    if (!NetManagerPermission::IsSystemCaller()) {
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return NetworkShareTracker::GetInstance().GetSharableRegexs(type, ifaceRegexs);
}

int32_t NetworkShareService::GetSharingState(int32_t typeInt, int32_t &stateInt)
{
    SharingIfaceType type = SharingIfaceType(typeInt);
    SharingIfaceState state = SharingIfaceState(stateInt);
    if (!NetManagerPermission::IsSystemCaller()) {
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    int32_t ret = NetworkShareTracker::GetInstance().GetSharingState(type, state);
    stateInt = static_cast<int32_t>(state);
    return ret;
}

int32_t NetworkShareService::GetNetSharingIfaces(int32_t stateInt, std::vector<std::string> &ifaces)
{
    SharingIfaceState state = SharingIfaceState(stateInt);
    if (!NetManagerPermission::IsSystemCaller()) {
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return NetworkShareTracker::GetInstance().GetNetSharingIfaces(state, ifaces);
}

int32_t NetworkShareService::GetStatsRxBytes(int32_t &bytes)
{
    if (!NetManagerPermission::IsSystemCaller()) {
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return NetworkShareTracker::GetInstance().GetSharedSubSMTraffic(TrafficType::TRAFFIC_RX, bytes);
}

int32_t NetworkShareService::GetStatsTxBytes(int32_t &bytes)
{
    if (!NetManagerPermission::IsSystemCaller()) {
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return NetworkShareTracker::GetInstance().GetSharedSubSMTraffic(TrafficType::TRAFFIC_TX, bytes);
}

int32_t NetworkShareService::GetStatsTotalBytes(int32_t &bytes)
{
    if (!NetManagerPermission::IsSystemCaller()) {
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return NetworkShareTracker::GetInstance().GetSharedSubSMTraffic(TrafficType::TRAFFIC_ALL, bytes);
}

int32_t NetworkShareService::SetConfigureForShare(bool enabled)
{
    if (!NetManagerPermission::IsSystemCaller()) {
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    NETMGR_EXT_LOG_I("SetConfigureForShare begin, %{public}d", enabled);
    std::lock_guard<ffrt::mutex> lock(setConfigureMutex_);
    if (enabled) {
        setConfigTimes_++;

        if (setConfigTimes_ > 1) {
            return NETMANAGER_EXT_SUCCESS;
        }
    } else {
        setConfigTimes_ = setConfigTimes_ > 0 ? --setConfigTimes_ : setConfigTimes_;
        if (setConfigTimes_ > 0) {
            return NETMANAGER_EXT_SUCCESS;
        }
    }
    std::shared_ptr<ffrt::queue> networkShareFfrtQueue = std::make_shared<ffrt::queue>("NetworkShare");
    if (!networkShareFfrtQueue) {
        NETMGR_EXT_LOG_E("SetConfigureForShare error, FFRT Init Fail");
        return NETMANAGER_EXT_ERR_OPERATION_FAILED;
    }
    auto ffrtSetConfig = [setConfigSharedPtr = std::make_shared<NetworkShareConfiguration>(), enabled,
            shareServiceWeakPtr = std::weak_ptr<NetworkShareService>(shared_from_this())]() {
        auto shareServiceSharedPtr = shareServiceWeakPtr.lock();
        if (!shareServiceSharedPtr) {
            NETMGR_EXT_LOG_E("SetConfigureForShare error, NetworkShareService instance is invalid");
            return;
        }
        std::lock_guard<ffrt::mutex> lock(shareServiceSharedPtr->openFileMutex_);
        setConfigSharedPtr->SetConfigureForShare(enabled);
    };
    networkShareFfrtQueue->submit(ffrtSetConfig);
    return NETMANAGER_EXT_SUCCESS;
}

void NetworkShareService::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    NETMGR_EXT_LOG_D("OnAddSystemAbility systemAbilityId[%{public}d]", systemAbilityId);
    if (systemAbilityId == COMM_NETSYS_NATIVE_SYS_ABILITY_ID) {
        if (hasSARemoved_) {
            OnNetSysRestart();
            hasSARemoved_ = false;
        }
    }
    if (systemAbilityId == COMM_NET_CONN_MANAGER_SYS_ABILITY_ID) {
        NetworkShareTracker::GetInstance().Init();
    }
}

void NetworkShareService::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    NETMGR_EXT_LOG_D("OnRemoveSystemAbility systemAbilityId[%{public}d]", systemAbilityId);
    if (systemAbilityId == COMM_NETSYS_NATIVE_SYS_ABILITY_ID) {
        hasSARemoved_ = true;
    }
}

void NetworkShareService::OnNetSysRestart()
{
    NETMGR_EXT_LOG_I("OnNetSysRestart");
    NetworkShareTracker::GetInstance().RestartResume();
}

void NetworkShareService::DisAllowNetworkShareEventCallback(const char *key, const char *value, void *context)
{
    if (strcmp(value, "true") == 0) {
        NETMGR_EXT_LOG_I("DisAllowNetworkShareEventCallback calledstop all network sharing with %{public}s", value);

        if (!context) {
            NETMGR_EXT_LOG_I("DisAllowNetworkShareEventCallback context is NULL");
            return;
        }

        NetworkShareService* servicePtr = static_cast<NetworkShareService*>(context);
        std::string sharingType;
        servicePtr->GetSharingType(SharingIfaceType::SHARING_WIFI, "wifi;", sharingType);
        servicePtr->GetSharingType(SharingIfaceType::SHARING_USB, "usb;", sharingType);
        servicePtr->GetSharingType(SharingIfaceType::SHARING_BLUETOOTH, "bluetooth;", sharingType);
        if (sharingType.find("wifi") != std::string::npos) {
            std::function<void()> StopNetworkSharingWifi =
                [servicePtr]() { servicePtr->StopNetworkSharing(
                    static_cast<int32_t>(SharingIfaceType::SHARING_WIFI)); };
            ffrt::task_handle wifiHandle = ffrt::submit_h(StopNetworkSharingWifi,
                ffrt::task_attr().name("StopNetworkSharingWifi_task"));
            ffrt::wait({wifiHandle});
            NETMGR_EXT_LOG_D("DisAllowNetworkShareEventCallback stop wifi end");
        }
        if (sharingType.find("usb") != std::string::npos) {
            std::function<void()> StopNetworkSharingUsb =
                [servicePtr]() { servicePtr->StopNetworkSharing(
                    static_cast<int32_t>(SharingIfaceType::SHARING_USB)); };
            ffrt::task_handle usbHandle = ffrt::submit_h(StopNetworkSharingUsb,
                ffrt::task_attr().name("StopNetworkSharingUsb_task"));
            ffrt::wait({usbHandle});
            NETMGR_EXT_LOG_D("DisAllowNetworkShareEventCallback stop usb end");
        }
        if (sharingType.find("bluetooth") != std::string::npos) {
            std::function<void()> StopNetworkSharingBluetooth =
                [servicePtr]() { servicePtr->StopNetworkSharing(
                    static_cast<int32_t>(SharingIfaceType::SHARING_BLUETOOTH)); };
            ffrt::task_handle bluetoothHandle = ffrt::submit_h(StopNetworkSharingBluetooth,
                ffrt::task_attr().name("StopNetworkSharingBluetooth_task"));
            ffrt::wait({bluetoothHandle});
            NETMGR_EXT_LOG_D("DisAllowNetworkShareEventCallback stop bluetooth end");
        }
        NETMGR_EXT_LOG_D("DisAllowNetworkShareEventCallback all end");
        return;
    }
}

void NetworkShareService::SubscribeCommonEvent()
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
    matchingSkills.AddEvent(OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED);
#ifdef USB_MODOULE
    matchingSkills.AddEvent(OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_USB_STATE);
#endif
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // 1 means CORE_EVENT_PRIORITY
    subscribeInfo.SetPriority(1);
    commonEventSubscriber_ = std::make_shared<CommonEventSubscriber>(subscribeInfo);
    if (commonEventSubscriber_ == nullptr) {
        NETMGR_EXT_LOG_E("Subscribe common event subscriber_ is NULL");
        return;
    }
    bool ret = EventFwk::CommonEventManager::SubscribeCommonEvent(commonEventSubscriber_);
    if (!ret) {
        NETMGR_EXT_LOG_E("Subscribe common event fail:%{public}d", ret);
    }
}

void NetworkShareService::CommonEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    const auto &action = eventData.GetWant().GetAction();
    NETMGR_EXT_LOG_I("NetworkShareService::OnReceiveEvent: %{public}s.", action.c_str());
    if (action == OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED) {
        NetworkShareTracker::GetInstance().OnPowerConnected();
    } else if (action == OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED) {
        NetworkShareTracker::GetInstance().OnPowerDisConnected();
#ifdef USB_MODOULE
    } else if (action == OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_USB_STATE) {
        auto want = eventData.GetWant();
        auto connected = want.GetBoolParam(std::string {USB::UsbSrvSupport::CONNECTED}, false);
        NETMGR_EXT_LOG_I("UsbSrvSupport::CONNECTED: %{public}d.", connected);
        if (!connected) {
            NetworkShareTracker::GetInstance().StopNetworkSharing(SharingIfaceType::SHARING_USB);
        }
#endif
    }
}

#ifdef SHARE_NOTIFICATION_ENABLE
void NetworkShareService::SubscribeWifiShareNtfEvent()
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(IDLE_AP_USER_RESTART_NOTIFICATION);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // 1 means CORE_EVENT_PRIORITY
    subscribeInfo.SetPriority(1);
    subscribeInfo.SetPermission("ohos.permission.SET_WIFI_CONFIG");
    wifiShareNtfSubscriber_ = std::make_shared<WifiShareNtfSubscriber>(subscribeInfo);
    if (wifiShareNtfSubscriber_ == nullptr) {
        NETMGR_EXT_LOG_E("Subscribe common event subscriber_ is NULL");
        return;
    }
    bool ret = EventFwk::CommonEventManager::SubscribeCommonEvent(wifiShareNtfSubscriber_);
    if (!ret) {
        NETMGR_EXT_LOG_E("Subscribe common event fail:%{public}d", ret);
    }
}

void NetworkShareService::WifiShareNtfSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    const auto &action = eventData.GetWant().GetAction();
    NETMGR_EXT_LOG_I("NetworkShareService::OnReceiveEvent: %{public}s.", action.c_str());
    if (action == IDLE_AP_USER_RESTART_NOTIFICATION) {
        NetworkShareTracker::GetInstance().StartNetworkSharing(SharingIfaceType::SHARING_WIFI);
    }
}
#endif
} // namespace NetManagerStandard
} // namespace OHOS
