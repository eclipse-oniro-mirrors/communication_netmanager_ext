/*
 * Copyright (C) 2022-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NET_FIREWALL_ANALYSIS_JSON_H
#define NET_FIREWALL_ANALYSIS_JSON_H

#include "netfirewall_common.h"
#include "cJSON.h"

namespace OHOS {
namespace NetManagerStandard {
class NetFireWallAnalysisJson {
public:
    NetFireWallAnalysisJson() = default;
    ~NetFireWallAnalysisJson() = default;

    // Get default firewall rules
    static bool GetDefaultRules(std::vector<NetFirewallRule> &ruleList);

private:
    // Parsing firewall rules in JSON
    static void ConvertFirewallRuleToConfig(NetFirewallRule &rule, const cJSON * const mem);

    // Parsing IP parameters in JSON
    static void ConvertIpParamToConfig(NetFirewallIpParam &rule, const cJSON * const mem);

    static void ConvertPortParamToConfig(NetFirewallPortParam &rule, const cJSON * const mem);

    static void ConvertDomainParamToConfig(NetFirewallDomainParam &rule, const cJSON * const mem);

    static void ConvertDnsParamToConfig(NetFirewallDnsParam &rule, const cJSON * const mem);

    // Read JSON file
    static std::string ReadJsonFile(const std::string &filePath);

    static void ParseIpList(std::vector<NetFirewallIpParam> &ipParamlist, const cJSON * const mem,
        const std::string jsonKey);

    // Parse port list
    static void ParsePortList(std::vector<NetFirewallPortParam> &portParamlist, const cJSON * const mem,
        const std::string jsonKey);

    static void ParseDomainList(std::vector<NetFirewallDomainParam> &domainParamlist, const cJSON * const mem,
        const std::string jsonKey);

    static void ParseDnsObject(NetFirewallDnsParam &dnsParam, const cJSON * const mem, const std::string jsonKey);

    // Parse list object
    static void ParseListObject(NetFirewallRule &rule, const cJSON * const mem);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_FIREWALL_ANALYSIS_JSON_H
