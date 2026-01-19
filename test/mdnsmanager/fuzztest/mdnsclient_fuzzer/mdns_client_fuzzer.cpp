/*
 * Copyright (C) 2023-2024 Huawei Device Co., Ltd.
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
#include <securec.h>

#include "imdns_service.h"
#include "iservice_registry.h"
#include "message_parcel.h"
#include "net_manager_ext_constants.h"
#include "refbase.h"
#include "system_ability_definition.h"
#include "netmgr_ext_log_wrapper.h"
#include "mdns_protocol_impl.h"
#define private public
#include "mdns_client.h"
#include "mdns_service.h"
#undef private

namespace OHOS {
namespace NetManagerStandard {

class IRegistrationCallbackTest : public IRemoteStub<IRegistrationCallback> {
public:
    int32_t HandleRegister(const MDnsServiceInfo &serviceInfo, int32_t retCode) override { return 0; }
    int32_t HandleUnRegister(const MDnsServiceInfo &serviceInfo, int32_t retCode) override { return 0; }
    int32_t HandleRegisterResult(const MDnsServiceInfo &serviceInfo, int32_t retCode) override { return 0; }
};

class IDiscoveryCallbackTest : public IRemoteStub<IDiscoveryCallback> {
public:
    int32_t HandleStartDiscover(const MDnsServiceInfo &serviceInfo, int32_t retCode) override { return 0; }
    int32_t HandleStopDiscover(const MDnsServiceInfo &serviceInfo, int32_t retCode) override { return 0; }
    int32_t HandleServiceFound(const MDnsServiceInfo &serviceInfo, int32_t retCode) override { return 0; }
    int32_t HandleServiceLost(const MDnsServiceInfo &serviceInfo, int32_t retCode) override { return 0; }
};

class IResolveCallbackTest : public IRemoteStub<IResolveCallback> {
public:
    int32_t HandleResolveResult(const MDnsServiceInfo &serviceInfo, int32_t retCode) override { return 0; }
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
    return data.WriteInterfaceToken(IMdnsService::GetDescriptor());
}

__attribute__((no_sanitize("cfi"))) bool GetMessageParcel(const uint8_t *data, size_t size, MessageParcel &dataParcel)
{
    if (!InitGlobalData(data, size)) {
        return false;
    }

    if (!WriteInterfaceToken(dataParcel)) {
        return false;
    }

    MDnsServiceInfo* info = new (std::nothrow) MDnsServiceInfo();
    info->name = GetStringFromData(STR_LEN);
    info->type = GetStringFromData(STR_LEN);
    info->family = GetData<int32_t>();
    info->addr = GetStringFromData(STR_LEN);
    info->port = GetData<int32_t>();
    std::string str = GetStringFromData(STR_LEN);
    info->txtRecord = std::vector<uint8_t>(str.begin(), str.end());
    if (!dataParcel.WriteParcelable(info)) {
        return false;
    }

    return true;
}

void Init()
{
    if (!g_isInited) {
        DelayedSingleton<MDnsService>::GetInstance()->Init();
        g_isInited = true;
    }
}

__attribute__((no_sanitize("cfi"))) int32_t OnRemoteRequest(uint32_t code, MessageParcel &data)
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
    MessageParcel dataParcel;
    if (!GetMessageParcel(data, size, dataParcel)) {
        return;
    }

    sptr<IRegistrationCallbackTest> callback = new (std::nothrow) IRegistrationCallbackTest();
    if (callback == nullptr) {
        return;
    }
    dataParcel.WriteRemoteObject(callback->AsObject());

    OnRemoteRequest(static_cast<uint32_t>(IMdnsServiceIpcCode::COMMAND_REGISTER_SERVICE), dataParcel);
}

void UnRegisterServiceFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("UnRegisterServiceFuzzTest enter");
    MessageParcel dataParcel;
    if (!InitGlobalData(data, size)) {
        return;
    }

    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    sptr<IRegistrationCallbackTest> callback = new (std::nothrow) IRegistrationCallbackTest();
    if (callback == nullptr) {
        return;
    }
    dataParcel.WriteRemoteObject(callback->AsObject());

    OnRemoteRequest(static_cast<uint32_t>(IMdnsServiceIpcCode::COMMAND_UN_REGISTER_SERVICE), dataParcel);
}

void StartDiscoverServiceFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("StartDiscoverServiceFuzzTest enter");
    MessageParcel dataParcel;
    if (!InitGlobalData(data, size)) {
        return;
    }

    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    std::string serviceType = GetStringFromData(STR_LEN);
    if (!dataParcel.WriteString16(Str8ToStr16(serviceType))) {
        return;
    }
    sptr<IDiscoveryCallbackTest> callback = new (std::nothrow) IDiscoveryCallbackTest();
    if (callback == nullptr) {
        return;
    }
    dataParcel.WriteRemoteObject(callback->AsObject());

    OnRemoteRequest(static_cast<uint32_t>(IMdnsServiceIpcCode::COMMAND_START_DISCOVER_SERVICE), dataParcel);
}

void StopDiscoverServiceFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("StopDiscoverServiceFuzzTest enter");
    MessageParcel dataParcel;
    if (!InitGlobalData(data, size)) {
        return;
    }

    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    sptr<IDiscoveryCallbackTest> callback = new (std::nothrow) IDiscoveryCallbackTest();
    if (callback == nullptr) {
        return;
    }
    dataParcel.WriteRemoteObject(callback->AsObject());

    OnRemoteRequest(static_cast<uint32_t>(IMdnsServiceIpcCode::COMMAND_STOP_DISCOVER_SERVICE), dataParcel);
}

void ResolveServiceFuzzTest(const uint8_t *data, size_t size)
{
    NETMGR_EXT_LOG_D("ResolveServiceFuzzTest enter");
    MessageParcel dataParcel;
    if (!GetMessageParcel(data, size, dataParcel)) {
        return;
    }

    sptr<IResolveCallbackTest> callback = new (std::nothrow) IResolveCallbackTest();
    if (callback == nullptr) {
        return;
    }
    dataParcel.WriteRemoteObject(callback->AsObject());

    OnRemoteRequest(static_cast<uint32_t>(IMdnsServiceIpcCode::COMMAND_RESOLVE_SERVICE), dataParcel);
}

void MdnsRegisterServiceFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }
    MDnsServiceInfo serviceInfo;
    std::string name(reinterpret_cast<const char *>(data), size);
    serviceInfo.name = name;
    serviceInfo.port = static_cast<int32_t>(size % STR_LEN);
    sptr<IRegistrationCallbackTest> callback = new (std::nothrow) IRegistrationCallbackTest();
    if (callback == nullptr) {
        return;
    }
    DelayedSingleton<MDnsClient>::GetInstance()->RegisterService(serviceInfo, callback);
    DelayedSingleton<MDnsClient>::GetInstance()->UnRegisterService(callback);
}

void MdnsStartDiscoverServiceFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }
    sptr<IDiscoveryCallbackTest> callback = new (std::nothrow) IDiscoveryCallbackTest();
    if (callback == nullptr) {
        return;
    }
    std::string serviceType(reinterpret_cast<const char *>(data), size);
    DelayedSingleton<MDnsClient>::GetInstance()->StartDiscoverService(serviceType, callback);
    DelayedSingleton<MDnsClient>::GetInstance()->StopDiscoverService(callback);
}

void MdnsResolveServiceFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }
    sptr<IResolveCallbackTest> callback = new (std::nothrow) IResolveCallbackTest();
    if (callback == nullptr) {
        return;
    }
    MDnsServiceInfo serviceInfo;
    std::string name(reinterpret_cast<const char *>(data), size);
    serviceInfo.port = static_cast<int32_t>(size % STR_LEN);
    serviceInfo.name = name;
    DelayedSingleton<MDnsClient>::GetInstance()->ResolveService(serviceInfo, callback);
    DelayedSingleton<MDnsClient>::GetInstance()->RestartResume();
}

void ReceivePacketTest(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return;
    }
    std::vector<uint8_t> copy = std::vector<uint8_t>(data, data + size);
    MDnsPayloadParser parser;
    MDnsMessage msg = parser.FromBytes(copy);
}
} // namespace NetManagerStandard
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::NetManagerStandard::RegisterServiceFuzzTest(data, size);
    OHOS::NetManagerStandard::StartDiscoverServiceFuzzTest(data, size);
    OHOS::NetManagerStandard::StopDiscoverServiceFuzzTest(data, size);
    OHOS::NetManagerStandard::ResolveServiceFuzzTest(data, size);
    OHOS::NetManagerStandard::UnRegisterServiceFuzzTest(data, size);
    OHOS::NetManagerStandard::MdnsRegisterServiceFuzzTest(data, size);
    OHOS::NetManagerStandard::MdnsStartDiscoverServiceFuzzTest(data, size);
    OHOS::NetManagerStandard::MdnsResolveServiceFuzzTest(data, size);
    OHOS::NetManagerStandard::ReceivePacketTest(data, size);
    return 0;
}
