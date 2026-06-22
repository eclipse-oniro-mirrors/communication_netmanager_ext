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

#ifndef NETFIREWALL_CLIENT_H
#define NETFIREWALL_CLIENT_H

#include <cstdint>

#include "i_netfirewall_service.h"
#include "netfirewall_common.h"
#include "system_ability_load_callback_stub.h"

namespace OHOS {
namespace NetManagerStandard {

class NetFirewallClient {
public:
    NetFirewallClient() = default;
    ~NetFirewallClient() = default;
    NetFirewallClient(const NetFirewallClient &) = delete;
    NetFirewallClient &operator = (const NetFirewallClient &) = delete;

public:
    static NetFirewallClient &GetInstance();

public:
    int32_t SetNetFirewallPolicy(const int32_t userId, const sptr<NetFirewallPolicy> &status);

    int32_t GetNetFirewallPolicy(const int32_t userId, sptr<NetFirewallPolicy> &status);

    int32_t AddNetFirewallRule(const sptr<NetFirewallRule> &rule, int32_t &result);

    int32_t UpdateNetFirewallRule(const sptr<NetFirewallRule> &rule);

    int32_t DeleteNetFirewallRule(const int32_t userId, const int32_t ruleId);

    int32_t GetNetFirewallRules(const int32_t userId, const sptr<RequestParam> &requestParam,
        sptr<FirewallRulePage> &info);

    int32_t GetNetFirewallRule(const int32_t userId, const int32_t ruleId, sptr<NetFirewallRule> &rule);

    int32_t GetInterceptRecords(const int32_t userId, const sptr<RequestParam> &requestParam,
        sptr<InterceptRecordPage> &info);
    int32_t RegisterInterceptRecordsCallback(const sptr<INetInterceptRecordCallback> &callback);

    int32_t UnregisterInterceptRecordsCallback(const sptr<INetInterceptRecordCallback> &callback);

    int32_t CreateRedirector(uint32_t groupId, uint32_t priority, std::string& redirectorId);

    int32_t DestroyRedirector(const std::string& redirectorId);

    int32_t AddRedirectRule(const std::string& redirectorId,
        const sptr<TrafficFilterRedirectRule> &rule);

    int32_t ClearRedirectRule(const std::string& redirectorId);
    int32_t GlobalEnableTrafficFilter();
    int32_t GlobalDisableTrafficFilter();
    int32_t GetTrafficFilterGlobalStatus(bool& isEnabled);
    int32_t QueryProcess(const std::string& srcIp, uint16_t srcPort,
        const std::string& dstIp, uint16_t dstPort, uint8_t protocol, uint32_t& uid, uint32_t& pid);

private:
    class MonitorPcfirewallServiceDead : public IRemoteObject::DeathRecipient {
    public:
        explicit MonitorPcfirewallServiceDead(NetFirewallClient &client) : client_(client) {}
        ~MonitorPcfirewallServiceDead() override = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override
        {
            client_.OnRemoteDied(remote);
        }

    private:
        NetFirewallClient &client_;
    };

    sptr<INetFirewallService> GetProxy();

    sptr<IRemoteObject> LoadSaOnDemand();

    bool RestartNetFirewallManagerSysAbility();

    void OnRemoteDied(const wptr<IRemoteObject> &remote);

private:
    std::mutex mutex_;
    sptr<INetFirewallService> netfirewallService_ = nullptr;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ = nullptr;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETFIREWALL_CLIENT_H
