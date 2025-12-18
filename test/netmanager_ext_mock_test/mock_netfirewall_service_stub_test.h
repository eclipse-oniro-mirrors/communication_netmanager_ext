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

#ifndef MOCK_NETWORKVPN_SERVICE_STUB_TEST_H
#define MOCK_NETWORKVPN_SERVICE_STUB_TEST_H

#include "i_netfirewall_service.h"
#include "netfirewall_stub.h"

namespace OHOS {
namespace NetManagerStandard {
class MockNetFirewallServiceStub : public NetFirewallStub {
public:
    int32_t SetNetFirewallPolicy(const int32_t userId, const sptr<NetFirewallPolicy> &status) override
    {
        return 0;
    }

    int32_t GetNetFirewallPolicy(const int32_t userId, sptr<NetFirewallPolicy> &status) override
    {
        return 0;
    }

    int32_t AddNetFirewallRule(const sptr<NetFirewallRule> &rule, int32_t &result) override
    {
        return 0;
    }

    int32_t UpdateNetFirewallRule(const sptr<NetFirewallRule> &rule) override
    {
        return 0;
    }

    int32_t DeleteNetFirewallRule(const int32_t userId, const int32_t ruleId) override
    {
        return 0;
    }

    int32_t GetNetFirewallRules(const int32_t userId, const sptr<RequestParam> &requestParam,
        sptr<FirewallRulePage> &info) override
    {
        return 0;
    }

    int32_t GetNetFirewallRule(const int32_t userId, const int32_t ruleId, sptr<NetFirewallRule> &rule) override
    {
        return 0;
    }

    int32_t GetInterceptRecords(const int32_t userId, const sptr<RequestParam> &requestParam,
        sptr<InterceptRecordPage> &info) override
    {
        return 0;
    }

    int32_t RegisterInterceptRecordsCallback(const sptr<INetInterceptRecordCallback> &callback) override
    {
        return 0;
    }

    int32_t UnregisterInterceptRecordsCallback(const sptr<INetInterceptRecordCallback> &callback) override
    {
        return 0;
    }
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // MOCK_NETWORKVPN_SERVICE_STUB_TEST_H
