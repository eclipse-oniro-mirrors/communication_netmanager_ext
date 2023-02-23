/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef MDNS_COMMON_H
#define MDNS_COMMON_H

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {

using MDnsPayload = std::vector<uint8_t>;
using TxtRecord = std::map<std::string, std::vector<uint8_t>>;
using TxtRecordEncoded = std::vector<uint8_t>;

static constexpr int32_t LOAD_SA_TIMEOUT = 5;
static constexpr int32_t SYNC_TIMEOUT = 5;
static constexpr size_t MDNS_MAX_DOMAIN_LABEL = 63;
static constexpr const char *MDNS_TOP_DOMAIN_DEFAULT = ".local";
static constexpr const char *MDNS_DOMAIN_SPLITER_STR = ".";
static constexpr const char *MDNS_HOSTPORT_SPLITER_STR = ":";
static constexpr char MDNS_DOMAIN_SPLITER = '.';

} // namespace NetManagerStandard
} // namespace OHOS
#endif // MDNS_COMMON_H
