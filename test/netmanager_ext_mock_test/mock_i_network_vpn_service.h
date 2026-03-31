/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef MOCK_I_NETWORK_VPN_SERVICE_H
#define MOCK_I_NETWORK_VPN_SERVICE_H

#include <gmock/gmock.h>
#include "inetwork_vpn_service.h"

namespace OHOS {
namespace NetManagerStandard {

class MockINetworkVpnService : public INetworkVpnService {
public:
    MOCK_METHOD(sptr<IRemoteObject>, AsObject, ());
    MOCK_METHOD(int32_t, Prepare, (bool &isExistVpn, bool &isRun, std::string &pkg), (override));
    MOCK_METHOD(int32_t, SetUpVpn, (const VpnConfigRawData &configData, bool isVpnExtCall, bool isInternalChannel),
                (override));
    MOCK_METHOD(int32_t, Protect, (bool isVpnExtCall), (override));
    MOCK_METHOD(int32_t, DestroyVpn, (bool isVpnExtCall), (override));
    MOCK_METHOD(int32_t, RegisterVpnEvent, (const sptr<IVpnEventCallback> &callback), (override));
    MOCK_METHOD(int32_t, UnregisterVpnEvent, (const sptr<IVpnEventCallback> &callback), (override));
    MOCK_METHOD(int32_t, CreateVpnConnection, (bool isVpnExtCall), (override));
    MOCK_METHOD(int32_t, FactoryResetVpn, (), (override));
    MOCK_METHOD(int32_t, GetSelfAppName, (std::string &selfAppName, std::string &selfBundleName), (override));
    MOCK_METHOD(int32_t, StartVpnExtensionAbility, (const AAFwk::Want &want), (override));
    MOCK_METHOD(int32_t, StopVpnExtensionAbility, (const AAFwk::Want &want), (override));
    MOCK_METHOD(int32_t, RequestVpnPermission,
                (int32_t uid, const std::string &bundleName, const std::string &abilityName, bool &isAuthorized),
                (override));

#ifdef SUPPORT_SYSVPN
    MOCK_METHOD(int32_t, GetVpnCertData, (const int32_t certType, std::vector<int8_t> &certData), (override));
    MOCK_METHOD(int32_t, SetUpSysVpn, (const sptr<SysVpnConfig> &config, bool isVpnExtCall), (override));
    MOCK_METHOD(int32_t, AddSysVpnConfig, (const sptr<SysVpnConfig> &config), (override));
    MOCK_METHOD(int32_t, DeleteSysVpnConfig, (const std::string &vpnId), (override));
    MOCK_METHOD(int32_t, GetConnectedVpnAppInfo, (std::vector<std::string> &bundleNameList), (override));
    MOCK_METHOD(int32_t, GetSysVpnConfigList, (std::vector<sptr<SysVpnConfig>> &vpnList), (override));
    MOCK_METHOD(int32_t, GetSysVpnConfig, (sptr<SysVpnConfig> &config, const std::string &vpnId), (override));
    MOCK_METHOD(int32_t, GetConnectedSysVpnConfig, (sptr<SysVpnConfig> &config), (override));
    MOCK_METHOD(int32_t, NotifyConnectStage, (const std::string &stage, const int32_t resultIpc), (override));
    MOCK_METHOD(int32_t, GetSysVpnCertUri, (const int32_t certType, std::string &certUri), (override));
    MOCK_METHOD(int32_t, DestroyVpn, (const std::string &vpnId), (override));
    MOCK_METHOD(int32_t, RegisterMultiVpnEvent, (const sptr<IVpnEventCallback> &callback), (override));
    MOCK_METHOD(int32_t, UnregisterMultiVpnEvent, (const sptr<IVpnEventCallback> &callback), (override));
#endif // SUPPORT_SYSVPN
};

} // namespace NetManagerStandard
} // namespace OHOS
#endif // MOCK_I_NETWORK_VPN_SERVICE_H
