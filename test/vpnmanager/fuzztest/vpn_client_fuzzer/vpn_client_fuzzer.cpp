/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "vpn_client_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <iostream>
#include <memory>
#include <new>
#include <string>

#include "accesstoken_kit.h"
#include "refbase.h"
#include "token_setproc.h"
#include <securec.h>

#include "i_networkvpn_service.h"
#include "i_vpn_event_callback.h"
#include "iremote_stub.h"
#include "net_manager_ext_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "networkvpn_client.h"
#include "networkvpn_service_stub.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const uint8_t *g_baseFuzzData = nullptr;
size_t g_baseFuzzSize = 0;
size_t g_baseFuzzPos;

using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;
HapInfoParams testInfoParms = {.userID = 1,
                               .bundleName = "vpn_client_fuzzer",
                               .instIndex = 0,
                               .appIDDesc = "test",
                               .isSystemApp = true};

PermissionDef testPermDef = {.permissionName = "ohos.permission.CONNECTIVITY_INTERNAL",
                             .bundleName = "vpn_client_fuzzer",
                             .grantMode = 1,
                             .availableLevel = OHOS::Security::AccessToken::ATokenAplEnum::APL_SYSTEM_BASIC,
                             .label = "label",
                             .labelId = 1,
                             .description = "Test vpn maneger network info",
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

class IVpnEventCallbackTest : public IRemoteStub<IVpnEventCallback> {
public:
    void OnVpnStateChanged(const bool &isConnected) override
    {
        return;
    }
};

class TestVpnStub : public NetworkVpnServiceStub {
public:
    int32_t Prepare(bool &isExistVpn, bool &isRun, std::string &pkg) override
    {
        return 0;
    }
    int32_t SetUpVpn(const sptr<VpnConfig> &config) override
    {
        return 0;
    }
    int32_t Protect() override
    {
        return 0;
    }
    int32_t DestroyVpn() override
    {
        return 0;
    }
    int32_t RegisterVpnEvent(const sptr<IVpnEventCallback> callback) override
    {
        return 0;
    }
    int32_t UnregisterVpnEvent(const sptr<IVpnEventCallback> callback) override
    {
        return 0;
    }
    TestVpnStub();
    ~TestVpnStub();
};

TestVpnStub::TestVpnStub() {}

TestVpnStub::~TestVpnStub() {}

int32_t OnRemoteRequest(INetworkVpnService::MessageCode code, MessageParcel &data)
{
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<NetworkVpnServiceStub> test = std::make_shared<TestVpnStub>();
    return test->OnRemoteRequest(static_cast<uint32_t>(code), data, reply, option);
}

class AccessToken {
public:
    AccessToken() : currentID_(GetSelfTokenID())
    {
        AccessTokenIDEx tokenIdEx = AccessTokenKit::AllocHapToken(testInfoParms, testPolicyPrams);
        accessID_ = tokenIdEx.tokenIdExStruct.tokenID;
        SetSelfTokenID(tokenIdEx.tokenIDEx);
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

bool WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return false;
    }
    return true;
}

void PrepareFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("PrepareFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }

    AccessToken token;
    AccessTokenInternetInfo tokenInfo;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    OnRemoteRequest(INetworkVpnService::MessageCode::CMD_PREPARE, dataParcel);
}

void ProtectFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("ProtectFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }

    AccessToken token;
    AccessTokenInternetInfo tokenInfo;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    OnRemoteRequest(INetworkVpnService::MessageCode::CMD_PROTECT, dataParcel);
}

void SetUpVpnFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("SetUpVpnFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }

    AccessToken token;
    AccessTokenInternetInfo tokenInfo;

    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    if (!config->Marshalling(dataParcel)) {
        return;
    }

    OnRemoteRequest(INetworkVpnService::MessageCode::CMD_START_VPN, dataParcel);
}

void DestroyVpnFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("DestroyVpnFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }

    AccessToken token;
    AccessTokenInternetInfo tokenInfo;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    OnRemoteRequest(INetworkVpnService::MessageCode::CMD_STOP_VPN, dataParcel);
}

void RegisterVpnEventFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("RegisterVpnEventFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }

    AccessToken token;
    AccessTokenInternetInfo tokenInfo;

    sptr<IVpnEventCallback> callback = new (std::nothrow) IVpnEventCallbackTest();

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    if (!dataParcel.WriteRemoteObject(callback->AsObject())) {
        return;
    }
    OnRemoteRequest(INetworkVpnService::MessageCode::CMD_REGISTER_EVENT_CALLBACK, dataParcel);
}

void UnregisterVpnEventFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("UnregisterVpnEventFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }

    AccessToken token;
    AccessTokenInternetInfo tokenInfo;

    sptr<IVpnEventCallback> callback = new (std::nothrow) IVpnEventCallbackTest();

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    if (!dataParcel.WriteRemoteObject(callback->AsObject())) {
        return;
    }
    OnRemoteRequest(INetworkVpnService::MessageCode::CMD_UNREGISTER_EVENT_CALLBACK, dataParcel);
}
} // namespace NetManagerStandard
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::NetManagerStandard::PrepareFuzzTest(data, size);
    OHOS::NetManagerStandard::ProtectFuzzTest(data, size);
    OHOS::NetManagerStandard::SetUpVpnFuzzTest(data, size);
    OHOS::NetManagerStandard::DestroyVpnFuzzTest(data, size);
    OHOS::NetManagerStandard::RegisterVpnEventFuzzTest(data, size);
    OHOS::NetManagerStandard::UnregisterVpnEventFuzzTest(data, size);
    return 0;
}