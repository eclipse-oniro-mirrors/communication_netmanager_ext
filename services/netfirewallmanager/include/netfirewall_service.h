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

#include "event_handler.h"
#include "os_account_manager.h"
#include "netfirewall_common.h"
#include "netfirewall_stub.h"
#include "singleton.h"
#include "system_ability.h"

#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"
#include "netfirewall_callback_stub.h"

#include "netfirewall_preferences_util.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const std::string FIREWALL_PREFERENCE_PATH = "/data/service/el1/public/netmanager/netfirewall_status_";
} // namespace

using namespace OHOS::EventFwk;
class NetFirewallService : public SystemAbility,
    public NetFirewallStub,
    public std::enable_shared_from_this<NetFirewallService> {
    DECLARE_SINGLETON(NetFirewallService)
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
            NetFirewallService &netfirewallService)
            : EventFwk::CommonEventSubscriber(subscriberInfo), _netfirewallService(netfirewallService) {};

        virtual void OnReceiveEvent(const EventFwk::CommonEventData &eventData) override;

    private:
        NetFirewallService &_netfirewallService;
    };

    // Firewall interception log callback
    class FirewallCallback : public OHOS::NetsysNative::NetFirewallCallbackStub {
    public:
        FirewallCallback(NetFirewallService &netfirewallService) : netfirewallService_(netfirewallService) {};
        ~FirewallCallback() override = default;
        virtual int32_t OnIntercept(sptr<InterceptRecord> &record) override;

    private:
        NetFirewallService &netfirewallService_;
    };

    /*
     * Turn on or off the firewall
     *
     * @param userId User id
     * @param status The firewall status to be set
     * @return Error code
     */
    int32_t SetNetFirewallStatus(const int32_t userId, const sptr<NetFirewallStatus> &status) override;

    /*
     * Query firewall status
     *
     * @param userId User id
     * @param status Return to firewall status
     * @return Error code
     */
    int32_t GetNetFirewallStatus(const int32_t userId, sptr<NetFirewallStatus> &status) override;

    /*
     * Add firewall rules
     *
     * @param rule Firewall rules
     * @param result Rule ID
     * @return Error code
     */
    int32_t AddNetFirewallRule(const sptr<NetFirewallRule> &rule, int32_t &result) override;

    /*
     * Modify firewall rules
     *
     * @param rule Firewall rules
     * @return Error code
     */
    int32_t UpdateNetFirewallRule(const sptr<NetFirewallRule> &rule) override;

    /*
     * Delete firewall rules
     *
     * @param userId User ID
     * @param ruleId Rule ID
     * @return Error code
     */
    int32_t DeleteNetFirewallRule(const int32_t userId, const int32_t ruleId) override;

    /*
     * Get all firewall rules
     *
     * @param userId User ID
     * @param requestParam Paging in parameter information
     * @param info Paging data information
     * @return Error code
     */
    int32_t GetAllNetFirewallRules(const int32_t userId, const sptr<RequestParam> &requestParam,
        sptr<FirewallRulePage> &info) override;
    /*
     * Get information about the specified rule ID
     *
     * @param ruleId Rule ID
     * @param rule Return to firewall rules
     * @return Error code
     */
    int32_t GetNetFirewallRule(const int32_t userId, const int32_t ruleId, sptr<NetFirewallRule> &rule) override;

    /*
     * Get all interception records
     *
     * @param userId User ID
     * @param requestParam Paging in parameter information
     * @param info Paging data information
     * @return Error code
     */
    int32_t GetAllInterceptRecords(const int32_t userId, const sptr<RequestParam> &requestParam,
        sptr<InterceptRecordPage> &info) override;

    /*
     * dump function
     *
     * @param fd File handle
     * @param args Input data
     * @return Error code
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

    int32_t DeleteNetFirewallRuleByAppId(const int32_t appUid);

    int32_t DeleteNetFirewallRuleByUserId(const int32_t userId);

    int32_t AddDefaultNetFirewallRule(int32_t userId);

    int32_t GetCurrentAccountId();

    int32_t ClearCurrentNetFirewallPreferences(const int32_t userId);

    int32_t DistributeAllRules();

    int32_t DistributeDnsAndDomainRules();

    void InitQueryUserId(int32_t times);

    bool InitUsersOnBoot();

    void InitServiceHandler();

    bool InitQueryNetFirewallRules();

    int32_t ChekcUserExits(const int32_t userId);

    int32_t CheckRuleExits(const int32_t ruleId, NetFirewallRule &oldRule);

    int32_t GetAllRuleConstraint(const int32_t userId);

    int32_t CheckRuleConstraint(const sptr<NetFirewallRule> &rule);

    void ClearRecordCache(const int32_t userId);

    std::string GetServiceState();

    std::string GetLastRulePushTime();

    std::string GetLastRulePushResult();

    int32_t GetAllUserFirewallState(std::map<int32_t, bool> &firewallStateMap);

    void GetStatusFormPreference(const int32_t userId, sptr<NetFirewallStatus> &status);

    bool GetCurrentFirewallState();

    bool CheckAccountExits(int32_t userId);

    int32_t GetEnabledNetFirewallRules(const int32_t userId, std::vector<NetFirewallRule> &ruleList);

    int32_t GetEnabledDomainOrDnsRules(const int32_t userId, std::vector<NetFirewallRule> &ruleList);

    int32_t AddNetFirewallRule(const sptr<NetFirewallRule> &rule, bool isNotify, int32_t &result);

    void initFirewallStatusCache();

    void NetFirewallRule2IpRule(const NetFirewallRule &rule, NetFirewallIpRule &ip);

    void NetFirewallRule2DomainRule(const NetFirewallRule &rule, std::vector<NetFirewallDomainRule> &domainRules);

    void SplitFirewallRules(const std::vector<NetFirewallRule> firewallRules, std::vector<NetFirewallIpRule> &ipRules,
        std::vector<NetFirewallDnsRule> &dnsRules, std::vector<NetFirewallDomainRule> &domainRules);

    void SplitFirewallRules(const std::vector<NetFirewallRule> firewallRules, std::vector<NetFirewallDnsRule> &dnsRules,
        std::vector<NetFirewallDomainRule> &domainRules);

    bool IsNetFirewallOpen(const int32_t userId);

    int32_t SetFirewallIpRules2Bpf(const std::vector<NetFirewallIpRule> &ipRules);

private:
    static std::shared_ptr<AppExecFwk::EventHandler> serviceHandler_;
    int64_t allUserRule_ = 0;
    int64_t allUserDomain_ = 0;
    uint64_t currentSetRuleSecond_ = 0;
    int64_t lastRulePushResult_ = -1;
    uint64_t serviceSpendTime_ = 0;
    int64_t maxDefaultRuleSize_ = 0;
    uint64_t startTimeTest_ = 0;
    std::mutex netfirewallMutex_;
    std::shared_ptr<ReceiveMessage> subscriber_ = nullptr;
    std::shared_ptr<NetFirewallPreferencesUtil> referencesUtil_ = nullptr;
    sptr<OHOS::NetsysNative::INetFirewallCallback> callback_ = nullptr;
    ServiceRunningState state_;
    // Cache the current state
    sptr<NetFirewallStatus> firewallStatus_ = nullptr;
    std::vector<sptr<InterceptRecord>> recordCache_;
    int32_t currentUserId_ = 0;
    std::map<int32_t, int64_t> userRuleSize_;
    bool isServicePublished_ = false;
    bool hasSaRemoved_ = false;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif /* NET_FIREWALL_SERVICE_H */
