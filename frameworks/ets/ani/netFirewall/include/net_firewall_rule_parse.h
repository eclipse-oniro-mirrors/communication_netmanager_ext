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

#ifndef NET_FIREWALL_RULE_PARSE_H
#define NET_FIREWALL_RULE_PARSE_H

#include <cstdint>
#include <string>
#include <vector>

#include "cxx.h"
#include "netfirewall_common.h"
#include "wrapper.rs.h"

namespace OHOS {
namespace NetManagerAni {
using namespace NetManagerStandard;

class NetFirewallRuleParse {
public:
    static void ParseRuleParams(const NetFirewallRuleAni &ruleAni, const sptr<NetFirewallRule> &rule);

    static void ConvertInterceptRecordPage(const sptr<InterceptRecordPage> &pageInfo,
        InterceptedRecordPageAni &pageAni);

    static void ConvertNetFirewallRule(const NetFirewallRule &rule, NetFirewallRuleAni &ffi);

private:
    static void ConvertInterceptRecord(const InterceptRecord &record, InterceptedRecordAni &recordAni);

    static void ParseIpList(const rust::Vec<NetFirewallIpParamsAni> &ipsAni, std::vector<NetFirewallIpParam> &ips);

    static void ParsePortList(const rust::Vec<NetFirewallPortParamAni> &portsAni,
        std::vector<NetFirewallPortParam> &ports);

    static void ParseDomainList(const rust::Vec<NetFirewallDomainParamAni> &domainsAni,
        std::vector<NetFirewallDomainParam> &domains);

    static void ParseDns(const NetFirewallDnsParamAni &dnsAni, NetFirewallDnsParam &param);
};
} // namespace NetManagerAni
} // namespace OHOS

#endif // NET_FIREWALL_RULE_PARSE_H
