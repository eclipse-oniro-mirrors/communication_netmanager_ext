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

#include "ethernet_client_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <new>
#include <securec.h>
#include <string>

#include "accesstoken_kit.h"
#include "refbase.h"
#include "singleton.h"
#include "token_setproc.h"

#include "dev_interface_state.h"
#define private public
#include "ethernet_client.h"
#include "ethernet_service.h"
#include "interface_configuration.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const uint8_t *g_baseFuzzData = nullptr;
static constexpr uint32_t CREATE_BOOL_TYPE_VALUE = 2;
size_t g_baseFuzzSize = 0;
size_t g_baseFuzzPos;
constexpr size_t IFACE_LEN = 5;

using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;
HapInfoParams testInfoParms = {.userID = 1,
                               .bundleName = "ethernet_client_fuzzer",
                               .instIndex = 0,
                               .appIDDesc = "test"};

PermissionDef testPermDef = {.permissionName = "ohos.permission.GET_NETWORK_INFO",
                             .bundleName = "ethernet_client_fuzzer",
                             .grantMode = 1,
                             .availableLevel = OHOS::Security::AccessToken::ATokenAplEnum::APL_SYSTEM_BASIC,
                             .label = "label",
                             .labelId = 1,
                             .description = "Test ethernet maneger network info",
                             .descriptionId = 1};

PermissionDef testInternetPermDef = {.permissionName = "ohos.permission.CONNECTIVITY_INTERNAL",
                                     .bundleName = "net_conn_client_fuzzer",
                                     .grantMode = 1,
                                     .availableLevel = OHOS::Security::AccessToken::ATokenAplEnum::APL_SYSTEM_BASIC,
                                     .label = "label",
                                     .labelId = 1,
                                     .description = "Test ethernet connectivity internet",
                                     .descriptionId = 1};

PermissionStateFull testState = {.permissionName = "ohos.permission.GET_NETWORK_INFO",
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
    AccessToken()
    {
        currentID_ = GetSelfTokenID();
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
    AccessTokenID currentID_ = 0;
    AccessTokenID accessID_ = 0;
};

class AccessTokenInternetInfo {
public:
    AccessTokenInternetInfo()
    {
        currentID_ = GetSelfTokenID();
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
    AccessTokenID currentID_ = 0;
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

int32_t OnRemoteRequest(uint32_t code, MessageParcel &data)
{
    if (!g_isInited) {
        Init();
    }

    MessageParcel reply;
    MessageOption option;
    return DelayedSingleton<EthernetService>::GetInstance()->OnRemoteRequest(code, data, reply, option);
}

void SetIfaceConfigFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token;
    AccessTokenInternetInfo tokenInfo;

    MessageParcel parcel;
    WriteInterfaceToken(parcel);
    std::string iface = GetStringFromData(IFACE_LEN);
    if (!parcel.WriteString(iface)) {
        return;
    }
    auto ic = std::make_unique<InterfaceConfiguration>();
    if (!ic->Marshalling(parcel)) {
        return;
    }
    OnRemoteRequest(EthernetService::CMD_SET_IF_CFG, parcel);
}

void GetIfaceConfigFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token;
    AccessTokenInternetInfo tokenInfo;
    MessageParcel parcel;
    std::string iface = GetStringFromData(IFACE_LEN);
    WriteInterfaceToken(parcel);
    if (!parcel.WriteString(iface)) {
        return;
    }
    OnRemoteRequest(EthernetService::CMD_GET_IF_CFG, parcel);
}

void IsIfaceActiveFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token;
    AccessTokenInternetInfo tokenInfo;
    MessageParcel parcel;
    std::string iface = GetStringFromData(IFACE_LEN);
    WriteInterfaceToken(parcel);
    if (!parcel.WriteString(iface)) {
        return;
    }
    OnRemoteRequest(EthernetService::CMD_IS_ACTIVATE, parcel);
}

void GetAllActiveIfacesFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    AccessToken token;
    AccessTokenInternetInfo tokenInfo;
    MessageParcel parcel;
    WriteInterfaceToken(parcel);
    OnRemoteRequest(EthernetService::CMD_GET_ACTIVATE_INTERFACE, parcel);
}

void ResetFactoryFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    MessageParcel parcel;
    WriteInterfaceToken(parcel);
    OnRemoteRequest(EthernetService::CMD_RESET_FACTORY, parcel);
}

void SetInterfaceUpFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    AccessToken token;
    AccessTokenInternetInfo tokenInfo;
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    MessageParcel parcel;
    std::string iface = GetStringFromData(IFACE_LEN);
    WriteInterfaceToken(parcel);
    if (!parcel.WriteString(iface)) {
        return;
    }
    OnRemoteRequest(EthernetService::CMD_SET_INTERFACE_UP, parcel);
}

void SetInterfaceDownFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    AccessToken token;
    AccessTokenInternetInfo tokenInfo;
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    MessageParcel parcel;
    std::string iface = GetStringFromData(IFACE_LEN);
    WriteInterfaceToken(parcel);
    if (!parcel.WriteString(iface)) {
        return;
    }
    OnRemoteRequest(EthernetService::CMD_SET_INTERFACE_DOWN, parcel);
}

void GetInterfaceConfigFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    AccessToken token;
    AccessTokenInternetInfo tokenInfo;
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    MessageParcel parcel;
    std::string iface = GetStringFromData(IFACE_LEN);
    WriteInterfaceToken(parcel);
    if (!parcel.WriteString(iface)) {
        return;
    }
    OnRemoteRequest(EthernetService::CMD_GET_INTERFACE_CONFIG, parcel);
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
    ethernetDhcpController->StartDhcpClient(iface, bIpv6);

    ethernetDhcpController->StopDhcpClient(iface, bIpv6);
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
    OHOS::NetManagerStandard::SetInterfaceUpFuzzTest(data, size);
    OHOS::NetManagerStandard::SetInterfaceDownFuzzTest(data, size);
    OHOS::NetManagerStandard::GetInterfaceConfigFuzzTest(data, size);
    OHOS::NetManagerStandard::EthernetServiceCommonFuzzTest(data, size);
    OHOS::NetManagerStandard::EthernetManagementFuzzTest(data, size);
    OHOS::NetManagerStandard::EthernetDhcpControllerFuzzTest(data, size);
    return 0;
}