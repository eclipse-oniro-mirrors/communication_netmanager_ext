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
    std::unique_ptr<bool> data;
    std::shared_ptr<EventManager> manager;
    napi_async_work work;
    
    VpnAuthorizationContext() : env(nullptr), work(nullptr) {}
    ~VpnAuthorizationContext() = default;
};

static void VpnAuthorizationExecute(napi_env env, void *data)
{
    NETMANAGER_EXT_LOG_D("VpnAuthorizationExecute enter");
}

static void VpnAuthorizationComplete(napi_env env, napi_status status, void *data)
{
    if (status != napi_ok) {
        NETMANAGER_EXT_LOGE("VpnAuthorizationComplete status failed: %{public}d", status);
        auto *context = reinterpret_cast<VpnAuthorizationContext *>(data);
        if (context) {
            delete context;
        }
        return;
    }

    auto *context = reinterpret_cast<VpnAuthorizationContext *>(data);
    if (context == nullptr) {
        NETMANAGER_EXT_LOGE("VpnAuthorizationComplete context is nullptr!");
        return;
    }
    if (context->manager == nullptr) {
        NETMANAGER_EXT_LOGE("VpnAuthorizationComplete manager is nullptr!");
        delete context;
        return;
    }

    napi_handle_scope scope = NapiUtils::OpenScope(env);
    napi_value isAuthorized = NapiUtils::GetBoolean(env, *(context->data));
    
    std::pair<napi_value, napi_value> arg = {NapiUtils::GetUndefined(env), isAuthorized};
    context->manager->Emit(context->type, arg);
    NapiUtils::CloseScope(env, scope);

    if (context->work != nullptr) {
        napi_delete_async_work(env, context->work);
    }
    delete context;
}

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

static VpnAuthorizationContext *CreateAuthorizationContext(napi_env env,
    std::shared_ptr<EventManager> &manager, bool isAuthorized)
{
    auto *context = new VpnAuthorizationContext();
    context->env = env;
    context->type = EVENT_AUTHORIZATION;
    context->manager = manager;
    context->data = std::make_unique<bool>(isAuthorized);
    return context;
}

static bool CreateResourceName(napi_env env, napi_value &resourceName)
{
    napi_status status = napi_create_string_utf8(env, "VpnAuthorizationEvent",
        NAPI_AUTO_LENGTH, &resourceName);
    if (status != napi_ok) {
        NETMANAGER_EXT_LOGE("HandleResult napi_create_string_utf8 FAILED: %{public}d", status);
        return false;
    }
    return true;
}

static bool CreateAsyncWork(napi_env env, napi_value resourceName, VpnAuthorizationContext *context)
{
    napi_status status = napi_create_async_work(env, nullptr, resourceName,
        VpnAuthorizationExecute, VpnAuthorizationComplete, context, &context->work);
    if (status != napi_ok) {
        NETMANAGER_EXT_LOGE("HandleResult napi_create_async_work FAILED: %{public}d", status);
        return false;
    }
    return true;
}

static bool QueueAsyncWork(napi_env env, VpnAuthorizationContext *context)
{
    napi_status status = napi_queue_async_work(env, context->work);
    if (status != napi_ok) {
        NETMANAGER_EXT_LOGE("HandleResult napi_queue_async_work FAILED: %{public}d", status);
        if (context->work != nullptr) {
            napi_delete_async_work(env, context->work);
        }
        return false;
    }
    return true;
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

    VpnAuthorizationContext *context = CreateAuthorizationContext(env, manager, isAuthorized);
    if (context == nullptr) {
        return 0;
    }

    napi_value resourceName;
    if (!CreateResourceName(env, resourceName)) {
        delete context;
        return 0;
    }

    if (!CreateAsyncWork(env, resourceName, context)) {
        delete context;
        return 0;
    }

    if (!QueueAsyncWork(env, context)) {
        delete context;
        return 0;
    }
    return 0;
}
} // namespace NetManagerStandard
} // namespace OHOS
