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

#include "ethernet_management.h"

#include <regex>
#include <thread>
#include <pthread.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <vector>

#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "netsys_controller.h"
#include "securec.h"

namespace OHOS {
namespace NetManagerStandard {
const std::string IFACE_MATCH = "eth\\d";
constexpr const char *IFACE_LINK_UP = "up";
constexpr const char *IFACE_RUNNING = "running";
constexpr int SLEEP_TIME_S = 2;
constexpr uint32_t INDEX_ONE = 1;
constexpr uint32_t INDEX_TWO = 2;
constexpr uint32_t INDEX_THREE = 3;
constexpr uint32_t INDEX_FOUR = 4;
constexpr uint32_t INDEX_FIVE = 5;
constexpr uint32_t BUFFER_SIZE = 64;
int32_t EthernetManagement::EhternetDhcpNotifyCallback::OnDhcpSuccess(EthernetDhcpCallback::DhcpResult &dhcpResult)
{
    ethernetManagement_.UpdateDevInterfaceLinkInfo(dhcpResult);
    return 0;
}

int32_t EthernetManagement::DevInterfaceStateCallback::OnInterfaceAddressUpdated(const std::string &,
                                                                                 const std::string &, int, int)
{
    return 0;
}

int32_t EthernetManagement::DevInterfaceStateCallback::OnInterfaceAddressRemoved(const std::string &,
                                                                                 const std::string &, int, int)
{
    return 0;
}

int32_t EthernetManagement::DevInterfaceStateCallback::OnInterfaceAdded(const std::string &iface)
{
    std::regex re(IFACE_MATCH);
    if (std::regex_search(iface, re)) {
        ethernetManagement_.DevInterfaceAdd(iface);
        if (NetsysController::GetInstance().SetInterfaceUp(iface) != ERR_NONE) {
            NETMGR_EXT_LOG_E("Iface[%{public}s] added set up fail!", iface.c_str());
        }
    }
    return 0;
}

int32_t EthernetManagement::DevInterfaceStateCallback::OnInterfaceRemoved(const std::string &iface)
{
    std::regex re(IFACE_MATCH);
    if (std::regex_search(iface, re)) {
        ethernetManagement_.DevInterfaceRemove(iface);
        if (NetsysController::GetInstance().SetInterfaceDown(iface) != ERR_NONE) {
            NETMGR_EXT_LOG_E("Iface[%{public}s] added set down fail!", iface.c_str());
        }
    }
    return 0;
}

int32_t EthernetManagement::DevInterfaceStateCallback::OnInterfaceChanged(const std::string &, bool)
{
    return 0;
}

int32_t EthernetManagement::DevInterfaceStateCallback::OnInterfaceLinkStateChanged(const std::string &ifName, bool up)
{
    auto startTime = std::chrono::steady_clock::now();
    std::regex re(IFACE_MATCH);
    if (std::regex_search(ifName, re)) {
        ethernetManagement_.UpdateInterfaceState(ifName, up);
    }
    auto endTime = std::chrono::steady_clock::now();
    auto durationNs = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime);
    NETMGR_EXT_LOG_I("OnInterfaceLinkStateChanged iface[%{public}s] up[%{public}d], cost=%{public}lld",
        ifName.c_str(), up, durationNs.count());
    return 0;
}

int32_t EthernetManagement::DevInterfaceStateCallback::OnRouteChanged(bool, const std::string &, const std::string &,
                                                                      const std::string &)
{
    return 0;
}

int32_t EthernetManagement::DevInterfaceStateCallback::OnDhcpSuccess(NetsysControllerCallback::DhcpResult &dhcpResult)
{
    return 0;
}

int32_t EthernetManagement::DevInterfaceStateCallback::OnBandwidthReachedLimit(const std::string &limitName,
                                                                               const std::string &iface)
{
    return 0;
}

EthernetManagement &EthernetManagement::GetInstance()
{
    static EthernetManagement gInstance;
    return gInstance;
}

EthernetManagement::EthernetManagement()
{
    ethDhcpController_ = std::make_unique<EthernetDhcpController>();
    ethDhcpNotifyCallback_ = new (std::nothrow) EhternetDhcpNotifyCallback(*this);
    if (ethDhcpNotifyCallback_ != nullptr) {
        ethDhcpController_->RegisterDhcpCallback(ethDhcpNotifyCallback_);
    }

    ethDevInterfaceStateCallback_ = new (std::nothrow) DevInterfaceStateCallback(*this);
    ethConfiguration_ = std::make_unique<EthernetConfiguration>();
    ethConfiguration_->ReadSystemConfiguration(devCaps_, devCfgs_);
    ethLanManageMent_ = std::make_unique<EthernetLanManagement>();
}

EthernetManagement::~EthernetManagement() = default;

void EthernetManagement::UpdateInterfaceState(const std::string &dev, bool up)
{
    NETMGR_EXT_LOG_D("EthernetManagement UpdateInterfaceState dev[%{public}s] up[%{public}d]", dev.c_str(), up);
    sptr<DevInterfaceState> devState = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        auto fit = devs_.find(dev);
        if (fit == devs_.end()) {
            return;
        }
        devState = fit->second;
    }
    if (devState == nullptr) {
        NETMGR_EXT_LOG_E("devState is nullptr");
        return;
    }
    devState->SetLinkUp(up);
    IPSetMode mode = devState->GetIPSetMode();
    bool dhcpReqState = devState->GetDhcpReqState();
    NETMGR_EXT_LOG_D("EthernetManagement UpdateInterfaceState mode[%{public}d] dhcpReqState[%{public}d]",
                     static_cast<int32_t>(mode), dhcpReqState);
    if (up) {
        if (!devState->IsLanIface()) {
            devState->RemoteUpdateNetSupplierInfo();
        }
        if ((mode == DHCP || mode == LAN_DHCP) && !dhcpReqState) {
            StartDhcpClient(dev, devState);
        } else {
            if (devState->IsLanIface()) {
                ethLanManageMent_->UpdateLanLinkInfo(devState);
            } else {
                devState->RemoteUpdateNetLinkInfo();
            }
        }
    } else {
        if ((mode == DHCP || mode == LAN_DHCP) && dhcpReqState) {
            StopDhcpClient(dev, devState);
        }
        if (devState->IsLanIface()) {
            ethLanManageMent_->ReleaseLanNetLink(devState);
        } else {
            devState->RemoteUpdateNetSupplierInfo();
        }
        netLinkConfigs_[dev] = nullptr;
    }
}

int32_t EthernetManagement::GetMacAddress(std::vector<MacAddressInfo> &macAddrList)
{
    std::regex re(IFACE_MATCH);
    std::vector<std::string> ifaceLists = NetsysController::GetInstance().InterfaceGetList();
    if (ifaceLists.empty()) {
        NETMGR_EXT_LOG_E("EthernetManagement iface list is empty");
        return ETHERNET_ERR_DEVICE_INFORMATION_NOT_EXIST;
    }
    for (const auto &iface : ifaceLists) {
        if (std::regex_search(iface, re)) {
            MacAddressInfo macAddressInfo;
            auto spMacAddr = GetMacAddr(iface);
            if (spMacAddr.c_str() == nullptr) {
                NETMGR_EXT_LOG_E("The iface[%{public}s] device does not find MAC address", iface.c_str());
                continue;
            }
            macAddressInfo.iface_ = iface;
            macAddressInfo.macAddress_ = spMacAddr;
            macAddrList.push_back(macAddressInfo);
        }
    }
    if (macAddrList.size() == 0) {
        NETMGR_EXT_LOG_E("EthernetManagement mac address list is empty");
        return ETHERNET_ERR_DEVICE_INFORMATION_NOT_EXIST;
    }
    return NETMANAGER_EXT_SUCCESS;
}

std::string EthernetManagement::GetMacAddr(const std::string &iface)
{
    NETMGR_EXT_LOG_D("GetMacAddr when iface is [%{public}s]", iface.c_str());
    std::string macAddr;

    int fd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    struct ifreq ifr = {};
    strncpy_s(ifr.ifr_name, IFNAMSIZ, iface.c_str(), iface.length());
    
    if (ioctl(fd, SIOCGIFHWADDR, &ifr) != -1) {
        macAddr = HwAddrToStr(ifr.ifr_hwaddr.sa_data);
    }
    close(fd);
    return macAddr;
}

std::string EthernetManagement::HwAddrToStr(char *hwaddr)
{
    char buf[BUFFER_SIZE] = {'\0'};
    if (hwaddr == nullptr) {
        NETMGR_EXT_LOG_E("hwaddr is nullptr");
        return "";
    }
    errno_t result =
        sprintf_s(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x", hwaddr[0], hwaddr[INDEX_ONE],
            hwaddr[INDEX_TWO], hwaddr[INDEX_THREE], hwaddr[INDEX_FOUR],
            hwaddr[INDEX_FIVE]);
    if (result < 0) {
        NETMGR_EXT_LOG_E("[hwAddrToStr] result error : [%{public}d]", result);
        return "";
    }
    return std::string(buf);
}

int32_t EthernetManagement::UpdateDevInterfaceCfg(const std::string &iface, sptr<InterfaceConfiguration> cfg)
{
    if (cfg == nullptr) {
        NETMGR_EXT_LOG_E("cfg is nullptr");
        return NETMANAGER_EXT_ERR_LOCAL_PTR_NULL;
    }
    sptr<DevInterfaceState> devState = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        auto fit = devs_.find(iface);
        if (fit == devs_.end() || fit->second == nullptr) {
            NETMGR_EXT_LOG_E("The iface[%{public}s] device or device information does not exist", iface.c_str());
            return ETHERNET_ERR_DEVICE_INFORMATION_NOT_EXIST;
        }
        devState = fit->second;
    }
    if (!devState->GetLinkUp()) {
        NETMGR_EXT_LOG_E("The iface[%{public}s] device is unlink", iface.c_str());
        return ETHERNET_ERR_DEVICE_NOT_LINK;
    }
    if (!ModeInputCheck(devState->GetIfcfg()->mode_, cfg->mode_)) {
        NETMGR_EXT_LOG_E("The iface[%{public}s] device can not exchange between WAN and LAN", iface.c_str());
        return NETMANAGER_ERR_INVALID_PARAMETER;
    }
    if (!ethConfiguration_->WriteUserConfiguration(iface, cfg)) {
        NETMGR_EXT_LOG_E("EthernetManagement write user configurations error!");
        return ETHERNET_ERR_USER_CONIFGURATION_WRITE_FAIL;
    }
    if (devState->GetIfcfg()->mode_ != cfg->mode_) {
        if (cfg->mode_ == DHCP || cfg->mode_ == LAN_DHCP) {
            StartDhcpClient(iface, devState);
        } else {
            StopDhcpClient(iface, devState);
            netLinkConfigs_[iface] = nullptr;
        }
    } else if (cfg->mode_ == DHCP) {
        devState->UpdateNetHttpProxy(cfg->httpProxy_);
    }
    if (devState->IsLanIface()) {
        ethLanManageMent_->GetOldLinkInfo(devState);
        devState->SetLancfg(cfg);
        ethLanManageMent_->UpdateLanLinkInfo(devState);
    } else {
        devState->SetIfcfg(cfg);
    }
    devCfgs_[iface] = cfg;
    return NETMANAGER_EXT_SUCCESS;
}

int32_t EthernetManagement::UpdateDevInterfaceLinkInfo(EthernetDhcpCallback::DhcpResult &dhcpResult)
{
    NETMGR_EXT_LOG_D("EthernetManagement::UpdateDevInterfaceLinkInfo");
    std::lock_guard<std::mutex> locker(mutex_);
    auto fit = devs_.find(dhcpResult.iface);
    if (fit == devs_.end() || fit->second == nullptr) {
        NETMGR_EXT_LOG_E("The iface[%{public}s] device or device information does not exist", dhcpResult.iface.c_str());
        return ETHERNET_ERR_DEVICE_INFORMATION_NOT_EXIST;
    }
    if (!fit->second->GetLinkUp()) {
        NETMGR_EXT_LOG_E("The iface[%{public}s] The device is not turned on", dhcpResult.iface.c_str());
        return ETHERNET_ERR_DEVICE_NOT_LINK;
    }

    IPSetMode mode = fit->second->GetIPSetMode();
    if (mode == IPSetMode::STATIC || mode == IPSetMode::LAN_STATIC) {
        NETMGR_EXT_LOG_E("The iface[%{public}s] set mode is STATIC now", dhcpResult.iface.c_str());
        return ETHERNET_ERR_DEVICE_NOT_LINK;
    }

    auto &config = netLinkConfigs_[dhcpResult.iface];
    if (config == nullptr) {
        config = new (std::nothrow) StaticConfiguration();
        if (config == nullptr) {
            NETMGR_EXT_LOG_E("Iface:%{public}s's link info config is nullptr", dhcpResult.iface.c_str());
        }
    }

    if (!ethConfiguration_->ConvertToConfiguration(dhcpResult, config)) {
        NETMGR_EXT_LOG_E("EthernetManagement dhcp convert to configurations error!");
        return ETHERNET_ERR_CONVERT_CONFIGURATINO_FAIL;
    }
    if (fit->second->IsLanIface()) {
        ethLanManageMent_->GetOldLinkInfo(fit->second);
        fit->second->UpdateLanLinkInfo(config);
        ethLanManageMent_->UpdateLanLinkInfo(fit->second);
    } else {
        fit->second->UpdateLinkInfo(config);
        fit->second->RemoteUpdateNetLinkInfo();
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t EthernetManagement::GetDevInterfaceCfg(const std::string &iface, sptr<InterfaceConfiguration> &ifaceConfig)
{
    std::unique_lock<std::mutex> lock(mutex_);
    auto fit = devs_.find(iface);
    if (fit == devs_.end() || fit->second == nullptr) {
        NETMGR_EXT_LOG_E("The iface[%{public}s] device does not exist", iface.c_str());
        return ETHERNET_ERR_DEVICE_INFORMATION_NOT_EXIST;
    }
    if (!fit->second->GetLinkUp()) {
        ifaceConfig = fit->second->GetIfcfg();
        return NETMANAGER_EXT_SUCCESS;
    }
    auto temp = ethConfiguration_->MakeInterfaceConfiguration(fit->second->GetIfcfg(), fit->second->GetLinkInfo());
    if (temp == nullptr) {
        return ETHERNET_ERR_DEVICE_INFORMATION_NOT_EXIST;
    }
    *ifaceConfig = *temp;
    return NETMANAGER_EXT_SUCCESS;
}

int32_t EthernetManagement::IsIfaceActive(const std::string &iface, int32_t &activeStatus)
{
    std::unique_lock<std::mutex> lock(mutex_);
    auto fit = devs_.find(iface);
    if (fit == devs_.end() || fit->second == nullptr) {
        NETMGR_EXT_LOG_E("The iface[%{public}s] device does not exist", iface.c_str());
        return ETHERNET_ERR_DEVICE_INFORMATION_NOT_EXIST;
    }
    activeStatus = static_cast<int32_t>(fit->second->GetLinkUp());
    return NETMANAGER_EXT_SUCCESS;
}

int32_t EthernetManagement::GetAllActiveIfaces(std::vector<std::string> &activeIfaces)
{
    std::unique_lock<std::mutex> lock(mutex_);
    for (auto it = devs_.begin(); it != devs_.end(); ++it) {
        if (it->second->GetLinkUp()) {
            activeIfaces.push_back(it->first);
        }
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t EthernetManagement::ResetFactory()
{
    if (!ethConfiguration_->ClearAllUserConfiguration()) {
        NETMGR_EXT_LOG_E("Failed to ResetFactory!");
        return ETHERNET_ERR_USER_CONIFGURATION_CLEAR_FAIL;
    }
    NETMGR_EXT_LOG_I("Success to ResetFactory!");
    return NETMANAGER_EXT_SUCCESS;
}

void EthernetManagement::Init()
{
    static const unsigned int SLEEP_TIME = 4;
    std::this_thread::sleep_for(std::chrono::seconds(SLEEP_TIME));
    if (ethDevInterfaceStateCallback_ != nullptr) {
        NetsysController::GetInstance().RegisterCallback(ethDevInterfaceStateCallback_);
    }
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
        DevInterfaceAdd(devName);
    }
    std::thread t(&EthernetManagement::StartSetDevUpThd, &EthernetManagement::GetInstance());
    std::string threadName = "SetDevUpThd";
    pthread_setname_np(t.native_handle(), threadName.c_str());
    t.detach();
    NETMGR_EXT_LOG_D("EthernetManagement devs_ size[%{public}zd", devs_.size());
}

void EthernetManagement::StartSetDevUpThd()
{
    NETMGR_EXT_LOG_D("EthernetManagement StartSetDevUpThd in.");
    std::map<std::string, sptr<DevInterfaceState>> tempDevMap;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        tempDevMap = devs_;
    }

    for (auto &dev : tempDevMap) {
        std::string devName = dev.first;
        if (IsIfaceLinkUp(devName)) {
            continue;
        }
        while (true) {
            if (NetsysController::GetInstance().SetInterfaceUp(devName) != ERR_NONE) {
                sleep(SLEEP_TIME_S);
                continue;
            }
            break;
        }
    }
}

bool EthernetManagement::IsIfaceLinkUp(const std::string &iface)
{
    OHOS::nmd::InterfaceConfigurationParcel config;
    config.ifName = iface;
    if (NetsysController::GetInstance().GetInterfaceConfig(config) != ERR_NONE) {
        return false;
    }
    if (std::find(config.flags.begin(), config.flags.end(), IFACE_LINK_UP) == config.flags.end() ||
        std::find(config.flags.begin(), config.flags.end(), IFACE_RUNNING) == config.flags.end()) {
        return false;
    }
    UpdateInterfaceState(iface, true);
    return true;
}

void EthernetManagement::StartDhcpClient(const std::string &dev, sptr<DevInterfaceState> &devState)
{
    NETMGR_EXT_LOG_D("EthernetManagement StartDhcpClient[%{public}s]", dev.c_str());
    ethDhcpController_->StartClient(dev, true);
    devState->SetDhcpReqState(true);
}

void EthernetManagement::StopDhcpClient(const std::string &dev, sptr<DevInterfaceState> &devState)
{
    NETMGR_EXT_LOG_D("EthernetManagement StopDhcpClient[%{public}s]", dev.c_str());
    ethDhcpController_->StopClient(dev, true);
    devState->SetDhcpReqState(false);
}

void EthernetManagement::DevInterfaceAdd(const std::string &devName)
{
    NETMGR_EXT_LOG_D("Interface name:[%{public}s] add.", devName.c_str());
    std::unique_lock<std::mutex> lock(mutex_);
    auto fitDev = devs_.find(devName);
    if (fitDev != devs_.end()) {
        NETMGR_EXT_LOG_E("Interface name:[%{public}s] has added.", devName.c_str());
        return;
    }
    sptr<DevInterfaceState> devState = new (std::nothrow) DevInterfaceState();
    if (devState == nullptr) {
        NETMGR_EXT_LOG_E("devState is nullptr");
        return;
    }
    ethConfiguration_->ReadSystemConfiguration(devCaps_, devCfgs_);
    devs_.insert(std::make_pair(devName, devState));
    devState->SetDevName(devName);
    auto fitCfg = devCfgs_.find(devName);
    if (fitCfg != devCfgs_.end()) {
        if (fitCfg->second->mode_ == LAN_STATIC || fitCfg->second->mode_ == LAN_DHCP) {
            NETMGR_EXT_LOG_D("Lan Interface name:[%{public}s] add, mode [%{public}d]",
                             devName.c_str(), fitCfg->second->mode_);
            devState->SetLancfg(fitCfg->second);
            ethLanManageMent_->UpdateLanLinkInfo(devState);
            return;
        }
        devState->RemoteRegisterNetSupplier();
        devState->SetIfcfg(fitCfg->second);
    } else {
        sptr<InterfaceConfiguration> ifCfg = new (std::nothrow) InterfaceConfiguration();
        if (ifCfg == nullptr) {
            NETMGR_EXT_LOG_E("ifCfg is nullptr");
            return;
        }
        ifCfg->mode_ = DHCP;
        devState->RemoteRegisterNetSupplier();
        devState->SetIfcfg(ifCfg);
    }
    auto fitCap = devCaps_.find(devName);
    if (fitCap != devCaps_.end()) {
        devState->SetNetCaps(fitCap->second);
    }
}

void EthernetManagement::DevInterfaceRemove(const std::string &devName)
{
    NETMGR_EXT_LOG_D("Interface name:[%{public}s] remove.", devName.c_str());
    std::unique_lock<std::mutex> lock(mutex_);
    auto fitDev = devs_.find(devName);
    if (fitDev != devs_.end()) {
        if (fitDev->second != nullptr) {
            fitDev->second->RemoteUnregisterNetSupplier();
        }
        devs_.erase(fitDev);
    }
}

void EthernetManagement::GetDumpInfo(std::string &info)
{
    std::for_each(devs_.begin(), devs_.end(), [&info](const auto &dev) { dev.second->GetDumpInfo(info); });
}

bool EthernetManagement::ModeInputCheck(IPSetMode origin, IPSetMode input)
{
    if (origin == STATIC || origin == DHCP) {
        if (input == LAN_STATIC || input == LAN_DHCP) {
            return false;
        }
    } else if (origin == LAN_STATIC || origin == LAN_DHCP) {
        if (input == STATIC || input == DHCP) {
            return false;
        }
    }
    return true;
}
} // namespace NetManagerStandard
} // namespace OHOS
