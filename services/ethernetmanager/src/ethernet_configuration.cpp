/*
 * Copyright (C) 2022-2023 Huawei Device Co., Ltd.
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
#include "ethernet_configuration.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstdlib>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <limits>
#include <regex>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "net_manager_constants.h"
#include "netmanager_base_common_utils.h"
#include "netmgr_ext_log_wrapper.h"
#include "route.h"
#include "securec.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const std::string IFACE_MATCH = "eth\\d";
const std::string CONFIG_KEY_ETH_COMPONENT_FLAG = "config_ethernet_interfaces";
const std::string CONFIG_KEY_ETH_IFACE = "iface";
const std::string CONFIG_KEY_ETH_LANIFACE = "laniface";
const std::string CONFIG_KEY_ETH_CAPS = "caps";
const std::string CONFIG_KEY_ETH_IP = "ip";
const std::string CONFIG_KEY_ETH_GATEWAY = "gateway";
const std::string CONFIG_KEY_ETH_DNS = "dns";
const std::string CONFIG_KEY_ETH_NETMASK = "netmask";
const std::string CONFIG_KEY_ETH_ROUTE = "route";
const std::string CONFIG_KEY_ETH_ROUTE_MASK = "routemask";
constexpr int32_t MKDIR_ERR = -1;
constexpr int32_t USER_PATH_LEN = 25;
constexpr const char *FILE_OBLIQUE_LINE = "/";
constexpr const char *KEY_DEVICE = "DEVICE=";
constexpr const char *KEY_BOOTPROTO = "BOOTPROTO=";
constexpr const char *KEY_STATIC = "STATIC";
constexpr const char *KEY_DHCP = "DHCP";
constexpr const char *KEY_LAN_STATIC = "LAN_STATIC";
constexpr const char *KEY_LAN_DHCP = "LAN_DHCP";
constexpr const char *KEY_IPADDR = "IPADDR=";
constexpr const char *KEY_NETMASK = "NETMASK=";
constexpr const char *KEY_GATEWAY = "GATEWAY=";
constexpr const char *KEY_ROUTE = "ROUTE=";
constexpr const char *KEY_ROUTE_NETMASK = "ROUTE_NETMASK=";
constexpr const char *KEY_DNS = "DNS=";
constexpr const char *KEY_PROXY_HOST = "PROXY_HOST=";
constexpr const char *KEY_PROXY_PORT = "PROXY_PORT=";
constexpr const char *KEY_PROXY_EXCLUSIONS = "PROXY_EXCLUSIONS=";
constexpr const char *WRAP = "\n";
constexpr const char *DEFAULT_IPV4_ADDR = "0.0.0.0";
constexpr const char *DEFAULT_IPV6_ADDR = "::";
constexpr const char *DEFAULT_IPV6_MAX_ADDRESS = "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff";
constexpr const char *EMPTY_NET_ADDR = "*";
constexpr const char *ADDR_SEPARATOR = ",";
constexpr const char *EXCLUSIONS_DELIMITER = ",";
} // namespace

EthernetConfiguration::EthernetConfiguration()
{
    CreateDir(USER_CONFIG_DIR);
}

bool EthernetConfiguration::ReadEthernetInterfaces(std::map<std::string, std::set<NetCap>> &devCaps,
                                                   std::map<std::string, sptr<InterfaceConfiguration>> &devCfgs,
                                                   const cJSON* const json)
{
    for (uint32_t i = 0; i < cJSON_GetArraySize(json); i++) {
        cJSON *item = cJSON_GetArrayItem(json, i);
        if (item == nullptr) {
            continue;
        }
        std::string iface;
        bool isLan = false;
        cJSON *lanIface = cJSON_GetObjectItem(item, CONFIG_KEY_ETH_LANIFACE.c_str());
        if (lanIface == nullptr) {
            cJSON *ethIface = cJSON_GetObjectItem(item, CONFIG_KEY_ETH_IFACE.c_str());
            iface = cJSON_GetStringValue(ethIface);
            isLan = false;
        } else {
            iface = cJSON_GetStringValue(lanIface);
            isLan = true;
        }
        cJSON *capsObj = cJSON_GetObjectItem(item, CONFIG_KEY_ETH_CAPS.c_str());
        std::set<NetCap> caps;
        for (uint32_t j = 0; j < cJSON_GetArraySize(capsObj); j++) {
            cJSON *capsItem = cJSON_GetArrayItem(capsObj, j);
            if (capsItem == nullptr) {
                continue;
            }
            const auto capsValue = capsItem->valueint;
            NETMGR_EXT_LOG_D("ReadConfigData capsValue : %{public}d", capsValue);
            caps.insert(NetCap(capsValue));
        }
        if (!caps.empty()) {
            devCaps[iface] = caps;
        }
        const auto &fit = devCfgs.find(iface);
        if (fit != devCfgs.end()) {
            NETMGR_EXT_LOG_E("The iface=%{public}s device have set!", fit->first.c_str());
            continue;
        }
        sptr<InterfaceConfiguration> config = ConvertJsonToConfiguration(item, isLan);
        if (config == nullptr) {
            NETMGR_EXT_LOG_E("config is nullptr");
            return false;
        }
        std::regex re(IFACE_MATCH);
        if (cJSON_GetObjectItem(item, CONFIG_KEY_ETH_IP.c_str()) && std::regex_search(iface, re)) {
            devCfgs[iface] = config;
        }
    }
    return true;
}

bool EthernetConfiguration::ReadSystemConfiguration(std::map<std::string, std::set<NetCap>> &devCaps,
                                                    std::map<std::string, sptr<InterfaceConfiguration>> &devCfgs)
{
    const auto &jsonStr = ReadJsonFile(NETWORK_CONFIG_PATH);
    if (jsonStr.length() == 0) {
        NETMGR_EXT_LOG_E("ReadConfigData config file is return empty!");
        return false;
    }
    cJSON *json = cJSON_Parse(jsonStr.c_str());
    if (json == nullptr) {
        NETMGR_EXT_LOG_E("json parse failed!");
        return false;
    }
    cJSON *jsonEth = cJSON_GetObjectItem(json, CONFIG_KEY_ETH_COMPONENT_FLAG.c_str());
    if (jsonEth == nullptr) {
        NETMGR_EXT_LOG_E("ReadConfigData not find config_ethernet_interfaces!");
        cJSON_Delete(json);
        return false;
    }
    ReadEthernetInterfaces(devCaps, devCfgs, jsonEth);
    cJSON_Delete(json);
    return true;
}

sptr<InterfaceConfiguration> EthernetConfiguration::ConvertJsonToConfiguration(const cJSON* const jsonData, bool isLan)
{
    sptr<InterfaceConfiguration> config = new (std::nothrow) InterfaceConfiguration();
    if (config == nullptr) {
        NETMGR_EXT_LOG_E("config is nullptr");
        return nullptr;
    }

    if (isLan) {
        config->mode_ = LAN_STATIC;
    } else {
        config->mode_ = STATIC;
    }
    std::string ip = cJSON_GetObjectItem(jsonData, CONFIG_KEY_ETH_IP.c_str())->valuestring;
    std::string route = cJSON_GetObjectItem(jsonData, CONFIG_KEY_ETH_ROUTE.c_str())->valuestring;
    std::string gateway = cJSON_GetObjectItem(jsonData, CONFIG_KEY_ETH_GATEWAY.c_str())->valuestring;
    std::string netmask = cJSON_GetObjectItem(jsonData, CONFIG_KEY_ETH_NETMASK.c_str())->valuestring;
    std::string dns = cJSON_GetObjectItem(jsonData, CONFIG_KEY_ETH_DNS.c_str())->valuestring;
    StaticConfiguration::ExtractNetAddrBySeparator(ip, config->ipStatic_.ipAddrList_);
    StaticConfiguration::ExtractNetAddrBySeparator(route, config->ipStatic_.routeList_);
    StaticConfiguration::ExtractNetAddrBySeparator(gateway, config->ipStatic_.gatewayList_);
    StaticConfiguration::ExtractNetAddrBySeparator(netmask, config->ipStatic_.netMaskList_);
    StaticConfiguration::ExtractNetAddrBySeparator(dns, config->ipStatic_.dnsServers_);
    std::string routeMask = cJSON_GetObjectItem(jsonData, CONFIG_KEY_ETH_ROUTE_MASK.c_str())->valuestring;
    NETMGR_EXT_LOG_D("ip:%{public}s, route:%{public}s, gateway:%{public}s, netmask:%{public}s, dns:%{public}s,\
                     routemask:%{public}s", ip.c_str(), route.c_str(), gateway.c_str(), netmask.c_str(),
                     dns.c_str(), routeMask.c_str());
    ParserIfaceIpAndRoute(config, routeMask);
    return config;
}

bool EthernetConfiguration::ReadUserConfiguration(std::map<std::string, sptr<InterfaceConfiguration>> &devCfgs)
{
    DIR *dir = nullptr;
    dirent *ptr = nullptr;
    if ((dir = opendir(USER_CONFIG_DIR)) == nullptr) {
        NETMGR_EXT_LOG_E("Read user configuration open dir error dir=[%{public}s]", USER_CONFIG_DIR);
        return false;
    }
    std::string iface;
    while ((ptr = readdir(dir)) != nullptr) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        }
        if (ptr->d_type == DT_REG) {
            std::string filePath = std::string(USER_CONFIG_DIR) + FILE_OBLIQUE_LINE + ptr->d_name;
            std::string fileContent;
            if (!ReadFile(filePath, fileContent)) {
                continue;
            }
            std::string().swap(iface);
            sptr<InterfaceConfiguration> cfg = new (std::nothrow) InterfaceConfiguration();
            if (cfg == nullptr) {
                NETMGR_EXT_LOG_E("cfg new failed for devname[%{public}s]", iface.c_str());
                continue;
            }
            ParserFileConfig(fileContent, iface, cfg);
            std::regex re(IFACE_MATCH);
            if (!iface.empty() && std::regex_search(iface, re)) {
                NETMGR_EXT_LOG_D("ReadFileList devname[%{public}s]", iface.c_str());
                devCfgs[iface] = cfg;
            }
        }
    }
    closedir(dir);
    return true;
}

bool EthernetConfiguration::WriteUserConfiguration(const std::string &iface, sptr<InterfaceConfiguration> &cfg)
{
    if (cfg == nullptr) {
        NETMGR_EXT_LOG_E("cfg is nullptr");
        return false;
    }
    if (!CreateDir(USER_CONFIG_DIR)) {
        NETMGR_EXT_LOG_E("create dir failed");
        return false;
    }

    if (cfg->mode_ == STATIC || cfg->mode_ == LAN_STATIC) {
        ParserIfaceIpAndRoute(cfg, std::string());
    }

    std::string fileContent;
    GenCfgContent(iface, cfg, fileContent);

    std::string filePath = std::string(USER_CONFIG_DIR) + FILE_OBLIQUE_LINE + iface;
    return WriteFile(filePath, fileContent);
}

bool EthernetConfiguration::ClearAllUserConfiguration()
{
    return DelDir(USER_CONFIG_DIR);
}

bool EthernetConfiguration::ConvertToConfiguration(const EthernetDhcpCallback::DhcpResult &dhcpResult,
                                                   sptr<StaticConfiguration> &config)
{
    if (config == nullptr) {
        NETMGR_EXT_LOG_E("Error ConvertToIpConfiguration config is null");
        return false;
    }
    if (!IsValidDhcpResult(dhcpResult, config)) {
        return false;
    }

    INetAddr ipAddr;
    ipAddr.address_ = dhcpResult.ipAddr;
    ipAddr.family_ = static_cast<uint8_t>(CommonUtils::GetAddrFamily(dhcpResult.ipAddr));
    ipAddr.prefixlen_ = (ipAddr.family_ == AF_INET6)
                            ? static_cast<uint8_t>(CommonUtils::Ipv6PrefixLen(dhcpResult.subNet))
                            : static_cast<uint8_t>(CommonUtils::Ipv4PrefixLen(dhcpResult.subNet));
    config->ipAddrList_.push_back(ipAddr);

    INetAddr netMask;
    netMask.address_ = dhcpResult.subNet;
    config->netMaskList_.push_back(netMask);

    INetAddr gateway;
    gateway.address_ = dhcpResult.gateWay;
    gateway.family_ = static_cast<uint8_t>(CommonUtils::GetAddrFamily(dhcpResult.gateWay));
    config->gatewayList_.push_back(gateway);

    INetAddr route;
    if (dhcpResult.gateWay != dhcpResult.route1 && dhcpResult.route1 != EMPTY_NET_ADDR) {
        route.address_ = dhcpResult.route1;
        route.prefixlen_ = ipAddr.prefixlen_;
    } else if (dhcpResult.gateWay != dhcpResult.route2 && dhcpResult.route2 != EMPTY_NET_ADDR) {
        route.address_ = dhcpResult.route2;
        route.prefixlen_ = ipAddr.prefixlen_;
    } else {
        route.address_ = (ipAddr.family_ == AF_INET6) ? DEFAULT_IPV6_ADDR : DEFAULT_IPV4_ADDR;
        route.prefixlen_ = 0;
    }
    route.family_ = static_cast<uint8_t>(CommonUtils::GetAddrFamily(route.address_));
    config->routeList_.push_back(route);

    INetAddr dnsNet1;
    dnsNet1.address_ = dhcpResult.dns1;
    INetAddr dnsNet2;
    dnsNet2.address_ = dhcpResult.dns2;
    config->dnsServers_.push_back(dnsNet1);
    config->dnsServers_.push_back(dnsNet2);
    return true;
}

std::vector<INetAddr> EthernetConfiguration::GetGatewayFromMap(const std::unordered_map<std::string, INetAddr> &temp)
{
    std::vector<INetAddr> t;
    for (auto [k, v] : temp) {
        t.push_back(v);
    }
    return t;
}

std::vector<INetAddr> EthernetConfiguration::GetGatewayFromRouteList(std::list<Route> &routeList)
{
    std::unordered_map<std::string, INetAddr> temp;
    for (const auto &route : routeList) {
        temp.emplace(route.gateway_.address_, route.gateway_);
    }
    auto temp2 = temp;
    temp.erase(DEFAULT_IPV4_ADDR);
    temp.erase(DEFAULT_IPV6_ADDR);
    if (temp.size() > 0) {
        return GetGatewayFromMap(temp);
    }
    return GetGatewayFromMap(temp2);
}

sptr<InterfaceConfiguration> EthernetConfiguration::MakeInterfaceConfiguration(
    const sptr<InterfaceConfiguration> &devCfg, const sptr<NetLinkInfo> &devLinkInfo)
{
    if (devCfg == nullptr || devLinkInfo == nullptr) {
        NETMGR_EXT_LOG_E("param is nullptr");
        return nullptr;
    }
    sptr<InterfaceConfiguration> cfg = new (std::nothrow) InterfaceConfiguration();
    if (cfg == nullptr) {
        NETMGR_EXT_LOG_E("cfg new failed");
        return nullptr;
    }
    cfg->mode_ = devCfg->mode_;
    for (const auto &ipAddr : devLinkInfo->netAddrList_) {
        cfg->ipStatic_.ipAddrList_.push_back(ipAddr);
        auto family = CommonUtils::GetAddrFamily(ipAddr.address_);
        INetAddr netMask;
        netMask.address_ =
            ipAddr.netMask_.empty()
                ? (((family == AF_INET6) ? CommonUtils::GetIpv6Prefix(DEFAULT_IPV6_MAX_ADDRESS, ipAddr.prefixlen_)
                                         : CommonUtils::GetMaskByLength(ipAddr.prefixlen_)))
                : ipAddr.netMask_;
        cfg->ipStatic_.netMaskList_.push_back(netMask);
    }
    for (const auto &route : devLinkInfo->routeList_) {
        cfg->ipStatic_.routeList_.push_back(route.destination_);
    }
    cfg->ipStatic_.gatewayList_ = GetGatewayFromRouteList(devLinkInfo->routeList_);

    cfg->ipStatic_.domain_ = devLinkInfo->domain_;
    for (const auto &addr : devLinkInfo->dnsList_) {
        cfg->ipStatic_.dnsServers_.push_back(addr);
    }
    return cfg;
}

std::string EthernetConfiguration::ReadJsonFile(const std::string &filePath)
{
    std::ifstream infile;
    std::string strLine;
    std::string strAll;
    infile.open(filePath);
    if (!infile.is_open()) {
        NETMGR_EXT_LOG_E("ReadJsonFile filePath failed");
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
        NETMGR_EXT_LOG_E("mkdir failed %{public}d: %{public}s", errno, strerror(errno));
        return false;
    }
    return true;
}

bool EthernetConfiguration::DelDir(const std::string &dirPath)
{
    DIR *dir = nullptr;
    dirent *entry = nullptr;
    struct stat statbuf;
    if ((dir = opendir(dirPath.c_str())) == nullptr) {
        NETMGR_EXT_LOG_E("EthernetConfiguration DelDir open user dir failed!");
        return false;
    }
    while ((entry = readdir(dir)) != nullptr) {
        std::string filePath = dirPath + FILE_OBLIQUE_LINE + entry->d_name;
        lstat(filePath.c_str(), &statbuf);
        if (S_ISREG(statbuf.st_mode)) {
            remove(filePath.c_str());
        }
    }
    closedir(dir);
    sync();
    return rmdir(dirPath.c_str()) >= 0;
}

bool EthernetConfiguration::IsFileExist(const std::string &filePath, std::string &realPath)
{
    char tmpPath[PATH_MAX] = {0};
    if (!realpath(filePath.c_str(), tmpPath)) {
        NETMGR_EXT_LOG_E("file name is error");
        return false;
    }
    if (strncmp(tmpPath, USER_CONFIG_DIR, USER_PATH_LEN) != 0) {
        NETMGR_EXT_LOG_E("file path is error");
        return false;
    }
    realPath = tmpPath;
    return true;
}

bool EthernetConfiguration::ReadFile(const std::string &filePath, std::string &fileContent)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (filePath.empty()) {
        NETMGR_EXT_LOG_E("filePath empty.");
        return false;
    }
    std::string realPath;
    if (!IsFileExist(filePath, realPath)) {
        NETMGR_EXT_LOG_E("[%{public}s] not exist.", filePath.c_str());
        return false;
    }
    std::fstream file(realPath.c_str(), std::fstream::in);
    if (!file.is_open()) {
        NETMGR_EXT_LOG_E("EthernetConfiguration read file failed.err %{public}d %{public}s", errno, strerror(errno));
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
    if (!file.is_open()) {
        NETMGR_EXT_LOG_E("EthernetConfiguration write file=%{public}s fstream failed. err %{public}d %{public}s",
                         filePath.c_str(), errno, strerror(errno));
        return false;
    }
    file << fileContent.c_str();
    file.close();
    sync();
    return true;
}

void EthernetConfiguration::ParserFileConfig(const std::string &fileContent, std::string &iface,
                                             sptr<InterfaceConfiguration> cfg)
{
    ParseDevice(fileContent, iface);
    ParseBootProto(fileContent, cfg);
    ParseStaticConfig(fileContent, cfg);
    ParserFileHttpProxy(fileContent, cfg);
}

void EthernetConfiguration::ParseDevice(const std::string &fileContent, std::string &iface)
{
    std::string::size_type pos = fileContent.find(KEY_DEVICE);
    if (pos == std::string::npos) {
        return;
    }
    pos += strlen(KEY_DEVICE);
    const auto &device = fileContent.substr(pos, fileContent.find(WRAP, pos) - pos);
    iface = device;
}

void EthernetConfiguration::ParseBootProto(const std::string &fileContent, sptr<InterfaceConfiguration> cfg)
{
    std::string::size_type pos = fileContent.find(KEY_BOOTPROTO);
    if (pos == std::string::npos) {
        return;
    }
    pos += strlen(KEY_BOOTPROTO);
    const auto &bootProto = fileContent.substr(pos, fileContent.find(WRAP, pos) - pos);
    if (bootProto == KEY_LAN_STATIC) {
        cfg->mode_ = LAN_STATIC;
    } else if (bootProto == KEY_LAN_DHCP) {
        cfg->mode_ = LAN_DHCP;
    } else if (bootProto == KEY_STATIC) {
        cfg->mode_ = STATIC;
    } else {
        cfg->mode_ = DHCP;
    }
}

void EthernetConfiguration::ParseStaticConfig(const std::string &fileContent, sptr<InterfaceConfiguration> cfg)
{
    if (cfg->mode_ != STATIC && cfg->mode_ != LAN_STATIC) {
        return;
    }
    std::string ipAddresses, netMasks, gateways, routes, routeMasks, dnsServers;
    auto pos = fileContent.find(KEY_IPADDR);
    if (pos != std::string::npos) {
        pos += strlen(KEY_IPADDR);
        ipAddresses = fileContent.substr(pos, fileContent.find(WRAP, pos) - pos);
    }

    pos = fileContent.find(KEY_NETMASK);
    if (pos != std::string::npos) {
        pos += strlen(KEY_NETMASK);
        netMasks = fileContent.substr(pos, fileContent.find(WRAP, pos) - pos);
    }

    pos = fileContent.find(KEY_GATEWAY);
    if (pos != std::string::npos) {
        pos += strlen(KEY_GATEWAY);
        gateways = fileContent.substr(pos, fileContent.find(WRAP, pos) - pos);
    }

    pos = fileContent.find(KEY_ROUTE);
    if (pos != std::string::npos) {
        pos += strlen(KEY_ROUTE);
        routes = fileContent.substr(pos, fileContent.find(WRAP, pos) - pos);
    }

    pos = fileContent.find(KEY_ROUTE_NETMASK);
    if (pos != std::string::npos) {
        pos += strlen(KEY_ROUTE_NETMASK);
        routeMasks = fileContent.substr(pos, fileContent.find(WRAP, pos) - pos);
    }

    pos = fileContent.find(KEY_DNS);
    if (pos != std::string::npos) {
        pos += strlen(KEY_DNS);
        dnsServers = fileContent.substr(pos, fileContent.find(WRAP, pos) - pos);
    }

    StaticConfiguration::ExtractNetAddrBySeparator(ipAddresses, cfg->ipStatic_.ipAddrList_);
    StaticConfiguration::ExtractNetAddrBySeparator(routes, cfg->ipStatic_.routeList_);
    StaticConfiguration::ExtractNetAddrBySeparator(gateways, cfg->ipStatic_.gatewayList_);
    StaticConfiguration::ExtractNetAddrBySeparator(netMasks, cfg->ipStatic_.netMaskList_);
    StaticConfiguration::ExtractNetAddrBySeparator(dnsServers, cfg->ipStatic_.dnsServers_);
    ParserIfaceIpAndRoute(cfg, routeMasks);
}

void EthernetConfiguration::ParserFileHttpProxy(const std::string &fileContent, const sptr<InterfaceConfiguration> &cfg)
{
    std::string::size_type pos = fileContent.find(KEY_PROXY_HOST);
    if (pos != std::string::npos) {
        pos += strlen(KEY_PROXY_HOST);
        cfg->httpProxy_.SetHost(fileContent.substr(pos, fileContent.find(WRAP, pos) - pos));
    }

    pos = fileContent.find(KEY_PROXY_PORT);
    if (pos != std::string::npos) {
        pos += strlen(KEY_PROXY_PORT);
        uint32_t port = CommonUtils::StrToUint(fileContent.substr(pos, fileContent.find(WRAP, pos) - pos));
        cfg->httpProxy_.SetPort(static_cast<uint16_t>(port));
    }

    pos = fileContent.find(KEY_PROXY_EXCLUSIONS);
    if (pos != std::string::npos) {
        pos += strlen(KEY_PROXY_EXCLUSIONS);
        auto exclusions = fileContent.substr(pos, fileContent.find(WRAP, pos) - pos);
        std::list<std::string> exclusionList;
        for (const auto &exclusion : CommonUtils::Split(exclusions, EXCLUSIONS_DELIMITER)) {
            exclusionList.push_back(exclusion);
        }
        cfg->httpProxy_.SetExclusionList(exclusionList);
    }
}

void EthernetConfiguration::ParserIfaceIpAndRoute(sptr<InterfaceConfiguration> &cfg, const std::string &rootNetMask)
{
    if (cfg == nullptr) {
        NETMGR_EXT_LOG_E("cfg is nullptr");
        return;
    }

    std::for_each(cfg->ipStatic_.netMaskList_.begin(), cfg->ipStatic_.netMaskList_.end(), [&cfg](const auto &netMask) {
        auto maskFamily = CommonUtils::GetAddrFamily(netMask.address_);
        for (auto &ipAddr : cfg->ipStatic_.ipAddrList_) {
            if (maskFamily != CommonUtils::GetAddrFamily(ipAddr.address_)) {
                continue;
            }
            ipAddr.netMask_ = netMask.address_;
            ipAddr.prefixlen_ =
	    (maskFamily == AF_INET6) ? static_cast<uint32_t>(CommonUtils::Ipv6PrefixLen(netMask.address_))
                                     : static_cast<uint32_t>(CommonUtils::Ipv4PrefixLen(netMask.address_));
            break;
        }
    });

    for (const auto &routeMask : CommonUtils::Split(rootNetMask, ADDR_SEPARATOR)) {
        auto maskFamily = CommonUtils::GetAddrFamily(routeMask);
        for (auto &route : cfg->ipStatic_.routeList_) {
            if (maskFamily != CommonUtils::GetAddrFamily(route.address_)) {
                continue;
            }
            route.prefixlen_ = (maskFamily == AF_INET6) ? static_cast<uint32_t>(CommonUtils::Ipv6PrefixLen(routeMask))
                                                        : static_cast<uint32_t>(CommonUtils::Ipv4PrefixLen(routeMask));
            break;
        }
    }
}

std::string EthernetConfiguration::GetIfaceMode(IPSetMode mode)
{
    switch (mode) {
        case LAN_STATIC:
            return KEY_LAN_STATIC;
        case LAN_DHCP:
            return KEY_LAN_DHCP;
        case STATIC:
            return KEY_STATIC;
        default:
            return KEY_DHCP;
    }
}

void EthernetConfiguration::GenCfgContent(const std::string &iface, sptr<InterfaceConfiguration> cfg,
                                          std::string &fileContent)
{
    if (cfg == nullptr) {
        NETMGR_EXT_LOG_E("cfg is nullptr");
        return;
    }
    std::string().swap(fileContent);
    fileContent = fileContent + KEY_DEVICE + iface + WRAP;
    std::string mode = GetIfaceMode(cfg->mode_);
    fileContent = fileContent + KEY_BOOTPROTO + mode + WRAP;
    if (cfg->mode_ == STATIC || cfg->mode_ == LAN_STATIC) {
        std::string ipAddresses = AccumulateNetAddress(cfg->ipStatic_.ipAddrList_);
        std::string netMasks = AccumulateNetAddress(cfg->ipStatic_.netMaskList_);
        std::string gateways = AccumulateNetAddress(cfg->ipStatic_.gatewayList_);
        std::string routes = AccumulateNetAddress(cfg->ipStatic_.routeList_);
        std::string routeMasks =
            std::accumulate(cfg->ipStatic_.routeList_.begin(), cfg->ipStatic_.routeList_.end(), std::string(),
                            [](const std::string &routeMask, const INetAddr &iter) {
                                auto family = CommonUtils::GetAddrFamily(iter.address_);
                                std::string mask = (family == AF_INET6) ? DEFAULT_IPV6_ADDR : DEFAULT_IPV4_ADDR;
                                return routeMask.empty() ? routeMask + mask : (routeMask + ADDR_SEPARATOR + mask);
                            });
        std::string dnsServers = AccumulateNetAddress(cfg->ipStatic_.dnsServers_);

        fileContent = fileContent + KEY_IPADDR + ipAddresses + WRAP;
        fileContent = fileContent + KEY_NETMASK + netMasks + WRAP;
        fileContent = fileContent + KEY_GATEWAY + gateways + WRAP;
        fileContent = fileContent + KEY_ROUTE + routes + WRAP;
        fileContent = fileContent + KEY_ROUTE_NETMASK + routeMasks + WRAP;
        fileContent = fileContent + KEY_DNS + dnsServers + WRAP;
    }
    GenHttpProxyContent(cfg, fileContent);
}

void EthernetConfiguration::GenHttpProxyContent(const sptr<InterfaceConfiguration> &cfg, std::string &fileContent)
{
    const auto &exclusionList = cfg->httpProxy_.GetExclusionList();
    std::string exclusions =
        std::accumulate(exclusionList.begin(), exclusionList.end(), std::string(),
                        [](const std::string &exclusion, const std::string &next) {
                            return exclusion.empty() ? exclusion + next : (exclusion + EXCLUSIONS_DELIMITER + next);
                        });

    fileContent = fileContent + KEY_PROXY_HOST + cfg->httpProxy_.GetHost() + WRAP;
    fileContent = fileContent + KEY_PROXY_PORT + std::to_string(cfg->httpProxy_.GetPort()) + WRAP;
    fileContent = fileContent + KEY_PROXY_EXCLUSIONS + exclusions + WRAP;
}

std::string EthernetConfiguration::AccumulateNetAddress(const std::vector<INetAddr> &netAddrList)
{
    return std::accumulate(netAddrList.begin(), netAddrList.end(), std::string(),
                           [](const std::string &addr, const INetAddr &iter) {
                               return addr.empty() ? (addr + iter.address_) : (addr + ADDR_SEPARATOR + iter.address_);
                           });
}

bool EthernetConfiguration::IsValidDhcpResult(const EthernetDhcpCallback::DhcpResult &dhcpResult,
                                              sptr<StaticConfiguration> &config)
{
    if (config == nullptr) {
        NETMGR_EXT_LOG_E("config is nullptr");
        return false;
    }
    if (dhcpResult.ipAddr.empty()) {
        NETMGR_EXT_LOG_E("DhcpResult ip addr is empty");
        return false;
    }

    bool isSameIp = false;
    bool isSameGateway = false;
    if (std::any_of(config->ipAddrList_.begin(), config->ipAddrList_.end(), [&dhcpResult](const auto &ipAddr) {
        return dhcpResult.ipAddr == ipAddr.address_;
        })) {
        NETMGR_EXT_LOG_I("Same ip addr:%{public}s", CommonUtils::ToAnonymousIp(dhcpResult.ipAddr).c_str());
        isSameIp = true;
    }

    if (std::any_of(config->gatewayList_.begin(), config->gatewayList_.end(), [&dhcpResult](const auto &gateway) {
        return dhcpResult.gateWay == gateway.address_;
        })) {
        NETMGR_EXT_LOG_I("Same gateway:%{public}s", CommonUtils::ToAnonymousIp(dhcpResult.gateWay).c_str());
        isSameGateway = true;
    }
    return !(isSameIp && isSameGateway);
}
} // namespace NetManagerStandard
} // namespace OHOS
