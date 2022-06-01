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

#ifndef ETHERNET_MANAGEMENT_H
#define ETHERNET_MANAGEMENT_H

#include <map>
#include <mutex>

#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "dev_interface_state.h"
#include "nlk_event_handle.h"
#include "netLink_rtnl.h"

#include "ethernet_configuration.h"
#include "ethernet_dhcp_controller.h"
namespace OHOS {
namespace NetManagerStandard {
class EthernetManagement : public NlkEventHandle {
    class EhternetDhcpNotifyCallback : public EthernetDhcpCallback {
    public:
        EhternetDhcpNotifyCallback(EthernetManagement &ethernetManagement);
        int32_t OnDhcpSuccess(EthernetDhcpCallback::DhcpResult &dhcpResult) override;
    private:
        EthernetManagement &ethernetManagement_;
    };
public:
    EthernetManagement();
    ~EthernetManagement();
    void Init();
    void UpdateInterfaceState(const std::string &dev, bool up, bool lowerUp);
    int32_t UpdateDevInterfaceState(const std::string &iface, sptr<InterfaceConfiguration> cfg);
    int32_t UpdateDevInterfaceLinkInfo(EthernetDhcpCallback::DhcpResult &dhcpResult);
    sptr<InterfaceConfiguration> GetDevInterfaceCfg(const std::string &iface);
    int32_t IsIfaceActive(const std::string &iface);
    std::vector<std::string> GetAllActiveIfaces();
    int32_t ResetFactory();
    void RegisterNlk(NetLinkRtnl &nlk);
    void Handle(const struct NlkEventInfo &info) override;

private:
    void StartDhcpClient(const std::string &dev, sptr<DevInterfaceState> &devState);
    void StopDhcpClient(const std::string &dev, sptr<DevInterfaceState> &devState);
    void SetDevState(sptr<DevInterfaceState> &devState, const std::string &devName,
        const std::vector<uint8_t> &hwAddr, bool up, bool lowerUp);
    void StartSetDevUpThd();

private:
    std::map<std::string, std::set<NetCap>> devCaps_;
    std::map<std::string, sptr<InterfaceConfiguration>> devCfgs_;
    std::map<std::string, sptr<DevInterfaceState>> devs_;
    std::unique_ptr<EthernetConfiguration> ethConfiguration_ = nullptr;
    std::unique_ptr<EthernetDhcpController> ethDhcpController_ = nullptr;
    sptr<EhternetDhcpNotifyCallback> ethDhcpNotifyCallback_;
    std::mutex mutex_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // ETHERNET_MANAGEMENT_H