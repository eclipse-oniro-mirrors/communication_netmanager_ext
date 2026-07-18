/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "ets_vpn_extension.h"

#include "ability_handler.h"
#include "ability_info.h"
#include "ani_common_want.h"
#include "configuration_utils.h"
#include "connection_manager.h"
#include "ets_extension_common.h"
#include "ets_extension_context.h"
#include "ets_vpn_extension_context.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "ipc_object_stub.h"
#include "netmgr_ext_log_wrapper.h"
#include "runtime.h"
#ifdef SUPPORT_GRAPHICS
#include "display_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "window_scene.h"
#endif

using namespace OHOS::AbilityRuntime;
namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr const char *ON_CREATE_SIGNATURE = "C{@ohos.app.ability.Want.Want}:";
constexpr const char *VOID_SIGNATURE = ":";
constexpr const char *ON_DISCONNECT_SIGNATURE = "C{@ohos.app.ability.Want.Want}:";
constexpr const char *ON_REQUEST_SIGNATURE = "C{@ohos.app.ability.Want.Want}i:";
constexpr const char *ON_CONFIGURATION_UPDATE_SIGNATURE = "C{@ohos.app.ability.Configuration.Configuration}:";
constexpr const char *ON_DUMP_SIGNATURE = "C{std.core.Array}:C{std.core.Array}";
constexpr uint32_t UID_NET_MANAGER = 1099;
}  // namespace

using namespace OHOS::AppExecFwk;

EtsVpnExtension *EtsVpnExtension::Create(const std::unique_ptr<Runtime> &runtime)
{
    return new EtsVpnExtension(static_cast<ETSRuntime &>(*runtime));
}

EtsVpnExtension::EtsVpnExtension(ETSRuntime &etsRuntime) : etsRuntime_(etsRuntime) {}

EtsVpnExtension::~EtsVpnExtension()
{
    NETMGR_EXT_LOG_D("Ets vpn extension destructor.");
    auto context = GetContext();
    if (context) {
        context->Unbind();
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        NETMGR_EXT_LOG_E("null env");
        return;
    }
    if (shellContextRef_ && shellContextRef_->aniRef) {
        env->GlobalReference_Delete(shellContextRef_->aniRef);
    }
}

void EtsVpnExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    VpnExtension::Init(record, application, handler, token);
    if (Extension::abilityInfo_ == nullptr || Extension::abilityInfo_->srcEntrance.empty()) {
        NETMGR_EXT_LOG_E("EtsVpnExtension Init abilityInfo error");
        return;
    }
    std::string srcPath;
    GetSrcPath(srcPath);
    if (srcPath.empty()) {
        NETMGR_EXT_LOG_E("Failed to get srcPath");
        return;
    }
    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    NETMGR_EXT_LOG_D("EtsVpnExtension::Init moduleName:%{public}s,srcPath:%{public}s.",
        moduleName.c_str(), srcPath.c_str());
    etsObj_ = etsRuntime_.LoadModule(moduleName, srcPath, abilityInfo_->hapPath,
        abilityInfo_->compileMode == CompileMode::ES_MODULE, false, abilityInfo_->srcEntrance);
    if (etsObj_ == nullptr) {
        NETMGR_EXT_LOG_E("Failed to get etsObj_");
        return;
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        NETMGR_EXT_LOG_E("null env");
        return;
    }
    BindContext(env, record->GetWant());
    SetExtensionCommon(EtsExtensionCommon::Create(
        etsRuntime_, static_cast<AppExecFwk::ETSNativeReference &>(*etsObj_), shellContextRef_));
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

void EtsVpnExtension::GetSrcPath(std::string &srcPath)
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

void EtsVpnExtension::ListenWMS()
{
#ifdef SUPPORT_GRAPHICS
    NETMGR_EXT_LOG_I("RegisterDisplayListener");
    auto abilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (abilityManager == nullptr) {
        NETMGR_EXT_LOG_E("Failed to get SaMgr.");
        return;
    }

    auto etsVpnExtension = std::static_pointer_cast<EtsVpnExtension>(shared_from_this());
    displayListener_ = sptr<ETSVpnExtensionDisplayListener>::MakeSptr(etsVpnExtension);
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

void EtsVpnExtension::SystemAbilityStatusChangeListener::OnAddSystemAbility(int32_t systemAbilityId,
    const std::string &deviceId)
{
    NETMGR_EXT_LOG_I("systemAbilityId: %{public}d add", systemAbilityId);
    if (systemAbilityId == WINDOW_MANAGER_SERVICE_ID) {
        Rosen::DisplayManager::GetInstance().RegisterDisplayListener(tmpDisplayListener_);
    }
}

void EtsVpnExtension::OnStart(const AAFwk::Want &want)
{
    Extension::OnStart(want);
    NETMGR_EXT_LOG_I("call");

    auto context = GetContext();
    if (context == nullptr) {
        NETMGR_EXT_LOG_E("context is null");
        return;
    }

#ifdef SUPPORT_GRAPHICS
    int displayId = want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, Rosen::WindowScene::DEFAULT_DISPLAY_ID);
    auto configUtils = std::make_shared<ConfigurationUtils>();
    configUtils->InitDisplayConfig(displayId, context->GetConfiguration(), context->GetResourceManager());
#endif

    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        NETMGR_EXT_LOG_E("env not found Ability.ets");
        return;
    }
    EtsExtensionContext::ConfigurationUpdated(env, shellContextRef_, context->GetConfiguration());

    ani_ref wantRef = OHOS::AppExecFwk::WrapWant(env, want);
    if (wantRef == nullptr) {
        NETMGR_EXT_LOG_E("null wantRef");
        return;
    }
    CallObjectMethod(false, "onCreate", ON_CREATE_SIGNATURE, wantRef);
    NETMGR_EXT_LOG_I("ok");
}

void EtsVpnExtension::OnStop()
{
    VpnExtension::OnStop();
    NETMGR_EXT_LOG_I("call");
    CallObjectMethod(false, "onDestroy", VOID_SIGNATURE);
    bool ret = ConnectionManager::GetInstance().DisconnectCaller(GetContext()->GetToken());
    if (ret) {
        ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
        NETMGR_EXT_LOG_I("The vpn extension connection is not disconnected.");
    }
#ifdef SUPPORT_GRAPHICS
    Rosen::DisplayManager::GetInstance().UnregisterDisplayListener(displayListener_);
#endif
    NETMGR_EXT_LOG_I("ok");
}

sptr<IRemoteObject> EtsVpnExtension::OnConnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnConnect(want);
    NETMGR_EXT_LOG_D("call");
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        NETMGR_EXT_LOG_E("null env");
        return nullptr;
    }
    ani_ref wantRef = OHOS::AppExecFwk::WrapWant(env, want);
    if (wantRef == nullptr) {
        NETMGR_EXT_LOG_E("null wantRef");
        return nullptr;
    }
    ani_ref result = CallObjectMethod(true, "onConnect", nullptr, wantRef);
    auto obj = reinterpret_cast<ani_object>(result);
    if (obj == nullptr && want.GetIntParam(Want::PARAM_RESV_CALLER_UID, -1) == static_cast<int>(UID_NET_MANAGER)) {
        return static_cast<sptr<IRemoteObject>>(new IPCObjectStub());
    }
    auto remoteObj = AniGetNativeRemoteObject(env, obj);
    if (remoteObj == nullptr) {
        NETMGR_EXT_LOG_E("remoteObj null.");
    }
    NETMGR_EXT_LOG_D("end");
    return remoteObj;
}

void EtsVpnExtension::OnDisconnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnDisconnect(want);
    NETMGR_EXT_LOG_D("call");
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        NETMGR_EXT_LOG_E("null env");
        return;
    }
    ani_ref wantRef = OHOS::AppExecFwk::WrapWant(env, want);
    if (wantRef == nullptr) {
        NETMGR_EXT_LOG_E("null wantRef");
        return;
    }
    CallObjectMethod(false, "onDisconnect", ON_DISCONNECT_SIGNATURE, wantRef);
    NETMGR_EXT_LOG_D("end");
}

void EtsVpnExtension::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnCommand(want, restart, startId);
    NETMGR_EXT_LOG_I("restart=%{public}s,startId=%{public}d.", restart ? "true" : "false", startId);
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        NETMGR_EXT_LOG_E("null env");
        return;
    }
    ani_ref wantRef = OHOS::AppExecFwk::WrapWant(env, want);
    if (wantRef == nullptr) {
        NETMGR_EXT_LOG_E("null wantRef");
        return;
    }
    ani_int iStartId = static_cast<ani_int>(startId);
    CallObjectMethod(false, "onRequest", ON_REQUEST_SIGNATURE, wantRef, iStartId);
    NETMGR_EXT_LOG_I("ok");
}

void EtsVpnExtension::OnConfigurationUpdated(const AppExecFwk::Configuration &configuration)
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

void EtsVpnExtension::ConfigurationUpdated()
{
    NETMGR_EXT_LOG_D("called.");
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        NETMGR_EXT_LOG_E("null env");
        return;
    }
    auto context = GetContext();
    if (context == nullptr) {
        NETMGR_EXT_LOG_E("Context is invalid.");
        return;
    }
    auto fullConfig = context->GetConfiguration();
    if (!fullConfig) {
        NETMGR_EXT_LOG_E("configuration is nullptr.");
        return;
    }
    ani_object aniConfiguration = OHOS::AppExecFwk::WrapConfiguration(env, *fullConfig);
    if (aniConfiguration == nullptr) {
        NETMGR_EXT_LOG_E("WrapConfiguration failed.");
        return;
    }
    CallObjectMethod(false, "onConfigurationUpdate", ON_CONFIGURATION_UPDATE_SIGNATURE, aniConfiguration);
    EtsExtensionContext::ConfigurationUpdated(env, shellContextRef_, fullConfig);
}

void EtsVpnExtension::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    Extension::Dump(params, info);
    NETMGR_EXT_LOG_I("call");
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        NETMGR_EXT_LOG_E("null env");
        return;
    }
    if (etsObj_ == nullptr) {
        NETMGR_EXT_LOG_E("Not found VpnExtension.ets");
        return;
    }
    ani_object aniParams = nullptr;
    if (!WrapArrayString(env, aniParams, params)) {
        NETMGR_EXT_LOG_E("WrapArrayString params failed.");
        return;
    }
    ani_method method = nullptr;
    if (env->Class_FindMethod(etsObj_->aniCls, "onDump", ON_DUMP_SIGNATURE, &method) != ANI_OK ||
        method == nullptr) {
        NETMGR_EXT_LOG_E("Failed to get onDump from VpnExtension object");
        return;
    }
    ani_ref result = nullptr;
    if (env->Object_CallMethod_Ref(etsObj_->aniObj, method, &result, aniParams) != ANI_OK || result == nullptr) {
        NETMGR_EXT_LOG_E("Call onDump failed.");
        return;
    }
    ani_object dumpInfo = reinterpret_cast<ani_object>(result);
    if (!UnwrapArrayString(env, dumpInfo, info)) {
        NETMGR_EXT_LOG_E("UnwrapArrayString dumpInfo failed.");
        return;
    }
    NETMGR_EXT_LOG_D("Dump info size: %{public}zu", info.size());
}

ani_ref EtsVpnExtension::CallObjectMethod(bool withResult, const char* name, const char* signature, ...)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, std::string("CallObjectMethod:") + name);
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        NETMGR_EXT_LOG_E("null env");
        return nullptr;
    }
    if (etsObj_ == nullptr) {
        NETMGR_EXT_LOG_E("Not found VpnExtension.ets");
        return nullptr;
    }
    if ((status = env->Class_FindMethod(etsObj_->aniCls, name, signature, &method)) != ANI_OK) {
        NETMGR_EXT_LOG_E("Failed to get '%{public}s' from VpnExtension object, status: %{public}d", name, status);
        return nullptr;
    }
    if (method == nullptr) {
        NETMGR_EXT_LOG_E("null method '%{public}s'", name);
        return nullptr;
    }
    env->ResetError();
    ani_ref res = nullptr;
    va_list args;
    if (withResult) {
        va_start(args, signature);
        if ((status = env->Object_CallMethod_Ref_V(etsObj_->aniObj, method, &res, args)) != ANI_OK) {
            NETMGR_EXT_LOG_E("Object_CallMethod_Ref_V '%{public}s' status: %{public}d", name, status);
            etsRuntime_.HandleUncaughtError();
            va_end(args);
            return nullptr;
        }
        va_end(args);
        return res;
    }
    va_start(args, signature);
    if ((status = env->Object_CallMethod_Void_V(etsObj_->aniObj, method, args)) != ANI_OK) {
        NETMGR_EXT_LOG_E("Object_CallMethod_Void_V '%{public}s' status: %{public}d", name, status);
        etsRuntime_.HandleUncaughtError();
    }
    va_end(args);
    return nullptr;
}

void EtsVpnExtension::BindContext(ani_env* env, std::shared_ptr<AAFwk::Want> want)
{
    NETMGR_EXT_LOG_D("BindContext");
    if (env == nullptr || want == nullptr) {
        NETMGR_EXT_LOG_E("Want info is null or env is null");
        return;
    }
    auto context = GetContext();
    if (context == nullptr) {
        NETMGR_EXT_LOG_E("Failed to get context");
        return;
    }
    ani_object contextObj = CreateEtsVpnExtensionContext(env, context);
    if (contextObj == nullptr) {
        NETMGR_EXT_LOG_E("null contextObj");
        return;
    }
    ani_field contextField = nullptr;
    auto status = env->Class_FindField(etsObj_->aniCls, "context", &contextField);
    if (status != ANI_OK) {
        NETMGR_EXT_LOG_E("Class_FindField context failed, status: %{public}d", status);
        return;
    }
    ani_ref contextRef = nullptr;
    if ((status = env->GlobalReference_Create(contextObj, &contextRef)) != ANI_OK) {
        NETMGR_EXT_LOG_E("GlobalReference_Create contextObj failed, status: %{public}d", status);
        return;
    }
    if ((status = env->Object_SetField_Ref(etsObj_->aniObj, contextField, contextRef)) != ANI_OK) {
        NETMGR_EXT_LOG_E("Object_SetField_Ref contextObj failed, status: %{public}d", status);
        return;
    }
    shellContextRef_ = std::make_shared<AppExecFwk::ETSNativeReference>();
    shellContextRef_->aniObj = contextObj;
    shellContextRef_->aniRef = contextRef;
    NETMGR_EXT_LOG_D("BindContext end");
}

#ifdef SUPPORT_GRAPHICS
void EtsVpnExtension::OnCreate(Rosen::DisplayId displayId)
{
    NETMGR_EXT_LOG_D("enter.");
}

void EtsVpnExtension::OnDestroy(Rosen::DisplayId displayId)
{
    NETMGR_EXT_LOG_D("exit.");
}

void EtsVpnExtension::OnChange(Rosen::DisplayId displayId)
{
    NETMGR_EXT_LOG_D("displayId: %{public}" PRIu64 "", displayId);
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
        auto etsVpnExtension = std::static_pointer_cast<EtsVpnExtension>(shared_from_this());
        auto task = [etsVpnExtension]() {
            if (etsVpnExtension) {
                etsVpnExtension->ConfigurationUpdated();
            }
        };
        if (handler_ != nullptr) {
            handler_->PostTask(task, "EtsVpnExtension:OnChange");
        }
    }

    NETMGR_EXT_LOG_D("finished.");
}
#endif
}  // namespace NetManagerStandard
}  // namespace OHOS
