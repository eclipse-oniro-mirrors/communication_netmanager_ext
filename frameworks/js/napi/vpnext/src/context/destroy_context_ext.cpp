/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "destroy_context_ext.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t PARAM_NONE = 0;
constexpr int32_t PARAM_JUST_CALLBACK = 1;
constexpr int32_t PARAM_OPTIONS_AND_CALLBACK = 2;

DestroyContext::DestroyContext(napi_env env, EventManager *manager) : BaseContext(env, manager) {}

void DestroyContext::ParseParams(napi_value *params, size_t paramsCount)
{
    switch (paramsCount) {
        case PARAM_NONE:
            SetParseOK(true);
            break;
        case PARAM_JUST_CALLBACK:
            if (NapiUtils::GetValueType(GetEnv(), params[0]) == napi_function) {
                SetParseOK(SetCallback(params[0]) == napi_ok);
                return;
            }
            if (NapiUtils::GetValueType(GetEnv(), params[0]) == napi_string) {
                vpnId_ = NapiUtils::GetStringFromValueUtf8(GetEnv(), params[0]);
                SetParseOK(true);
                return;
            }
            SetErrorCode(NETMANAGER_EXT_ERR_PARAMETER_ERROR);
            SetNeedThrowException(true);
            break;
        case PARAM_OPTIONS_AND_CALLBACK:
            if ((NapiUtils::GetValueType(GetEnv(), params[0]) == napi_string) &&
                (NapiUtils::GetValueType(GetEnv(), params[1]) == napi_function)) {
                vpnId_ = NapiUtils::GetStringFromValueUtf8(GetEnv(), params[0]);
                SetParseOK(SetCallback(params[1]) == napi_ok);
            } else {
                SetErrorCode(NETMANAGER_EXT_ERR_PARAMETER_ERROR);
                SetNeedThrowException(true);
            }
            break;
        default:
            SetErrorCode(NETMANAGER_EXT_ERR_PARAMETER_ERROR);
            SetNeedThrowException(true);
            break;
    }
}
} // namespace NetManagerStandard
} // namespace OHOS
