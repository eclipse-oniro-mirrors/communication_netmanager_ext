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

#include "ethernet_management.h"

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include <fstream>
#include <sstream>

#include "securec.h"
#include "netsys_controller.h"
#include "netmgr_ext_log_wrapper.h"
#include "ethernet_constants.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t BIT32 = 32;
constexpr int32_t BIT24 = 24;
constexpr int32_t BIT16 = 16;
constexpr int32_t BIT8 = 8;
constexpr int32_t INET_PTION_SUC = 1;
constexpr int32_t MKDIR_ERR = -1;
EthernetManagement::NotifyCallback::NotifyCallback(EthernetManagement &ethernetManagement)
    : ethernetManagement_(ethernetManagement)
{
}

EthernetManagement::NotifyCallback::~NotifyCallback() {}

int32_t EthernetManagement::NotifyCallback::OnInterfaceAddressUpdated(const std::string &, const std::string &,
    int, int)
{
    return 0;
}

int32_t EthernetManagement::NotifyCallback::OnInterfaceAddressRemoved(const std::string &, const std::string &,
    int, int)
{
    return 0;
}

int32_t EthernetManagement::NotifyCallback::OnInterfaceAdded(const std::string &)
{
    return 0;
}

int32_t EthernetManagement::NotifyCallback::OnInterfaceRemoved(const std::string &)
{
    return 0;
}

int32_t EthernetManagement::NotifyCallback::OnInterfaceChanged(const std::string &, bool)
{
    return 0;
}

int32_t EthernetManagement::NotifyCallback::OnInterfaceLinkStateChanged(const std::string &, bool)
{
    return 0;
}

int32_t EthernetManagement::NotifyCallback::OnRouteChanged(bool, const std::string &, const std::string &,
    const std::string &)
{
    return 0;
}

int32_t EthernetManagement::NotifyCallback::OnDhcpSuccess(NetsysControllerCallback::DhcpResult &dhcpResult)
{
    NETMGR_EXT_LOG_D("EthernetManagement::NativeNotifyCallback::OnDhcpSuccess");
    ethernetManagement_.UpdateDevInterfaceLinkInfo(dhcpResult);
    return 0;
}

EthernetManagement::EthernetManagement()
{
    notifyCallback_ = std::make_unique<NotifyCallback>(*this).release();
    NetsysController::GetInstance().RegisterCallback(notifyCallback_);
    if (!IsDirExist(configDir_)) {
        NETMGR_EXT_LOG_D("CreateDir start");
        bool ret = CreateDir(configDir_);
        NETMGR_EXT_LOG_D("CreateDir end ret[%{public}d]", ret);
    }
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
    sptr<DevInterfaceState> devState= fit->second;
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
    if (cfg->mode_ == STATIC) {
        uint32_t prefixlen = 0;
        cfg->ipStatic_.ipAddr_.family_ = GetAddrFamily(cfg->ipStatic_.ipAddr_.address_);
        if (cfg->ipStatic_.ipAddr_.family_ == AF_INET) {
            if (cfg->ipStatic_.netMask_.address_.empty()) {
                prefixlen = Ipv4PrefixLen(cfg->ipStatic_.ipAddr_.netMask_);
            } else {
                prefixlen = Ipv4PrefixLen(cfg->ipStatic_.netMask_.address_);
            }
        }
        cfg->ipStatic_.ipAddr_.prefixlen_ = prefixlen;
        cfg->ipStatic_.gateway_.family_ = GetAddrFamily(cfg->ipStatic_.gateway_.address_);
        cfg->ipStatic_.gateway_.prefixlen_ = prefixlen;
        cfg->ipStatic_.route_.family_ = GetAddrFamily(cfg->ipStatic_.route_.address_);
        cfg->ipStatic_.route_.prefixlen_ = prefixlen;
    }
    std::string fileContent;
    std::string filePath = configDir_ + "/" + iface;
    GenCfgContent(iface, cfg, fileContent);
    WriteFile(filePath, fileContent);
    fit->second->SetIfcfg(cfg);
    return ETHERNET_SUCCESS;
}

int32_t EthernetManagement::UpdateDevInterfaceLinkInfo(NetsysControllerCallback::DhcpResult &dhcpResult)
{
    NETMGR_EXT_LOG_D("EthernetManagement::UpdateDevInterfaceLinkInfo");
    auto fit = devs_.find(dhcpResult.iface_);
    if (fit == devs_.end() || fit->second == nullptr) {
        NETMGR_EXT_LOG_E("The iface[%{public}s] device or device information does not exist",
            dhcpResult.iface_.c_str());
        return ETHERNET_ERROR;
    }
    if (!fit->second->GetLinkUp()) {
        NETMGR_EXT_LOG_E("The iface[%{public}s] The device is not turned on", dhcpResult.iface_.c_str());
        return ETHERNET_ERROR;
    }

    int32_t prefixlen = 0;
    INetAddr addr;
    addr.address_ = dhcpResult.ipAddr_;
    addr.family_ = GetAddrFamily(dhcpResult.ipAddr_);
    if (addr.family_ == AF_INET) {
        addr.prefixlen_ = Ipv4PrefixLen(dhcpResult.subNet_);
    }
    prefixlen = addr.prefixlen_;
    INetAddr gate;
    gate.address_ = dhcpResult.gateWay_;
    gate.family_ = GetAddrFamily(dhcpResult.gateWay_);
    gate.prefixlen_ = prefixlen;
    INetAddr route;
    if (dhcpResult.gateWay_ != dhcpResult.route1_) {
        if (dhcpResult.route1_ == "*") {
            route.address_ = "0.0.0.0";
        } else {
            route.address_ = dhcpResult.route1_;
        }
    }
    if (dhcpResult.gateWay_ != dhcpResult.route2_) {
        if (dhcpResult.route2_ == "*") {
            route.address_ = "0.0.0.0";
        } else {
            route.address_ = dhcpResult.route2_;
        }
    }
    route.family_ = GetAddrFamily(route.address_);
    route.prefixlen_ = prefixlen;
    INetAddr dnsNet1;
    dnsNet1.address_ = dhcpResult.dns1_;
    INetAddr dnsNet2;
    dnsNet2.address_ = dhcpResult.dns2_;
    fit->second->UpdateLinkInfo(addr, gate, route, dnsNet1, dnsNet2);
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
    return static_cast<int32_t>(fit->second->GetLinkUp());
}

std::vector<std::string> EthernetManagement::GetAllActiveIfaces()
{
    std::unique_lock<std::mutex> lock(mutex_);
    std::vector<std::string> a;
    for (auto it = devs_.begin(); it != devs_.end(); ++it) {
        a.push_back(it->first);
    }
    return a;
}

int32_t EthernetManagement::ResetFactory()
{
    if (!DelDir(configDir_)) {
        NETMGR_EXT_LOG_E("Failed to Delete a dir[%{public}s]", configDir_.c_str());
        return ETHERNET_ERROR;
    } else {
        NETMGR_EXT_LOG_I("Success deleting a dir[%{public}s]", configDir_.c_str());
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
    std::vector<NlkEventInfo> linkInfos;
    NetLinkRtnl::GetLinkInfo(linkInfos);
    if (linkInfos.size() == 0) {
        NETMGR_EXT_LOG_E("EthernetManagement link list is empty");
        return;
    }
    NETMGR_EXT_LOG_D("EthernetManagement devs size[%{public}zd]", linkInfos.size());
    std::map<std::string, sptr<InterfaceConfiguration>> devCfgs;
    ReadFileList(configDir_, devCfgs);
    std::map<std::string, sptr<InterfaceConfiguration>>::iterator fit;
    NETMGR_EXT_LOG_D("EthernetManagement ReadFileList size[%{public}zd]", devCfgs.size());
    for (auto it = linkInfos.begin(); it != linkInfos.end(); it++) {
        std::string devName = it->iface_;
        NETMGR_EXT_LOG_D("EthernetManagement devName[%{public}s]", devName.c_str());
        if (devName.empty()) {
            continue;
        }
        NetsysController::GetInstance().SetInterfaceUp(devName);
        sptr<DevInterfaceState> devState = std::make_unique<DevInterfaceState>().release();
        devs_.insert(std::make_pair(devName, devState));
        std::vector<uint8_t> hwAddr = NetLinkRtnl::GetHWaddr(devName);
        bool up = it->ifiFlags_ & IFF_UP;
        bool lowerUp = it->ifiFlags_ & IFF_LOWER_UP;
        SetDevState(devState, devName, hwAddr, up, lowerUp);
        devState->RemoteRegisterNetSupplier();
        if (up && lowerUp) {
            devState->RemoteUpdateNetSupplierInfo();
        }
        fit = devCfgs.find(devName);
        if (fit != devCfgs.end()) {
            devState->SetIfcfg(fit->second);
            if (fit->second->mode_ == DHCP) {
                StartDhcpClient(devName, devState);
            } else {
                devState->RemoteUpdateNetLinkInfo();
            }
        } else {
            sptr<InterfaceConfiguration> ifCfg = std::make_unique<InterfaceConfiguration>().release();
            ifCfg->mode_ = STATIC;
            devState->SetIfcfg(ifCfg);
        }
    }
    NETMGR_EXT_LOG_D("EthernetManagement devs_ size[%{public}zd", devs_.size());
}

bool EthernetManagement::IsDirExist(const std::string &dirPath)
{
    DIR *dp = nullptr;
    NETMGR_EXT_LOG_D("EthernetManagement::IsDirExist start");
    if ((dp = opendir(dirPath.c_str())) == nullptr) {
        NETMGR_EXT_LOG_E("EthernetManagement::IsDirExist open failed");
        return false;
    }
    NETMGR_EXT_LOG_D("EthernetManagement::IsDirExist open suc");
    closedir(dp);
    return true;
}

bool EthernetManagement::CreateDir(const std::string &dirPath)
{
    if (mkdir(dirPath.c_str(), 0755) == MKDIR_ERR) {
        return false;
    }
    return true;
}

bool EthernetManagement::DelDir(const std::string &dirPath)
{
    DIR *dir = nullptr;
    struct dirent *entry = nullptr;
    struct stat statbuf;
    if ((dir = opendir(dirPath.c_str())) == nullptr) {
        return false;
    }
    while ((entry = readdir(dir)) != nullptr) {
        std::string filePath = dirPath + entry->d_name;
        lstat(filePath.c_str(), &statbuf);
        if (S_ISREG(statbuf.st_mode)) {
            remove(filePath.c_str());
        }
    }
    closedir(dir);
    if (rmdir(dirPath.c_str()) < 0) {
        return false;
    }
    return true;
}

bool EthernetManagement::IsFileExist(const std::string& filePath)
{
    return !access(filePath.c_str(), F_OK);
}

bool EthernetManagement::ReadFile(const std::string &filePath, std::string &fileContent)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (filePath.empty()) {
        NETMGR_EXT_LOG_E("filePath empty.");
        return false;
    }
    if (!IsFileExist(filePath)) {
        NETMGR_EXT_LOG_E("[%{public}s] not exist.", filePath.c_str());
        return false;
    }
    std::fstream file(filePath.c_str(), std::fstream::in);
    if (file.is_open() == false) {
        NETMGR_EXT_LOG_E("fstream failed.");
        return false;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    fileContent = buffer.str();
    file.close();
    return true;
}

bool EthernetManagement::WriteFile(const std::string &filePath, const std::string &fileContent)
{
    std::fstream file(filePath.c_str(), std::fstream::out | std::fstream::trunc);
    if (file.is_open() == false) {
        NETMGR_EXT_LOG_E("fstream failed.");
        return false;
    }
    file << fileContent.c_str();
    file.close();
    return true;
}

void EthernetManagement::ReadFileList(const std::string &dirPath, std::map<std::string,
    sptr<InterfaceConfiguration>> &devCfgs)
{
    DIR *dir = nullptr;
    struct dirent *ptr = nullptr;
    if ((dir = opendir(dirPath.c_str())) == nullptr) {
        return;
    }
    std::string iface;
    sptr<InterfaceConfiguration> cfg = nullptr;
    while ((ptr = readdir(dir)) != nullptr) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        } else if (ptr->d_type == DT_REG) {
            std::string filePath = dirPath + "/" + ptr->d_name;
            std::string fileContent;
            if (!ReadFile(filePath, fileContent)) {
                continue;
            }
            std::string().swap(iface);
            cfg = std::make_unique<InterfaceConfiguration>().release();
            ParserFileConfig(fileContent, iface, cfg);
            if (!iface.empty()) {
                NETMGR_EXT_LOG_E("ReadFileList devname[%{public}s]", iface.c_str());
                devCfgs.insert(std::make_pair(iface, cfg));
            }
        }
    }
    closedir(dir);
    return;
}

void EthernetManagement::ParserFileConfig(const std::string &fileContent, std::string &iface,
    sptr<InterfaceConfiguration> cfg)
{
    std::string::size_type pos = fileContent.find("DEVICE=") + strlen("DEVICE=");
    std::string device = fileContent.substr(pos, fileContent.find("\n", pos) - pos);
    pos = fileContent.find("BOOTPROTO=") + strlen("BOOTPROTO=");
    std::string bootProto = fileContent.substr(pos, fileContent.find("\n", pos) - pos);
    iface = device;
    if (bootProto == "STATIC") {
        cfg->mode_ = STATIC;
        pos = fileContent.find("IPADDR=") + strlen("IPADDR=");
        std::string ipAddr = fileContent.substr(pos, fileContent.find("\n", pos) - pos);
        pos = fileContent.find("NETMASK=") + strlen("NETMASK=");
        std::string netMask = fileContent.substr(pos, fileContent.find("\n", pos) - pos);
        pos = fileContent.find("GATEWAY=") + strlen("GATEWAY=");
        std::string gatway = fileContent.substr(pos, fileContent.find("\n", pos) - pos);
        pos = fileContent.find("ROUTE=") + strlen("ROUTE=");
        std::string route = fileContent.substr(pos, fileContent.find("\n", pos) - pos);
        cfg->ipStatic_.ipAddr_.address_ = ipAddr;
        cfg->ipStatic_.ipAddr_.netMask_ = netMask;
        cfg->ipStatic_.ipAddr_.family_ = GetAddrFamily(ipAddr);
        int32_t prefixlen = Ipv4PrefixLen(netMask);
        if (cfg->ipStatic_.ipAddr_.family_ == AF_INET) {
            cfg->ipStatic_.ipAddr_.prefixlen_ = prefixlen;
        }
        cfg->ipStatic_.netMask_.address_ = netMask;
        cfg->ipStatic_.gateway_.address_ = gatway;
        cfg->ipStatic_.gateway_.family_ = GetAddrFamily(gatway);
        if (cfg->ipStatic_.gateway_.family_ == AF_INET) {
            cfg->ipStatic_.gateway_.prefixlen_ = prefixlen;
        }
        cfg->ipStatic_.route_.address_ = route;
        cfg->ipStatic_.route_.family_ = GetAddrFamily(route);
        if (cfg->ipStatic_.route_.family_ == AF_INET) {
            cfg->ipStatic_.route_.prefixlen_ = prefixlen;
        }
    } else if (bootProto == "DHCP") {
        cfg->mode_ = DHCP;
    }
}

void EthernetManagement::GenCfgContent(const std::string &iface, sptr<InterfaceConfiguration> cfg,
    std::string &fileContent)
{
    std::string().swap(fileContent);
    fileContent = fileContent + "DEVICE=" + iface + "\n";
    if (cfg->mode_ == STATIC) {
        fileContent = fileContent + "BOOTPROTO=STATIC\n";
        fileContent = fileContent + "IPADDR=" + cfg->ipStatic_.ipAddr_.address_ + "\n";
        if (cfg->ipStatic_.netMask_.address_.empty()) {
            fileContent = fileContent + "NETMASK=" + cfg->ipStatic_.ipAddr_.netMask_ + "\n";
        } else {
            fileContent = fileContent + "NETMASK=" + cfg->ipStatic_.netMask_.address_ + "\n";
        }
        fileContent = fileContent + "GATEWAY=" + cfg->ipStatic_.gateway_.address_ + "\n";
        fileContent = fileContent + "ROUTE=" + cfg->ipStatic_.route_.address_ + "\n";
    } else {
        fileContent = fileContent + "BOOTPROTO=DHCP\n";
    }
}

bool EthernetManagement::IsValidIPV4(const std::string &ip)
{
    if (ip.empty()) {
        return false;
    }
    struct in_addr s;
    int32_t result = inet_pton(AF_INET, ip.c_str(), reinterpret_cast<void*>(&s));
    if (result == INET_PTION_SUC) {
        return true;
    }
    return false;
}

bool EthernetManagement::IsValidIPV6(const std::string &ip)
{
    if (ip.empty()) {
        return false;
    }
    struct in6_addr s;
    int32_t result = inet_pton(AF_INET6, ip.c_str(), reinterpret_cast<void*>(&s));
    if (result == INET_PTION_SUC) {
        return true;
    }
    return false;
}

int8_t EthernetManagement::GetAddrFamily(const std::string &ip)
{
    if (IsValidIPV4(ip)) {
        return AF_INET;
    }
    if (IsValidIPV6(ip)) {
        return AF_INET6;
    }
    return 0;
}

int32_t EthernetManagement::Ipv4PrefixLen(const std::string &ip)
{
    if (ip.empty()) {
        return 0;
    }
    int32_t ret = 0;
    uint32_t ipNum = 0;
    uint8_t c1 = 0;
    uint8_t c2 = 0;
    uint8_t c3 = 0;
    uint8_t c4 = 0;
    int32_t cnt = 0;
    ret = sscanf_s(ip.c_str(), "%hhu.%hhu.%hhu.%hhu", &c1, &c2, &c3, &c4);
    if (ret != sizeof(int32_t)) {
        return 0;
    }
    ipNum = (c1 << BIT24) | (c2 << BIT16) | (c3 << BIT8) | c4;
    if (ipNum == 0xFFFFFFFF) {
        return BIT32;
    }
    if (ipNum == 0xFFFFFF00) {
        return BIT24;
    }
    if (ipNum == 0xFFFF0000) {
        return BIT16;
    }
    if (ipNum == 0xFF000000) {
        return BIT8;
    }
    for (int32_t i = 0; i < BIT32; i++) {
        if ((ipNum << i) & 0x80000000) {
            cnt++;
        } else {
            break;
        }
    }
    return cnt;
}

void EthernetManagement::StartDhcpClient(const std::string &dev, sptr<DevInterfaceState> &devState)
{
    NETMGR_EXT_LOG_D("EthernetManagement StartDhcpClient[%{public}s]", dev.c_str());
    NetsysController::GetInstance().StartDhcpClient(dev, false);
    devState->SetDhcpReqState(true);
}

void EthernetManagement::StopDhcpClient(const std::string &dev, sptr<DevInterfaceState> &devState)
{
    NETMGR_EXT_LOG_D("EthernetManagement StopDhcpClient[%{public}s]", dev.c_str());
    NetsysController::GetInstance().StopDhcpClient(dev, false);
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