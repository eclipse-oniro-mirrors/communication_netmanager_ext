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
    static NetFirewallPolicyManager &GetInstance();
    NetFirewallPolicyManager();
    ~NetFirewallPolicyManager();

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
     * true or false firewall status
     *
     * @return Returns true is open, Otherwise close
     */
    bool IsFirewallOpen();

    /**
     * Get user firewall open policy
     *
     * @param userId User id
     * @return Returns true is open, Otherwise close
     */
    bool IsNetFirewallOpen(const int32_t userId);

    /**
     * Clear user firewall policy
     *
     * @param userId Input User id
     * @return Returns true is open, Otherwise close
     */
    int32_t ClearFirewallPolicy(const int32_t userId);

    /**
     * Init netfirewall policy
     */
    int32_t InitNetfirewallPolicy();

private:
    void LoadPolicyFormPreference(const int32_t userId, sptr<NetFirewallPolicy> &policy);
    void GetAllUserId(std::vector<int32_t> &accountIds);

private:
    std::shared_mutex setPolicyMutex_;
    std::shared_ptr<NetFirewallPreferenceHelper> preferencesHelper_ = nullptr;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif /* NET_FIREWALL_POLICY_MANAGER_H */
