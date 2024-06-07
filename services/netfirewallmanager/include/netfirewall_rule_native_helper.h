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

#ifndef NET_FIREWALL_RULE_NATIVE_HELPER_H
#define NET_FIREWALL_RULE_NATIVE_HELPER_H

#include <string>
#include <mutex>

#include "netfirewall_common.h"

namespace OHOS {
namespace NetManagerStandard {
class NetFirewallRuleNativeHelper {
public:
    static std::shared_ptr<NetFirewallRuleNativeHelper> GetInstance();
    NetFirewallRuleNativeHelper();
    ~NetFirewallRuleNativeHelper();

    /**
     * Add firewall rules to bpf maps
     *
     * @param ruleList list of NetFirewallIpRule
     * @param isFinish transmit finish or not
     * @return 0 if success or -1 if an error occurred
     */
    int32_t AddFirewallIpRules(const std::vector<sptr<NetFirewallIpRule>> &ruleList, bool isFinish);

    /**
     * Update firewall rules to bpf maps
     *
     * @param rule list of NetFirewallIpRule
     * @return 0 if success or -1 if an error occurred
     */
    int32_t UpdateFirewallIpRule(const sptr<NetFirewallIpRule> &rule);

    /**
     * Add firewall domain rules
     *
     * @param ruleList list of NetFirewallDomainRule
     * @param isFinish transmit finish or not
     * @return 0 if success or -1 if an error occurred
     */
    int32_t AddFirewallDomainRules(const std::vector<sptr<NetFirewallDomainRule>> &ruleList, bool isFinish);

    /**
     * Update firewall domain rules
     *
     * @param rule list of NetFirewallDomainRule
     * @return 0 if success or -1 if an error occurred
     */
    int32_t UpdateFirewallDomainRules(const std::vector<sptr<NetFirewallDomainRule>> &ruleList);

    /**
     * Delete firewall rules
     *
     * @param ruleIds list of NetFirewallRule ids
     * @return 0 if success or -1 if an error occurred
     */
    int32_t DeleteFirewallRules(NetFirewallRuleType type, const std::vector<int32_t> &ruleIds);

    /**
     * Set firewall rules to bpf maps
     *
     * @param ruleList list of NetFirewallIpRule
     * @return 0 if success or -1 if an error occurred
     */
    int32_t SetFirewallIpRules(const std::vector<sptr<NetFirewallIpRule>> &ruleList);

    /**
     * Set firewall default action
     *
     * @param inDefault  Default action of NetFirewallRuleDirection:RULE_IN
     * @param outDefault Default action of NetFirewallRuleDirection:RULE_OUT
     * @return 0 if success or -1 if an error occurred
     */
    int32_t SetFirewallDefaultAction(FirewallRuleAction inDefault, FirewallRuleAction outDefault);

    /* *
     * Clear firewall rules by type
     *
     * @param type ip, dns, domain, all
     * @return 0 if success or -1 if an error occurred
     */
    int32_t ClearFirewallRules(NetFirewallRuleType type);

    /**
     * Set the Firewall DNS rules
     *
     * @param ruleList firewall rules
     * @return 0 if success or-1 if an error occurred
     */
    int32_t SetFirewallDnsRules(const std::vector<sptr<NetFirewallDnsRule>> &ruleList);

    /**
     * Set the Firewall domain rules
     *
     * @param  ruleList firewall rules
     * @return 0 if success or-1 if an error occurred
     */
    int32_t SetFirewallDomainRules(const std::vector<sptr<NetFirewallDomainRule>> &ruleList);

    /**
     * Set the Firewall current user id
     *
     * @param  userId firewall user id
     * @return 0 if success or-1 if an error occurred
     */
    int32_t SetCurrentUserId(int32_t userId);

private:
    std::mutex callNetSysController_;
    static std::shared_ptr<NetFirewallRuleNativeHelper> instance_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif /* NET_FIREWALL_RULE_NATIVE_HELPER_H */
