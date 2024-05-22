/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "net_firewall_exec.h"

#include <cstdint>
#include <securec.h>

#include "napi_utils.h"
#include "net_firewall_rule_parse.h"
#include "net_manager_constants.h"
#include "netmanager_ext_log.h"
#include "netfirewall_client.h"

#include "singleton.h"

namespace OHOS {
namespace NetManagerStandard {
namespace NetFirewallExec {
template <typename ContextT> static inline NetFirewallClient *GetNetFirewallInstance(ContextT *context)
{
    auto manager = context->GetManager();
    return (manager == nullptr) ? nullptr : reinterpret_cast<NetFirewallClient *>(manager->GetData());
}

bool ExecSetNetFirewallStatus(SetNetFirewallStatusContext *context)
{
    if (!context->IsParseOK()) {
        return false;
    }
    int32_t result =
        DelayedSingleton<NetFirewallClient>::GetInstance()->SetNetFirewallStatus(context->userId_, context->status_);
    if (result != FIREWALL_SUCCESS) {
        NETMANAGER_EXT_LOGE("ExecSetIfaceConfig error, errorCode: %{public}d", result);
        context->SetErrorCode(result);
        return false;
    }
    return true;
}

napi_value SetNetFirewallStatusCallback(SetNetFirewallStatusContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ExecGetNetFirewallStatus(GetNetFirewallStatusContext *context)
{
    if (!context->IsParseOK()) {
        return false;
    }
    int32_t result =
        DelayedSingleton<NetFirewallClient>::GetInstance()->GetNetFirewallStatus(context->userId_, context->status_);
    if (result != FIREWALL_SUCCESS || context->status_ == nullptr) {
        NETMANAGER_EXT_LOGE("ExecGetIfaceConfig error, errorCode: %{public}d", result);
        context->SetErrorCode(result);
        return false;
    }
    return true;
}

napi_value GetNetFirewallStatusCallback(GetNetFirewallStatusContext *context)
{
    napi_value firewallStatus = NapiUtils::CreateObject(context->GetEnv());

    NapiUtils::SetBooleanProperty(context->GetEnv(), firewallStatus, NET_FIREWALL_IS_OPEN, context->status_->isOpen);
    NapiUtils::SetInt32Property(context->GetEnv(), firewallStatus, NET_FIREWALL_IN_ACTION,
        static_cast<int32_t>(context->status_->inAction));
    NapiUtils::SetInt32Property(context->GetEnv(), firewallStatus, NET_FIREWALL_OUT_ACTION,
        static_cast<int32_t>(context->status_->outAction));
    return firewallStatus;
}
} // namespace NetFirewallExec
} // namespace NetManagerStandard
} // namespace OHOS