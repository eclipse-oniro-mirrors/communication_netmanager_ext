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
#ifndef NETWORKSHARE_TRAFFICLIMIT_H
#define NETWORKSHARE_TRAFFICLIMIT_H

#include "ffrt.h"
#include "singleton.h"
#include "data_ability_observer_stub.h"
#include "network_sharing.h"
#include "network_state.h"
#include "event_handler.h"
#include "event_runner.h"

namespace OHOS {
namespace NetManagerStandard {

constexpr int64_t NO_LIMIT = -1;
constexpr int64_t DEFAULT_INTERVAL_MINIMUM = 100;
constexpr int64_t STATS_INTERVAL_MINIMUM = 500;
constexpr int64_t STATS_INTERVAL_MAXIMUM = 30000;
constexpr int64_t STATS_INTERVAL_DEFAULT = 5000;
constexpr int64_t WRITE_DB_INTERVAL_MINIMUM = 2000;
constexpr int64_t KB_IN_BYTES = 1024;
constexpr int64_t MB_IN_BYTES = KB_IN_BYTES * 1024;
constexpr int64_t WIFI_AP_STATS_DEFAULT_VALUE = 0;

struct TetherTrafficInfos {
    int64_t mStartSize = 0;
    int64_t mLastStatsMills = 0;
    int64_t mLastStatsSize = 0;
    int64_t mLimitSize = NO_LIMIT;
    int64_t mRemainSize = NO_LIMIT;
    int64_t mMaxSpeed = -1;
    int64_t mNetSpeed = -1;
    int64_t SharingTrafficValue = 0;
};

enum class NetworkSpeed {
    NETWORK_SPEED_2G = 100 * KB_IN_BYTES,
    NETWORK_SPEED_3G = 10 * MB_IN_BYTES,
    NETWORK_SPEED_4G = 100 * MB_IN_BYTES,
};

class SharingTrafficDataObserver;
class NetworkShareTrafficLimit {
public:
    NetworkShareTrafficLimit();
    ~NetworkShareTrafficLimit() = default;
    static NetworkShareTrafficLimit &GetInstance(void);
    void InitTetherStatsInfo();
    void UpdataSharingSettingdata(int64_t &tetherInt);
    void SaveSharingTrafficToCachedData(nmd::NetworkSharingTraffic &traffic);
    int64_t GetMaxNetworkSpeed();
    void CheckSharingStatsData();
    int64_t GetNextUpdataDelay();
    void StartHandleSharingLimitEvent();
    void EndHandleSharingLimitEvent();
    void AddSharingTrafficBeforeConnChanged(nmd::NetworkSharingTraffic &traffic);
    bool IsCellularDataConnection();
    void SaveSharingTrafficToSettingsDB(nmd::NetworkSharingTraffic &traffic);
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_ = nullptr;

private:
    void SendSharingTrafficToCachedData(const nmd::NetworkSharingTraffic &traffic,  const std::string &upIface);
    void UpdataSharingTrafficStats();
    int64_t GetNetSpeedForRadioTech(int32_t radioTech);
    void SetTetherLimit(int64_t &tetherInt);
    void sendMsgDelayed(const std::string &name, int64_t delayTime);
    int32_t GetDefaultSlotId();
    bool IsValidSlotId(int32_t slotId);
    void WriteSharingTrafficToDB(const int64_t &traffic);
    void InitEventHandler();
    std::shared_ptr<SharingTrafficDataObserver> observer_ = std::make_shared<SharingTrafficDataObserver>();

private:
    TetherTrafficInfos tetherTrafficInfos;
    std::unique_ptr<Telephony::NetworkState> networkState_ = nullptr;
    int64_t lastSharingStatsSize = 0;
    int64_t tmpMills = 0;
    ffrt::mutex lock_;
};

class TetherSingleValueObserver : public AAFwk::DataAbilityObserverStub {
public:
    TetherSingleValueObserver() = default;
    ~TetherSingleValueObserver() = default;
    void OnChange() override;
};

class SharingTrafficDataObserver {
public:
    SharingTrafficDataObserver();
    ~SharingTrafficDataObserver() = default;
    void RegisterTetherDataSettingObserver();
    void UnregisterTetherDataSettingObserver();
    void ReadTetherTrafficSetting();

public:
    sptr<TetherSingleValueObserver> mTetherSingleValueObserver_ = nullptr;
};

inline int64_t GetCurrentMilliseconds()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
        .count();
}

} // namespace NetManagerStandard
} // namespace OHOS

#endif
