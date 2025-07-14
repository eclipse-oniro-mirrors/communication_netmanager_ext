/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
 
#include "eap_module.h"

#include "eth_eap_profile.h"
#include "netmanager_ext_log.h"
 
static constexpr const char *EAP_MODULE_NAME = "net.eap";
static constexpr const char *INTERFACE_ETH_METHOD = "EapMethod";
static constexpr const char *INTERFACE_PHASE_2_METHOD = "Phase2Method";
static constexpr const char *FUNCTION_START_ETH_EAP = "startEthEap";
static constexpr const char *FUNCTION_LOGOFF_ETH_EAP = "logOffEthEap";

namespace OHOS::NetManagerStandard {
 
static void AddCleanupHook(napi_env env)
{
    NapiUtils::SetEnvValid(env);
    auto envWrapper = new (std::nothrow) napi_env;
    if (envWrapper == nullptr) {
        NETMANAGER_EXT_LOGE("EnvWrapper create fail!");
        return;
    }
    *envWrapper = env;
    napi_add_env_cleanup_hook(env, NapiUtils::HookForEnvCleanup, envWrapper);
}

void EapModule::DeclareEapMethod(napi_env env, napi_value exports)
{
    napi_value result = NapiUtils::CreateObject(env);
    NapiUtils::DefineProperties(env, result, {
        DECLARE_NAPI_STATIC_PROPERTY("EAP_NONE",
            NapiUtils::CreateInt32(env, static_cast<int32_t>(EapMethod::EAP_NONE))),
        DECLARE_NAPI_STATIC_PROPERTY("EAP_PEAP",
            NapiUtils::CreateInt32(env, static_cast<int32_t>(EapMethod::EAP_PEAP))),
        DECLARE_NAPI_STATIC_PROPERTY("EAP_TLS",
            NapiUtils::CreateInt32(env, static_cast<int32_t>(EapMethod::EAP_TLS))),
        DECLARE_NAPI_STATIC_PROPERTY("EAP_TTLS",
            NapiUtils::CreateInt32(env, static_cast<int32_t>(EapMethod::EAP_TTLS))),
        DECLARE_NAPI_STATIC_PROPERTY("EAP_PWD",
            NapiUtils::CreateInt32(env, static_cast<int32_t>(EapMethod::EAP_PWD))),
        DECLARE_NAPI_STATIC_PROPERTY("EAP_SIM",
            NapiUtils::CreateInt32(env, static_cast<int32_t>(EapMethod::EAP_SIM))),
        DECLARE_NAPI_STATIC_PROPERTY("EAP_AKA",
            NapiUtils::CreateInt32(env, static_cast<int32_t>(EapMethod::EAP_AKA))),
        DECLARE_NAPI_STATIC_PROPERTY("EAP_AKA_PRIME",
            NapiUtils::CreateInt32(env, static_cast<int32_t>(EapMethod::EAP_AKA_PRIME))),
        DECLARE_NAPI_STATIC_PROPERTY("EAP_UNAUTH_TLS",
            NapiUtils::CreateInt32(env, static_cast<int32_t>(EapMethod::EAP_UNAUTH_TLS))),
    });
    NapiUtils::SetNamedProperty(env, exports, INTERFACE_ETH_METHOD, result);
}
 
void EapModule::DeclarePhase2Method(napi_env env, napi_value exports)
{
    napi_value result = NapiUtils::CreateObject(env);
    NapiUtils::DefineProperties(env, result, {
        DECLARE_NAPI_STATIC_PROPERTY("PHASE2_NONE",
            NapiUtils::CreateInt32(env, static_cast<int32_t>(Phase2Method::PHASE2_NONE))),
        DECLARE_NAPI_STATIC_PROPERTY("PHASE2_PAP",
            NapiUtils::CreateInt32(env, static_cast<int32_t>(Phase2Method::PHASE2_PAP))),
        DECLARE_NAPI_STATIC_PROPERTY("PHASE2_MSCHAP",
            NapiUtils::CreateInt32(env, static_cast<int32_t>(Phase2Method::PHASE2_MSCHAP))),
        DECLARE_NAPI_STATIC_PROPERTY("PHASE2_MSCHAPV2",
            NapiUtils::CreateInt32(env, static_cast<int32_t>(Phase2Method::PHASE2_MSCHAPV2))),
        DECLARE_NAPI_STATIC_PROPERTY("PHASE2_GTC",
            NapiUtils::CreateInt32(env, static_cast<int32_t>(Phase2Method::PHASE2_GTC))),
        DECLARE_NAPI_STATIC_PROPERTY("PHASE2_SIM",
            NapiUtils::CreateInt32(env, static_cast<int32_t>(Phase2Method::PHASE2_SIM))),
        DECLARE_NAPI_STATIC_PROPERTY("PHASE2_AKA",
            NapiUtils::CreateInt32(env, static_cast<int32_t>(Phase2Method::PHASE2_AKA))),
        DECLARE_NAPI_STATIC_PROPERTY("PHASE2_AKA_PRIME",
            NapiUtils::CreateInt32(env, static_cast<int32_t>(Phase2Method::PHASE2_AKA_PRIME))),
    });
    NapiUtils::SetNamedProperty(env, exports, INTERFACE_PHASE_2_METHOD, result);
}
 
napi_value EapModule::InitEapModule(napi_env env, napi_value exports)
{
    std::initializer_list<napi_property_descriptor> functions = {
        DECLARE_NAPI_FUNCTION(functionRegCustomEapHandler, RegCustomEapHandler),
        DECLARE_NAPI_FUNCTION(functionUnregCustomEapHandler, UnRegCustomEapHandler),
        DECLARE_NAPI_FUNCTION(functionReplyCustomEapData, ReplyCustomEapData),
        DECLARE_NAPI_FUNCTION(FUNCTION_START_ETH_EAP, StartEthEap),
        DECLARE_NAPI_FUNCTION(FUNCTION_LOGOFF_ETH_EAP, LogOffEthEap),
    };
    NapiUtils::DefineProperties(env, exports, functions);
    InitProperties(env, exports);
    DeclareEapMethod(env, exports);
    DeclarePhase2Method(env, exports);
    AddCleanupHook(env);
    return exports;
}
 
void EapModule::InitProperties(napi_env env, napi_value exports)
{
    std::initializer_list<napi_property_descriptor> results = {
        DECLARE_NAPI_STATIC_PROPERTY("RESULT_FAIL",
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(CustomResult::RESULT_FAIL))),
        DECLARE_NAPI_STATIC_PROPERTY("RESULT_NEXT",
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(CustomResult::RESULT_NEXT))),
        DECLARE_NAPI_STATIC_PROPERTY("RESULT_FINISH",
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(CustomResult::RESULT_FINISH))),
    };
    napi_value customResult = NapiUtils::CreateObject(env);
    NapiUtils::DefineProperties(env, customResult, results);
    NapiUtils::SetNamedProperty(env, exports, interfaceNetEapCustomResult, customResult);
}
 
static napi_module g_eapModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = EapModule::InitEapModule,
    .nm_modname = EAP_MODULE_NAME,
    .nm_priv = nullptr,
    .reserved = {nullptr},
};
 
extern "C" __attribute__((constructor)) void RegisterEapModule(void)
{
    NETMANAGER_EXT_LOGI("RegisterEapModule success!");
    napi_module_register(&g_eapModule);
}
} // namespace OHOS::NetManagerStandard