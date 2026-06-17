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

#include "vpn_extension_context.h"

#include "ability_connection.h"
#include "ability_manager_client.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "netmgr_ext_log_wrapper.h"
#include "runtime.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"

using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace NetManagerStandard {
const size_t VpnExtensionContext::CONTEXT_TYPE_ID(std::hash<const char*> {} ("VpnExtensionContext"));
const std::string START_ABILITY_TYPE = "ABILITY_INNER_START_WITH_ACCOUNT";

int32_t VpnExtensionContext::ILLEGAL_REQUEST_CODE(-1);

void VpnExtensionContext::ClearFailedCallConnection(const std::shared_ptr<CallerCallBack> &callback) const
{
    NETMGR_EXT_LOG_D("%{public}s begin.", __func__);
    if (localCallContainer_ == nullptr) {
        NETMGR_EXT_LOG_E("%{public}s failed, localCallContainer_ is nullptr.", __func__);
        return;
    }
    localCallContainer_->ClearFailedCallConnection(callback);
}
}  // namespace NetManagerStandard
}  // namespace OHOS
