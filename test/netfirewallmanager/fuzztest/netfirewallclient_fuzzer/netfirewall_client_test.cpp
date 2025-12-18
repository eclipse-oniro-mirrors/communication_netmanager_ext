/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "netfirewall_client_test.h"

#include "http_proxy.h"
#include "inet_addr.h"
#include "net_manager_constants.h"
#include "netmanager_ext_test_security.h"
#include "netmgr_ext_log_wrapper.h"
#include "refbase.h"
#include "singleton.h"

#define private public
#define protected public

#include "netfirewall_client.h"
#include "netfirewall_proxy.h"
#include "netfirewall_common.h"
#include "netfirewall_service.h"
#include "i_netfirewall_service.h"
#include "mock_i_net_intercept_record_callback_test.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const uint8_t *g_baseFuzzData = nullptr;
size_t g_baseFuzzSize = 0;
size_t g_baseFuzzPos;
constexpr size_t IFACE_LEN = 5;
}

template <class T> T GetData()
{
    T object {};
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
    if (!data.WriteInterfaceToken(NetFirewallStub::GetDescriptor())) {
        return false;
    }
    return true;
}

bool IsDataAndWriteVaild(const uint8_t *data, size_t size, MessageParcel &parcel)
{
    if ((data == nullptr) || (size == 0)) {
        return false;
    }

    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    std::string iface = GetStringFromData(IFACE_LEN);
    WriteInterfaceToken(parcel);
    if (!parcel.WriteString(iface)) {
        return false;
    }

    return true;
}

int32_t OnRemoteRequest(uint32_t code, MessageParcel &data)
{
    MessageParcel reply;
    MessageOption option;
    return DelayedSingleton<NetFirewallService>::GetInstance()->OnRemoteRequest(code, data, reply, option);
}

void SetNetFirewallPolicyTest(const uint8_t *data, size_t size)
{
    MessageParcel parcel;
    if (!IsDataAndWriteVaild(data, size, parcel)) {
        return;
    }
    auto status = std::make_unique<NetFirewallPolicy>();
    if (!status->Marshalling(parcel)) {
        return;
    }
    int32_t userId = GetData<int32_t>();
    parcel.WriteInt32(userId);
    OnRemoteRequest(static_cast<uint32_t>(INetFirewallService::SET_NET_FIREWALL_STATUS), parcel);
}

void RegisterInterceptRecordsCallbackTest(const uint8_t *data, size_t size)
{
    MessageParcel parcel;
    if (!IsDataAndWriteVaild(data, size, parcel)) {
        return;
    }
    auto status = std::make_unique<NetFirewallPolicy>();
    if (!status->Marshalling(parcel)) {
        return;
    }
    sptr<INetInterceptRecordCallback> callback = new (std::nothrow) MockINetInterceptRecordCallbackTest();
    if (callback == nullptr) {
        return;
    }
    parcel.WriteRemoteObject(callback->AsObject());
    OnRemoteRequest(static_cast<uint32_t>(INetFirewallService::REGISTER_INTERCEPT_RECORDS_CALLBACK), parcel);
}

void UnregisterInterceptRecordsCallbackTest(const uint8_t *data, size_t size)
{
    MessageParcel parcel;
    if (!IsDataAndWriteVaild(data, size, parcel)) {
        return;
    }
    auto status = std::make_unique<NetFirewallPolicy>();
    if (!status->Marshalling(parcel)) {
        return;
    }
    sptr<INetInterceptRecordCallback> callback = new (std::nothrow) MockINetInterceptRecordCallbackTest();
    if (callback == nullptr) {
        return;
    }
    parcel.WriteRemoteObject(callback->AsObject());
    OnRemoteRequest(static_cast<uint32_t>(INetFirewallService::UNREGISTER_INTERCEPT_RECORDS_CALLBACK), parcel);
}
} // namespace NetManagerStandard
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::NetManagerStandard::SetNetFirewallPolicyTest(data, size);
    OHOS::NetManagerStandard::RegisterInterceptRecordsCallbackTest(data, size);
    OHOS::NetManagerStandard::UnregisterInterceptRecordsCallbackTest(data, size);
    return 0;
}
