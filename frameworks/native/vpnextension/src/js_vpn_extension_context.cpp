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

#include "js_vpn_extension_context.h"

#include <chrono>
#include <cstdint>
#include "ability_manager_client.h"
#include "ability_runtime/js_caller_complex.h"
#include "hilog_wrapper.h"
#include "js_extension_context.h"
#include "js_error_utils.h"
#include "js_data_struct_converter.h"
#include "runtime.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_common_ability.h"
#include "napi_common_want.h"
#include "napi_common_util.h"
#include "napi_remote_object.h"
#include "napi_common_start_options.h"
#include "start_options.h"
#include "hitrace_meter.h"
#include "netmgr_ext_log_wrapper.h"
#include "ability_business_error/ability_business_error.h"

using namespace OHOS::AbilityRuntime;
namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr size_t ARGC_ONE = 1;

class StartAbilityByCallParameters {
public:
    int err = 0;
    sptr<IRemoteObject> remoteCallee = nullptr;
    std::shared_ptr<CallerCallBack> callerCallBack = nullptr;
    std::mutex mutexlock;
    std::condition_variable condition;
};

static std::map<ConnectionKey, sptr<JSVpnExtensionContext>, key_compare> g_connects;

class JsVpnExtensionContext final {
public:
    explicit JsVpnExtensionContext(const std::shared_ptr<VpnExtensionContext>& context) : context_(context) {}
    ~JsVpnExtensionContext() = default;

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        NETMGR_EXT_LOG_D("JsAbilityContext::Finalizer is called");
        std::unique_ptr<JsVpnExtensionContext>(static_cast<JsVpnExtensionContext*>(data));
    }

    static napi_value StartVpnExtensionAbility(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsVpnExtensionContext, OnStartExtensionAbility);
    }

    static napi_value StopVpnExtensionAbility(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsVpnExtensionContext, OnStopExtensionAbility);
    }

private:
    std::weak_ptr<VpnExtensionContext> context_;
    sptr<JsFreeInstallObserver> freeInstallObserver_ = nullptr;
    static void ClearFailedCallConnection(
        const std::weak_ptr<VpnExtensionContext>& vpnContext, const std::shared_ptr<CallerCallBack> &callback)
    {
        NETMGR_EXT_LOG_D("clear failed call of startup is called.");
        auto context = vpnContext.lock();
        if (context == nullptr || callback == nullptr) {
            NETMGR_EXT_LOG_E("clear failed call of startup input param is nullptr.");
            return;
        }

        context->ClearFailedCallConnection(callback);
    }

    napi_value OnStartExtensionAbility(napi_env env, NapiCallbackInfo& info)
    {
        NETMGR_EXT_LOG_I("StartExtensionAbility");
        if (info.argc < ARGC_ONE) {
            NETMGR_EXT_LOG_E("Start extension failed, not enough params.");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        AAFwk::Want want;
        if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [weak = context_, want](napi_env env, NapiAsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                    return;
                }
                auto innerErrorCode = context->StartVpnExtensionAbility(want);
                if (innerErrorCode == 0) {
                    task.Resolve(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env, CreateJsErrorByNativeErr(env, innerErrorCode));
                }
            };

        napi_value lastParam = (info.argc <= ARGC_ONE) ? nullptr : info.argv[ARGC_ONE];
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JSVpnExtensionContext::OnStartExtensionAbility",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnStopExtensionAbility(napi_env env, NapiCallbackInfo& info)
    {
        NETMGR_EXT_LOG_I("StopExtensionAbility");
        if (info.argc < ARGC_ONE) {
            NETMGR_EXT_LOG_E("Start extension failed, not enough params.");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        AAFwk::Want want;
        if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [weak = context_, want](napi_env env, NapiAsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                    return;
                }
                auto innerErrorCode = context->StopVpnExtensionAbility(want);
                if (innerErrorCode == 0) {
                    task.Resolve(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env, CreateJsErrorByNativeErr(env, innerErrorCode));
                }
            };

        napi_value lastParam = (info.argc <= ARGC_ONE) ? nullptr : info.argv[ARGC_ONE];
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JSVpnExtensionContext::OnStopExtensionAbility",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }
};
} // namespace

napi_value CreateJsVpnExtensionContext(napi_env env, std::shared_ptr<VpnExtensionContext> context)
{
    NETMGR_EXT_LOG_D("CreateJsVpnExtensionContext");
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    if (context) {
        abilityInfo = context->GetAbilityInfo();
    }
    napi_value object = CreateJsExtensionContext(env, context, abilityInfo);

    std::unique_ptr<JsVpnExtensionContext> jsContext = std::make_unique<JsVpnExtensionContext>(context);
    napi_wrap(env, object, jsContext.release(), JsVpnExtensionContext::Finalizer, nullptr, nullptr);

    const char *moduleName = "JsVpnExtensionContext";
    BindNativeFunction(env, object, "startVpnExtensionAbility", moduleName,
        JsVpnExtensionContext::StartVpnExtensionAbility);
    BindNativeFunction(env, object, "stopVpnExtensionAbility", moduleName,
        JsVpnExtensionContext::StopVpnExtensionAbility);
    return object;
}

JSVpnExtensionContext::JSVpnExtensionContext(napi_env env) : env_(env) {}

JSVpnExtensionContext::~JSVpnExtensionContext()
{
    if (jsConnectionObject_ == nullptr) {
        return;
    }

    uv_loop_t *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        return;
    }

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        return;
    }
    work->data = reinterpret_cast<void *>(jsConnectionObject_.release());
    int ret = uv_queue_work(loop, work, [](uv_work_t *work) {},
    [](uv_work_t *work, int status) {
        if (work == nullptr) {
            return;
        }
        if (work->data == nullptr) {
            delete work;
            work = nullptr;
            return;
        }
        delete reinterpret_cast<NativeReference *>(work->data);
        work->data = nullptr;
        delete work;
        work = nullptr;
    });
    if (ret != 0) {
        delete reinterpret_cast<NativeReference *>(work->data);
        work->data = nullptr;
        delete work;
        work = nullptr;
    }
}
}  // namespace NetManagerStandard
}  // namespace OHOS
