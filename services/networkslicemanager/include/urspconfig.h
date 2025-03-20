/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
    std::unordered_map<std::string, std::vector<UrspRule>> mPreConfigUrspMap;
    std::unordered_map<std::string, std::vector<UrspRule>> mUePolicyMap;
    int mUrspVersion;
    short sSuccUrspVersion = 0;
    std::unordered_map<int, std::vector<RouteSelectionDescriptor>> mImsRsdsMap;
    bool mIsTrafficDescriptorIncludeIms;
    void ParseConfig();
    void ParseAllUePolicy(xmlDocPtr doc);
    void ParseUePolicy(xmlNodePtr curNode);
    UrspRule ParsePreConfigUrsp(xmlNodePtr node);
    void ParseTrafficDescriptor(xmlNodePtr node, TrafficDescriptor &trafficDescriptor);
    void ParseTrafficDescriptorEx(xmlNodePtr node, TrafficDescriptor &trafficDescriptor, std::string attrValue);
    void ParseRouteSelectionDescriptor(xmlNodePtr node,
        std::vector<RouteSelectionDescriptor> &routeSelectionDescriptors);
    void ParseRouteRule(xmlNodePtr node, RouteSelectionDescriptor &routeSelectionDescriptors);
    bool ParseOsAppId(xmlNodePtr node, TrafficDescriptor& trafficDescriptor);
    bool ParseIpv4Addr(xmlNodePtr node, TrafficDescriptor& trafficDescriptor);
    bool ParseIpv6Addr(xmlNodePtr node, TrafficDescriptor& trafficDescriptor);
    bool ParseRemotePortRange(xmlNodePtr node, TrafficDescriptor& trafficDescriptor);
    bool ParseConnectionCapabilities(xmlNodePtr node, TrafficDescriptor& trafficDescriptor);
    void setUrspRules(const std::string& plmn, std::vector<UrspRule>& urspRules);
    void ClearUrspRules();
    void UrspRuleSort(std::vector<UrspRule>& urspRules);
    void SaveTrafficDescriptorWhiteListToDb();
    void FillTrafficDescriptorWhiteList(std::shared_ptr<TrafficDescriptorWhiteList>& whiteList,
        const std::vector<UrspRule>& urspRules);
    void SetUrspVersion(int urspVersion);
    bool DecodeUrspRules(int inputLen, int& startIndex, std::vector<uint8_t> buffer, std::vector<UrspRule>& urspRules);
    short GetSuccUrspVersion();
    bool DecodeUrspByVersion(int inputLen, int& startIndex, std::vector<uint8_t> buffer,
        std::vector<UrspRule>& urspRules, short version);
    int GetSubLenByversion(int& startIndex, std::vector<uint8_t> buffer, short version);
    int GetLenBytesByversion(short version);
    bool DecodeUrspRule(int inputLen, int& startIndex, std::vector<uint8_t> buffer,
        std::vector<UrspRule>& urspRules, short version);
    bool DecodeUrspRuleExtra(int inputLen, int& startIndex, std::vector<uint8_t> buffer,
        UrspRule& urspRule, std::vector<UrspRule>& urspRules, short version);
    int DecodeTrafficDescriptor(int inputLen, int& startIndex,
        std::vector<uint8_t> buffer, TrafficDescriptor& trafficDescriptor);
    int DecodeRouteRuleList(int inputLen, int& startIndex, std::vector<uint8_t> buffer,
        UrspRule& urspRule, short version);
    int DecodeRouteRule(int& startIndex, int inputLen,
        std::vector<uint8_t> buffer, RouteSelectionDescriptor& routeRule);
    int DecodeOsIdOsAppId(int& startIndex, std::vector<uint8_t> buffer,
        TrafficDescriptor& trafficDescriptor, bool isAppIdOnly);
    int DecodeIpv4Addr(int& startIndex, std::vector<uint8_t> buffer, TrafficDescriptor& trafficDescriptor);
    int DecodeIpv6Addr(int& startIndex, std::vector<uint8_t> buffer, TrafficDescriptor& trafficDescriptor);
    int DecodeProtocolId(int& startIndex, std::vector<uint8_t> buffer, TrafficDescriptor& trafficDescriptor);
    void ResetImsRsdsMap();
    int DecodeTrafficDescriptorExtra(int inputLen, int& startIndex, int type,
        std::vector<uint8_t> buffer, TrafficDescriptor& trafficDescriptor, int initBufferRemaining);
    int DecodeSingleRemotePort(int& startIndex, std::vector<uint8_t> buffer, TrafficDescriptor& trafficDescriptor);
    int DecodeRemotePortRange(int& startIndex, std::vector<uint8_t> buffer, TrafficDescriptor& trafficDescriptor);
    int DecodeTrafficDescriptorDnn(int& startIndex, std::vector<uint8_t> buffer,
        TrafficDescriptor& trafficDescriptor);
    int DecodeFqdn(int& startIndex, std::vector<uint8_t> buffer, TrafficDescriptor& trafficDescriptor);
    int DecodeConnectionCapabilities(int& startIndex, std::vector<uint8_t> buffer,
        TrafficDescriptor& trafficDescriptor);
    std::string DecodeSubDnns(int& startIndex, std::vector<uint8_t> buffer, uint8_t stringLen);
    int DecodeSscMode(int& startIndex, std::vector<uint8_t> buffer, RouteSelectionDescriptor &routeRule);
    int DecodeSnssai(int& startIndex, std::vector<uint8_t> buffer, RouteSelectionDescriptor &routeRule);
    int DecodeDnn(int& startIndex, std::vector<uint8_t> buffer, RouteSelectionDescriptor &routeRule);
    int DecodePduSessionType(int& startIndex, std::vector<uint8_t> buffer, RouteSelectionDescriptor &routeRule);
    int TransferPduSessionTypeToHal(int pduSessionType);
    int DecodePreferredAccessType(int& startIndex, std::vector<uint8_t> buffer, RouteSelectionDescriptor &routeRule);
    void SndPreImsRsdList();
    std::vector<uint8_t> GetImsRsdList();
    std::vector<RouteSelectionDescriptor> SortRsdsMap(
        std::unordered_map<int, std::vector<RouteSelectionDescriptor>> rsdsMap);
    std::vector<uint8_t> ConvertRsdList2BufferArray(
        std::unordered_map<int, std::vector<RouteSelectionDescriptor>> rsdsMap);
    void PutRsdListInfo(std::vector<uint8_t>& buffer, std::vector<RouteSelectionDescriptor> rsdList);
    void PutDnnsInfo(std::vector<uint8_t>& buffer, RouteSelectionDescriptor rsd);
    void PutNssaisInfo(std::vector<uint8_t>& buffer, RouteSelectionDescriptor rsd);
    int ConvertPduTypeFromHal2Imsa(int halPduType);
    std::vector<uint8_t> NotifyImsaDelRsdInfo();
    void PutInvalidValue (std::vector<uint8_t>& buffer, int num);
    short CalculateRsdListLen();
    short CalculateRsdLen();
    int SetBitOpt(int num, int position);
    bool hasAvailableUrspRule();
    bool SliceNetworkSelection(SelectedRouteDescriptor& routeRule, std::string plmn, AppDescriptor appDescriptor);
    void FillTrafficDescriptor(TrafficDescriptor urspTrafficDescriptor,
        AppDescriptor appDescriptor, SelectedRouteDescriptor& routeRule);
    bool FindAvailableRouteRule(
        const std::vector<RouteSelectionDescriptor>& routeSelectionDescriptors, SelectedRouteDescriptor& routeRule);
    bool FindAvailableSnssaiAndDnn(const RouteSelectionDescriptor& routeSelectionDescriptor,
                               SelectedRouteDescriptor& routeRule);
    bool FindAvailableSnssai(const RouteSelectionDescriptor& routeSelectionDescriptor,
                        SelectedRouteDescriptor& routeRule);
    bool FindAvailableDnn(const RouteSelectionDescriptor& routeSelectionDescriptor,
                      SelectedRouteDescriptor& routeRule);
    void FillOsAppIds(TrafficDescriptor urspTrafficDescriptor,
                   SelectedRouteDescriptor& routeRule);
    void FillIpv4Addrs(TrafficDescriptor urspTrafficDescriptor,
                    SelectedRouteDescriptor& routeRule);
    void FillIpv6Addrs(TrafficDescriptor urspTrafficDescriptor,
                   SelectedRouteDescriptor& routeRule);
    void FillProtocolIds(TrafficDescriptor urspTrafficDescriptor,
                   SelectedRouteDescriptor& routeRule);
    void FillRemotePorts(TrafficDescriptor urspTrafficDescriptor,
                   SelectedRouteDescriptor& routeRule);
    bool isTrafficDescriptorMatch(TrafficDescriptor urspTrafficDescriptor,
        AppDescriptor appDescriptor);
    bool isIpThreeTuplesInTrafficDescriptor(TrafficDescriptor urspTrafficDescriptor,
        AppDescriptor appDescriptor);
    bool isIpThreeTuplesInWhiteList(std::string plmn, AppDescriptor appDescriptor);
    bool isOsAppIdMatch(TrafficDescriptor urspTrafficDescriptor, AppDescriptor appDescriptor);
    bool isIpv4AddrMatch(TrafficDescriptor urspTrafficDescriptor, AppDescriptor appDescriptor);
    bool isIpv6AddrMatch(TrafficDescriptor urspTrafficDescriptor, AppDescriptor appDescriptor);
    bool isRemotePortMatch(TrafficDescriptor urspTrafficDescriptor, AppDescriptor appDescriptor);
    SelectedRouteDescriptor GetMatchAllUrspRule(const std::string& plmn);
    void DumpUePolicyMap();
    void DumpPreConfigUrspMap();
    void DumptrafficDescriptor(const TrafficDescriptor& trafficDescriptor);
    void DumpRouteSelectionDescriptors(const std::vector<RouteSelectionDescriptor>& routeSelectionDescriptors);
private:
    UrspConfig();
};

} // namespace NetManagerStandard
} // namespace OHOS
#endif  // URSPCONFIG_H

