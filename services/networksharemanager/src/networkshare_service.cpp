/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "networkshare_service.h"

#include <system_ability_definition.h>
#include "netmgr_ext_log_wrapper.h"
#include "networkshare_constants.h"
#include "net_manager_center.h"

namespace OHOS {
namespace NetManagerStandard {
const bool REGISTER_LOCAL_RESULT_NETSHARE =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<NetworkShareService>::GetInstance().get());

NetworkShareService::NetworkShareService() : SystemAbility(COMM_NET_TETHERING_MANAGER_SYS_ABILITY_ID, true) {}

NetworkShareService::~NetworkShareService() {};

void NetworkShareService::OnStart()
{
    if (state_ == STATE_RUNNING) {
        NETMGR_EXT_LOG_D("OnStart Service state is already running");
        return;
    }
    if (!Init()) {
        NETMGR_EXT_LOG_E("OnStart init failed");
        return;
    }
    state_ = STATE_RUNNING;
    NETMGR_EXT_LOG_I("OnStart successful");
}

void NetworkShareService::OnStop()
{
    NetworkShareTracker::GetInstance().Uninit();
    state_ = STATE_STOPPED;
    registerToService_ = false;
    NETMGR_EXT_LOG_I("OnStop successful");
}

bool NetworkShareService::Init()
{
    if (!REGISTER_LOCAL_RESULT_NETSHARE) {
        NETMGR_EXT_LOG_E("Register to local sa manager failed");
        return false;
    }
    if (!registerToService_) {
        if (!Publish(DelayedSingleton<NetworkShareService>::GetInstance().get())) {
            NETMGR_EXT_LOG_E("Register to sa manager failed");
            return false;
        }
        registerToService_ = true;
    }
    return NetworkShareTracker::GetInstance().Init();
}

int32_t NetworkShareService::IsNetworkSharingSupported()
{
    return NetworkShareTracker::GetInstance().IsNetworkSharingSupported();
}

int32_t NetworkShareService::IsSharing()
{
    return NetworkShareTracker::GetInstance().IsSharing();
}

int32_t NetworkShareService::StartNetworkSharing(const SharingIfaceType &type)
{
    return NetworkShareTracker::GetInstance().StartNetworkSharing(type);
}

int32_t NetworkShareService::StopNetworkSharing(const SharingIfaceType &type)
{
    return NetworkShareTracker::GetInstance().StopNetworkSharing(type);
}

int32_t NetworkShareService::RegisterSharingEvent(sptr<ISharingEventCallback> callback)
{
    return NetworkShareTracker::GetInstance().RegisterSharingEvent(callback);
}

int32_t NetworkShareService::UnregisterSharingEvent(sptr<ISharingEventCallback> callback)
{
    return NetworkShareTracker::GetInstance().UnregisterSharingEvent(callback);
}

std::vector<std::string> NetworkShareService::GetSharableRegexs(SharingIfaceType type)
{
    return NetworkShareTracker::GetInstance().GetSharableRegexs(type);
}

int32_t NetworkShareService::GetSharingState(SharingIfaceType type, SharingIfaceState &state)
{
    return NetworkShareTracker::GetInstance().GetSharingState(type, state);
}

std::vector<std::string> NetworkShareService::GetNetSharingIfaces(const SharingIfaceState &state)
{
    return NetworkShareTracker::GetInstance().GetNetSharingIfaces(state);
}
} // namespace NetManagerStandard
} // namespace OHOS
