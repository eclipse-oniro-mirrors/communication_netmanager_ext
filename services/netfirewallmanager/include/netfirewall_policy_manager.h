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

#ifndef NET_FIREWALL_POLICY_MANAGER_H
#define NET_FIREWALL_POLICY_MANAGER_H

#include <string>
#include <shared_mutex>

#include "netfirewall_common.h"
#include "netfirewall_preference_helper.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const std::string FIREWALL_PREFERENCE_PATH = "/data/service/el1/public/netmanager/netfirewall_status_";
} // namespace

class NetFirewallPolicyManager {
public:
    static std::shared_ptr<NetFirewallPolicyManager> GetInstance();
    NetFirewallPolicyManager();
    ~NetFirewallPolicyManager();

    /**
     * Set current forground user Id
     *
     * @param userId User id
     */
    void SetCurrentUserId(int32_t userId);

    /**
     * Turn on or off the firewall
     *
     * @param userId User id
     * @param policy The firewall policy to be set
     * @return Returns 0 success. Otherwise fail
     */
    int32_t SetNetFirewallPolicy(const int32_t userId, const sptr<NetFirewallPolicy> &policy);

    /**
     * Query firewall policy
     *
     * @param userId User id
     * @param policy Return to firewall policy
     * @return Returns 0 success. Otherwise fail
     */
    int32_t GetNetFirewallPolicy(const int32_t userId, sptr<NetFirewallPolicy> &policy);

    /**
     * Query current user firewall policy
     *
     * @param userId User id
     * @param policy Return to firewall policy
     * @return Returns 0 success. Otherwise fail
     */
    int32_t GetCurrentNetFirewallPolicy(sptr<NetFirewallPolicy> &policy);

    /**
     * Get user firewall open policy
     *
     * @param userId User id
     * @return Returns true is open, Otherwise close
     */
    bool IsNetFirewallOpen(const int32_t userId);

    /**
     * Get current user firewall open policy
     *
     * @param userId User id
     * @return Returns true is open, Otherwise close
     */
    bool IsCurrentFirewallOpen();

    /**
     * Clear user firewall policy
     *
     * @param userId Input User id
     * @return Returns true is open, Otherwise close
     */
    int32_t ClearFirewallPolicy(const int32_t userId);

    /**
     * Clear current user firewall policy
     *
     * @return Returns true is open, Otherwise close
     */
    int32_t ClearCurrentFirewallPolicy();

    /**
     * Get firewall policy inAction
     *
     * @return Returns FirewallRuleAction
     */
    FirewallRuleAction GetFirewallPolicyInAction();

    /**
     * Get firewall policy inAction
     *
     * @return Returns FirewallRuleAction
     */
    FirewallRuleAction GetFirewallPolicyOutAction();

    /**
     * Is firewall status change
     *
     * @param policy input policy status
     * @return Returns true is change, Otherwise not change
     */
    bool IsFirewallStatusChange(const sptr<NetFirewallPolicy> &policy);

    /**
     * Is firewall default action change
     *
     * @param policy input policy status
     * @return Returns true is change, Otherwise not change
     */
    bool IsFirewallActionChange(const sptr<NetFirewallPolicy> &policy);

    /**
     * Get firewall policy inAction
     *
     * @param policy input to firewall policy
     */
    void SetCurrentUserFirewallPolicy(const sptr<NetFirewallPolicy> &policy);

private:
    void RebuildFirewallPolicyCache(const int32_t userId);
    void EnsureCurrentFirewallPolicyCached();
    void LoadPolicyFormPreference(const int32_t userId, sptr<NetFirewallPolicy> &policy);

private:
    std::shared_mutex setPolicyMutex_;
    std::shared_ptr<NetFirewallPreferenceHelper> preferencesHelper_ = nullptr;
    // Cache the current state
    std::atomic<int32_t> currentUserId_ = 0;
    sptr<NetFirewallPolicy> currentFirewallPolicy_ = nullptr;
    static std::shared_ptr<NetFirewallPolicyManager> instance_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif /* NET_FIREWALL_POLICY_MANAGER_H */
