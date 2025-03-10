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
#include <arpa/inet.h>
#include <mutex>
#include <condition_variable>
#include "networkslice_kernel_proxy.h"
#include "networksliceutil.h"
#include "state_utils.h"
#include "net_conn_client.h"
#include "hwnetworkslicemanager.h"
#include "broadcast_proxy.h"
#include "core_service_client.h"
#include "networkslicemanager.h"

namespace OHOS {
namespace NetManagerStandard {
const std::string NETMANAGER_EXT_NETWORKSLICE_ABILITY = "persist.netmgr_ext.networkslice";
const std::string COMMA_SEPARATOR = ",";
constexpr uint8_t COLLECT_MODEM_DATA = 0x00;
constexpr int BASE_16 = 16;
constexpr int APPID_OSID_SIZE = 2;
constexpr int IPV4ADDRTOTALLEN = 15;
constexpr int IPV6ADDRTOTALLEN = 27;
constexpr short IP_PARA_REPORT_CONTROL_MSG = 9;
constexpr short KERNEL_BIND_UID_MSG = 15;
constexpr short KERNEL_DEL_UID_BIND_MSG = 16;
constexpr int DEL_BIND_ALL = 0;
constexpr int DEL_BIND_NETID = 1;
constexpr int DEL_BIND_PRECEDENCE = 2;
constexpr int64_t NETWORK_SLICE_PARA_FORBIDDEN_MS = 12 * 60 * 1000L;
constexpr int NETWORK_ACTIVATE_RESULT_SUCCESS = 0;
constexpr int NETWORK_ACTIVATE_RESULT_NORMAL_FAIL = 1;
constexpr int32_t WIFICONNECTED = 4;
constexpr int32_t WIFIDISCONNECTED = 6;
constexpr int SIZE_TWO = 2;
std::shared_ptr<broadcast_proxy> g_broadcastProxy = nullptr;
std::shared_ptr<NrUnsolicitedMsgParser> sNrUnsolicitedMsgParser_ = nullptr;
std::shared_ptr<UrspConfig> sUrspConfig_ = nullptr;

NetworkSliceManager::NetworkSliceManager() : NetworkSliceServiceBase(MODULE_NETWORKSLICE)
{}

NetworkSliceManager::~NetworkSliceManager()
{}

void NetworkSliceManager::OnInit()
{
    NETMGR_EXT_LOG_I("NetworkSliceManager::OnInit.");
    if (!isNrSlicesSupported()) {
        NETMGR_EXT_LOG_I("NetworkSliceManager Not support Nrslice.");
        return;
    }
    InitUePolicy();
    DelayedSingleton<NetworkSliceKernelProxy>::GetInstance()->StartNetlink();
    DelayedSingleton<NetworkSliceKernelProxy>::GetInstance()->StartRecvThread();
    Subscribe(EVENT_HANDLE_ALLOWED_NSSAI);
    Subscribe(EVENT_HANDLE_UE_POLICY);
    Subscribe(EVENT_INIT_UE_POLICY);
    Subscribe(EVENT_HANDLE_SIM_STATE_CHANGED);
    Subscribe(EVENT_HANDLE_EHPLMN);
    Subscribe(EVENT_BIND_TO_NETWORK);
    Subscribe(EVENT_DEL_BIND_TO_NETWORK);
    Subscribe(EVENT_FOREGROUND_APP_CHANGED);
    Subscribe(EVENT_AIR_MODE_CHANGED);
    Subscribe(EVENT_NETWORK_STATE_CHANGED);
    Subscribe(EVENT_WIFI_CONN_CHANGED);
    Subscribe(EVENT_VPN_MODE_CHANGED);
    Subscribe(EVENT_CONNECTIVITY_CHANGE);
    Subscribe(EVENT_SCREEN_ON);
    Subscribe(EVENT_SCREEN_OFF);
    Subscribe(EVENT_URSP_CHANGED);
    g_broadcastProxy = std::make_shared<broadcast_proxy>();
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->Init();
}

void NetworkSliceManager::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    if (event == nullptr) {
        return;
    }
    NETMGR_EXT_LOG_I("event id:%{public}d", event->GetInnerEventId());
    switch (event->GetInnerEventId()) {
        case EVENT_GET_SLICE_PARA:
            GetRouteSelectionDescriptorByAppDescriptor(event->GetSharedObject<GetSlicePara>());
            break;
        case EVENT_HANDLE_ALLOWED_NSSAI:
            HandleAllowedNssaiFromUnsolData(event->GetSharedObject<std::vector<uint8_t>>());
            break;
        case EVENT_HANDLE_UE_POLICY:
            HandleUrspFromUnsolData(event->GetSharedObject<std::vector<uint8_t>>());
            break;
        case EVENT_HANDLE_NETWORK_ACTIVATE_RESULT:
            HandleNetworkActivateResult(event->GetSharedObject<std::map<std::string, std::any>>());
            break;
        case EVENT_KERNEL_IP_ADDR_REPORT:
            HandleIpRpt(event->GetSharedObject<std::vector<uint8_t>>());
            break;
        case EVENT_BIND_TO_NETWORK:
            BindProcessToNetworkByFullPara(event->GetSharedObject<std::map<std::string, std::string>>());
            break;
        case EVENT_DEL_BIND_TO_NETWORK:
            DeleteNetworkBindByFullPara(event->GetSharedObject<std::map<std::string, std::string>>());
            break;
        case EVENT_HANDLE_EHPLMN:
            HandleEhplmnFromUnsolData(event->GetSharedObject<std::vector<uint8_t>>());
            break;
        case EVENT_URSP_CHANGED:
        case EVENT_NETWORK_STATE_CHANGED:
        case EVENT_CONNECTIVITY_CHANGE:
        case EVENT_SYSTEM_WIFI_NETWORK_STATE_CHANGED:
        case EVENT_FOREGROUND_APP_CHANGED:
        case EVENT_AIR_MODE_CHANGED:
        case EVENT_WIFI_CONN_CHANGED:
        case EVENT_VPN_MODE_CHANGED:
        case EVENT_SCREEN_ON:
        case EVENT_SCREEN_OFF:
        case EVENT_NETWORK_PARA_FORBIDDEN_TIMEOUT:
        case EVENT_HANDLE_SIM_STATE_CHANGED:
            ProcessEventEx(event);
            break;
        default:
            NETMGR_EXT_LOG_I("unknow msg");
            break;
    }
}

void NetworkSliceManager::ProcessEventEx(const AppExecFwk::InnerEvent::Pointer& event)
{
    switch (event->GetInnerEventId()) {
        case EVENT_NETWORK_STATE_CHANGED:
            IpParaReportControl();
            SendUrspUpdateMsg();
            break;
        case EVENT_CONNECTIVITY_CHANGE:
        case EVENT_SYSTEM_WIFI_NETWORK_STATE_CHANGED:
            IpParaReportControl();
            break;
        case EVENT_FOREGROUND_APP_CHANGED:
            HandleForegroundAppChanged(event->GetSharedObject<AppExecFwk::AppStateData>());
            break;
        case EVENT_AIR_MODE_CHANGED:
            HandleAirModeChanged(event->GetParam());
            break;
        case EVENT_WIFI_CONN_CHANGED:
            HandleWifiConnChanged(event->GetParam());
            break;
        case EVENT_VPN_MODE_CHANGED:
            HandleVpnModeChanged(event->GetParam());
            break;
        case EVENT_SCREEN_ON:
            HandleScreenOn();
            break;
        case EVENT_SCREEN_OFF:
            HandleScreenOff();
            break;
        case EVENT_HANDLE_SIM_STATE_CHANGED:
            HandleSimStateChanged();
            break;
        case EVENT_URSP_CHANGED:
            HandleUrspChanged(event->GetSharedObject<std::map<std::string, std::string>>());
            break;
        case EVENT_NETWORK_PARA_FORBIDDEN_TIMEOUT:
            ProcessNetworkParaForbiddenTimeOut();
            break;
    }
}

void NetworkSliceManager::HandleForegroundAppChanged(const std::shared_ptr<AppExecFwk::AppStateData>& msg)
{
    if (msg == nullptr) {
        NETMGR_EXT_LOG_E("HandleForegroundAppChanged appStateData is null");
        return;
    }
    int32_t uid = msg->uid;
    foregroundApp_uid = uid;
    std::string bundleName = msg->bundleName;
    NETMGR_EXT_LOG_E("HandleForegroundAppChanged uid = %{public}d, bundleName = %{public}s", uid, bundleName.c_str());
    if (msg->bundleName == "com.ohos.sceneboard") {
        return;
    }
    if (msg->state == static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_FOREGROUND) && msg->isFocused == 1) {
        NETMGR_EXT_LOG_E("HandleForegroundAppChanged RequestNetworkSliceForPackageName");
        DelayedSingleton<HwNetworkSliceManager>::GetInstance()->RequestNetworkSliceForPackageName(uid, bundleName);
        return;
    } else if (msg->state == static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_BACKGROUND)) {
        DelayedSingleton<HwNetworkSliceManager>::GetInstance()->ReleaseNetworkSliceByApp(uid);
    } else if (msg->state == static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_TERMINATED)) {
        DelayedSingleton<HwNetworkSliceManager>::GetInstance()->HandleUidGone(uid);
    }
}

void NetworkSliceManager::HandleUrspChanged(const std::shared_ptr<std::map<std::string, std::string>>& msg)
{
    NETMGR_EXT_LOG_I("NetworkSliceManager::HandleUrspChanged");
    if (!msg) {
        NETMGR_EXT_LOG_E("nullptr");
        return;
    }
    std::map<std::string, std::string> data = *msg;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->HandleUrspChanged(data);
}

void NetworkSliceManager::HandleAirModeChanged(int32_t mode)
{
    if (mode == 0) {
        airModeOn_ = false;
    } else {
        airModeOn_ = true;
    }
}
 
void NetworkSliceManager::HandleWifiConnChanged(int32_t state)
{
    bool wifiConnLastStatus = wifiConn_;
    if (state == WIFICONNECTED) {
        wifiConn_ = true;
    } else if (state == WIFIDISCONNECTED) {
        wifiConn_ = false;
    }
    if (wifiConnLastStatus != wifiConn_) {
        DelayedSingleton<HwNetworkSliceManager>::GetInstance()->onWifiNetworkStateChanged(wifiConn_);
    }
}

void NetworkSliceManager::HandleVpnModeChanged(bool mode)
{
    vpnMode_ = mode;
}

void NetworkSliceManager::HandleScreenOn()
{
    screenOn_ = true;
}

void NetworkSliceManager::HandleScreenOff()
{
    screenOn_ = false;
}

void NetworkSliceManager::SendUrspUpdateMsg()
{
    if ((!mIsUrspFirstReported) && isSaState()) {
        if (sNrUnsolicitedMsgParser_ != nullptr) {
        }
        mIsUrspFirstReported = true;
    }
}

void NetworkSliceManager::ProcessNetworkParaForbiddenTimeOut()
{
    if (mNormalForbiddenRules.size() == 1) {
        mNormalForbiddenRules.erase(mNormalForbiddenRules.begin());
    } else {
        int64_t timeMillies = mNormalForbiddenRules[1].getCurrentTimeMillies()
            - mNormalForbiddenRules[0].getCurrentTimeMillies();
        StartNetworkParaForbiddenTimer(timeMillies);
        mNormalForbiddenRules.erase(mNormalForbiddenRules.begin());
    }
}

void NetworkSliceManager::StartNetworkParaForbiddenTimer(int64_t timeMillies)
{
    return;
}

void NetworkSliceManager::HandleSimStateChanged()
{
    NETMGR_EXT_LOG_I("NetworkSliceManager::HandleSimStateChanged");
    if (sNrUnsolicitedMsgParser_ != nullptr) {
    }
    if (mApnStartflag == false) {
        DelayedSingleton<NetworkSliceService>::GetInstance()->UpdateNetworkSliceApn();
        mApnStartflag = true;
    }
    return;
}

void NetworkSliceManager::HandleAllowedNssaiFromUnsolData(const std::shared_ptr<std::vector<uint8_t>>& msg)
{
    NETMGR_EXT_LOG_I("NetworkSliceManager::HandleAllowedNssaiFromUnsolData");
    if (!msg) {
        NETMGR_EXT_LOG_E("nullptr");
        return;
    }
    std::vector<uint8_t> buffer = *msg;
    NETMGR_EXT_LOG_I("buffer size:%{public}d", (int)buffer.size());
    if (buffer.empty()) {
        NETMGR_EXT_LOG_E("NetworkSliceManager::HandleAllowedNssaiFromUnsolData: buffer is invalid");
        return;
    }
    if (sNrUnsolicitedMsgParser_ != nullptr) {
    }
    return;
}

void NetworkSliceManager::HandleUrspFromUnsolData(const std::shared_ptr<std::vector<uint8_t>>& msg)
{
    NETMGR_EXT_LOG_I("NetworkSliceManager::HandleUrspFromUnsolData");
    if (!msg) {
        NETMGR_EXT_LOG_E("nullptr");
        return;
    }
    
    std::vector<uint8_t> buffer = *msg;
    NETMGR_EXT_LOG_I("buffer size:%{public}d", (int)buffer.size());
    if (buffer.empty()) {
        NETMGR_EXT_LOG_E("NetworkSliceManager::HandleUrspFromUnsolData: buffer is invalid");
        return;
    }
    if (sNrUnsolicitedMsgParser_ == nullptr) {
        NETMGR_EXT_LOG_E("sNrUnsolicitedMsgParser_ == nullptr");
        return;
    }
    if (sNrUnsolicitedMsgParser_ != nullptr) {
    }
    return;
}

void NetworkSliceManager::HandleNetworkActivateResult(std::shared_ptr<std::map<std::string, std::any>> data)
{
    if (data->empty()) {
        return;
    }
    auto resultIt = data->find("result");
    if (resultIt == data->end() || std::any_cast<int>(resultIt->second) != NETWORK_ACTIVATE_RESULT_SUCCESS) {
        return;
    }
    std::shared_ptr<ForbiddenRouteDescriptor> forbiddenRouteDescriptor =
        std::make_shared<ForbiddenRouteDescriptor>();
    auto dnnIt = data->find("dnn");
    if (dnnIt != data->end()) {
        forbiddenRouteDescriptor->setDnn(std::any_cast<std::string>(dnnIt->second));
    }

    auto snssaiIt = data->find("snssai");
    if (snssaiIt != data->end()) {
        forbiddenRouteDescriptor->setSnssai(std::any_cast<std::string>(snssaiIt->second));
    }

    auto pduSessionTypeIt = data->find("pduSessionType");
    if (pduSessionTypeIt != data->end()) {
        forbiddenRouteDescriptor->setPduSessionType(std::any_cast<int>(pduSessionTypeIt->second));
    }

    auto sscModeIt = data->find("sscMode");
    if (sscModeIt != data->end()) {
        forbiddenRouteDescriptor->setSscMode(std::any_cast<uint8_t>(sscModeIt->second));
    }

    if (std::any_cast<int>(resultIt->second) != NETWORK_ACTIVATE_RESULT_NORMAL_FAIL) {
        return;
    }

    forbiddenRouteDescriptor->setCurrentTimeMillies(StateUtils::GetCurrentSysTimeMs());
    if (mNormalForbiddenRules.empty()) {
        StartNetworkParaForbiddenTimer(NETWORK_SLICE_PARA_FORBIDDEN_MS);
    }
    mNormalForbiddenRules.push_back(*forbiddenRouteDescriptor);
}

void NetworkSliceManager::HandleEhplmnFromUnsolData(const std::shared_ptr<std::vector<uint8_t>>& msg)
{
    NETMGR_EXT_LOG_I("NetworkSliceManager::HandleEhplmnFromUnsolData");
    if (!msg) {
        NETMGR_EXT_LOG_E("nullptr");
        return;
    }
    std::vector<uint8_t> buffer = *msg;
    NETMGR_EXT_LOG_I("buffer size:%{public}d", (int)buffer.size());
    if (buffer.empty()) {
        NETMGR_EXT_LOG_E("NetworkSliceManager::HandleEhplmnFromUnsolData: buffer is invalid");
        return;
    }
    if (sNrUnsolicitedMsgParser_ != nullptr) {
    }
    return;
}

void NetworkSliceManager::InitUePolicy()
{
    NETMGR_EXT_LOG_I("NetworkSliceManager:InitUePolicy");
    sUrspConfig_ = std::make_shared<UrspConfig>(UrspConfig::GetInstance());
    sNrUnsolicitedMsgParser_ = std::make_shared<NrUnsolicitedMsgParser>(NrUnsolicitedMsgParser::GetInstance());
    IpParaReportControl();
    return;
}

void NetworkSliceManager::HandleIpRpt(const std::shared_ptr<std::vector<uint8_t>>& msg)
{
    NETMGR_EXT_LOG_I("NetworkSliceManager::HandleIpRpt");
    if (!msg) {
        NETMGR_EXT_LOG_E("nullptr");
        return;
    }
    std::vector<uint8_t> buffer = *msg;
    NETMGR_EXT_LOG_I("buffer size:%{public}d", (int)buffer.size());
    if (buffer.size() < NetworkSliceCommConfig::LEN_SHORT) {
        NETMGR_EXT_LOG_E("get type, buffer.size() < NetworkSliceCommConfig::LEN_SHORT");
        return;
    }
    int startIndex = 0;
    short type = GetShort(startIndex, buffer, true);
    if (((int)buffer.size() - startIndex) < NetworkSliceCommConfig::LEN_SHORT) {
        NETMGR_EXT_LOG_E("get len, buffer.size() < NetworkSliceCommConfig::LEN_SHORT");
        return;
    }
    short len = GetShort(startIndex, buffer, true);
    AppDescriptor appDescriptor;
    if (((int)buffer.size() - startIndex) < NetworkSliceCommConfig::LEN_INT) {
        NETMGR_EXT_LOG_E("get uid for Ipv4Addr, buffer.size() < NetworkSliceCommConfig::LEN_INT");
        return;
    }
    appDescriptor.setUid(GetInt(startIndex, buffer, true));
    std::map<std::string, std::string> bundle;
    bundle["uid"] = appDescriptor.getUid();

    /* ipv4 addr total len is 15, ipv6 addr total len is 27 */
    if (len == IPV4ADDRTOTALLEN) {
        HandleIpv4Rpt(startIndex, buffer, bundle, appDescriptor);
    } else if (len == IPV6ADDRTOTALLEN) {
        HandleIpv6Rpt(startIndex, buffer, bundle, appDescriptor);
    } else {
        NETMGR_EXT_LOG_E("ip report len is invalid ");
    }
}
void NetworkSliceManager::onUrspAvailableStateChanged()
{
    IpParaReportControl();
}

void NetworkSliceManager::BindProcessToNetworkByFullPara(std::shared_ptr<std::map<std::string, std::string>> msg)
{
    if (!msg) {
        NETMGR_EXT_LOG_E("nullptr");
        return;
    }
    NETMGR_EXT_LOG_E("NetworkSliceManager::BindProcessToNetworkByFullPara");
    std::map<std::string, std::string> data = *(msg.get());
    AddRoutePara addRoutePara;
    if (!GetRoutePara(addRoutePara, data)) {
        return;
    }
    NETMGR_EXT_LOG_E("bindProcessToNetworkByFullPara, netId = %{public}d, urspPrecedence = %{public}d,\
        len = %{public}d, uidNum = %{public}d, protocolIdNum = %{public}d, singleRemotePortNum = %{public}d,\
        remotePortRangeNum = %{public}d, ipv4Num = %{public}d, ipv6Num = %{public}d",
        addRoutePara.netId, addRoutePara.urspPrecedence, addRoutePara.len, addRoutePara.uidNum,
        addRoutePara.protocolIdNum, addRoutePara.singleRemotePortNum, addRoutePara.remotePortRangeNum,
        addRoutePara.ipv4Num, addRoutePara.ipv6Num);
    std::vector<uint8_t> buffer;
    if (!FillRoutePara(buffer, addRoutePara)) {
        return;
    }
    for (int i = 0; i < addRoutePara.uidNum; i++) {
        NETMGR_EXT_LOG_I("Need bindUidProcessToNetworkForDns");
    }
    int startIndex = 0;
    short type = GetShort(startIndex, buffer);
    short len = GetShort(startIndex, buffer);
    std::vector<uint8_t> msgData;
    for (int i = startIndex; i < (int)buffer.size(); ++i) {
        msgData.push_back(buffer[i]);
    }
    std::unique_ptr<char[]> requestbuffer = std::make_unique<char[]>(len);
    const auto &requestMsg = reinterpret_cast<KernelBindMsg *>(requestbuffer.get());
    requestMsg->type = type;
    requestMsg->len = len;
    if (memcpy_s(reinterpret_cast<char *>(requestMsg->buf), len, msgData.data(), len) != EOK) {
        NETMGR_EXT_LOG_E("BindProcessToNetworkByFullPara memcpy_s error");
    }
    int ret = DelayedSingleton<NetworkSliceKernelProxy>::GetInstance()->SendDataToKernel(
        reinterpret_cast<KernelMsg &>(*requestMsg));
    if (ret != 0) {
        return;
    }
    if (addRoutePara.uidNum > 0) {
        NETMGR_EXT_LOG_I("Need clearDnsCache");
    }
}

std::vector<int> NetworkSliceManager::GetUidArray(std::string uids)
{
    std::vector<std::string> stringUidArrays;
    int uidNum = 0;
    std::vector<int> uidArrays = std::vector<int>();
    if (!uids.empty()) {
        stringUidArrays = Split(uids, COMMA_SEPARATOR);
        for (int i = 0; i < (int)stringUidArrays.size(); ++i) {
            int uid = std::stoi(stringUidArrays[i]);
            uidArrays.push_back(uid);
            uidNum++;
        }
    }
    return uidArrays;
}

std::vector<int> NetworkSliceManager::GetPrecedenceArray(std::string precedences)
{
    std::vector<std::string> precedencesString;
    int precedenceNum = 0;
    std::vector<int> precedenceArray = std::vector<int>();
    if (!precedences.empty()) {
        precedencesString = Split(precedences, COMMA_SEPARATOR);
        precedenceNum = (int)precedencesString.size();
        for (int i = 0; i < precedenceNum; ++i) {
            int precedence = std::stoi(precedencesString[i]);
            precedenceArray.push_back(precedence);
        }
    }
    return precedenceArray;
}

bool NetworkSliceManager::GetUidRoutePara(AddRoutePara& addRoutePara, std::map<std::string, std::string>& data)
{
    std::string uids;
    if (data.find("uids") != data.end()) {
        uids = data["uids"];
    }
    NETMGR_EXT_LOG_I("GetUidRoutePara uids = %{public}s", uids.c_str());
    if (!uids.empty()) {
        std::vector<std::string> stringUidArrays = Split(uids, COMMA_SEPARATOR);
        addRoutePara.uidNum = (int)stringUidArrays.size();
        for (int i = 0; i < addRoutePara.uidNum; i++) {
            if (!stringUidArrays[i].empty()) {
                addRoutePara.uidArrays.push_back(std::stoi(stringUidArrays[i]));
                NETMGR_EXT_LOG_I("addRoutePara.uidArrays[%{public}d] = %{public}d", i, addRoutePara.uidArrays[i]);
            }
        }
    }
    return true;
}

bool NetworkSliceManager::GetRoutePara(AddRoutePara& addRoutePara, std::map<std::string, std::string>& data)
{
    if (!GetUidRoutePara(addRoutePara, data)) {
        return false;
    }
    if (data.find("ipv4Num") != data.end()) {
        addRoutePara.ipv4Num = ConvertInt2UnsignedByte(std::stoi(data["ipv4Num"]));
    }
    if (data.find("ipv4AddrAndMask") != data.end()) {
        addRoutePara.ipv4AddrAndMasks = ConvertstringTouInt8Vector(data["ipv4AddrAndMask"]);
    } else {
        addRoutePara.ipv4AddrAndMasks.clear();
    }
    if (addRoutePara.ipv4Num != 0) {
        if (addRoutePara.ipv4AddrAndMasks.empty() || addRoutePara.ipv4AddrAndMasks.size() !=
        (NetworkSliceCommConfig::LEN_INT + NetworkSliceCommConfig::LEN_INT) * addRoutePara.ipv4Num) {
            NETMGR_EXT_LOG_E("ipv4Num not match, ipv4Num = %{public}d", addRoutePara.ipv4Num);
            return false;
        }
    }
    if (data.find("ipv6Num") != data.end()) {
        addRoutePara.ipv6Num = ConvertInt2UnsignedByte(std::stoi(data["ipv6Num"]));
    }
    if (data.find("ipv6AddrAndPrefix") != data.end()) {
        addRoutePara.ipv6AddrAndPrefixs = ConvertstringTouInt8Vector(data["ipv6AddrAndPrefix"]);
    } else {
        addRoutePara.ipv6AddrAndPrefixs.clear();
    }
    if (addRoutePara.ipv6Num != 0) {
        if (addRoutePara.ipv6AddrAndPrefixs.empty() || addRoutePara.ipv6AddrAndPrefixs.size() !=
        (NetworkSliceCommConfig::LEN_SIXTEEN_BYTE + NetworkSliceCommConfig::LEN_BYTE) * addRoutePara.ipv6Num) {
            NETMGR_EXT_LOG_E("ipv6Num not match, ipv4Num = %{public}d", addRoutePara.ipv6Num);
            return false;
        }
    }
    GetRouteParaEx(addRoutePara, data);
    return CalculateParaLen(addRoutePara);
}

void NetworkSliceManager::GetRouteParaEx(AddRoutePara& addRoutePara, std::map<std::string, std::string>& data)
{
    if (data.find("protocolIds") != data.end()) {
        std::string protocolIds = data["protocolIds"];
        if (!protocolIds.empty()) {
            addRoutePara.protocolIdArrays = Split(protocolIds, COMMA_SEPARATOR);
            addRoutePara.protocolIdNum = (int)addRoutePara.protocolIdArrays.size();
        }
    }
    if (data.find("remotePorts") != data.end() && !data["remotePorts"].empty()) {
        std::string remotePorts = data["remotePorts"];
        addRoutePara.remotePortsArrays = Split(remotePorts, COMMA_SEPARATOR);
        addRoutePara.remotePortNum = (int)addRoutePara.remotePortsArrays.size();
        for (int i = 0; i < addRoutePara.remotePortNum; i++) {
            if (addRoutePara.remotePortsArrays[i].find("-") != std::string::npos) {
                addRoutePara.remotePortRangeNum++;
            }
        }
        addRoutePara.singleRemotePortNum = addRoutePara.remotePortNum - addRoutePara.remotePortRangeNum;
    }
    if (data.find("netId") != data.end()) {
        addRoutePara.netId = std::stoi(data["netId"]);
    }
    if (data.find("urspPrecedence") != data.end()) {
        addRoutePara.urspPrecedence = ConvertInt2UnsignedByte(std::stoi(data["urspPrecedence"]));
    }
}

bool NetworkSliceManager::CalculateParaLen(AddRoutePara& addRoutePara)
{
    /*
        * len = type + len
        * + netId
        * + precedence
        * + uid num + uid
        * + ipv4 num + ipv4
        * + ipv6 num + ipv6
        * + protocol num + protocol
        * + singleRemotePort num + singleRemotePort
        * + remotePortRange num + remotePortRange
        */
    int totalLen = NetworkSliceCommConfig::LEN_SHORT + NetworkSliceCommConfig::LEN_SHORT
        + NetworkSliceCommConfig::LEN_INT
        + NetworkSliceCommConfig::LEN_BYTE
        + NetworkSliceCommConfig::LEN_BYTE + NetworkSliceCommConfig::LEN_INT * addRoutePara.uidNum
        + NetworkSliceCommConfig::LEN_BYTE
        + (NetworkSliceCommConfig::LEN_INT + NetworkSliceCommConfig::LEN_INT) * addRoutePara.ipv4Num
        + NetworkSliceCommConfig::LEN_BYTE
        + (NetworkSliceCommConfig::LEN_SIXTEEN_BYTE + NetworkSliceCommConfig::LEN_BYTE) * addRoutePara.ipv6Num
        + NetworkSliceCommConfig::LEN_BYTE
        + NetworkSliceCommConfig::LEN_BYTE * addRoutePara.protocolIdNum
        + NetworkSliceCommConfig::LEN_BYTE
        + NetworkSliceCommConfig::LEN_SHORT * addRoutePara.singleRemotePortNum
        + NetworkSliceCommConfig::LEN_BYTE
        + NetworkSliceCommConfig::LEN_INT * addRoutePara.remotePortRangeNum;

    if (totalLen > CONVERT_INT_AND_SHORT) {
        return false;
    }
    addRoutePara.len = (short) totalLen;
    return true;
}

bool NetworkSliceManager::FillRoutePara(std::vector<uint8_t>& buffer, AddRoutePara addRoutePara)
{
    PutShort(buffer, KERNEL_BIND_UID_MSG);
    PutShort(buffer, addRoutePara.len);
    PutInt(buffer, addRoutePara.netId, false);
    buffer.push_back(addRoutePara.urspPrecedence);
    buffer.push_back(ConvertInt2UnsignedByte(addRoutePara.uidNum));
    for (int i = 0; i < addRoutePara.uidNum; i++) {
        PutInt(buffer, addRoutePara.uidArrays[i], false);
    }
    buffer.push_back(addRoutePara.ipv4Num);
    if (addRoutePara.ipv4Num != 0) {
        buffer.insert(buffer.end(), addRoutePara.ipv4AddrAndMasks.begin(), addRoutePara.ipv4AddrAndMasks.end());
    }
    buffer.push_back(addRoutePara.ipv6Num);
    if (addRoutePara.ipv6Num != 0) {
        buffer.insert(buffer.end(), addRoutePara.ipv6AddrAndPrefixs.begin(), addRoutePara.ipv6AddrAndPrefixs.end());
    }
    buffer.push_back(ConvertInt2UnsignedByte(addRoutePara.protocolIdNum));
    for (int i = 0; i < addRoutePara.protocolIdNum; i++) {
        buffer.push_back(ConvertInt2UnsignedByte(std::stoi(addRoutePara.protocolIdArrays[i])));
    }
    buffer.push_back(ConvertInt2UnsignedByte(addRoutePara.singleRemotePortNum));
    for (int i = 0; i < addRoutePara.remotePortNum; i++) {
        if (addRoutePara.remotePortsArrays[i].find("-") == std::string::npos) {
            continue;
        }
        int singleRemotePort = std::stoi(addRoutePara.remotePortsArrays[i]);
        NETMGR_EXT_LOG_I("FillRoutePara singleRemotePort = %{public}d", ConvertInt2UnsignedShort(singleRemotePort));
        PutShort(buffer, ConvertInt2UnsignedShort(singleRemotePort), false);
    }

    buffer.push_back(ConvertInt2UnsignedByte(addRoutePara.remotePortRangeNum));
    for (int i = 0; i < addRoutePara.remotePortNum; i++) {
        if (addRoutePara.remotePortsArrays[i].find("-") == std::string::npos) {
            continue;
        }
        std::string remotePortRange = addRoutePara.remotePortsArrays[i];
        std::vector<std::string> remotePortRangeArrays = Split(remotePortRange, "-");
        NETMGR_EXT_LOG_I("FillRoutePara RemotePortRange = %{public}d - %{public}d",
            ConvertInt2UnsignedShort(std::stoi(remotePortRangeArrays[0])),
            ConvertInt2UnsignedShort(std::stoi(remotePortRangeArrays[1])));
        PutShort(buffer, ConvertInt2UnsignedShort(std::stoi(remotePortRangeArrays[0])), false);
        PutShort(buffer, ConvertInt2UnsignedShort(std::stoi(remotePortRangeArrays[1])), false);
    }
    return true;
}

void NetworkSliceManager::DeleteNetworkBindByFullPara(std::shared_ptr<std::map<std::string, std::string>> msg)
{
    if (!msg) {
        NETMGR_EXT_LOG_E("nullptr");
        return;
    }
    NETMGR_EXT_LOG_I("NetworkSliceManager::DeleteNetworkBindByFullPara");
    std::map<std::string, std::string> data = *(msg.get());
    if (data.empty()) {
        NETMGR_EXT_LOG_E("data is null in deleteNetworkBind");
        return;
    }
    int type = -1;
    if (data.find("type") != data.end()) {
        type = std::stoi(data["type"]);
    }
    if (type < DEL_BIND_ALL || type > DEL_BIND_PRECEDENCE) {
        NETMGR_EXT_LOG_E("invalid type");
        return;
    }
    std::string uids;
    if (data.find("uids") != data.end()) {
        uids = data["uids"];
    }
    std::vector<int> uidArrays = GetUidArray(uids);
    int uidNum = 0;
    if (!uidArrays.empty()) {
        uidNum = (int)uidArrays.size();
    }
    for (int i = 0; i < uidNum; i++) {
        NETMGR_EXT_LOG_I("Need bindUidProcessToNetworkForDns");
    }
    if (uidNum > 0) {
        NETMGR_EXT_LOG_I("Need clearDnsCache");
    }
    std::string precedences;
    if (data.find("urspPrecedence") != data.end()) {
        precedences = std::stoi(data["urspPrecedence"]);
    }
    std::vector<int> precedenceArray = GetPrecedenceArray(precedences);
    int precedenceNum = 0;
    if (!precedenceArray.empty()) {
        precedenceNum = (int)precedenceArray.size();
    }
    /*
    * len equals 2B dataType + 2B len + 4B delType + 4B netId + 4B precedenceNum + 4B * precedenceNum
    * + 4B uidNum + 4 * uidNum
    */
    short len = (short) (2 + 2 + 4 + 4 + 4 + (4 * precedenceNum) + 4 + (4 * uidNum));
    FillDeletePara(len, type, precedenceArray, uidArrays, data);
}

void NetworkSliceManager::FillDeletePara(short len, int type, std::vector<int> precedenceArray,
    std::vector<int> uidArrays, std::map<std::string, std::string> data)
{
    int precedenceNum = 0;
    if (!precedenceArray.empty()) {
        precedenceNum = (int)precedenceArray.size();
    }
    int uidNum = 0;
    if (!uidArrays.empty()) {
        uidNum = (int)uidArrays.size();
    }
    std::vector<uint8_t> buffer;
    PutInt(buffer, type, false);
    int netId = -1;
    if (data.find("netId") != data.end()) {
        netId = std::stoi(data["netId"]);
        PutInt(buffer, netId, false);
    }
    PutInt(buffer, precedenceNum, false);
    for (int i = 0; i < precedenceNum; i++) {
        PutInt(buffer, precedenceArray[i], false);
    }
    PutInt(buffer, uidNum, false);
    for (int i = 0; i < uidNum; i++) {
        PutInt(buffer, uidArrays[i], false);
    }
    NETMGR_EXT_LOG_I("DeleteNetworkBindByFullPara, len = %{public}d, netId = %{public}d, uidNum = %{public}d, \
        precedenceNum = %{public}d, delType = %{public}d", len, netId, uidNum, precedenceNum, type);

    std::unique_ptr<char[]> requestbuffer = std::make_unique<char[]>(len);
    const auto &requestMsg = reinterpret_cast<KernelBindMsg *>(requestbuffer.get());
    requestMsg->type = KERNEL_DEL_UID_BIND_MSG;
    requestMsg->len = len;
    if (memcpy_s(reinterpret_cast<char *>(requestMsg->buf), len, buffer.data(), len) != EOK) {
        NETMGR_EXT_LOG_E("DeleteNetworkBindByFullPara memcpy_s error");
    }
    int ret = DelayedSingleton<NetworkSliceKernelProxy>::GetInstance()->SendDataToKernel(
        reinterpret_cast<KernelMsg &>(*requestMsg));
    if (ret < 0) {
        NETMGR_EXT_LOG_E("SendDataToKernel : DeleteNetworkBindByFullPara failed");
    }
}

void NetworkSliceManager::IpParaReportControl()
{
    NETMGR_EXT_LOG_I("IpParaReportControl");
    bool isMeetConditions = isMeetNetworkSliceConditions();
    if (isMeetConditions == mIsIpParaReportEnable) {
        return;
    }
    mIsIpParaReportEnable = isMeetConditions;
    int enable = isMeetConditions ? 1 : 0;
    NETMGR_EXT_LOG_I("IpParaReportControl %{public}d", enable);
    // type + len + enable equals 12
    short len = 12;
    KernelIpRptEnableMsg msgData;
    msgData.type = IP_PARA_REPORT_CONTROL_MSG;
    msgData.len = len;
    msgData.isEnable = enable;
    int32_t ret = DelayedSingleton<NetworkSliceKernelProxy>::GetInstance()->SendDataToKernel(
        reinterpret_cast<KernelMsg&>(msgData));
    if (ret < 0) {
        NETMGR_EXT_LOG_E("SendDataToKernel : send ipParaReport failed");
    }
}

void NetworkSliceManager::GetRouteSelectionDescriptorByAppDescriptor(const std::shared_ptr<GetSlicePara>& getSlicePara)
{
    NETMGR_EXT_LOG_I("GetRouteSelectionDescriptorByAppDescriptor");
    if (!isMeetNetworkSliceConditions()) {
        NotifySlicePara(getSlicePara);
        return;
    }
    if (getSlicePara->data.empty()) {
        NETMGR_EXT_LOG_I("input data is null");
        NotifySlicePara(getSlicePara);
        return;
    }
    AppDescriptor appDescriptor;
    if (!GetAppDescriptor(getSlicePara->data, appDescriptor)) {
        NotifySlicePara(getSlicePara);
        return;
    }
    if (sUrspConfig_ == nullptr || sNrUnsolicitedMsgParser_ == nullptr) {
        NETMGR_EXT_LOG_E("getRouteSelectionDescriptorByAppDescriptor fail, params null");
        NotifySlicePara(getSlicePara);
        return;
    }
    std::string plmn = sNrUnsolicitedMsgParser_->GetHplmn();
    NETMGR_EXT_LOG_I("getRouteSelectionDescriptorByAppDescriptor, plmn = %{public}s", plmn.c_str());
    SelectedRouteDescriptor routeRule;
    if (!sUrspConfig_->SliceNetworkSelection(routeRule, plmn, appDescriptor)) {
        NETMGR_EXT_LOG_E("getRouteSelectionDescriptorByAppDescriptor fail, routeRule = null");
        NotifySlicePara(getSlicePara);
        return;
    }
    DumpSelectedRouteDescriptor(routeRule);
    FillRouteSelectionDescriptor(getSlicePara->ret, routeRule);
    NotifySlicePara(getSlicePara);
}

void NetworkSliceManager::HandleIpv4Rpt(int& startIndex, const std::vector<uint8_t>& buffer,
    std::map<std::string, std::string>& bundle, AppDescriptor& appDescriptor)
{
    if (buffer.size() < NetworkSliceCommConfig::LEN_INT) {
        NETMGR_EXT_LOG_E("get Ipv4Addr, buffer.size() < NetworkSliceCommConfig::LEN_INT");
        return;
    }
    std::vector<uint8_t> ipv4Addr_vec;
    for (size_t i = 0; i < NetworkSliceCommConfig::LEN_INT; ++i) {
        ipv4Addr_vec.push_back(buffer[startIndex + NetworkSliceCommConfig::LEN_INT - i - 1]); // Litter Endian
    }
    startIndex += NetworkSliceCommConfig::LEN_INT;
    appDescriptor.setIpv4Addr(vectorToUint32(ipv4Addr_vec));
    bundle["ip"] = std::to_string(appDescriptor.getIpv4Addr());

    if (buffer.size() < NetworkSliceCommConfig::LEN_THREE_BYTE) {
        NETMGR_EXT_LOG_E("get len, buffer.size() < NetworkSliceCommConfig::LEN_THREE_BYTE");
        return;
    }
    short remotePort = GetShort(startIndex, buffer, true);
    appDescriptor.setRemotePort(ConvertUnsignedShort2Int(remotePort));
    uint8_t protocolId = buffer[startIndex++];
    appDescriptor.setProtocolId(static_cast<int>(protocolId));

    DumpAppDescriptor(appDescriptor);

    bundle["remotePort"] = std::to_string(appDescriptor.getRemotePort());
    bundle["protocolId"] = std::to_string(appDescriptor.getProtocolId());

    SendIpPara(appDescriptor, bundle);
}

void NetworkSliceManager::HandleIpv6Rpt(int& startIndex, const std::vector<uint8_t>& buffer,
    std::map<std::string, std::string>& bundle, AppDescriptor& appDescriptor)
{
    if (buffer.size() < NetworkSliceCommConfig::LEN_SIXTEEN_BYTE) {
        NETMGR_EXT_LOG_E("get Ipv6Addr, buffer.size() < NetworkSliceCommConfig::LEN_SIXTEEN_BYTE");
        return;
    }
    std::array<uint8_t, NetworkSliceCommConfig::LEN_IPV6ADDR> ipv6Addr_vec;
    for (size_t i = 0; i < NetworkSliceCommConfig::LEN_SIXTEEN_BYTE; ++i) {
        ipv6Addr_vec[i] = buffer[startIndex + NetworkSliceCommConfig::LEN_SIXTEEN_BYTE - i - 1];
    }
    startIndex += NetworkSliceCommConfig::LEN_SIXTEEN_BYTE;
    appDescriptor.setIpv6Addr(ipv6Addr_vec);
    bundle["ip"] = transIpv6AddrToStr(appDescriptor.getIpv6Addr());

    if (buffer.size() < NetworkSliceCommConfig::LEN_THREE_BYTE) {
        NETMGR_EXT_LOG_E("get len, buffer.size() < NetworkSliceCommConfig::LEN_THREE_BYTE");
        return;
    }
    short remotePort = GetShort(startIndex, buffer, true);
    appDescriptor.setRemotePort(ConvertUnsignedShort2Int(remotePort));
    uint8_t protocolId = buffer[startIndex++];
    appDescriptor.setProtocolId(static_cast<int>(protocolId));
    DumpAppDescriptor(appDescriptor);
    bundle["remotePort"] = std::to_string(appDescriptor.getRemotePort());
    bundle["protocolId"] = std::to_string(appDescriptor.getProtocolId());
    SendIpPara(appDescriptor, bundle);
}

void NetworkSliceManager::SendIpPara(AppDescriptor appDescriptor, std::map<std::string, std::string> bundle)
{
    if (sUrspConfig_ == nullptr || sNrUnsolicitedMsgParser_ == nullptr) {
        NETMGR_EXT_LOG_E("sendIpPara fail, param null");
        return;
    }
    std::string plmn = sNrUnsolicitedMsgParser_->GetHplmn();
    if (!sUrspConfig_->isIpThreeTuplesInWhiteList(plmn, appDescriptor)) {
        NETMGR_EXT_LOG_I("Ip Three Tuples not In WhiteList");
        return;
    }
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->HandleIpReport(bundle);
}

bool NetworkSliceManager::isMeetNetworkSliceConditions()
{
    if (!isNrSlicesSupported()) {
        NETMGR_EXT_LOG_I("UE not support nr slices");
        return false;
    }
    if (!hasAvailableUrspRule()) {
        NETMGR_EXT_LOG_I("do not have available ursp rule");
        return false;
    }
    if (!isSaState()) {
        NETMGR_EXT_LOG_I("Rat is not 5G");
        return false;
    }
    if (isAirPlaneModeOn()) {
        NETMGR_EXT_LOG_I("Air plane mode on");
        return false;
    }
    if (isWifiConnected()) {
        NETMGR_EXT_LOG_I("WiFi is connected");
        return false;
    }
    if (!isScreenOn()) {
        NETMGR_EXT_LOG_I("Out of screen");
        return false;
    }
    if (isInVpnMode()) {
        NETMGR_EXT_LOG_I("VPN on");
        return false;
    }
    if (!isDefaultDataOnMainCard()) {
        NETMGR_EXT_LOG_I("Default data is not main card");
        return false;
    }
    return true;
}

bool NetworkSliceManager::isCanRequestNetwork()
{
    if (isMobileDataClose()) {
        NETMGR_EXT_LOG_I("isMobileDataClose");
        return false;
    }
    if (isAirPlaneModeOn()) {
        NETMGR_EXT_LOG_I("isAirPlaneModeOn");
        return false;
    }
    if (isWifiConnected()) {
        NETMGR_EXT_LOG_I("isWifiConnected");
        return false;
    }
    if (!isSaState()) {
        NETMGR_EXT_LOG_I("!isSaState()");
        return false;
    }
    if (!isDefaultDataOnMainCard()) {
        NETMGR_EXT_LOG_I("!isDefaultDataOnMainCard()");
        return false;
    }
    if (isInVpnMode()) {
        NETMGR_EXT_LOG_I("isInVpnMode");
        return false;
    }
    return true;
}

void NetworkSliceManager::NotifySlicePara(const std::shared_ptr<GetSlicePara>& getSlicePara)
{
    getSlicePara->isDone = true; // Update the status
}

void NetworkSliceManager::SetAppId(AppDescriptor& appDescriptor,
    const std::vector<std::string>& values, const std::string& appId)
{
    if (values.size() == APPID_OSID_SIZE) {
        std::string osId = values[0];
        std::string app = values[1];
        appDescriptor.setOsAppId(osId, app);
    } else {
        appDescriptor.setOsAppId("", appId);
        NETMGR_EXT_LOG_E("osid is empty");
    }
}

bool NetworkSliceManager::GetAppDescriptor(std::map<std::string, std::string>& data, AppDescriptor& appDescriptor)
{
    if (data.empty()) {
        return false;
    }
    if (data.find("appId") != data.end()) {
        std::string appId = data["appId"];
        std::vector<std::string> values;
        values = Split(appId, "#");
        if (values.size() > SIZE_TWO || appId.empty()) {
            return false;
        }
        SetAppId(appDescriptor, values, appId);
    }
    if (data.find("dnn") != data.end()) {
        appDescriptor.setDnn(data["dnn"]);
    }
    if (data.find("fqdn") != data.end()) {
        appDescriptor.setFqdn(data["fqdn"]);
    }
    if (data.find("ip") != data.end()) {
        std::string ipAddr = data["ip"];
        if (ipAddr.size() == NetworkSliceCommConfig::LEN_INT) {
            uint32_t ipv4Address = inet_addr(ipAddr.c_str());
            appDescriptor.setIpv4Addr(ipv4Address);
        } else if (ipAddr.size() == NetworkSliceCommConfig::LEN_SIXTEEN_BYTE) {
            struct in6_addr ipv6;
            if (inet_pton(AF_INET6, ipAddr.c_str(), &ipv6) == 0) {
                NETMGR_EXT_LOG_E("ipv6 ==null || ipv6.length != NetworkSliceCommConfig::LEN_INT");
                return false;
            }
            appDescriptor.setIpv6Addr(*reinterpret_cast<std::array<uint8_t, BASE_16>*>(ipv6.s6_addr));
        } else {
            NETMGR_EXT_LOG_E("wrong ipAddr.length = %{public}d", (int)ipAddr.size());
        }
    }
    if (data.find("protocolId") != data.end()) {
        std::string protocolId = data["protocolId"];
        if (!protocolId.empty()) {
            appDescriptor.setProtocolId(std::stoi(protocolId));
        }
    }
    if (data.find("remotePort") != data.end()) {
        std::string remotePort = data["remotePort"];
        if (!remotePort.empty()) {
            appDescriptor.setRemotePort(std::stoi(remotePort));
        }
    }
    DumpAppDescriptor(appDescriptor);
    return true;
}

void NetworkSliceManager::FillRouteSelectionDescriptor(std::map<std::string, std::string>& ret,
    SelectedRouteDescriptor routeRule)
{
    NETMGR_EXT_LOG_I("FillRouteSelectionDescriptor");
    ret["sscMode"] = std::to_string(routeRule.getSscMode());
    NETMGR_EXT_LOG_I("ret[sscMode] = %{public}s", ret["sscMode"].c_str());
    ret["sNssai"] = routeRule.getSnssai();
    NETMGR_EXT_LOG_I("ret[sNssai] = %{public}s", ret["sNssai"].c_str());
    if (routeRule.getPduSessionType() != -1) {
        ret["pduSessionType"] = std::to_string(routeRule.getPduSessionType());
        NETMGR_EXT_LOG_I("ret[pduSessionType] = %{public}s", ret["pduSessionType"].c_str());
    }
    if (routeRule.getDnn().length() != 0) {
        ret["dnn"] = routeRule.getDnn();
        NETMGR_EXT_LOG_I("ret[dnn] = %{public}s", ret["dnn"].c_str());
    }
    ret["routeBitmap"] = std::to_string(routeRule.getRouteBitmap());
    NETMGR_EXT_LOG_I("ret[routeBitmap] = %{public}s", ret["routeBitmap"].c_str());
    ret["urspPrecedence"] = std::to_string(routeRule.getUrspPrecedence());
    NETMGR_EXT_LOG_I("ret[urspPrecedence] = %{public}s", ret["urspPrecedence"].c_str());
    if (routeRule.getAppIds().length() != 0) {
        ret["appIds"] = routeRule.getAppIds();
        NETMGR_EXT_LOG_I("ret[appIds] = %{public}s", ret["appIds"].c_str());
    }
    if (routeRule.getProtocolIds().length() != 0) {
        ret["protocolIds"] = routeRule.getProtocolIds();
        NETMGR_EXT_LOG_I("ret[protocolIds] = %{public}s", ret["protocolIds"].c_str());
    }
    if (routeRule.getRemotePorts().length() != 0) {
        ret["remotePorts"] = routeRule.getRemotePorts();
        NETMGR_EXT_LOG_I("ret[remotePorts] = %{public}s", ret["remotePorts"].c_str());
    }
    if (routeRule.getIpv4Num() != 0) {
        ret["ipv4Num"] = std::to_string(routeRule.getIpv4Num());
        NETMGR_EXT_LOG_I("ret[ipv4Num] = %{public}s", ret["ipv4Num"].c_str());
        std::vector<uint8_t> Ipv4AddrAndMask = routeRule.getIpv4AddrAndMask();
        ret["ipv4AddrAndMask"] = std::string(reinterpret_cast<const char*>(Ipv4AddrAndMask.data()),
            Ipv4AddrAndMask.size());
    }
    if (routeRule.getIpv6Num() != 0) {
        ret["ipv6Num"] = std::to_string(routeRule.getIpv6Num());
        NETMGR_EXT_LOG_I("ret[ipv6Num] = %{public}s", ret["ipv6Num"].c_str());
        std::vector<uint8_t> Ipv6AddrAndPrefix = routeRule.getIpv6AddrAndPrefix();
        ret["ipv6AddrAndPrefix"] = std::string(reinterpret_cast<const char*>(Ipv6AddrAndPrefix.data()),
            Ipv6AddrAndPrefix.size());
    }
}

bool NetworkSliceManager::hasAvailableUrspRule()
{
    if (sUrspConfig_ == nullptr) {
        return false;
    }
    return false;
}

bool NetworkSliceManager::isSaState()
{
    return isSaState_;
}

void NetworkSliceManager::SetSaState(bool isSaState)
{
    isSaState_ = isSaState;
}

bool NetworkSliceManager::isNrSlicesSupported()
{
    bool isSupportNrSlice = system::GetBoolParameter(NETMANAGER_EXT_NETWORKSLICE_ABILITY, false);
    return isSupportNrSlice;
}

bool NetworkSliceManager::isDefaultDataOnMainCard()
{
    bool isDefaultDataOnMainCard
        = (StateUtils::GetDefaultSlotId() == StateUtils::GetPrimarySlotId()) ? true : false;
    return isDefaultDataOnMainCard;
}

bool NetworkSliceManager::isWifiConnected()
{
    return wifiConn_;
}

bool NetworkSliceManager::isScreenOn()
{
    return screenOn_;
}

bool NetworkSliceManager::isAirPlaneModeOn()
{
    return airModeOn_;
}

bool NetworkSliceManager::isInVpnMode()
{
    return vpnMode_;
}

bool NetworkSliceManager::isMobileDataClose()
{
    bool dataEnabled = false;
    Telephony::CellularDataClient::GetInstance().IsCellularDataEnabled(dataEnabled);
    if (dataEnabled) {
        return false;
    }
    return true;
}

int32_t NetworkSliceManager::GetForeGroundAppUid()
{
    return foregroundApp_uid;
}

bool NetworkSliceManager::isRouteRuleInForbiddenList(const SelectedRouteDescriptor& routeRule)
{
    for (size_t i = 0; i < mNormalForbiddenRules.size(); i++) {
        ForbiddenRouteDescriptor forbiddenRule = mNormalForbiddenRules[i];
        if (routeRule.getDnn() == forbiddenRule.getDnn()
            && routeRule.getSnssai() == forbiddenRule.getSnssai()
            && (routeRule.getPduSessionType() == forbiddenRule.getPduSessionType())
            && (routeRule.getSscMode() == forbiddenRule.getSscMode())) {
            return true;
        }
    }
    return false;
}

void NetworkSliceManager::GetRouteSelectionDescriptorByDNN(const std::string dnn, std::string& snssai, uint8_t& sscmode)
{
    NETMGR_EXT_LOG_I("GetRouteSelectionDescriptorByDNN");
    if (!isMeetNetworkSliceConditions()) {
        return;
    }
    AppDescriptor appDescriptor;
    appDescriptor.setDnn(dnn);
    if (sUrspConfig_ == nullptr || sNrUnsolicitedMsgParser_ == nullptr) {
        NETMGR_EXT_LOG_E("GetRouteSelectionDescriptorByDNN fail, params null");
        return;
    }
    std::string plmn = sNrUnsolicitedMsgParser_->GetHplmn();
    NETMGR_EXT_LOG_I("GetRouteSelectionDescriptorByDNN, plmn = %{public}s", plmn.c_str());
    SelectedRouteDescriptor routeRule;
    if (!sUrspConfig_->SliceNetworkSelection(routeRule, plmn, appDescriptor)) {
        NETMGR_EXT_LOG_E("GetRouteSelectionDescriptorByDNN fail, routeRule = null");
        return;
    }
    sscmode = routeRule.getSscMode();
    snssai = routeRule.getSnssai();
}

void NetworkSliceManager::DumpAppDescriptor(AppDescriptor appDescriptor)
{
    NETMGR_EXT_LOG_I("dump AppDescriptor");
    uint32_t ipv4Addr = appDescriptor.getIpv4Addr();
    std::array<uint8_t, NetworkSliceCommConfig::LEN_IPV6ADDR> ipv6Addr = appDescriptor.getIpv6Addr();
    NETMGR_EXT_LOG_I("mAppDescriptor.mUid = %{public}d", appDescriptor.getUid());
    NETMGR_EXT_LOG_I("mAppDescriptor.mOsAppId.mAppId = %{public}s", appDescriptor.getOsAppId().getAppId().c_str());
    NETMGR_EXT_LOG_I("mAppDescriptor.mOsAppId.mOsId = %{public}s", appDescriptor.getOsAppId().getOsId().c_str());
    std::string ipv6 = transIpv6AddrToStr(appDescriptor.getIpv6Addr());
    NETMGR_EXT_LOG_I("mAppDescriptor.mIpv4Addr = %{public}d, mAppDescriptor.mIpv6Addr = %{public}s",
        appDescriptor.getIpv4Addr(), ipv6.c_str());
    NETMGR_EXT_LOG_I("mAppDescriptor.mProtocolId = %{public}d", appDescriptor.getProtocolId());
    NETMGR_EXT_LOG_I("mAppDescriptor.mRemotePort = %{public}d", appDescriptor.getRemotePort());
    NETMGR_EXT_LOG_I("mAppDescriptor.mDnn = %{public}s", appDescriptor.getDnn().c_str());
    NETMGR_EXT_LOG_I("mAppDescriptor.mFqdn = %{public}s", appDescriptor.getFqdn().c_str());
}
void NetworkSliceManager::DumpSelectedRouteDescriptor(SelectedRouteDescriptor routeRule)
{
    NETMGR_EXT_LOG_I("dump SelectedRouteDescriptor");
    NETMGR_EXT_LOG_I("routeRule.mSscMode = %{public}d, routeRule.mSNssai = %{public}s",
        routeRule.getSscMode(), routeRule.getSnssai().c_str());
    NETMGR_EXT_LOG_I("routeRule.mDnn = %{public}s, routeRule.mPduSessionType = %{public}d",
        routeRule.getDnn().c_str(), routeRule.getPduSessionType());
}

}
}
