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

#include "errorcode_convertor.h"
#include "module_template.h"
#include "napi_utils.h"
#include "net_manager_constants.h"
#include "netmanager_ext_log.h"
#include "networkvpn_client.h"
#include "vpn_connection_ext.h"
#include "want.h"
#include "ability_manager_client.h"
#include "extension_ability_info.h"
#include "napi_common_want.h"
#include "vpn_extension_context.h"
#include "vpn_monitor_ext.h"
#include "net_datashare_utils_iface.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t ARG_NUM_0 = 0;
constexpr int32_t PARAM_ONE = 1;

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

static bool g_started = false;

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

napi_value StartVpnExtensionAbility(napi_env env, napi_callback_info info)
{
    napi_value thisVal = nullptr;
    std::size_t argc = MAX_PARAM_NUM;

    napi_value argv[MAX_PARAM_NUM] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVal, nullptr));
    if ((argc != PARAM_ONE) || (NapiUtils::GetValueType(env, argv[ARG_NUM_0]) != napi_object)) {
        NETMANAGER_EXT_LOGE("funciton prameter error");
        napi_throw_error(env, std::to_string(NETMANAGER_EXT_ERR_PARAMETER_ERROR).c_str(), "Parameter error");
        return NapiUtils::GetUndefined(env);
    }
    AAFwk::Want want;
    int32_t accountId = -1;
    if (!AppExecFwk::UnwrapWant(env, argv[0], want)) {
        NETMANAGER_EXT_LOGE("Failed to parse want");
        napi_throw_error(env, std::to_string(NETMANAGER_EXT_ERR_PARAMETER_ERROR).c_str(), "Parse want error");
        return NapiUtils::GetUndefined(env);
    }

    std::string bundleName = want.GetElement().GetBundleName();
    int32_t rst = NetworkVpnClient::GetInstance().RegisterBundleName(bundleName);
    NETMANAGER_EXT_LOGI("VPN RegisterBundleName result = %{public}d", rst);
    std::string abilityName = want.GetElement().GetAbilityName();
    if (abilityName.find(VPN_DIALOG_POSTFIX) == std::string::npos) {
        bool vpnDialogSelect = false;
        std::string vpnExtMode = std::to_string(vpnDialogSelect);
        int32_t ret = NetDataShareHelperUtilsIface::Query(VPNEXT_MODE_URI, bundleName, vpnExtMode);
        if (!g_started || ret != 0 || vpnExtMode != "1") {
            g_started = true;
            VpnMonitor::GetInstance().ShowVpnDialog(bundleName, abilityName);
            NETMANAGER_EXT_LOGE("dataShareHelperUtils Query error, err = %{public}d", ret);
            return NapiUtils::GetUndefined(env);
        }
    }
    auto elem = want.GetElement();
    elem.SetAbilityName(Replace(abilityName));
    want.SetElement(elem);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartExtensionAbility(
        want, nullptr, accountId, AppExecFwk::ExtensionAbilityType::VPN);
    NETMANAGER_EXT_LOGI("execute StartVpnExtensionAbility result: %{public}d", err);
    return NapiUtils::GetUndefined(env);
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
        return NapiUtils::GetUndefined(env);
    }
    AAFwk::Want want;
    int32_t accountId = -1;
    if (!AppExecFwk::UnwrapWant(env, argv[0], want)) {
        NETMANAGER_EXT_LOGE("Failed to parse want");
        napi_throw_error(env, std::to_string(NETMANAGER_EXT_ERR_PARAMETER_ERROR).c_str(), "Parse want error");
        return NapiUtils::GetUndefined(env);
    }

    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StopExtensionAbility(
        want, nullptr, accountId, AppExecFwk::ExtensionAbilityType::VPN);
    NETMANAGER_EXT_LOGI("execute StopExtensionAbility result: %{public}d", err);
    return NapiUtils::GetUndefined(env);
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
    std::string bundleName  = NapiUtils::GetStringFromValueUtf8(env, argv[ARG_NUM_0]);

    bool vpnDialogSelect = true;
    if (bundleName.find(VPN_DIALOG_POSTFIX) != std::string::npos) {
        vpnDialogSelect = false;
        bundleName = Replace(bundleName);
    }
    std::string vpnExtMode = std::to_string(vpnDialogSelect);
    int32_t ret = NetDataShareHelperUtilsIface::Update(VPNEXT_MODE_URI, bundleName, vpnExtMode);
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
