/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "uuid.h"
#include "sys/random.h"
#include "netmanager_ext_log.h"

namespace OHOS {
namespace NetManagerStandard {

static constexpr std::size_t UUID_STRING_LENGTH = 36;
static constexpr std::size_t UUID_DASH_POS_1 = 4;
static constexpr std::size_t UUID_DASH_POS_2 = 6;
static constexpr std::size_t UUID_DASH_POS_3 = 8;
static constexpr std::size_t UUID_DASH_POS_4 = 10;
static constexpr std::size_t HALF_BYTE_SHIFT = 4;

UUID UUID::RandomUUID()
{
    UUID random;
    auto ret = getrandom(random.uuid_.data(), random.uuid_.size(), 0);
    if (ret < 0) {
        NETMANAGER_BASE_LOGE("get random failed");
    }
    return random;
}

std::string UUID::ToString() const
{
    std::string ret;
    ret.reserve(UUID_STRING_LENGTH);

    static const char hex[] = "0123456789abcdef";

    for (std::size_t i = 0; i < UUID128_BYTES_LEN; ++i) {
        if (i == UUID_DASH_POS_1 || i == UUID_DASH_POS_2 ||
            i == UUID_DASH_POS_3 || i == UUID_DASH_POS_4) {
            ret.push_back('-');
        }
        ret.push_back(hex[uuid_[i] >> HALF_BYTE_SHIFT]);
        ret.push_back(hex[uuid_[i] & 0xF]);
    }

    return ret;
}
} // namespace NetManagerStandard
} // namespace OHOS
