/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include <cstdint>
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#include "ethernet_management.h"
#include "ethernet_service_common.h"
#include "ethernet_service_stub.h"
#include "event_handler.h"
#include "netsys_controller_callback.h"
#include "refbase.h"
#include "singleton.h"
#include "system_ability.h"

namespace OHOS {
namespace NetManagerStandard {
class EthernetService : public SystemAbility,
                        public EthernetServiceStub,
                        public std::enable_shared_from_this<EthernetService> {
    DECLARE_DELAYED_SINGLETON(EthernetService)
    DECLARE_SYSTEM_ABILITY(EthernetService)

    class GlobalInterfaceStateCallback : public NetsysControllerCallback {
    public:
        explicit GlobalInterfaceStateCallback(EthernetService &ethService) : ethernetService_(ethService) {}
        ~GlobalInterfaceStateCallback() = default;
        int32_t OnInterfaceAddressUpdated(const std::string &addr, const std::string &ifName, int flags,
                                          int scope) override;
        int32_t OnInterfaceAddressRemoved(const std::string &addr, const std::string &ifName, int flags,
                                          int scope) override;
        int32_t OnInterfaceAdded(const std::string &iface) override;
        int32_t OnInterfaceRemoved(const std::string &iface) override;
        int32_t OnInterfaceChanged(const std::string &iface, bool up) override;
        int32_t OnInterfaceLinkStateChanged(const std::string &ifName, bool up) override;
        int32_t OnRouteChanged(bool updated, const std::string &route, const std::string &gateway,
                               const std::string &ifName) override;
        int32_t OnDhcpSuccess(NetsysControllerCallback::DhcpResult &dhcpResult) override;
        int32_t OnBandwidthReachedLimit(const std::string &limitName, const std::string &iface) override;

    private:
        EthernetService &ethernetService_;
    };

public:
    void OnStart() override;
    void OnStop() override;
    int32_t Dump(int32_t fd, const std::vector<std::u16string> &args) override;

    int32_t SetIfaceConfig(const std::string &iface, sptr<InterfaceConfiguration> &ic) override;
    int32_t GetIfaceConfig(const std::string &iface, sptr<InterfaceConfiguration> &ifaceConfig) override;
    int32_t IsIfaceActive(const std::string &iface, int32_t &activeStatus) override;
    int32_t GetAllActiveIfaces(std::vector<std::string> &activeIfaces) override;
    int32_t ResetFactory() override;
    int32_t RegisterIfacesStateChanged(const sptr<InterfaceStateCallback> &callback) override;
    int32_t UnregisterIfacesStateChanged(const sptr<InterfaceStateCallback> &callback) override;
    int32_t SetInterfaceUp(const std::string &iface) override;
    int32_t SetInterfaceDown(const std::string &iface) override;
    int32_t GetInterfaceConfig(const std::string &iface, OHOS::nmd::InterfaceConfigurationParcel &config) override;
    int32_t SetInterfaceConfig(const std::string &iface, OHOS::nmd::InterfaceConfigurationParcel &cfg) override;

protected:
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;

private:
    enum ServiceRunningState {
        STATE_STOPPED = 0,
        STATE_RUNNING,
    };

    using OnFunctionT = std::function<void(const sptr<InterfaceStateCallback> &callback)>;

    bool Init();
    void InitManagement();

    int32_t RegisterMonitorIfaceCallbackAsync(const sptr<InterfaceStateCallback> &callback);
    int32_t UnregisterMonitorIfaceCallbackAsync(const sptr<InterfaceStateCallback> &callback);
    void NotifyMonitorIfaceCallbackAsync(OnFunctionT onFunction);

private:
    ServiceRunningState state_ = ServiceRunningState::STATE_STOPPED;
    bool registerToService_ = false;
    uint16_t dependentServiceState_ = 0;
    EthernetManagement& ethManagement_ = EthernetManagement::GetInstance();
    sptr<EthernetServiceCommon> serviceComm_ = nullptr;
    sptr<NetsysControllerCallback> interfaceStateCallback_ = nullptr;
    std::vector<sptr<InterfaceStateCallback>> monitorIfaceCallbacks_;
    std::shared_ptr<AppExecFwk::EventRunner> policyCallRunner_;
    std::shared_ptr<AppExecFwk::EventHandler> policyCallHandler_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // ETHERNET_SERVICE_H
