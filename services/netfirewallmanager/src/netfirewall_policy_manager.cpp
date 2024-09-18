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
#include "netmanager_base_common_utils.h"

namespace OHOS {
namespace NetManagerStandard {
NetFirewallPolicyManager &NetFirewallPolicyManager::GetInstance()
{
    static NetFirewallPolicyManager instance;
    return instance;
}

NetFirewallPolicyManager::NetFirewallPolicyManager()
{
    preferencesHelper_ = NetFirewallPreferenceHelper::GetInstance();
    NETMGR_EXT_LOG_I("NetFirewallPolicyManager()");
}

NetFirewallPolicyManager::~NetFirewallPolicyManager()
{
    NETMGR_EXT_LOG_I("~NetFirewallPolicyManager()");
}

void NetFirewallPolicyManager::SetCurrentUserId(int32_t userId)
{
    currentUserId_ = userId;
    RebuildFirewallPolicyCache(userId);
}

int32_t NetFirewallPolicyManager::SetNetFirewallPolicy(const int32_t userId, const sptr<NetFirewallPolicy> &policy)
{
    if (policy == nullptr) {
        NETMGR_EXT_LOG_E("SetNetFirewallPolicy failed, policy is nullptr.");
        return FIREWALL_ERR_PARAMETER_ERROR;
    }
    if (preferencesHelper_ == nullptr) {
        NETMGR_EXT_LOG_E("SetNetFirewallPolicy failed, reference is nullptr.");
        return FIREWALL_ERR_INTERNAL;
    }

    preferencesHelper_->GetPreference(FIREWALL_PREFERENCE_PATH + std::to_string(userId) + ".xml");
    preferencesHelper_->SaveBool(NET_FIREWALL_IS_OPEN, policy->isOpen);
    preferencesHelper_->SaveInt(NET_FIREWALL_IN_ACTION, static_cast<int>(policy->inAction));
    preferencesHelper_->SaveInt(NET_FIREWALL_OUT_ACTION, static_cast<int>(policy->outAction));

    return FIREWALL_SUCCESS;
}

int32_t NetFirewallPolicyManager::GetNetFirewallPolicy(const int32_t userId, sptr<NetFirewallPolicy> &policy)
{
    if (policy == nullptr) {
        NETMGR_EXT_LOG_E("GetNetFirewallPolicy failed, policy is nullptr.");
        return FIREWALL_ERR_INTERNAL;
    }
    if (currentUserId_ == userId) {
        EnsureCurrentFirewallPolicyCached();
        policy->isOpen = IsPolicyCacheOpen();
        policy->inAction = GetPolicyCacheInInternal();
        policy->outAction = GetPolicyCacheOutInternal();
    } else {
        LoadPolicyFormPreference(userId, policy);
    }

    return FIREWALL_SUCCESS;
}

bool NetFirewallPolicyManager::IsFirewallStatusChange(const sptr<NetFirewallPolicy> &policy)
{
    NETMGR_EXT_LOG_D("IsFirewallStatusChange");
    EnsureCurrentFirewallPolicyCached();
    return (policy->isOpen != IsPolicyCacheOpen());
}

bool NetFirewallPolicyManager::IsFirewallActionChange(const sptr<NetFirewallPolicy> &policy)
{
    NETMGR_EXT_LOG_D("IsFirewallActionChange");
    EnsureCurrentFirewallPolicyCached();
    return policy->isOpen &&
        (policy->inAction != GetPolicyCacheInInternal() || policy->outAction != GetPolicyCacheOutInternal());
}

void NetFirewallPolicyManager::SetCurrentUserFirewallPolicy(const sptr<NetFirewallPolicy> &policy)
{
    std::unique_lock<std::shared_mutex> locker(setPolicyMutex_);
    if (currentFirewallPolicy_ == nullptr) {
        currentFirewallPolicy_ = new (std::nothrow) NetFirewallPolicy();
    }
    if (currentFirewallPolicy_ != nullptr) {
        currentFirewallPolicy_->isOpen = policy->isOpen;
        currentFirewallPolicy_->inAction = policy->inAction;
        currentFirewallPolicy_->outAction = policy->outAction;
    }
}

int32_t NetFirewallPolicyManager::GetCurrentNetFirewallPolicy(sptr<NetFirewallPolicy> &policy)
{
    return GetNetFirewallPolicy(currentUserId_, policy);
}

bool NetFirewallPolicyManager::IsNetFirewallOpen(const int32_t userId)
{
    NETMGR_EXT_LOG_D("IsNetFirewallOpen");
    // Current user fetching cache
    if (userId == currentUserId_) {
        EnsureCurrentFirewallPolicyCached();
        return IsPolicyCacheOpen();
    }
    preferencesHelper_->GetPreference(FIREWALL_PREFERENCE_PATH + std::to_string(userId) + ".xml");
    return preferencesHelper_->ObtainBool("isOpen", true);
}

bool NetFirewallPolicyManager::IsCurrentFirewallOpen()
{
    return IsNetFirewallOpen(currentUserId_);
}

int32_t NetFirewallPolicyManager::ClearFirewallPolicy(const int32_t userId)
{
    if (preferencesHelper_ == nullptr) {
        NETMGR_EXT_LOG_E("ClearFirewallPolicy failed");
        return FIREWALL_ERR_INTERNAL;
    }
    preferencesHelper_->Clear(FIREWALL_PREFERENCE_PATH + std::to_string(userId) + ".xml");
    return ClearCurrentFirewallPolicy();
}

int32_t NetFirewallPolicyManager::ClearCurrentFirewallPolicy()
{
    std::lock_guard<std::shared_mutex> locker(setPolicyMutex_);
    currentFirewallPolicy_ = nullptr;
    return FIREWALL_SUCCESS;
}

FirewallRuleAction NetFirewallPolicyManager::GetFirewallPolicyInAction()
{
    NETMGR_EXT_LOG_D("GetCurrentNetFirewallPolicyInAction");
    EnsureCurrentFirewallPolicyCached();
    return GetPolicyCacheInInternal();
}

FirewallRuleAction NetFirewallPolicyManager::GetFirewallPolicyOutAction()
{
    NETMGR_EXT_LOG_D("GetCurrentFirewallOutAction");
    EnsureCurrentFirewallPolicyCached();
    return GetPolicyCacheOutInternal();
}

void NetFirewallPolicyManager::EnsureCurrentFirewallPolicyCached()
{
    if (!IsPolicyCacheInvalid()) {
        RebuildFirewallPolicyCache(currentUserId_);
    }
}

bool NetFirewallPolicyManager::IsPolicyCacheInvalid()
{
    std::shared_lock<std::shared_mutex> locker(setPolicyMutex_);
    return currentFirewallPolicy_ == nullptr;
}

bool NetFirewallPolicyManager::IsPolicyCacheOpen()
{
    std::shared_lock<std::shared_mutex> locker(setPolicyMutex_);
    return currentFirewallPolicy_ != nullptr && currentFirewallPolicy_->isOpen;
}

FirewallRuleAction NetFirewallPolicyManager::GetPolicyCacheInInternal()
{
    std::shared_lock<std::shared_mutex> locker(setPolicyMutex_);
    if (currentFirewallPolicy_ == nullptr) {
        return FirewallRuleAction::RULE_ALLOW;
    }
    return currentFirewallPolicy_->inAction;
}

FirewallRuleAction NetFirewallPolicyManager::GetPolicyCacheOutInternal()
{
    std::shared_lock<std::shared_mutex> locker(setPolicyMutex_);
    if (currentFirewallPolicy_ == nullptr) {
        return FirewallRuleAction::RULE_ALLOW;
    }
    return currentFirewallPolicy_->outAction;
}

void NetFirewallPolicyManager::RebuildFirewallPolicyCache(const int32_t userId)
{
    std::unique_lock<std::shared_mutex> locker(setPolicyMutex_);
    currentFirewallPolicy_ = nullptr;
    currentFirewallPolicy_ = new (std::nothrow) NetFirewallPolicy();
    if (currentFirewallPolicy_ != nullptr) {
        LoadPolicyFormPreference(userId, currentFirewallPolicy_);
    }
}

void NetFirewallPolicyManager::LoadPolicyFormPreference(const int32_t userId, sptr<NetFirewallPolicy> &policy)
{
    preferencesHelper_->GetPreference(FIREWALL_PREFERENCE_PATH + std::to_string(userId) + ".xml");
    policy->isOpen = preferencesHelper_->ObtainBool(NET_FIREWALL_IS_OPEN, false);
    policy->inAction = static_cast<FirewallRuleAction>(
        preferencesHelper_->ObtainInt(NET_FIREWALL_IN_ACTION, static_cast<int>(FirewallRuleAction::RULE_ALLOW)));
    policy->outAction = static_cast<FirewallRuleAction>(
        preferencesHelper_->ObtainInt(NET_FIREWALL_OUT_ACTION, static_cast<int>(FirewallRuleAction::RULE_ALLOW)));
}
} // namespace NetManagerStandard
} // namespace OHOS
