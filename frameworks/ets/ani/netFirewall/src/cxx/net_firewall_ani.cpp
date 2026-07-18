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

#include "net_firewall_ani.h"

#include "cxx.h"
#include "net_firewall_param_check.h"
#include "net_firewall_rule_parse.h"
#include "netfirewall_client.h"
#include "net_manager_constants.h"
#include "singleton.h"
#include "wrapper.rs.h"
#include <dlfcn.h>

namespace OHOS {
namespace NetManagerAni {
using namespace NetManagerStandard;

int32_t SetNetFirewallPolicy(int32_t userId, const NetFirewallPolicyAni &policyAni)
{
    if (userId < 0) {
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    int32_t ret = NetFirewallParamCheck::CheckFirewallRulePolicy(policyAni);
    if (ret != FIREWALL_SUCCESS) {
        return ret;
    }
    sptr<NetFirewallPolicy> policy = new (std::nothrow) NetFirewallPolicy();
    if (policy == nullptr) {
        return FIREWALL_ERR_INTERNAL;
    }
    policy->isOpen = policyAni.is_open;
    policy->inAction = static_cast<FirewallRuleAction>(policyAni.in_action);
    policy->outAction = static_cast<FirewallRuleAction>(policyAni.out_action);
    ret = DelayedSingleton<NetFirewallClient>::GetInstance()->SetNetFirewallPolicy(userId, policy);
    return ret;
}

int32_t GetNetFirewallPolicy(int32_t userId, NetFirewallPolicyAni &policyAni)
{
    if (userId < 0) {
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    sptr<NetFirewallPolicy> policy = new (std::nothrow) NetFirewallPolicy();
    if (policy == nullptr) {
        return FIREWALL_ERR_INTERNAL;
    }
    int32_t ret = DelayedSingleton<NetFirewallClient>::GetInstance()->GetNetFirewallPolicy(userId, policy);
    if (ret == FIREWALL_SUCCESS) {
        policyAni.is_open = policy->isOpen;
        policyAni.in_action = static_cast<int32_t>(policy->inAction);
        policyAni.out_action = static_cast<int32_t>(policy->outAction);
    }
    return ret;
}

int32_t AddNetFirewallRule(const NetFirewallRuleAni &ruleAni, int32_t &ruleId)
{
    int32_t ret = NetFirewallParamCheck::CheckFirewallRule(ruleAni);
    if (ret != FIREWALL_SUCCESS) {
        return ret;
    }
    sptr<NetFirewallRule> rule = new (std::nothrow) NetFirewallRule();
    if (rule == nullptr) {
        return FIREWALL_ERR_INTERNAL;
    }
    NetFirewallRuleParse::ParseRuleParams(ruleAni, rule);
    ret = DelayedSingleton<NetFirewallClient>::GetInstance()->AddNetFirewallRule(rule, ruleId);
    return ret;
}

int32_t UpdateNetFirewallRule(const NetFirewallRuleAni &ruleAni)
{
    int32_t ret = NetFirewallParamCheck::CheckUpdateFirewallRule(ruleAni);
    if (ret != FIREWALL_SUCCESS) {
        return ret;
    }
    sptr<NetFirewallRule> rule = new (std::nothrow) NetFirewallRule();
    if (rule == nullptr) {
        return FIREWALL_ERR_INTERNAL;
    }
    NetFirewallRuleParse::ParseRuleParams(ruleAni, rule);
    ret = DelayedSingleton<NetFirewallClient>::GetInstance()->UpdateNetFirewallRule(rule);
    return ret;
}

int32_t RemoveNetFirewallRule(int32_t userId, int32_t ruleId)
{
    if (userId <= 0 || ruleId <= 0) {
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    int32_t ret = DelayedSingleton<NetFirewallClient>::GetInstance()->DeleteNetFirewallRule(userId, ruleId);
    return ret;
}

int32_t GetNetFirewallRules(int32_t userId, const RequestParamAni &paramAni, FirewallRulePageAni &pageAni)
{
    if (userId <= 0) {
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    if (paramAni.page < 1 || paramAni.page > FIREWALL_USER_MAX_RULE ||
        paramAni.page_size < 1 || paramAni.page_size > static_cast<int32_t>(MAX_PAGE_SIZE)) {
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    if (static_cast<NetFirewallOrderField>(paramAni.order_field) != NetFirewallOrderField::ORDER_BY_RULE_NAME) {
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    if (static_cast<NetFirewallOrderType>(paramAni.order_type) != NetFirewallOrderType::ORDER_ASC &&
        static_cast<NetFirewallOrderType>(paramAni.order_type) != NetFirewallOrderType::ORDER_DESC) {
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    sptr<RequestParam> param = new (std::nothrow) RequestParam();
    if (param == nullptr) {
        return FIREWALL_ERR_INTERNAL;
    }
    param->page = paramAni.page;
    param->pageSize = paramAni.page_size;
    param->orderField = static_cast<NetFirewallOrderField>(paramAni.order_field);
    param->orderType = static_cast<NetFirewallOrderType>(paramAni.order_type);
    sptr<FirewallRulePage> pageInfo = new (std::nothrow) FirewallRulePage();
    if (pageInfo == nullptr) {
        return FIREWALL_ERR_INTERNAL;
    }
    int32_t ret = DelayedSingleton<NetFirewallClient>::GetInstance()->GetNetFirewallRules(
        userId, param, pageInfo);
    if (ret == FIREWALL_SUCCESS && pageInfo != nullptr) {
        pageAni.page = pageInfo->page;
        pageAni.page_size = pageInfo->pageSize;
        pageAni.total_page = pageInfo->totalPage;
        pageAni.data.clear();
        for (const auto &rule : pageInfo->data) {
            NetFirewallRuleAni ffiRule;
            NetFirewallRuleParse::ConvertNetFirewallRule(rule, ffiRule);
            pageAni.data.push_back(std::move(ffiRule));
        }
    }
    return ret;
}

int32_t GetNetFirewallRule(int32_t userId, int32_t ruleId, NetFirewallRuleAni &ruleAni)
{
    if (userId <= 0 || ruleId <= 0) {
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    sptr<NetFirewallRule> rule = new (std::nothrow) NetFirewallRule();
    if (rule == nullptr) {
        return FIREWALL_ERR_INTERNAL;
    }

    int32_t ret = DelayedSingleton<NetFirewallClient>::GetInstance()->GetNetFirewallRule(userId, ruleId, rule);
    if (ret == FIREWALL_SUCCESS && rule != nullptr) {
        NetFirewallRuleParse::ConvertNetFirewallRule(*rule, ruleAni);
    }
    return ret;
}

int32_t GetInterceptedRecords(int32_t userId, const RequestParamAni &paramAni, InterceptedRecordPageAni &pageAni)
{
    if (userId <= 0) {
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    if (paramAni.page < 1 || paramAni.page > FIREWALL_USER_MAX_RULE ||
        paramAni.page_size < 1 || paramAni.page_size > static_cast<int32_t>(MAX_PAGE_SIZE)) {
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    if (static_cast<NetFirewallOrderField>(paramAni.order_field) != NetFirewallOrderField::ORDER_BY_RECORD_TIME) {
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    if (static_cast<NetFirewallOrderType>(paramAni.order_type) != NetFirewallOrderType::ORDER_ASC &&
        static_cast<NetFirewallOrderType>(paramAni.order_type) != NetFirewallOrderType::ORDER_DESC) {
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    sptr<RequestParam> param = new (std::nothrow) RequestParam();
    if (param == nullptr) {
        return FIREWALL_ERR_INTERNAL;
    }
    param->page = paramAni.page;
    param->pageSize = paramAni.page_size;
    param->orderField = static_cast<NetFirewallOrderField>(paramAni.order_field);
    param->orderType = static_cast<NetFirewallOrderType>(paramAni.order_type);

    sptr<InterceptRecordPage> pageInfo = new (std::nothrow) InterceptRecordPage();
    if (pageInfo == nullptr) {
        return FIREWALL_ERR_INTERNAL;
    }
    int32_t ret = DelayedSingleton<NetFirewallClient>::GetInstance()->GetInterceptRecords(
        userId, param, pageInfo);
    if (ret == FIREWALL_SUCCESS && pageInfo != nullptr) {
        NetFirewallRuleParse::ConvertInterceptRecordPage(pageInfo, pageAni);
    }
    return ret;
}
} // namespace NetManagerAni
} // namespace OHOS
