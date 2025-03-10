/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef URSPCONFIG_H
#define URSPCONFIG_H

#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <algorithm>
#include "refbase.h"
#include "allowednssaiconfig.h"
#include "networksliceutil.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {

struct TrafficDescriptorWhiteList {
    std::string osAppIds = "";
    std::string dnns = "";
    std::string fqdns = "";
    std::string cct = "";
};

class TrafficDescriptor {
public:
    bool isMatchAll;
    std::vector<OsAppId> osAppIds;
    std::vector<Ipv4Addr> ipv4Addrs;
    std::vector<Ipv6Addr> ipv6Addrs;
    std::vector<int> protocolIds;
    std::vector<int> singleRemotePorts;
    std::vector<RemotePortRange> remotePortRanges;
    std::vector<std::string> dnns;
    std::vector<std::string> fqdns;
    std::vector<int> connectionCapabilities;

    nlohmann::json to_json() const
    {
        nlohmann::json tdJson = nlohmann::json::object({
            {"isMatchAll", isMatchAll},
            {"protocolIds", protocolIds},
            {"singleRemotePorts", singleRemotePorts},
            {"dnns", dnns},
            {"fqdns", fqdns},
            {"connectionCapabilities", connectionCapabilities}
        });

        for (const auto& osAppId : osAppIds) {
            tdJson["osAppIds"].push_back(osAppId.to_json());
        }
        for (const auto& ipv4Addr : ipv4Addrs) {
            tdJson["ipv4Addrs"].push_back(ipv4Addr.to_json());
        }
        for (const auto& ipv6Addr : ipv6Addrs) {
            tdJson["ipv6Addrs"].push_back(ipv6Addr.to_json());
        }
        for (const auto& remotePortRange : remotePortRanges) {
            tdJson["remotePortRanges"].push_back(remotePortRange.to_json());
        }
        return tdJson;
    }
    void from_json(const nlohmann::json& jsonDescriptor)
    {
        if (jsonDescriptor.find("isMatchAll") != jsonDescriptor.end()) {
            this->isMatchAll = jsonDescriptor.at("isMatchAll").get<bool>();
        }
        if (jsonDescriptor.find("osAppIds") != jsonDescriptor.end()) {
            NETMGR_EXT_LOG_I("TrafficDescriptor find osAppIds");
            const nlohmann::json& osAppIdsJson = jsonDescriptor.at("osAppIds");
            this->osAppIds.clear();
            for (const auto& osAppIdJson : osAppIdsJson) {
                OsAppId osAppId;
                osAppId.from_json(osAppIdJson);
                this->osAppIds.push_back(osAppId);
            }
        }
        if (jsonDescriptor.find("ipv4Addrs") != jsonDescriptor.end()) {
            const nlohmann::json& ipv4AddrsJson = jsonDescriptor.at("ipv4Addrs");
            this->ipv4Addrs.clear();
            for (const auto& ipv4AddrJson : ipv4AddrsJson) {
                Ipv4Addr ipv4Addr;
                ipv4Addr.from_json(ipv4AddrJson);
                this->ipv4Addrs.push_back(ipv4Addr);
            }
        }
        if (jsonDescriptor.find("ipv6Addrs") != jsonDescriptor.end()) {
            const nlohmann::json& ipv6AddrsJson = jsonDescriptor.at("ipv6Addrs");
            this->ipv6Addrs.clear();
            for (const auto& ipv6AddrJson : ipv6AddrsJson) {
                Ipv6Addr ipv6Addr;
                ipv6Addr.from_json(ipv6AddrJson);
                this->ipv6Addrs.push_back(ipv6Addr);
            }
        }
        from_json_ex(jsonDescriptor);
    }

    void from_json_ex(const nlohmann::json& jsonDescriptor)
    {
        if (jsonDescriptor.find("protocolIds") != jsonDescriptor.end()) {
            const nlohmann::json& protocolIdsJson = jsonDescriptor.at("protocolIds");
            this->protocolIds.clear();
            for (const auto& protocolIdJson : protocolIdsJson) {
                this->protocolIds.push_back(protocolIdJson.get<int>());
            }
        }
        if (jsonDescriptor.find("singleRemotePorts") != jsonDescriptor.end()) {
            const nlohmann::json& singleRemotePortsJson = jsonDescriptor.at("singleRemotePorts");
            this->singleRemotePorts.clear();
            for (const auto& singleRemotePortJson : singleRemotePortsJson) {
                this->singleRemotePorts.push_back(singleRemotePortJson.get<int>());
            }
        }
        if (jsonDescriptor.find("remotePortRanges") != jsonDescriptor.end()) {
            const nlohmann::json& remotePortRangesJson = jsonDescriptor.at("remotePortRanges");
            this->remotePortRanges.clear();
            for (const auto& remotePortRangeJson : remotePortRangesJson) {
                RemotePortRange remotePortRange;
                remotePortRange.from_json(remotePortRangeJson);
                this->remotePortRanges.push_back(remotePortRange);
            }
        }
        if (jsonDescriptor.find("dnns") != jsonDescriptor.end()) {
            const nlohmann::json& dnnsJson = jsonDescriptor.at("dnns");
            this->dnns.clear();
            for (const auto& dnnJson : dnnsJson) {
                this->dnns.push_back(dnnJson.get<std::string>());
            }
        }
        if (jsonDescriptor.find("fqdns") != jsonDescriptor.end()) {
            const nlohmann::json& fqdnsJson = jsonDescriptor.at("fqdns");
            this->fqdns.clear();
            for (const auto& fqdnJson : fqdnsJson) {
                this->fqdns.push_back(fqdnJson.get<std::string>());
            }
        }
        if (jsonDescriptor.find("connectionCapabilities") != jsonDescriptor.end()) {
            const nlohmann::json& connectionCapabilitiesJson = jsonDescriptor.at("connectionCapabilities");
            this->connectionCapabilities.clear();
            for (const auto& connectionCapabilityJson : connectionCapabilitiesJson) {
                this->connectionCapabilities.push_back(connectionCapabilityJson.get<int>());
            }
        }
    }
};

class RouteSelectionDescriptor {
public:
    int routePrecedence;
    int pduSessionType;
    uint8_t sscMode;
    std::vector<Snssai> snssais;
    std::vector<std::string> dnns;
    
    nlohmann::json to_json() const
    {
        nlohmann::json rsdJson = nlohmann::json::object({
            {"routePrecedence", routePrecedence},
            {"pduSessionType", pduSessionType},
            {"sscMode", sscMode},
            {"dnns", dnns}
        });
        for (const auto& snssai : snssais) {
            rsdJson["snssais"].push_back(snssai.to_json());
        }
        return rsdJson;
    }

    void from_json(const nlohmann::json& jsonDescriptor)
    {
        if (jsonDescriptor.find("routePrecedence") != jsonDescriptor.end()) {
            this->routePrecedence = jsonDescriptor.at("routePrecedence").get<int>();
        }
        if (jsonDescriptor.find("pduSessionType") != jsonDescriptor.end()) {
            this->pduSessionType = jsonDescriptor.at("pduSessionType").get<int>();
        }
        if (jsonDescriptor.find("sscMode") != jsonDescriptor.end()) {
            this->sscMode = jsonDescriptor.at("sscMode").get<uint8_t>();
        }
        if (jsonDescriptor.find("snssais") != jsonDescriptor.end()) {
            const nlohmann::json& snssaisJson = jsonDescriptor.at("snssais");
            this->snssais.clear();
            for (const auto& snssaiJson : snssaisJson) {
                Snssai snssai;
                snssai.from_json(snssaiJson);
                this->snssais.push_back(snssai);
            }
        }
        if (jsonDescriptor.find("dnns") != jsonDescriptor.end()) {
            const nlohmann::json& dnnsJson = jsonDescriptor.at("dnns");
            this->dnns.clear();
            for (const auto& dnnJson : dnnsJson) {
                this->dnns.push_back(dnnJson.get<std::string>());
            }
        }
    }
};

class UrspRule {
public:
    int32_t urspPrecedence;
    TrafficDescriptor trafficDescriptor;
    std::vector<RouteSelectionDescriptor> routeSelectionDescriptors;

    nlohmann::json to_json() const
    {
        nlohmann::json urspruleJson = nlohmann::json::object({
            {"urspPrecedence", urspPrecedence},
            {"trafficDescriptor", trafficDescriptor.to_json()},
        });
        for (const auto& routeSelectionDescriptor : routeSelectionDescriptors) {
            urspruleJson["routeSelectionDescriptors"].push_back(routeSelectionDescriptor.to_json());
        }
        return urspruleJson;
    }
    void from_json(const nlohmann::json& urspruleJson)
    {
        if (urspruleJson.find("urspPrecedence") != urspruleJson.end()) {
            this->urspPrecedence = urspruleJson.at("urspPrecedence").get<int32_t>();
        }

        if (urspruleJson.find("trafficDescriptor") != urspruleJson.end()) {
            const nlohmann::json& trafficDescriptorJson = urspruleJson.at("trafficDescriptor");
            TrafficDescriptor td;
            td.from_json(trafficDescriptorJson);
            this->trafficDescriptor = td;
        }

        if (urspruleJson.find("routeSelectionDescriptors") != urspruleJson.end()) {
            const nlohmann::json& routeSelectionDescriptorsJson = urspruleJson.at("routeSelectionDescriptors");
            routeSelectionDescriptors.clear();
            for (const auto& descriptorJson : routeSelectionDescriptorsJson) {
                RouteSelectionDescriptor rsd;
                rsd.from_json(descriptorJson);
                this->routeSelectionDescriptors.push_back(rsd);
            }
        }
    }
};

class UrspConfig {
public:
    static UrspConfig& GetInstance();
    ~UrspConfig() = default;
    bool SliceNetworkSelection(SelectedRouteDescriptor& routeRule, std::string plmn, AppDescriptor appDescriptor);
    bool isIpThreeTuplesInWhiteList(std::string plmn, AppDescriptor appDescriptor);
private:
    UrspConfig();
};


} // namespace NetManagerStandard
} // namespace OHOS
#endif  // URSPCONFIG_H
