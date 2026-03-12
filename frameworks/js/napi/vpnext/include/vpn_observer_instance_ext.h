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

#ifndef NETMANAGER_EXT_VPN_OBSERVER_INSTANCES_H
#define NETMANAGER_EXT_VPN_OBSERVER_INSTANCES_H

#include <map>

#include "event_manager.h"
#include "vpn_observer_callback.h"

namespace OHOS::NetManagerStandard {
class VpnObserverInstance final {
public:
    VpnObserverInstance(napi_env env, std::shared_ptr<EventManager>& eventManager);
    ~VpnObserverInstance() = default;
    [[nodiscard]] napi_env GetNapiEnv() const;
    [[nodiscard]] sptr<VpnObserver> GetObserver() const;
    [[nodiscard]] std::shared_ptr<EventManager> GetEventManager() const;

    static VpnObserverInstance *MakeVpnObserver(napi_env env, std::shared_ptr<EventManager>& eventManager);
    static void DeleteVpnObserver(VpnObserverInstance *vpnObserverInstance);
    static std::map<VpnObserver *, VpnObserverInstance *> observerInstanceMap_;
    static std::mutex g_vpnObserverMutex;

private:
    napi_env env_;
    sptr<VpnObserver> observer_;
    std::shared_ptr<EventManager> manager_ = nullptr;
};
} // namespace OHOS::NetManagerStandard
#endif /* NETMANAGER_EXT_VPN_OBSERVER_INSTANCES_H */
