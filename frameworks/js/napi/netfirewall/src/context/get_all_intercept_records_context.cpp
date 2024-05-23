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

#include "get_all_intercept_records_context.h"
#include "constant.h"
#include "napi_utils.h"
#include "net_firewall_rule_parse.h"
#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "netfirewall_common.h"

namespace OHOS {
namespace NetManagerStandard {
static bool CheckParamsType(napi_env env, napi_value *params, size_t paramsCount)
{
    if (paramsCount == PARAM_DOUBLE_OPTIONS || paramsCount == PARAM_DOUBLE_OPTIONS_AND_CALLBACK) {
        if (NapiUtils::GetValueType(env, params[ARG_INDEX_0]) != napi_number ||
            NapiUtils::GetValueType(env, params[ARG_INDEX_1]) != napi_object) {
            return false;
        }
    } else {
        return false;
    }
    return true;
}

GetAllInterceptRecordsContext::GetAllInterceptRecordsContext(napi_env env, EventManager *manager)
    : BaseContext(env, manager)
{}

void GetAllInterceptRecordsContext::ParseParams(napi_value *params, size_t paramsCount)
{
    if (!CheckParamsType(GetEnv(), params, paramsCount)) {
        SetErrorCode(FIREWALL_ERR_PARAMETER_ERROR);
        return;
    }

    userId_ = NapiUtils::GetInt32FromValue(GetEnv(), params[ARG_INDEX_0]);
    if (userId_ <= 0) {
        NETMGR_EXT_LOG_E("userid param invalid.");
        SetErrorCode(FIREWALL_ERR_INVALID_PARAMETER);
        return;
    }

    requestParam_ = new (std::nothrow) RequestParam();
    if (requestParam_ == nullptr) {
        NETMGR_EXT_LOG_E("firewall requestParam_ object is nullptr.");
        SetErrorCode(FIREWALL_ERR_INTERNAL);
        return;
    }
    int32_t ret = NetFirewallRuleParse::ParsePageParam(GetEnv(), params[ARG_INDEX_1], requestParam_, false);
    if (ret != FIREWALL_SUCCESS) {
        SetErrorCode(FIREWALL_ERR_INVALID_PARAMETER);
        return;
    }
    if (paramsCount == PARAM_DOUBLE_OPTIONS_AND_CALLBACK) {
        SetParseOK(SetCallback(params[ARG_INDEX_2]) == napi_ok);
        return;
    }
    SetParseOK(true);
}
} // namespace NetManagerStandard
} // namespace OHOS