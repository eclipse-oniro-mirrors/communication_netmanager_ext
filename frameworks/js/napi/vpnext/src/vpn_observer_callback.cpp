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

#include <vector>

#include "net_manager_constants.h"
#include "netmanager_ext_log.h"

#include "vpn_observer_callback.h"
#include "vpn_observer_instance_ext.h"
#include "vpn_connection_ext.h"

namespace OHOS {
namespace NetManagerStandard {
struct VpnAuthorizationContext {
    napi_env env;
    std::string type;
    bool data;
    std::shared_ptr<EventManager> manager;

    VpnAuthorizationContext() : env(nullptr), data(false) {}
    ~VpnAuthorizationContext() = default;
};

static bool GetVpnObserverInstance(VpnObserver *observer, VpnObserverInstance *&instance)
{
    std::lock_guard<std::mutex> lock{VpnObserverInstance::g_vpnObserverMutex};
    auto it = VpnObserverInstance::observerInstanceMap_.find(observer);
    if (it == VpnObserverInstance::observerInstanceMap_.end()) {
        NETMANAGER_EXT_LOGE("can not find VpnObserverInstance handle");
        return false;
    }
    instance = it->second;
    return true;
}

static bool CheckVpnObserverInstance(
    VpnObserverInstance *instance, napi_env &env, std::shared_ptr<EventManager> &manager)
{
    if (instance == nullptr) {
        NETMANAGER_EXT_LOGE("HandleResult vpnObserverInstance is nullptr");
        return false;
    }

    manager = instance->GetEventManager();
    if (manager == nullptr) {
        NETMANAGER_EXT_LOGE("HandleResult manager is nullptr");
        return false;
    }
    if (!manager->HasEventListener(EVENT_AUTHORIZATION)) {
        NETMANAGER_EXT_LOGE("no event listener find %{public}s", EVENT_AUTHORIZATION);
        return false;
    }

    env = instance->GetNapiEnv();
    if (env == nullptr) {
        NETMANAGER_EXT_LOGE("HandleResult env is nullptr!");
        return false;
    }
    return true;
}

static std::shared_ptr<VpnAuthorizationContext> CreateAuthorizationContext(
    napi_env env, std::shared_ptr<EventManager> &manager, bool isAuthorized)
{
    auto context = std::make_shared<VpnAuthorizationContext>();
    context->env = env;
    context->type = EVENT_AUTHORIZATION;
    context->manager = manager;
    context->data = isAuthorized;
    return context;
}

int32_t VpnObserver::HandleAuthorizeResult(bool isAuthorized)
{
    VpnObserverInstance *vpnObserverInstance = nullptr;
    if (!GetVpnObserverInstance(this, vpnObserverInstance)) {
        return 0;
    }

    napi_env env = nullptr;
    std::shared_ptr<EventManager> manager = nullptr;
    if (!CheckVpnObserverInstance(vpnObserverInstance, env, manager)) {
        return 0;
    }

    auto context = CreateAuthorizationContext(env, manager, isAuthorized);
    if (context == nullptr) {
        return 0;
    }

    napi_send_event(
        env, [context]() {
            if (context->manager == nullptr) {
                NETMANAGER_EXT_LOGE("VpnAuthorizationEvent manager is nullptr!");
                return;
            }

            napi_handle_scope scope = NapiUtils::OpenScope(context->env);
            napi_value isAuthorized = NapiUtils::GetBoolean(context->env, context->data);

            std::pair<napi_value, napi_value> arg = {NapiUtils::GetUndefined(context->env), isAuthorized};
            context->manager->Emit(context->type, arg);
            NapiUtils::CloseScope(context->env, scope);
        },
        napi_eprio_high);
    return 0;
}
} // namespace NetManagerStandard
} // namespace OHOS
