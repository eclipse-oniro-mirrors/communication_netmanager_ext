/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include <fstream>
#include <sstream>

#include "securec.h"
#include "ethernet_constants.h"
#include "netmanager_base_common_utils.h"
#include "netmgr_ext_log_wrapper.h"
#include "ethernet_configuration.h"

namespace OHOS {
namespace NetManagerStandard {
const std::string CONFIG_KEY_ETH_COMPONENT_FLAG = "config_ethernet_interfaces";
const std::string CONFIG_KEY_ETH_IFACE_REGEX = "eth";
const std::string CONFIG_KEY_ETH_IFACE = "iface";
const std::string CONFIG_KEY_ETH_CAPS = "caps";
const std::string CONFIG_KEY_ETH_IP = "ip";
const std::string CONFIG_KEY_ETH_GATEWAY = "gateway";
const std::string CONFIG_KEY_ETH_DNS = "dns";
const std::string CONFIG_KEY_ETH_NETMASK = "netmask";
const std::string CONFIG_KEY_ETH_ROUTE = "route";
const std::string CONFIG_KEY_ETH_ROUTE_MASK = "routemask";
constexpr int32_t MKDIR_ERR = -1;

EthernetConfiguration::EthernetConfiguration()
{
    bool ret = CreateDir(USER_CONFIG_DIR);
    NETMGR_EXT_LOG_D("CreateDir ret[%{public}d]", ret);
}

EthernetConfiguration::~EthernetConfiguration() {}

bool EthernetConfiguration::ReadSysteamConfiguration(std::map<std::string, std::set<NetCap>> &devCaps,
    std::map<std::string, sptr<InterfaceConfiguration>> &devCfgs)
{
    NETMGR_EXT_LOG_D("EthernetConfiguration::ReadSysteamConfiguration Enter");
    const auto& jsonStr = ReadJsonFile(NETWORK_CONFIG_PATH);
    if (jsonStr.length() == 0) {
        NETMGR_EXT_LOG_E("ReadConfigData config file is return empty!");
        return false;
    }
    const auto& jsonCfg = nlohmann::json::parse(jsonStr);
    if (jsonCfg.find(CONFIG_KEY_ETH_COMPONENT_FLAG) == jsonCfg.end()) {
        NETMGR_EXT_LOG_E("ReadConfigData not find network_ethernet_component!");
        return false;
    }
    const auto& arrIface = jsonCfg.at(CONFIG_KEY_ETH_COMPONENT_FLAG);
    NETMGR_EXT_LOG_D("read ConfigData ethValue:%{public}s", arrIface.dump().c_str());
    for (const auto &item : arrIface) {
        const auto& iface = item[CONFIG_KEY_ETH_IFACE];
        const auto& caps = item.at(CONFIG_KEY_ETH_CAPS).get<std::set<NetCap>>();
        const auto& fit = devCfgs.find(iface);
        if (fit != devCfgs.end()) {
            NETMGR_EXT_LOG_E("The iface=%{public}s device have set!", fit->first.c_str());
            continue;
        }
        sptr<InterfaceConfiguration> config = ConvertJsonToConfiguration(item);
        if (!item[CONFIG_KEY_ETH_IP].empty()) {
            devCfgs[iface] = config;
        }
        if (!caps.empty()) {
            devCaps[iface] = caps;
        }
    }
    return true;
}

sptr<InterfaceConfiguration> EthernetConfiguration::ConvertJsonToConfiguration(const nlohmann::json &jsonData)
{
    sptr<InterfaceConfiguration> config = new InterfaceConfiguration();
    config->mode_ = STATIC;
    config->ipStatic_.ipAddr_.address_ = jsonData[CONFIG_KEY_ETH_IP];
    config->ipStatic_.ipAddr_.netMask_ = jsonData[CONFIG_KEY_ETH_NETMASK];
    config->ipStatic_.ipAddr_.family_ = CommonUtils::GetAddrFamily(jsonData[CONFIG_KEY_ETH_IP]);
    int32_t prefixLen = CommonUtils::Ipv4PrefixLen(jsonData[CONFIG_KEY_ETH_NETMASK]);
    if (config->ipStatic_.ipAddr_.family_ == AF_INET) {
        config->ipStatic_.ipAddr_.prefixlen_ = prefixLen;
    }
    config->ipStatic_.netMask_.address_ = jsonData[CONFIG_KEY_ETH_NETMASK];
    config->ipStatic_.gateway_.address_ = jsonData[CONFIG_KEY_ETH_GATEWAY];
    config->ipStatic_.gateway_.family_ = CommonUtils::GetAddrFamily(jsonData[CONFIG_KEY_ETH_GATEWAY]);
    if (config->ipStatic_.gateway_.family_ == AF_INET) {
        config->ipStatic_.gateway_.prefixlen_ = prefixLen;
    }
    config->ipStatic_.route_.address_ = jsonData[CONFIG_KEY_ETH_ROUTE];
    int32_t routePrefixLen = 0;
    if (!jsonData[CONFIG_KEY_ETH_ROUTE_MASK].empty()) {
        routePrefixLen = CommonUtils::Ipv4PrefixLen(jsonData[CONFIG_KEY_ETH_ROUTE_MASK]);
    }
    config->ipStatic_.route_.family_ = CommonUtils::GetAddrFamily(jsonData[CONFIG_KEY_ETH_ROUTE]);
    if (config->ipStatic_.route_.family_ == AF_INET) {
        config->ipStatic_.route_.prefixlen_ = routePrefixLen;
    }
    std::vector<std::string> servers = CommonUtils::Split(jsonData[CONFIG_KEY_ETH_DNS], ",");
    for (const auto &dns: servers) {
        INetAddr addr;
        addr.address_ = dns;
        config->ipStatic_.dnsServers_.push_back(addr);
    }
    return config;
}

bool EthernetConfiguration::ReadUserConfiguration(std::map<std::string, sptr<InterfaceConfiguration>> &devCfgs)
{
    NETMGR_EXT_LOG_I("EthernetConfiguration::ReadUserConfiguration Enter");
    DIR *dir = nullptr;
    struct dirent *ptr = nullptr;
    if ((dir = opendir(USER_CONFIG_DIR)) == nullptr) {
        NETMGR_EXT_LOG_E("Read user configuration open dir error dir=[%{public}s]", USER_CONFIG_DIR);
        return false;
    }
    std::string iface;
    sptr<InterfaceConfiguration> cfg = nullptr;
    while ((ptr = readdir(dir)) != nullptr) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        } else if (ptr->d_type == DT_REG) {
            std::string filePath = std::string(USER_CONFIG_DIR) + "/" + ptr->d_name;
            std::string fileContent;
            if (!ReadFile(filePath, fileContent)) {
                continue;
            }
            std::string().swap(iface);
            cfg = std::make_unique<InterfaceConfiguration>().release();
            ParserFileConfig(fileContent, iface, cfg);
            if (!iface.empty()) {
                NETMGR_EXT_LOG_E("ReadFileList devname[%{public}s]", iface.c_str());
                devCfgs[iface] = cfg;
            }
        }
    }
    closedir(dir);
    return true;
}

bool EthernetConfiguration::WriteUserConfiguration(const std::string &iface, sptr<InterfaceConfiguration> &cfg)
{
    NETMGR_EXT_LOG_D("EthernetConfiguration::WriteUserConfiguration Enter");
    bool ret = CreateDir(USER_CONFIG_DIR);
    NETMGR_EXT_LOG_D("CreateDir ret[%{public}d]", ret);
    if (cfg->mode_ == STATIC) {
        uint32_t prefixlen = 0;
        cfg->ipStatic_.ipAddr_.family_ = CommonUtils::GetAddrFamily(cfg->ipStatic_.ipAddr_.address_);
        if (cfg->ipStatic_.ipAddr_.family_ == AF_INET) {
            if (cfg->ipStatic_.netMask_.address_.empty()) {
                prefixlen = CommonUtils::Ipv4PrefixLen(cfg->ipStatic_.ipAddr_.netMask_);
            } else {
                prefixlen = CommonUtils::Ipv4PrefixLen(cfg->ipStatic_.netMask_.address_);
            }
        }
        cfg->ipStatic_.ipAddr_.prefixlen_ = prefixlen;
        cfg->ipStatic_.gateway_.family_ = CommonUtils::GetAddrFamily(cfg->ipStatic_.gateway_.address_);
        cfg->ipStatic_.gateway_.prefixlen_ = prefixlen;
        cfg->ipStatic_.route_.family_ = CommonUtils::GetAddrFamily(cfg->ipStatic_.route_.address_);
        int32_t routePrefixLen = 0;
        if (!cfg->ipStatic_.route_.netMask_.empty()) {
            routePrefixLen = CommonUtils::Ipv4PrefixLen(cfg->ipStatic_.route_.netMask_);
        }
        cfg->ipStatic_.route_.prefixlen_ = routePrefixLen;
    }
    std::string fileContent;
    std::string filePath = std::string(USER_CONFIG_DIR) + "/" + iface;
    GenCfgContent(iface, cfg, fileContent);
    return WriteFile(filePath, fileContent);
}

bool EthernetConfiguration::ClearAllUserConfiguration()
{
    NETMGR_EXT_LOG_D("EthernetConfiguration::ClearAllUserConfiguration Enter");
    return DelDir(USER_CONFIG_DIR);
}

bool EthernetConfiguration::ConvertToConfiguration(const EthernetDhcpCallback::DhcpResult &dhcpResult,
    sptr<StaticConfiguration> &config)
{
    NETMGR_EXT_LOG_D("EthernetConfiguration::ConvertToConfiguration Enter");
    if (config == nullptr) {
        NETMGR_EXT_LOG_E("Error ConvertToIpConfiguration config is null!");
        return false;
    }
    const auto& emSymbol = "*";
    const auto& emAddr = "0.0.0.0";
    const auto& emPrefixlen = 0;
    int32_t prefixlen = 0;
    config->ipAddr_.address_ = dhcpResult.ipAddr;
    config->ipAddr_.family_ = CommonUtils::GetAddrFamily(dhcpResult.ipAddr);
    if (config->ipAddr_.family_ == AF_INET) {
        config->ipAddr_.prefixlen_ = CommonUtils::Ipv4PrefixLen(dhcpResult.subNet);
    }
    prefixlen = config->ipAddr_.prefixlen_;
    config->gateway_.address_ = dhcpResult.gateWay;
    config->gateway_.family_ = CommonUtils::GetAddrFamily(dhcpResult.gateWay);
    config->gateway_.prefixlen_ = prefixlen;
    if (dhcpResult.gateWay != dhcpResult.route1) {
        if (dhcpResult.route1 == emSymbol) {
            config->route_.address_ = emAddr;
            config->route_.prefixlen_ = emPrefixlen;
        } else {
            config->route_.address_ = dhcpResult.route1;
            config->route_.prefixlen_ = prefixlen;
        }
    }
    if (dhcpResult.gateWay != dhcpResult.route2) {
        if (dhcpResult.route2 == emSymbol) {
            config->route_.address_ = emAddr;
            config->route_.prefixlen_ = emPrefixlen;
        } else {
            config->route_.address_ = dhcpResult.route2;
            config->route_.prefixlen_ = prefixlen;
        }
    }
    config->route_.family_ = CommonUtils::GetAddrFamily(config->route_.address_);
    INetAddr dnsNet1;
    dnsNet1.address_ = dhcpResult.dns1;
    INetAddr dnsNet2;
    dnsNet2.address_ = dhcpResult.dns2;
    config->dnsServers_.push_back(dnsNet1);
    config->dnsServers_.push_back(dnsNet2);
    return true;
}

std::string EthernetConfiguration::ReadJsonFile(const std::string &filePath)
{
    std::ifstream infile;
    std::string strLine;
    std::string strAll = "";
    infile.open(filePath);
    if (!infile.is_open()) {
        NETMGR_EXT_LOG_D("ReadJsonFile filePath:%{public}s fail", filePath.c_str());
        return strAll;
    }
    while (getline(infile, strLine)) {
        strAll.append(strLine);
    }
    infile.close();
    return strAll;
}

bool EthernetConfiguration::IsDirExist(const std::string &dirPath)
{
    struct stat status;
    if (dirPath.empty()) {
        return false;
    }
    return (stat(dirPath.c_str(), &status) == 0);
}

bool EthernetConfiguration::CreateDir(const std::string &dirPath)
{
    if (IsDirExist(dirPath)) {
        return true;
    }
    if (mkdir(dirPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == MKDIR_ERR) {
        return false;
    }
    return true;
}

bool EthernetConfiguration::DelDir(const std::string &dirPath)
{
    DIR *dir = nullptr;
    struct dirent *entry = nullptr;
    struct stat statbuf;
    if ((dir = opendir(dirPath.c_str())) == nullptr) {
        NETMGR_EXT_LOG_E("EthernetConfiguration DelDir open user dir failed!");
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

bool EthernetConfiguration::IsFileExist(const std::string& filePath)
{
    return !access(filePath.c_str(), F_OK);
}

bool EthernetConfiguration::ReadFile(const std::string &filePath, std::string &fileContent)
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
        NETMGR_EXT_LOG_E("EthernetConfiguration read file=%{public}s fstream failed.", filePath.c_str());
        return false;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    fileContent = buffer.str();
    file.close();
    return true;
}

bool EthernetConfiguration::WriteFile(const std::string &filePath, const std::string &fileContent)
{
    std::fstream file(filePath.c_str(), std::fstream::out | std::fstream::trunc);
    if (file.is_open() == false) {
        NETMGR_EXT_LOG_E("EthernetConfiguration write file=%{public}s fstream failed.", filePath.c_str());
        return false;
    }
    file << fileContent.c_str();
    file.close();
    return true;
}

void EthernetConfiguration::ParserFileConfig(const std::string &fileContent, std::string &iface,
    sptr<InterfaceConfiguration> cfg)
{
    std::string::size_type pos = fileContent.find("DEVICE=") + strlen("DEVICE=");
    const auto& device = fileContent.substr(pos, fileContent.find("\n", pos) - pos);
    pos = fileContent.find("BOOTPROTO=") + strlen("BOOTPROTO=");
    const auto& bootProto = fileContent.substr(pos, fileContent.find("\n", pos) - pos);
    iface = device;
    if (bootProto == "STATIC") {
        cfg->mode_ = STATIC;
        pos = fileContent.find("IPADDR=") + strlen("IPADDR=");
        const auto& ipAddr = fileContent.substr(pos, fileContent.find("\n", pos) - pos);
        pos = fileContent.find("NETMASK=") + strlen("NETMASK=");
        const auto& netMask = fileContent.substr(pos, fileContent.find("\n", pos) - pos);
        pos = fileContent.find("GATEWAY=") + strlen("GATEWAY=");
        const auto& gatway = fileContent.substr(pos, fileContent.find("\n", pos) - pos);
        pos = fileContent.find("ROUTE=") + strlen("ROUTE=");
        const auto& route = fileContent.substr(pos, fileContent.find("\n", pos) - pos);
        pos = fileContent.find("ROUTE_NETMASK=") + strlen("ROUTE_NETMASK=");
        const auto& routeNetmask = fileContent.substr(pos, fileContent.find("\n", pos) - pos);
        cfg->ipStatic_.ipAddr_.address_ = ipAddr;
        cfg->ipStatic_.ipAddr_.netMask_ = netMask;
        cfg->ipStatic_.ipAddr_.family_ = CommonUtils::GetAddrFamily(ipAddr);
        int32_t prefixLen = CommonUtils::Ipv4PrefixLen(netMask);
        if (cfg->ipStatic_.ipAddr_.family_ == AF_INET) {
            cfg->ipStatic_.ipAddr_.prefixlen_ = prefixLen;
        }
        cfg->ipStatic_.netMask_.address_ = netMask;
        cfg->ipStatic_.gateway_.address_ = gatway;
        cfg->ipStatic_.gateway_.family_ = CommonUtils::GetAddrFamily(gatway);
        if (cfg->ipStatic_.gateway_.family_ == AF_INET) {
            cfg->ipStatic_.gateway_.prefixlen_ = prefixLen;
        }
        cfg->ipStatic_.route_.address_ = route;
        int32_t routePrefixLen = 0;
        if (!routeNetmask.empty()) {
            routePrefixLen = CommonUtils::Ipv4PrefixLen(routeNetmask);
        }
        cfg->ipStatic_.route_.family_ = CommonUtils::GetAddrFamily(route);
        if (cfg->ipStatic_.route_.family_ == AF_INET) {
            cfg->ipStatic_.route_.prefixlen_ = routePrefixLen;
        }
    } else if (bootProto == "DHCP") {
        cfg->mode_ = DHCP;
    }
}

void EthernetConfiguration::GenCfgContent(const std::string &iface, sptr<InterfaceConfiguration> cfg,
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
} // namespace NetManagerStandard
} // namespace OHOS