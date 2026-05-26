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

#ifndef NETTRAFFICFILTER_REDIRECT_MANAGER_H
#define NETTRAFFICFILTER_REDIRECT_MANAGER_H

#include <algorithm>
#include <atomic>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include "netfirewall_common.h"
#include "nettrafficfilter_redirector_context.h"
#include "application_state_observer_stub.h"
#include "app_mgr_client.h"
#include "net_port_states_info.h"

namespace OHOS {
namespace NetManagerStandard {

class NetTrafficFilterRedirectManager {
public:
    static NetTrafficFilterRedirectManager& GetInstance();
    ~NetTrafficFilterRedirectManager();

    int32_t CreateRedirector(const std::string& bundleName, uint32_t groupId, uint32_t priority,
                             const NetTrafficFilterConfig* config, std::string& redirectorId);
    int32_t DestroyRedirector(const std::string& redirectorId);
    int32_t AddRedirectRule(const std::string& redirectorId,
                            const TrafficFilterRedirectRule* rule);
    int32_t ClearRedirectRule(const std::string& redirectorId);

    int32_t DestroyRedirectorsByBundleName(const std::string& bundleName);

    int32_t PauseAllRedirectors();
    int32_t ResumeAllRedirectors();
    int32_t PauseRedirectorsByBundleName(const std::string& bundleName);
    int32_t ResumeRedirectorsByBundleName(const std::string& bundleName);

    int32_t GlobalEnableTrafficFilter();
    int32_t GlobalDisableTrafficFilter();
    int32_t GetTrafficFilterGlobalStatus(bool& isEnabled);
    int32_t QueryProcess(const std::string& srcIp, uint16_t srcPort,
        const std::string& dstIp, uint16_t dstPort, uint8_t protocol, uint32_t& uid, uint32_t& pid);

private:
    NetTrafficFilterRedirectManager();
    NetTrafficFilterRedirectManager(const NetTrafficFilterRedirectManager&) = delete;
    NetTrafficFilterRedirectManager& operator=(const NetTrafficFilterRedirectManager&) = delete;

    class TrafficFilterHapObserver : public AppExecFwk::ApplicationStateObserverStub {
    public:
        explicit TrafficFilterHapObserver(NetTrafficFilterRedirectManager& manager,
                                          const std::string& bundleName,
                                          int32_t uid)
            : redirectManager_(manager), bundleName_(bundleName), uid_(uid) {}
        virtual ~TrafficFilterHapObserver() = default;
        void OnProcessDied(const AppExecFwk::ProcessData& processData) override;
        void OnProcessStateChanged(const AppExecFwk::ProcessData& processData) override {}
        void OnProcessCreated(const AppExecFwk::ProcessData& processData) override {}
        void OnExtensionStateChanged(const AppExecFwk::AbilityStateData& abilityStateData) override {}
    private:
        NetTrafficFilterRedirectManager& redirectManager_;
        std::string bundleName_;
        int32_t uid_;
    };

    void HandleTrafficFilterObserverRegistration(const std::string& bundleName, int32_t uid, int32_t pid);
    void UnregisterTrafficFilterObserver(int32_t uid, const sptr<TrafficFilterHapObserver>& observer);
    int32_t CleanupRedirectorsByUid(int32_t uid, int32_t pid);
    int32_t CleanupRedirectorsByBundleName(const std::string& bundleName, int32_t uid, int32_t pid);
    int32_t ExecuteIptablesCommand(const std::string& command, TrafficFilterIPFamily family);
    std::string GenerateRedirectorId();
    static bool ValidateRedirectRuleFields(const TrafficFilterRedirectRule& rule);
    static bool ValidateCreateRedirectorParams(const std::string& bundleName, uint32_t groupId, uint32_t priority,
                                               const NetTrafficFilterConfig* config);
    static bool ValidateIPFamilyConsistency(const TrafficFilterIPMatch& srcIp, const TrafficFilterIPMatch& dstIp);
    static TrafficFilterIPFamily DetermineRuleFamily(const TrafficFilterRedirectRule& rule);
    static TrafficFilterIPFamily GetIPFamilyFromMatch(const TrafficFilterIPMatch& ipMatch);
    static bool ValidateIPMatch(const TrafficFilterIPMatch& ipMatch);
    static bool ValidatePortMatch(const TrafficFilterPortMatch& portMatch);
    static bool ValidateInterfaceMatch(const TrafficFilterInterfaceMatch& ifMatch);
    static bool ValidateUidMatch(const TrafficFilterRedirectRule& rule);
    static bool ValidateProxyFamilyConsistency(const TrafficFilterRedirectRule& rule);
    static bool ValidateRuleForAdd(const TrafficFilterRedirectRule& rule);
    static bool ValidateCidrIPMatch(const TrafficFilterIPMatch& ipMatch);
    static bool ValidateRangeIPMatch(const TrafficFilterIPMatch& ipMatch);
    static bool ValidateMultiIPMatch(const TrafficFilterIPMatch& ipMatch);
    bool IsRedirectorExists(const std::string& bundleName, uint32_t groupId);
    int32_t CleanupRedirectorIptablesResources(const std::string& chainName,
        const std::set<TrafficFilterHookPoint>& usedHookPoints);
    void RemoveRedirectorFromDataStructures(const std::string& redirectorId, const std::string& bundleName);
    void RebuildGlobalJumpRulesAfterDestroy(const std::set<TrafficFilterHookPoint>& usedHookPoints);
    int32_t RollbackRedirectorRules(const std::shared_ptr<NetTrafficFilterRedirectorContext>& redirector,
        const std::string& chainName, const std::vector<TrafficFilterRedirectRule>& oldRules,
        const std::set<TrafficFilterHookPoint>& affectedHookPoints);

    void SortRedirectorList();
    int32_t UpdateGlobalJumpRules(TrafficFilterHookPoint hookPoint, TrafficFilterIPFamily family);
    std::vector<std::string> GetActiveRedirectorsForHookPoint(TrafficFilterHookPoint hookPoint,
        TrafficFilterIPFamily family) const;
    int32_t RemoveJumpRulesFromHookPoint(TrafficFilterHookPoint hookPoint, TrafficFilterIPFamily family);
    int32_t ApplyRulesToChain(const std::shared_ptr<NetTrafficFilterRedirectorContext>& redirector,
                              const std::string& chainName);
    int32_t AppendRedirectRulesToChain(const std::vector<TrafficFilterRedirectRule>& sortedRules,
        const std::string& chainName);
    int32_t ApplyGlobalJumpRules(TrafficFilterHookPoint hookPoint);
    int32_t ResumeRedirectorStateByBundleName(const std::string& bundleName);
    int32_t RebuildRedirectorRulesByBundleName(const std::string& bundleName);
    bool MatchTcpConnection(const TcpNetPortStatesInfo& tcpInfo,
        const std::string& srcIp, uint16_t srcPort, const std::string& dstIp, uint16_t dstPort);
    bool MatchUdpConnection(const UdpNetPortStatesInfo& udpInfo,
        const std::string& srcIp, uint16_t srcPort, const std::string& dstIp, uint16_t dstPort);

    std::unordered_map<std::string, std::shared_ptr<NetTrafficFilterRedirectorContext>> redirectors_;
    std::unordered_map<std::string, std::vector<std::string>> bundleNameToRedirectorsMap_;
    std::vector<std::string> redirectorIdList_;
    std::atomic<uint32_t> redirectorIdCounter_;
    mutable std::mutex mutex_;
    std::unordered_map<int32_t, sptr<TrafficFilterHapObserver>> uidToObserverMap_;
    mutable std::mutex observerMutex_;
    bool isGloballyEnabled_ = true;
};

} // namespace NetManagerStandard
} // namespace OHOS

#endif // NETTRAFFICFILTER_REDIRECT_MANAGER_H
