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

#include "i_networkshare_service.h"
#include "interface_configuration.h"
#include "netmgr_ext_log_wrapper.h"
#define private public
#include "networkshare_service.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const uint8_t *g_baseFuzzData = nullptr;
static constexpr uint32_t CREATE_SHARE_IFACE_TYPE_VALUE = 3;
static constexpr uint32_t CREATE_SHARE_IFACE_STATE_VALUE = 3;
size_t g_baseFuzzSize = 0;
size_t g_baseFuzzPos;

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
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token;
    AccessTokenInternetInfo tokenInfo;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    OnRemoteRequest(INetworkShareService::MessageCode::CMD_GET_SHARING_SUPPORTED, dataParcel);
}

void IsSharingFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
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
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
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
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
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
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
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
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
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
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token;
    AccessTokenInternetInfo tokenInfo;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    uint32_t state = GetData<int32_t>() % CREATE_SHARE_IFACE_STATE_VALUE + 1;
    dataParcel.WriteInt32(state);
    OnRemoteRequest(INetworkShareService::MessageCode::CMD_GET_SHARING_IFACES, dataParcel);
}

void RegisterSharingEventFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
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
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
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
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
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
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
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
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token;
    AccessTokenInternetInfo tokenInfo;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    OnRemoteRequest(INetworkShareService::MessageCode::CMD_GET_TOTAL_BYTES, dataParcel);
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
    return 0;
}
