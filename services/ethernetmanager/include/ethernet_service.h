/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef ETHERNET_SERVICE_H
#define ETHERNET_SERVICE_H

#include "singleton.h"
#include "system_ability.h"

#include "ethernet_service_stub.h"
#include "ethernet_management.h"
#include "ethernet_service_common.h"

namespace OHOS {
namespace NetManagerStandard {
class EthernetService : public SystemAbility, public EthernetServiceStub,
    public std::enable_shared_from_this<EthernetService> {
    DECLARE_DELAYED_SINGLETON(EthernetService)
    DECLARE_SYSTEM_ABILITY(EthernetService)

    enum ServiceRunningState {
        STATE_STOPPED = 0,
        STATE_RUNNING,
    };

public:
    void OnStart() override;
    void OnStop() override;

    int32_t SetIfaceConfig(const std::string &iface, sptr<InterfaceConfiguration> &ic) override;
    sptr<InterfaceConfiguration> GetIfaceConfig(const std::string &iface) override;
    int32_t IsIfaceActive(const std::string &iface) override;
    std::vector<std::string> GetAllActiveIfaces() override;
    int32_t ResetFactory() override;

private:
    bool Init();

private:
    ServiceRunningState state_ = ServiceRunningState::STATE_STOPPED;
    bool registerToService_ = false;
    std::unique_ptr<EthernetManagement> ethManagement_;
    sptr<EthernetServiceCommon> serviceComm_ = nullptr;
    NetLinkRtnl nlkRtnl_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // ETHERNET_SERVICE_H