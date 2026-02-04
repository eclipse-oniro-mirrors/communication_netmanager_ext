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
#include "vpn_connection.h"
#include "vpn_monitor.h"
#ifdef SUPPORT_SYSVPN
#include "add_context.h"
#include "delete_context.h"
#include "get_app_info_context.h"
#include "get_list_context.h"
#include "get_context.h"
#include "get_connected_context.h"
#include "vpn_async_work.h"
#endif // SUPPORT_SYSVPN

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t ARG_NUM_0 = 0;
constexpr int32_t PARAM_ONE = 1;
constexpr const char *ON = "on";
constexpr const char *OFF = "off";
#ifdef SUPPORT_SYSVPN
constexpr const char *ADD_SYS_VPN_CONFIG = "addSysVpnConfig";
constexpr const char *DELETE_SYS_VPN_CONFIG = "deleteSysVpnConfig";
constexpr const char *GET_CONNECTED_VPN_APP_INFO = "getConnectedVpnAppInfo";
constexpr const char *GET_SYS_VPN_CONFIG_LIST = "getSysVpnConfigList";
constexpr const char *GET_SYS_VPN_CONFIG = "getSysVpnConfig";
constexpr const char *GET_CONNECTED_SYS_VPN_CONFIG = "getConnectedSysVpnConfig";
constexpr const char *ENUM_SYSVPN_TYPE = "SysVpnType";
constexpr const char *IKEV2_IPSEC_MSCHAPv2_TYPE = "IKEV2_IPSEC_MSCHAPv2";
constexpr const char *IKEV2_IPSEC_PSK_TYPE = "IKEV2_IPSEC_PSK";
constexpr const char *IKEV2_IPSEC_RSA_TYPE = "IKEV2_IPSEC_RSA";
constexpr const char *L2TP_IPSEC_PSK_TYPE = "L2TP_IPSEC_PSK";
constexpr const char *L2TP_IPSEC_RSA_TYPE = "L2TP_IPSEC_RSA";
constexpr const char *IPSEC_XAUTH_PSK_TYPE = "IPSEC_XAUTH_PSK";
constexpr const char *IPSEC_XAUTH_RSA_TYPE = "IPSEC_XAUTH_RSA";
constexpr const char *IPSEC_HYBRID_RSA_TYPE = "IPSEC_HYBRID_RSA";
constexpr const char *OPENVPN_TYPE = "OPENVPN";
constexpr const char *L2TP_TYPE = "L2TP";
#endif // SUPPORT_SYSVPN

static void *MakeData(napi_env env, size_t argc, napi_value *argv, std::shared_ptr<EventManager>& manager)
{
    if ((argc != PARAM_ONE) || (NapiUtils::GetValueType(env, argv[ARG_NUM_0]) != napi_object)) {
        NETMANAGER_EXT_LOGE("funciton prameter error");
        napi_throw_error(env, std::to_string(NETMANAGER_EXT_ERR_PARAMETER_ERROR).c_str(), "Parameter error");
        return nullptr;
    }

    int32_t ret = NetworkVpnClient::GetInstance().CreateVpnConnection();
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMANAGER_EXT_LOGE("execute CreateVpnConnection failed: %{public}d", ret);
        std::string errorMsg = NetBaseErrorCodeConvertor().ConvertErrorCode(ret);
        napi_throw_error(env, std::to_string(ret).c_str(), errorMsg.c_str());
        return nullptr;
    }
    return reinterpret_cast<void *>(&NetworkVpnClient::GetInstance());
}

static napi_value CreateVpnConnection(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::NewInstance(env, info, VPN_CONNECTION, MakeData, [](napi_env, void *data, void *) {
        NETMANAGER_EXT_LOGI("finalize VpnConnection");
    });
}

static napi_value On(napi_env env, napi_callback_info info)
{
    return VpnMonitor::GetInstance().On(env, info);
}

static napi_value Off(napi_env env, napi_callback_info info)
{
    return VpnMonitor::GetInstance().Off(env, info);
}

#ifdef SUPPORT_SYSVPN
static napi_value AddSysVpnConfig(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<AddContext>(env, info, ADD_SYS_VPN_CONFIG, nullptr,
        VpnAsyncWork::ExecAddSysVpnConfig, VpnAsyncWork::AddSysVpnConfigCallback);
}

static napi_value DeleteSysVpnConfig(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<DeleteContext>(env, info, DELETE_SYS_VPN_CONFIG, nullptr,
        VpnAsyncWork::ExecDeleteSysVpnConfig, VpnAsyncWork::DeleteSysVpnConfigCallback);
}

static napi_value GetSysVpnConfigList(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetListContext>(env, info, GET_SYS_VPN_CONFIG_LIST, nullptr,
        VpnAsyncWork::ExecGetSysVpnConfigList, VpnAsyncWork::GetSysVpnConfigListCallback);
}

static napi_value GetSysVpnConfig(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetContext>(env, info, GET_SYS_VPN_CONFIG, nullptr,
        VpnAsyncWork::ExecGetSysVpnConfig, VpnAsyncWork::GetSysVpnConfigCallback);
}

static napi_value GetConnectedSysVpnConfig(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetConnectedContext>(env, info, GET_CONNECTED_SYS_VPN_CONFIG, nullptr,
        VpnAsyncWork::ExecGetConnectedSysVpnConfig, VpnAsyncWork::GetConnectedSysVpnConfigCallback);
}

static napi_value GetConnectedVpnAppInfo(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetAppInfoContext>(env, info, GET_CONNECTED_VPN_APP_INFO, nullptr,
        VpnAsyncWork::ExecGetConnectedVpnAppInfo, VpnAsyncWork::GetConnectedVpnAppInfoCallback);
}

static void InitSysVpnType(napi_env env, napi_value exports)
{
    NapiUtils::DefineProperties(env, exports, {
        DECLARE_NAPI_STATIC_PROPERTY(IKEV2_IPSEC_MSCHAPv2_TYPE,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(VpnType::IKEV2_IPSEC_MSCHAPv2))),
        DECLARE_NAPI_STATIC_PROPERTY(IKEV2_IPSEC_PSK_TYPE,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(VpnType::IKEV2_IPSEC_PSK))),
        DECLARE_NAPI_STATIC_PROPERTY(IKEV2_IPSEC_RSA_TYPE,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(VpnType::IKEV2_IPSEC_RSA))),
        DECLARE_NAPI_STATIC_PROPERTY(L2TP_IPSEC_PSK_TYPE,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(VpnType::L2TP_IPSEC_PSK))),
        DECLARE_NAPI_STATIC_PROPERTY(L2TP_IPSEC_RSA_TYPE,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(VpnType::L2TP_IPSEC_RSA))),
        DECLARE_NAPI_STATIC_PROPERTY(IPSEC_XAUTH_PSK_TYPE,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(VpnType::IPSEC_XAUTH_PSK))),
        DECLARE_NAPI_STATIC_PROPERTY(IPSEC_XAUTH_RSA_TYPE,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(VpnType::IPSEC_XAUTH_RSA))),
        DECLARE_NAPI_STATIC_PROPERTY(IPSEC_HYBRID_RSA_TYPE,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(VpnType::IPSEC_HYBRID_RSA))),
        DECLARE_NAPI_STATIC_PROPERTY(OPENVPN_TYPE,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(VpnType::OPENVPN))),
        DECLARE_NAPI_STATIC_PROPERTY(L2TP_TYPE,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(VpnType::L2TP))),
    });

    std::initializer_list<napi_property_descriptor> properties = {
        DECLARE_NAPI_STATIC_PROPERTY(IKEV2_IPSEC_MSCHAPv2_TYPE,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(VpnType::IKEV2_IPSEC_MSCHAPv2))),
        DECLARE_NAPI_STATIC_PROPERTY(IKEV2_IPSEC_PSK_TYPE,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(VpnType::IKEV2_IPSEC_PSK))),
        DECLARE_NAPI_STATIC_PROPERTY(IKEV2_IPSEC_RSA_TYPE,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(VpnType::IKEV2_IPSEC_RSA))),
        DECLARE_NAPI_STATIC_PROPERTY(L2TP_IPSEC_PSK_TYPE,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(VpnType::L2TP_IPSEC_PSK))),
        DECLARE_NAPI_STATIC_PROPERTY(L2TP_IPSEC_RSA_TYPE,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(VpnType::L2TP_IPSEC_RSA))),
        DECLARE_NAPI_STATIC_PROPERTY(IPSEC_XAUTH_PSK_TYPE,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(VpnType::IPSEC_XAUTH_PSK))),
        DECLARE_NAPI_STATIC_PROPERTY(IPSEC_XAUTH_RSA_TYPE,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(VpnType::IPSEC_XAUTH_RSA))),
        DECLARE_NAPI_STATIC_PROPERTY(IPSEC_HYBRID_RSA_TYPE,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(VpnType::IPSEC_HYBRID_RSA))),
        DECLARE_NAPI_STATIC_PROPERTY(OPENVPN_TYPE,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(VpnType::OPENVPN))),
        DECLARE_NAPI_STATIC_PROPERTY(L2TP_TYPE,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(VpnType::L2TP))),
    };

    napi_value sysVpnType = NapiUtils::CreateObject(env);
    NapiUtils::DefineProperties(env, sysVpnType, properties);
    NapiUtils::SetNamedProperty(env, exports, ENUM_SYSVPN_TYPE, sysVpnType);
}
#endif // SUPPORT_SYSVPN

napi_value RegisterVpnModule(napi_env env, napi_value exports)
{
    NapiUtils::DefineProperties(env, exports,
                                {
                                    DECLARE_NAPI_FUNCTION(CREATE_VPN_CONNECTION, CreateVpnConnection),
                                    DECLARE_NAPI_FUNCTION(ON, On),
                                    DECLARE_NAPI_FUNCTION(OFF, Off),
                                    #ifdef SUPPORT_SYSVPN
                                    DECLARE_NAPI_FUNCTION(ADD_SYS_VPN_CONFIG, AddSysVpnConfig),
                                    DECLARE_NAPI_FUNCTION(DELETE_SYS_VPN_CONFIG, DeleteSysVpnConfig),
                                    DECLARE_NAPI_FUNCTION(GET_CONNECTED_VPN_APP_INFO, GetConnectedVpnAppInfo),
                                    DECLARE_NAPI_FUNCTION(GET_SYS_VPN_CONFIG_LIST, GetSysVpnConfigList),
                                    DECLARE_NAPI_FUNCTION(GET_SYS_VPN_CONFIG, GetSysVpnConfig),
                                    DECLARE_NAPI_FUNCTION(GET_CONNECTED_SYS_VPN_CONFIG, GetConnectedSysVpnConfig),
                                    #endif // SUPPORT_SYSVPN
                                });
    ModuleTemplate::DefineClass(env, exports,
                                {
                                    DECLARE_NAPI_FUNCTION(SET_UP, VpnConnection::SetUp),
                                    DECLARE_NAPI_FUNCTION(PROTECT, VpnConnection::Protect),
                                    DECLARE_NAPI_FUNCTION(DESTROY, VpnConnection::Destroy),
                                },
                                VPN_CONNECTION);
#ifdef SUPPORT_SYSVPN
    InitSysVpnType(env, exports);
#endif // SUPPORT_SYSVPN
    NapiUtils::SetEnvValid(env);
    napi_add_env_cleanup_hook(env, NapiUtils::HookForEnvCleanup, env);
    return exports;
}

static napi_module g_vpnModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = RegisterVpnModule,
    .nm_modname = VPN_MODULE_NAME,
    .nm_priv = nullptr,
    .reserved = {nullptr},
};

extern "C" __attribute__((constructor)) void VpnNapiRegister()
{
    napi_module_register(&g_vpnModule);
}
} // namespace NetManagerStandard
} // namespace OHOS
