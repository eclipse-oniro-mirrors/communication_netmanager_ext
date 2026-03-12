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

#include <mutex>

#include "vpn_observer_instance_ext.h"
#include "net_manager_constants.h"
#include "netmanager_ext_log.h"

namespace OHOS::NetManagerStandard {
std::map<VpnObserver *, VpnObserverInstance *> VpnObserverInstance::observerInstanceMap_;
std::mutex VpnObserverInstance::g_vpnObserverMutex;

VpnObserverInstance::VpnObserverInstance(napi_env env, std::shared_ptr<EventManager>& eventManager)
    :  env_(env), observer_(sptr<VpnObserver>::MakeSptr()), manager_(eventManager)
{
}

VpnObserverInstance *VpnObserverInstance::MakeVpnObserver(napi_env env, std::shared_ptr<EventManager>& eventManager)
{
    std::lock_guard<std::mutex> lock{VpnObserverInstance::g_vpnObserverMutex};
    auto vpnObserverInstance = new VpnObserverInstance(env, eventManager);
    if (vpnObserverInstance->observer_ == nullptr) {
        NETMANAGER_EXT_LOGE("vpnObserverInstance->observer_ is nullptr");
        delete vpnObserverInstance;
        return nullptr;
    }
    observerInstanceMap_[vpnObserverInstance->observer_.GetRefPtr()] = vpnObserverInstance;
    return vpnObserverInstance;
}

void VpnObserverInstance::DeleteVpnObserver(VpnObserverInstance *vpnObserverInstance)
{
    if (vpnObserverInstance == nullptr) {
        NETMANAGER_EXT_LOGE("vpnObserverInstance is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock{VpnObserverInstance::g_vpnObserverMutex};
    
    if (vpnObserverInstance->manager_ != nullptr) {
        vpnObserverInstance->manager_->DeleteAllListener();
    }
    
    if (vpnObserverInstance->observer_ != nullptr) {
        observerInstanceMap_.erase(vpnObserverInstance->observer_.GetRefPtr());
    }
    delete vpnObserverInstance;
}

napi_env VpnObserverInstance::GetNapiEnv() const
{
    return env_;
}

sptr<VpnObserver> VpnObserverInstance::GetObserver() const
{
    return observer_;
}

std::shared_ptr<EventManager> VpnObserverInstance::GetEventManager() const
{
    return manager_;
}
} // namespace OHOS::NetManagerStandard
