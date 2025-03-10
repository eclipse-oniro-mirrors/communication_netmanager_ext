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

#include "networksliceutil.h"
#include <locale>
#include <codecvt>
#include <openssl/sha.h>

namespace OHOS {
namespace NetManagerStandard {
static constexpr int CONVERT_INT_AND_BYTE = 0x000000ff;
static constexpr int CONVERT_INT_AND_SHORT = 0x0000ffff;
static constexpr int LEN_UINT32 = 4;
static constexpr int BIT0 = 0;
static constexpr int BIT1 = 1;
static constexpr int BIT2 = 2;
static constexpr int BIT3 = 3;
static constexpr int MOVE_4 = 4;
static constexpr int MOVE_8 = 8;
static constexpr int MOVE_16 = 16;
static constexpr int MOVE_24 = 24;
static constexpr int LEN_IPV6ADDR = 16;
static constexpr int LEN_14 = 4;
static constexpr int LEN_4 = 4;
constexpr int32_t HEX_WIDTH = 2;
const char DEFAULT_STRING[] = "error";
const std::wstring DEFAULT_WSTRING = L"error";
const std::u16string DEFAULT_USTRING = u"error";

std::vector<std::string> Split(const std::string &str, const std::string delimiter)
{
    std::vector<std::string> items;
    size_t start = 0;
    size_t pos = str.find(delimiter);
    while (pos != std::string::npos) {
        std::string item = str.substr(start, pos - start);
        items.emplace_back(item);
        start = pos + 1;
        pos = str.find(delimiter, start);
    }
    if (start < str.length()) {
        std::string tail = str.substr(start);
        items.emplace_back(tail);
    }
    return items;
}

uint8_t ConvertInt2UnsignedByte(int value)
{
    return static_cast<uint8_t>(value & CONVERT_INT_AND_BYTE);
}

short ConvertInt2UnsignedShort(int value)
{
    return static_cast<short>(value & CONVERT_INT_AND_SHORT);
}

int ConvertUnsignedShort2Int(short value)
{
    return value & CONVERT_INT_AND_SHORT;
}

std::string ByteToHexStr(uint8_t byte)
{
    static const char* hexDigits = "0123456789abcdef";
    std::string result;
    result.push_back(hexDigits[(byte >> MOVE_4) & 0x0F]);
    result.push_back(hexDigits[byte & 0x0F]);
    return result;
}

short GetShort(int& startIndex, std::vector<uint8_t> buffer, bool isLitterEndian)
{
    if (startIndex >= (int)buffer.size()) {
        NETMGR_EXT_LOG_E("GetShort fail!");
        return 0;
    }
    std::pair<uint8_t, uint8_t> buff_pair;
    if (!isLitterEndian) {
        buff_pair = std::make_pair(buffer[startIndex], buffer[startIndex + 1]);
    } else {
        buff_pair = std::make_pair(buffer[startIndex + 1], buffer[startIndex]);
    }
    short buffshort = static_cast<short>((buff_pair.first << MOVE_8) | buff_pair.second);
    startIndex += NetworkSliceCommConfig::LEN_SHORT;
    return buffshort;
}

void PutShort(std::vector<uint8_t>& buffer, short value, bool isLitterEndian)
{
    uint8_t lowByte = static_cast<uint8_t>((value >> MOVE_8) & 0xFF);
    uint8_t highByte = static_cast<uint8_t>(value & 0xFF);
    if (isLitterEndian) {
        buffer.push_back(lowByte);
        buffer.push_back(highByte);
    } else {
        buffer.push_back(highByte);
        buffer.push_back(lowByte);
    }
}

int GetInt(int& startIndex, std::vector<uint8_t> buffer, bool isLitterEndian)
{
    if (startIndex >= (int)buffer.size()) {
        NETMGR_EXT_LOG_E("GetShort fail!");
        return 0;
    }
    std::array<uint8_t, NetworkSliceCommConfig::LEN_INT> buff_quad;
    if (!isLitterEndian) {
        buff_quad = {
            buffer[startIndex],
            buffer[startIndex + 1],
            buffer[startIndex + 2],
            buffer[startIndex + 3]
        };
    } else {
        buff_quad = {
            buffer[startIndex + 3],
            buffer[startIndex + 2],
            buffer[startIndex + 1],
            buffer[startIndex]
        };
    }
    int buffint = static_cast<int>((buff_quad[0] << MOVE_24) | (buff_quad[1] << MOVE_16) |
                    (buff_quad[2] << MOVE_8) | buff_quad[3]);
    startIndex += NetworkSliceCommConfig::LEN_INT;
    return buffint;
}

void PutInt(std::vector<uint8_t>& buffer, int value, bool isLitterEndian)
{
    uint8_t lowByte = static_cast<uint8_t>((value >> MOVE_24) & 0xFF);
    uint8_t middleByte = static_cast<uint8_t>((value >> MOVE_16) & 0xFF);
    uint8_t highByte = static_cast<uint8_t>((value >> MOVE_8) & 0xFF);
    uint8_t highestByte = static_cast<uint8_t>(value & 0xFF);
    if (isLitterEndian) {
        buffer.push_back(lowByte);
        buffer.push_back(middleByte);
        buffer.push_back(highByte);
        buffer.push_back(highestByte);
    } else {
        buffer.push_back(highestByte);
        buffer.push_back(highByte);
        buffer.push_back(middleByte);
        buffer.push_back(lowByte);
    }
}

uint32_t vectorToUint32(const std::vector<uint8_t>& vec)
{
    if (vec.size() != LEN_UINT32) {
        NETMGR_EXT_LOG_E("The vector must have exactly 4 elements.");
    }
  
    uint32_t result = 0;
    result |= static_cast<uint32_t>(vec[BIT0]) << MOVE_24;
    result |= static_cast<uint32_t>(vec[BIT1]) << MOVE_16;
    result |= static_cast<uint32_t>(vec[BIT2]) << MOVE_8;
    result |= static_cast<uint32_t>(vec[BIT3]);
    return result;
}

std::vector<uint8_t> uInt32ToVector(uint32_t value)
{
    std::vector<uint8_t> vec(LEN_UINT32);
    vec[BIT0] = static_cast<uint8_t>((value >> MOVE_24) & 0xFF);
    vec[BIT1] = static_cast<uint8_t>((value >> MOVE_16) & 0xFF);
    vec[BIT2] = static_cast<uint8_t>((value >> MOVE_8) & 0xFF);
    vec[BIT3] = static_cast<uint8_t>(value & 0xFF);
    return vec;
}

std::array<uint8_t, LEN_IPV6ADDR> vectorToIPv6Array(const std::vector<uint8_t>& vec)
{
    if (vec.size() != LEN_IPV6ADDR) {
        NETMGR_EXT_LOG_E("The vector must have exactly 16 elements.");
    }
    std::array<uint8_t, LEN_IPV6ADDR> ipv6Addr;
    std::copy(vec.begin(), vec.end(), ipv6Addr.begin());
    return ipv6Addr;
}

std::vector<uint8_t> ConvertstringTouInt8Vector(const std::string& str)
{
    std::vector<uint8_t> vec;
    vec.resize(str.size());
    for (char c : str) {
        vec.push_back(static_cast<uint8_t>(c));
    }
    return vec;
}

std::string transIpv6AddrToStr(std::array<uint8_t, NetworkSliceCommConfig::LEN_IPV6ADDR> Ipv6Addr)
{
    std::string ipv6AddrStr;
    std::stringstream ss;
    for (size_t i = 0; i < Ipv6Addr.size(); i += BIT2) {
        ss << std::hex << std::setw(LEN_4) << std::setfill('0') <<
            static_cast<uint32_t>((Ipv6Addr[i] << MOVE_8) | Ipv6Addr[i + 1]);
        if (i < LEN_14) {
            ss << ":";
        }
    }
    ipv6AddrStr = ss.str();
    return ipv6AddrStr;
}

std::string ConvertIntListToString(const std::vector<int>& intList)
{
    std::string str;
    bool first = true;
    for (int num : intList) {
        if (!first) {
            str += ",";
        } else {
            first = false;
        }
        str += std::to_string(num);
    }
    return str;
}

std::string ConvertUint8vecToString(const std::vector<uint8_t>& vec)
{
    std::string str;
    bool first = true;
    for (uint8_t num : vec) {
        if (!first) {
            str += ",";
        } else {
            first = false;
        }
        str += std::to_string(num);
    }
    return str;
}

std::string ConvertIntSetToString(const std::set<int>& intList)
{
    std::string str;
    for (int num : intList) {
        str += std::to_string(num);
    }
    return str;
}

std::string GetSha256Str(const std::string &str)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, str.c_str(), str.length());
    SHA256_Final(hash, &ctx);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(HEX_WIDTH) << std::setfill('0') << static_cast<uint32_t>(hash[i]);
    }
    return ss.str();
}

std::string Str16ToStr8(std::u16string str)
{
    if (str == DEFAULT_USTRING) {
        return DEFAULT_STRING;
    }
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert(DEFAULT_STRING);
    std::string result = convert.to_bytes(str);
    return result == DEFAULT_STRING ? "" : result;
}

} // namespace NetManagerStandard
} // namespace OHOS
