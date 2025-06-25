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

#include "vpn_monitor.h"

#include <cstddef>
#include <string>
#ifdef SUPPORT_SYSVPN
#include <utility>
#endif // SUPPORT_SYSVPN

#include <napi/native_common.h>
#include <uv.h>

#include "module_template.h"
#include "napi_utils.h"
#include "netmanager_ext_log.h"
#include "networkvpn_client.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr const char *CONNECT = "connect";
#ifdef SUPPORT_SYSVPN
constexpr const char *CONNECT_MULTI = "connectMulti";
#endif // SUPPORT_SYSVPN
constexpr int32_t PARAM_JUST_OPTIONS = 1;
constexpr int32_t PARAM_OPTIONS_AND_CALLBACK = 2;

#ifdef SUPPORT_SYSVPN
struct MultiVpnData {
    bool isConnected;
    std::string bundleName;
    std::string vpnId;
};
#endif // SUPPORT_SYSVPN

void EventConnectCallback(uv_work_t *work, int status)
{
    if (work == nullptr) {
        NETMANAGER_EXT_LOGE("work is nullptr");
        return;
    }
    auto workWrapper = reinterpret_cast<UvWorkWrapper *>(work->data);
    if (workWrapper == nullptr) {
        NETMANAGER_EXT_LOGE("workWrapper is nullptr");
        delete work;
        return;
    }
    bool *data = reinterpret_cast<bool *>(workWrapper->data);
    if (data == nullptr) {
        NETMANAGER_EXT_LOGE("isConnected is nullptr");
        delete workWrapper;
        delete work;
        return;
    }

    napi_env env = workWrapper->env;
    napi_handle_scope scope = NapiUtils::OpenScope(env);
    napi_value isConnected = NapiUtils::GetBoolean(env, *data);
    napi_value result = NapiUtils::CreateObject(env);
    NapiUtils::SetNamedProperty(env, result, "isConnected", isConnected);
    workWrapper->manager->Emit(CONNECT, std::make_pair(NapiUtils::GetUndefined(env), result));
    NapiUtils::CloseScope(env, scope);
    delete data;
    delete workWrapper;
    delete work;
}

#ifdef SUPPORT_SYSVPN
void EventConnectMultiCallback(uv_work_t *work, int status)
{
    if (work == nullptr) {
        NETMANAGER_EXT_LOGE("work is nullptr");
        return;
    }
    auto workWrapper = reinterpret_cast<UvWorkWrapper *>(work->data);
    if (workWrapper == nullptr) {
        NETMANAGER_EXT_LOGE("workWrapper is nullptr");
        delete work;
        return;
    }
    MultiVpnData *data = reinterpret_cast<MultiVpnData *>(workWrapper->data);
    if (data == nullptr) {
        NETMANAGER_EXT_LOGE("MultiVpnData is nullptr");
        delete workWrapper;
        delete work;
        return;
    }

    napi_env env = workWrapper->env;
    napi_handle_scope scope = NapiUtils::OpenScope(env);
    napi_value obj = NapiUtils::CreateObject(env);
    NapiUtils::SetBooleanProperty(env, obj, "isConnected", data->isConnected);
    NapiUtils::SetStringPropertyUtf8(env, obj, "bundleName", data->bundleName);
    NapiUtils::SetStringPropertyUtf8(env, obj, "vpnId", data->vpnId);
    workWrapper->manager->Emit(CONNECT_MULTI, std::make_pair(NapiUtils::GetUndefined(env), obj));
    NapiUtils::CloseScope(env, scope);
    delete data;
    delete workWrapper;
    delete work;
}
#endif // SUPPORT_SYSVPN

bool CheckParamType(napi_env env, napi_value *params, size_t paramsCount)
{
    switch (paramsCount) {
        case PARAM_JUST_OPTIONS:
            return (NapiUtils::GetValueType(env, params[0]) == napi_string);
        case PARAM_OPTIONS_AND_CALLBACK:
            return ((NapiUtils::GetValueType(env, params[0]) == napi_string) &&
                    (NapiUtils::GetValueType(env, params[1]) == napi_function));
        default:
            return false;
    }
}
} // namespace

int32_t VpnEventCallback::OnVpnStateChanged(bool isConnected)
{
    auto manager = VpnMonitor::GetInstance().GetManager();
    bool *data = new bool(isConnected);
    manager->EmitByUv(CONNECT, reinterpret_cast<void *>(data), EventConnectCallback);
    return ERR_OK;
}

int32_t VpnEventCallback::OnMultiVpnStateChanged(bool isConnected, const std::string &bundleName,
    const std::string &vpnId)
{
#ifdef SUPPORT_SYSVPN
    auto manager = VpnMonitor::GetInstance().GetManager();
    MultiVpnData *data = new MultiVpnData();
    data->isConnected = isConnected;
    data->bundleName = bundleName;
    data->vpnId = vpnId;
    manager->EmitByUv(CONNECT_MULTI, reinterpret_cast<void *>(data), EventConnectMultiCallback);
#endif // SUPPORT_SYSVPN
    return ERR_OK;
}

VpnMonitor::VpnMonitor()
{
    manager_ = std::make_shared<EventManager>();
}

VpnMonitor::~VpnMonitor() = default;

VpnMonitor &VpnMonitor::GetInstance()
{
    static VpnMonitor instance;
    return instance;
}

napi_value VpnMonitor::On(napi_env env, napi_callback_info info)
{
#ifdef SUPPORT_SYSVPN
    std::string event;
    if (!ParseParams(env, info, event)) {
        NETMANAGER_EXT_LOGE("parse failed");
        NAPI_CALL(env, napi_throw_error(env, "0", "parse failed"));
        return NapiUtils::GetUndefined(env);
    }
    Register(env, event);
#else
    if (!ParseParams(env, info)) {
        NETMANAGER_EXT_LOGE("parse failed");
        NAPI_CALL(env, napi_throw_error(env, "0", "parse failed"));
        return NapiUtils::GetUndefined(env);
    }
    Register(env);
#endif // SUPPORT_SYSVPN
    return NapiUtils::GetUndefined(env);
}

napi_value VpnMonitor::Off(napi_env env, napi_callback_info info)
{
#ifdef SUPPORT_SYSVPN
    std::string event;
    if (!ParseParams(env, info, event)) {
        NETMANAGER_EXT_LOGE("parse failed");
        NAPI_CALL(env, napi_throw_error(env, "0", "parse failed"));
        return NapiUtils::GetUndefined(env);
    }
    Unregister(env, event);
#else
    if (!ParseParams(env, info)) {
        NETMANAGER_EXT_LOGE("parse failed");
        NAPI_CALL(env, napi_throw_error(env, "0", "parse failed"));
        return NapiUtils::GetUndefined(env);
    }
    Unregister(env);
#endif // SUPPORT_SYSVPN
    return NapiUtils::GetUndefined(env);
}

#ifdef SUPPORT_SYSVPN
bool VpnMonitor::ParseParams(napi_env env, napi_callback_info info, std::string &event)
{
    napi_value jsObject = nullptr;
    size_t paramsCount = MAX_PARAM_NUM;
    napi_value params[MAX_PARAM_NUM] = {nullptr};
    NAPI_CALL_BASE(env, napi_get_cb_info(env, info, &paramsCount, params, &jsObject, nullptr), false);

    if (!CheckParamType(env, params, paramsCount)) {
        NETMANAGER_EXT_LOGE("CheckParamType failed");
        return false;
    }
    if (manager_ == nullptr) {
        NETMANAGER_EXT_LOGE("manager_ is nullptr");
        return false;
    }
    const std::string eventType = NapiUtils::GetStringFromValueUtf8(env, params[0]);
    if (CONNECT != eventType && CONNECT_MULTI != eventType) {
        NETMANAGER_EXT_LOGE("%{public}s eventtyep is error", eventType.c_str());
        return false;
    }
    event = eventType;
    if (paramsCount == PARAM_OPTIONS_AND_CALLBACK) {
        callback_ = params[1];
    }
    return true;
}

void VpnMonitor::Register(napi_env env, const std::string &event)
{
    if (manager_ == nullptr) {
        NETMANAGER_EXT_LOGE("manager_ is nullptr");
        return;
    }
    manager_->AddListener(env, event, callback_, false, false);
    if (CONNECT == event) {
        if (eventCallback_ != nullptr) {
            NetworkVpnClient::GetInstance().UnregisterVpnEvent(eventCallback_);
        }
        eventCallback_ = new (std::nothrow) VpnEventCallback();
        if (nullptr == eventCallback_) {
            NETMANAGER_EXT_LOGE("eventCallback_ is nullptr");
            return;
        }
        NetworkVpnClient::GetInstance().RegisterVpnEvent(eventCallback_);
        return;
    }
    if (CONNECT_MULTI == event) {
        if (multiEventCallback_ != nullptr) {
            NetworkVpnClient::GetInstance().UnregisterMultiVpnEvent(multiEventCallback_);
        }
        multiEventCallback_ = new (std::nothrow) VpnEventCallback();
        if (nullptr == multiEventCallback_) {
            NETMANAGER_EXT_LOGE("eventCallback_ is nullptr");
            return;
        }
        NetworkVpnClient::GetInstance().RegisterMultiVpnEvent(multiEventCallback_);
        return;
    }
}

void VpnMonitor::Unregister(napi_env env, const std::string &event)
{
    if (manager_ == nullptr) {
        NETMANAGER_EXT_LOGE("manager_ is nullptr");
        return;
    }
    manager_->DeleteListener(event);
    if (CONNECT == event) {
        NetworkVpnClient::GetInstance().UnregisterVpnEvent(eventCallback_);
        return;
    }
    if (CONNECT_MULTI == event) {
        NetworkVpnClient::GetInstance().UnregisterMultiVpnEvent(multiEventCallback_);
        return;
    }
}
#endif // SUPPORT_SYSVPN

bool VpnMonitor::ParseParams(napi_env env, napi_callback_info info)
{
    napi_value jsObject = nullptr;
    size_t paramsCount = MAX_PARAM_NUM;
    napi_value params[MAX_PARAM_NUM] = {nullptr};
    NAPI_CALL_BASE(env, napi_get_cb_info(env, info, &paramsCount, params, &jsObject, nullptr), false);

    if (!CheckParamType(env, params, paramsCount)) {
        NETMANAGER_EXT_LOGE("CheckParamType failed");
        return false;
    }
    if (manager_ == nullptr) {
        NETMANAGER_EXT_LOGE("manager_ is nullptr");
        return false;
    }
    const std::string event = NapiUtils::GetStringFromValueUtf8(env, params[0]);
    if (CONNECT != event) {
        NETMANAGER_EXT_LOGE("%{public}s event is error", event.c_str());
        return false;
    }
    if (paramsCount == PARAM_OPTIONS_AND_CALLBACK) {
        callback_ = params[1];
    }
    return true;
}

void VpnMonitor::Register(napi_env env)
{
    if (manager_ == nullptr) {
        NETMANAGER_EXT_LOGE("manager_ is nullptr");
        return;
    }
    manager_->AddListener(env, CONNECT, callback_, false, false);

    if (eventCallback_ != nullptr) {
        NetworkVpnClient::GetInstance().UnregisterVpnEvent(eventCallback_);
    }
    eventCallback_ = new (std::nothrow) VpnEventCallback();
    if (nullptr == eventCallback_) {
        NETMANAGER_EXT_LOGE("eventCallback_ is nullptr");
        return;
    }
    NetworkVpnClient::GetInstance().RegisterVpnEvent(eventCallback_);
}

void VpnMonitor::Unregister(napi_env env)
{
    if (manager_ == nullptr) {
        NETMANAGER_EXT_LOGE("manager_ is nullptr");
        return;
    }
    manager_->DeleteListener(CONNECT);
    NetworkVpnClient::GetInstance().UnregisterVpnEvent(eventCallback_);
}
} // namespace NetManagerStandard
} // namespace OHOS