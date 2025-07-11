/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "ethernet_client_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <new>
#include <securec.h>
#include <string>

#include "mac_address_info.h"
#include "netmanager_ext_test_security.h"
#include "refbase.h"
#include "singleton.h"

#include "dev_interface_state.h"
#define private public
#include "ethernet_client.h"
#include "ethernet_service.h"
#include "interface_configuration.h"
#include "interface_state_callback_stub.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const uint8_t *g_baseFuzzData = nullptr;
static constexpr uint32_t CREATE_BOOL_TYPE_VALUE = 2;
size_t g_baseFuzzSize = 0;
size_t g_baseFuzzPos;
constexpr size_t IFACE_LEN = 5;
constexpr size_t NET_TYPE_LEN = 1;
constexpr size_t REG_CMD_LEN = 1024;
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

class MonitorInterfaceStateCallback : public InterfaceStateCallbackStub {
public:
    int32_t OnInterfaceAdded(const std::string &ifName) override
    {
        return 0;
    }

    int32_t OnInterfaceRemoved(const std::string &ifName) override
    {
        return 0;
    }

    int32_t OnInterfaceChanged(const std::string &ifName, bool up) override
    {
        return 0;
    }
};

class NetRegisterEapCallbackTest : public NetRegisterEapCallbackStub {
public:
    int32_t OnRegisterCustomEapCallback(const std::string &regCmd) override
    {
        return 0;
    }
    int32_t OnReplyCustomEapDataEvent(int result, const sptr<EapData> &eapData) override
    {
        return 0;
    }
};
 
class NetEapPostBackCallbackTest : public NetEapPostbackCallbackStub {
public:
    int32_t OnEapSupplicantPostback(NetType netType, const sptr<EapData> &eapData) override
    {
        return 0;
    }
};
 
static inline sptr<INetRegisterEapCallback> g_registerEapCallback = new (std::nothrow) NetRegisterEapCallbackTest();
static inline sptr<INetEapPostbackCallback> g_eapPostbackCallback = new (std::nothrow) NetEapPostBackCallbackTest();
 
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

static bool g_isInited = false;

void Init()
{
    if (!g_isInited) {
        DelayedSingleton<EthernetService>::GetInstance()->Init();
        g_isInited = true;
    }
}

bool WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(EthernetServiceStub::GetDescriptor())) {
        return false;
    }
    return true;
}

bool IsDataAndWriteVaild(const uint8_t *data, size_t size, MessageParcel &parcel)
{
    if ((data == nullptr) || (size == 0)) {
        return false;
    }
    NetManagerExtAccessToken token;
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    std::u16string iface = Str8ToStr16(GetStringFromData(IFACE_LEN));
    WriteInterfaceToken(parcel);
    if (!parcel.WriteString16(iface)) {
        return false;
    }

    return true;
}

int32_t OnRemoteRequest(uint32_t code, MessageParcel &data)
{
    if (!g_isInited) {
        Init();
    }

    MessageParcel reply;
    MessageOption option;
    return DelayedSingleton<EthernetService>::GetInstance()->OnRemoteRequest(code, data, reply, option);
}

void GetMacAddressFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    NetManagerExtAccessToken token;
    MessageParcel parcel;
    std::string iface = GetStringFromData(IFACE_LEN);
    WriteInterfaceToken(parcel);
    if (!parcel.WriteString(iface)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(IEthernetServiceIpcCode::COMMAND_GET_MAC_ADDRESS), parcel);
}

void SetIfaceConfigFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel parcel;
    if (!IsDataAndWriteVaild(data, size, parcel)) {
        return;
    }
    sptr<InterfaceConfiguration> ic = new (std::nothrow) InterfaceConfiguration();
    if (!parcel.WriteParcelable(ic)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(IEthernetServiceIpcCode::COMMAND_SET_IFACE_CONFIG), parcel);
}

void GetIfaceConfigFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    NetManagerExtAccessToken token;
    MessageParcel parcel;
    std::string iface = GetStringFromData(IFACE_LEN);
    WriteInterfaceToken(parcel);
    if (!parcel.WriteString(iface)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(IEthernetServiceIpcCode::COMMAND_GET_IFACE_CONFIG), parcel);
}

void IsIfaceActiveFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    NetManagerExtAccessToken token;
    MessageParcel parcel;
    std::string iface = GetStringFromData(IFACE_LEN);
    WriteInterfaceToken(parcel);
    if (!parcel.WriteString(iface)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(IEthernetServiceIpcCode::COMMAND_IS_IFACE_ACTIVE), parcel);
}

void GetAllActiveIfacesFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return;
    }
    NetManagerExtAccessToken token;
    MessageParcel parcel;
    if (!IsDataAndWriteVaild(data, size, parcel)) {
        return;
    }
    WriteInterfaceToken(parcel);
    OnRemoteRequest(static_cast<uint32_t>(IEthernetServiceIpcCode::COMMAND_GET_ALL_ACTIVE_IFACES), parcel);
}

void ResetFactoryFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return;
    }
    NetManagerExtAccessToken token;
    MessageParcel parcel;
    if (!IsDataAndWriteVaild(data, size, parcel)) {
        return;
    }
    WriteInterfaceToken(parcel);
    OnRemoteRequest(static_cast<uint32_t>(IEthernetServiceIpcCode::COMMAND_RESET_FACTORY), parcel);
}

void UnregisterIfacesStateChangedFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    NetManagerExtAccessToken token;
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    sptr<InterfaceStateCallback> interfaceCallback = new (std::nothrow) MonitorInterfaceStateCallback();
    DelayedSingleton<EthernetClient>::GetInstance()->RegisterIfacesStateChanged(interfaceCallback);
    DelayedSingleton<EthernetClient>::GetInstance()->UnregisterIfacesStateChanged(interfaceCallback);
}

void OnRegisterIfacesStateChangedFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return;
    }
    MessageParcel parcel;
    if (WriteInterfaceToken(parcel)) {
        return;
    }
    sptr<InterfaceStateCallback> remote = new (std::nothrow) MonitorInterfaceStateCallback();
    parcel.WriteRemoteObject(remote->AsObject());
    OnRemoteRequest(static_cast<uint32_t>(IEthernetServiceIpcCode::COMMAND_REGISTER_IFACES_STATE_CHANGED), parcel);
    OnRemoteRequest(static_cast<uint32_t>(IEthernetServiceIpcCode::COMMAND_UNREGISTER_IFACES_STATE_CHANGED), parcel);
}

void SetInterfaceUpFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel parcel;
    if (!IsDataAndWriteVaild(data, size, parcel)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(IEthernetServiceIpcCode::COMMAND_SET_INTERFACE_UP), parcel);
}

void SetInterfaceDownFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel parcel;
    if (!IsDataAndWriteVaild(data, size, parcel)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(IEthernetServiceIpcCode::COMMAND_SET_INTERFACE_DOWN), parcel);
}

void GetInterfaceConfigFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel parcel;
    if (!IsDataAndWriteVaild(data, size, parcel)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(IEthernetServiceIpcCode::COMMAND_GET_INTERFACE_CONFIG), parcel);
}

void SetInterfaceConfigFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return;
    }
    NetManagerExtAccessToken token;
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    MessageParcel parcel;
    if (!IsDataAndWriteVaild(data, size, parcel)) {
        return;
    }
    ConfigurationParcelIpc randObj;
    randObj.ifName_ = GetStringFromData(IFACE_LEN);
    randObj.hwAddr_ = GetStringFromData(IFACE_LEN);
    randObj.ipv4Addr_ = GetStringFromData(IFACE_LEN);
    if (!parcel.WriteParcelable(&randObj)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(IEthernetServiceIpcCode::COMMAND_SET_INTERFACE_CONFIG), parcel);
}

void EthernetServiceCommonFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    auto ethernetServiceCommon = std::make_unique<EthernetServiceCommon>();

    ethernetServiceCommon->ResetEthernetFactory();
}

void RegCustomEapHandlerFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    MessageParcel parcel;
    uint32_t netType = GetData<uint32_t>();
    std::string regCmd = GetStringFromData(REG_CMD_LEN);
    WriteInterfaceToken(parcel);
    if (!parcel.WriteUint32(netType)) {
        return;
    }
    if (!parcel.WriteString(regCmd)) {
        return;
    }
    if (!parcel.WriteRemoteObject(g_eapPostbackCallback->AsObject())) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(IEthernetServiceIpcCode::COMMAND_REG_CUSTOM_EAP_HANDLER), parcel);
}
 
void ReplyCustomEapDataFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    MessageParcel parcel;
    uint32_t result = GetData<uint32_t>();
    WriteInterfaceToken(parcel);
    if (!parcel.WriteUint32(result)) {
        return;
    }
 
    sptr<EapData> eapData = new (std::nothrow) EapData();
    eapData->eapCode = GetData<uint32_t>();
    eapData->eapType = GetData<uint32_t>();
    eapData->msgId = GetData<uint32_t>();
    eapData->bufferLen = GetData<uint32_t>() % REG_CMD_LEN;
    std::vector<uint8_t> tmp = {0x11, 0x12, 0x13};
    eapData->eapBuffer = tmp;
    eapData->Marshalling(parcel);
    OnRemoteRequest(static_cast<uint32_t>(IEthernetServiceIpcCode::COMMAND_REPLY_CUSTOM_EAP_DATA), parcel);
}
 
void RegisterCustomEapCallbackFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    MessageParcel parcel;
    uint32_t netType = GetData<uint32_t>();
    WriteInterfaceToken(parcel);
    if (!parcel.WriteUint32(netType)) {
        return;
    }
    if (!parcel.WriteRemoteObject(g_registerEapCallback->AsObject())) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(IEthernetServiceIpcCode::COMMAND_REGISTER_CUSTOM_EAP_CALLBACK), parcel);
}
 
void UnRegisterCustomEapCallbackFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    MessageParcel parcel;
    uint32_t netType = GetData<uint32_t>();
    WriteInterfaceToken(parcel);
    if (!parcel.WriteUint32(netType)) {
        return;
    }
    if (!parcel.WriteRemoteObject(g_registerEapCallback->AsObject())) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(IEthernetServiceIpcCode::COMMAND_UN_REGISTER_CUSTOM_EAP_CALLBACK), parcel);
}
 
void NotifyWpaEapInterceptInfoFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    MessageParcel parcel;
    uint32_t netType = GetData<uint32_t>();
    WriteInterfaceToken(parcel);
    if (!parcel.WriteUint32(netType)) {
        return;
    }
 
    sptr<EapData> eapData = new (std::nothrow) EapData();
    eapData->eapCode = GetData<uint32_t>();
    eapData->eapType = GetData<uint32_t>();
    eapData->msgId = GetData<uint32_t>();
    eapData->bufferLen = GetData<uint32_t>();;
    std::vector<uint8_t> tmp = {0x11, 0x12, 0x13};
    eapData->eapBuffer = tmp;
    eapData->Marshalling(parcel);
    OnRemoteRequest(static_cast<uint32_t>(IEthernetServiceIpcCode::COMMAND_NOTIFY_WPA_EAP_INTERCEPT_INFO), parcel);
}
 
void GetDeviceInformationFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    NetManagerExtAccessToken token;
    MessageParcel parcel;
    OnRemoteRequest(static_cast<uint32_t>(IEthernetServiceIpcCode::COMMAND_GET_DEVICE_INFORMATION), parcel);
}

void EthernetManagementFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    auto ethernetManagement = std::make_unique<EthernetManagement>();
    EthernetDhcpCallback::DhcpResult dhcpResult;
    ethernetManagement->UpdateDevInterfaceLinkInfo(dhcpResult);

    std::string dev = GetStringFromData(IFACE_LEN);
    bool up = GetData<uint32_t>() % CREATE_BOOL_TYPE_VALUE == 0;
    ethernetManagement->UpdateInterfaceState(dev, up);

    std::string iface = GetStringFromData(IFACE_LEN);
    std::vector<MacAddressInfo> mai;
    ethernetManagement->GetMacAddress(mai);
    
    sptr<InterfaceConfiguration> cfg;
    ethernetManagement->UpdateDevInterfaceCfg(iface, cfg);

    sptr<InterfaceConfiguration> ifaceConfig;
    ethernetManagement->GetDevInterfaceCfg(iface, ifaceConfig);

    int32_t activeStatus = 0;
    ethernetManagement->IsIfaceActive(iface, activeStatus);

    std::vector<std::string> result;
    ethernetManagement->GetAllActiveIfaces(result);

    ethernetManagement->ResetFactory();
    std::string devName = GetStringFromData(IFACE_LEN);
    ethernetManagement->DevInterfaceAdd(devName);
    ethernetManagement->DevInterfaceRemove(devName);
    
    std::vector<EthernetDeviceInfo> devInfoList;
    ethernetManagement->GetDeviceInformation(devInfoList);
}

void EthernetDhcpControllerFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    auto ethernetDhcpController = std::make_unique<EthernetDhcpController>();

    sptr<EthernetDhcpCallback> callback;
    ethernetDhcpController->RegisterDhcpCallback(callback);

    std::string iface = GetStringFromData(IFACE_LEN);
    bool bIpv6 = GetData<uint32_t>() % CREATE_BOOL_TYPE_VALUE == 0;
    ethernetDhcpController->StartClient(iface, bIpv6);

    ethernetDhcpController->StopClient(iface, bIpv6);
}
} // namespace NetManagerStandard
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::NetManagerStandard::SetIfaceConfigFuzzTest(data, size);
    OHOS::NetManagerStandard::GetIfaceConfigFuzzTest(data, size);
    OHOS::NetManagerStandard::IsIfaceActiveFuzzTest(data, size);
    OHOS::NetManagerStandard::GetAllActiveIfacesFuzzTest(data, size);
    OHOS::NetManagerStandard::ResetFactoryFuzzTest(data, size);
    OHOS::NetManagerStandard::UnregisterIfacesStateChangedFuzzTest(data, size);
    OHOS::NetManagerStandard::OnRegisterIfacesStateChangedFuzzTest(data, size);
    OHOS::NetManagerStandard::SetInterfaceUpFuzzTest(data, size);
    OHOS::NetManagerStandard::SetInterfaceDownFuzzTest(data, size);
    OHOS::NetManagerStandard::GetInterfaceConfigFuzzTest(data, size);
    OHOS::NetManagerStandard::SetInterfaceConfigFuzzTest(data, size);
    OHOS::NetManagerStandard::EthernetServiceCommonFuzzTest(data, size);
    OHOS::NetManagerStandard::EthernetManagementFuzzTest(data, size);
    OHOS::NetManagerStandard::EthernetDhcpControllerFuzzTest(data, size);
    OHOS::NetManagerStandard::RegCustomEapHandlerFuzzTest(data, size);
    OHOS::NetManagerStandard::ReplyCustomEapDataFuzzTest(data, size);
    OHOS::NetManagerStandard::RegisterCustomEapCallbackFuzzTest(data, size);
    OHOS::NetManagerStandard::UnRegisterCustomEapCallbackFuzzTest(data, size);
    OHOS::NetManagerStandard::NotifyWpaEapInterceptInfoFuzzTest(data, size);
    return 0;
}
