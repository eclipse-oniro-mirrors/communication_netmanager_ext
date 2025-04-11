/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef MOCK_VPN_EVENT_CALLBACK_TEST_H
#define MOCK_VPN_EVENT_CALLBACK_TEST_H

#include "imdns_service.h"
#include "refbase.h"

namespace OHOS {
namespace NetManagerStandard {
class MockIDiscoveryCallbackTest : public IRemoteStub<IDiscoveryCallback> {
public:
    int32_t HandleStartDiscover(const MDnsServiceInfo &serviceInfo, int32_t retCode) override { return 0; }
    int32_t HandleStopDiscover(const MDnsServiceInfo &serviceInfo, int32_t retCode) override { return 0; }
    int32_t HandleServiceFound(const MDnsServiceInfo &serviceInfo, int32_t retCode) override { return 0; }
    int32_t HandleServiceLost(const MDnsServiceInfo &serviceInfo, int32_t retCode) override { return 0; }
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // MOCK_VPN_EVENT_CALLBACK_TEST_H
