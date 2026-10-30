/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef COMMON_MOCK_NETMANAGER_PERMISSION_H
#define COMMON_MOCK_NETMANAGER_PERMISSION_H

namespace OHOS {
namespace NetManagerStandard {
/**
 * @brief control the mocked NetManagerPermission::CheckPermission / CheckPermissionWithCache
 * result, so that unit tests do not depend on the AccessToken environment of the device.
 */
void SetMockCheckPermissionResult(bool result);

/**
 * @brief control the mocked NetManagerPermission::IsSystemCaller result.
 */
void SetMockIsSystemCallerResult(bool result);
} // namespace NetManagerStandard
} // namespace OHOS

#endif // COMMON_MOCK_NETMANAGER_PERMISSION_H
