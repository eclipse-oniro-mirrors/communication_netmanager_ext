/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "netfirewall_common.h"

#include "netmgr_ext_log_wrapper.h"
#include "refbase.h"
#include <sstream>

namespace OHOS {
namespace NetManagerStandard {
// 防火墙状态
bool NetFirewallStatus::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteBool(isOpen)) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(inAction))) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(outAction))) {
        return false;
    }
    return true;
}

sptr<NetFirewallStatus> NetFirewallStatus::Unmarshalling(Parcel &parcel)
{
    sptr<NetFirewallStatus> ptr = new (std::nothrow) NetFirewallStatus();
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("NetFirewallStatus ptr is null");
        return nullptr;
    }
    if (!parcel.ReadBool(ptr->isOpen)) {
        return nullptr;
    }
    int32_t inAction = 0;
    if (!parcel.ReadInt32(inAction)) {
        return nullptr;
    }
    int32_t outAction = 0;
    if (!parcel.ReadInt32(outAction)) {
        return nullptr;
    }
    ptr->inAction = static_cast<FirewallRuleAction>(inAction);
    ptr->outAction = static_cast<FirewallRuleAction>(outAction);
    return ptr;
}
} // namespace NetManagerStandard
} // namespace OHOS