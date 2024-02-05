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

#ifndef NETWORKSHARE_SERVICE_STUB_TEST_H
#define NETWORKSHARE_SERVICE_STUB_TEST_H

#include <gtest/gtest.h>

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "i_networkshare_service.h"
#include "networkshare_service_stub.h"

namespace OHOS {
namespace NetManagerStandard {
class MockNetworkShareServiceStub : public NetworkShareServiceStub {
public:
    MockNetworkShareServiceStub() = default;
    ~MockNetworkShareServiceStub() override {};

    int32_t IsNetworkSharingSupported(int32_t &supported) override
    {
        return 0;
    }

    int32_t IsSharing(int32_t &sharingStatus) override
    {
        return 0;
    }

    int32_t StartNetworkSharing(const SharingIfaceType &type) override
    {
        return 0;
    }

    int32_t StopNetworkSharing(const SharingIfaceType &type) override
    {
        return 0;
    }

    int32_t GetSharableRegexs(SharingIfaceType type, std::vector<std::string> &ifaceRegexs) override
    {
        return 0;
    }

    int32_t GetSharingState(SharingIfaceType type, SharingIfaceState &state) override
    {
        return 0;
    }

    int32_t GetNetSharingIfaces(const SharingIfaceState &state, std::vector<std::string> &ifaces) override
    {
        return 0;
    }

    int32_t RegisterSharingEvent(sptr<ISharingEventCallback> callback) override
    {
        return 0;
    }

    int32_t UnregisterSharingEvent(sptr<ISharingEventCallback> callback) override
    {
        return 0;
    }

    int32_t GetStatsRxBytes(int32_t &bytes) override
    {
        return 0;
    }

    int32_t GetStatsTxBytes(int32_t &bytes) override
    {
        return 0;
    }

    int32_t GetStatsTotalBytes(int32_t &bytes) override
    {
        return 0;
    }

    int32_t Dump(int32_t fd, const std::vector<std::u16string> &args) override
    {
        return 0;
    }
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETWORKSHARE_SERVICE_STUB_TEST_H
