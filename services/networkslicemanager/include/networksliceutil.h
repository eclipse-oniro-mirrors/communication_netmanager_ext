/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef NETWORKSLICEUTIL_H
#define NETWORKSLICEUTIL_H

#include <cstdint>
#include <string>
#include <vector>
#include <sstream>
#include <nlohmann/json.hpp>
#include <bit>
#include <set>
#include "netmgr_ext_log_wrapper.h"
#include "networkslicecommconfig.h"

namespace OHOS {
namespace NetManagerStandard {
    std::vector<std::string> Split(const std::string &str, const std::string delimiter);
    uint8_t ConvertInt2UnsignedByte(int value);
    std::string ByteToHexStr(uint8_t byte);
    short GetShort(int& startIndex, std::vector<uint8_t> buffer, bool isLitterEndian = false);
    int32_t GetInt(int& startIndex, std::vector<uint8_t> buffer, bool isLitterEndian = false);
    uint32_t vectorToUint32(const std::vector<uint8_t>& vec);
    std::array<uint8_t, 16> vectorToIPv6Array(const std::vector<uint8_t>& vec);
    int ConvertUnsignedShort2Int(short value);
    short ConvertInt2UnsignedShort(int value);
    std::string transIpv6AddrToStr(std::array<uint8_t, NetworkSliceCommConfig::LEN_IPV6ADDR> Ipv6Addr);
    void PutShort(std::vector<uint8_t>& buffer, short value, bool isLitterEndian = true);
    void PutInt(std::vector<uint8_t>& buffer, int value, bool isLitterEndian = true);
    std::vector<uint8_t> ConvertstringTouInt8Vector(const std::string& str);
    std::string ConvertIntListToString(const std::vector<int>& intList);
    std::vector<uint8_t> uInt32ToVector(uint32_t value);
    std::string ConvertIntSetToString(const std::set<int>& intList);
    std::string ConvertUint8vecToString(const std::vector<uint8_t>& vec);
    std::string GetSha256Str(const std::string &str);
}
}

#endif  //NETWORKSLICEUTIL_H
