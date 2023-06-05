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

#include "networkvpn_client.h"

#include <cerrno>
#include <chrono>
#include <cstdio>
#include <ostream>
#include <sstream>
#include <thread>

#include <netinet/in.h>
#include <securec.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include "fwmark_client.h"
#include "iservice_registry.h"
#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace NetManagerStandard {

int32_t NetworkVpnClient::Prepare(bool &isExistVpn, bool &isRun, std::string &pkg)
{
    sptr<INetworkVpnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("Prepare proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->Prepare(isExistVpn, isRun, pkg);
}

int32_t NetworkVpnClient::Protect(uint32_t socketFd)
{
    nmd::FwmarkClient fwmarkClient;
    return fwmarkClient.ProtectFromVpn(socketFd);
}

int32_t NetworkVpnClient::SetUp(sptr<VpnConfig> config, int32_t &tunFd)
{
    return 0;
}

int32_t NetworkVpnClient::DestroyVpn()
{
    return 0;
}

int32_t NetworkVpnClient::RegisterVpnEvent(sptr<IVpnEventCallback> callback)
{
    sptr<INetworkVpnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("RegisterVpnEvent proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->RegisterVpnEvent(callback);
}

int32_t NetworkVpnClient::UnregisterVpnEvent(sptr<IVpnEventCallback> callback)
{
    sptr<INetworkVpnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("UnregisterVpnEvent proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->UnregisterVpnEvent(callback);
}

sptr<INetworkVpnService> NetworkVpnClient::GetProxy()
{
    std::lock_guard lock(mutex_);
    if (networkVpnService_ != nullptr) {
        return networkVpnService_;
    }
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        NETMGR_EXT_LOG_E("get SystemAbilityManager failed");
        return nullptr;
    }
    sptr<IRemoteObject> remote = sam->CheckSystemAbility(COMM_VPN_MANAGER_SYS_ABILITY_ID);
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("get Remote vpn service failed");
        return nullptr;
    }
    deathRecipient_ = new (std::nothrow) MonitorVpnServiceDead(*this);
    if (deathRecipient_ == nullptr) {
        NETMGR_EXT_LOG_E("deathRecipient_ is nullptr");
        return nullptr;
    }
    if ((remote->IsProxyObject()) && (!remote->AddDeathRecipient(deathRecipient_))) {
        NETMGR_EXT_LOG_E("add death recipient failed");
        return nullptr;
    }
    networkVpnService_ = iface_cast<INetworkVpnService>(remote);
    if (networkVpnService_ == nullptr) {
        NETMGR_EXT_LOG_E("get Remote service proxy failed");
        return nullptr;
    }
    return networkVpnService_;
}

void NetworkVpnClient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("remote object is nullptr");
        return;
    }
    std::lock_guard lock(mutex_);
    if (networkVpnService_ == nullptr) {
        NETMGR_EXT_LOG_E("networkVpnService_ is nullptr");
        return;
    }
    sptr<IRemoteObject> local = networkVpnService_->AsObject();
    if (local != remote.promote()) {
        NETMGR_EXT_LOG_E("proxy and stub is not same remote object");
        return;
    }
    local->RemoveDeathRecipient(deathRecipient_);
    networkVpnService_ = nullptr;
}
} // namespace NetManagerStandard
} // namespace OHOS
