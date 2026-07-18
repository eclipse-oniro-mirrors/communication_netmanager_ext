/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef NET_FIREWALL_PARAM_CHECK_H
#define NET_FIREWALL_PARAM_CHECK_H

#include <cstdint>
#include <string>

#include "cxx.h"
#include "netfirewall_common.h"
#include "wrapper.rs.h"

namespace OHOS {
namespace NetManagerAni {

class NetFirewallParamCheck {
public:
    static int32_t CheckFirewallRulePolicy(const NetFirewallPolicyAni &policyAni);

    static int32_t CheckFirewallRule(const NetFirewallRuleAni &ruleAni);

    static int32_t CheckUpdateFirewallRule(const NetFirewallRuleAni &ruleAni);

private:
    static int32_t CheckRuleBaseParam(const NetFirewallRuleAni &ruleAni,
        NetManagerStandard::NetFirewallRuleType ruleType);

    static int32_t CheckRuleStringParam(const NetFirewallRuleAni &ruleAni);

    static int32_t CheckRuleObjectParams(const NetFirewallRuleAni &ruleAni,
        NetManagerStandard::NetFirewallRuleType ruleType);

    static int32_t CheckFirewallDns(const NetFirewallDnsParamAni &dnsAni);

    static int32_t CheckIpList(const rust::Vec<NetFirewallIpParamsAni> &ipsAni);

    static int32_t CheckPortList(const rust::Vec<NetFirewallPortParamAni> &portsAni);

    static int32_t CheckDomainList(const rust::Vec<NetFirewallDomainParamAni> &domainsAni);

    /**
     * Verify IP address
     *
     * @param ipStr string of ip
     * @return Returns 0 success. Otherwise fail
     */
    static int32_t CheckIpAddress(const std::string &ipStr);

    /**
     * Verify IP address
     *
     * @param ipStr string of ip
     * @param family IPV4 or IPV6
     * @return Returns 0 success. Otherwise fail
     */
    static int32_t CheckIpAddress(const std::string &ipStr, const int32_t family);

    /**
     * Verify IP segment address
     *
     * @param startIp Starting IP
     * @param endIp Terminate IP
     * @param family IPV4 or IPV6
     * @return Success returns true, otherwise returns false
     */
    static bool CheckIpAddress(const std::string &startIp, const std::string &endIp, const int32_t family);

    /**
     * Verify if the given IP is IPv4
     *
     * @param ipV4 string of ip
     * @return Returns 0 success. Otherwise fail
     */
    static int32_t CheckIpV4(const std::string &ipV4);

    /**
     * Verify if the given IP is IPv6
     *
     * @param ipV6 string of ip
     * @return Returns 0 success. Otherwise fail
     */
    static int32_t CheckIpV6(const std::string &ipV6);

    static int32_t CheckSingeIp(const NetFirewallIpParamsAni &ipAni, int32_t family);

    static int32_t CheckMultipleIp(const NetFirewallIpParamsAni &ipAni, int32_t family);

    static int32_t CheckInterfaceName(const std::string &interfaceName);
};
} // namespace NetManagerAni
} // namespace OHOS

#endif // NET_FIREWALL_PARAM_CHECK_H
