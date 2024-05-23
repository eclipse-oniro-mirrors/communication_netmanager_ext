/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

    /*
     * Get default netfirewall rules
     *
     * @param ruleList Netfirewall rule list
     * @return False or true
     */
    static bool GetDefaultRules(std::vector<NetFirewallRule> &ruleList);

    /*
     * Get configure value
     *
     * @param ruleList Netfirewall rule list
     * @return False or true
     */
    static bool GetConfigValue(std::vector<NetFirewallRule> &ruleList);

    /*
     * Get intercept log rotation parameter from jason file
     *
     * @param params Intercept log rotation parameters
     * @return False or true
     */
    static bool GetInterceptLogParams(InterceptLogRotationParam &params);

private:
    /*
     * Parsing netfirewall rules in JSON.
     *
     * @param rule Netfirewall rule
     * @param mem Memory
     * @return void
     */
    static void ConvertFirewallRuleToConfig(NetFirewallRule &rule, const cJSON * const mem);

    /*
     * Parsing ip parameters in JSON
     *
     * @param rule Netfirewall ip parameter rule
     * @param mem Memory
     * @return void
     */
    static void ConvertIpParamToConfig(NetFirewallIpParam &rule, const cJSON * const mem);

    /*
     * Parsing port parameters in JSON
     *
     * @param rule Netfirewall port parameter rule
     * @param mem Memory
     * @return void
     */
    static void ConvertPortParamToConfig(NetFirewallPortParam &rule, const cJSON * const mem);

    /*
     * Parsing domain name parameters in JSON
     *
     * @param rule Netfirewall domain name parameter rule
     * @param mem Memory
     * @return void
     */
    static void ConvertDomainParamToConfig(NetFirewallDomainParam &rule, const cJSON * const mem);

    /*
     * Parsing dns parameters in JSON
     *
     * @param rule Netfirewall dns parameter rule
     * @param mem Memory
     * @return void
     */
    static void ConvertDnsParamToConfig(NetFirewallDnsParam &rule, const cJSON * const mem);

    /*
     * Read JSON file
     *
     * @param filePath Json file path
     * @return String
     */
    static std::string ReadJsonFile(const std::string &filePath);

    /*
     * Parse ip list
     *
     * @param ipParamlist Ip paramter list
     * @param mem Memory
     * @param jsonKey json key
     * @return void
     */
    static void ParseIpList(std::vector<NetFirewallIpParam> &ipParamlist, const cJSON * const mem,
        const std::string jsonKey);

    /*
     * Parse port list
     *
     * @param portParamlist Port paramter list
     * @param mem Memory
     * @param jsonKey Json key
     * @return void
     */
    static void ParsePortList(std::vector<NetFirewallPortParam> &portParamlist, const cJSON * const mem,
        const std::string jsonKey);

    /*
     * Parse domain list
     *
     * @param domain name paramter list
     * @param mem Memory
     * @param jsonKey Json key
     * @return void
     */
    static void ParseDomainList(std::vector<NetFirewallDomainParam> &domainParamlist, const cJSON * const mem,
        const std::string jsonKey);

    /*
     * Parse dns object
     *
     * @param dnsParam Dns paramter list
     * @param mem Memory
     * @param jsonKey Json key
     * @return void
     */
    static void ParseDnsObject(NetFirewallDnsParam &dnsParam, const cJSON * const mem, const std::string jsonKey);

    /*
     * Parse list object
     *
     * @param rule Netfirwall rule
     * @param mem Memory
     * @return void
     */
    static void ParseListObject(NetFirewallRule &rule, const cJSON * const mem);

    /*
     * Parse intercept log rotation config in JASON
     *
     * @param param Intercept log rotation parameter
     * @param mem Memory
     * @return void
     */
    static void ConvertInteceptLogParamToConfig(InterceptLogRotationParam &param,
                                                const cJSON * const mem);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_FIREWALL_ANALYSIS_JSON_H
