/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef NET_VPN_ANI_H
#define NET_VPN_ANI_H

#include <cstdint>
#include <memory>

#include "cxx.h"
#include "inet_addr.h"
#include "ipsecvpn_config.h"
#include "l2tpvpn_config.h"
#include "networkvpn_client.h"
#include "openvpn_config.h"
#include "route.h"
#include "sysvpn_config.h"
#include "vpn_config.h"
#include "vpn_event_callback_stub.h"
#include "net_manager_ext_constants.h"
#include "wrapper.rs.h"

namespace OHOS {
namespace NetManagerAni {

sptr<NetManagerStandard::VpnConfig> ConvertToVpnConfig(const VpnConfigData &data);
sptr<NetManagerStandard::SysVpnConfig> ConvertToSysVpnConfig(const SysVpnConfigData &data);
SysVpnConfigData ConvertFromSysVpnConfig(const NetManagerStandard::SysVpnConfig &config);

bool CreateVpnConnection(int32_t &ret);
int32_t SetUp(const VpnConfigData &config, int32_t &fd);
int32_t Protect(int32_t socketFd);
int32_t Destroy();
int32_t AddSysVpnConfig(const SysVpnConfigData &config);
int32_t DeleteSysVpnConfig(const rust::String &vpnId);
rust::Vec<SysVpnConfigData> GetSysVpnConfigList(int32_t &ret);
SysVpnConfigData GetSysVpnConfig(const rust::String &vpnId, int32_t &ret);
SysVpnConfigData GetConnectedSysVpnConfig(int32_t &ret);
rust::Vec<rust::String> GetConnectedVpnAppInfo(int32_t &ret);

class VpnEventCallbackObserverAni : public NetManagerStandard::VpnEventCallbackStub {
public:
    ErrCode OnVpnStateChanged(bool isConnected, const sptr<NetManagerStandard::VpnState> &vpnState) override;
    ErrCode OnMultiVpnStateChanged(bool isConnected, const std::string &bundleName,
        const std::string &vpnId) override;
    ErrCode OnVpnMultiUserSetUp() override { return 0; }
};

int32_t VpnObserverRegister();
int32_t VpnObserverUnRegister();
rust::String GetErrorCodeAndMessage(int32_t &errorCode);

} // namespace NetManagerAni
} // namespace OHOS
#endif // NET_VPN_ANI_H
