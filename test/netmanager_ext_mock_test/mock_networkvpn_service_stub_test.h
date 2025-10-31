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

#include "inetwork_vpn_service.h"
#include "network_vpn_service_stub.h"

namespace OHOS {
namespace NetManagerStandard {
class MockNetworkVpnServiceStub : public NetworkVpnServiceStub {
public:
    int32_t Prepare(bool &isExistVpn, bool &isRun, std::string &pkg) override
    {
        return 0;
    }

    int32_t SetUpVpn(const VpnConfig &config, bool isVpnExtCall = false, bool isInternalChannel = false) override
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

#ifdef SUPPORT_SYSVPN
    int32_t GetVpnCertData(const int32_t certType, std::vector<int8_t> &certData) override
    {
        return 0;
    }

    int32_t SetUpSysVpn(const sptr<SysVpnConfig> &config, bool isVpnExtCall = false) override
    {
        return 0;
    }

    int32_t AddSysVpnConfig(const sptr<SysVpnConfig> &config) override
    {
        return 0;
    }

    int32_t DeleteSysVpnConfig(const std::string &vpnId) override
    {
        return 0;
    }

    int32_t GetSysVpnConfigList(std::vector<sptr<SysVpnConfig>> &vpnList) override
    {
        return 0;
    }

    int32_t GetSysVpnConfig(sptr<SysVpnConfig> &config, const std::string &vpnId) override
    {
        return 0;
    }

    int32_t GetConnectedSysVpnConfig(sptr<SysVpnConfig> &config) override
    {
        return 0;
    }

    int32_t NotifyConnectStage(const std::string &stage, const int32_t errorCode) override
    {
        return 0;
    }

    int32_t GetSysVpnCertUri(const int32_t certType, std::string &certUri) override
    {
        return 0;
    }

    int32_t GetConnectedVpnAppInfo(std::vector<std::string> &bundleNameList) override
    {
        return 0;
    }

    int32_t DestroyVpn(const std::string &vpnId) override
    {
        return 0;
    }

    int32_t RegisterMultiVpnEvent(const sptr<IVpnEventCallback> &callback) override
    {
        return 0;
    }

    int32_t UnregisterMultiVpnEvent(const sptr<IVpnEventCallback> &callback) override
    {
        return 0;
    }
#endif // SUPPORT_SYSVPN

    int32_t RegisterVpnEvent(const sptr<IVpnEventCallback>& callback) override
    {
        return 0;
    }

    int32_t UnregisterVpnEvent(const sptr<IVpnEventCallback>& callback) override
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

    int32_t RegisterBundleName(const std::string &bundleName, const std::string &abilityName) override
    {
        return 0;
    }

    int32_t GetSelfAppName(std::string &selfAppName, std::string &selfBundleName) override
    {
        return 0;
    }

    int32_t SetSelfVpnPid() override
    {
        return 0;
    }
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // MOCK_NETWORKVPN_SERVICE_STUB_TEST_H
