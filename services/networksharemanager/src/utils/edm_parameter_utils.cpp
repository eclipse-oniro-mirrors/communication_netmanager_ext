/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

#include "edm_parameter_utils.h"

#include "netmgr_ext_log_wrapper.h"
#include "parameter.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr uint32_t PARAM_BUFFER_LENGTH = 128;

EdmParameterUtils &EdmParameterUtils::GetInstance()
{
    static EdmParameterUtils instance;
    return instance;
}

bool EdmParameterUtils::CheckBoolEdmParameter(const char *key, const char *defaultValue)
{
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    char paramOutBuf[PARAM_BUFFER_LENGTH] = {0};
    int ret = GetParameter(key, defaultValue, paramOutBuf, PARAM_BUFFER_LENGTH);
    NETMGR_EXT_LOG_I("NetworkShare StartSharing check EDM param %{public}d", ret);
    if (ret > 0) {
        if (strcmp(paramOutBuf, "true") == 0) {
            return true;
        } else {
            NETMGR_EXT_LOG_E("NetworkShare StartSharing check EDM param result: %{public}s", paramOutBuf);
        }
    }
    return false;
}

void EdmParameterUtils::RegisterEdmParameterChangeEvent(const char *key, ParameterChgPtr callback, void *context)
{
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    int ret = WatchParameter(key, callback, context);
    if (ret != 0) {
        NETMGR_EXT_LOG_E("RegisterEdmParameterChangeEvent %{public}s failed with %{public}d.",
            key, ret);
    }
}

void EdmParameterUtils::UnRegisterEdmParameterChangeEvent(const char *key)
{
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    int ret = RemoveParameterWatcher(key, nullptr, nullptr);
    if (ret != 0) {
        NETMGR_EXT_LOG_E("UnRegisterEdmParameterChangeEvent %{public}s err: %{public}d", key, ret);
    }
}

} // namespace NetManagerStandard
} // namespace OHOS