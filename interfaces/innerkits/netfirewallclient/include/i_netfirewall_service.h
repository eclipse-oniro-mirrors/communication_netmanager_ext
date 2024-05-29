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

#ifndef I_NET_FIREWALL_H
#define I_NET_FIREWALL_H

#include "iremote_broker.h"
#include "netfirewall_common.h"

namespace OHOS {
namespace NetManagerStandard {
class INetFirewallService : public IRemoteBroker {
public:
    virtual int32_t SetNetFirewallPolicy(const int32_t userId, const sptr<NetFirewallPolicy> &status) = 0;

    virtual int32_t GetNetFirewallPolicy(const int32_t userId, sptr<NetFirewallPolicy> &status) = 0;

    virtual int32_t AddNetFirewallRule(const sptr<NetFirewallRule> &rule, int32_t &result) = 0;

    virtual int32_t UpdateNetFirewallRule(const sptr<NetFirewallRule> &rule) = 0;

    virtual int32_t DeleteNetFirewallRule(const int32_t userId, const int32_t ruleId) = 0;

    virtual int32_t GetNetFirewallRules(const int32_t userId, const sptr<RequestParam> &requestParam,
        sptr<FirewallRulePage> &info) = 0;

    virtual int32_t GetNetFirewallRule(const int32_t userId, const int32_t ruleId, sptr<NetFirewallRule> &rule) = 0;

    virtual int32_t GetInterceptRecords(const int32_t userId, const sptr<RequestParam> &requestParam,
        sptr<InterceptRecordPage> &info) = 0;

    enum {
        SET_NET_FIREWALL_STATUS,
        GET_NET_FIREWALL_STATUS,
        ADD_NET_FIREWALL_RULE,
        UPDATE_NET_FIREWALL_RULE,
        DELETE_NET_FIREWALL_RULE,
        GET_ALL_NET_FIREWALL_RULES,
        GET_NET_FIREWALL_RULE,
        GET_ALL_INTERCEPT_RECORDS
    };
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.NetManagerStandard.INetFirewallService");
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif /* I_NET_FIREWALL_H */
