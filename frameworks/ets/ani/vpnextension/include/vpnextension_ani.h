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

#ifndef NET_VPNEXTENSION_ANI_H
#define NET_VPNEXTENSION_ANI_H

#include <cstdint>
#include <memory>

#include "cxx.h"
#include "inet_addr.h"
#include "networkvpn_client.h"
#include "route.h"
#include "vpn_config.h"
#include "net_manager_ext_constants.h"
#include "wrapper.rs.h"

namespace OHOS {
namespace NetManagerAni {

sptr<NetManagerStandard::VpnConfig> ConvertToVpnConfig(const VpnConfigData &data);

int32_t StartVpnExtensionAbility(AniEnv **env, AniObject *wantObj);
int32_t StopVpnExtensionAbility(AniEnv **env, AniObject *wantObj);
int32_t SetAlwaysOnVpnEnabled(bool enable, const rust::String &bundleName);
int32_t IsAlwaysOnVpnEnabled(const rust::String &bundleName, bool &isEnabled);
int32_t UpdateVpnAuthorizedState(const rust::String &bundleName, bool &state);
int32_t CreateVpnConnection(AniEnv **env, AniObject *contextObj, int64_t &ptr);
int32_t DestroyVpnConnection(const int64_t ptr);
int32_t Create(const int64_t ptr, const VpnConfigData &config, int32_t &fd);
int32_t Protect(const int64_t ptr, const int32_t socketFd);
int32_t DestroyVpn(const int64_t ptr, const rust::String &vpnId);
int32_t ProtectProcessNet(const int64_t ptr);
int32_t GenerateVpnId(const int64_t ptr, rust::String &vpnId);

int32_t CreateVpnObserver(int64_t &ptr);
int32_t DestroyVpnObserver(int64_t ptr);

rust::String GetErrorCodeAndMessage(int32_t &errorCode);

} // namespace NetManagerAni
} // namespace OHOS
#endif // NET_VPNEXTENSION_ANI_H
