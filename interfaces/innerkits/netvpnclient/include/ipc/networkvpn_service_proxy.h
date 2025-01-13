/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef NETWORKVPN_SERVICE_PROXY_H
#define NETWORKVPN_SERVICE_PROXY_H

#include <string>

#include "i_networkvpn_service.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace NetManagerStandard {
class NetworkVpnServiceProxy : public IRemoteProxy<INetworkVpnService> {
public:
    explicit NetworkVpnServiceProxy(const sptr<IRemoteObject> &impl);
    virtual ~NetworkVpnServiceProxy() = default;

    int32_t Prepare(bool &isExistVpn, bool &isRun, std::string &pkg) override;
    int32_t SetUpVpn(const sptr<VpnConfig> &config, bool isVpnExtCall = false) override;
    int32_t Protect(bool isVpnExtCall = false) override;
    int32_t DestroyVpn(bool isVpnExtCall = false) override;
    int32_t RegisterVpnEvent(sptr<IVpnEventCallback> callback) override;
    int32_t UnregisterVpnEvent(sptr<IVpnEventCallback> callback) override;
    int32_t CreateVpnConnection(bool isVpnExtCall = false) override;
    int32_t FactoryResetVpn() override;
    int32_t RegisterBundleName(const std::string &bundleName, const std::string &abilityName) override;
    int32_t GetSelfAppName(std::string &selfAppName, std::string &selfBundleName) override;
    int32_t SetSelfVpnPid() override;
#ifdef SUPPORT_SYSVPN
    int32_t SetUpVpn(const sptr<SysVpnConfig> &config) override;
    int32_t AddSysVpnConfig(sptr<SysVpnConfig> &config) override;
    int32_t DeleteSysVpnConfig(const std::string &vpnId) override;
    int32_t GetSysVpnConfigList(std::vector<SysVpnConfig> &vpnList) override;
    int32_t GetSysVpnConfig(sptr<SysVpnConfig> &config, const std::string &vpnId) override;
    int32_t GetConnectedSysVpnConfig(sptr<SysVpnConfig> &config) override;
    int32_t NotifyConnectStage(const std::string &stage, const int32_t &result) override;
    int32_t GetSysVpnCertUri(const int32_t certType, std::string &certUri) override;
#endif // SUPPORT_SYSVPN

private:
    int32_t WriteTokenAndSendRequest(INetworkVpnService::MessageCode code, MessageParcel &data, MessageParcel &reply);
    int32_t SendRequest(INetworkVpnService::MessageCode code, MessageParcel &data, MessageParcel &reply);

private:
    static inline BrokerDelegator<NetworkVpnServiceProxy> delegator_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETWORKVPN_SERVICE_PROXY_H
