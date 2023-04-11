/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "netshare_client_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <new>
#include <string>

#include <securec.h>

#include "accesstoken_kit.h"
#include "refbase.h"
#include "singleton.h"
#include "token_setproc.h"
#include "wifi_ap_msg.h"

#include "i_networkshare_service.h"
#include "interface_configuration.h"
#include "net_manager_ext_constants.h"
#include "netmgr_ext_log_wrapper.h"
#define private public
#include "networkshare_service.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const uint8_t *g_baseFuzzData = nullptr;
static constexpr uint32_t CREATE_SHARE_IFACE_TYPE_VALUE = 3;
static constexpr uint32_t CREATE_SHARE_IFACE_STATE_VALUE = 3;
static constexpr uint32_t ENUM_TYPE_VALUE3 = 3;
static constexpr uint32_t ENUM_TYPE_VALUE4 = 4;
static constexpr uint32_t ENUM_TYPE_VALUE5 = 5;
static constexpr uint32_t ENUM_TYPE_VALUE6 = 6;
static constexpr uint32_t ENUM_TYPE_BEGIN = 1;
size_t g_baseFuzzSize = 0;
size_t g_baseFuzzPos;
constexpr size_t IFACE_LEN = 5;

using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;
HapInfoParams testInfoParms = {.userID = 1,
                               .bundleName = "netshare_client_fuzzer",
                               .instIndex = 0,
                               .appIDDesc = "test"};

PermissionDef testPermDef = {.permissionName = "ohos.permission.CONNECTIVITY_INTERNAL",
                             .bundleName = "netshare_client_fuzzer",
                             .grantMode = 1,
                             .availableLevel = OHOS::Security::AccessToken::ATokenAplEnum::APL_SYSTEM_BASIC,
                             .label = "label",
                             .labelId = 1,
                             .description = "Test netshare maneger network info",
                             .descriptionId = 1};

PermissionDef testInternetPermDef = {.permissionName = "ohos.permission.CONNECTIVITY_INTERNAL",
                                     .bundleName = "net_conn_client_fuzzer",
                                     .grantMode = 1,
                                     .availableLevel = OHOS::Security::AccessToken::ATokenAplEnum::APL_SYSTEM_BASIC,
                                     .label = "label",
                                     .labelId = 1,
                                     .description = "Test netshare connectivity internet",
                                     .descriptionId = 1};

PermissionStateFull testState = {.permissionName = "ohos.permission.CONNECTIVITY_INTERNAL",
                                 .isGeneral = true,
                                 .resDeviceID = {"local"},
                                 .grantStatus = {PermissionState::PERMISSION_GRANTED},
                                 .grantFlags = {2}};

PermissionStateFull testInternetState = {.permissionName = "ohos.permission.CONNECTIVITY_INTERNAL",
                                         .isGeneral = true,
                                         .resDeviceID = {"local"},
                                         .grantStatus = {PermissionState::PERMISSION_GRANTED},
                                         .grantFlags = {2}};

HapPolicyParams testPolicyPrams = {.apl = APL_SYSTEM_BASIC,
                                   .domain = "test.domain",
                                   .permList = {testPermDef},
                                   .permStateList = {testState}};

HapPolicyParams testInternetPolicyPrams = {.apl = APL_SYSTEM_BASIC,
                                           .domain = "test.domain",
                                           .permList = {testPermDef, testInternetPermDef},
                                           .permStateList = {testState, testInternetState}};
} // namespace

template <class T> T GetData()
{
    T object{};
    size_t objectSize = sizeof(object);
    if (g_baseFuzzData == nullptr || objectSize > g_baseFuzzSize - g_baseFuzzPos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, g_baseFuzzData + g_baseFuzzPos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_baseFuzzPos += objectSize;
    return object;
}
class AccessToken {
public:
    AccessToken() : currentID_(GetSelfTokenID())
    {
        AccessTokenIDEx tokenIdEx = AccessTokenKit::AllocHapToken(testInfoParms, testPolicyPrams);
        accessID_ = tokenIdEx.tokenIdExStruct.tokenID;
        SetSelfTokenID(accessID_);
    }
    ~AccessToken()
    {
        AccessTokenKit::DeleteToken(accessID_);
        SetSelfTokenID(currentID_);
    }

private:
    AccessTokenID currentID_;
    AccessTokenID accessID_ = 0;
};

class AccessTokenInternetInfo {
public:
    AccessTokenInternetInfo() : currentID_(GetSelfTokenID())
    {
        AccessTokenIDEx tokenIdEx = AccessTokenKit::AllocHapToken(testInfoParms, testInternetPolicyPrams);
        accessID_ = tokenIdEx.tokenIdExStruct.tokenID;
        SetSelfTokenID(accessID_);
    }
    ~AccessTokenInternetInfo()
    {
        AccessTokenKit::DeleteToken(accessID_);
        SetSelfTokenID(currentID_);
    }

private:
    AccessTokenID currentID_;
    AccessTokenID accessID_ = 0;
};

std::string GetStringFromData(int strlen)
{
    char cstr[strlen];
    cstr[strlen - 1] = '\0';
    for (int i = 0; i < strlen - 1; i++) {
        cstr[i] = GetData<char>();
    }
    std::string str(cstr);
    return str;
}

bool InitGlobalData(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return false;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    return true;
}

class INetShareCallbackTest : public IRemoteStub<ISharingEventCallback> {
public:
    void OnSharingStateChanged(const bool &isRunning)
    {
        return;
    }
    void OnInterfaceSharingStateChanged(const SharingIfaceType &type, const std::string &iface,
                                        const SharingIfaceState &state)
    {
        return;
    }
    void OnSharingUpstreamChanged(const sptr<NetHandle> netHandle)
    {
        return;
    }
};

class SubStateMachineCallbackTest : public NetworkShareSubStateMachine::SubStateMachineCallback {
    void OnUpdateInterfaceState(const std::shared_ptr<NetworkShareSubStateMachine> &paraSubStateMachine, int state,
                                int lastError)
    {
    }
};

class NotifyUpstreamCallbackTest : public NetworkShareUpstreamMonitor::NotifyUpstreamCallback {
    void OnUpstreamStateChanged(int32_t msgName, int32_t param1) {}
    void OnUpstreamStateChanged(int32_t msgName, int32_t param1, int32_t param2, const std::any &messageObj) {}
};

static bool g_isInited = false;
void Init()
{
    if (!g_isInited) {
        DelayedSingleton<NetworkShareService>::GetInstance()->Init();
        g_isInited = true;
    }
}

int32_t OnRemoteRequest(INetworkShareService::MessageCode code, MessageParcel &data)
{
    if (!g_isInited) {
        Init();
    }

    MessageParcel reply;
    MessageOption option;

    return DelayedSingleton<NetworkShareService>::GetInstance()->OnRemoteRequest(static_cast<uint32_t>(code), data,
                                                                                 reply, option);
}

bool WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(NetworkShareServiceStub::GetDescriptor())) {
        return false;
    }
    return true;
}

void IsNetworkSharingSupportedFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("IsNetworkSharingSupportedFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }
    AccessTokenInternetInfo tokenInfo;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    OnRemoteRequest(INetworkShareService::MessageCode::CMD_GET_SHARING_SUPPORTED, dataParcel);
}

void IsSharingFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("IsSharingFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }

    AccessToken token;
    AccessTokenInternetInfo tokenInfo;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    OnRemoteRequest(INetworkShareService::MessageCode::CMD_GET_IS_SHARING, dataParcel);
}

void StartNetworkSharingFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("StartNetworkSharingFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }

    AccessToken token;
    AccessTokenInternetInfo tokenInfo;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    int32_t type = GetData<int32_t>() % CREATE_SHARE_IFACE_TYPE_VALUE;
    dataParcel.WriteInt32(type);
    OnRemoteRequest(INetworkShareService::MessageCode::CMD_START_NETWORKSHARE, dataParcel);
}

void StopNetworkSharingFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("StopNetworkSharingFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }

    AccessToken token;
    AccessTokenInternetInfo tokenInfo;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    int32_t type = GetData<int32_t>() % CREATE_SHARE_IFACE_TYPE_VALUE;
    dataParcel.WriteInt32(type);
    OnRemoteRequest(INetworkShareService::MessageCode::CMD_STOP_NETWORKSHARE, dataParcel);
}

void GetSharableRegexsFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("GetSharableRegexsFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }

    AccessToken token;
    AccessTokenInternetInfo tokenInfo;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    int32_t type = GetData<int32_t>() % CREATE_SHARE_IFACE_TYPE_VALUE;
    dataParcel.WriteInt32(type);
    OnRemoteRequest(INetworkShareService::MessageCode::CMD_GET_SHARABLE_REGEXS, dataParcel);
}

void GetSharingStateFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("GetSharingStateFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }

    AccessToken token;
    AccessTokenInternetInfo tokenInfo;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    int32_t type = GetData<int32_t>() % CREATE_SHARE_IFACE_TYPE_VALUE;
    dataParcel.WriteInt32(type);
    OnRemoteRequest(INetworkShareService::MessageCode::CMD_GET_SHARING_STATE, dataParcel);
}

void GetNetSharingIfacesFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("GetNetSharingIfacesFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }

    AccessToken token;
    AccessTokenInternetInfo tokenInfo;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    uint32_t state = GetData<int32_t>() % CREATE_SHARE_IFACE_STATE_VALUE + ENUM_TYPE_BEGIN;
    dataParcel.WriteInt32(state);
    OnRemoteRequest(INetworkShareService::MessageCode::CMD_GET_SHARING_IFACES, dataParcel);
}

void RegisterSharingEventFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("RegisterSharingEventFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }

    AccessToken token;
    AccessTokenInternetInfo tokenInfo;

    sptr<ISharingEventCallback> callback = new (std::nothrow) INetShareCallbackTest();
    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteRemoteObject(callback->AsObject());
    OnRemoteRequest(INetworkShareService::MessageCode::CMD_REGISTER_EVENT_CALLBACK, dataParcel);
}

void UnregisterSharingEventFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("UnregisterSharingEventFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }

    AccessToken token;
    AccessTokenInternetInfo tokenInfo;

    sptr<ISharingEventCallback> callback = new (std::nothrow) INetShareCallbackTest();
    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteRemoteObject(callback->AsObject());
    OnRemoteRequest(INetworkShareService::MessageCode::CMD_UNREGISTER_EVENT_CALLBACK, dataParcel);
}

void GetStatsRxBytesFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("GetStatsRxBytesFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }

    AccessToken token;
    AccessTokenInternetInfo tokenInfo;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    OnRemoteRequest(INetworkShareService::MessageCode::CMD_GET_RX_BYTES, dataParcel);
}

void GetStatsTxBytesFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("GetStatsTxBytesFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }

    AccessToken token;
    AccessTokenInternetInfo tokenInfo;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    OnRemoteRequest(INetworkShareService::MessageCode::CMD_GET_TX_BYTES, dataParcel);
}

void GetStatsTotalBytesFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("GetStatsTotalBytesFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }

    AccessToken token;
    AccessTokenInternetInfo tokenInfo;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    OnRemoteRequest(INetworkShareService::MessageCode::CMD_GET_TOTAL_BYTES, dataParcel);
}

void NetworkShareMainStateMachineFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("NetworkShareMainStateMachineFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }

    std::shared_ptr<NetworkShareUpstreamMonitor> networkmonitor =
        DelayedSingleton<NetworkShareUpstreamMonitor>::GetInstance();

    auto networkShareMainStateMachine = std::make_unique<NetworkShareMainStateMachine>(networkmonitor);
    int32_t errType = GetData<int32_t>();
    networkShareMainStateMachine->SwitcheToErrorState(errType);
    int32_t newState = GetData<int32_t>();
    networkShareMainStateMachine->MainSmStateSwitch(newState);
    int32_t anyNum = GetData<int32_t>();
    networkShareMainStateMachine->TurnOnMainShareSettings();
    networkShareMainStateMachine->TurnOffMainShareSettings();
    MessageIfaceActive messageIfaceActive;
    messageIfaceActive.value_ = anyNum;
    networkShareMainStateMachine->HandleInitInterfaceStateActive(messageIfaceActive);
    networkShareMainStateMachine->HandleInitInterfaceStateInactive(messageIfaceActive);
    auto networkShareMainStateMachine1 = std::make_unique<NetworkShareMainStateMachine>(networkmonitor);
    networkShareMainStateMachine1->HandleAliveInterfaceStateActive(messageIfaceActive);
    networkShareMainStateMachine1->HandleAliveInterfaceStateInactive(messageIfaceActive);
    MessageUpstreamInfo upstreamNetInfo;
    upstreamNetInfo.cmd_ = anyNum;
    auto networkShareMainStateMachine3 = std::make_unique<NetworkShareMainStateMachine>(networkmonitor);
    networkShareMainStateMachine3->HandleAliveUpstreamMonitorCallback(upstreamNetInfo);
    networkShareMainStateMachine3->HandleErrorInterfaceStateInactive(messageIfaceActive);
    networkShareMainStateMachine3->HandleErrorClear(upstreamNetInfo);
    auto networkShareMainStateMachine2 = std::make_unique<NetworkShareMainStateMachine>(networkmonitor);
    networkShareMainStateMachine2->InitStateEnter();
    networkShareMainStateMachine2->AliveStateEnter();
    networkShareMainStateMachine2->ErrorStateEnter();
    networkShareMainStateMachine2->InitStateExit();
    networkShareMainStateMachine2->AliveStateExit();
    networkShareMainStateMachine2->ErrorStateExit();
    networkShareMainStateMachine2->ChooseUpstreamNetwork();
    networkShareMainStateMachine2->DisableForward();
    networkShareMainStateMachine2->EraseSharedSubSM(messageIfaceActive);
}

void NetworkShareSubStateMachineFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("NetworkShareSubStateMachineFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }

    std::shared_ptr<NetworkShareConfiguration> configuration;
    std::string ifaceName = GetStringFromData(IFACE_LEN);
    SharingIfaceType interfaceType = static_cast<SharingIfaceType>(GetData<uint32_t>() % CREATE_SHARE_IFACE_TYPE_VALUE);
    auto networkShareSubStateMachine =
        std::make_unique<NetworkShareSubStateMachine>(ifaceName, interfaceType, configuration);

    networkShareSubStateMachine->GetNetShareType();
    networkShareSubStateMachine->GetInterfaceName();
    int32_t newState = GetData<int32_t>();
    networkShareSubStateMachine->SubSmStateSwitch(newState);
    std::string downIface = GetStringFromData(IFACE_LEN);
    networkShareSubStateMachine->GetDownIfaceName(downIface);
    std::string upIface = GetStringFromData(IFACE_LEN);
    int32_t num = GetData<int32_t>();
    sptr<NetHandle> netHandle = new (std::nothrow) NetHandle(num);
    sptr<NetAllCapabilities> netcap = new (std::nothrow) NetManagerStandard::NetAllCapabilities();
    sptr<NetLinkInfo> netlinkinfo = new (std::nothrow) NetManagerStandard::NetLinkInfo();
    networkShareSubStateMachine->GetUpIfaceName(upIface);
}

void NetworkShareSubStateMachinePrivateFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("NetworkShareSubStateMachinePrivateFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }
    std::shared_ptr<NetworkShareConfiguration> config;
    std::string str = GetStringFromData(IFACE_LEN);
    SharingIfaceType interfaceType = static_cast<SharingIfaceType>(GetData<uint32_t>() % CREATE_SHARE_IFACE_TYPE_VALUE);
    auto networkShareSubStateMachine = std::make_unique<NetworkShareSubStateMachine>(str, interfaceType, config);
    sptr<NetHandle> handle = new (std::nothrow) NetHandle(GetData<int32_t>());
    sptr<NetAllCapabilities> netcap = new (std::nothrow) NetManagerStandard::NetAllCapabilities();
    sptr<NetLinkInfo> link = new (std::nothrow) NetManagerStandard::NetLinkInfo();
    std::shared_ptr<UpstreamNetworkInfo> upstreamNetInfo = std::make_shared<UpstreamNetworkInfo>(handle, netcap, link);
    std::shared_ptr<INetAddr> addr = std::make_shared<INetAddr>();
    addr->family_ = GetData<int8_t>();
    addr->prefixlen_ = GetData<int8_t>();
    addr->address_ = str;
    addr->netMask_ = str;
    addr->hostName_ = str;
    addr->port_ = GetData<int8_t>();
    networkShareSubStateMachine->CreateInitStateTable();
    networkShareSubStateMachine->CreateSharedStateTable();
    networkShareSubStateMachine->InitStateEnter();
    networkShareSubStateMachine->SharedStateEnter();
    networkShareSubStateMachine->UnavailableStateEnter();
    networkShareSubStateMachine->InitStateExit();
    networkShareSubStateMachine->SharedStateExit();
    networkShareSubStateMachine->UnavailableStateExit();
    networkShareSubStateMachine->HandleInitSharingRequest(upstreamNetInfo);
    networkShareSubStateMachine->HandleInitInterfaceDown(upstreamNetInfo);
    networkShareSubStateMachine->HandleSharedUnrequest(upstreamNetInfo);
    networkShareSubStateMachine->HandleSharedInterfaceDown(upstreamNetInfo);
    networkShareSubStateMachine->HandleSharedConnectionChange(upstreamNetInfo);
    networkShareSubStateMachine->HandleSharedErrors(upstreamNetInfo);
    networkShareSubStateMachine->ConfigureShareDhcp(GetData<int32_t>() > 0);
    networkShareSubStateMachine->RequestIpv4Address(addr);
    networkShareSubStateMachine->StartDhcp(addr);
    networkShareSubStateMachine->StopDhcp();
    networkShareSubStateMachine->HandleConnectionChanged(upstreamNetInfo);
    networkShareSubStateMachine->HandleConnection();
    networkShareSubStateMachine->RemoveRoutesToLocalNetwork();
    networkShareSubStateMachine->AddRoutesToLocalNetwork();
    networkShareSubStateMachine->CleanupUpstreamInterface();
    networkShareSubStateMachine->HasChangeUpstreamIfaceSet(str);
    networkShareSubStateMachine->GetWifiHotspotDhcpFlag();
    networkShareSubStateMachine->GetBtDestinationAddr(str);
    networkShareSubStateMachine->GetWifiApDestinationAddr(str);
    networkShareSubStateMachine->GetUsbDestinationAddr(str);
    networkShareSubStateMachine->CheckConfig(str, str);
    networkShareSubStateMachine->FindDestinationAddr(str);
}

void UpstreammonitorFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("UpstreammonitorFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }

    int32_t num = GetData<int32_t>();
    std::shared_ptr<NetworkShareUpstreamMonitor> networkmonitor =
        DelayedSingleton<NetworkShareUpstreamMonitor>::GetInstance();

    networkmonitor->ListenDefaultNetwork();
    sptr<NetHandle> netHandle = new (std::nothrow) NetHandle(num);
    sptr<NetAllCapabilities> netcap = new (std::nothrow) NetManagerStandard::NetAllCapabilities();
    sptr<NetLinkInfo> netlinkinfo = new (std::nothrow) NetManagerStandard::NetLinkInfo();
    std::shared_ptr<UpstreamNetworkInfo> upstreamNetInfo =
        std::make_shared<UpstreamNetworkInfo>(netHandle, netcap, netlinkinfo);
    networkmonitor->GetCurrentGoodUpstream(upstreamNetInfo);

    networkmonitor->NotifyMainStateMachine(num, upstreamNetInfo);
    networkmonitor->NotifyMainStateMachine(num);
    networkmonitor->HandleNetAvailable(netHandle);
    networkmonitor->HandleNetCapabilitiesChange(netHandle, netcap);
    networkmonitor->HandleConnectionPropertiesChange(netHandle, netlinkinfo);
    networkmonitor->HandleNetLost(netHandle);
}

void NetworkShareTrackerFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("NetworkShareTrackerFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }

    int32_t num = GetData<int32_t>();
    std::string iface = GetStringFromData(IFACE_LEN);
    sptr<NetHandle> netHandle = new (std::nothrow) NetHandle(num);
    sptr<NetAllCapabilities> netcap = new (std::nothrow) NetManagerStandard::NetAllCapabilities();
    sptr<NetLinkInfo> netlinkinfo = new (std::nothrow) NetManagerStandard::NetLinkInfo();
    SharingIfaceType ifaceType = SharingIfaceType(SharingIfaceType(num % ENUM_TYPE_VALUE3));
    SharingIfaceState ifaceState = SharingIfaceState((num % ENUM_TYPE_VALUE3) + ENUM_TYPE_BEGIN);
    TrafficType trafficTYpe = TrafficType((num % ENUM_TYPE_VALUE3) + ENUM_TYPE_BEGIN);
    int32_t supported = 0;
    int32_t sharingStatus = 0;
    std::vector<std::string> ifaceRegexs;
    std::vector<std::string> ifaces;
    int32_t kbByte = 0;
    NetworkShareTracker::GetInstance().Init();
    NetworkShareTracker::GetInstance().Uninit();
    NetworkShareTracker::GetInstance().IsNetworkSharingSupported(supported);
    NetworkShareTracker::GetInstance().IsSharing(sharingStatus);
    NetworkShareTracker::GetInstance().StartNetworkSharing(ifaceType);
    NetworkShareTracker::GetInstance().StopNetworkSharing(ifaceType);
    NetworkShareTracker::GetInstance().GetSharableRegexs(ifaceType, ifaceRegexs);
    NetworkShareTracker::GetInstance().GetSharingState(ifaceType, ifaceState);
    NetworkShareTracker::GetInstance().GetNetSharingIfaces(ifaceState, ifaces);
    sptr<INetShareCallbackTest> callback = new (std::nothrow) INetShareCallbackTest();
    NetworkShareTracker::GetInstance().RegisterSharingEvent(callback);
    NetworkShareTracker::GetInstance().UnregisterSharingEvent(callback);
    NetworkShareTracker::GetInstance().UpstreamWanted();
    std::shared_ptr<NetworkShareSubStateMachine> subSm =
        std::make_shared<NetworkShareSubStateMachine>(iface, ifaceType, nullptr);
    NetworkShareTracker::GetInstance().ModifySharedSubStateMachineList(num > 0, subSm);
    NetworkShareTracker::GetInstance().GetMainStateMachine();
    std::shared_ptr<UpstreamNetworkInfo> upstreamNetInfo =
        std::make_shared<UpstreamNetworkInfo>(netHandle, netcap, netlinkinfo);
    NetworkShareTracker::GetInstance().SetUpstreamNetHandle(upstreamNetInfo);
    NetworkShareTracker::GetInstance().GetUpstreamInfo(upstreamNetInfo);
    NetworkShareTracker::GetInstance().NotifyDownstreamsHasNewUpstreamIface(upstreamNetInfo);
    NetworkShareTracker::GetInstance().GetSharedSubSMTraffic(trafficTYpe, kbByte);
}

void NetworkShareTrackerPrivateFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("NetworkShareTrackerPrivateFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }

    int32_t num = GetData<int32_t>();
    std::string iface = GetStringFromData(IFACE_LEN);
    sptr<NetHandle> netHandle = new (std::nothrow) NetHandle(num);
    SharingIfaceType ifaceType = SharingIfaceType(SharingIfaceType(num % ENUM_TYPE_VALUE3));
    std::shared_ptr<NetworkShareSubStateMachine> subSm =
        std::make_shared<NetworkShareSubStateMachine>(iface, ifaceType, nullptr);
    SharingIfaceState ifaceState = SharingIfaceState((num % ENUM_TYPE_VALUE3) + ENUM_TYPE_BEGIN);

    auto &tra = NetworkShareTracker::GetInstance();
    tra.HandleSubSmUpdateInterfaceState(subSm, num, num);
    tra.EnableNetSharingInternal(ifaceType, num > 0);
    tra.SetWifiNetworkSharing(num > 0);
    tra.SetUsbNetworkSharing(num > 0);
    tra.SetBluetoothNetworkSharing(num > 0);
    tra.EnableWifiSubStateMachine();
    tra.EnableBluetoothSubStateMachine();
    tra.Sharing(iface, num);
    tra.SendGlobalSharingStateChange();
    tra.SendIfaceSharingStateChange(ifaceType, iface, ifaceState);
    tra.SendSharingUpstreamChange(netHandle);
    tra.CreateSubStateMachine(iface, ifaceType, num > 0);
    tra.StopSubStateMachine(iface, ifaceType);
    tra.IsInterfaceMatchType(iface, ifaceType);
    tra.InterfaceNameToType(iface, ifaceType);
    tra.IsHandleNetlinkEvent(ifaceType, num > 0);
    tra.FindSubStateMachine(iface, ifaceType, subSm, iface);
    tra.InterfaceAdded(iface);
    tra.InterfaceRemoved(iface);
    tra.InterfaceStatusChanged(iface, num > 0);
    tra.SetDnsForwarders(*netHandle);
    tra.StopDnsProxy();
    tra.SubSmStateToExportState(num);
    tra.RegisterWifiApCallback();
    tra.RegisterBtPanCallback();
    tra.SetWifiState(Wifi::ApState(num % ENUM_TYPE_VALUE6));
#ifdef BLUETOOTH_MODOULE
    tra.SetBluetoothState(Bluetooth::BTConnectState(num % ENUM_TYPE_VALUE4));
#endif
    tra.SendMainSMEvent(subSm, num, num);
}

void NetworkShareHisysEventFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("NetworkShareHisysEventFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }

    int32_t num = GetData<int32_t>();
    std::string str = GetStringFromData(IFACE_LEN);
    SharingIfaceType ifaceType = SharingIfaceType(SharingIfaceType(num % ENUM_TYPE_VALUE3));
    NetworkShareEventType eventType = NetworkShareEventType(num % ENUM_TYPE_VALUE3);
    NetworkShareEventOperator eventOper = NetworkShareEventOperator(num % ENUM_TYPE_VALUE5);
    auto &hisys = NetworkShareHisysEvent::GetInstance();
    hisys.SendFaultEvent(eventOper, NetworkShareEventErrorType::ERROR_CANCEL_FORWARD, str, eventType);
    hisys.SendFaultEvent(ifaceType, eventOper, NetworkShareEventErrorType::ERROR_CANCEL_FORWARD, str, eventType);
    hisys.SendBehaviorEvent(num, ifaceType);
}

void NetworkShareConfigurationFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("NetworkShareConfigurationFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }

    uint32_t vectorSize = GetData<uint32_t>() % 10;
    std::string str = GetStringFromData(IFACE_LEN);
    std::vector<std::string> match;
    match.reserve(vectorSize);
    for (size_t i = 0; i < vectorSize; i++) {
        match.emplace_back(str);
    }
    NetworkShareConfiguration config;
    config.IsNetworkSharingSupported();
    config.IsUsbIface(str);
    config.IsWifiIface(str);
    config.IsBluetoothIface(str);
    config.GetUsbIfaceRegexs();
    config.GetWifiIfaceRegexs();
    config.GetBluetoothIfaceRegexs();
    config.GetWifiHotspotSetDhcpFlag();
    config.GetBtpanIpv4Addr();
    config.GetWifiHotspotIpv4Addr();
    config.GetUsbRndisIpv4Addr();
    config.GetRouteSuffix();
    config.GetBtpanDhcpServerName();
    config.GetWifiHotspotDhcpServerName();
    config.GetUsbRndisDhcpServerName();
    config.GetDefaultMask();
    config.GetDhcpEndIP();

    config.LoadConfigData();
    config.MatchesDownstreamRegexs(str, match);
    config.ReadConfigFile();
    config.ParseLineData(str, str);
    std::vector<std::string> res;
    config.ParseRegexsData(res, str);
}
} // namespace NetManagerStandard
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::NetManagerStandard::IsNetworkSharingSupportedFuzzTest(data, size);
    OHOS::NetManagerStandard::IsSharingFuzzTest(data, size);
    OHOS::NetManagerStandard::StartNetworkSharingFuzzTest(data, size);
    OHOS::NetManagerStandard::StopNetworkSharingFuzzTest(data, size);
    OHOS::NetManagerStandard::GetSharableRegexsFuzzTest(data, size);
    OHOS::NetManagerStandard::GetSharingStateFuzzTest(data, size);
    OHOS::NetManagerStandard::GetNetSharingIfacesFuzzTest(data, size);
    OHOS::NetManagerStandard::RegisterSharingEventFuzzTest(data, size);
    OHOS::NetManagerStandard::UnregisterSharingEventFuzzTest(data, size);
    OHOS::NetManagerStandard::GetStatsRxBytesFuzzTest(data, size);
    OHOS::NetManagerStandard::GetStatsTxBytesFuzzTest(data, size);
    OHOS::NetManagerStandard::GetStatsTotalBytesFuzzTest(data, size);
    OHOS::NetManagerStandard::NetworkShareConfigurationFuzzTest(data, size);
    OHOS::NetManagerStandard::UpstreammonitorFuzzTest(data, size);
    OHOS::NetManagerStandard::NetworkShareTrackerFuzzTest(data, size);
    OHOS::NetManagerStandard::NetworkShareTrackerPrivateFuzzTest(data, size);
    OHOS::NetManagerStandard::NetworkShareSubStateMachineFuzzTest(data, size);
    OHOS::NetManagerStandard::NetworkShareSubStateMachinePrivateFuzzTest(data, size);
    OHOS::NetManagerStandard::NetworkShareMainStateMachineFuzzTest(data, size);
    OHOS::NetManagerStandard::NetworkShareHisysEventFuzzTest(data, size);
    return 0;
}
