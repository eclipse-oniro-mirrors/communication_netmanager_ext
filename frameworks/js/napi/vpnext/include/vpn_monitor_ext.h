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

#ifndef NET_EXT_NAPI_VPN_MONITOR_H
#define NET_EXT_NAPI_VPN_MONITOR_H

#include <napi/native_api.h>
#include <refbase.h>

#include "event_manager.h"
#include "vpn_event_callback_stub.h"
#include "ability_connect_callback_stub.h"
#include "ability_manager_client.h"
#include "netmanager_ext_log.h"

namespace OHOS {
namespace NetManagerStandard {
/*
 * There is several functionalities of this string:
 * 1: to identify the application which called StartVpnExtensionAbility is the VPN APP or VPNDialog APP.
 * 2: to identify VPNDialog APP call UpdateVpnAuthorize in order to update vpnExtMode to 1 or 0.
 */
static constexpr const char *VPN_DIALOG_POSTFIX = "**vpndialog**";
class VpnEventCallback : public VpnEventCallbackStub {
public:
    void OnVpnStateChanged(const bool &isConnected) override;
    void OnVpnMultiUserSetUp() override{};
};

class VpnMonitor {
private:
    VpnMonitor() = default;
    ~VpnMonitor() = default;
    VpnMonitor(const VpnMonitor &) = delete;
    VpnMonitor &operator=(const VpnMonitor &) = delete;

public:
    static VpnMonitor &GetInstance();

public:
    napi_value On(napi_env env, napi_callback_info info);
    napi_value Off(napi_env env, napi_callback_info info);
    bool ShowVpnDialog(const std::string &bundleName, const std::string &abilityName);

    EventManager inline *GetManager() const
    {
        return manager_;
    }

private:
    sptr<VpnEventCallback> eventCallback_ = nullptr;
    napi_value callback_ = nullptr;
    EventManager *manager_ = nullptr;

class VpnAbilityConn : public AAFwk::AbilityConnectionStub {
    void OnAbilityConnectDone(const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject,
        int32_t resultCode) override
    {
        NETMANAGER_EXT_LOGI("connect done");
    }
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode) override
    {
        NETMANAGER_EXT_LOGI("disconnect done");
    }
};

private:
    bool ParseParams(napi_env env, napi_callback_info info);
    bool UnwrapManager(napi_env env, napi_value jsObject);
    void Register(napi_env env);
    void Unregister(napi_env env);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_EXT_NAPI_VPN_MONITOR_H