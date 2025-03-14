/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "urspconfig.h"
#include "cellular_data_client.h"
#include "hwnetworkslicemanager.h"
#include "networkslicemanager.h"
#include <parameters.h>
namespace OHOS {
namespace NetManagerStandard {
UrspConfig& UrspConfig::GetInstance()
{
    static UrspConfig instance;
    return instance;
}

UrspConfig::UrspConfig()
{
}

bool UrspConfig::SliceNetworkSelection(
    SelectedRouteDescriptor& routeRule, std::string plmn, AppDescriptor appDescriptor)
{
    return false;
}

bool UrspConfig::isIpThreeTuplesInWhiteList(std::string plmn, AppDescriptor appDescriptor)
{
    return false;
}

} // namespace NetManagerStandard
} // namespace OHOS
