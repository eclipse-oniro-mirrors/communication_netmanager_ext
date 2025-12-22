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

#ifndef NET_FIREWALL_PROXY_H
#define NET_FIREWALL_PROXY_H

#include "i_netfirewall_service.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "refbase.h"

namespace OHOS {
namespace NetManagerStandard {
class NetFirewallProxy : public IRemoteProxy<INetFirewallService> {
public:
    int32_t SetNetFirewallPolicy(const int32_t userId, const sptr<NetFirewallPolicy> &status) override;

    int32_t GetNetFirewallPolicy(const int32_t userId, sptr<NetFirewallPolicy> &status) override;

    int32_t AddNetFirewallRule(const sptr<NetFirewallRule> &rule, int32_t &result) override;

    int32_t UpdateNetFirewallRule(const sptr<NetFirewallRule> &rule) override;

    int32_t DeleteNetFirewallRule(const int32_t userId, const int32_t ruleId) override;

    int32_t GetNetFirewallRules(const int32_t userId, const sptr<RequestParam> &requestParam,
        sptr<FirewallRulePage> &info) override;

    int32_t GetNetFirewallRule(const int32_t userId, const int32_t ruleId, sptr<NetFirewallRule> &rule) override;

    int32_t GetInterceptRecords(const int32_t userId, const sptr<RequestParam> &requestParam,
        sptr<InterceptRecordPage> &info) override;

    int32_t RegisterInterceptRecordsCallback(const sptr<INetInterceptRecordCallback> &callback) override;

    int32_t UnregisterInterceptRecordsCallback(const sptr<INetInterceptRecordCallback> &callback) override;

    explicit NetFirewallProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<INetFirewallService>(impl) {}
    ~NetFirewallProxy() = default;

private:
    static inline BrokerDelegator<NetFirewallProxy> delegator_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif /* NET_FIREWALL_PROXY_H */
