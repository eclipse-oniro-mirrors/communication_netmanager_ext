/*
 * Copyright (C) 2022-2024 Huawei Device Co., Ltd.
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

#include <napi/native_api.h>
#include <napi/native_common.h>

#include "ethernet_async_work.h"
#include "get_all_active_ifaces_context.h"
#include "get_device_infomation.h"
#include "get_mac_address_context.h"
#include "get_iface_config_context.h"
#include "mac_address_info.h"
#include "interface_configuration.h"
#include "interface_state_observer_wrapper.h"
#include "is_iface_active_context.h"
#include "set_iface_config_context.h"
#include "module_template.h"
#include "napi_utils.h"

static constexpr const char *EVENT_STATS_CHANGE = "interfaceStateChange";

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr const char *GET_MAC_ADDR = "getMacAddress";
constexpr const char *GET_IFACE = "getIfaceConfig";
constexpr const char *SET_IFACE = "setIfaceConfig";
constexpr const char *IS_IFACE = "isIfaceActive";
constexpr const char *GET_ALL_IFACES = "getAllActiveIfaces";
constexpr const char *STATIC_NAME = "STATIC";
constexpr const char *DHCP_NAME = "DHCP";
constexpr const char *IP_SET_MODE = "IPSetMode";
constexpr const char *FUNCTION_ON = "on";
constexpr const char *FUNCTION_OFF = "off";
constexpr const char *GET_DEVICE_INFO = "getEthernetDeviceInfos";

napi_value GetMacAddress(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetMacAddressContext>(env, info, GET_MAC_ADDR, nullptr,
        EthernetAsyncWork::ExecGetMacAddress, EthernetAsyncWork::GetMacAddressCallback);
}

napi_value GetIfaceConfig(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetIfaceConfigContext>(env, info, GET_IFACE, nullptr,
        EthernetAsyncWork::ExecGetIfaceConfig, EthernetAsyncWork::GetIfaceConfigCallback);
}

napi_value SetIfaceConfig(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<SetIfaceConfigContext>(env, info, SET_IFACE, nullptr,
        EthernetAsyncWork::ExecSetIfaceConfig, EthernetAsyncWork::SetIfaceConfigCallback);
}

napi_value IsIfaceActive(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<IsIfaceActiveContext>(env, info, IS_IFACE, nullptr,
        EthernetAsyncWork::ExecIsIfaceActive, EthernetAsyncWork::IsIfaceActiveCallback);
}

napi_value GetAllActiveIfaces(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetAllActiveIfacesContext>(env, info, GET_ALL_IFACES, nullptr,
        EthernetAsyncWork::ExecGetAllActiveIfaces, EthernetAsyncWork::GetAllActiveIfacesCallback);
}
} // namespace

static napi_value DeclareEthernetData(napi_env env, napi_value exports)
{
    NapiUtils::DefineProperties(env, exports, {
        DECLARE_NAPI_STATIC_PROPERTY(STATIC_NAME, NapiUtils::CreateInt32(env, static_cast<int32_t>(STATIC))),
        DECLARE_NAPI_STATIC_PROPERTY(DHCP_NAME, NapiUtils::CreateInt32(env, static_cast<int32_t>(DHCP))),
    });
    return exports;
}

napi_value On(napi_env env, napi_callback_info info)
{
    return InterfaceStateObserverWrapper::GetInstance().On(env, info, {EVENT_STATS_CHANGE}, false);
}

napi_value Off(napi_env env, napi_callback_info info)
{
    return InterfaceStateObserverWrapper::GetInstance().Off(env, info, {EVENT_STATS_CHANGE}, false);
}

napi_value GetDeviceInformation(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetDeviceInformationContext>(env, info, GET_DEVICE_INFO, nullptr,
        EthernetAsyncWork::ExecGetDeviceInformation, EthernetAsyncWork::GetDeviceInformationCallback);
}

static napi_value DeclareEthernetInterface(napi_env env, napi_value exports)
{
    NapiUtils::DefineProperties(env, exports, {
        DECLARE_NAPI_FUNCTION(GET_MAC_ADDR, GetMacAddress),
        DECLARE_NAPI_FUNCTION(GET_IFACE, GetIfaceConfig),
        DECLARE_NAPI_FUNCTION(SET_IFACE, SetIfaceConfig),
        DECLARE_NAPI_FUNCTION(IS_IFACE, IsIfaceActive),
        DECLARE_NAPI_FUNCTION(GET_ALL_IFACES, GetAllActiveIfaces),
        DECLARE_NAPI_FUNCTION(FUNCTION_ON, On),
        DECLARE_NAPI_FUNCTION(FUNCTION_OFF, Off),
        DECLARE_NAPI_FUNCTION(GET_DEVICE_INFO, GetDeviceInformation),
    });
    return exports;
}

static void AddCleanupHook(napi_env env)
{
    NapiUtils::SetEnvValid(env);
    auto envWrapper = new (std::nothrow) napi_env;
    if (envWrapper == nullptr) {
        NETMANAGER_BASE_LOGE("EnvWrapper create fail!");
        return;
    }
    *envWrapper = env;
    napi_add_env_cleanup_hook(env, NapiUtils::HookForEnvCleanup, envWrapper);
}

napi_value RegisterEthernetInterface(napi_env env, napi_value exports)
{
    DeclareEthernetInterface(env, exports);
    DeclareEthernetData(env, exports);

    std::initializer_list<napi_property_descriptor> ipSetMode = {
        DECLARE_NAPI_STATIC_PROPERTY(STATIC_NAME,
                                     NapiUtils::CreateUint32(env, static_cast<uint32_t>(IPSetMode::STATIC))),
        DECLARE_NAPI_STATIC_PROPERTY(DHCP_NAME, NapiUtils::CreateUint32(env, static_cast<uint32_t>(IPSetMode::DHCP))),
    };
    napi_value ipSetMOdes = NapiUtils::CreateObject(env);
    NapiUtils::DefineProperties(env, ipSetMOdes, ipSetMode);
    NapiUtils::SetNamedProperty(env, exports, IP_SET_MODE, ipSetMOdes);
    AddCleanupHook(env);
    return exports;
}

static napi_module g_ethernetModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = RegisterEthernetInterface,
    .nm_modname = "net.ethernet",
    .nm_priv = (0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterEthernetModule(void)
{
    napi_module_register(&g_ethernetModule);
}
} // namespace NetManagerStandard
} // namespace OHOS
