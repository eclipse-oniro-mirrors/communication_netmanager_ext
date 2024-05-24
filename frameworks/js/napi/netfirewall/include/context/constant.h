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

#ifndef NET_EXT_NAPI_ETHERNET_CONSTANT_H
#define NET_EXT_NAPI_ETHERNET_CONSTANT_H

#include <cstdint>

namespace OHOS {
namespace NetManagerStandard {
// No ginseng input
constexpr int32_t PARAM_NONE = 0;
// One input parameter
constexpr int32_t PARAM_JUST_OPTIONS = 1;
// Two input parameters
constexpr int32_t PARAM_DOUBLE_OPTIONS = 2;
constexpr int32_t PARAM_TRIPLE_OPTIONS = 3;
constexpr int32_t PARAM_FOUR_OPTIONS = 4;

// Callback without input parameters
constexpr int32_t PARAM_JUST_CALLBACK = 1;
// Callback when entering a parameter
constexpr int32_t PARAM_OPTIONS_AND_CALLBACK = 2;
// Callback when entering two parameters
constexpr int32_t PARAM_DOUBLE_OPTIONS_AND_CALLBACK = 3;
constexpr int32_t PARAM_TRIPLE_OPTIONS_AND_CALLBACK = 4;
constexpr int32_t PARAM_FOUR_OPTIONS_AND_CALLBACK = 5;

enum {
    ARG_NUM_0 = 0,
    ARG_NUM_1,
    ARG_NUM_2,
};

enum {
    ARG_INDEX_0 = 0,
    ARG_INDEX_1,
    ARG_INDEX_2,
    ARG_INDEX_3,
    ARG_INDEX_4,
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_EXT_NAPI_ETHERNET_CONSTANT_H
