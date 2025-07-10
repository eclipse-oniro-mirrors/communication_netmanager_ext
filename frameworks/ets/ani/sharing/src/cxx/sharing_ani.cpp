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

#include "sharing_ani.h"

#include <cstdint>

#include "cxx.h"
#include "net_manager_ext_constants.h"
#include "wrapper.rs.h"

namespace OHOS {
namespace NetManagerAni {

SharingEventCallback::SharingEventCallback(rust::Box<SharingCallback> &&callback) : callback_(std::move(callback))
{
}

void SharingEventCallback::OnSharingStateChanged(const bool &isRunning)
{
    callback_->on_sharing_state_change(isRunning);
}

void SharingEventCallback::OnInterfaceSharingStateChanged(NetManagerStandard::SharingIfaceType const &type,
    std::string const &iface, NetManagerStandard::SharingIfaceState const &state)
{
    InterfaceSharingStateInfo info{
        .share_type = type,
        .iface = rust::String(iface),
        .state = state,
    };
    callback_->on_interface_sharing_state_change(info);
}

void SharingEventCallback::OnSharingUpstreamChanged(sptr<NetManagerStandard::NetHandle> netHandle)
{
    NetHandle handle{
        .net_id = netHandle->GetNetId(),
    };
    callback_->on_sharing_upstream_change(handle);
}

std::unique_ptr<SharingCallbackUnregister> RegisterSharingCallback(rust::Box<SharingCallback> callback, int32_t &ret)
{
    auto eventCallback = sptr<SharingEventCallback>::MakeSptr(std::move(callback));
    ret = DelayedSingleton<NetManagerStandard::NetworkShareClient>::GetInstance()->RegisterSharingEvent(eventCallback);
    return std::make_unique<SharingCallbackUnregister>(eventCallback);
}

SharingCallbackUnregister::SharingCallbackUnregister(sptr<SharingEventCallback> eventCallback)
    : eventCallback_(eventCallback)
{
}

int32_t SharingCallbackUnregister::Unregister() const
{
    return DelayedSingleton<NetManagerStandard::NetworkShareClient>::GetInstance()->UnregisterSharingEvent(
        eventCallback_);
}

} // namespace NetManagerAni
} // namespace OHOS
