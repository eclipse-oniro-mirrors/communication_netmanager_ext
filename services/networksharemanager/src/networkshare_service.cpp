
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

#include "networkshare_service.h"

#include "net_event_report.h"
#include "net_manager_center.h"
#include "netmanager_base_permission.h"
#include "netmgr_ext_log_wrapper.h"
#include "networkshare_constants.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace NetManagerStandard {
const bool REGISTER_LOCAL_RESULT_NETSHARE =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<NetworkShareService>::GetInstance().get());

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
    NETMGR_EXT_LOG_I("OnStop successful");
}

int32_t NetworkShareService::Dump(int32_t fd, const std::vector<std::u16string> &args)
{
    NETMGR_EXT_LOG_I("Start Dump, fd: %{public}d", fd);
    std::string result;
    GetDumpMessage(result);
    NETMGR_EXT_LOG_I("Dump content: %{public}s", result.c_str());
    int32_t ret = dprintf(fd, "%s\n", result.c_str());
    return ret < 0 ? static_cast<int32_t>(NetStatsResultCode::ERR_INTERNAL_ERROR)
                   : static_cast<int32_t>(NetStatsResultCode::ERR_NONE);
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
    return NetworkShareTracker::GetInstance().Init();
}

void NetworkShareService::GetDumpMessage(std::string &message)
{
    message.append("Net Sharing Info:\n");
    std::string surpportContent =
        NetworkShareTracker::GetInstance().IsNetworkSharingSupported() == NETWORKSHARE_IS_SUPPORTED ? "surpported"
                                                                                                    : "not surpported";
    message.append("\tIs Sharing Supported: " + surpportContent + "\n");
    bool isSharing = NetworkShareTracker::GetInstance().IsSharing() == NETWORKSHARE_IS_SHARING;
    std::string sharingState = isSharing ? "is sharing" : "not sharing";
    message.append("\tSharing State: " + sharingState + "\n");
    if (isSharing) {
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
    std::vector<std::string> regexs = NetworkShareTracker::GetInstance().GetSharableRegexs(type);
    for_each(regexs.begin(), regexs.end(),
             [&shareRegexsContent](std::string &regex) { shareRegexsContent += regex + ";"; });
}

int32_t NetworkShareService::IsNetworkSharingSupported()
{
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETWORKSHARE_ERROR_PERMISSION_CHECK_FAIL;
    }
    return NetworkShareTracker::GetInstance().IsNetworkSharingSupported();
}

int32_t NetworkShareService::IsSharing()
{
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETWORKSHARE_ERROR_PERMISSION_CHECK_FAIL;
    }
    return NetworkShareTracker::GetInstance().IsSharing();
}

int32_t NetworkShareService::StartNetworkSharing(const SharingIfaceType &type)
{
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETWORKSHARE_ERROR_PERMISSION_CHECK_FAIL;
    }
    return NetworkShareTracker::GetInstance().StartNetworkSharing(type);
}

int32_t NetworkShareService::StopNetworkSharing(const SharingIfaceType &type)
{
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETWORKSHARE_ERROR_PERMISSION_CHECK_FAIL;
    }
    return NetworkShareTracker::GetInstance().StopNetworkSharing(type);
}

int32_t NetworkShareService::RegisterSharingEvent(sptr<ISharingEventCallback> callback)
{
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETWORKSHARE_ERROR_PERMISSION_CHECK_FAIL;
    }
    return NetworkShareTracker::GetInstance().RegisterSharingEvent(callback);
}

int32_t NetworkShareService::UnregisterSharingEvent(sptr<ISharingEventCallback> callback)
{
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETWORKSHARE_ERROR_PERMISSION_CHECK_FAIL;
    }
    return NetworkShareTracker::GetInstance().UnregisterSharingEvent(callback);
}

std::vector<std::string> NetworkShareService::GetSharableRegexs(SharingIfaceType type)
{
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return {};
    }
    return NetworkShareTracker::GetInstance().GetSharableRegexs(type);
}

int32_t NetworkShareService::GetSharingState(SharingIfaceType type, SharingIfaceState &state)
{
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETWORKSHARE_ERROR_PERMISSION_CHECK_FAIL;
    }
    return NetworkShareTracker::GetInstance().GetSharingState(type, state);
}

std::vector<std::string> NetworkShareService::GetNetSharingIfaces(const SharingIfaceState &state)
{
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return {};
    }
    return NetworkShareTracker::GetInstance().GetNetSharingIfaces(state);
}

int32_t NetworkShareService::GetStatsRxBytes()
{
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETWORKSHARE_ERROR_PERMISSION_CHECK_FAIL;
    }
    return NetworkShareTracker::GetInstance().GetSharedSubSMTraffic(TrafficType::TRAFFIC_RX);
}

int32_t NetworkShareService::GetStatsTxBytes()
{
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETWORKSHARE_ERROR_PERMISSION_CHECK_FAIL;
    }
    return NetworkShareTracker::GetInstance().GetSharedSubSMTraffic(TrafficType::TRAFFIC_TX);
}

int32_t NetworkShareService::GetStatsTotalBytes()
{
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETWORKSHARE_ERROR_PERMISSION_CHECK_FAIL;
    }
    return NetworkShareTracker::GetInstance().GetSharedSubSMTraffic(TrafficType::TRAFFIC_ALL);
}
} // namespace NetManagerStandard
} // namespace OHOS
