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

#ifndef NETMGR_EXT_UUID_H
#define NETMGR_EXT_UUID_H

#include <string>
#include <array>

namespace OHOS {
namespace NetManagerStandard {
class UUID {
public:
    //128 bits uuid length
    const static int UUID128_BYTES_LEN = 16;

    UUID(){};

    ~UUID(){};

    /**
     * @brief A constructor used to create an <b>UUID</b> instance. Constructor a new random UUID.
     *
     * @return Returns a random UUID.
     */
    static UUID RandomUUID();

    /**
     * @brief Convert UUID to string.
     *
     * @return Returns a String object representing this UUID.
     */
    std::string ToString() const;

private:
    std::array<uint8_t, UUID128_BYTES_LEN> uuid_ = {0x00};
};

using ParcelUuid = UUID;
} // namespace NetManagerStandard
} // namespace OHOS

#endif  //NETMGR_EXT_UUID_H