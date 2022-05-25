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
#include "ethernet_client.h"

namespace OHOS {
namespace NetManagerStandard {
void SetIfaceConfigFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    std::string iface(reinterpret_cast<const char*>(data), size);
    sptr<InterfaceConfiguration> ic = (std::make_unique<InterfaceConfiguration>()).release();

    DelayedSingleton<EthernetClient>::GetInstance()->SetIfaceConfig(iface, ic);
}

void GetIfaceConfigFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    std::string iface(reinterpret_cast<const char*>(data), size);

    DelayedSingleton<EthernetClient>::GetInstance()->GetIfaceConfig(iface);
}

void IsIfaceActiveFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    std::string iface(reinterpret_cast<const char*>(data), size);

    DelayedSingleton<EthernetClient>::GetInstance()->IsIfaceActive(iface);
}

void GetAllActiveIfacesFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    std::string iface(reinterpret_cast<const char*>(data), size);

    DelayedSingleton<EthernetClient>::GetInstance()->GetAllActiveIfaces();
}

void ResetFactoryFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    std::string iface(reinterpret_cast<const char*>(data), size);

    DelayedSingleton<EthernetClient>::GetInstance()->ResetFactory();
}
} // NetManagerStandard
} // OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::NetManagerStandard::SetIfaceConfigFuzzTest(data, size);
    OHOS::NetManagerStandard::GetIfaceConfigFuzzTest(data, size);
    OHOS::NetManagerStandard::IsIfaceActiveFuzzTest(data, size);
    OHOS::NetManagerStandard::GetAllActiveIfacesFuzzTest(data, size);
    OHOS::NetManagerStandard::ResetFactoryFuzzTest(data, size);

    return 0;
}