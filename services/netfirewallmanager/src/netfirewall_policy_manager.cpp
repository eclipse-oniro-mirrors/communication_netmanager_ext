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
#include "netfirewall_policy_manager.h"

#include "net_manager_constants.h"
#include "netfirewall_rule_manager.h"
#include "netfirewall_rule_native_helper.h"
#include "netmanager_base_common_utils.h"
#include "netsys_controller.h"
#include "os_account_manager.h"

namespace OHOS {
namespace NetManagerStandard {
NetFirewallPolicyManager &NetFirewallPolicyManager::GetInstance()
{
    static NetFirewallPolicyManager instance;
    return instance;
}

NetFirewallPolicyManager::NetFirewallPolicyManager()
{
    NETMGR_EXT_LOG_I("NetFirewallPolicyManager()");
}

NetFirewallPolicyManager::~NetFirewallPolicyManager()
{
    NETMGR_EXT_LOG_I("~NetFirewallPolicyManager()");
}

int32_t NetFirewallPolicyManager::SetNetFirewallPolicy(const int32_t userId, const sptr<NetFirewallPolicy> &policy)
{
    std::unique_lock<std::shared_mutex> locker(setPolicyMutex_);
    if (policy == nullptr) {
        NETMGR_EXT_LOG_E("SetNetFirewallPolicy failed, policy is nullptr.");
        return FIREWALL_ERR_PARAMETER_ERROR;
    }
    auto helper = NetFirewallPreferenceHelper::CreateInstance(FirewallPreferencePathOfUser(userId));
    if (helper == nullptr) {
        NETMGR_EXT_LOG_E("SetNetFirewallPolicy failed, reference is nullptr.");
        return FIREWALL_ERR_INTERNAL;
    }
    bool ret = true;
    ret &= helper->SaveBool(NET_FIREWALL_IS_OPEN, policy->isOpen);
    ret &= helper->SaveInt(NET_FIREWALL_IN_ACTION, static_cast<int>(policy->inAction));
    ret &= helper->SaveInt(NET_FIREWALL_OUT_ACTION, static_cast<int>(policy->outAction));
    if (!ret) {
        NETMGR_EXT_LOG_E("SetNetFirewallPolicy failed");
        return FIREWALL_ERR_INTERNAL;
    }

    return FIREWALL_SUCCESS;
}

bool NetFirewallPolicyManager::IsFirewallOpen()
{
    std::unique_lock<std::shared_mutex> locker(setPolicyMutex_);
    std::vector<int32_t> accountIds;
    GetAllUserId(accountIds);
    for (auto &accountId : accountIds) {
        if (IsNetFirewallOpen(accountId)) {
            return true;
        }
    }
    return false;
}

void NetFirewallPolicyManager::GetAllUserId(std::vector<int32_t> &accountIds)
{
    std::vector<AccountSA::OsAccountInfo> osAccountInfos;
    int32_t ret = AccountSA::OsAccountManager::QueryAllCreatedOsAccounts(osAccountInfos);
    if (ret != ERR_OK || osAccountInfos.empty()) {
        NETMGR_EXT_LOG_W("query user failed errCode=%{public}d", ret);
    }
    for (const auto &info : osAccountInfos) {
        accountIds.push_back(info.GetLocalId());
    }
}

std::string NetFirewallPolicyManager::FirewallPreferencePathOfUser(int32_t userId)
{
    return FIREWALL_PREFERENCE_PATH + std::to_string(userId) + ".xml";
}

int32_t NetFirewallPolicyManager::InitNetfirewallPolicy()
{
    std::unique_lock<std::shared_mutex> locker(setPolicyMutex_);
    std::vector<int32_t> accountIds;
    GetAllUserId(accountIds);
    NETMGR_EXT_LOG_I("InitNetfirewallPolicy accountIds size =%{public}zu", accountIds.size());
    sptr<NetFirewallPolicy> netfirewallPolicy = new (std::nothrow) NetFirewallPolicy();
    if (netfirewallPolicy == nullptr) {
        NETMGR_EXT_LOG_E("InitNetfirewallPolicy error");
        return FIREWALL_ERR_INTERNAL;
    }
    NetFirewallRuleNativeHelper::GetInstance().ClearFirewallRules(NetFirewallRuleType::RULE_DEFAULT_ACTION);
    for (auto &accountId : accountIds) {
        NETMGR_EXT_LOG_I("InitNetfirewallPolicy accountId=%{public}d", accountId);
        LoadPolicyFormPreference(accountId, netfirewallPolicy);
        // set policy is open to bpf map
        if (netfirewallPolicy->isOpen) {
            NetFirewallRuleNativeHelper::GetInstance().SetFirewallDefaultAction(accountId,
                netfirewallPolicy->inAction, netfirewallPolicy->outAction);
        }
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallPolicyManager::GetNetFirewallPolicy(const int32_t userId, sptr<NetFirewallPolicy> &policy)
{
    if (policy == nullptr) {
        NETMGR_EXT_LOG_E("GetNetFirewallPolicy failed, policy is nullptr.");
        return FIREWALL_ERR_INTERNAL;
    }
    LoadPolicyFormPreference(userId, policy);
    return FIREWALL_SUCCESS;
}

bool NetFirewallPolicyManager::GetNetFirewallStatus(const int32_t userId)
{
    std::unique_lock<std::shared_mutex> locker(setPolicyMutex_);
    return IsNetFirewallOpen(userId);
}

bool NetFirewallPolicyManager::IsNetFirewallOpen(const int32_t userId)
{
    NETMGR_EXT_LOG_D("IsNetFirewallOpen");
    bool defVal = false;
    auto helper = NetFirewallPreferenceHelper::CreateInstance(FirewallPreferencePathOfUser(userId));
    if (helper == nullptr) {
        NETMGR_EXT_LOG_E("IsNetFirewallOpen failed, reference is nullptr.");
        return defVal;
    }
    return helper->ObtainBool(NET_FIREWALL_IS_OPEN, defVal);
}

int32_t NetFirewallPolicyManager::ClearFirewallPolicy(const int32_t userId)
{
    NetFirewallPreferenceHelper::Clear(FirewallPreferencePathOfUser(userId));
    InitNetfirewallPolicy();
    return FIREWALL_SUCCESS;
}

void NetFirewallPolicyManager::LoadPolicyFormPreference(const int32_t userId, sptr<NetFirewallPolicy> &policy)
{
    auto helper = NetFirewallPreferenceHelper::CreateInstance(FirewallPreferencePathOfUser(userId));
    if (helper == nullptr) {
        NETMGR_EXT_LOG_E("LoadPolicyFormPreference failed, reference is nullptr.");
        return;
    }
    policy->isOpen = helper->ObtainBool(NET_FIREWALL_IS_OPEN, false);
    policy->inAction = static_cast<FirewallRuleAction>(
        helper->ObtainInt(NET_FIREWALL_IN_ACTION, static_cast<int>(FirewallRuleAction::RULE_ALLOW)));
    policy->outAction = static_cast<FirewallRuleAction>(
        helper->ObtainInt(NET_FIREWALL_OUT_ACTION, static_cast<int>(FirewallRuleAction::RULE_ALLOW)));
}
} // namespace NetManagerStandard
} // namespace OHOS
