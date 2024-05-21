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

#ifndef NET_FIREWALL_GET_RULE_CONTEXT_H
#define NET_FIREWALL_GET_RULE_CONTEXT_H

#include <cstddef>
#include <napi/native_api.h>

#include "base_context.h"
#include "event_manager.h"
#include "netfirewall_common.h"

namespace OHOS {
namespace NetManagerStandard {
class GetNetFirewallRuleContext : public BaseContext {
public:
    GetNetFirewallRuleContext() = delete;
    
    GetNetFirewallRuleContext(napi_env env, EventManager *manager);

    void ParseParams(napi_value *params, size_t paramsCount);

public:
    int32_t userId_ = 0;
    int32_t ruleId_ = 0;
    sptr<NetFirewallRule> rule_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_FIREWALL_GET_RULE_CONTEXT_H