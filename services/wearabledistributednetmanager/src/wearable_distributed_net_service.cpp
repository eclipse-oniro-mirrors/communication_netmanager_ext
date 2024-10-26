/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "netmanager_base_permission.h"
#include "wearable_distributed_net_service.h"

namespace OHOS {
namespace NetManagerStandard {
REGISTER_SYSTEM_ABILITY_BY_ID(WearableDistributedNetService, COMM_WEARABLE_DISTRIBUTED_NET_ABILITY_ID, true);

WearableDistributedNetService::WearableDistributedNetService(int32_t saId, bool runOnCreate)
    : SystemAbility(saId, runOnCreate) {}

WearableDistributedNetService::~WearableDistributedNetService() = default;

void WearableDistributedNetService::OnStart()
{
    NETMGR_EXT_LOG_I("WearableDistributedNetService::OnStart begin");
    if (state_ == STATE_RUNNING) {
        NETMGR_EXT_LOG_D("WearableDistributedNetService the state is already running");
        return;
    }
    if (!Init()) {
        NETMGR_EXT_LOG_E("WearableDistributedNetService init failed");
        return;
    }
    state_ = STATE_RUNNING;
    NETMGR_EXT_LOG_I("WearableDistributedNetService::OnStart end");
}

void WearableDistributedNetService::OnStop()
{
    state_ = STATE_STOPPED;
    registerToService_ = false;
}

int32_t WearableDistributedNetService::SetupWearableDistributedNet(int32_t tcpPortId, int32_t udpPortId, bool isMetered)
{
    NETMGR_EXT_LOG_I("Wearable Distributed Net Service Setup Net");
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        NETMGR_EXT_LOG_E("Wearable Distributed Net Service Setup Net no permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return WearableDistributedNetManagement::GetInstance()
        .StartWearableDistributedNetwork(tcpPortId, udpPortId, isMetered);
}

int32_t WearableDistributedNetService::TearDownWearableDistributedNet()
{
    NETMGR_EXT_LOG_I("Wearable Distributed Net Service TearDown Net");
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        NETMGR_EXT_LOG_E("Wearable Distributed Net Service TearDown Net no permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return WearableDistributedNetManagement::GetInstance().StopWearableDistributedNetwork();
}

bool WearableDistributedNetService::Init()
{
    NETMGR_EXT_LOG_I("Wearable Distributed Net Service Init");
    AddSystemAbilityListener(NET_MANAGER_SYS_ABILITY_ID);
    if (!registerToService_) {
        if (!Publish(this)) {
            NETMGR_EXT_LOG_E("Wearable Distributed Net Service Register to sa manager failed");
            return false;
        }
        registerToService_ = true;
    }
    return true;
}
} // namespace NetManagerStandard
} // namespace OHOS