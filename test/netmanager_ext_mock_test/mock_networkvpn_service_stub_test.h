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

#ifndef MOCK_NETWORKVPN_SERVICE_STUB_TEST_H
#define MOCK_NETWORKVPN_SERVICE_STUB_TEST_H

#include "i_networkvpn_service.h"
#include "networkvpn_service_stub.h"

namespace OHOS {
namespace NetManagerStandard {
class MockNetworkVpnServiceStub : public NetworkVpnServiceStub {
public:
    int32_t Prepare(bool &isExistVpn, bool &isRun, std::string &pkg) override
    {
        return 0;
    }

    int32_t SetUpVpn(const sptr<VpnConfig> &config, bool isVpnExtCall = false) override
    {
        return 0;
    }

    int32_t Protect(bool isVpnExtCall = false) override
    {
        return 0;
    }

    int32_t DestroyVpn(bool isVpnExtCall = false) override
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

    int32_t CreateVpnConnection(bool isVpnExtCall = false) override
    {
        return 0;
    }

    int32_t Dump(int32_t fd, const std::vector<std::u16string> &args) override
    {
        return 0;
    }

    int32_t FactoryResetVpn() override
    {
        return 0;
    }

    int32_t RegisterBundleName(const std::string &bundleName) override
    {
        return 0;
    }
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // MOCK_NETWORKVPN_SERVICE_STUB_TEST_H
