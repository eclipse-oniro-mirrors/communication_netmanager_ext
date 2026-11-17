/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "net_mgr_log_wrapper.h"
#include "netmanager_base_permission.h"

#include "common_mock_netmanager_permission.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
// Defaults keep the historical all-pass behavior (sys_vpn_manager_test relies on it).
// Ethernet unit tests set the device-realistic baseline (system caller passes,
// permission check fails) in their fixture SetUp and override per case as needed.
bool g_mockCheckPermissionResult = true;
bool g_mockIsSystemCallerResult = true;
} // namespace

void SetMockCheckPermissionResult(bool result)
{
    g_mockCheckPermissionResult = result;
}

void SetMockIsSystemCallerResult(bool result)
{
    g_mockIsSystemCallerResult = result;
}

bool NetManagerPermission::CheckPermissionWithCache(const std::string &permissionName)
{
    NETMGR_LOG_D("permissionName: %{public}s", permissionName.c_str());
    return g_mockCheckPermissionResult;
}

bool NetManagerPermission::CheckPermission(const std::string &permissionName)
{
    NETMGR_LOG_D("permissionName: %{public}s", permissionName.c_str());
    return g_mockCheckPermissionResult;
}

bool NetManagerPermission::IsSystemCaller()
{
    NETMGR_LOG_D("Is conn caller.");
    return g_mockIsSystemCallerResult;
}

bool NetManagerPermission::CheckNetSysInternalPermission(const std::string &permissionName)
{
    NETMGR_LOG_D("permissionName: %{public}s", permissionName.c_str());
    return true;
}

} // namespace NetManagerStandard
} // namespace OHOS
