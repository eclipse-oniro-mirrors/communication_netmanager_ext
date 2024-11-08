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

#ifndef WEARABLE_DISTRIBUTED_NET_SERVICE_H
#define WEARABLE_DISTRIBUTED_NET_SERVICE_H

#include <cstdint>
#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"
#include "iservice_registry.h"
#include "system_ability.h"
#include "system_ability_definition.h"
#include "wearable_distributed_net_stub.h"
#include "wearable_distributed_net_management.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
    using ChargeState = OHOS::PowerMgr::BatteryChargeState;
    using Event = EventFwk::CommonEventSupport;
}
class WearableDistributedNetService : public SystemAbility,
                                      public WearableDistributedNetStub {
    DECLARE_SYSTEM_ABILITY(WearableDistributedNetService)

public:
    WearableDistributedNetService(int32_t saId, bool runOnCreate = true);
    ~WearableDistributedNetService();
    void OnStart() override;
    void OnStop() override;
    int32_t SetupWearableDistributedNet(int32_t tcpPortId, int32_t udpPortId, bool isMetered) override;
    int32_t TearDownWearableDistributedNet() override;

private:
    class ReceiveMessage : public OHOS::EventFwk::CommonEventSubscriber {
    public:
        ReceiveMessage(const EventFwk::CommonEventSubscribeInfo &subscriberInfo,
            WearableDistributedNetService &WearableDistributedNetService) :
            EventFwk::CommonEventSubscriber(subscriberInfo),
            WearableDistributedNetService_(WearableDistributedNetService) {};
 
        void OnReceiveEvent(const EventFwk::CommonEventData &eventData) override;
 
    private:
        WearableDistributedNetService &WearableDistributedNetService_;
    };

    enum ServiceRunningState {
        STATE_STOPPED = 0,
        STATE_RUNNING,
    };

    bool Init();
    void UpdateNetScore(const bool isCharging);
    bool SubscribeCommonEvent();

private:
    ServiceRunningState state_ = ServiceRunningState::STATE_STOPPED;
    bool registerToService_ = false;
    std::shared_ptr<ReceiveMessage> subscriber_ = nullptr;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // WEARABLE_DISTRIBUTED_NET_SERVICE_H
