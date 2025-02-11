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
#include "networkshare_trafficlimit.h"
#include "netmgr_ext_log_wrapper.h"
#include "net_datashare_utils.h"
#include "edm_parameter_utils.h"
#include "net_manager_constants.h"
#include "net_stats_constants.h"
#include "netsys_controller.h"
#include "ffrt_inner.h"
#include "net_stats_client.h"
#include "networkshare_tracker.h"
#include "cellular_data_client.h"
#include "networkshare_sub_statemachine.h"
#include "core_service_client.h"


namespace OHOS {
namespace NetManagerStandard {

static constexpr const char *SHARE_SETTING_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/USER_SETTINGSDATA_SECURE_100?Proxy=true&key=";
const std::string SHARE_LIMIT = "wifiap_one_usage_limit";
const std::string SHARING_LIMIT_TASK_NAME = "networkshare_traffic_limit";
constexpr int64_t SECOND_IN_MILLIS = 1000;
constexpr int32_t NUMBER_THREE = 3;
constexpr int32_t UTIL_MIN_SUB_ID = 0;
constexpr int32_t UTIL_SLOT_0 = 0;


NetworkShareTrafficLimit::NetworkShareTrafficLimit()
{
    InitEventHandler();
}

NetworkShareTrafficLimit &NetworkShareTrafficLimit::GetInstance()
{
    static NetworkShareTrafficLimit instance;
    return instance;
}

void NetworkShareTrafficLimit::InitTetherStatsInfo()
{
    tetherTrafficInfos.mMaxSpeed = GetMaxNetworkSpeed();
    tetherTrafficInfos.mStartSize = 0;
    tetherTrafficInfos.mLastStatsMills = GetCurrentMilliseconds();
    tetherTrafficInfos.mLastStatsSize = 0;
    tetherTrafficInfos.mRemainSize = tetherTrafficInfos.mLimitSize;
    tetherTrafficInfos.mNetSpeed = 0;
    tetherTrafficInfos.SharingTrafficValue = 0;
}

int64_t NetworkShareTrafficLimit::GetMaxNetworkSpeed()
{
    int32_t slotId = GetDefaultSlotId();
    int32_t radioTech = static_cast<int32_t>(Telephony::RadioTech::RADIO_TECHNOLOGY_INVALID);
    Telephony::CoreServiceClient::GetInstance().GetPsRadioTech(slotId, radioTech);
    NETMGR_EXT_LOG_D("RadioTech=[%{public}d]", static_cast<int32_t>(radioTech));
    return GetNetSpeedForRadioTech(radioTech);
}

int32_t NetworkShareTrafficLimit::GetDefaultSlotId()
{
    int32_t slotId = DelayedRefSingleton<Telephony::CellularDataClient>::GetInstance().GetDefaultCellularDataSlotId();
    return IsValidSlotId(slotId) ? slotId : UTIL_SLOT_0;
}

bool NetworkShareTrafficLimit::IsValidSlotId(int32_t slotId)
{
    return (slotId >= UTIL_MIN_SUB_ID);
}

int64_t NetworkShareTrafficLimit::GetNetSpeedForRadioTech(int32_t radioTech)
{
    switch (static_cast<Telephony::RadioTech>(radioTech)) {
        case Telephony::RadioTech::RADIO_TECHNOLOGY_WCDMA:
        case Telephony::RadioTech::RADIO_TECHNOLOGY_HSPAP:
        case Telephony::RadioTech::RADIO_TECHNOLOGY_HSPA:
            return static_cast<int64_t>(NetworkSpeed::NETWORK_SPEED_3G);
        case Telephony::RadioTech::RADIO_TECHNOLOGY_LTE:
        case Telephony::RadioTech::RADIO_TECHNOLOGY_LTE_CA:
            return static_cast<int64_t>(NetworkSpeed::NETWORK_SPEED_4G);
        default:
            return static_cast<int64_t>(NetworkSpeed::NETWORK_SPEED_2G);
    }
}

void NetworkShareTrafficLimit::StartHandleSharingLimitEvent()
{
    NETMGR_EXT_LOG_I("StartHandleSharingLimitEvent");
        std::shared_ptr<SharingTrafficDataObserver> observer = std::make_shared<SharingTrafficDataObserver>();
        observer->ReadTetherTrafficSetting();
        observer->RegisterTetherDataSettingObserver();
        InitTetherStatsInfo();
        eventHandler_->HandleRemoveTask(SHARING_LIMIT_TASK_NAME);
        sendMsgDelayed(SHARING_LIMIT_TASK_NAME, STATS_INTERVAL_DEFAULT);
}

void NetworkShareTrafficLimit::EndHandleSharingLimitEvent()
{
    NETMGR_EXT_LOG_I("EndHandleSharingLimitEvent");
        std::shared_ptr<SharingTrafficDataObserver> observer = std::make_shared<SharingTrafficDataObserver>();
        observer->UnregisterTetherDataSettingObserver();
        eventHandler_->HandleRemoveTask(SHARING_LIMIT_TASK_NAME);
}

SharingTrafficDataObserver::SharingTrafficDataObserver()
{
    mTetherSingleValueObserver_ = std::make_unique<TetherSingleValueObserver>().release();
}

void SharingTrafficDataObserver::RegisterTetherDataSettingObserver()
{
    NETMGR_EXT_LOG_E("RegisterTetherDataSettingObserver start.");
    auto dataShareHelperUtils = std::make_unique<NetDataShareHelperUtils>();
    Uri uriTether(SHARE_SETTING_URI + SHARE_LIMIT);
    if (dataShareHelperUtils->RegisterSettingsObserver(uriTether, mTetherSingleValueObserver_) != NETSYS_SUCCESS) {
        NETMGR_EXT_LOG_E("register mTetherSingleValueObserver_ failed.");
    }
}

void SharingTrafficDataObserver::UnregisterTetherDataSettingObserver()
{
    NETMGR_EXT_LOG_E("UnregisterTetherDataSettingObserver start.");
    auto dataShareHelperUtils = std::make_unique<NetDataShareHelperUtils>();
    Uri uriTether(SHARE_SETTING_URI + SHARE_LIMIT);
    if (dataShareHelperUtils->UnRegisterSettingsObserver(uriTether, mTetherSingleValueObserver_) != NETSYS_SUCCESS) {
        NETMGR_EXT_LOG_E("unregister mTetherSingleValueObserver_ failed.");
    }
}

void SharingTrafficDataObserver::ReadTetherTrafficSetting()
{
    NETMGR_EXT_LOG_E("ReadTetherTrafficSetting start.");
    auto dataShareHelperUtils = std::make_unique<NetDataShareHelperUtils>();
    Uri mLimitUri(SHARE_SETTING_URI + SHARE_LIMIT);
    std::string value = "";
    dataShareHelperUtils->Query(mLimitUri, SHARE_LIMIT, value);
    int64_t tetherInt = -1;
    int64_t tetherTmp = -1;
    if (!value.empty() && EdmParameterUtils::ConvertToInt64(value, tetherTmp)) {
        tetherInt = tetherTmp;
    }
    NETMGR_EXT_LOG_D("tether limit value=[%{public}s].", std::to_string(tetherInt).c_str());
    NetworkShareTrafficLimit::GetInstance().UpdataSharingSettingdata(tetherInt);
}

void TetherSingleValueObserver::OnChange()
{
    Uri uriTether(SHARE_SETTING_URI + SHARE_LIMIT);
    std::string value = "";
    auto dataShareHelperUtils = std::make_unique<NetDataShareHelperUtils>();
    dataShareHelperUtils->Query(uriTether, SHARE_LIMIT, value);
    int64_t tetherInt = -1;
    int64_t tetherTmp = -1;
    if (!value.empty() && EdmParameterUtils::ConvertToInt64(value, tetherTmp)) {
        tetherInt = tetherTmp;
    }
    NETMGR_EXT_LOG_E("TetherSingleValueObserver OnChanged. dataString: %{public}s, TrafficInt: %{public}lu",
        value.c_str(), tetherInt);
    NetworkShareTrafficLimit::GetInstance().UpdataSharingSettingdata(tetherInt);
    int32_t sharingStatus;
    NetworkShareTracker::GetInstance().IsSharing(sharingStatus);
    if (sharingStatus == NETWORKSHARE_IS_SHARING) {
        NetworkShareTrafficLimit::GetInstance().CheckSharingStatsData();
    }
}

void NetworkShareTrafficLimit::UpdataSharingSettingdata(int64_t &tetherInt)
{
    SetTetherLimit(tetherInt);
}

void NetworkShareTrafficLimit::SetTetherLimit(int64_t &tetherInt)
{
    if (tetherInt < 0) {
        tetherTrafficInfos.mLimitSize = NO_LIMIT;
        return;
    }
    tetherTrafficInfos.mLimitSize = tetherInt * MB_IN_BYTES;
}

void NetworkShareTrafficLimit::CheckSharingStatsData()
{
    UpdataSharingTrafficStats();
    if (tetherTrafficInfos.mLimitSize < 0) {
        flag = true;
        eventHandler_->HandleRemoveTask(SHARING_LIMIT_TASK_NAME);
        return;
    }

    if (flag == true && eventHandler_ != nullptr) {
        eventHandler_->HandleRemoveTask(SHARING_LIMIT_TASK_NAME);
        sendMsgDelayed(SHARING_LIMIT_TASK_NAME, STATS_INTERVAL_MINIMUM);
        flag = false;
        return;
    }

    if (tetherTrafficInfos.mRemainSize < tetherTrafficInfos.mNetSpeed * STATS_INTERVAL_MINIMUM) {
        NETMGR_EXT_LOG_I("sharing taffic limit, shut down AP");
        int32_t ret = NetworkShareTracker::GetInstance().StopNetworkSharing(SharingIfaceType::SHARING_WIFI);
        if (ret != NETWORKSHARE_ERROR_WIFI_SHARING) {
            NETMGR_EXT_LOG_I("DisableHotspot wifiSharing successful.");
        }
        return;
    }

    if (eventHandler_ != nullptr) {
        int64_t updateDelay = STATS_INTERVAL_DEFAULT;
        if (IsCellularDataConnection()) {
            updateDelay = GetNextUpdataDelay();
        }
        eventHandler_->HandleRemoveTask(SHARING_LIMIT_TASK_NAME);
        sendMsgDelayed(SHARING_LIMIT_TASK_NAME, updateDelay);
        NETMGR_EXT_LOG_I("keep update when mIsDataConnAvailable, UpdateDelay=[%{public}ld]", updateDelay);
    }
}

void NetworkShareTrafficLimit::sendMsgDelayed(const std::string &name, int64_t delayTime)
{
    std::function<void()> delayed = ([this]() {
        NetworkShareTrafficLimit::CheckSharingStatsData();
        NETMGR_EXT_LOG_D("sendMsgDelayed enter");
    });
    eventHandler_->HandlePostTask(delayed, name, delayTime);
}

bool NetworkShareTrafficLimit::IsCellularDataConnection()
{
    int32_t dataState = Telephony::CellularDataClient::GetInstance().GetCellularDataState();
    return dataState == static_cast<int32_t>(Telephony::DataConnectionStatus::DATA_STATE_CONNECTED);
}

int64_t NetworkShareTrafficLimit::GetNextUpdataDelay()
{
    int64_t maxDelay = STATS_INTERVAL_MAXIMUM;
    if (tetherTrafficInfos.mMaxSpeed > 0) {
        maxDelay = tetherTrafficInfos.mRemainSize * SECOND_IN_MILLIS / tetherTrafficInfos.mMaxSpeed;
        maxDelay = EdmParameterUtils::constrain(maxDelay, STATS_INTERVAL_MINIMUM, STATS_INTERVAL_MAXIMUM);
    }
    int64_t delay;
    if (tetherTrafficInfos.mNetSpeed == 0) {
        delay = maxDelay;
    } else {
        delay = tetherTrafficInfos.mRemainSize / tetherTrafficInfos.mNetSpeed / NUMBER_THREE;
        delay = EdmParameterUtils::constrain(delay, STATS_INTERVAL_MINIMUM, maxDelay);
    }
    return delay;
}

void NetworkShareTrafficLimit::UpdataSharingTrafficStats()
{
    nmd::NetworkSharingTraffic traffic;
    std::string ifaceName;
    int32_t ret = NetsysController::GetInstance().GetNetworkCellularSharingTraffic(traffic, ifaceName);
    int64_t tetherStats = static_cast<int64_t>(traffic.receive + traffic.send);//bytes
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_EXT_LOG_E("GetTrafficBytes err, ret[%{public}d].", ret);
        return;
    }

    int64_t statsMills = GetCurrentMilliseconds();
    if (tetherStats < tetherTrafficInfos.mStartSize) {
        NETMGR_EXT_LOG_E("updata tether traffic error");
        return;
    }

    int64_t statsSize = tetherStats - tetherTrafficInfos.mStartSize;
    int64_t elapsedSize = statsSize + tetherTrafficInfos.SharingTrafficValue - tetherTrafficInfos.mLastStatsSize;
    int64_t elapsedMills = statsMills - tetherTrafficInfos.mLastStatsMills;
    if (elapsedMills == 0) {
        return;
    }
    tetherTrafficInfos.mNetSpeed = static_cast<int64_t>(elapsedSize / elapsedMills);
    tetherTrafficInfos.mLastStatsMills = statsMills;
    tetherTrafficInfos.mLastStatsSize = statsSize + tetherTrafficInfos.SharingTrafficValue;
    if (tetherTrafficInfos.mLimitSize >= 0 && tetherTrafficInfos.mLimitSize >= tetherTrafficInfos.mLastStatsSize) {
        tetherTrafficInfos.mRemainSize = tetherTrafficInfos.mLimitSize - tetherTrafficInfos.mLastStatsSize;
    } else {
        tetherTrafficInfos.mRemainSize = -1;
    }
}

void NetworkShareTrafficLimit::AddSharingTrafficBeforeConnChanged(nmd::NetworkSharingTraffic &traffic)
{
    NETMGR_EXT_LOG_E("AddSharingTrafficBeforeConnChanged enter");
    tetherTrafficInfos.SharingTrafficValue += traffic.receive;
    tetherTrafficInfos.SharingTrafficValue += traffic.send;
}

void NetworkShareTrafficLimit::InitEventHandler()
{
    auto eventRunner = AppExecFwk::EventRunner::Create(true, AppExecFwk::ThreadMode::FFRT);
    if (eventRunner == nullptr) {
        NETMGR_EXT_LOG_E("Failed to create a recvRunner");
        return;
    }
    eventHandler_ = std::make_shared<TrafficEventHandler>(eventRunner);
}

TrafficEventHandler::TrafficEventHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner)
    : EventHandler(runner)
{}

TrafficEventHandler::~TrafficEventHandler() = default;

bool TrafficEventHandler::HandlePostTask(const Callback &callback, int64_t delayTime)
{
    return AppExecFwk::EventHandler::PostTask(callback, delayTime);
}

bool TrafficEventHandler::HandlePostTask(const Callback &callback, const std::string &name, int64_t delayTime)
{
    return AppExecFwk::EventHandler::PostTask(callback, name, delayTime);
}

void TrafficEventHandler::HandleRemoveTask(const std::string &name)
{
    AppExecFwk::EventHandler::RemoveTask(name);
}

// 写数据库
void NetworkShareTrafficLimit::SaveSharingTrafficToCachedData(nmd::NetworkSharingTraffic &traffic)
{
    NETMGR_EXT_LOG_I("SaveSharingTrafficToCachedData enter");
    std::string ifaceName;
    int32_t ret0 = NetsysController::GetInstance().GetNetworkCellularSharingTraffic(traffic, ifaceName);
    if (ret0 != NETMANAGER_SUCCESS) {
        NETMGR_EXT_LOG_E("GetTrafficBytes err, ret[%{public}d].", ret0);
        return;
    }
    ffrt::submit([traffic, ifaceName, this]() {SendSharingTrafficToCachedData(traffic, ifaceName);}, {}, {},
        ffrt::task_attr().name(("SendSharingTrafficToCachedData")));
}

void NetworkShareTrafficLimit::SendSharingTrafficToCachedData(const nmd::NetworkSharingTraffic &traffic,
                                                                const std::string &upIface)
{
    NETMGR_EXT_LOG_I("NetworkShareSubStateMachine SendSharingTrafficToCachedData enter");
    NetStatsInfo infos;
    infos.txBytes_ = static_cast<int64_t>(traffic.send);
    infos.rxBytes_ = static_cast<int64_t>(traffic.receive);
    infos.iface_ = upIface;
    auto ret = DelayedSingleton<NetStatsClient>::GetInstance()->SaveSharingTraffic(infos);
    if (ret != 0) {
        NETMGR_EXT_LOG_E("SaveSharingTraffic ERROR");
    }
}
} // namespace NetManagerStandard
} // namespace OHOS