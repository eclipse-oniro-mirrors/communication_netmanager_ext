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

#ifndef NET_FIREWALL_SERVICE_H
#define NET_FIREWALL_SERVICE_H

#include <string>

#include "os_account_manager.h"
#include "netfirewall_common.h"
#include "netfirewall_stub.h"
#include "singleton.h"
#include "system_ability.h"
#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"

#include "netfirewall_policy_manager.h"
#include "netfirewall_rule_manager.h"
#include "netfirewall_rule_native_helper.h"
#include "ffrt.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace OHOS::EventFwk;
class NetFirewallService : public SystemAbility,
    public NetFirewallStub,
    public std::enable_shared_from_this<NetFirewallService> {
    DECLARE_DELAYED_SINGLETON(NetFirewallService);
    DECLARE_SYSTEM_ABILITY(NetFirewallService)
    enum class ServiceRunningState {
        STATE_NOT_START,
        STATE_RUNNING
    };

public:
    void SubscribeCommonEvent();

    // Broadcast Listener
    class ReceiveMessage : public OHOS::EventFwk::CommonEventSubscriber {
    public:
        explicit ReceiveMessage(const EventFwk::CommonEventSubscribeInfo &subscriberInfo,
            std::shared_ptr<NetFirewallService> netfirewallService)
            : EventFwk::CommonEventSubscriber(subscriberInfo), netfirewallService_(netfirewallService) {};

        virtual void OnReceiveEvent(const EventFwk::CommonEventData &eventData) override;

    private:
        std::shared_ptr<NetFirewallService> netfirewallService_ = nullptr;
    };

    /* *
     * Turn on or off the firewall
     *
     * @param userId User id
     * @param status The firewall status to be set
     * @return Returns 0 success. Otherwise fail
     */
    int32_t SetNetFirewallPolicy(const int32_t userId, const sptr<NetFirewallPolicy> &status) override;

    /**
     * Query firewall status
     *
     * @param userId User id
     * @param status Return to firewall status
     * @return Returns 0 success. Otherwise fail
     */
    int32_t GetNetFirewallPolicy(const int32_t userId, sptr<NetFirewallPolicy> &status) override;

    /**
     * Add firewall rules
     *
     * @param rule Firewall rules
     * @param ruleId Rule id genarated by database
     * @return Returns 0 success. Otherwise fail
     */
    int32_t AddNetFirewallRule(const sptr<NetFirewallRule> &rule, int32_t &ruleId) override;

    /**
     * Modify firewall rules
     *
     * @param rule Firewall rules
     * @return Returns 0 success. Otherwise fail
     */
    int32_t UpdateNetFirewallRule(const sptr<NetFirewallRule> &rule) override;

    /**
     * Delete firewall rules
     *
     * @param userId User ID
     * @param ruleId Rule ID
     * @return Returns 0 success. Otherwise fail
     */
    int32_t DeleteNetFirewallRule(const int32_t userId, const int32_t ruleId) override;

    /**
     * Get all firewall rules
     *
     * @param userId User ID
     * @param requestParam Paging in parameter information
     * @param info Paging data information
     * @return Returns 0 success. Otherwise fail
     */
    int32_t GetNetFirewallRules(const int32_t userId, const sptr<RequestParam> &requestParam,
        sptr<FirewallRulePage> &info) override;
    /**
     * Get information about the specified rule ID
     *
     * @param ruleId Rule ID
     * @param rule Return to firewall rules
     * @return Returns 0 success. Otherwise fail
     */
    int32_t GetNetFirewallRule(const int32_t userId, const int32_t ruleId, sptr<NetFirewallRule> &rule) override;

    /**
     * Get all interception records
     *
     * @param userId User ID
     * @param requestParam Paging in parameter information
     * @param info Paging data information
     * @return Returns 0 success. Otherwise fail
     */
    int32_t GetInterceptRecords(const int32_t userId, const sptr<RequestParam> &requestParam,
        sptr<InterceptRecordPage> &info) override;

    /**
     * dump function
     *
     * @param fd File handle
     * @param args Input data
     * @return Returns 0 success. Otherwise fail
     */
    int32_t Dump(int32_t fd, const std::vector<std::u16string> &args) override;

protected:
    void OnStart() override;

    void OnStop() override;

    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;

    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;

private:
    void GetDumpMessage(std::string &message);

    int32_t OnInit();

    int32_t GetCurrentAccountId();

    void SetCurrentUserId(int32_t userId);

    void InitQueryUserId(int32_t times);

    bool InitUsersOnBoot();

    void InitServiceHandler();

    void InitQueryNetFirewallRules();

    std::string GetServiceState();

    std::string GetLastRulePushTime();

    std::string GetLastRulePushResult();

    int32_t GetAllUserFirewallState(std::map<int32_t, bool> &firewallStateMap);

    int32_t AddDefaultNetFirewallRule(int32_t userId);

    int32_t CheckUserExist(const int32_t userId);

    void RegisterSubscribeCommonEvent();

private:
    static std::shared_ptr<ffrt::queue> ffrtServiceHandler_;
    std::atomic<uint64_t> currentSetRuleSecond_ = 0;
    std::atomic<int64_t> lastRulePushResult_ = -1;
    std::atomic<uint64_t> serviceSpendTime_ = 0;
    std::atomic<int32_t> currentUserId_ = 0;
    std::atomic<ServiceRunningState> state_;
    bool isServicePublished_ = false;
    bool hasSaRemoved_ = false;
    std::shared_ptr<ReceiveMessage> subscriber_ = nullptr;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif /* NET_FIREWALL_SERVICE_H */
