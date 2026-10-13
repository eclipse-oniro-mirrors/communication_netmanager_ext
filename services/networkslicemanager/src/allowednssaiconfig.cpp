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
#include "allowednssaiconfig.h"
#include "networksliceutil.h"
#include "state_utils.h"
#include <charconv>

namespace OHOS {
namespace NetManagerStandard {
const std::string SST_AND_SD_SEPARATOR = ".";
constexpr int SNSSAI_LEN_1_SST = 1;
constexpr int SNSSAI_LEN_2_SST_MAPPED_SST = 2;
constexpr int SNSSAI_LEN_4_SST_SD = 4;
constexpr int SNSSAI_LEN_5_SST_SD_MAPPED_SST = 5;
constexpr int SNSSAI_LEN_8_SST_SD_MAPPED_SST_MAPPED_SD = 8;
constexpr int RADIX = 16;

static bool ParseHexStringToInt(const std::string &str, int32_t &value)
{
    if (str.empty()) {
        return false;
    }
    const char *begin = str.data();
    const char *end = str.data() + str.size();
    auto [ptr, ec] = std::from_chars(begin, end, value, RADIX);
    return ec == std::errc{} && ptr == end;
}

std::shared_ptr<AllowedNssaiConfig> sAllowedNssaiConfig_ =
    std::make_shared<AllowedNssaiConfig>(AllowedNssaiConfig::GetInstance());

AllowedNssaiConfig& AllowedNssaiConfig::GetInstance()
{
    static AllowedNssaiConfig instance;
    return instance;
}

bool AllowedNssaiConfig::ParseSnssai(Snssai& snssai)
{
    std::vector<std::string> values;
    std::vector<std::string> snssaiValues;
    if (snssai.getSnssai().empty()) {
        NETMGR_EXT_LOG_E("invalid snssai to parse");
        return false;
    }

    values = Split(snssai.getSnssai(), ";");
    NETMGR_EXT_LOG_I("valuessize = %{public}d", (int)values.size());
    if (values.size() != 1 && values.size() != NetworkSliceCommConfig::LEN_SHORT) {
        NETMGR_EXT_LOG_E("values.length invalid, values.length = [%{public}d]", (int)values.size());
        return false;
    }
    snssaiValues = Split(values[0], SST_AND_SD_SEPARATOR);
    NETMGR_EXT_LOG_I("snssaiValuessize = %{public}d", (int)snssaiValues.size());
    for (size_t i = 0; i < snssaiValues.size(); ++i) {
        NETMGR_EXT_LOG_I("snssaiValues[%{public}s]", snssaiValues[i].c_str());
    }
    int32_t sstValue = 0;
    int32_t sdValue = 0;
    if (snssaiValues.size() == 1) {
        if (!ParseHexStringToInt(snssaiValues[0], sstValue)) {
            NETMGR_EXT_LOG_E("invalid sst string: [%{public}s]", snssaiValues[0].c_str());
            return false;
        }
        snssai.setSst(ConvertInt2UnsignedByte(sstValue));
        NETMGR_EXT_LOG_I("size = 1, Sst = %{public}d", snssai.getSst());
        snssai.setSnssaiLen(1);
    } else if (snssaiValues.size() == NetworkSliceCommConfig::LEN_SHORT) {
        if (!ParseHexStringToInt(snssaiValues[0], sstValue) ||
            !ParseHexStringToInt(snssaiValues[1], sdValue)) {
            NETMGR_EXT_LOG_E("invalid sst/sd string: [%{public}s]:[%{public}s]",
                snssaiValues[0].c_str(), snssaiValues[1].c_str());
            return false;
        }
        snssai.setSst(ConvertInt2UnsignedByte(sstValue));
        snssai.setSd(sdValue);
        NETMGR_EXT_LOG_I("size = 2, Sst = %{public}d, Sd = %{public}d", snssai.getSst(), snssai.getSd());
        snssai.setSnssaiLen(NetworkSliceCommConfig::LEN_INT);
    } else {
        NETMGR_EXT_LOG_E("values[0].snssaiValues.length invalid, snssaiValues.length = [%{public}d]",
            (int)snssaiValues.size());
        return false;
    }

    if (values.size() == NetworkSliceCommConfig::LEN_SHORT) {
        snssaiValues = Split(values[1], SST_AND_SD_SEPARATOR);
        if (snssaiValues.size() == 1) {
            if (!ParseHexStringToInt(snssaiValues[0], sstValue)) {
                NETMGR_EXT_LOG_E("invalid mapped sst string: [%{public}s]", snssaiValues[0].c_str());
                return false;
            }
            snssai.setMappedSst(ConvertInt2UnsignedByte(sstValue));
            snssai.setSnssaiLen(snssai.getSnssaiLen() + 1);
        } else if (snssaiValues.size() == NetworkSliceCommConfig::LEN_SHORT) {
            if (!ParseHexStringToInt(snssaiValues[0], sstValue) ||
                !ParseHexStringToInt(snssaiValues[1], sdValue)) {
                NETMGR_EXT_LOG_E("invalid mapped sst/sd string: [%{public}s]:[%{public}s]",
                    snssaiValues[0].c_str(), snssaiValues[1].c_str());
                return false;
            }
            snssai.setMappedSst(ConvertInt2UnsignedByte(sstValue));
            snssai.setMappedSd(sdValue);
            snssai.setSnssaiLen(snssai.getSnssaiLen() + NetworkSliceCommConfig::LEN_INT);
        } else {
            NETMGR_EXT_LOG_E("values[1].snssaiValues.length invalid, snssaiValues.length = [%{public}d]",
                (int)snssaiValues.size());
            return false;
        }
    }
    return true;
}

void AllowedNssaiConfig::DecodeAllowedNssai(std::vector<uint8_t> buffer)
{
    if (buffer.empty()) {
        NETMGR_EXT_LOG_I("getAllowedNssai failed, invalid buffer");
        return;
    }
    NETMGR_EXT_LOG_I("start decode AllowedNssai");
    int inputLen = buffer.size();
    mAllowedNssais.clear();
    std::string nssai = "";
    std::vector<std::string> values;
    for (int i = 0; i < inputLen; ++i) {
        nssai += static_cast<char>(buffer[i]);
    }
    values = Split(nssai, ":");
    for (int i = 0; i < (int)values.size(); i++) {
        NETMGR_EXT_LOG_I("start decode AllowedNssai %{public}s", values[i].c_str());
        Snssai snssai;
        snssai.setSnssai(values[i]);
        if (!ParseSnssai(snssai)) {
            NETMGR_EXT_LOG_I("parseSNssai(sNssai) is false");
            continue;
        }
        mAllowedNssais.push_back(snssai);
    }
    NETMGR_EXT_LOG_I("end decode AllowedNssai");
    DumpAllowedNssai();
    return;
}

bool AllowedNssaiConfig::DecodeSnssai(int& startIndex, uint8_t len, std::vector<uint8_t> buffer, Snssai& snssai)
{
    int msd = 0;
    int mappedsd = 0;
    switch (len) {
        case SNSSAI_LEN_1_SST: /* Length of S-NSSAI contents = 1, SST */
            snssai.setSnssaiLen(SNSSAI_LEN_1_SST);
            snssai.setSst(buffer[startIndex++]);
            break;
        case SNSSAI_LEN_2_SST_MAPPED_SST: /* Length of S-NSSAI contents = 2, SST and mapped HPLMN SST */
            snssai.setSnssaiLen(SNSSAI_LEN_2_SST_MAPPED_SST);
            snssai.setSst(buffer[startIndex++]);
            snssai.setMappedSst(buffer[startIndex++]);
            break;
        case SNSSAI_LEN_4_SST_SD: /* Length of S-NSSAI contents = 4, SST and SD */
            snssai.setSnssaiLen(SNSSAI_LEN_4_SST_SD);
            snssai.setSst(buffer[startIndex++]);
            msd = buffer[startIndex++] << NetworkSliceCommConfig::BIT_MOVE_16;
            msd += buffer[startIndex++] << NetworkSliceCommConfig::BIT_MOVE_8;
            msd += buffer[startIndex++];
            snssai.setSd(msd);
            break;
        case SNSSAI_LEN_5_SST_SD_MAPPED_SST: /* Length of S-NSSAI contents = 5, SST, SD and mapped HPLMN SST */
            snssai.setSnssaiLen(SNSSAI_LEN_5_SST_SD_MAPPED_SST);
            snssai.setSst(buffer[startIndex++]);
            msd = buffer[startIndex++] << NetworkSliceCommConfig::BIT_MOVE_16;
            msd += buffer[startIndex++] << NetworkSliceCommConfig::BIT_MOVE_8;
            msd += buffer[startIndex++];
            snssai.setSd(msd);
            snssai.setMappedSst(buffer[startIndex++]);
            break;
        case SNSSAI_LEN_8_SST_SD_MAPPED_SST_MAPPED_SD:
            /* Length of S-NSSAI contents = 8, SST, SD, mapped HPLMN SST and mapped HPLMN SD */
            snssai.setSnssaiLen(SNSSAI_LEN_8_SST_SD_MAPPED_SST_MAPPED_SD);
            snssai.setSst(buffer[startIndex++]);
            msd = buffer[startIndex++] << NetworkSliceCommConfig::BIT_MOVE_16;
            msd += buffer[startIndex++] << NetworkSliceCommConfig::BIT_MOVE_8;
            msd += buffer[startIndex++];
            snssai.setSd(msd);
            snssai.setMappedSst(buffer[startIndex++]);
            mappedsd = buffer[startIndex++] << NetworkSliceCommConfig::BIT_MOVE_16;
            mappedsd += buffer[startIndex++] << NetworkSliceCommConfig::BIT_MOVE_8;
            mappedsd += buffer[startIndex++];
            snssai.setMappedSd(mappedsd);
            break;
        default:
            NETMGR_EXT_LOG_E("Length of S-NSSAI contents invalid, len = %{public}d", len);
            return false;
        }
    return true;
}


void AllowedNssaiConfig::DumpAllowedNssai()
{
    NETMGR_EXT_LOG_I("dump AllowedNssaiConfig.mAllowedNssais begin");
    int i;
    for (i = 0; i < (int)mAllowedNssais.size(); i++) {
        std::string snssai = mAllowedNssais[i].getSnssai();
        NETMGR_EXT_LOG_I("mSnssaiLen = %{public}d, mSst = %{public}d, mSd = %{public}d, "
            "mMappedSst = %{public}d, mMappedSd = %{public}d, mSnssai = %{public}s ",
            mAllowedNssais[i].getSnssaiLen(), mAllowedNssais[i].getSst(), mAllowedNssais[i].getSd(),
            mAllowedNssais[i].getMappedSst(), mAllowedNssais[i].getMappedSd(), snssai.c_str());
    }
    NETMGR_EXT_LOG_I("dump end");
}

std::string AllowedNssaiConfig::FindSnssaiInAllowedNssai(const std::vector<Snssai>& snssais)
{
    int slotId = StateUtils::GetPrimarySlotId();
    if (StateUtils::IsRoaming(slotId)) {
        /* currently donot consider roaming situation */
        return "";
    }
    for (const auto& snssai : snssais) {
        auto it = std::find_if(mAllowedNssais.begin(), mAllowedNssais.end(), [&snssai](const Snssai& nssai) {
            return snssai.getSnssaiLen() == nssai.getSnssaiLen() &&
                   snssai.getSst() == nssai.getSst() &&
                   snssai.getSd() == nssai.getSd() &&
                   snssai.getMappedSst() == nssai.getMappedSst() &&
                   snssai.getMappedSd() == nssai.getMappedSd();
        });
        if (it != mAllowedNssais.end()) {
            return it->getSnssai();
        }
    }
    return "";
}

std::string AllowedNssaiConfig::isSnssaiInAllowedNssai(Snssai snssai)
{
    int slotId = StateUtils::GetPrimarySlotId();
    if (StateUtils::IsRoaming(slotId)) {
        /* currently donot consider roaming situation */
        NETMGR_EXT_LOG_I("isSnssaiInAllowedNssai IsRoaming");
        return "";
    }
    NETMGR_EXT_LOG_I("mAllowedNssais size = %{public}d", (int)mAllowedNssais.size());
    for (int j = 0; j < (int)mAllowedNssais.size(); ++j) {
        NETMGR_EXT_LOG_I("snssai Len = %{public}d, AllowedNssais Len = %{public}d",
            snssai.getSnssaiLen(), mAllowedNssais[j].getSnssaiLen());
        NETMGR_EXT_LOG_I("snssai Sst = %{public}d, AllowedNssais Sst = %{public}d",
            snssai.getSst(), mAllowedNssais[j].getSst());
        NETMGR_EXT_LOG_I("snssai Sd = %{public}d, AllowedNssais Sd = %{public}d",
            snssai.getSd(), mAllowedNssais[j].getSd());
        NETMGR_EXT_LOG_I("snssai MapSst = %{public}d, AllowedNssais MapSst = %{public}d",
            snssai.getMappedSst(), mAllowedNssais[j].getMappedSst());
        NETMGR_EXT_LOG_I("snssai MapSd = %{public}d, AllowedNssais MapSd = %{public}d",
            snssai.getMappedSd(), mAllowedNssais[j].getMappedSd());
            if (snssai.getSnssaiLen() == mAllowedNssais[j].getSnssaiLen() &&
                   snssai.getSst() == mAllowedNssais[j].getSst() &&
                   snssai.getSd() == mAllowedNssais[j].getSd() &&
                   snssai.getMappedSst() == mAllowedNssais[j].getMappedSst() &&
                   snssai.getMappedSd() == mAllowedNssais[j].getMappedSd()) {
                return mAllowedNssais[j].getSnssai();
            }
        };
    return "";
}

}
}
