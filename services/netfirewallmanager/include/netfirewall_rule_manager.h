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

#ifndef NET_FIREWALL_RULES_MANAGER_H
#define NET_FIREWALL_RULES_MANAGER_H

#include <string>
#include <shared_mutex>

#include "netfirewall_common.h"

namespace OHOS {
namespace NetManagerStandard {
class NetFirewallRuleManager {
public:
    static std::shared_ptr<NetFirewallRuleManager> GetInstance();
    NetFirewallRuleManager();
    ~NetFirewallRuleManager();

    /**
     * Add firewall rules
     *
     * @param rule Firewall rules
     * @param ruleId Rule id genarated by database
     * @return Returns 0 success. Otherwise fail
     */
    int32_t AddNetFirewallRule(const sptr<NetFirewallRule> &rule, int32_t &ruleId);

    /**
     * Modify firewall rules
     *
     * @param rule Firewall rules
     * @return Returns 0 success. Otherwise fail
     */
    int32_t UpdateNetFirewallRule(const sptr<NetFirewallRule> &rule);

    /**
     * Delete firewall rules
     *
     * @param userId User ID
     * @param ruleId Rule ID
     * @return Returns 0 success. Otherwise fail
     */
    int32_t DeleteNetFirewallRule(const int32_t userId, const int32_t ruleId);

    /**
     * Get all firewall rules
     *
     * @param userId User ID
     * @param requestParam Paging in parameter information
     * @param info Paging data information
     * @return Returns 0 success. Otherwise fail
     */
    int32_t GetNetFirewallRules(const int32_t userId, const sptr<RequestParam> &requestParam,
        sptr<FirewallRulePage> &info);

    /**
     * Get information about the specified rule ID
     *
     * @param ruleId Rule ID
     * @param rule Return to firewall rules
     * @return Returns 0 success. Otherwise fail
     */
    int32_t GetNetFirewallRule(const int32_t userId, const int32_t ruleId, sptr<NetFirewallRule> &rule);

    int32_t DeleteNetFirewallRuleByUserId(const int32_t userId);

    int32_t DeleteNetFirewallRuleByAppId(const int32_t appUid);

    int32_t GetEnabledNetFirewallRules(const int32_t userId, std::vector<NetFirewallRule> &ruleList,
        NetFirewallRuleType type = NetFirewallRuleType::RULE_ALL);

    int32_t AddDefaultNetFirewallRule(int32_t userId);

    void ChangeUserRuleSize(const int32_t userId);

    int32_t OpenOrCloseNativeFirewall(bool isOpen);

    uint64_t GetCurrentSetRuleSecond();

    int64_t GetLastRulePushResult();

private:
    int32_t AddNetFirewallRule(const sptr<NetFirewallRule> &rule, bool isNotify, int32_t &ruleId);

    int32_t CheckUserExist(const int32_t userId);

    int32_t CheckRuleExist(const int32_t ruleId, NetFirewallRule &oldRule);

    int32_t GetAllRuleConstraint(const int32_t userId);

    int32_t CheckRuleConstraint(const sptr<NetFirewallRule> &rule);

    bool CheckAccountExist(int32_t userId);

    bool ExtractIpRules(const std::vector<NetFirewallRule> &rules);

    bool ExtractDomainRules(const std::vector<NetFirewallRule> &rules);

    bool ExtractDnsRules(const std::vector<NetFirewallRule> &rules);

    int32_t HandleIpTypeForDistributeRules(std::vector<NetFirewallRule> &rules);

    int32_t HandleDnsTypeForDistributeRules(std::vector<NetFirewallRule> &rules);

    int32_t HandleDomainTypeForDistributeRules(std::vector<NetFirewallRule> &rules);

    int32_t GetCurrentAccountId();

    int32_t SetRulesToNativeByType(const int32_t userId, const NetFirewallRuleType type);

    int32_t DistributeRulesToNative(NetFirewallRuleType type = NetFirewallRuleType::RULE_ALL);

    void SetNetFirewallDumpMessage(const int32_t result);

private:
    // Cache the current state
    int64_t allUserRule_ = 0;
    int64_t allUserDomain_ = 0;
    int64_t maxDefaultRuleSize_ = 0;
    std::shared_mutex setFirewallRuleMutex_;
    std::map<int32_t, int64_t> userRuleSize_;
    std::vector<sptr<NetFirewallIpRule>> ipRules_;
    std::vector<sptr<NetFirewallDnsRule>> dnsRules_;
    std::vector<sptr<NetFirewallDomainRule>> domainRules_;
    std::atomic<uint64_t> currentSetRuleSecond_ = 0;
    std::atomic<int64_t> lastRulePushResult_ = -1;
    static std::shared_ptr<NetFirewallRuleManager> instance_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif /* NET_FIREWALL_RULES_MANAGER_H */
