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

#include "nrunsolicitedmsgparser.h"
#include "cellular_data_client.h"
#include "core_service_client.h"
#include "networkslicemanager.h"
#include "state_utils.h"

namespace OHOS {
namespace NetManagerStandard {
static constexpr int MODEM_ID = 0;
static constexpr uint8_t MSG_MANAGE_UE_POLICY_COMMAND = 1;
static constexpr uint8_t MSG_MANAGE_UE_POLICY_COMPLETE = 0;
static constexpr uint8_t MSG_MANAGE_UE_POLICY_COMMAND_REJECT = 1;
static constexpr uint8_t MSG_UE_STATE_INDICATION = 2;
static constexpr uint8_t SPACELENGTH = 4;
static constexpr uint8_t NSSAI_TYPE_DEFAULT_CONFIGURED_REJECT_CONFIGURED_ALLOWED_NSSAI = 3;
static constexpr uint8_t UE_POLICY_PART_TYPE_URSP = 1;
static std::string REPORTURSP_PATH = "/system/profile/reportUrsp.json";
static std::string DEFAULT_PLMN = "00101";

NrUnsolicitedMsgParser& NrUnsolicitedMsgParser::GetInstance()
{
    static NrUnsolicitedMsgParser instance;
    return instance;
}

NrUnsolicitedMsgParser::NrUnsolicitedMsgParser()
{
    NETMGR_EXT_LOG_I("init NrUnsolicitedMsgParser start");
    if (sUrspConfig_ == nullptr) {
        NETMGR_EXT_LOG_E("NrUnsolicitedMsgParser sUrspConfig_ = nullptr");
        sUrspConfig_ = std::make_shared<UrspConfig>(UrspConfig::GetInstance());
    }
    GetSubscriberIdAndUrspFromFile();
    NETMGR_EXT_LOG_I("init NrUnsolicitedMsgParser end");
}


void NrUnsolicitedMsgParser::GetSubscriberIdAndUrspFromFile()
{
    NETMGR_EXT_LOG_I("nrunsolicited:Enter getSubscriberIdAndUrspFromFile");
    if (mSubscriberId.empty()) {
        NETMGR_EXT_LOG_E("imsi null");
        return;
    }
    std::unordered_map<std::string, UePolicy> uePolicyMaptmp = ReadObjectFromJsonFile(REPORTURSP_PATH);
    if (uePolicyMaptmp.empty()) {
        NETMGR_EXT_LOG_I("nrunsolicited:read ue policy from file failed");
        WriteObjectToJsonFile(REPORTURSP_PATH, uePolicyMap);
        SendUePolicySectionIdentifier();
        return;
    }
    uePolicyMap = uePolicyMaptmp;
    if (uePolicyMap.empty()) {
        return;
    }
    UpdateUrspRules();
}

void NrUnsolicitedMsgParser::HandleSimStateChanged()
{
    NETMGR_EXT_LOG_E("enter handleSimStateChanged isAllowedNssaiSync = %{public}d", isAllowedNssaiSync);
    SyncSubscriberId();
    if (!isAllowedNssaiSync) {
        GetNetworkSliceAllowedNssai();
    }
    GetNetworkSliceEhplmn();
}

void NrUnsolicitedMsgParser::SyncSubscriberId()
{
    int32_t slotId = StateUtils::GetPrimarySlotId();
    NETMGR_EXT_LOG_I("NrUnsolicitedMsgParser slotId = %{public}d", slotId);
    std::u16string imsi;
    Telephony::CoreServiceClient::GetInstance().GetIMSI(slotId, imsi);
    if (imsi.empty()) {
        NETMGR_EXT_LOG_E("syncSubscriberId fail, imsi null");
        return;
    }
    std::string imsistr = Str16ToStr8(imsi);
    std::string encryptImsi = GetSha256Str(imsistr);
    if (mSubscriberId.empty()) {
        mSubscriberId = encryptImsi;
        return;
    }
    if (mSubscriberId == encryptImsi) {
        return;
    }
    mSubscriberId = encryptImsi;
    uePolicyMap = std::unordered_map<std::string, UePolicy>(NetworkSliceCommConfig::HASH_MAP_DEFAULT_CAPACITY);
    WriteObjectToJsonFile(REPORTURSP_PATH, uePolicyMap);
    SendUePolicySectionIdentifier();
    UpdateUrspRules();
    SendUrspUpdate();
    auto networkSliceManager = DelayedSingleton<NetworkSliceManager>::GetInstance();
    if (networkSliceManager != nullptr) {
        networkSliceManager->onUrspAvailableStateChanged();
    }
}

void NrUnsolicitedMsgParser::GetNetworkSliceAllowedNssai()
{
    NETMGR_EXT_LOG_E("GetNetworkSliceAllowedNssai");
    int32_t slotId = StateUtils::GetPrimarySlotId();
    std::u16string operatorNumeric;
    Telephony::CoreServiceClient::GetInstance().GetSimOperatorNumeric(slotId, operatorNumeric);
    std::string plmn = Str16ToStr8(operatorNumeric);
    if (plmn == "") {
        NETMGR_EXT_LOG_E("GetPlmn fail");
        return;
    }
    NETMGR_EXT_LOG_E("NrUnsolicitedMsgParser GetNetworkSliceAllowedNssai plmn = %{public}s", plmn.c_str());
    std::vector<uint8_t> plmns = ConvertstringTouInt8Vector(plmn);
    std::vector<uint8_t> buffer;
    buffer.push_back(plmns.size());
    for (size_t i = 0; i < plmns.size(); ++i) {
        buffer.push_back(plmns[i]);
    }
    buffer.push_back(NSSAI_TYPE_DEFAULT_CONFIGURED_REJECT_CONFIGURED_ALLOWED_NSSAI);
    Telephony::CellularDataClient::GetInstance().GetNetworkSliceAllowedNssai(MODEM_ID, buffer);
    isAllowedNssaiSync = true;
}

void NrUnsolicitedMsgParser::GetNetworkSliceEhplmn()
{
    NETMGR_EXT_LOG_I("GetNetworkSliceEhplmn");
    Telephony::CellularDataClient::GetInstance().GetNetworkSliceEhplmn(MODEM_ID);
}

void NrUnsolicitedMsgParser::WriteObjectToJsonFile(const std::string& fileName,
    std::unordered_map<std::string, UePolicy>& obj)
{
    if (fileName.empty()) {
        return;
    }
    NETMGR_EXT_LOG_I("WriteObjectToJsonFile:%{public}s", fileName.c_str());
    nlohmann::json j = nlohmann::json::object();
    for (const auto& pair : obj) {
        std::string plmn = pair.first;
        NETMGR_EXT_LOG_I("WriteObjectToJsonFile plmn = %{public}s", plmn.c_str());
        UePolicy uePolicy = pair.second;
        nlohmann::json uePolicyJson = uePolicy.to_json();
        j[plmn] = uePolicyJson;
    }

    std::fstream f;
    f.open(fileName, std::ios::out);
    if (!f.is_open()) {
        NETMGR_EXT_LOG_E("file not opened! ");
    } else {
        f << j.dump(SPACELENGTH);
        f.close();
    }
}

std::unordered_map<std::string, UePolicy> NrUnsolicitedMsgParser::ReadObjectFromJsonFile(const std::string& fileName)
{
    std::ifstream file(fileName);
    if (!file.is_open()) {
        NETMGR_EXT_LOG_E("failed to open file ");
        return {};
    }
    NETMGR_EXT_LOG_I("ReadObjectFromJsonFile");
    nlohmann::json j;
    file >> j;
    if (j.empty()) {
        return {};
    }
    std::unordered_map<std::string, UePolicy> uePolicies;
    for (const auto& item : j.items()) {
        const std::string& plmn = item.key();
        const nlohmann::json& policyJson = item.value();

        UePolicy uePolicy;
        uePolicy.from_json(policyJson);
        uePolicies[plmn] = uePolicy;
    }
    if (file.is_open()) {
        file.close();
    }
    return uePolicies;
}


void NrUnsolicitedMsgParser::UpdateUrspRules()
{
    if (sUrspConfig_ == nullptr) {
        NETMGR_EXT_LOG_E("sUrspConfig == null");
        return;
    }
    NETMGR_EXT_LOG_I("UpdateUrspRules");
    std::string plmn;
    UePolicy uePolicy;
    PolicyInstruction policyInstruction;
    sUrspConfig_->ClearUrspRules();
    for (const auto& entry : uePolicyMap) {
        std::string plmn = entry.first;
        const UePolicy& uePolicy = entry.second;
        std::vector<UrspRule> urspRules;
        for (const auto& instructionEntry : uePolicy.policyInstructionMap) {
            PolicyInstruction policyInstruction = instructionEntry.second;
            for (size_t i = 0; i < policyInstruction.urspRules.size(); ++i) {
                urspRules.push_back(policyInstruction.urspRules[i]);
            }
        }
        sUrspConfig_->setUrspRules(plmn, urspRules);
    }
    sUrspConfig_->DumpUePolicyMap();
    sUrspConfig_->SaveTrafficDescriptorWhiteListToDb();
}

void NrUnsolicitedMsgParser::SendUrspUpdate()
{
    if (sUrspConfig_ == nullptr) {
        NETMGR_EXT_LOG_E("sendUrspUpdate, mUrspConfig == null");
        return;
    }
    std::string plmn = GetHplmn();
    NETMGR_EXT_LOG_I("SendUrspUpdate GetHplmn = %{public}s", plmn.c_str());
    SelectedRouteDescriptor routeRule = sUrspConfig_->GetMatchAllUrspRule(plmn);
    std::map<std::string, std::string> data;
    data["routeBitmap"] = std::to_string(routeRule.getRouteBitmap());
    NETMGR_EXT_LOG_I("SendUrspUpdate routeBitmap = %{public}s", data["routeBitmap"].c_str());
    if ((routeRule.getRouteBitmap() & 0x01) == 0x01) {
        data["sscMode"] = std::to_string(routeRule.getSscMode());
        if (routeRule.getPduSessionType() != -1) {
            data["pduSessionType"] = std::to_string(routeRule.getPduSessionType());
        }
        data["sNssai"] = routeRule.getSnssai();
        data["dnn"] = routeRule.getDnn();
        data["urspPrecedence"] = std::to_string(routeRule.getUrspPrecedence());
    }
    std::shared_ptr<std::map<std::string, std::string>> msg =
        std::make_shared<std::map<std::string, std::string>>(data);
    Singleton<NetworkSliceMsgCenter>::GetInstance().Publish(EVENT_URSP_CHANGED, msg);
}

std::string NrUnsolicitedMsgParser::GetHplmn()
{
    return (mEhplmns.size() > 0) ? mEhplmns[0] : DEFAULT_PLMN;
}

void NrUnsolicitedMsgParser::GetAllowedNssaiFromUnsolData(std::vector<uint8_t> buffer)
{
    sAllowedNssaiConfig_->DecodeAllowedNssai(buffer);
}

void NrUnsolicitedMsgParser::GetEhplmnFromUnsolData(std::vector<uint8_t> buffer)
{
    if (buffer.empty()) {
        NETMGR_EXT_LOG_E("GetEhplmnFromUnsolData failed, invalid buffer");
        return;
    }
    NETMGR_EXT_LOG_I("start GetEhplmnFromUnsolData");
    int inputLen = (int)buffer.size();
    mEhplmns.clear();
    std::string ehplmn = "";
    std::vector<std::string> values;
    int i;
    for (i = 0; i < inputLen; ++i) {
        ehplmn += static_cast<char>(buffer[i]);
    }
    values = Split(ehplmn, ",");
    for (i = 0; i < (int)values.size(); ++i) {
        mEhplmns.push_back(values[i]);
    }
    for (i = 0; i < (int)mEhplmns.size(); ++i) {
        NETMGR_EXT_LOG_I("mEhplmn[%{public}d] = %{public}s", i, mEhplmns[i].c_str());
    }
}

void NrUnsolicitedMsgParser::GetUrspFromUnsolData(std::vector<uint8_t> buffer)
{
    if (buffer.empty()) {
        NETMGR_EXT_LOG_E("getUrspFromUnsolData failed, invalid buffer");
        return;
    }
    if (buffer.size() < NetworkSliceCommConfig::LEN_THREE_BYTE) {
        return;
    }
    int startIndex = 0;
    const uint8_t pti = buffer[startIndex++];
    const uint8_t segmentNum = buffer[startIndex++];
    if (segmentNum < 1) {
        return;
    }
    const uint8_t segmentIndex = buffer[startIndex++];
    if (segmentNum == 1) { /* one segment buffer, directly decode buffer */
        DecodeUrspFromUnsolData(startIndex, buffer);
        return;
    }
    if (mMultipleBufferMap.find(pti) == mMultipleBufferMap.end()) { /* first segment of this pti */
        MultipleBuffer multipleBuffer;
        multipleBuffer.segmentNum = segmentNum;
        multipleBuffer.bufferSegmentMap[segmentIndex] = buffer;
        multipleBuffer.totalBufferLen += (int(buffer.size()) - startIndex);
        mMultipleBufferMap[pti] = multipleBuffer;
    } else {
        MultipleBuffer multipleBuffer = mMultipleBufferMap[pti];
        if (segmentNum != multipleBuffer.segmentNum || segmentIndex > segmentNum || segmentIndex <= 0) {
            NETMGR_EXT_LOG_E("segmentNum or segmentIndex invalid");
            return;
        }
        if (multipleBuffer.bufferSegmentMap.find(segmentIndex) != multipleBuffer.bufferSegmentMap.end()) {
            NETMGR_EXT_LOG_E("segmentIndex already exist");
            return;
        }
        multipleBuffer.bufferSegmentMap[segmentIndex] = buffer;
        multipleBuffer.totalBufferLen += (int(buffer.size()) - startIndex);
        if (multipleBuffer.bufferSegmentMap.size() == segmentNum) { /* all segment is received */
            std::vector<uint8_t> combinationBuffer(multipleBuffer.totalBufferLen);
            for (int index = 1; index <= segmentNum; ++index) {
                std::vector<uint8_t> tmpvec = multipleBuffer.bufferSegmentMap[static_cast<uint8_t>(index)];
                combinationBuffer.insert(combinationBuffer.end(), tmpvec.begin(), tmpvec.end());
            }
            startIndex = 0;
            DecodeUrspFromUnsolData(startIndex, combinationBuffer);
            mMultipleBufferMap.erase(pti);
        }
    }
}

void NrUnsolicitedMsgParser::DecodeUrspFromUnsolData(int& startIndex, std::vector<uint8_t> buffer)
{
    if (sUrspConfig_ == nullptr) {
        NETMGR_EXT_LOG_E("DecodeUrspFromUnsolData, sUrspConfig_ == nullptr");
        return;
    }
    if (buffer.empty()) {
        NETMGR_EXT_LOG_E("getursp failed, invalid buffer");
        return;
    }
    NETMGR_EXT_LOG_I("start decode ursp, buffer.remaining() = [%{public}d]", (int)buffer.size());
    int inputLen = (int(buffer.size()) - startIndex);
    if (inputLen < NetworkSliceCommConfig::LEN_SHORT) {
        NETMGR_EXT_LOG_E("inputLen < NetworkSliceCommConfig::LEN_SHORT");
        return;
    }
    short version = GetShort(startIndex, buffer);
    NETMGR_EXT_LOG_I("ursp version = [%{public}d]", version);
    if (version == NetworkSliceCommConfig::URSP_VERSION_1510
        || version == NetworkSliceCommConfig::URSP_VERSION_1520) {
        sUrspConfig_->SetUrspVersion(version);
    } else {
        NETMGR_EXT_LOG_E("version is invalid = %{public}d", version);
        return;
    }
    inputLen -= NetworkSliceCommConfig::LEN_SHORT;
    if (inputLen < NetworkSliceCommConfig::LEN_INT) {
        /* LEN_INT = PTI + message type + Length of contents */
        NETMGR_EXT_LOG_E("inputLen < NetworkSliceCommConfig::LEN_INT");
        return;
    }
    /* PTI */
    uint8_t pti = buffer[startIndex++];
    inputLen -= NetworkSliceCommConfig::LEN_BYTE;

    /* UE policy delivery service message type */
    if (buffer[startIndex++] != MSG_MANAGE_UE_POLICY_COMMAND) {
        NETMGR_EXT_LOG_E("buffer.get() != MSG_MANAGE_UE_POLICY_COMMAND");
        return;
    }
    inputLen -= NetworkSliceCommConfig::LEN_BYTE;
    std::unordered_map<std::string, UePolicy> decodeUePolicyMap;
    if (!DecodePolicySectionList(inputLen, startIndex, buffer, decodeUePolicyMap)) {
        return;
    }
    NETMGR_EXT_LOG_I("end decode ursp = [%{public}d]", pti);
    HandleDecodeResult(pti, decodeUePolicyMap);
}

void NrUnsolicitedMsgParser::HandleDecodeResult(uint8_t pti,
    std::unordered_map<std::string, UePolicy>& decodeUePolicyMap)
{
    NETMGR_EXT_LOG_I("HandleDecodeResult");
    UePolicyRejectMsg uePolicyRejectMsg;
    if (!isUePolicyLegal(decodeUePolicyMap, uePolicyRejectMsg)) {
        NETMGR_EXT_LOG_I("NrUnsolicitedMsgParser::ue policy illegal");
        SndManageUePolicyCommandReject(pti, uePolicyRejectMsg);
        return;
    }
    SndManageUePolicyComplete(pti);
    AddNewUePolicy(decodeUePolicyMap);
    SendUePolicySectionIdentifier();
    SendImsRsdList();
    WriteObjectToJsonFile(REPORTURSP_PATH, uePolicyMap);
    UpdateUrspRules();
    SendUrspUpdate();
    DelayedSingleton<NetworkSliceManager>::GetInstance()->onUrspAvailableStateChanged();
}

void NrUnsolicitedMsgParser::SndManageUePolicyComplete(uint8_t pti)
{
    NETMGR_EXT_LOG_I("SndManageUePolicyComplete, ModemID:%{public}d, MSGID:%{public}d",
        MODEM_ID, MSG_MANAGE_UE_POLICY_COMPLETE);
    std::vector<uint8_t> buffer;
    buffer.push_back(pti);
    buffer.push_back(MSG_MANAGE_UE_POLICY_COMPLETE);
    Telephony::CellularDataClient::GetInstance().SendUrspDecodeResult(MODEM_ID, buffer);
}

void NrUnsolicitedMsgParser::SndManageUePolicyCommandReject(uint8_t pti,
    UePolicyRejectMsg uePolicyRejectMsg)
{
    NETMGR_EXT_LOG_I("SndManageUePolicyCommandReject");
    std::vector<uint8_t> buffer;
    int32_t startlen = 0;
    /* put pti */
    buffer.push_back(pti);
    startlen++;
    /* put UE policy delivery service message type */
    buffer.push_back(MSG_MANAGE_UE_POLICY_COMMAND_REJECT);
    startlen++;
    /* put UE policy section management result information element */
    PutShort(buffer, uePolicyRejectMsg.totalLen);
    startlen += NetworkSliceCommConfig::LEN_SHORT;
    for (int i = 0; i < (int)uePolicyRejectMsg.uePolicyRejects.size(); ++i) {
        /* put Number of results */
        buffer.push_back(static_cast<uint8_t>(uePolicyRejectMsg.uePolicyRejects.size()));
        startlen++;
        /* put plmn */
        for (int j = 0; j < PLMN_LEN; ++j) {
            buffer.push_back(uePolicyRejectMsg.uePolicyRejects[i].plmn[j]);
            startlen++;
        }

        /* put UE policy section management result contents */
        std::vector<UePolicyResult> uePolicyResults = uePolicyRejectMsg.uePolicyRejects[i].uePolicyResult;
        for (int j = 0; j < (int)uePolicyResults.size(); ++j) {
            PutShort(buffer, uePolicyResults[j].upsc);
            startlen += NetworkSliceCommConfig::LEN_SHORT;
            PutShort(buffer, uePolicyResults[j].failedInstructionOrder);
            startlen += NetworkSliceCommConfig::LEN_SHORT;
            buffer.push_back(uePolicyResults[j].cause);
            startlen++;
        }
    }
    Telephony::CellularDataClient::GetInstance().SendUrspDecodeResult(MODEM_ID, buffer);
}

void NrUnsolicitedMsgParser::SendUePolicySectionIdentifier()
{
    if (sUrspConfig_ == nullptr) {
        NETMGR_EXT_LOG_E("SendUePolicySectionIdentifier sUrspConfig_ == nullptr ");
        return;
    }
    short upsiListLen = 0;
    for (const auto& entry : uePolicyMap) {
        const UePolicy& uePolicy = entry.second;
        upsiListLen = (short)(upsiListLen + NetworkSliceCommConfig::LEN_SHORT + PLMN_LEN
            + NetworkSliceCommConfig::LEN_SHORT * uePolicy.policyInstructionMap.size());
    }
    uint16_t allocateLen = NetworkSliceCommConfig::LEN_BYTE + NetworkSliceCommConfig::LEN_BYTE
        + NetworkSliceCommConfig::LEN_SHORT + upsiListLen
        + NetworkSliceCommConfig::LEN_BYTE + NetworkSliceCommConfig::LEN_BYTE + NetworkSliceCommConfig::LEN_SHORT;
    std::vector<uint8_t> buffer;
    int32_t startlen = 0;
    buffer.push_back(0);     /* put pti, use 0 */
    startlen++;
    buffer.push_back(MSG_UE_STATE_INDICATION); /* put UE policy delivery service message type */
    startlen++;
    PutShort(buffer, upsiListLen);     /* put upsi */
    startlen += NetworkSliceCommConfig::LEN_SHORT;
    for (const auto& entry : uePolicyMap) {
        const UePolicy& uePolicy = entry.second;
        uint8_t upsiSublistLen = (uint8_t)(PLMN_LEN
            + NetworkSliceCommConfig::LEN_SHORT * uePolicy.policyInstructionMap.size());
        PutShort(buffer, upsiSublistLen); /* put upsi sublist len */
        startlen += NetworkSliceCommConfig::LEN_SHORT;
        for (int i = 0; i < PLMN_LEN; ++i) { /* put plmn */
            buffer.push_back(uePolicy.plmn[i]);
            startlen++;
        }
        for (const auto& instructionEntry : uePolicy.policyInstructionMap) { /* put upsc list */
            PutShort(buffer, instructionEntry.first);
            startlen += NetworkSliceCommConfig::LEN_SHORT;
        }
    }
    buffer.push_back(1); /* put Length of Policy information contents */
    startlen++;
    buffer.push_back(0); /* put UE policy classmark, 0 means ANDSP not supported by the UE */
    startlen++;
    PutShort(buffer, sUrspConfig_->GetSuccUrspVersion()); /* put ursp version */
    startlen += NetworkSliceCommConfig::LEN_SHORT;
    Telephony::CellularDataClient::GetInstance().SendUePolicySectionIdentifier(MODEM_ID, buffer);
}

void NrUnsolicitedMsgParser::SendImsRsdList()
{
    if (sUrspConfig_ == nullptr) {
        NETMGR_EXT_LOG_E("SendImsRsdList sUrspConfig_ == nullptr ");
        return;
    }
    if (!sUrspConfig_->mImsRsdsMap.empty()) {
        NETMGR_EXT_LOG_I("GetImsRsdList mImsRsdsMap is not empty");
    }
    std::vector<uint8_t> rsdBytes = sUrspConfig_->GetImsRsdList();
    if (!rsdBytes.empty()) {
        NETMGR_EXT_LOG_I("NrUnsolicitedMsgParser::SendImsRsdList");
        Telephony::CellularDataClient::GetInstance().SendImsRsdList(MODEM_ID, rsdBytes);
    }
}

void NrUnsolicitedMsgParser::AddNewUePolicy(std::unordered_map<std::string, UePolicy>& decodeUePolicyMap)
{
    NETMGR_EXT_LOG_I("AddNewUePolicy");
    /*
    * Upon receipt of the MANAGE UE POLICY COMMAND message,
    * for each instruction included in the UE policy section management list IE,
    * the UE shall:
    * a)store the received UE policy section of the instruction,
    * if the UE has no stored UE policy section associated with the same UPSI
    * as the UPSI associated with the instruction;
    * b)replace the stored UE policy section with the received UE policy section of the instruction,
    * if the UE has a stored UE policy section associated with the same UPSI as the UPSI associated
    * with the instruction; or
    * c)delete the stored UE policy section, if the UE has a stored UE policy section associated with
    * the same UPSI as the UPSI associated with the instruction and the UE policy section contents of
    * the instruction is empty.
    */
    for (const auto& entry : decodeUePolicyMap) {
        std::string plmn = entry.first;
        NETMGR_EXT_LOG_I("AddNewUePolicy plmn = %{public}s", plmn.c_str());
        UePolicy uePolicy = entry.second;
        if (uePolicyMap.find(plmn) == uePolicyMap.end()) {
            uePolicyMap[plmn] = uePolicy;
            continue;
        }

        std::unordered_map<short, PolicyInstruction>& oldUePolicy = uePolicyMap[plmn].policyInstructionMap;

        for (const auto& instructionEntry : uePolicy.policyInstructionMap) {
            short upsc = instructionEntry.first;
            if (oldUePolicy.find(upsc) == oldUePolicy.end()) {
                oldUePolicy[upsc] = instructionEntry.second;
                continue;
            }

            if (!instructionEntry.second.hasUrspType) {
                oldUePolicy.erase(upsc);
                continue;
            }

            oldUePolicy.erase(upsc);
            oldUePolicy[upsc] = instructionEntry.second;
        }

        if (oldUePolicy.empty()) {
            uePolicyMap.erase(plmn);
        }
    }
}

bool NrUnsolicitedMsgParser::isUePolicyLegal(std::unordered_map<std::string, UePolicy>& decodeUePolicyMap,
    UePolicyRejectMsg& uePolicyRejectMsg)
{
    for (const auto& entry : decodeUePolicyMap) {
        const std::string& plmn = entry.first;
        if (isPlmnInHplmn(plmn)) {
            continue;
        }
        const UePolicy& uePolicy = entry.second;
        UePolicyReject uePolicyReject;
        std::copy(uePolicy.plmn.begin(), uePolicy.plmn.begin() + PLMN_LEN, std::back_inserter(uePolicyReject.plmn));
        for (const auto& instructionEntry : uePolicy.instructionOrderMap) {
            UePolicyResult uePolicyResult;
            uePolicyResult.upsc = instructionEntry.second;
            uePolicyResult.failedInstructionOrder = instructionEntry.first;
            uePolicyResult.cause = CAUSE_PROTOCOL_ERROR;
            uePolicyReject.uePolicyResult.push_back(uePolicyResult);
            uePolicyRejectMsg.totalLen += RESULT_LEN;
        }
        uePolicyRejectMsg.uePolicyRejects.push_back(uePolicyReject);
        uePolicyRejectMsg.totalLen += NetworkSliceCommConfig::LEN_BYTE + PLMN_LEN;
    }
    if (!uePolicyRejectMsg.uePolicyRejects.empty()) {
        return false;
    }
    return true;
}

bool NrUnsolicitedMsgParser::isPlmnInHplmn(std::string plmn)
{
    if (mEhplmns.empty()) {
        mEhplmns.push_back(DEFAULT_PLMN);
    }
    if (mEhplmns.empty()) {
        NETMGR_EXT_LOG_I("mEhplmns == null");
        return false;
    }
    NETMGR_EXT_LOG_I("mEhplmns.size() = %{public}d", (int)mEhplmns.size());
    if (mEhplmns.size() > 0 && plmn == mEhplmns[0]) {
        return true;
    }
    return false;
}

bool NrUnsolicitedMsgParser::DecodePolicySectionList(int inputLen, int& startIndex, std::vector<uint8_t> buffer,
    std::unordered_map<std::string, UePolicy>& decodeUePolicyMap)
{
    NETMGR_EXT_LOG_I("DecodePolicySectionList");
    int len = inputLen;
    short subLen_short = GetShort(startIndex, buffer);
    len -= NetworkSliceCommConfig::LEN_SHORT;
    if (ConvertUnsignedShort2Int(subLen_short) != len) {
        NETMGR_EXT_LOG_E("buffer.getShort() != len, %{public}d != %{public}d",
            ConvertUnsignedShort2Int(subLen_short), len);
        return false;
    }
    while (len > 0) {
        if ((int(buffer.size()) - startIndex) < NetworkSliceCommConfig::LEN_SHORT) {
            NETMGR_EXT_LOG_E("buffer.remaining() is not enough when get sublist length");
            return false;
        }
        subLen_short = GetShort(startIndex, buffer);
        int subLen = ConvertUnsignedShort2Int(subLen_short);
        if (((int(buffer.size()) - startIndex) < subLen) ||
            (len < (NetworkSliceCommConfig::LEN_SHORT + subLen))) {
            NETMGR_EXT_LOG_I("size = %{public}d, subLen = %{public}d, Len = %{public}d",
                (int(buffer.size()) - startIndex), subLen, len);
            NETMGR_EXT_LOG_E("buffer.remaining() or len is not enough when decode policy section list");
            return false;
        }
        len = len - NetworkSliceCommConfig::LEN_SHORT - subLen;
        if (!DecodePolicySection(subLen, startIndex, buffer, decodeUePolicyMap)) {
            NETMGR_EXT_LOG_E("decodePolicySection(subLen, buffer) is false");
            return false;
        }
    }
    return true;
}

bool NrUnsolicitedMsgParser::DecodePolicySection(int inputLen, int& startIndex, std::vector<uint8_t> buffer,
    std::unordered_map<std::string, UePolicy>& decodeUePolicyMap)
{
    std::vector<uint8_t> plmns;
    int len = inputLen;
    if (len < NetworkSliceCommConfig::LEN_THREE_BYTE) {     /* plmn */
        NETMGR_EXT_LOG_E("len < NetworkSliceCommConfig::LEN_THREE_BYTE");
        return false;
    }
    plmns.push_back(buffer[startIndex]);
    plmns.push_back(buffer[startIndex + ARRAY_INDEX_1]);
    plmns.push_back(buffer[startIndex + ARRAY_INDEX_2]);
    startIndex += NetworkSliceCommConfig::LEN_THREE_BYTE;
    len -= NetworkSliceCommConfig::LEN_THREE_BYTE;
    std::string plmn = PlmnToString(plmns);
    NETMGR_EXT_LOG_I("DecodePolicySection::PLMN = %{public}s", plmn.c_str());
    if (plmn == "") {
        return false;
    }
    UePolicy uePolicy;
    if (decodeUePolicyMap.find(plmn) != decodeUePolicyMap.end()) {
        uePolicy = decodeUePolicyMap[plmn];
    } else {
        uePolicy.plmn = plmns;
    }
    short instructionOrder = 0;
    while (len > 0) {
        if ((int(buffer.size()) - startIndex) < NetworkSliceCommConfig::LEN_SHORT) {
            NETMGR_EXT_LOG_E("buffer.remaining() < NetworkSliceCommConfig::LEN_SHORT");
            return false;
        }
        short subLen_short = GetShort(startIndex, buffer);
        int subLen = ConvertUnsignedShort2Int(subLen_short);
        if ((int(buffer.size()) - startIndex) < subLen || (len < (NetworkSliceCommConfig::LEN_SHORT + subLen))) {
            NETMGR_EXT_LOG_E("(buffer.remaining() < subLen) || (len < (NetworkSliceCommConfig::LEN_SHORT + subLen))");
            return false;
        }
        len = len - NetworkSliceCommConfig::LEN_SHORT - subLen;
        instructionOrder++;
        if (!DecodeInstruction(subLen, startIndex, buffer, uePolicy, instructionOrder)) {
            NETMGR_EXT_LOG_E("decodeInstruction(subLen, buffer, uePolicy) is false");
            return false;
        }
    }
    if (decodeUePolicyMap.find(plmn) == decodeUePolicyMap.end()) {
        decodeUePolicyMap[plmn] = uePolicy;
    }
    if (uePolicy.policyInstructionMap.empty()) {
        decodeUePolicyMap.erase(plmn);
    }
    return true;
}

bool NrUnsolicitedMsgParser::DecodeInstruction(int inputLen, int& startIndex, std::vector<uint8_t> buffer,
    UePolicy& uePolicy, short instructionOrder)
{
    NETMGR_EXT_LOG_I("DecodeInstruction");
    PolicyInstruction policyInstruction;
    int len = inputLen;

    /* upsc */
    if (len < NetworkSliceCommConfig::LEN_SHORT) {
        NETMGR_EXT_LOG_E("len < NetworkSliceCommConfig::LEN_SHORT");
        return false;
    }
    short upsc = GetShort(startIndex, buffer);
    len -= NetworkSliceCommConfig::LEN_SHORT;
    NETMGR_EXT_LOG_E("upsc = %{public}d, len = %{public}d", upsc, len);
    if (uePolicy.policyInstructionMap.find(upsc) != uePolicy.policyInstructionMap.end()) {
        policyInstruction = uePolicy.policyInstructionMap[upsc];
        policyInstruction.hasUrspType = false;
        policyInstruction.urspRules.clear();
    } else {
        PolicyInstruction newPolicyInstruction;
        newPolicyInstruction.hasUrspType = false;
        uePolicy.policyInstructionMap[upsc] = newPolicyInstruction;
    }

    while (len > 0) {
        /* UE policy part contents length */
        if ((int(buffer.size()) - startIndex) < NetworkSliceCommConfig::LEN_SHORT) {
            NETMGR_EXT_LOG_E("buffer.size() < NetworkSliceCommConfig::LEN_SHORT");
            return false;
        }
        short subLen_short = GetShort(startIndex, buffer);
        int subLen = ConvertUnsignedShort2Int(subLen_short);
        if (((int(buffer.size()) - startIndex) < subLen) || (len < (NetworkSliceCommConfig::LEN_SHORT + subLen))) {
            NETMGR_EXT_LOG_I("size = %{public}d, subLen = %{public}d, Len = %{public}d",
                (int(buffer.size()) - startIndex), subLen, len);
            NETMGR_EXT_LOG_E("(buffer.size() < subLen) || (len < (NetworkSliceCommConfig::LEN_SHORT + subLen))");
            return false;
        }
        len = len - NetworkSliceCommConfig::LEN_SHORT - subLen;
        if (!DecodeUePolicyPart(subLen, startIndex, buffer, policyInstruction)) {
            NETMGR_EXT_LOG_E("decodeUePolicyPart(subLen, buffer, policyInstruction.urspRules) is false");
            return false;
        }
    }

    uePolicy.policyInstructionMap[upsc] = policyInstruction;
    if (policyInstruction.hasUrspType) {
        uePolicy.instructionOrderMap[instructionOrder] = upsc;
    }
    return true;
}

bool NrUnsolicitedMsgParser::DecodeUePolicyPart(int inputLen, int& startIndex,
    std::vector<uint8_t> buffer, PolicyInstruction& policyInstruction)
{
    NETMGR_EXT_LOG_I("DecodeUePolicyPart");
    uint8_t uePolicyType;
    std::vector<uint8_t> dsts;
    int len = inputLen;
    /* ue policy part type */
    if (len < NetworkSliceCommConfig::LEN_BYTE) {
        NETMGR_EXT_LOG_E("len < NetworkSliceCommConfig::LEN_BYTE");
        return false;
    }
    /* use low 4 bits for ue policy type, high 4 bits reserved */
    uePolicyType = buffer[startIndex] & 0x0F;
    startIndex += NetworkSliceCommConfig::LEN_BYTE;
    len -= NetworkSliceCommConfig::LEN_BYTE;
    if (uePolicyType != UE_POLICY_PART_TYPE_URSP) {
        NETMGR_EXT_LOG_I("uePolicyType != UE_POLICY_PART_TYPE_URSP, %{public}d != %{public}d",
            uePolicyType, UE_POLICY_PART_TYPE_URSP);
        /* not ursp type */
        dsts = std::vector<uint8_t>(len);
        for (int i = 0; i < len; ++i) {
            dsts[i] = buffer[startIndex];
            startIndex++;
        }
        return true;
    }
    policyInstruction.hasUrspType = true;
    if (sUrspConfig_ == nullptr) {
        NETMGR_EXT_LOG_E("mUrspConfig == null");
        return false;
    }
    if (!sUrspConfig_->DecodeUrspRules(len, startIndex, buffer, policyInstruction.urspRules)) {
        NETMGR_EXT_LOG_E("mUrspConfig->decodeUrspRules(subLen, buffer, urspRules) is false");
        return false;
    }
    return true;
}

std::string NrUnsolicitedMsgParser::PlmnToString(const std::vector<uint8_t>& plmns)
{
    std::string plmn;
    std::string firstByte = ByteToHexStr(plmns[ARRAY_INDEX_0] & CONVERT_INT_AND_BYTE);
    plmn += std::to_string(std::stoi(firstByte) % RADIX);
    plmn += std::to_string(std::stoi(firstByte) / RADIX);
    std::string secondByte = ByteToHexStr(plmns[ARRAY_INDEX_1] & CONVERT_INT_AND_BYTE);
    bool isThreeDigitsForMnc = true;
    if (secondByte.find("f") == 0) {
        secondByte = "0" + secondByte.substr(1);
        isThreeDigitsForMnc = false;
    }
    plmn += std::to_string(std::stoi(secondByte) % RADIX);
    std::string thirdByte = ByteToHexStr(plmns[ARRAY_INDEX_2] & CONVERT_INT_AND_BYTE);
    plmn += std::to_string(std::stoi(thirdByte) % RADIX);
    plmn += std::to_string(std::stoi(thirdByte) / RADIX);
    if (isThreeDigitsForMnc) {
        plmn += std::to_string(std::stoi(secondByte) / RADIX);
    }
    NETMGR_EXT_LOG_I("NrUnsolicitedMsgParser::PlmnToString is %{public}s", plmn.c_str());
    return plmn;
}

} // namespace NetManagerStandard
} // namespace OHOS
