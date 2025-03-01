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
 
#ifndef NETWORKSHARE_UTILS_H
#define NETWORKSHARE_UTILS_H
 
#include <mutex>
 
namespace OHOS {
namespace NetManagerStandard {
class NetworkShareUtils {
public:
    static NetworkShareUtils &GetInstance();
    NetworkShareUtils() = default;
    ~NetworkShareUtils() = default;
    static bool ConvertToInt64(const std::string &str, int64_t &value);
    static int64_t Constrain(int64_t amount, int64_t low, int64_t high);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETWORKSHARE_UTILS_H