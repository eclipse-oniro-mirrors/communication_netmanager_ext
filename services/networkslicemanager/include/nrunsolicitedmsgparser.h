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

#ifndef NRUNSOLICITEDMSGPARSER_H
#define NRUNSOLICITEDMSGPARSER_H

#include <cstdint>
#include <string>
#include <vector>
#include <any>
#include <memory>
#include <unordered_map>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <utility>
#include <mutex>
#include "hwnetworkslicemanager.h"
#include "urspconfig.h"
#include "networksliceutil.h"

namespace OHOS {
namespace NetManagerStandard {
static constexpr int PLMN_LEN = 3;
static constexpr int RADIX = 10;
static constexpr short RESULT_LEN = 5;
static constexpr int CONVERT_INT_AND_BYTE = 0x000000ff;
static constexpr int CONVERT_INT_AND_SHORT = 0x0000ffff;
static constexpr int ARRAY_INDEX_0 = 0;
static constexpr int ARRAY_INDEX_1 = 1;
static constexpr int ARRAY_INDEX_2 = 2;
static constexpr uint8_t CAUSE_PROTOCOL_ERROR = 0x6F;

class PolicyInstruction {
public:
    bool hasUrspType;
    std::vector<UrspRule> urspRules;

    nlohmann::json to_json() const
    {
        nlohmann::json policyinstructionJson = nlohmann::json::object({
            {"hasUrspType", hasUrspType},
        });
        for (const auto& urspRule : urspRules) {
            policyinstructionJson["urspRules"].push_back(urspRule.to_json());
        }
        return policyinstructionJson;
    }

    void from_json(const nlohmann::json& jsonPolicyInstruction)
    {
        if (jsonPolicyInstruction.find("hasUrspType") != jsonPolicyInstruction.end()) {
            this->hasUrspType = jsonPolicyInstruction.at("hasUrspType").get<bool>();
        }

        if (jsonPolicyInstruction.find("urspRules") != jsonPolicyInstruction.end()) {
            const nlohmann::json& urspRulesJson = jsonPolicyInstruction.at("urspRules");
            this->urspRules.clear();
            for (const auto& ruleJson : urspRulesJson) {
                UrspRule rule;
                rule.from_json(ruleJson);
                this->urspRules.push_back(rule);
            }
        }
    }
};

class UePolicy {
public:
    std::vector<uint8_t> plmn = std::vector<uint8_t>(PLMN_LEN);
    /**
    * instruction order and upsc HashMap
    */
    std::unordered_map<short, short> instructionOrderMap;
    /**
    * upsc and PolicyInstruction HashMap
    */
    std::unordered_map<short, PolicyInstruction> policyInstructionMap;

    nlohmann::json to_json() const
    {
        nlohmann::json uePolicyJson = nlohmann::json::object({
            {"plmn", plmn},
            {"instructionOrderMap", nlohmann::json::object()}
        });
        for (const auto& item : instructionOrderMap) {
            uePolicyJson["instructionOrderMap"][std::to_string(item.first)] = item.second;
        }
        nlohmann::json policyInstructionMapJson;
        for (const auto& item : policyInstructionMap) {
            NETMGR_EXT_LOG_I("uePolicyJson policyInstructionMap key = %{public}d", item.first);
            policyInstructionMapJson[std::to_string(item.first)] = item.second.to_json();
        }
        uePolicyJson["policyInstructionMap"] = policyInstructionMapJson;
        return uePolicyJson;
    }

    void from_json(const nlohmann::json& jsonPolicy)
    {
        if (jsonPolicy.find("plmn") != jsonPolicy.end()) {
            const nlohmann::json& plmnJson = jsonPolicy.at("plmn");
            this->plmn.resize(plmnJson.size());
            NETMGR_EXT_LOG_I("UePolicy plmn size = %{public}d", (int)plmnJson.size());
            for (size_t i = 0; i < plmnJson.size(); ++i) {
                NETMGR_EXT_LOG_I("UePolicy plmn[%{public}d] = %{public}d", (int)i, (int)plmnJson[i]);
                this->plmn[i] = plmnJson[i];
            }
        }

        if (jsonPolicy.find("instructionOrderMap") != jsonPolicy.end()) {
            const nlohmann::json& instructionOrderMapJson = jsonPolicy.at("instructionOrderMap");
            this->instructionOrderMap.clear();
            for (const auto& item : instructionOrderMapJson.items()) {
                short key = std::stoi(item.key());
                short value = item.value();
                this->instructionOrderMap[key] = value;
                NETMGR_EXT_LOG_I("UePolicy instructionOrderMap = [%{public}d, %{public}d]",
                    key, value);
            }
        }

        if (jsonPolicy.find("policyInstructionMap") != jsonPolicy.end()) {
            const nlohmann::json& policyInstructionMapJson = jsonPolicy.at("policyInstructionMap");
            this->policyInstructionMap.clear();
            for (const auto& item : policyInstructionMapJson.items()) {
                short key = std::stoi(item.key());
                NETMGR_EXT_LOG_I("UePolicy policyInstructionMap = %{public}d", key);
                PolicyInstruction value;
                value.from_json(item.value());
                this->policyInstructionMap[key] = value;
            }
        }
    }
};

class MultipleBuffer {
public:
    uint8_t segmentNum;
    int totalBufferLen;
    std::unordered_map<uint8_t, std::vector<uint8_t>> bufferSegmentMap;
};

class UePolicyResult {
public:
    short upsc;
    short failedInstructionOrder;
    uint8_t cause;
};

class UePolicyReject {
public:
    std::vector<uint8_t> plmn;
    std::vector<UePolicyResult> uePolicyResult;
};

class UePolicyRejectMsg {
public:
    short totalLen;
    std::vector<UePolicyReject> uePolicyRejects;
};

class NrUnsolicitedMsgParser {
public:
    static NrUnsolicitedMsgParser& GetInstance();
    ~NrUnsolicitedMsgParser() = default;
    void GetAllowedNssaiFromUnsolData(std::vector<uint8_t> buffer);
    void GetUrspFromUnsolData(std::vector<uint8_t> buffer);
    void GetEhplmnFromUnsolData(std::vector<uint8_t> buffer);
    std::string GetHplmn();
    void HandleSimStateChanged();
    void SendUrspUpdate();
private:
    NrUnsolicitedMsgParser();
    bool isAllowedNssaiSync = false;
    std::string mSubscriberId;
    std::unordered_map<std::string, UePolicy> uePolicyMap;
    std::unordered_map<uint8_t, MultipleBuffer> mMultipleBufferMap;
    std::vector<std::string> mEhplmns;
    void GetSubscriberIdAndUrspFromFile();
    void SyncSubscriberId();
    void DecodeUrspFromUnsolData(int& startIndex, std::vector<uint8_t> buffer);
    std::unordered_map<std::string, UePolicy> ReadObjectFromJsonFile(const std::string& fileName);
    void WriteObjectToJsonFile(const std::string& fileName, std::unordered_map<std::string, UePolicy>& obj);
    void UpdateUrspRules();
    void HandleDecodeResult(uint8_t pti, std::unordered_map<std::string, UePolicy>& decodeUePolicyMap);
    bool isUePolicyLegal(std::unordered_map<std::string, UePolicy>& decodeUePolicyMap,
        UePolicyRejectMsg& uePolicyRejectMsg);
    bool isPlmnInHplmn(std::string plmn);
    void SndManageUePolicyComplete(uint8_t pti);
    void SndManageUePolicyCommandReject(uint8_t pti, UePolicyRejectMsg uePolicyRejectMsg);
    void SendUePolicySectionIdentifier();
    void AddNewUePolicy(std::unordered_map<std::string, UePolicy>& decodeUePolicyMap);
    bool DecodePolicySectionList(int inputLen, int& startIndex, std::vector<uint8_t> buffer,
        std::unordered_map<std::string, UePolicy>& decodeUePolicyMap);
    bool DecodePolicySection(int inputLen, int& startIndex, std::vector<uint8_t> buffer,
        std::unordered_map<std::string, UePolicy>& decodeUePolicyMap);
    bool DecodeInstruction(int inputLen, int& startIndex, std::vector<uint8_t> buffer,
        UePolicy& uePolicy, short instructionOrder);
    bool DecodeUePolicyPart(int inputLen, int& startIndex, std::vector<uint8_t> buffer,
        PolicyInstruction& policyInstruction);
    std::string PlmnToString(const std::vector<uint8_t>& plmns);
    void SendImsRsdList();
    void GetNetworkSliceAllowedNssai();
    void GetNetworkSliceEhplmn();
};

} // namespace NetManagerStandard
} // namespace OHOS

#endif  // NRUNSOLICITEDMSGPARSER_H
