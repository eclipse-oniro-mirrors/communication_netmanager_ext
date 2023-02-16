/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "mdns_client_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "message_parcel.h"
#include "refbase.h"
#include <securec.h>

#include "i_mdns_event.h"
#include "net_manager_ext_constants.h"
#define private public
#include "mdns_service.h"
#undef private

namespace OHOS {
namespace NetManagerStandard {

class IRegistrationCallbackTest : public IRemoteStub<IRegistrationCallback> {
public:
    void HandleRegister(const MDnsServiceInfo &serviceInfo, int32_t retCode) override {}
    void HandleUnRegister(const MDnsServiceInfo &serviceInfo, int32_t retCode) override {}
    void HandleRegisterResult(const MDnsServiceInfo &serviceInfo, int32_t retCode) override {}
};

class IDiscoveryCallbackTest : public IRemoteStub<IDiscoveryCallback> {
public:
    void HandleStartDiscover(const MDnsServiceInfo &serviceInfo, int32_t retCode) override {}
    void HandleStopDiscover(const MDnsServiceInfo &serviceInfo, int32_t retCode) override {}
    void HandleServiceFound(const MDnsServiceInfo &serviceInfo, int32_t retCode) override {}
    void HandleServiceLost(const MDnsServiceInfo &serviceInfo, int32_t retCode) override {}
};

class IResolveCallbackTest : public IRemoteStub<IResolveCallback> {
public:
    void HandleResolveResult(const MDnsServiceInfo &serviceInfo, int32_t retCode) override {}
};

static const uint8_t *g_baseFuzzData = nullptr;
static size_t g_baseFuzzSize = 0;
static size_t g_baseFuzzPos;
static bool g_isInited = false;
static constexpr size_t STR_LEN = 10;

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

bool WriteInterfaceToken(MessageParcel &data)
{
    return data.WriteInterfaceToken(IMDnsService::GetDescriptor());
}

void Init()
{
    if (!g_isInited) {
        DelayedSingleton<MDnsService>::GetInstance()->Init();
        g_isInited = true;
    }
}

int32_t OnRemoteRequest(uint32_t code, MessageParcel &data)
{
    if (!g_isInited) {
        Init();
    }

    MessageParcel reply;
    MessageOption option;
    return DelayedSingleton<MDnsService>::GetInstance()->OnRemoteRequest(code, data, reply, option);
}

void RegisterServiceFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("RegisterServiceFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    sptr<MDnsServiceInfo> info = new (std::nothrow) MDnsServiceInfo();
    info->name = GetStringFromData(STR_LEN);
    info->type = GetStringFromData(STR_LEN);
    info->family = GetData<int32_t>();
    info->addr = GetStringFromData(STR_LEN);
    info->port = GetData<int32_t>();
    std::string str = GetStringFromData(STR_LEN);
    info->txtRecord = std::vector<uint8_t>(str.begin(), str.end());
    if (!MDnsServiceInfo::Marshalling(dataParcel, info)) {
        return;
    }

    sptr<IRegistrationCallbackTest> callback = new (std::nothrow) IRegistrationCallbackTest();
    if (callback == nullptr) {
        return;
    }
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    OnRemoteRequest(IMDnsService::CMD_REGISTER, dataParcel);
}

void UnRegisterServiceFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("UnRegisterServiceFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    sptr<MDnsServiceInfo> info = new (std::nothrow) MDnsServiceInfo();
    info->name = GetStringFromData(STR_LEN);
    info->type = GetStringFromData(STR_LEN);
    info->family = GetData<int32_t>();
    info->addr = GetStringFromData(STR_LEN);
    info->port = GetData<int32_t>();
    std::string str = GetStringFromData(STR_LEN);
    info->txtRecord = std::vector<uint8_t>(str.begin(), str.end());
    if (!MDnsServiceInfo::Marshalling(dataParcel, info)) {
        return;
    }

    sptr<IRegistrationCallbackTest> callback = new (std::nothrow) IRegistrationCallbackTest();
    if (callback == nullptr) {
        return;
    }
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    OnRemoteRequest(IMDnsService::CMD_STOP_REGISTER, dataParcel);
}

void StartDiscoverServiceFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("StartDiscoverServiceFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    sptr<MDnsServiceInfo> info = new (std::nothrow) MDnsServiceInfo();
    info->name = GetStringFromData(STR_LEN);
    info->type = GetStringFromData(STR_LEN);
    info->family = GetData<int32_t>();
    info->addr = GetStringFromData(STR_LEN);
    info->port = GetData<int32_t>();
    std::string str = GetStringFromData(STR_LEN);
    info->txtRecord = std::vector<uint8_t>(str.begin(), str.end());
    if (!MDnsServiceInfo::Marshalling(dataParcel, info)) {
        return;
    }

    sptr<IDiscoveryCallbackTest> callback = new (std::nothrow) IDiscoveryCallbackTest();
    if (callback == nullptr) {
        return;
    }
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    OnRemoteRequest(IMDnsService::CMD_DISCOVER, dataParcel);
}

void StopDiscoverServiceFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("StopDiscoverServiceFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    sptr<MDnsServiceInfo> info = new (std::nothrow) MDnsServiceInfo();
    info->name = GetStringFromData(STR_LEN);
    info->type = GetStringFromData(STR_LEN);
    info->family = GetData<int32_t>();
    info->addr = GetStringFromData(STR_LEN);
    info->port = GetData<int32_t>();
    std::string str = GetStringFromData(STR_LEN);
    info->txtRecord = std::vector<uint8_t>(str.begin(), str.end());
    if (!MDnsServiceInfo::Marshalling(dataParcel, info)) {
        return;
    }

    sptr<IDiscoveryCallbackTest> callback = new (std::nothrow) IDiscoveryCallbackTest();
    if (callback == nullptr) {
        return;
    }
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    OnRemoteRequest(IMDnsService::CMD_STOP_DISCOVER, dataParcel);
}

void ResolveServiceFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("ResolveServiceFuzzTest enter");
    if (!InitGlobalData(data, size)) {
        return;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    sptr<MDnsServiceInfo> info = new (std::nothrow) MDnsServiceInfo();
    info->name = GetStringFromData(STR_LEN);
    info->type = GetStringFromData(STR_LEN);
    info->family = GetData<int32_t>();
    info->addr = GetStringFromData(STR_LEN);
    info->port = GetData<int32_t>();
    std::string str = GetStringFromData(STR_LEN);
    info->txtRecord = std::vector<uint8_t>(str.begin(), str.end());
    if (!MDnsServiceInfo::Marshalling(dataParcel, info)) {
        return;
    }

    sptr<IResolveCallbackTest> callback = new (std::nothrow) IResolveCallbackTest();
    if (callback == nullptr) {
        return;
    }
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    OnRemoteRequest(IMDnsService::CMD_RESOLVE, dataParcel);
}
} // namespace NetManagerStandard
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::NetManagerStandard::RegisterServiceFuzzTest(data, size);
    OHOS::NetManagerStandard::UnRegisterServiceFuzzTest(data, size);
    OHOS::NetManagerStandard::StartDiscoverServiceFuzzTest(data, size);
    OHOS::NetManagerStandard::StopDiscoverServiceFuzzTest(data, size);
    OHOS::NetManagerStandard::ResolveServiceFuzzTest(data, size);
    return 0;
}
