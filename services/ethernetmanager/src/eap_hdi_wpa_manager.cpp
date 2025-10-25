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
 
#include "eap_hdi_wpa_manager.h"
 
#include <filesystem>
#include <map>
#include <charconv>
#include <codecvt>
#include <fstream>
 
#include "base64_utils.h"
#include "net_eap_handler.h"
#include "netmgr_ext_log_wrapper.h"
#include "netmanager_base_common_utils.h"
#include "net_manager_constants.h"
 
namespace OHOS {
namespace NetManagerStandard {
 
static constexpr const char* ETHERNET_SERVICE_NAME = "ethernet_service";
static constexpr const char* ITEM_WPA_CTRL = "wpa_ctrl_";
static constexpr const char* ETH_CONFIG_ROOR_DIR = "/data/service/el1/public/eth";
static constexpr const char* ETH_WPA_CONFIG_PATH = "/data/service/el1/public/eth/eth_wpa_supplicant.conf";
 
static constexpr const char* ITEM_CTRL_IFACE = "ctrl_interface=/data/service/el1/public/eth\n";
static constexpr const char* ITEM_LINE = "\n";
static constexpr const char* ITEM_AP_SCAN = "ap_scan=0\n";
static constexpr const char* ITEM_NETWORK_START = "network={\n";
static constexpr const char* ITEM_NETWORK_END = "}\n";
static constexpr const char* ITEM_KEYMGMT = "key_mgmt=IEEE8021X\n";
static constexpr const char* ITEM_EAP = "eap=";
static constexpr const char* ITEM_PHASE2 = "phase2=";
static constexpr const char* ITEM_IDENTITY = "identity=";
static constexpr const char* ITEM_PASSWORD = "password=";
static constexpr const char* ITEM_CA_CERT = "ca_cert=";
static constexpr const char* ITEM_CLIENT_CERT = "client_cert=";
static constexpr const char* ITEM_PRIVATE_KEY = "private_key=";
static const std::string ITEM_QUOTE = "\"";
 
static constexpr int8_t IDX_0 = 1;
static constexpr int8_t IDX_1 = 2;
static constexpr int8_t IDX_2 = 3;
static constexpr int8_t IDX_3 = 4;
static constexpr int8_t IDX_4 = 5;
static constexpr int8_t BASE_10 = 10;
static constexpr int8_t WPA_EVENT_REPORT_PARAM_CNT = 6;
 
static std::map<Phase2Method, std::string> PHASE2_METHOD_STR_MAP = {
    { Phase2Method::PHASE2_NONE, "NONE" },
    { Phase2Method::PHASE2_PAP, "PAP" },
    { Phase2Method::PHASE2_MSCHAP, "MSCHAP" },
    { Phase2Method::PHASE2_MSCHAPV2, "MSCHAPV2" },
    { Phase2Method::PHASE2_GTC, "GTC" },
    { Phase2Method::PHASE2_SIM, "SIM" },
    { Phase2Method::PHASE2_AKA, "AKA" },
    { Phase2Method::PHASE2_AKA_PRIME, "AKA" },
};
static std::map<EapMethod, std::string> EAP_METHOD_STR_MAP = {
    { EapMethod::EAP_NONE, "NONE" },
    { EapMethod::EAP_PEAP, "PEAP" },
    { EapMethod::EAP_TLS, "TLS" },
    { EapMethod::EAP_TTLS, "TTLS" },
    { EapMethod::EAP_PWD, "PWD" },
    { EapMethod::EAP_SIM, "SIM" },
    { EapMethod::EAP_AKA, "AKA" },
    { EapMethod::EAP_AKA_PRIME, "AKA'" },
    { EapMethod::EAP_UNAUTH_TLS, "AKA" },
};
 
EapHdiWpaManager::EapHdiWpaManager()
{
    memset_s(&ethCallback_, sizeof(ethCallback_), 0, sizeof(ethCallback_));
}
 
int32_t EapHdiWpaManager::LoadEthernetHdiService()
{
    std::lock_guard<std::mutex> lock(wpaMutex_);
    if (iEthernet_ != nullptr && devMgr_ != nullptr) {
        NETMGR_EXT_LOG_I("EthService already load");
        return EAP_ERRCODE_SUCCESS;
    }
    devMgr_ = HDIDeviceManagerGet();
    if (devMgr_ == nullptr) {
        NETMGR_EXT_LOG_E("EthService devMgr_ null");
        return EAP_ERRCODE_INTERNAL_ERROR;
    }
    int32_t loadDevRet = devMgr_->LoadDevice(devMgr_, ETHERNET_SERVICE_NAME);
    if ((loadDevRet != HDF_SUCCESS)) {
        HDIDeviceManagerRelease(devMgr_);
        devMgr_ = nullptr;
        NETMGR_EXT_LOG_E("EthService load fail");
        return EAP_ERRCODE_INTERNAL_ERROR;
    }
    iEthernet_ = IEthernetGetInstance(ETHERNET_SERVICE_NAME, false);
    if (iEthernet_ == nullptr) {
        NETMGR_EXT_LOG_E("EthService iEthernet_ null");
        UnloadDeviceManager();
        return EAP_ERRCODE_INTERNAL_ERROR;
    }
    NETMGR_EXT_LOG_I("EthService succ");
    return EAP_ERRCODE_SUCCESS;
}
 
int32_t EapHdiWpaManager::StartEap(const std::string& ifName, const EthEapProfile& profile)
{
    std::lock_guard<std::mutex> lock(wpaMutex_);
    if (iEthernet_ == nullptr) {
        NETMGR_EXT_LOG_E("StartEap iEthernet_ null");
        return EAP_ERRCODE_INTERNAL_ERROR;
    }
    RemoveHistoryCtrl();
    SetEapConfig(profile, ifName);
    RegisterEapEventCallback(ifName);
    int32_t ret = iEthernet_->StartEap(iEthernet_, ifName.c_str());
    if (ret != HDF_SUCCESS) {
        NETMGR_EXT_LOG_E("StartEap fail %{public}d", ret);
        IEthernetReleaseInstance(ETHERNET_SERVICE_NAME, iEthernet_, false);
        iEthernet_ = nullptr;
        UnloadDeviceManager();
        return EAP_ERRCODE_INTERNAL_ERROR;
    }
    NETMGR_EXT_LOG_I("StartEap succ");
    return EAP_ERRCODE_SUCCESS;
}
 
int32_t EapHdiWpaManager::StopEap(const std::string& ifName)
{
    std::lock_guard<std::mutex> lock(wpaMutex_);
    if (iEthernet_ == nullptr) {
        NETMGR_EXT_LOG_E("StopEap iEthernet_ null");
        return EAP_ERRCODE_INTERNAL_ERROR;
    }
    UnregisterEapEventCallback(ifName);
    int32_t ret = iEthernet_->StopEap(iEthernet_, ifName.c_str());
    NETMGR_EXT_LOG_I("StopEap ret %{public}d", ret);
    IEthernetReleaseInstance(ETHERNET_SERVICE_NAME, iEthernet_, false);
    iEthernet_ = nullptr;
    if (devMgr_ != nullptr) {
        devMgr_->UnloadDevice(devMgr_, ETHERNET_SERVICE_NAME);
        HDIDeviceManagerRelease(devMgr_);
        devMgr_ = nullptr;
    }
    NETMGR_EXT_LOG_I("StopEap succ");
    return (ret == HDF_SUCCESS) ? EAP_ERRCODE_SUCCESS : EAP_ERRCODE_LOGOFF_FAIL;
}
 
void EapHdiWpaManager::RemoveHistoryCtrl()
{
    std::filesystem::path filePath(ETH_CONFIG_ROOR_DIR);
    if (!std::filesystem::exists(filePath)) {
        return;
    }
    auto truePath = std::filesystem::canonical(filePath);
    for (const auto& entry : std::filesystem::directory_iterator(truePath)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();
            if (filename.find(ITEM_WPA_CTRL) != std::string::npos) {
                std::filesystem::remove(entry.path());
            }
        }
    }
}
 
int32_t EapHdiWpaManager::SetEapConfig(const EthEapProfile& config, const std::string& ifName)
{
    std::string fileContext;
    fileContext.append(ITEM_CTRL_IFACE);
    fileContext.append(ITEM_AP_SCAN);
    fileContext.append(ITEM_NETWORK_START);
    fileContext.append(ITEM_KEYMGMT);
    fileContext.append(ITEM_EAP + EAP_METHOD_STR_MAP[config.eapMethod] + ITEM_LINE);
    fileContext.append(ITEM_IDENTITY + ITEM_QUOTE + config.identity + ITEM_QUOTE + ITEM_LINE);
    switch (config.eapMethod) {
        case EapMethod::EAP_PEAP:
        case EapMethod::EAP_TTLS:
            fileContext.append(ITEM_PASSWORD + ITEM_QUOTE + config.password + ITEM_QUOTE + ITEM_LINE);
            fileContext.append(ITEM_CA_CERT + ITEM_QUOTE + config.caPath + ITEM_QUOTE + ITEM_LINE);
            fileContext.append(ITEM_PHASE2 + ITEM_QUOTE + Phase2MethodToStr(config.eapMethod, config.phase2Method)
                + ITEM_QUOTE + ITEM_LINE);
            break;
        case EapMethod::EAP_TLS:
            fileContext.append(ITEM_PASSWORD + ITEM_QUOTE + config.password + ITEM_QUOTE + ITEM_LINE);
            fileContext.append(ITEM_CA_CERT + ITEM_QUOTE + config.caPath + ITEM_QUOTE + ITEM_LINE);
            fileContext.append(ITEM_CLIENT_CERT + ITEM_QUOTE + config.clientCertAliases + ITEM_QUOTE + ITEM_LINE);
            fileContext.append(ITEM_PRIVATE_KEY + ITEM_QUOTE + config.certPassword + ITEM_QUOTE + ITEM_LINE);
            break;
        case EapMethod::EAP_PWD:
            fileContext.append(ITEM_PASSWORD + ITEM_QUOTE + config.password + ITEM_QUOTE + ITEM_LINE);
            break;
        default:
            break;
    }
    fileContext.append(ITEM_NETWORK_END);
    if (!WriteEapConfigToFile(fileContext)) {
        NETMGR_EXT_LOG_E("SetEapConfig fail");
        return EAP_ERRCODE_INTERNAL_ERROR;
    }
    NETMGR_EXT_LOG_I("SetEapConfig succ");
    return EAP_ERRCODE_SUCCESS;
}
 
int32_t EapHdiWpaManager::EapShellCmd(const std::string& ifName, const std::string& cmd)
{
    std::lock_guard<std::mutex> lock(wpaMutex_);
    if (iEthernet_ == nullptr) {
        NETMGR_EXT_LOG_E("EapShellCmd iEthernet_ null");
        return EAP_ERRCODE_INTERNAL_ERROR;
    }
    int32_t ret = iEthernet_->EapShellCmd(iEthernet_, ifName.c_str(), cmd.c_str());
    NETMGR_EXT_LOG_I("EthShellCmd cmd = %{public}s res = %{public}d", cmd.c_str(), ret);
    return ret;
}
 
int32_t EapHdiWpaManager::RegisterEapEventCallback(const std::string& ifName)
{
    if (iEthernet_ == nullptr) {
        NETMGR_EXT_LOG_E("RegisterEapEvent iEthernet_ null");
        return EAP_ERRCODE_INTERNAL_ERROR;
    }
    ethCallback_.OnEapEventNotify = OnEapEventReport;
    return iEthernet_->RegisterEapEventCallback(iEthernet_, &ethCallback_, ifName.c_str());
}
 
int32_t EapHdiWpaManager::UnregisterEapEventCallback(const std::string& ifName)
{
    if (iEthernet_ == nullptr) {
        NETMGR_EXT_LOG_E("UnregisterEapEvent iEthernet_ null");
        return EAP_ERRCODE_INTERNAL_ERROR;
    }
    ethCallback_.OnEapEventNotify = OnEapEventReport;
    return iEthernet_->UnregisterEapEventCallback(iEthernet_, &ethCallback_, ifName.c_str());
}
 
std::string EapHdiWpaManager::Phase2MethodToStr(EapMethod eap, Phase2Method method)
{
    std::string prefix = (eap == EapMethod::EAP_TTLS && method == Phase2Method::PHASE2_GTC) ? "autheap=" : "auth=";
    return prefix + PHASE2_METHOD_STR_MAP[method];
}
 
bool EapHdiWpaManager::WriteEapConfigToFile(const std::string &fileContext)
{
    std::string destPath = ETH_WPA_CONFIG_PATH;
    std::ofstream file;
    file.open(destPath, std::ios::out);
    if (!file.is_open()) {
        NETMGR_EXT_LOG_E("WriteEapConfig fail");
        return false;
    }
    file << fileContext << std::endl;
    file.close();
    return true;
}
 
int32_t EapHdiWpaManager::OnEapEventReport(IEthernetCallback *self, const char *ifName, const char *value)
{
    /* @param value -> msgType:msgId:eapCode:eapType:bufferLen:eapBuffer */
    NETMGR_EXT_LOG_I("OnEapEventReport ifName = %{public}s", ifName);
    std::vector<std::string> vecEapDatas = CommonUtils::Split(value, ":");
    if (vecEapDatas.size() != WPA_EVENT_REPORT_PARAM_CNT) {
        NETMGR_EXT_LOG_E("OnEapEventReport value size err %{public}d", static_cast<int32_t>(vecEapDatas.size()));
        return EAP_ERRCODE_INTERNAL_ERROR;
    }
    sptr<EapData> notifyEapData = (std::make_unique<EapData>()).release();
    ConvertStrToInt(vecEapDatas[IDX_0], notifyEapData->msgId);
    int32_t eapValue;
    ConvertStrToInt(vecEapDatas[IDX_1], eapValue);
    notifyEapData->eapCode = static_cast<uint32_t>(eapValue);
    ConvertStrToInt(vecEapDatas[IDX_2], eapValue);
    notifyEapData->eapType = static_cast<uint32_t>(eapValue);
    ConvertStrToInt(vecEapDatas[IDX_3], notifyEapData->bufferLen);
    std::string decodeEapBuf = Base64::Decode(vecEapDatas[IDX_4]);
    notifyEapData->eapBuffer.assign(decodeEapBuf.begin(), decodeEapBuf.end());
    return NetEapHandler::GetInstance().NotifyWpaEapInterceptInfo(NetType::ETH0, notifyEapData);
}
 
bool EapHdiWpaManager::ConvertStrToInt(const std::string &str, int32_t &value)
{
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value, BASE_10);
    return ec == std::errc{} && ptr == str.data() + str.size();
}
 
int32_t EapHdiWpaManager::RegisterCustomEapCallback(const std::string &ifName, const std::string &regCmd)
{
    /*  @param regCmd -> netType:size:composeParam1:composeParam2...((eapCode << 8) | eapType) */
    NETMGR_EXT_LOG_I("RegisterEapCallback regCmd = %{public}s", regCmd.c_str());
    std::lock_guard<std::mutex> lock(wpaMutex_);
    if (iEthernet_ == nullptr) {
        NETMGR_EXT_LOG_E("RegisterEapCallback iEthernet_ null");
        return EAP_ERRCODE_INTERNAL_ERROR;
    }
    std::string cmd = "EXT_AUTH_REG " + regCmd;
    int32_t result = iEthernet_->EapShellCmd(iEthernet_, ifName.c_str(), cmd.c_str());
    if (result != HDF_SUCCESS) {
        NETMGR_EXT_LOG_E("EapShellCmd fail %{public}d", result);
        return EAP_ERRCODE_INTERNAL_ERROR;
    }
    return EAP_ERRCODE_SUCCESS;
}
 
int32_t EapHdiWpaManager::ReplyCustomEapData(const std::string &ifName, int32_t result,
    const sptr<EapData> &eapData)
{
    NETMGR_EXT_LOG_I("ReplyEapData ifName = %{public}s res = %{public}d", ifName.c_str(), result);
    std::lock_guard<std::mutex> lock(wpaMutex_);
    if (iEthernet_ == nullptr) {
        NETMGR_EXT_LOG_E("ReplyEapData iEthernet_ null");
        return EAP_ERRCODE_INTERNAL_ERROR;
    }
    std::string replyCmd = "EXT_AUTH_DATA " + std::to_string(result) + ":";
    replyCmd.append(std::to_string(eapData->msgId) + ":");
    replyCmd.append(std::to_string(eapData->bufferLen) + ":");
    std::string encodeEapBuf = Base64::Encode(std::string(eapData->eapBuffer.begin(), eapData->eapBuffer.end()));
    replyCmd.append(encodeEapBuf);
    NETMGR_EXT_LOG_I("ReplyEapData cmd = %{public}s", replyCmd.c_str());
    int32_t ret = iEthernet_->EapShellCmd(iEthernet_, ifName.c_str(), replyCmd.c_str());
    if (ret != HDF_SUCCESS) {
        NETMGR_EXT_LOG_E("ReplyEapData fail %{public}d", ret);
        return EAP_ERRCODE_INTERNAL_ERROR;
    }
    return EAP_ERRCODE_SUCCESS;
}
 
void EapHdiWpaManager::UnloadDeviceManager()
{
    if (devMgr_ != nullptr) {
        devMgr_->UnloadDevice(devMgr_, ETHERNET_SERVICE_NAME);
        HDIDeviceManagerRelease(devMgr_);
        devMgr_ = nullptr;
    }
}
 
} // namespace NetManagerStandard
}  // namespace OHOS
