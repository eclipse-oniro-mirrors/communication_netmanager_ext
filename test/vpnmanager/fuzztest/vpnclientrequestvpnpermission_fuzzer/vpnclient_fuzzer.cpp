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

__attribute__((no_sanitize("cfi"))) int32_t OnRemoteRequest(INetworkVpnServiceIpcCode code,
    MessageParcel &data)
{
    MessageParcel reply;
    MessageOption option;
    return DelayedSingleton<NetworkVpnService>::GetInstance()->OnRemoteRequest(static_cast<uint32_t>(code), data,
        reply, option);
}

void RequestVpnPermissionFuzzTest(const uint8_t *data, size_t size)
{
    if (!InitGlobalData(data, size)) {
        return;
    }
    int32_t uid = GetData<int32_t>();
    std::string bundleName = GetStringData();
    std::string abilityName = GetStringData();
    NetManagerExtAccessToken token;
    MessageParcel dataParcel;
    if (!dataParcel.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }

    if (!dataParcel.WriteInt32(uid)) {
        return;
    }

    if (!dataParcel.WriteString16(Str8ToStr16(bundleName))) {
        return;
    }

    if (!dataParcel.WriteString16(Str8ToStr16(abilityName))) {
        return;
    }
    OnRemoteRequest(INetworkVpnServiceIpcCode::COMMAND_REQUEST_VPN_PERMISSION, dataParcel);
}
} // namespace NetManagerStandard
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::NetManagerStandard::RequestVpnPermissionFuzzTest(data, size);
    return 0;
}
