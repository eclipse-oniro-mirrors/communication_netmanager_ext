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

#include "update_netfirewall_rule_context.h"
#include "constant.h"
#include "napi_utils.h"
#include "net_firewall_param_check.h"
#include "net_firewall_rule_parse.h"
#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "netfirewall_common.h"

namespace OHOS {
namespace NetManagerStandard {
static bool CheckParamsType(napi_env env, napi_value *params, size_t paramsCount)
{
    if (paramsCount == PARAM_JUST_OPTIONS || paramsCount == PARAM_OPTIONS_AND_CALLBACK) {
        if (NapiUtils::GetValueType(env, params[ARG_INDEX_0]) != napi_object) {
            return false;
        }
    } else {
        return false;
    }
    return true;
}

UpdateNetFirewallRuleContext::UpdateNetFirewallRuleContext(napi_env env, std::shared_ptr<EventManager>& manager)
    : BaseContext(env, manager)
{}

void UpdateNetFirewallRuleContext::ParseParams(napi_value *params, size_t paramsCount)
{
    if (!CheckParamsType(GetEnv(), params, paramsCount)) {
        SetErrorCode(FIREWALL_ERR_PARAMETER_ERROR);
        return;
    }

    int32_t ret = NetFirewallParamCheck::CheckUpdateFirewallRule(GetEnv(), params[ARG_INDEX_0]);
    if (ret != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("UpdateNetFirewallRuleContext firewall rule parma invalid.");
        SetErrorCode(ret);
        return;
    }

    rule_ = new (std::nothrow) NetFirewallRule();
    if (rule_ == nullptr) {
        NETMGR_EXT_LOG_E("UpdateNetFirewallRuleContext firewall rule object is nullptr.");
        SetErrorCode(FIREWALL_ERR_INTERNAL);
        return;
    }
    NetFirewallRuleParse::ParseRuleParams(GetEnv(), params[ARG_INDEX_0], rule_);
    if (paramsCount == PARAM_OPTIONS_AND_CALLBACK) {
        SetParseOK(SetCallback(params[ARG_INDEX_1]) == napi_ok);
        return;
    }
    SetParseOK(true);
}
} // namespace NetManagerStandard
} // namespace OHOS