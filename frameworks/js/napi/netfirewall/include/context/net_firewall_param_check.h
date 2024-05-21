/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef NET_FIREWALL_PARAM_CHECK_H
#define NET_FIREWALL_PARAM_CHECK_H

#include "netfirewall_common.h"
#include <napi/native_api.h>

namespace OHOS {
namespace NetManagerStandard {
class NetFirewallParamCheck {
public:
    // Firewall state input verification
    static int32_t CheckFirewallRuleStatus(napi_env env, napi_value object);

    // Add firewall rule entry verification
    static int32_t CheckAddFirewallRule(napi_env env, napi_value object);

    // Update firewall rule entry verification
    static int32_t CheckUpdateFirewallRule(napi_env env, napi_value object);

private:
    // Verify firewall rules
    static int32_t CheckFirewallRule(napi_env env, napi_value object);

    // Verify DNS parameters in firewall rules
    static int32_t CheckFirewallDns(napi_env env, napi_value object);

    // Verify actions in firewall rules
    static int32_t CheckFirewallAction(napi_env env, napi_value object);

    // Verify the direction in firewall rules
    static int32_t CheckFirewallDirection(napi_env env, napi_value object);

    // Verify IP list
    static int32_t CheckIpList(napi_env env, napi_value object);

    static int32_t CheckPortList(napi_env env, napi_value object);

    static int32_t CheckDomainList(napi_env env, napi_value object);

    /*
     * Verify IP address
     *
     * @param ipStr string of ip
     * @return Error code
     *
     */
    static int32_t CheckIpAddress(const std::string &ipStr);

    /*
     * Verify IP address
     *
     * @param ipStr string of ip
     * @param family IPV4 or IPV6
     * @return Error code
     *
     */
    static int32_t CheckIpAddress(const std::string &ipStr, const int32_t family);

    /*
     * Verify IP segment address
     *
     * @param startIp Starting IP
     * @param endIp Terminate IP
     * @param family IPV4 or IPV6
     * @return Success returns true, otherwise returns false
     *
     */
    static bool CheckIpAddress(const std::string &startIp, const std::string &endIp, const int32_t family);

    /*
     * Verify if the given IP is IPv4
     *
     * @param ipV4 string of ip
     * @return Error code
     *
     */
    static int32_t CheckIpV4(const std::string &ipV4);

    /*
     * Verify if the given IP is IPv4
     *
     * @param ipV4 string of ip
     * @return Error code
     *
     */
    static int32_t CheckIpV6(const std::string &ipV6);

    // Single IP rule input verification
    static int32_t CheckSingeIp(napi_env env, napi_value valAttr, int32_t family);

    static int32_t CheckMultipleIp(napi_env env, napi_value valAttr, int32_t family);

    // Verify the value of the object attribute for the rule type
    static int32_t CheckRuleObjectParamValue(napi_env env, napi_value object, const std::string &propertyName);

    // Verify attributes of rule type object
    static int32_t CheckRuleObjectParams(napi_env env, napi_value object, const NetFirewallRuleType &type);

    // Verify firewall rule name
    static int32_t CheckRuleName(napi_env env, napi_value object);

    // Verify properties with rule type number
    static int32_t CheckRuleNumberParam(napi_env env, napi_value object, const std::string &propertyName);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_FIREWALL_PARAM_CHECK_H
