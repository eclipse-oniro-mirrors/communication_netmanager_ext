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

#include "vpn_extension_module_loader.h"
#include "vpn_extension.h"
#include "netmgr_ext_log_wrapper.h"
#include "runtime.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
using namespace OHOS::AbilityRuntime;
namespace OHOS::NetManagerStandard {
VpnExtensionModuleLoader::VpnExtensionModuleLoader() = default;
VpnExtensionModuleLoader::~VpnExtensionModuleLoader() = default;

Extension *VpnExtensionModuleLoader::Create(const std::unique_ptr<OHOS::AbilityRuntime::Runtime>& runtime) const
{
    NETMGR_EXT_LOG_I("VpnExtensionModuleLoader::Create");
    return VpnExtension::Create(runtime);
}

std::map<std::string, std::string> VpnExtensionModuleLoader::GetParams()
{
    std::map<std::string, std::string> params;
    // type means extension type in ExtensionAbilityType of extension_ability_info.h, 502 means vpn.
    params.insert(std::pair<std::string, std::string>("type", "502"));
    // extension name
    params.insert(std::pair<std::string, std::string>("name", "VpnExtension"));
    return params;
}

extern "C" __attribute__((visibility("default"))) void* OHOS_EXTENSION_GetExtensionModule()
{
    return &VpnExtensionModuleLoader::GetInstance();
}
} // namespace OHOS::NetManagerStandard
