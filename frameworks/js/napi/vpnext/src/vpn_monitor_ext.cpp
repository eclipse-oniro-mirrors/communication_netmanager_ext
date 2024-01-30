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

#include "vpn_monitor_ext.h"

#include <cstddef>
#include <string>

#include <napi/native_common.h>
#include <uv.h>

#include "module_template.h"
#include "napi_utils.h"
#include "netmanager_ext_log.h"
#include "networkvpn_client.h"
#include "want.h"
#include "ability_manager_client.h"
#include "extension_ability_info.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr const char *CONNECT = "connect";
constexpr int32_t PARAM_JUST_OPTIONS = 1;
constexpr int32_t PARAM_OPTIONS_AND_CALLBACK = 2;

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

void VpnEventCallback::OnVpnStateChanged(const bool &isConnected)
{
    auto manager = VpnMonitor::GetInstance().GetManager();
    bool *data = new bool(isConnected);
    manager->EmitByUv(CONNECT, reinterpret_cast<void *>(data), EventConnectCallback);
}

VpnMonitor &VpnMonitor::GetInstance()
{
    static VpnMonitor instance;
    return instance;
}

napi_value VpnMonitor::On(napi_env env, napi_callback_info info)
{
    if (!ParseParams(env, info)) {
        NETMANAGER_EXT_LOGE("parse failed");
        NAPI_CALL(env, napi_throw_error(env, "0", "parse failed"));
        return NapiUtils::GetUndefined(env);
    }
    Register(env);
    return NapiUtils::GetUndefined(env);
}

napi_value VpnMonitor::Off(napi_env env, napi_callback_info info)
{
    if (!ParseParams(env, info)) {
        NETMANAGER_EXT_LOGE("parse failed");
        NAPI_CALL(env, napi_throw_error(env, "0", "parse failed"));
        return NapiUtils::GetUndefined(env);
    }
    Unregister(env);
    return NapiUtils::GetUndefined(env);
}

bool VpnMonitor::ParseParams(napi_env env, napi_callback_info info)
{
    napi_value jsObject = nullptr;
    size_t paramsCount = MAX_PARAM_NUM;
    napi_value params[MAX_PARAM_NUM] = {nullptr};
    NAPI_CALL_BASE(env, napi_get_cb_info(env, info, &paramsCount, params, &jsObject, nullptr), false);

    if (!CheckParamType(env, params, paramsCount)) {
        return false;
    }
    if (!UnwrapManager(env, jsObject)) {
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

bool VpnMonitor::UnwrapManager(napi_env env, napi_value jsObject)
{
    NAPI_CALL_BASE(env, napi_unwrap(env, jsObject, reinterpret_cast<void **>(&manager_)), false);
    if (manager_ == nullptr) {
        return false;
    }
    return true;
}

void VpnMonitor::Register(napi_env env)
{
    auto vpnClient = reinterpret_cast<NetworkVpnClient *>(manager_->GetData());
    if (vpnClient == nullptr) {
        NETMANAGER_EXT_LOGE("vpnClient is nullptr");
        return;
    }
    manager_->AddListener(env, CONNECT, callback_, false, false);

    if (eventCallback_ != nullptr) {
        vpnClient->UnregisterVpnEvent(eventCallback_);
    }
    eventCallback_ = new (std::nothrow) VpnEventCallback();
    if (nullptr == eventCallback_) {
        NETMANAGER_EXT_LOGE("eventCallback_ is nullptr");
        return;
    }
    vpnClient->RegisterVpnEvent(eventCallback_);
}

void VpnMonitor::Unregister(napi_env env)
{
    auto vpnClient = reinterpret_cast<NetworkVpnClient *>(manager_->GetData());
    if (vpnClient == nullptr) {
        NETMANAGER_EXT_LOGE("vpnClient is nullptr");
        return;
    }
    manager_->DeleteListener(CONNECT);
    vpnClient->UnregisterVpnEvent(eventCallback_);
}

bool VpnMonitor::ShowVpnDialog(const std::string &bundleName, const std::string &abilityName)
{
    auto abmc = AAFwk::AbilityManagerClient::GetInstance();
    if (abmc == nullptr) {
        NETMANAGER_EXT_LOGE("GetInstance failed");
        return false;
    }

    AAFwk::Want want;
    want.SetElementName("com.ohos.vpndialog", "VpnServiceExtAbility");
    want.SetParam("bundleName", bundleName);
    want.SetParam("abilityName", abilityName + VPN_DIALOG_POSTFIX);

    sptr<VpnMonitor::VpnAbilityConn> vpnAbilityConn_ = new (std::nothrow) VpnMonitor::VpnAbilityConn();
    auto ret = abmc->ConnectAbility(want, vpnAbilityConn_, -1);
    if (ret != 0) {
        NETMANAGER_EXT_LOGE("connectAbility failed %{public}d", ret);
        return false;
    }
    /* Waiting for the user to click */
    NETMANAGER_EXT_LOGI("click done");
    return true;
}
} // namespace NetManagerStandard
} // namespace OHOS