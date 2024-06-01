/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "get_context.h"
#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "napi_utils.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t PARAM_JUST_OPTIONS = 1;
constexpr int32_t PARAM_OPTIONS_AND_CALLBACK = 2;

GetContext::GetContext(napi_env env, EventManager *manager) : BaseContext(env, manager) {}

bool GetContext::CheckParamsType(napi_env env, napi_value *params, size_t paramsCount)
{
    switch (paramsCount) {
        case PARAM_JUST_OPTIONS:
            return (NapiUtils::GetValueType(env, params[0]) == napi_string);
        case PARAM_OPTIONS_AND_CALLBACK:
            return ((NapiUtils::GetValueType(env, params[0]) == napi_string) &&
                    (NapiUtils::GetValueType(env, params[1]) == napi_function));
        default:
            return false;
    }
}

void GetContext::ParseParams(napi_value *params, size_t paramsCount)
{
    if (!CheckParamsType(GetEnv(), params, paramsCount)) {
        NETMGR_EXT_LOG_E("params type is mismatch");
        SetNeedThrowException(true);
        SetErrorCode(NETMANAGER_EXT_ERR_PARAMETER_ERROR);
        return;
    }
    vpnId_ = NapiUtils::GetStringFromValueUtf8(GetEnv(), params[0]);
    if (paramsCount == PARAM_OPTIONS_AND_CALLBACK) {
        SetParseOK(SetCallback(params[1]) == napi_ok);
        return;
    }
    SetParseOK(true);
}
} // namespace NetManagerStandard
} // namespace OHOS