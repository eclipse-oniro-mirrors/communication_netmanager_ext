/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "netshare_issharing_context.h"

#include "constant.h"
#include "napi_utils.h"
#include "netmanager_ext_log.h"

namespace OHOS {
namespace NetManagerStandard {
NetShareIsSharingContext::NetShareIsSharingContext(napi_env env, EventManager *manager) : BaseContext(env, manager) {}

void NetShareIsSharingContext::ParseParams(napi_value *params, size_t paramsCount)
{
    if (!CheckParamsType(params, paramsCount)) {
        NETMANAGER_EXT_LOGE("CheckParamsType failed");
        return;
    }
    if (paramsCount == PARAM_JUST_CALLBACK) {
        SetParseOK(SetCallback(params[ARG_INDEX_0]) == napi_ok);
        return;
    }
    SetParseOK(true);
}

bool NetShareIsSharingContext::CheckParamsType(napi_value *params, size_t paramsCount)
{
    if (paramsCount == PARAM_NONE) {
        return true;
    }
    if (paramsCount == PARAM_JUST_CALLBACK) {
        return NapiUtils::GetValueType(GetEnv(), params[ARG_INDEX_0]) == napi_function;
    }
    return false;
}

void NetShareIsSharingContext::SetSharingSupported(int32_t isSharingSupported)
{
    isSharingSupported_ = isSharingSupported;
}

void NetShareIsSharingContext::SetSharing(int32_t isSharing)
{
    isSharing_ = isSharing;
}

int32_t NetShareIsSharingContext::GetSharingSupported()
{
    return isSharingSupported_;
}

int32_t NetShareIsSharingContext::GetSharing()
{
    return isSharing_;
}
} // namespace NetManagerStandard
} // namespace OHOS
