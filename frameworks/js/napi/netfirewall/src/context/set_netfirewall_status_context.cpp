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

#include "set_netfirewall_status_context.h"
#include "constant.h"
#include "napi_utils.h"
#include "net_firewall_param_check.h"
#include "net_firewall_rule_parse.h"
#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "netfirewall_common.h"

namespace OHOS {
namespace NetManagerStandard {
static bool CheckParamsType(napi_env env, napi_value *params, size_t count)
{
    if (count == PARAM_DOUBLE_OPTIONS || count == PARAM_DOUBLE_OPTIONS_AND_CALLBACK) {
        if (NapiUtils::GetValueType(env, params[ARG_INDEX_0]) != napi_number ||
            NapiUtils::GetValueType(env, params[ARG_INDEX_1]) != napi_object) {
            return false;
        }
    } else {
        // if count is not 2 or 3, means count error.
        return false;
    }
    return true;
}

SetNetFirewallStatusContext::SetNetFirewallStatusContext(napi_env env, EventManager *manager)
    : BaseContext(env, manager)
{}

void SetNetFirewallStatusContext::ParseParams(napi_value *params, size_t paramsCount)
{
    if (!CheckParamsType(GetEnv(), params, paramsCount)) {
        SetErrorCode(FIREWALL_ERR_PARAMETER_ERROR);
        return;
    }

    userId_ = NapiUtils::GetInt32FromValue(GetEnv(), params[ARG_INDEX_0]);
    if (userId_ < 0) {
        NETMGR_EXT_LOG_E("SetNetFirewallStatusContext userId parma invalid.");
        SetErrorCode(FIREWALL_ERR_INVALID_PARAMETER);
        return;
    }

    int ret = NetFirewallParamCheck::CheckFirewallRuleStatus(GetEnv(), params[ARG_INDEX_1]);
    if (ret != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("rule action parma invalid.");
        SetErrorCode(ret);
        return;
    }

    status_ = new (std::nothrow) NetFirewallStatus();
    if (status_ == nullptr) {
        NETMGR_EXT_LOG_E("firewall rule object is nullptr.");
        SetErrorCode(FIREWALL_ERR_INTERNAL);
        return;
    }
    status_->isOpen = NapiUtils::GetBooleanProperty(GetEnv(), params[ARG_INDEX_1], NET_FIREWALL_IS_OPEN);
    status_->inAction = static_cast<FirewallRuleAction>(
        NapiUtils::GetInt32Property(GetEnv(), params[ARG_INDEX_1], NET_FIREWALL_IN_ACTION));
    status_->outAction = static_cast<FirewallRuleAction>(
        NapiUtils::GetInt32Property(GetEnv(), params[ARG_INDEX_1], NET_FIREWALL_OUT_ACTION));
    if (paramsCount == PARAM_DOUBLE_OPTIONS_AND_CALLBACK) {
        SetParseOK(SetCallback(params[ARG_INDEX_2]) == napi_ok);
        return;
    }
    SetParseOK(true);
}
} // namespace NetManagerStandard
} // namespace OHOS