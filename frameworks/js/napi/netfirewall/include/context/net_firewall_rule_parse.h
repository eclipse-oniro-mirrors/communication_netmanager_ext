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
#ifndef NET_FIREWALL_RULE_PARSE_H
#define NET_FIREWALL_RULE_PARSE_H

#include "netfirewall_common.h"
#include <napi/native_api.h>

namespace OHOS {
namespace NetManagerStandard {
class NetFirewallRuleParse {
public:
    // Parsing firewall rule input parameters
    static void ParseRuleParams(napi_env env, napi_value object, const sptr<NetFirewallRule> &rule);

    // Set firewall parameters
    static void SetRuleParams(napi_env env, napi_value object, const NetFirewallRule &rule);

    // Parsing Paging Information Parameters
    static int32_t ParsePageParam(napi_env env, napi_value object, const sptr<RequestParam> &param, bool isRule);

private:
    // Parse IP list
    static void ParseIpList(napi_env env, napi_value params, std::vector<NetFirewallIpParam> &list);

    // Parse port list
    static void ParsePortList(napi_env env, napi_value params, std::vector<NetFirewallPortParam> &list);

    static void ParseDomainList(napi_env env, napi_value params, std::vector<NetFirewallDomainParam> &list);

    static void SetIpList(napi_env env, napi_value object, const std::string &propertyName,
        const std::vector<NetFirewallIpParam> &list);

    static void SetPortList(napi_env env, napi_value object, const std::string &propertyName,
        const std::vector<NetFirewallPortParam> &list);

    // Set domain name list
    static void SetDomainList(napi_env env, napi_value object, const std::string &propertyName,
        const std::vector<NetFirewallDomainParam> &list);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_FIREWALL_RULE_PARSE_H
