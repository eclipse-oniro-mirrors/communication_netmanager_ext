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

#ifndef ETHERNET_CONFIGURATION_H
#define ETHERNET_CONFIGURATION_H

#include <map>
#include <mutex>
#include <numeric>
#include <set>
#include <string>
#include <vector>

#include "cJSON.h"
#include "ethernet_dhcp_callback.h"
#include "http_proxy.h"
#include "interface_configuration.h"
#include "net_all_capabilities.h"
#include "net_link_info.h"

namespace OHOS {
namespace NetManagerStandard {
class EthernetConfiguration {
public:
    EthernetConfiguration();
    ~EthernetConfiguration() = default;

    bool ReadSystemConfiguration(std::map<std::string, std::set<NetCap>> &devCaps,
                                 std::map<std::string, sptr<InterfaceConfiguration>> &devCfgs);
    bool ReadUserConfiguration(std::map<std::string, sptr<InterfaceConfiguration>> &devCfgs);
    bool WriteUserConfiguration(const std::string &iface, sptr<InterfaceConfiguration> &cfg);
    bool ClearAllUserConfiguration();
    bool ConvertToConfiguration(const EthernetDhcpCallback::DhcpResult &dhcpResult, sptr<StaticConfiguration> &config);
    sptr<InterfaceConfiguration> MakeInterfaceConfiguration(const sptr<InterfaceConfiguration> &devCfg,
                                                            const sptr<NetLinkInfo> &devLinkInfo);

private:
    void ParseDevice(const std::string &fileContent, std::string &iface);
    void ParseBootProto(const std::string &fileContent, sptr<InterfaceConfiguration> cfg);
    void ParseStaticConfig(const std::string &fileContent, sptr<InterfaceConfiguration> cfg);

    bool ReadEthernetInterfaces(std::map<std::string, std::set<NetCap>> &devCaps,
                                std::map<std::string, sptr<InterfaceConfiguration>> &devCfgs,
                                const cJSON* const json);
    std::string ReadJsonFile(const std::string &filePath);
    sptr<InterfaceConfiguration> ConvertJsonToConfiguration(const cJSON* const jsonData, bool isLan);
    bool IsDirExist(const std::string &dirPath);
    bool CreateDir(const std::string &dirPath);
    bool DelDir(const std::string &dirPath);
    bool IsFileExist(const std::string &filePath, std::string &realPath);
    bool ReadFile(const std::string &filePath, std::string &fileContent);
    bool WriteFile(const std::string &filePath, const std::string &fileContent);
    void ParserFileConfig(const std::string &fileContent, std::string &iface, sptr<InterfaceConfiguration> cfg);
    void ParserFileHttpProxy(const std::string &fileContent, const sptr<InterfaceConfiguration> &cfg);
    void ParserIfaceIpAndRoute(sptr<InterfaceConfiguration> &cfg, const std::string &rootNetMask);
    void GenCfgContent(const std::string &iface, sptr<InterfaceConfiguration> cfg, std::string &fileContent);
    void GenHttpProxyContent(const sptr<InterfaceConfiguration> &cfg, std::string &fileContent);
    std::string AccumulateNetAddress(const std::vector<INetAddr> &netAddrList);
    bool IsValidDhcpResult(const EthernetDhcpCallback::DhcpResult &dhcpResult, sptr<StaticConfiguration> &config);
    std::string GetIfaceMode(IPSetMode mode);
    std::vector<INetAddr> GetGatewayFromMap(const std::unordered_map<std::string, INetAddr> &temp);
    std::vector<INetAddr> GetGatewayFromRouteList(std::list<Route> &routeList);

private:
    std::mutex mutex_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // ETHERNET_CONFIGURATION_H
