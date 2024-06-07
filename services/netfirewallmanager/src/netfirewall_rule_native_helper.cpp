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
std::shared_ptr<NetFirewallRuleNativeHelper> NetFirewallRuleNativeHelper::instance_ = nullptr;

std::shared_ptr<NetFirewallRuleNativeHelper> NetFirewallRuleNativeHelper::GetInstance()
{
    static std::mutex instanceMutex;
    std::lock_guard<std::mutex> guard(instanceMutex);
    if (instance_ == nullptr) {
        instance_.reset(new NetFirewallRuleNativeHelper());
    }
    return instance_;
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
 * Add firewall rules to bpf maps
 *
 * @param ruleList list of NetFirewallIpRule
 * @param isFinish transmit finish or not
 * @return 0 if success or -1 if an error occurred
 */
int32_t NetFirewallRuleNativeHelper::AddFirewallIpRules(const std::vector<sptr<NetFirewallIpRule>> &ruleList,
    bool isFinish)
{
    std::lock_guard<std::mutex> locker(callNetSysController_);
    return NetsysController::GetInstance().AddFirewallIpRules(ruleList, isFinish);
}

/**
 * Update firewall rules to bpf maps
 *
 * @param rule list of NetFirewallIpRule
 * @return 0 if success or -1 if an error occurred
 */
int32_t NetFirewallRuleNativeHelper::UpdateFirewallIpRule(const sptr<NetFirewallIpRule> &rule)
{
    std::lock_guard<std::mutex> locker(callNetSysController_);
    return NetsysController::GetInstance().UpdateFirewallIpRule(rule);
}

/**
 * Add firewall domain rules
 *
 * @param ruleList list of NetFirewallDomainRule
 * @param isFinish transmit finish or not
 * @return 0 if success or -1 if an error occurred
 */
int32_t NetFirewallRuleNativeHelper::AddFirewallDomainRules(const std::vector<sptr<NetFirewallDomainRule>> &ruleList,
    bool isFinish)
{
    std::lock_guard<std::mutex> locker(callNetSysController_);
    return NetsysController::GetInstance().AddFirewallDomainRules(ruleList, isFinish);
}

/**
 * Update firewall domain rules
 *
 * @param rule list of NetFirewallDomainRule
 * @return 0 if success or -1 if an error occurred
 */
int32_t NetFirewallRuleNativeHelper::UpdateFirewallDomainRules(const std::vector<sptr<NetFirewallDomainRule>> &ruleList)
{
    std::lock_guard<std::mutex> locker(callNetSysController_);
    return NetsysController::GetInstance().UpdateFirewallDomainRules(ruleList);
}

/**
 * Delete firewall rules
 *
 * @param ruleIds list of NetFirewallRule ids
 * @return 0 if success or -1 if an error occurred
 */
int32_t NetFirewallRuleNativeHelper::DeleteFirewallRules(NetFirewallRuleType type, const std::vector<int32_t> &ruleIds)
{
    std::lock_guard<std::mutex> locker(callNetSysController_);
    return NetsysController::GetInstance().DeleteFirewallRules(type, ruleIds);
}

/**
 * Set firewall rules to bpf maps
 *
 * @param ruleList list of NetFirewallIpRule
 * @return 0 if success or -1 if an error occurred
 */
int32_t NetFirewallRuleNativeHelper::SetFirewallIpRules(const std::vector<sptr<NetFirewallIpRule>> &ruleList)
{
    std::lock_guard<std::mutex> locker(callNetSysController_);
    return NetsysController::GetInstance().SetFirewallIpRules(ruleList);
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
 * Set the Firewall DNS rules
 *
 * @param ruleList firewall rules
 * @return 0 if success or-1 if an error occurred
 */
int32_t NetFirewallRuleNativeHelper::SetFirewallDnsRules(const std::vector<sptr<NetFirewallDnsRule>> &ruleList)
{
    std::lock_guard<std::mutex> locker(callNetSysController_);
    return NetsysController::GetInstance().SetFirewallDnsRules(ruleList);
}

/**
 * Set the Firewall domain rules
 *
 * @param  ruleList firewall rules
 * @return 0 if success or-1 if an error occurred
 */
int32_t NetFirewallRuleNativeHelper::SetFirewallDomainRules(const std::vector<sptr<NetFirewallDomainRule>> &ruleList)
{
    std::lock_guard<std::mutex> locker(callNetSysController_);
    return NetsysController::GetInstance().SetFirewallDomainRules(ruleList);
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
