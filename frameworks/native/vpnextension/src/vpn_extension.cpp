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

#include "vpn_extension.h"

#include "configuration_utils.h"
#include "connection_manager.h"
#include "js_vpn_extension.h"
#include "runtime.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "vpn_extension_context.h"
#include "netmgr_ext_log_wrapper.h"

using namespace OHOS::AbilityRuntime;
namespace OHOS {
namespace NetManagerStandard {

using namespace OHOS::AppExecFwk;

CreatorFunc VpnExtension::creator_ = nullptr;
void VpnExtension::SetCreator(const CreatorFunc& creator)
{
    creator_ = creator;
}

VpnExtension* VpnExtension::Create(const std::unique_ptr<OHOS::AbilityRuntime::Runtime>& runtime)
{
    if (!runtime) {
        return new VpnExtension();
    }

    if (creator_) {
        return creator_(runtime);
    }

    NETMGR_EXT_LOG_I("VpnExtension::Create runtime");
    switch (runtime->GetLanguage()) {
        case OHOS::AbilityRuntime::Runtime::Language::JS:
            return JsVpnExtension::Create(runtime);

        default:
            return new VpnExtension();
    }
}

void VpnExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    ExtensionBase<VpnExtensionContext>::Init(record, application, handler, token);
    NETMGR_EXT_LOG_I("VpnExtension begin init context");
}

std::shared_ptr<VpnExtensionContext> VpnExtension::CreateAndInitContext(
    const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    std::shared_ptr<VpnExtensionContext> context =
        ExtensionBase<VpnExtensionContext>::CreateAndInitContext(record, application, handler, token);
    if (context == nullptr) {
        NETMGR_EXT_LOG_E("VpnExtension::CreateAndInitContext context is nullptr");
        return context;
    }
    return context;
}

void VpnExtension::OnConfigurationUpdated(const AppExecFwk::Configuration &configuration)
{
    Extension::OnConfigurationUpdated(configuration);

    auto context = GetContext();
    if (context == nullptr) {
        NETMGR_EXT_LOG_E("Context is invalid.");
        return;
    }

    auto configUtils = std::make_shared<OHOS::AbilityRuntime::ConfigurationUtils>();
    configUtils->UpdateGlobalConfig(configuration, context->GetResourceManager());
}
} // namespace NetManagerStandard
} // namespace OHOS
