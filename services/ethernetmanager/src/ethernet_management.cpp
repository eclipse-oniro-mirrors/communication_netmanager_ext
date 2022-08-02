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

#include "ethernet_management.h"

#include <regex>
#include <unistd.h>
#include <thread>

#include "netsys_controller.h"
#include "netmgr_ext_log_wrapper.h"
#include "ethernet_constants.h"
#include "securec.h"

namespace OHOS {
namespace NetManagerStandard {
const std::string IFACE_MATCH = "eth\\d";
EthernetManagement::EhternetDhcpNotifyCallback::EhternetDhcpNotifyCallback(EthernetManagement &ethernetManagement)
    : ethernetManagement_(ethernetManagement)
{
}

int32_t EthernetManagement::EhternetDhcpNotifyCallback::OnDhcpSuccess(EthernetDhcpCallback::DhcpResult &dhcpResult)
{
    NETMGR_EXT_LOG_D("EthernetManagement::EhternetDhcpNotifyCallback::OnDhcpSuccess");
    ethernetManagement_.UpdateDevInterfaceLinkInfo(dhcpResult);
    return 0;
}

EthernetManagement::EthernetManagement()
{
    ethDhcpNotifyCallback_  = std::make_unique<EhternetDhcpNotifyCallback>(*this).release();
    ethDhcpController_ = std::make_unique<EthernetDhcpController>();
    ethDhcpController_->RegisterDhcpCallback(ethDhcpNotifyCallback_);

    ethConfiguration_ = std::make_unique<EthernetConfiguration>();
    ethConfiguration_->ReadSysteamConfiguration(devCaps_, devCfgs_);
}

EthernetManagement::~EthernetManagement() {}

void EthernetManagement::UpdateInterfaceState(const std::string &dev, bool up, bool lowerUp)
{
    NETMGR_EXT_LOG_D("EthernetManagement UpdateInterfaceState dev[%{public}s] up[%{public}d] lowerUp[%{public}d]",
        dev.c_str(), up, lowerUp);
    std::unique_lock<std::mutex> lock(mutex_);
    auto fit = devs_.find(dev);
    if (fit == devs_.end()) {
        return;
    }
    sptr<DevInterfaceState> devState = fit->second;
    devState->SetLinkUp(up);
    devState->SetLowerUp(lowerUp);
    IPSetMode mode = devState->GetIPSetMode();
    bool dhcpReqState = devState->GetDhcpReqState();
    NETMGR_EXT_LOG_D("EthernetManagement UpdateInterfaceState mode[%{public}d] dhcpReqState[%{public}d]",
        static_cast<int32_t>(mode), dhcpReqState);
    if (lowerUp) {
        devState->RemoteUpdateNetSupplierInfo();
        if (mode == DHCP && !dhcpReqState) {
            StartDhcpClient(dev, devState);
        } else {
            devState->RemoteUpdateNetLinkInfo();
        }
    } else {
        if (mode == DHCP && dhcpReqState) {
            StopDhcpClient(dev, devState);
        }
        devState->RemoteUpdateNetSupplierInfo();
    }
}

int32_t EthernetManagement::UpdateDevInterfaceState(const std::string &iface, sptr<InterfaceConfiguration> cfg)
{
    std::unique_lock<std::mutex> lock(mutex_);
    auto fit = devs_.find(iface);
    if (fit == devs_.end() || fit->second == nullptr) {
        NETMGR_EXT_LOG_E("The iface[%{public}s] device or device information does not exist", iface.c_str());
        return ETHERNET_ERROR;
    }
    if (!fit->second->GetLinkUp()) {
        return ETHERNET_ERROR;
    }
    if (!ethConfiguration_->WriteUserConfiguration(iface, cfg)) {
        NETMGR_EXT_LOG_E("EthernetManagement write user configurations error!");
        return ETHERNET_ERROR;
    }
    fit->second->SetIfcfg(cfg);
    return ETHERNET_SUCCESS;
}

int32_t EthernetManagement::UpdateDevInterfaceLinkInfo(EthernetDhcpCallback::DhcpResult &dhcpResult)
{
    NETMGR_EXT_LOG_D("EthernetManagement::UpdateDevInterfaceLinkInfo");
    auto fit = devs_.find(dhcpResult.iface);
    if (fit == devs_.end() || fit->second == nullptr) {
        NETMGR_EXT_LOG_E("The iface[%{public}s] device or device information does not exist",
            dhcpResult.iface.c_str());
        return ETHERNET_ERROR;
    }
    if (!fit->second->GetLinkUp()) {
        NETMGR_EXT_LOG_E("The iface[%{public}s] The device is not turned on", dhcpResult.iface.c_str());
        return ETHERNET_ERROR;
    }
    sptr<StaticConfiguration> config = std::make_unique<StaticConfiguration>().release();
    if (!ethConfiguration_->ConvertToConfiguration(dhcpResult, config)) {
        NETMGR_EXT_LOG_E("EthernetManagement dhcp convert to configurations error!");
        return ETHERNET_ERROR;
    }
    fit->second->UpdateLinkInfo(config->ipAddr_, config->gateway_, config->route_,
        config->dnsServers_.front(), config->dnsServers_.back());
    fit->second->RemoteUpdateNetLinkInfo();
    return ETHERNET_SUCCESS;
}

sptr<InterfaceConfiguration> EthernetManagement::GetDevInterfaceCfg(const std::string &iface)
{
    std::unique_lock<std::mutex> lock(mutex_);
    auto fit = devs_.find(iface);
    if (fit == devs_.end() || fit->second == nullptr) {
        NETMGR_EXT_LOG_E("The iface[%{public}s] device does not exist", iface.c_str());
        return nullptr;
    }
    return fit->second->GetIfcfg();
}

int32_t EthernetManagement::IsIfaceActive(const std::string &iface)
{
    std::unique_lock<std::mutex> lock(mutex_);
    auto fit = devs_.find(iface);
    if (fit == devs_.end() || fit->second == nullptr) {
        NETMGR_EXT_LOG_E("The iface[%{public}s] device does not exist", iface.c_str());
        return ETHERNET_ERROR;
    }
    return static_cast<int32_t>(fit->second->IsStateUp());
}

std::vector<std::string> EthernetManagement::GetAllActiveIfaces()
{
    std::unique_lock<std::mutex> lock(mutex_);
    std::vector<std::string> ifaces;
    for (auto it = devs_.begin(); it != devs_.end(); ++it) {
        if (it->second->IsStateUp()) {
            ifaces.push_back(it->first);
        }
    }
    return ifaces;
}

int32_t EthernetManagement::ResetFactory()
{
    if (!ethConfiguration_->ClearAllUserConfiguration()) {
        NETMGR_EXT_LOG_E("Failed to ResetFactory!");
        return ETHERNET_ERROR;
    } else {
        NETMGR_EXT_LOG_I("Success to ResetFactory!");
        return ETHERNET_SUCCESS;
    }
}

void EthernetManagement::RegisterNlk(NetLinkRtnl &nlk)
{
    nlk.RegisterHandle(this);
}

void EthernetManagement::Handle(const struct NlkEventInfo &info)
{
    bool up = static_cast<bool>(info.ifiFlags_ & IFF_UP);
    bool lowerUp = static_cast<bool>(info.ifiFlags_ & IFF_LOWER_UP);
    NETMGR_EXT_LOG_D("EthernetManagement Handle info dev[%{public}s] up[%{public}d] lowerUp[%{public}d]",
        info.iface_.c_str(), up, lowerUp);
    UpdateInterfaceState(info.iface_, up, lowerUp);
}

void EthernetManagement::Init()
{
    std::regex re(IFACE_MATCH);
    std::vector<std::string> ifaces = NetsysController::GetInstance().InterfaceGetList();
    if (ifaces.empty()) {
        NETMGR_EXT_LOG_E("EthernetManagement link list is empty");
        return;
    }
    NETMGR_EXT_LOG_D("EthernetManagement devs size[%{public}zd]", ifaces.size());
    if (!ethConfiguration_->ReadUserConfiguration(devCfgs_)) {
        NETMGR_EXT_LOG_E("EthernetManagement read user configurations error! ");
        return;
    }
    for (const auto &devName : ifaces) {
        NETMGR_EXT_LOG_D("EthernetManagement devName[%{public}s]", devName.c_str());
        if (!std::regex_search(devName, re)) {
            continue;
        }
        sptr<DevInterfaceState> devState = std::make_unique<DevInterfaceState>().release();
        devs_.insert(std::make_pair(devName, devState));
        std::vector<uint8_t> hwAddr = NetLinkRtnl::GetHWaddr(devName);
        SetDevState(devState, devName, hwAddr, false, false);
        devState->RemoteRegisterNetSupplier();
        auto fitCfg = devCfgs_.find(devName);
        if (fitCfg != devCfgs_.end()) {
            devState->SetIfcfg(fitCfg->second);
        } else {
            sptr<InterfaceConfiguration> ifCfg = std::make_unique<InterfaceConfiguration>().release();
            ifCfg->mode_ = DHCP;
            devState->SetIfcfg(ifCfg);
        }
        auto fitCap = devCaps_.find(devName);
        if (fitCap != devCaps_.end()) {
            devState->SetNetCaps(fitCap->second);
        }
    }
    std::thread t(&EthernetManagement::StartSetDevUpThd, this);
    t.detach();
    NETMGR_EXT_LOG_D("EthernetManagement devs_ size[%{public}zd", devs_.size());
}

void EthernetManagement::StartSetDevUpThd()
{
    NETMGR_EXT_LOG_D("EthernetManagement StartSetDevUpThd in.");
    for (auto & dev : devs_) {
        std::string devName = dev.first;
        while (true) {
            if (NetsysController::GetInstance().SetInterfaceUp(devName) != ERR_NONE) {
                sleep(2);
                continue;
            }
            break;
        }
    }
}

void EthernetManagement::StartDhcpClient(const std::string &dev, sptr<DevInterfaceState> &devState)
{
    NETMGR_EXT_LOG_D("EthernetManagement StartDhcpClient[%{public}s]", dev.c_str());
    ethDhcpController_->StartDhcpClient(dev, false);
    devState->SetDhcpReqState(true);
}

void EthernetManagement::StopDhcpClient(const std::string &dev, sptr<DevInterfaceState> &devState)
{
    NETMGR_EXT_LOG_D("EthernetManagement StopDhcpClient[%{public}s]", dev.c_str());
    ethDhcpController_->StopDhcpClient(dev, false);
    devState->SetDhcpReqState(false);
}

void EthernetManagement::SetDevState(sptr<DevInterfaceState> &devState, const std::string &devName,
    const std::vector<uint8_t> &hwAddr, bool up, bool lowerUp)
{
    devState->SetDevName(devName);
    devState->SetDevHWaddr(hwAddr);
    devState->SetLinkUp(up);
    devState->SetLowerUp(lowerUp);
}
} // namespace NetManagerStandard
} // namespace OHOS