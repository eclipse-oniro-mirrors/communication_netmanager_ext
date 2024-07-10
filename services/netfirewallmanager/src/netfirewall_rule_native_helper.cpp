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

#include "net_manager_constants.h"
#include "net_manager_ext_constants.h"
#include "netmanager_base_common_utils.h"
#include "netmgr_ext_log_wrapper.h"
#include "netfirewall_rule_native_helper.h"
#include "netsys_controller.h"

namespace OHOS {
namespace NetManagerStandard {
NetFirewallRuleNativeHelper &NetFirewallRuleNativeHelper::GetInstance()
{
    static NetFirewallRuleNativeHelper instance;
    return instance;
}

NetFirewallRuleNativeHelper::NetFirewallRuleNativeHelper()
{
    NETMGR_EXT_LOG_I("NetFirewallRuleNativeHelper()");
}

NetFirewallRuleNativeHelper::~NetFirewallRuleNativeHelper()
{
    NETMGR_EXT_LOG_I("~NetFirewallRuleNativeHelper()");
}

/**
 * Set firewall default action
 *
 * @param inDefault  Default action of NetFirewallRuleDirection:RULE_IN
 * @param outDefault Default action of NetFirewallRuleDirection:RULE_OUT
 * @return 0 if success or -1 if an error occurred
 */
int32_t NetFirewallRuleNativeHelper::SetFirewallDefaultAction(FirewallRuleAction inDefault,
    FirewallRuleAction outDefault)
{
    std::lock_guard<std::mutex> locker(callNetSysController_);
    return NetsysController::GetInstance().SetFirewallDefaultAction(inDefault, outDefault);
}

/**
 * Clear firewall rules by type
 *
 * @param type ip, dns, domain, all
 * @return 0 if success or -1 if an error occurred
 */
int32_t NetFirewallRuleNativeHelper::ClearFirewallRules(NetFirewallRuleType type)
{
    std::lock_guard<std::mutex> locker(callNetSysController_);
    return NetsysController::GetInstance().ClearFirewallRules(type);
}

/**
 * Set firewall rules to bpf maps
 *
 * @param ruleList list of NetFirewallIpRule
 * @return 0 if success or -1 if an error occurred
 */
int32_t NetFirewallRuleNativeHelper::SetFirewallIpRules(const std::vector<sptr<NetFirewallIpRule>> &ruleList)
{
    std::vector<sptr<NetFirewallBaseRule>> rules;
    rules.assign(ruleList.begin(), ruleList.end());
    return SetFirewallRulesInner(NetFirewallRuleType::RULE_IP, rules, FIREWALL_IPC_IP_RULE_PAGE_SIZE);
}

/**
 * Set the Firewall DNS rules
 *
 * @param ruleList firewall rules
 * @return 0 if success or-1 if an error occurred
 */
int32_t NetFirewallRuleNativeHelper::SetFirewallDnsRules(const std::vector<sptr<NetFirewallDnsRule>> &ruleList)
{
    std::vector<sptr<NetFirewallBaseRule>> rules;
    rules.assign(ruleList.begin(), ruleList.end());
    return SetFirewallRulesInner(NetFirewallRuleType::RULE_DNS, rules, FIREWALL_RULE_SIZE_MAX);
}

/**
 * Set the Firewall domain rules
 *
 * @param  ruleList firewall rules
 * @return 0 if success or-1 if an error occurred
 */
int32_t NetFirewallRuleNativeHelper::SetFirewallDomainRules(const std::vector<sptr<NetFirewallDomainRule>> &ruleList)
{
    std::vector<sptr<NetFirewallBaseRule>> rules;
    rules.assign(ruleList.begin(), ruleList.end());
    return SetFirewallRulesInner(NetFirewallRuleType::RULE_DOMAIN, rules, FIREWALL_IPC_DOMAIN_RULE_PAGE_SIZE);
}

int32_t NetFirewallRuleNativeHelper::SetFirewallRulesInner(NetFirewallRuleType type,
    const std::vector<sptr<NetFirewallBaseRule>> &ruleList, uint32_t pageSize)
{
    NETMGR_EXT_LOG_I("SetFirewallRulesInner: type=%{public}d ruleSize=%{public}zu pageSize=%{public}d", type,
        ruleList.size(), pageSize);
    std::lock_guard<std::mutex> locker(callNetSysController_);
    return NetsysController::GetInstance().SetFirewallRules(type, ruleList, true);
}

/**
 * Set the Firewall current user id
 *
 * @param  userId firewall user id
 * @return 0 if success or-1 if an error occurred
 */
int32_t NetFirewallRuleNativeHelper::SetCurrentUserId(int32_t userId)
{
    std::lock_guard<std::mutex> locker(callNetSysController_);
    return NetsysController::GetInstance().SetFirewallCurrentUserId(userId);
}
} // namespace NetManagerStandard
} // namespace OHOS
