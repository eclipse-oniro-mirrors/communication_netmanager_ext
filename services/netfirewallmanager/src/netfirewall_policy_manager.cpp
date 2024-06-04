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
#include "netmanager_base_common_utils.h"
#include "netfirewall_policy_manager.h"

namespace OHOS {
namespace NetManagerStandard {
std::shared_ptr<NetFirewallPolicyManager> NetFirewallPolicyManager::instance_ = nullptr;

std::shared_ptr<NetFirewallPolicyManager> NetFirewallPolicyManager::GetInstance()
{
    static std::mutex instanceMutex;
    std::lock_guard<std::mutex> guard(instanceMutex);
    if (instance_ == nullptr) {
        instance_.reset(new NetFirewallPolicyManager());
    }
    return instance_;
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
    std::lock_guard<std::shared_mutex> locker(setPolicyMutex_);
    if (currentFirewallPolicy_) {
        delete currentFirewallPolicy_;
        currentFirewallPolicy_ = nullptr;
    }
    currentUserId_ = userId;
    RebuildFirewallPolicyCache(userId);
}

int32_t NetFirewallPolicyManager::SetNetFirewallPolicy(const int32_t userId, const sptr<NetFirewallPolicy> &policy)
{
    std::lock_guard<std::shared_mutex> locker(setPolicyMutex_);
    if (preferencesHelper_ == nullptr) {
        NETMGR_EXT_LOG_E("SetNetFirewallPolicy failed, reference is nullptr.");
        return FIREWALL_ERR_INTERNAL;
    }

    preferencesHelper_->GetPreference(FIREWALL_PREFERENCE_PATH + std::to_string(userId) + ".xml");
    preferencesHelper_->SaveBool("isOpen", policy->isOpen);
    preferencesHelper_->SaveInt("inAction", static_cast<int>(policy->inAction));
    preferencesHelper_->SaveInt("outAction", static_cast<int>(policy->outAction));

    return FIREWALL_SUCCESS;
}

int32_t NetFirewallPolicyManager::GetNetFirewallPolicy(const int32_t userId, sptr<NetFirewallPolicy> &policy)
{
    std::shared_lock<std::shared_mutex> locker(setPolicyMutex_);
    if (currentUserId_ == userId) {
        EnsureCurrentFirewallPolicyCached();
        policy->isOpen = currentFirewallPolicy_->isOpen;
        policy->inAction = currentFirewallPolicy_->inAction;
        policy->outAction = currentFirewallPolicy_->outAction;
    } else {
        LoadPolicyFormPreference(userId, policy);
    }

    return FIREWALL_SUCCESS;
}

bool NetFirewallPolicyManager::IsOpenOrCloseNativeFirewall(const sptr<NetFirewallPolicy> &policy)
{
    std::shared_lock<std::shared_mutex> locker(setPolicyMutex_);
    NETMGR_EXT_LOG_D("IsNetFirewallPolicyChanage");

    EnsureCurrentFirewallPolicyCached();
    return (policy->isOpen != currentFirewallPolicy_->isOpen) && currentFirewallPolicy_->isOpen;
}

bool NetFirewallPolicyManager::IsSetFirewallDefaultAction(const sptr<NetFirewallPolicy> &policy)
{
    std::shared_lock<std::shared_mutex> locker(setPolicyMutex_);
    EnsureCurrentFirewallPolicyCached();
    return currentFirewallPolicy_->isOpen && (policy->inAction != currentFirewallPolicy_->inAction ||
        policy->outAction != currentFirewallPolicy_->outAction);
}

void NetFirewallPolicyManager::SetCurrentUserFirewallPolicy(const sptr<NetFirewallPolicy> &policy)
{
    std::lock_guard<std::shared_mutex> locker(setPolicyMutex_);
    if (currentFirewallPolicy_ == nullptr) {
        currentFirewallPolicy_ = new (std::nothrow) NetFirewallPolicy();
    }

    currentFirewallPolicy_->isOpen = policy->isOpen;
    currentFirewallPolicy_->inAction = policy->inAction;
    currentFirewallPolicy_->outAction = policy->outAction;
}

int32_t NetFirewallPolicyManager::GetCurrentNetFirewallPolicy(sptr<NetFirewallPolicy> &policy)
{
    return GetNetFirewallPolicy(currentUserId_, policy);
}

bool NetFirewallPolicyManager::IsNetFirewallOpen(const int32_t userId)
{
    std::shared_lock<std::shared_mutex> locker(setPolicyMutex_);
    NETMGR_EXT_LOG_D("IsNetFirewallOpen");
    // Current user fetching cache
    if (userId == currentUserId_) {
        EnsureCurrentFirewallPolicyCached();
        return currentFirewallPolicy_->isOpen;
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
    std::lock_guard<std::shared_mutex> locker(setPolicyMutex_);
    if (preferencesHelper_ == nullptr) {
        NETMGR_EXT_LOG_E("ClearFirewallPolicy failed");
        return FIREWALL_ERR_INTERNAL;
    }
    preferencesHelper_->Clear(FIREWALL_PREFERENCE_PATH + std::to_string(userId) + ".xml");

    if (currentFirewallPolicy_) {
        delete currentFirewallPolicy_;
        currentFirewallPolicy_ = nullptr;
    }

    return FIREWALL_SUCCESS;
}

int32_t NetFirewallPolicyManager::ClearCurrentFirewallPolicy()
{
    return ClearFirewallPolicy(currentUserId_);
}

FirewallRuleAction NetFirewallPolicyManager::GetFirewallPolicyInAction()
{
    std::shared_lock<std::shared_mutex> locker(setPolicyMutex_);
    NETMGR_EXT_LOG_D("GetCurrentNetFirewallPolicyInAction");
    EnsureCurrentFirewallPolicyCached();
    return currentFirewallPolicy_->inAction;
}

FirewallRuleAction NetFirewallPolicyManager::GetFirewallPolicyOutAction()
{
    std::shared_lock<std::shared_mutex> locker(setPolicyMutex_);
    NETMGR_EXT_LOG_D("GetCurrentFirewallOutAction");
    EnsureCurrentFirewallPolicyCached();
    return currentFirewallPolicy_->outAction;
}

void NetFirewallPolicyManager::EnsureCurrentFirewallPolicyCached()
{
    if (currentFirewallPolicy_ == nullptr) {
        RebuildFirewallPolicyCache(currentUserId_);
    }
}

void NetFirewallPolicyManager::RebuildFirewallPolicyCache(const int32_t userId)
{
    // If the userId is not valid, return directly
    if (userId == 0) {
        return;
    }
    if (currentFirewallPolicy_) {
        delete currentFirewallPolicy_;
        currentFirewallPolicy_ = nullptr;
    }
    currentFirewallPolicy_ = new (std::nothrow) NetFirewallPolicy();
    LoadPolicyFormPreference(userId, currentFirewallPolicy_);
}

void NetFirewallPolicyManager::LoadPolicyFormPreference(const int32_t userId, sptr<NetFirewallPolicy> &policy)
{
    preferencesHelper_->GetPreference(FIREWALL_PREFERENCE_PATH + std::to_string(userId) + ".xml");
    policy->isOpen = preferencesHelper_->ObtainBool("isOpen", true);
    policy->inAction = static_cast<FirewallRuleAction>(
        preferencesHelper_->ObtainInt("inAction", static_cast<int>(FirewallRuleAction::RULE_DENY)));
    policy->outAction = static_cast<FirewallRuleAction>(
        preferencesHelper_->ObtainInt("outAction", static_cast<int>(FirewallRuleAction::RULE_ALLOW)));
}
} // namespace NetManagerStandard
} // namespace OHOS
