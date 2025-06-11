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

#include <cstdint>
#include <memory>
#include <mutex>

#include "ability_manager_client.h"
#include "errorcode_convertor.h"
#include "extension_ability_info.h"
#include "module_template.h"
#include "napi/native_node_api.h"
#include "napi_common_want.h"
#include "napi_utils.h"
#include "net_datashare_utils_iface.h"
#include "net_manager_constants.h"
#include "netmanager_ext_log.h"
#include "networkvpn_client.h"
#include "vpn_connection_ext.h"
#include "vpn_extension_context.h"
#include "vpn_monitor_ext.h"
#include "want.h"
#ifdef SUPPORT_SYSVPN
#include "ipc_skeleton.h"
#include "os_account_manager.h"
#endif // SUPPORT_SYSVPN

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t ARG_NUM_0 = 0;
constexpr int32_t PARAM_ONE = 1;

static napi_value CreateResolvedPromise(napi_env env)
{
    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    if (napi_create_promise(env, &deferred, &promise) != napi_ok) {
        return NapiUtils::GetUndefined(env);
    }
    napi_resolve_deferred(env, deferred, NapiUtils::GetUndefined(env));
    return promise;
}

static napi_value CreateRejectedPromise(napi_env env)
{
    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    if (napi_create_promise(env, &deferred, &promise) != napi_ok) {
        return NapiUtils::GetUndefined(env);
    }
    napi_reject_deferred(env, deferred, NapiUtils::GetUndefined(env));
    return promise;
}

static void ResolvePromiseInIpcThread(napi_env env, napi_deferred deferred)
{
    napi_send_event(
        env, [env, deferred]() { napi_resolve_deferred(env, deferred, NapiUtils::GetUndefined(env)); },
        napi_eprio_high);
}

static void RejectPromiseInIpcThread(napi_env env, napi_deferred deferred)
{
    napi_send_event(
        env, [env, deferred]() { napi_reject_deferred(env, deferred, NapiUtils::GetUndefined(env)); }, napi_eprio_high);
}

static napi_value CreateObserveDataSharePromise(napi_env env, const std::string &bundleName)
{
    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    if (napi_create_promise(env, &deferred, &promise) != napi_ok) {
        return NapiUtils::GetUndefined(env);
    }

    auto once = std::make_shared<std::once_flag>();
    auto callbackId = std::make_shared<int32_t>();
    auto deferWrapper = std::make_shared<napi_deferred>();
    *deferWrapper = deferred;
    auto onChange = [env, deferWrapper, bundleName, once, callbackId]() {
        if (!once) {
            return;
        }
        std::call_once(*once, [env, deferWrapper, bundleName, callbackId]() {
            bool vpnDialogSelect = false;
            std::string vpnExtMode = std::to_string(vpnDialogSelect);
            int32_t ret = NetDataShareHelperUtilsIface::Query(VPNEXT_MODE_URI, bundleName, vpnExtMode);
            NETMANAGER_EXT_LOGI("query vpn state after dialog: %{public}d %{public}s", ret, vpnExtMode.c_str());
            if (callbackId) {
                NetDataShareHelperUtilsIface::UnregisterObserver(VPNEXT_MODE_URI, *callbackId);
            }
            if (deferWrapper && *deferWrapper) {
                auto deferred = *deferWrapper;
                *deferWrapper = nullptr;
                if (vpnExtMode == "1") {
                    ResolvePromiseInIpcThread(env, deferred);
                } else {
                    RejectPromiseInIpcThread(env, deferred);
                }
            }
        });
    };

    *callbackId = NetDataShareHelperUtilsIface::RegisterObserver(VPNEXT_MODE_URI, onChange);
    return promise;
}

static void *MakeDataExt(napi_env env, size_t argc, napi_value *argv, EventManager *manager)
{
    if ((argc != PARAM_ONE) || (NapiUtils::GetValueType(env, argv[ARG_NUM_0]) != napi_object)) {
        NETMANAGER_EXT_LOGE("funciton prameter error");
        napi_throw_error(env, std::to_string(NETMANAGER_EXT_ERR_PARAMETER_ERROR).c_str(), "Parameter error");
        return nullptr;
    }

    VpnExtensionContext *vpnExtensionContext = nullptr;
    napi_status status = napi_unwrap(env, argv[ARG_NUM_0], reinterpret_cast<void **>(&vpnExtensionContext));
    if (status != napi_ok || vpnExtensionContext == nullptr) {
        NETMANAGER_EXT_LOGE("Failed to get vpnExtensionContext napi instance");
        napi_throw_error(env, std::to_string(NETMANAGER_EXT_ERR_PARAMETER_ERROR).c_str(), "Parameter error");
        return nullptr;
    }

    int32_t ret = NetworkVpnClient::GetInstance().CreateVpnConnection(true);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMANAGER_EXT_LOGE("execute CreateVpnConnection failed: %{public}d", ret);
        std::string errorMsg = NetBaseErrorCodeConvertor().ConvertErrorCode(ret);
        napi_throw_error(env, std::to_string(ret).c_str(), errorMsg.c_str());
        return nullptr;
    }
    return reinterpret_cast<void *>(&NetworkVpnClient::GetInstance());
}

static std::string Replace(std::string s)
{
    std::string tmp = VPN_DIALOG_POSTFIX;
    auto pos = s.find(tmp);
    if (pos == std::string::npos) {
        return s;
    }
    s.replace(pos, tmp.length(), "");
    return s;
}

#ifdef SUPPORT_SYSVPN
int32_t CheckVpnPermission(const std::string &bundleName, std::string &vpnExtMode)
{
    int32_t userId = AppExecFwk::Constants::UNSPECIFIED_USERID;
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(uid, userId) != ERR_OK) {
        NETMANAGER_EXT_LOGE("checkVpnPermission::GetOsAccountLocalIdFromUid error, uid: %{public}d.", uid);
        return -1;
    }
    NETMANAGER_EXT_LOGI("checkVpnPermission uid: %{public}d, userid: %{public}d", uid, userId);
    int32_t ret = NetDataShareHelperUtilsIface::Query(VPNEXT_MODE_URI, bundleName + std::to_string(userId), vpnExtMode);
    if (ret != 0 || vpnExtMode != "1") {
        ret = NetDataShareHelperUtilsIface::Query(VPNEXT_MODE_URI, bundleName, vpnExtMode);
        if (ret != 0 || vpnExtMode != "1") {
            NETMANAGER_EXT_LOGE("checkVpnPermission::dataShareHelperUtils Query error, err = %{public}d", ret);
            return -1;
        }
    }
    return 0;
}
#endif // SUPPORT_SYSVPN

napi_value ProcessPermissionRequests(napi_env env, const std::string &bundleName, const std::string &abilityName)
{
    std::string selfAppName;
    std::string selfBundleName;
    auto getAppNameRes = NetworkVpnClient::GetInstance().GetSelfAppName(selfAppName, selfBundleName);
    NETMANAGER_EXT_LOGI("StartVpnExtensionAbility SelfAppName = %{public}s %{public}d", selfAppName.c_str(),
        getAppNameRes);
    if (bundleName != selfBundleName) {
        NETMANAGER_EXT_LOGE("Not allowed to start other bundleName vpn!");
        return CreateRejectedPromise(env);
    }

    bool vpnDialogSelect = false;
    std::string vpnExtMode = std::to_string(vpnDialogSelect);
    int32_t ret = 0;
#ifdef SUPPORT_SYSVPN
    ret = CheckVpnPermission(bundleName, vpnExtMode);
#else
    ret = NetDataShareHelperUtilsIface::Query(VPNEXT_MODE_URI, bundleName, vpnExtMode);
#endif // SUPPORT_SYSVPN
    if (ret != 0 || vpnExtMode != "1") {
        NETMANAGER_EXT_LOGE("dataShareHelperUtils Query error, err = %{public}d", ret);
        VpnMonitor::GetInstance().ShowVpnDialog(bundleName, abilityName, selfAppName);
        return CreateObserveDataSharePromise(env, bundleName);
    }
    return nullptr;
}

napi_value StartVpnExtensionAbility(napi_env env, napi_callback_info info)
{
    napi_value thisVal = nullptr;
    std::size_t argc = MAX_PARAM_NUM;

    napi_value argv[MAX_PARAM_NUM] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVal, nullptr));
    if ((argc != PARAM_ONE) || (NapiUtils::GetValueType(env, argv[ARG_NUM_0]) != napi_object)) {
        NETMANAGER_EXT_LOGE("funciton prameter error");
        napi_throw_error(env, std::to_string(NETMANAGER_EXT_ERR_PARAMETER_ERROR).c_str(), "Parameter error");
        return CreateRejectedPromise(env);
    }
    AAFwk::Want want;
    int32_t accountId = -1;
    if (!AppExecFwk::UnwrapWant(env, argv[0], want)) {
        NETMANAGER_EXT_LOGE("Failed to parse want");
        napi_throw_error(env, std::to_string(NETMANAGER_EXT_ERR_PARAMETER_ERROR).c_str(), "Parse want error");
        return CreateRejectedPromise(env);
    }

    std::string bundleName = want.GetElement().GetBundleName();
    std::string abilityName = want.GetElement().GetAbilityName();
    if (abilityName.find(VPN_DIALOG_POSTFIX) == std::string::npos) {
        NetworkVpnClient::GetInstance().SetSelfVpnPid();
        napi_value retVal = ProcessPermissionRequests(env, bundleName, abilityName);
        if (retVal != nullptr) {
            return retVal;
        }
    }
    auto elem = want.GetElement();
    elem.SetAbilityName(Replace(abilityName));
    want.SetElement(elem);
    if (OHOS::system::GetBoolParameter("persist.edm.vpn_disable", false)) {
        napi_throw_error(env, std::to_string(NETMANAGER_EXT_ERR_PERMISSION_DENIED).c_str(),
            "persist.edm.vpn_disable disallowed setting up vpn");
        return CreateRejectedPromise(env);
    }
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartExtensionAbility(
        want, nullptr, accountId, AppExecFwk::ExtensionAbilityType::VPN);
    NETMANAGER_EXT_LOGI("execute StartVpnExtensionAbility result: %{public}d", err);
    if (err == 0) {
        int32_t rst = NetworkVpnClient::GetInstance().RegisterBundleName(bundleName, Replace(abilityName));
        NETMANAGER_EXT_LOGI("VPN RegisterBundleName result = %{public}d", rst);
    }
    return CreateResolvedPromise(env);
}

napi_value StopVpnExtensionAbility(napi_env env, napi_callback_info info)
{
    napi_value thisVal = nullptr;
    std::size_t argc = MAX_PARAM_NUM;

    napi_value argv[MAX_PARAM_NUM] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVal, nullptr));
    if ((argc != PARAM_ONE) || (NapiUtils::GetValueType(env, argv[ARG_NUM_0]) != napi_object)) {
        NETMANAGER_EXT_LOGE("funciton prameter error");
        napi_throw_error(env, std::to_string(NETMANAGER_EXT_ERR_PARAMETER_ERROR).c_str(), "Parameter error");
        return CreateRejectedPromise(env);
    }
    AAFwk::Want want;
    int32_t accountId = -1;
    if (!AppExecFwk::UnwrapWant(env, argv[0], want)) {
        NETMANAGER_EXT_LOGE("Failed to parse want");
        napi_throw_error(env, std::to_string(NETMANAGER_EXT_ERR_PARAMETER_ERROR).c_str(), "Parse want error");
        return CreateRejectedPromise(env);
    }

    std::string bundleName = want.GetElement().GetBundleName();
    std::string selfAppName;
    std::string selfBundleName;
    auto getAppNameRes = NetworkVpnClient::GetInstance().GetSelfAppName(selfAppName, selfBundleName);
    NETMANAGER_EXT_LOGI("StopVpnExtensionAbility SelfAppName = %{public}s %{public}d", selfAppName.c_str(),
        getAppNameRes);
    if (bundleName != selfBundleName) {
        NETMANAGER_EXT_LOGE("Not allowed to stop other bundleName vpn!");
        return CreateRejectedPromise(env);
    }
    bool vpnDialogSelect = false;
    std::string vpnExtMode = std::to_string(vpnDialogSelect);
    int32_t ret = 0;
#ifdef SUPPORT_SYSVPN
    ret = CheckVpnPermission(bundleName, vpnExtMode);
#else
    ret = NetDataShareHelperUtilsIface::Query(VPNEXT_MODE_URI, bundleName, vpnExtMode);
#endif // SUPPORT_SYSVPN
    if (ret != 0 || vpnExtMode != "1") {
        NETMANAGER_EXT_LOGE("dataShareHelperUtils Query error, err = %{public}d", ret);
        return CreateRejectedPromise(env);
    }

    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StopExtensionAbility(
        want, nullptr, accountId, AppExecFwk::ExtensionAbilityType::VPN);
    NETMANAGER_EXT_LOGI("execute StopExtensionAbility result: %{public}d", err);
    return CreateResolvedPromise(env);
}

static napi_value CreateVpnConnection(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::NewInstance(env, info, VPN_CONNECTION_EXT, MakeDataExt, [](napi_env, void *data, void *) {
        NETMANAGER_EXT_LOGI("finalize VpnConnection");
    });
}

static napi_value UpdateVpnAuthorize(napi_env env, napi_callback_info info)
{
    napi_value thisVal = nullptr;
    std::size_t argc = MAX_PARAM_NUM;
    napi_value argv[MAX_PARAM_NUM] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVal, nullptr));

    if ((argc != PARAM_ONE) || (NapiUtils::GetValueType(env, argv[ARG_NUM_0]) != napi_string)) {
        NETMANAGER_EXT_LOGE("funciton prameter error");
        napi_throw_error(env, std::to_string(NETMANAGER_EXT_ERR_PARAMETER_ERROR).c_str(), "Parameter error");
        return nullptr;
    }
    std::string bundleName = NapiUtils::GetStringFromValueUtf8(env, argv[ARG_NUM_0]);

    bool vpnDialogSelect = true;
    if (bundleName.find(VPN_DIALOG_POSTFIX) != std::string::npos) {
        vpnDialogSelect = false;
        bundleName = Replace(bundleName);
    }
    std::string vpnExtMode = std::to_string(vpnDialogSelect);
    int32_t ret = 0;
#ifdef SUPPORT_SYSVPN
    int32_t userId = AppExecFwk::Constants::UNSPECIFIED_USERID;
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(uid, userId) != ERR_OK) {
        NETMANAGER_EXT_LOGE("GetOsAccountLocalIdFromUid error, uid: %{public}d.", uid);
        return nullptr;
    }
    NETMANAGER_EXT_LOGE("UpdateVpnAuthorize uid: %{public}d, userid: %{public}d", uid, userId);
    ret = NetDataShareHelperUtilsIface::Update(VPNEXT_MODE_URI, bundleName + std::to_string(userId), vpnExtMode);
#else
    ret = NetDataShareHelperUtilsIface::Update(VPNEXT_MODE_URI, bundleName, vpnExtMode);
#endif // SUPPORT_SYSVPN
    NETMANAGER_EXT_LOGI("UpdateVpnAuthorize result. ret = %{public}d", ret);

    napi_value jsValue = nullptr;
    napi_get_boolean(env, true, &jsValue);
    return jsValue;
}

napi_value RegisterVpnExtModule(napi_env env, napi_value exports)
{
    NapiUtils::DefineProperties(env, exports,
                                {
                                    DECLARE_NAPI_FUNCTION(CREATE_VPN_CONNECTION, CreateVpnConnection),
                                    DECLARE_NAPI_FUNCTION(START_VPN_EXTENSION, StartVpnExtensionAbility),
                                    DECLARE_NAPI_FUNCTION(STOP_VPN_EXTENSION, StopVpnExtensionAbility),
                                    DECLARE_NAPI_FUNCTION(UPDATE_VPN_AUTHORIZE, UpdateVpnAuthorize),
                                });
    ModuleTemplate::DefineClass(env, exports,
                                {
                                    DECLARE_NAPI_FUNCTION(SET_UP_EXT, VpnConnectionExt::SetUp),
                                    DECLARE_NAPI_FUNCTION(PROTECT_EXT, VpnConnectionExt::Protect),
                                    DECLARE_NAPI_FUNCTION(DESTROY_EXT, VpnConnectionExt::Destroy),
                                },
                                VPN_CONNECTION_EXT);
    return exports;
}

static napi_module g_vpnModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = RegisterVpnExtModule,
    .nm_modname = VPN_EXT_MODULE_NAME,
    .nm_priv = nullptr,
    .reserved = {nullptr},
};

extern "C" __attribute__((constructor)) void VpnNapiRegister()
{
    napi_module_register(&g_vpnModule);
}
} // namespace NetManagerStandard
} // namespace OHOS
