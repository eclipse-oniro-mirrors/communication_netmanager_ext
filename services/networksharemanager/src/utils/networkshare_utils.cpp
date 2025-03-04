/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include <charconv>
#include "networkshare_utils.h"
#include "netmgr_ext_log_wrapper.h"
 
namespace OHOS {
namespace NetManagerStandard {
 
constexpr int32_t TEN = 10;

bool NetworkShareUtils::ConvertToInt64(const std::string &str, int64_t &value)
{
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value, 10);
    return ec == std::errc{} && ptr == str.data() + str.size();
}

int64_t NetworkShareUtils::Constrain(int64_t amount, int64_t low, int64_t high)
{
    return (amount < low) ? low : (amount > high) ? high : amount;
}
} // namespace NetManagerStandard
} // namespace OHOS