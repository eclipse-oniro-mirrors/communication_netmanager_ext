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

#include <cerrno>
#include <ctime>
#include <regex>

#include "sys/time.h"

#define CONSTANT_ZERO 0
#define CONSTANT_ONE 1
#define CONSTANT_TWO 2
#define CONSTANT_THREE 3
#define CONSTANT_FOUR 4
#define CONSTANT_FIVE 5
#define CONSTANT_SIX 6
#define CONSTANT_SEVEN 7
#define CONSTANT_EIGHT 8
#define CONSTANT_NINE 9
#define CONSTANT_TEN 10
#define CONSTANT_ELEVEN 11
#define CONSTANT_TWELVE 12
#define CONSTANT_THIRTEEN 13
#define CONSTANT_FOURTEEN 14
#define CONSTANT_FIFTEEN 15
#define CONSTANT_SIXTEEN 16
#define CONSTANT_TWENTY 20
#define CONSTANT_TWENTY_FOUR 24
#define CONSTANT_THIRTY_TWO 32
#define CONSTANT_FOURTY 40
#define CONSTANT_FOURTY_EIGHT 48
#define CONSTANT_FIFTY_SIX 56

namespace OHOS {
namespace NetManagerStandard {

UUID UUID::RandomUUID()
{
    UUID random;

    struct timeval tv;
    struct timezone tz;
    struct tm randomTime;
    unsigned int randNum = 0;

    rand_r(&randNum);
    gettimeofday(&tv, &tz);
    localtime_r(&tv.tv_sec, &randomTime);
    random.uuid_[CONSTANT_FIFTEEN] = static_cast<uint8_t>(tv.tv_usec & 0x00000000000000FF);
    random.uuid_[CONSTANT_FOURTEEN] = static_cast<uint8_t>((tv.tv_usec & 0x000000000000FF00) >> CONSTANT_EIGHT);
    random.uuid_[CONSTANT_THIRTEEN] = static_cast<uint8_t>((tv.tv_usec & 0x0000000000FF0000) >> CONSTANT_SIXTEEN);
    random.uuid_[CONSTANT_TWELVE] = static_cast<uint8_t>((tv.tv_usec & 0x00000000FF000000) >> CONSTANT_TWENTY_FOUR);
    random.uuid_[CONSTANT_TEN] = static_cast<uint8_t>((tv.tv_usec & 0x000000FF00000000) >> CONSTANT_THIRTY_TWO);
    random.uuid_[CONSTANT_NINE] = static_cast<uint8_t>((tv.tv_usec & 0x0000FF0000000000) >> CONSTANT_FOURTY);
    random.uuid_[CONSTANT_EIGHT] = static_cast<uint8_t>((tv.tv_usec & 0x00FF000000000000) >> CONSTANT_FOURTY_EIGHT);
    random.uuid_[CONSTANT_SEVEN] = static_cast<uint8_t>((tv.tv_usec & 0xFF00000000000000) >> CONSTANT_FIFTY_SIX);
    random.uuid_[CONSTANT_SIX] = static_cast<uint8_t>((randomTime.tm_sec + static_cast<int>(randNum)) & 0xFF);
    random.uuid_[CONSTANT_FIVE] = static_cast<uint8_t>((randomTime.tm_min + (randNum >> CONSTANT_EIGHT)) & 0xFF);
    random.uuid_[CONSTANT_FOUR] = static_cast<uint8_t>((randomTime.tm_hour + (randNum >> CONSTANT_SIXTEEN)) & 0xFF);
    random.uuid_[CONSTANT_THREE] = static_cast<uint8_t>((randomTime.tm_mday +
        (randNum >> CONSTANT_TWENTY_FOUR)) & 0xFF);
    random.uuid_[CONSTANT_TWO] = static_cast<uint8_t>(randomTime.tm_mon & 0xFF);
    random.uuid_[CONSTANT_ONE] = static_cast<uint8_t>(randomTime.tm_year & 0xFF);
    random.uuid_[CONSTANT_ZERO] = static_cast<uint8_t>((randomTime.tm_year & 0xFF00) >> CONSTANT_EIGHT);
    return random;
}

std::string UUID::ToString() const
{
    std::string tmp = "";
    std::string ret = "";
    static const char *hex = "0123456789ABCDEF";

    for (auto it = this->uuid_.begin(); it != this->uuid_.end(); it++) {
        tmp.push_back(hex[(((*it) >> CONSTANT_FOUR) & 0xF)]);
        tmp.push_back(hex[(*it) & 0xF]);
    }
    ret = tmp.substr(CONSTANT_ZERO, CONSTANT_EIGHT) + "-" +
            tmp.substr(CONSTANT_EIGHT, CONSTANT_FOUR) + "-" +
            tmp.substr(CONSTANT_TWELVE, CONSTANT_FOUR) + "-" +
            tmp.substr(CONSTANT_SIXTEEN, CONSTANT_FOUR) + "-" +
            tmp.substr(CONSTANT_TWENTY);

    return ret;
}
}
}