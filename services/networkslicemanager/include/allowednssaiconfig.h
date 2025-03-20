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

#ifndef ALLOWEDNSSAICONFIG_H
#define ALLOWEDNSSAICONFIG_H

#include <cstdint>
#include <string>
#include <vector>
#include <iostream>
#include <nlohmann/json.hpp>

namespace OHOS {
namespace NetManagerStandard {

class Snssai {
public:
    void setSnssaiLen(uint8_t snssaiLen)
    {
        mSnssaiLen = snssaiLen;
    }

    uint8_t getSnssaiLen() const
    {
        return mSnssaiLen;
    }

    void setSst(uint8_t sst)
    {
        mSst = sst;
    }

    uint8_t getSst() const
    {
        return mSst;
    }

    void setSd(int sd)
    {
        mSd = sd;
    }

    int getSd() const
    {
        return mSd;
    }

    void setMappedSst(uint8_t mappedSst)
    {
        mMappedSst = mappedSst;
    }

    uint8_t getMappedSst() const
    {
        return mMappedSst;
    }

    void setMappedSd(int mappedSd)
    {
        mMappedSd = mappedSd;
    }

    int getMappedSd() const
    {
        return mMappedSd;
    }

    void setSnssai(const std::string& snssai)
    {
        mSnssai = snssai;
    }

    const std::string& getSnssai() const
    {
        return mSnssai;
    }

    nlohmann::json to_json() const
    {
        return nlohmann::json::object({
            {"snssaiLen", mSnssaiLen},
            {"sst", mSst},
            {"sd", mSd},
            {"mappedSst", mMappedSst},
            {"mappedSd", mMappedSd},
            {"snssai", mSnssai}
        });
    }
    void from_json(const nlohmann::json& jsonSnssai)
    {
        if (jsonSnssai.find("snssaiLen") != jsonSnssai.end()) {
            mSnssaiLen = jsonSnssai.at("snssaiLen").get<uint8_t>();
        }

        if (jsonSnssai.find("sst") != jsonSnssai.end()) {
            mSst = jsonSnssai.at("sst").get<uint8_t>();
        }

        if (jsonSnssai.find("sd") != jsonSnssai.end()) {
            mSd = jsonSnssai.at("sd").get<int>();
        }

        if (jsonSnssai.find("mappedSst") != jsonSnssai.end()) {
            mMappedSst = jsonSnssai.at("mappedSst").get<uint8_t>();
        }

        if (jsonSnssai.find("mappedSd") != jsonSnssai.end()) {
            mMappedSd = jsonSnssai.at("mappedSd").get<int>();
        }

        if (jsonSnssai.find("snssai") != jsonSnssai.end()) {
            mSnssai = jsonSnssai.at("snssai").get<std::string>();
        }
    }

private:
    uint8_t mSnssaiLen; // Snssai的长度
    uint8_t mSst; // Service Type
    int mSd; // Service Domain
    uint8_t mMappedSst; // Mapped Service Type
    int mMappedSd; // Mapped Service Domain
    std::string mSnssai; // Snssai的字符串表示
};

class AllowedNssaiConfig {
public:
    static AllowedNssaiConfig& GetInstance();
    ~AllowedNssaiConfig() = default;
    std::vector<Snssai> mAllowedNssais;
    bool ParseSnssai(Snssai& snssai);
    void DecodeAllowedNssai(std::vector<uint8_t> buffer);
    bool DecodeSnssai(int& startIndex, uint8_t len, std::vector<uint8_t> buffer, Snssai& snssai);
    void DumpAllowedNssai();
    std::string FindSnssaiInAllowedNssai(const std::vector<Snssai>& snssais);
    std::string isSnssaiInAllowedNssai(Snssai snssai);
private:
    AllowedNssaiConfig() = default;
};
extern std::shared_ptr<AllowedNssaiConfig> sAllowedNssaiConfig_;

} // namespace NetManagerStandard
} // namespace OHOS
#endif  // ALLOWEDNSSAICONFIG_H
