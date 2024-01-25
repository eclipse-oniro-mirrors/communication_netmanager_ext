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

#include "js_vpn_extension.h"

#include "ability_handler.h"
#include "ability_info.h"
#include "configuration_utils.h"
#include "hitrace_meter.h"
#include "hilog_wrapper.h"
#include "js_extension_common.h"
#include "js_extension_context.h"
#include "runtime.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "display_manager.h"
#include "js_vpn_extension_context.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_common_configuration.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#ifdef SUPPORT_GRAPHICS
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "window_scene.h"
#include "netmgr_ext_log_wrapper.h"
#endif

using namespace OHOS::AbilityRuntime;
namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
}

namespace {
napi_value PromiseCallback(napi_env env, napi_callback_info info)
{
    void *data = nullptr;
    NAPI_CALL_NO_THROW(napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &data), nullptr);
    auto *callbackInfo = static_cast<AppExecFwk::AbilityTransactionCallbackInfo<> *>(data);
    callbackInfo->Call();
    AppExecFwk::AbilityTransactionCallbackInfo<>::Destroy(callbackInfo);
    data = nullptr;
    return nullptr;
}

napi_value OnConnectPromiseCallback(napi_env env, napi_callback_info info)
{
    NETMGR_EXT_LOG_D("enter");
    void *data = nullptr;
    size_t argc = ARGC_MAX_COUNT;
    napi_value argv[ARGC_MAX_COUNT] = {nullptr};
    NAPI_CALL_NO_THROW(napi_get_cb_info(env, info, &argc, argv, nullptr, &data), nullptr);
    auto *callbackInfo = static_cast<AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *>(data);
    sptr<IRemoteObject> vpn = nullptr;
    if (argc > 0) {
        vpn = NAPI_ohos_rpc_getNativeRemoteObject(env, argv[0]);
    }
    callbackInfo->Call(vpn);
    AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>>::Destroy(callbackInfo);
    data = nullptr;
    NETMGR_EXT_LOG_D("end");
    return nullptr;
}
}

using namespace OHOS::AppExecFwk;

napi_value AttachVpnExtensionContext(napi_env env, void *value, void *)
{
    NETMGR_EXT_LOG_I("call");
    if (value == nullptr) {
        HILOG_WARN("invalid parameter.");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<VpnExtensionContext> *>(value)->lock();
    if (ptr == nullptr) {
        HILOG_WARN("invalid context.");
        return nullptr;
    }
    napi_value object = CreateJsVpnExtensionContext(env, ptr);
    auto contextObj = JsRuntime::LoadSystemModuleByEngine(env,
        "application.VpnExtensionContext", &object, 1)->GetNapiValue();
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachVpnExtensionContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<VpnExtensionContext>(ptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            NETMGR_EXT_LOG_I("Finalizer for weak_ptr vpn extension context is called");
            delete static_cast<std::weak_ptr<VpnExtensionContext> *>(data);
        },
        nullptr, nullptr);
    return contextObj;
}

JsVpnExtension* JsVpnExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    return new JsVpnExtension(static_cast<JsRuntime&>(*runtime));
}

JsVpnExtension::JsVpnExtension(JsRuntime& jsRuntime) : jsRuntime_(jsRuntime) {}
JsVpnExtension::~JsVpnExtension()
{
    NETMGR_EXT_LOG_D("Js vpn extension destructor.");
    auto context = GetContext();
    if (context) {
        context->Unbind();
    }

    jsRuntime_.FreeNativeReference(std::move(jsObj_));
    jsRuntime_.FreeNativeReference(std::move(shellContextRef_));
}

void JsVpnExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    VpnExtension::Init(record, application, handler, token);
    std::string srcPath = "";
    GetSrcPath(srcPath);
    if (srcPath.empty()) {
        NETMGR_EXT_LOG_E("Failed to get srcPath");
        return;
    }

    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    NETMGR_EXT_LOG_D("JsVpnExtension::Init moduleName:%{public}s,srcPath:%{public}s.",
        moduleName.c_str(), srcPath.c_str());
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    jsObj_ = jsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo_->hapPath, abilityInfo_->compileMode == CompileMode::ES_MODULE);
    if (jsObj_ == nullptr) {
        NETMGR_EXT_LOG_E("Failed to get jsObj_");
        return;
    }

    NETMGR_EXT_LOG_I("JsVpnExtension::Init ConvertNativeValueTo.");
    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        NETMGR_EXT_LOG_E("Failed to get JsVpnExtension object");
        return;
    }

    BindContext(env, obj);

    SetExtensionCommon(JsExtensionCommon::Create(jsRuntime_, static_cast<NativeReference&>(*jsObj_), shellContextRef_));

    handler_ = handler;
    auto context = GetContext();
    auto appContext = Context::GetApplicationContext();
    if (context != nullptr && appContext != nullptr) {
        auto appConfig = appContext->GetConfiguration();
        if (appConfig != nullptr) {
            NETMGR_EXT_LOG_D("Original config dump: %{public}s", appConfig->GetName().c_str());
            context->SetConfiguration(std::make_shared<Configuration>(*appConfig));
        }
    }
    ListenWMS();
}

void JsVpnExtension::ListenWMS()
{
#ifdef SUPPORT_GRAPHICS
    NETMGR_EXT_LOG_I("RegisterDisplayListener");
    auto abilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (abilityManager == nullptr) {
        NETMGR_EXT_LOG_E("Failed to get SaMgr.");
        return;
    }

    auto jsVpnExtension = std::static_pointer_cast<JsVpnExtension>(shared_from_this());
    displayListener_ = sptr<JsVpnExtensionDisplayListener>::MakeSptr(jsVpnExtension);
    if (displayListener_ == nullptr) {
        NETMGR_EXT_LOG_E("Failed to create display listener.");
        return;
    }

    auto listener = sptr<SystemAbilityStatusChangeListener>::MakeSptr(displayListener_);
    if (listener == nullptr) {
        NETMGR_EXT_LOG_E("Failed to create status change listener.");
        return;
    }

    auto ret = abilityManager->SubscribeSystemAbility(WINDOW_MANAGER_SERVICE_ID, listener);
    if (ret != 0) {
        NETMGR_EXT_LOG_E("subscribe system ability failed, ret = %{public}d.", ret);
    }
#endif
}

void JsVpnExtension::SystemAbilityStatusChangeListener::OnAddSystemAbility(int32_t systemAbilityId,
    const std::string& deviceId)
{
    NETMGR_EXT_LOG_I("systemAbilityId: %{public}d add", systemAbilityId);
    if (systemAbilityId == WINDOW_MANAGER_SERVICE_ID) {
        Rosen::DisplayManager::GetInstance().RegisterDisplayListener(tmpDisplayListener_);
    }
}

void JsVpnExtension::BindContext(napi_env env, napi_value obj)
{
    auto context = GetContext();
    if (context == nullptr) {
        NETMGR_EXT_LOG_E("Failed to get context");
        return;
    }
    NETMGR_EXT_LOG_I("call");
    napi_value contextObj = CreateJsVpnExtensionContext(env, context);
    shellContextRef_ = JsRuntime::LoadSystemModuleByEngine(env, "application.VpnExtensionContext",
        &contextObj, ARGC_ONE);
    contextObj = shellContextRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        NETMGR_EXT_LOG_E("Failed to get context native object");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<VpnExtensionContext>(context);
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachVpnExtensionContext, workContext, nullptr);
    NETMGR_EXT_LOG_I("JsVpnExtension::Init Bind.");
    context->Bind(jsRuntime_, shellContextRef_.get());
    NETMGR_EXT_LOG_I("JsVpnExtension::SetProperty.");
    napi_set_named_property(env, obj, "context", contextObj);
    NETMGR_EXT_LOG_I("Set vpn extension context");

    napi_wrap(env, contextObj, workContext,
        [](napi_env, void* data, void*) {
            NETMGR_EXT_LOG_I("Finalizer for weak_ptr vpn extension context is called");
            delete static_cast<std::weak_ptr<VpnExtensionContext>*>(data);
        },
        nullptr, nullptr);

    NETMGR_EXT_LOG_I("JsVpnExtension::Init end.");
}

void JsVpnExtension::OnStart(const AAFwk::Want &want)
{
    Extension::OnStart(want);
    NETMGR_EXT_LOG_I("call");

    auto context = GetContext();
    if (context == nullptr) {
        NETMGR_EXT_LOG_E("context is null");
        return;
    }

    int displayId = want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, Rosen::WindowScene::DEFAULT_DISPLAY_ID);
    auto configUtils = std::make_shared<ConfigurationUtils>();
    configUtils->InitDisplayConfig(displayId, context->GetConfiguration(), context->GetResourceManager());

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    // display config has changed, need update context.config
    JsExtensionContext::ConfigurationUpdated(env, shellContextRef_, context->GetConfiguration());

    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = {napiWant};
    CallObjectMethod("onCreate", argv, ARGC_ONE);
    NETMGR_EXT_LOG_I("ok");
}

void JsVpnExtension::OnStop()
{
    VpnExtension::OnStop();
    NETMGR_EXT_LOG_I("call");
    CallObjectMethod("onDestroy");
    bool ret = ConnectionManager::GetInstance().DisconnectCaller(GetContext()->GetToken());
    if (ret) {
        ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
        NETMGR_EXT_LOG_I("The vpn extension connection is not disconnected.");
    }
    Rosen::DisplayManager::GetInstance().UnregisterDisplayListener(displayListener_);
    NETMGR_EXT_LOG_I("ok");
}

sptr<IRemoteObject> JsVpnExtension::OnConnect(const AAFwk::Want &want)
{
    HandleScope handleScope(jsRuntime_);
    napi_value result = CallOnConnect(want);
    napi_env env = jsRuntime_.GetNapiEnv();
    auto remoteObj = NAPI_ohos_rpc_getNativeRemoteObject(env, result);
    if (remoteObj == nullptr) {
        NETMGR_EXT_LOG_E("remoteObj null.");
    }
    return remoteObj;
}

sptr<IRemoteObject> JsVpnExtension::OnConnect(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *callbackInfo, bool &isAsyncCallback)
{
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value result = CallOnConnect(want);
    bool isPromise = CheckPromise(result);
    if (!isPromise) {
        isAsyncCallback = false;
        sptr<IRemoteObject> remoteObj = NAPI_ohos_rpc_getNativeRemoteObject(env, result);
        if (remoteObj == nullptr) {
            NETMGR_EXT_LOG_E("remoteObj null.");
        }
        return remoteObj;
    }

    bool callResult = false;
    do {
        if (!CheckTypeForNapiValue(env, result, napi_object)) {
            NETMGR_EXT_LOG_E("CallPromise, error to convert native value to NativeObject.");
            break;
        }
        napi_value then = nullptr;
        napi_get_named_property(env, result, "then", &then);
        if (then == nullptr) {
            NETMGR_EXT_LOG_E("CallPromise, error to get property: then.");
            break;
        }
        bool isCallable = false;
        napi_is_callable(env, then, &isCallable);
        if (!isCallable) {
            NETMGR_EXT_LOG_E("CallPromise, property then is not callable");
            break;
        }
        napi_value promiseCallback = nullptr;
        napi_create_function(env, "promiseCallback", strlen("promiseCallback"),
            OnConnectPromiseCallback, callbackInfo, &promiseCallback);
        napi_value argv[1] = { promiseCallback };
        napi_call_function(env, result, then, 1, argv, nullptr);
        callResult = true;
    } while (false);

    if (!callResult) {
        NETMGR_EXT_LOG_E("error to call promise.");
        isAsyncCallback = false;
    } else {
        isAsyncCallback = true;
    }
    return nullptr;
}

void JsVpnExtension::OnDisconnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnDisconnect(want);
    NETMGR_EXT_LOG_D("%{public}s begin.", __func__);
    CallOnDisconnect(want, false);
    NETMGR_EXT_LOG_D("%{public}s end.", __func__);
}

void JsVpnExtension::OnDisconnect(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnDisconnect(want);
    NETMGR_EXT_LOG_D("%{public}s start.", __func__);
    napi_value result = CallOnDisconnect(want, true);
    bool isPromise = CheckPromise(result);
    if (!isPromise) {
        isAsyncCallback = false;
        return;
    }
    bool callResult = CallPromise(result, callbackInfo);
    if (!callResult) {
        NETMGR_EXT_LOG_E("error to call promise.");
        isAsyncCallback = false;
    } else {
        isAsyncCallback = true;
    }

    NETMGR_EXT_LOG_D("%{public}s end.", __func__);
}

void JsVpnExtension::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    Extension::OnCommand(want, restart, startId);
    NETMGR_EXT_LOG_I("restart=%{public}s,startId=%{public}d.",
        restart ? "true" : "false",
        startId);
    // wrap want
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    // wrap startId
    napi_value napiStartId = nullptr;
    napi_create_int32(env, startId, &napiStartId);
    napi_value argv[] = {napiWant, napiStartId};
    CallObjectMethod("onRequest", argv, ARGC_TWO);
    NETMGR_EXT_LOG_I("ok");
}

napi_value JsVpnExtension::CallObjectMethod(const char* name, napi_value const* argv, size_t argc)
{
    NETMGR_EXT_LOG_I("CallObjectMethod(%{public}s)", name);

    if (!jsObj_) {
        HILOG_WARN("Not found VpnExtension.js");
        return nullptr;
    }

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        NETMGR_EXT_LOG_E("Failed to get VpnExtension object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, name, &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        NETMGR_EXT_LOG_E("Failed to get '%{public}s' from VpnExtension object", name);
        return nullptr;
    }
    NETMGR_EXT_LOG_I("CallFunction(%{public}s) ok", name);
    napi_value result = nullptr;
    napi_call_function(env, obj, method, argc, argv, &result);
    return result;
}

void JsVpnExtension::GetSrcPath(std::string &srcPath)
{
    NETMGR_EXT_LOG_D("GetSrcPath start.");
    if (!Extension::abilityInfo_->isModuleJson) {
        /* temporary compatibility api8 + config.json */
        srcPath.append(Extension::abilityInfo_->package);
        srcPath.append("/assets/js/");
        if (!Extension::abilityInfo_->srcPath.empty()) {
            srcPath.append(Extension::abilityInfo_->srcPath);
        }
        srcPath.append("/").append(Extension::abilityInfo_->name).append(".abc");
        return;
    }

    if (!Extension::abilityInfo_->srcEntrance.empty()) {
        srcPath.append(Extension::abilityInfo_->moduleName + "/");
        srcPath.append(Extension::abilityInfo_->srcEntrance);
        srcPath.erase(srcPath.rfind('.'));
        srcPath.append(".abc");
    }
}

napi_value JsVpnExtension::CallOnConnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnConnect(want);
    NETMGR_EXT_LOG_D("call");
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = {napiWant};
    if (!jsObj_) {
        HILOG_WARN("Not found VpnExtension.js");
        return nullptr;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        NETMGR_EXT_LOG_E("Failed to get VpnExtension object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onConnect", &method);
    if (method == nullptr) {
        NETMGR_EXT_LOG_E("Failed to get onConnect from VpnExtension object");
        return nullptr;
    }
    napi_value remoteNative = nullptr;
    napi_call_function(env, obj, method, ARGC_ONE, argv, &remoteNative);
    if (remoteNative == nullptr) {
        NETMGR_EXT_LOG_E("remoteNative nullptr.");
    }
    NETMGR_EXT_LOG_I("ok");
    return remoteNative;
}

napi_value JsVpnExtension::CallOnDisconnect(const AAFwk::Want &want, bool withResult)
{
    HandleEscape handleEscape(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = { napiWant };
    if (!jsObj_) {
        HILOG_WARN("Not found VpnExtension.js");
        return nullptr;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        NETMGR_EXT_LOG_E("Failed to get VpnExtension object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onDisconnect", &method);
    if (method == nullptr) {
        NETMGR_EXT_LOG_E("Failed to get onDisconnect from VpnExtension object");
        return nullptr;
    }

    if (withResult) {
        napi_value result = nullptr;
        napi_call_function(env, obj, method, ARGC_ONE, argv, &result);
        return handleEscape.Escape(result);
    } else {
        napi_call_function(env, obj, method, ARGC_ONE, argv, nullptr);
        return nullptr;
    }
}

bool JsVpnExtension::CheckPromise(napi_value result)
{
    if (result == nullptr) {
        NETMGR_EXT_LOG_D("CheckPromise, result is nullptr, no need to call promise.");
        return false;
    }
    napi_env env = jsRuntime_.GetNapiEnv();
    bool isPromise = false;
    napi_is_promise(env, result, &isPromise);
    if (!isPromise) {
        NETMGR_EXT_LOG_D("CheckPromise, result is not promise, no need to call promise.");
        return false;
    }
    return true;
}

bool JsVpnExtension::CallPromise(napi_value result, AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo)
{
    napi_env env = jsRuntime_.GetNapiEnv();
    if (!CheckTypeForNapiValue(env, result, napi_object)) {
        NETMGR_EXT_LOG_E("CallPromise, Error to convert native value to NativeObject.");
        return false;
    }
    napi_value then = nullptr;
    napi_get_named_property(env, result, "then", &then);
    if (then == nullptr) {
        NETMGR_EXT_LOG_E("CallPromise, Error to get property: then.");
        return false;
    }
    bool isCallable = false;
    napi_is_callable(env, then, &isCallable);
    if (!isCallable) {
        NETMGR_EXT_LOG_E("CallPromise, Property then is not callable.");
        return false;
    }
    HandleScope handleScope(jsRuntime_);
    napi_value promiseCallback = nullptr;
    napi_create_function(env, "promiseCallback", strlen("promiseCallback"), PromiseCallback,
        callbackInfo, &promiseCallback);
    napi_value argv[1] = { promiseCallback };
    napi_call_function(env, result, then, 1, argv, nullptr);
    NETMGR_EXT_LOG_D("end");
    return true;
}

void JsVpnExtension::OnConfigurationUpdated(const AppExecFwk::Configuration& configuration)
{
    VpnExtension::OnConfigurationUpdated(configuration);
    NETMGR_EXT_LOG_I("call");
    auto context = GetContext();
    if (context == nullptr) {
        NETMGR_EXT_LOG_E("Context is invalid.");
        return;
    }

    auto contextConfig = context->GetConfiguration();
    if (contextConfig != nullptr) {
        NETMGR_EXT_LOG_D("Config dump: %{public}s", contextConfig->GetName().c_str());
        std::vector<std::string> changeKeyV;
        contextConfig->CompareDifferent(changeKeyV, configuration);
        if (!changeKeyV.empty()) {
            contextConfig->Merge(changeKeyV, configuration);
        }
        NETMGR_EXT_LOG_D("Config dump after merge: %{public}s", contextConfig->GetName().c_str());
    }
    ConfigurationUpdated();
}

void JsVpnExtension::ConfigurationUpdated()
{
    NETMGR_EXT_LOG_D("called.");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    // Notify extension context
    auto fullConfig = GetContext()->GetConfiguration();
    if (!fullConfig) {
        NETMGR_EXT_LOG_E("configuration is nullptr.");
        return;
    }

    napi_value napiConfiguration = OHOS::AppExecFwk::WrapConfiguration(env, *fullConfig);
    CallObjectMethod("onConfigurationUpdated", &napiConfiguration, ARGC_ONE);
    CallObjectMethod("onConfigurationUpdate", &napiConfiguration, ARGC_ONE);
    JsExtensionContext::ConfigurationUpdated(env, shellContextRef_, fullConfig);
}

void JsVpnExtension::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    Extension::Dump(params, info);
    NETMGR_EXT_LOG_I("call");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    // create js array object of params
    napi_value argv[] = { CreateNativeArray(env, params) };

    if (!jsObj_) {
        HILOG_WARN("Not found VpnExtension.js");
        return;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        NETMGR_EXT_LOG_E("Failed to get VpnExtension object");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onDump", &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        method = nullptr;
        napi_get_named_property(env, obj, "dump", &method);
        if (!CheckTypeForNapiValue(env, method, napi_function)) {
            NETMGR_EXT_LOG_E("Failed to get onConnect from VpnExtension object");
            return;
        }
    }
    NETMGR_EXT_LOG_I("JsVpnExtension::CallFunction onConnect, success");
    napi_value dumpInfo = nullptr;
    napi_call_function(env, obj, method, ARGC_ONE, argv, &dumpInfo);
    if (dumpInfo == nullptr) {
        NETMGR_EXT_LOG_E("dumpInfo nullptr.");
        return;
    }
    uint32_t len = 0;
    napi_get_array_length(env, dumpInfo, &len);
    for (uint32_t i = 0; i < len; i++) {
        std::string dumpInfoStr;
        napi_value element = nullptr;
        napi_get_element(env, dumpInfo, i, &element);
        if (!ConvertFromJsValue(env, element, dumpInfoStr)) {
            NETMGR_EXT_LOG_E("Parse dumpInfoStr failed.");
            return;
        }
        info.push_back(dumpInfoStr);
    }
    NETMGR_EXT_LOG_D("Dump info size: %{public}zu", info.size());
}

#ifdef SUPPORT_GRAPHICS
void JsVpnExtension::OnCreate(Rosen::DisplayId displayId)
{
    NETMGR_EXT_LOG_D("enter.");
}

void JsVpnExtension::OnDestroy(Rosen::DisplayId displayId)
{
    NETMGR_EXT_LOG_D("exit.");
}

void JsVpnExtension::OnChange(Rosen::DisplayId displayId)
{
    NETMGR_EXT_LOG_D("displayId: %{public}" PRIu64"", displayId);
    auto context = GetContext();
    if (context == nullptr) {
        NETMGR_EXT_LOG_E("Context is invalid.");
        return;
    }

    auto contextConfig = context->GetConfiguration();
    if (contextConfig == nullptr) {
        NETMGR_EXT_LOG_E("Configuration is invalid.");
        return;
    }

    NETMGR_EXT_LOG_D("Config dump: %{public}s", contextConfig->GetName().c_str());
    bool configChanged = false;
    auto configUtils = std::make_shared<ConfigurationUtils>();
    configUtils->UpdateDisplayConfig(displayId, contextConfig, context->GetResourceManager(), configChanged);
    NETMGR_EXT_LOG_D("Config dump after update: %{public}s", contextConfig->GetName().c_str());

    if (configChanged) {
        auto jsVpnExtension = std::static_pointer_cast<JsVpnExtension>(shared_from_this());
        auto task = [jsVpnExtension]() {
            if (jsVpnExtension) {
                jsVpnExtension->ConfigurationUpdated();
            }
        };
        if (handler_ != nullptr) {
            handler_->PostTask(task, "JsVpnExtension:OnChange");
        }
    }

    NETMGR_EXT_LOG_D("finished.");
}
#endif
} // NetManagerStandard
} // OHOS
