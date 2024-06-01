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

#ifndef EDM_PARAMETER_UTILS_H
#define EDM_PARAMETER_UTILS_H

#include <mutex>

namespace OHOS {
namespace NetManagerStandard {
class EdmParameterUtils {
public:
    static EdmParameterUtils &GetInstance();
    ~EdmParameterUtils() = default;
    EdmParameterUtils(const EdmParameterUtils&) = delete;
    EdmParameterUtils& operator=(const EdmParameterUtils&) = delete;
    
    bool CheckBoolEdmParameter(const char *key, const char *defaultValue);
    typedef void (*ParameterChgPtr)(const char *key, const char *value, void *context);
    void RegisterEdmParameterChangeEvent(const char *key, ParameterChgPtr callback, void *context);
    void UnRegisterEdmParameterChangeEvent(const char *key);

private:
    EdmParameterUtils() = default;
    
    std::recursive_mutex mutex_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // EDM_PARAMETER_UTILS_H