/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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
#include "netmanager_ext_log.h"
 
static constexpr const char *EAP_MODULE_NAME = "net.eap";
#define DECLARE_EAP_CUSTOM_RESULT(result) \
    DECLARE_NAPI_STATIC_PROPERTY(#result, NapiUtils::CreateUint32(env, static_cast<uint32_t>(CustomResult::result)))
 
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
 
napi_value EapModule::InitEapModule(napi_env env, napi_value exports)
{
    std::initializer_list<napi_property_descriptor> functions = {
        DECLARE_NAPI_FUNCTION(FUNCTION_REG_CUSTOM_EAP_HANDLER, RegCustomEapHandler),
        DECLARE_NAPI_FUNCTION(FUNCTION_UNREG_CUSTOM_EAP_HANDLER, UnRegCustomEapHandler),
        DECLARE_NAPI_FUNCTION(FUNCTION_REPLY_CUSTOM_EAP_DATA, ReplyCustomEapData),
    };
    NapiUtils::DefineProperties(env, exports, functions);
    InitProperties(env, exports);
    AddCleanupHook(env);
    return exports;
}
 
void EapModule::InitProperties(napi_env env, napi_value exports)
{
    std::initializer_list<napi_property_descriptor> results = {
        DECLARE_EAP_CUSTOM_RESULT(RESULT_FAIL),
        DECLARE_EAP_CUSTOM_RESULT(RESULT_NEXT),
        DECLARE_EAP_CUSTOM_RESULT(RESULT_FINISH),
    };
    napi_value customResult = NapiUtils::CreateObject(env);
    NapiUtils::DefineProperties(env, customResult, results);
    NapiUtils::SetNamedProperty(env, exports, INTERFACE_NET_EAP_CUSTOM_RESULT, customResult);
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