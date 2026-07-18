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

#include "ets_vpn_extension_context.h"

#include "ets_context_utils.h"
#include "ets_extension_context.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace AbilityRuntime;
namespace {
constexpr const char *VPN_EXTENSION_CONTEXT_CLASS_NAME = "application.VpnExtensionContext.VpnExtensionContext";

class EtsVpnExtensionContext final {
public:
    explicit EtsVpnExtensionContext(const std::shared_ptr<VpnExtensionContext> &context) : context_(context) {}

private:
    std::weak_ptr<VpnExtensionContext> context_;
};
}

ani_object CreateEtsVpnExtensionContext(ani_env *env, std::shared_ptr<VpnExtensionContext> context)
{
    NETMGR_EXT_LOG_D("CreateEtsVpnExtensionContext begin");
    if (env == nullptr) {
        NETMGR_EXT_LOG_E("Failed to create ets vpn extension context, env is null");
        return nullptr;
    }
    if (context == nullptr) {
        NETMGR_EXT_LOG_E("Failed to create ets vpn extension context, context is null");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_object contextObj = nullptr;
    if ((env->FindClass(VPN_EXTENSION_CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        NETMGR_EXT_LOG_E("Failed to find class, status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", "l:", &method)) != ANI_OK) {
        NETMGR_EXT_LOG_E("Failed to find constructor, status: %{public}d", status);
        return nullptr;
    }
    std::unique_ptr<EtsVpnExtensionContext> etsContext =
        std::make_unique<EtsVpnExtensionContext>(context);
    if ((status = env->Object_New(cls, method, &contextObj, reinterpret_cast<ani_long>(etsContext.release()))) !=
        ANI_OK) {
        NETMGR_EXT_LOG_E("Failed to create context object, status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_SetFieldByName_Long(contextObj, "nativeContext", (ani_long)context.get())) != ANI_OK) {
        NETMGR_EXT_LOG_E("Failed to set nativeContext, status: %{public}d", status);
        return nullptr;
    }
    ContextUtil::CreateEtsBaseContext(env, cls, contextObj, context);
    CreateEtsExtensionContext(env, cls, contextObj, context, context->GetAbilityInfo());
    NETMGR_EXT_LOG_D("CreateEtsVpnExtensionContext end");
    return contextObj;
}
}  // namespace NetManagerStandard
}  // namespace OHOS
