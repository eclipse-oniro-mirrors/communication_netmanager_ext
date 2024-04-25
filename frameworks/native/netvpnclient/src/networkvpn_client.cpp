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

#include <thread>

#include "fwmark_client.h"
#include "iservice_registry.h"
#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace NetManagerStandard {

static constexpr uint32_t WAIT_FOR_SERVICE_TIME_MS = 500;
static constexpr uint32_t MAX_GET_SERVICE_COUNT = 10;

void VpnSetUpEventCallback::OnVpnMultiUserSetUp()
{
    NETMGR_EXT_LOG_I("vpn multiple user setup event.");
    NetworkVpnClient::GetInstance().multiUserSetUpEvent();
}

NetworkVpnClient &NetworkVpnClient::GetInstance()
{
    static NetworkVpnClient instance;
    return instance;
}

int32_t NetworkVpnClient::Prepare(bool &isExistVpn, bool &isRun, std::string &pkg)
{
    sptr<INetworkVpnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("Prepare proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->Prepare(isExistVpn, isRun, pkg);
}

int32_t NetworkVpnClient::Protect(int32_t socketFd, bool isVpnExtCall)
{
    if (socketFd <= 0) {
        NETMGR_EXT_LOG_E("Invalid socket file discriptor");
        return NETWORKVPN_ERROR_INVALID_FD;
    }

    sptr<INetworkVpnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("Protect proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    int32_t result = proxy->Protect(isVpnExtCall);
    if (result != NETMANAGER_EXT_SUCCESS) {
        return result;
    }
    nmd::FwmarkClient fwmarkClient;
    int32_t protectResult = fwmarkClient.ProtectFromVpn(socketFd);
    if (protectResult == NETMANAGER_ERROR) {
        return NETWORKVPN_ERROR_INVALID_FD;
    }
    return protectResult;
}

int32_t NetworkVpnClient::SetUpVpn(sptr<VpnConfig> config, int32_t &tunFd, bool isVpnExtCall)
{
    if (config == nullptr) {
        NETMGR_EXT_LOG_E("SetUpVpn param config is nullptr");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }

    sptr<INetworkVpnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("SetUpVpn proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    NETMGR_EXT_LOG_I("enter SetUpVpn 1, %{public}d", isVpnExtCall);
    int32_t result = proxy->SetUpVpn(config, isVpnExtCall);
    if (result != NETMANAGER_EXT_SUCCESS) {
        tunFd = 0;
        return result;
    }
    clientVpnConfig_.first = config;
    clientVpnConfig_.second = isVpnExtCall;

    tunFd = vpnInterface_.GetVpnInterfaceFd();
    if (tunFd <= 0) {
        return NETMANAGER_EXT_ERR_INTERNAL;
    }

    if (vpnEventCallback_ != nullptr) {
        UnregisterVpnEvent(vpnEventCallback_);
    }
    vpnEventCallback_ = new (std::nothrow) VpnSetUpEventCallback();
    if (vpnEventCallback_ == nullptr) {
        NETMGR_EXT_LOG_E("vpnEventCallback_ is nullptr");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    RegisterVpnEvent(vpnEventCallback_);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnClient::DestroyVpn(bool isVpnExtCall)
{
    vpnInterface_.CloseVpnInterfaceFd();
    if (vpnEventCallback_ != nullptr) {
        UnregisterVpnEvent(vpnEventCallback_);
        vpnEventCallback_ = nullptr;
    }

    sptr<INetworkVpnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("DestroyVpn proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->DestroyVpn(isVpnExtCall);
}

int32_t NetworkVpnClient::RegisterVpnEvent(sptr<IVpnEventCallback> callback)
{
    if (callback == nullptr) {
        NETMGR_EXT_LOG_E("RegisterVpnEvent callback is null.");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    sptr<INetworkVpnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("RegisterVpnEvent proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->RegisterVpnEvent(callback);
}

int32_t NetworkVpnClient::UnregisterVpnEvent(sptr<IVpnEventCallback> callback)
{
    if (callback == nullptr) {
        NETMGR_EXT_LOG_E("UnregisterVpnEvent callback is null.");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    sptr<INetworkVpnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("UnregisterVpnEvent proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->UnregisterVpnEvent(callback);
}

int32_t NetworkVpnClient::CreateVpnConnection(bool isVpnExtCall)
{
    sptr<INetworkVpnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("CreateVpnConnection proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->CreateVpnConnection(isVpnExtCall);
}

int32_t NetworkVpnClient::RegisterBundleName(const std::string &bundleName)
{
    NETMGR_EXT_LOG_D("VpnClient::RegisterBundleName is %{public}s", bundleName.c_str());
    sptr<INetworkVpnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("CreateVpnConnection proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->RegisterBundleName(bundleName);
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

void NetworkVpnClient::RecoverCallback()
{
    uint32_t count = 0;
    while (GetProxy() == nullptr && count < MAX_GET_SERVICE_COUNT) {
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_FOR_SERVICE_TIME_MS));
        count++;
    }
    auto proxy = GetProxy();
    if (proxy != nullptr && clientVpnConfig_.first != nullptr) {
        proxy->SetUpVpn(clientVpnConfig_.first, clientVpnConfig_.second);
    }
    NETMGR_EXT_LOG_D("Get proxy %{public}s, count: %{public}u", proxy == nullptr ? "failed" : "success", count);
    if (proxy != nullptr && vpnEventCallback_ != nullptr) {
        int32_t ret = proxy->RegisterVpnEvent(vpnEventCallback_);
        NETMGR_EXT_LOG_D("Register result %{public}d", ret);
    }
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

    if (vpnEventCallback_ != nullptr) {
        NETMGR_EXT_LOG_D("on remote died recover callback");
        std::thread t([this]() {
            RecoverCallback();
        });
        std::string threadName = "networkvpnRecoverCallback";
        pthread_setname_np(t.native_handle(), threadName.c_str());
        t.detach();
    }
}

void NetworkVpnClient::multiUserSetUpEvent()
{
    vpnInterface_.CloseVpnInterfaceFd();
    if (vpnEventCallback_ != nullptr) {
        UnregisterVpnEvent(vpnEventCallback_);
        vpnEventCallback_ = nullptr;
    }
}
} // namespace NetManagerStandard
} // namespace OHOS
