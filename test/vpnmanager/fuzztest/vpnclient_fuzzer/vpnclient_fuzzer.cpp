/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "vpnclient_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <securec.h>

#include "i_vpn_event_callback.h"
#include "netmanager_ext_test_security.h"
#include "networkvpn_service.h"
#include "refbase.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {

constexpr size_t STR_LEN = 16;

size_t g_baseFuzzPos = 0;
size_t g_baseFuzzSize = 0;
const uint8_t *g_baseFuzzData = nullptr;
} // namespace

bool InitGlobalData(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return false;
    }
    g_baseFuzzPos = 0;
    g_baseFuzzSize = size;
    g_baseFuzzData = data;
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

std::string GetStringData()
{
    char cstr[STR_LEN] = {0};
    for (uint32_t i = 0; i < STR_LEN - 1; i++) {
        cstr[i] = GetData<char>();
    }
    return std::string(cstr);
}

INetAddr GetAddressData()
{
    INetAddr netAddr;
    netAddr.type_ = GetData<uint8_t>();
    netAddr.family_ = GetData<uint8_t>();
    netAddr.prefixlen_ = GetData<uint8_t>();
    netAddr.address_ = GetStringData();
    netAddr.netMask_ = GetStringData();
    netAddr.hostName_ = GetStringData();
    netAddr.port_ = GetData<uint8_t>();
    return netAddr;
}

Route GetRouteData()
{
    Route route;
    route.iface_ = GetStringData();
    route.destination_ = GetAddressData();
    route.gateway_ = GetAddressData();
    route.mtu_ = GetData<int32_t>();
    route.isHost_ = GetData<bool>();
    route.hasGateway_ = GetData<bool>();
    route.isDefaultRoute_ = GetData<bool>();
    return route;
}

class VpnEventCallbackTest : public IRemoteStub<IVpnEventCallback> {
public:
    void OnVpnStateChanged(const bool &isConnected) override{};
    void OnVpnMultiUserSetUp() override{};
};

__attribute__((no_sanitize("cfi"))) int32_t OnRemoteRequest(INetworkVpnService::MessageCode code, MessageParcel &data)
{
    MessageParcel reply;
    MessageOption option;
    return NetworkVpnService::GetInstance().OnRemoteRequest(static_cast<uint32_t>(code), data, reply, option);
}

void PrepareFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerExtAccessToken token;
    MessageParcel dataParcel;
    if (!dataParcel.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    OnRemoteRequest(INetworkVpnService::MessageCode::CMD_PREPARE, dataParcel);
}

void ProtectFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerExtAccessToken token;
    MessageParcel dataParcel;
    if (!dataParcel.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    OnRemoteRequest(INetworkVpnService::MessageCode::CMD_PROTECT, dataParcel);
}

void SetUpVpnFuzzTest(const uint8_t *data, size_t size)
{
    if (!InitGlobalData(data, size)) {
        return;
    }

    NetManagerExtAccessToken token;
    MessageParcel dataParcel;
    if (!dataParcel.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    config->addresses_.emplace_back(GetAddressData());
    config->routes_.emplace_back(GetRouteData());
    config->mtu_ = GetData<int32_t>();
    config->isAcceptIPv4_ = GetData<bool>();
    config->isAcceptIPv6_ = GetData<bool>();
    config->isLegacy_ = GetData<bool>();
    config->isMetered_ = GetData<bool>();
    config->isBlocking_ = GetData<bool>();
    config->dnsAddresses_.emplace_back(GetStringData());
    config->searchDomains_.emplace_back(GetStringData());
    config->acceptedApplications_.emplace_back(GetStringData());
    config->acceptedApplications_.emplace_back(GetStringData());
    if (!config->Marshalling(dataParcel)) {
        return;
    }
    OnRemoteRequest(INetworkVpnService::MessageCode::CMD_START_VPN, dataParcel);
}

void DestroyVpnFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerExtAccessToken token;
    MessageParcel dataParcel;
    if (!dataParcel.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    OnRemoteRequest(INetworkVpnService::MessageCode::CMD_STOP_VPN, dataParcel);
}

void RegisterVpnEventFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerExtAccessToken token;
    MessageParcel dataParcel;
    if (!dataParcel.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }

    sptr<IVpnEventCallback> callback = new (std::nothrow) VpnEventCallbackTest();
    if (callback == nullptr || !dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr())) {
        return;
    }
    OnRemoteRequest(INetworkVpnService::MessageCode::CMD_REGISTER_EVENT_CALLBACK, dataParcel);
}

void UnregisterVpnEventFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerExtAccessToken token;
    MessageParcel dataParcel;
    if (!dataParcel.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }

    sptr<IVpnEventCallback> callback = new (std::nothrow) VpnEventCallbackTest();
    if (callback == nullptr || !dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr())) {
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
