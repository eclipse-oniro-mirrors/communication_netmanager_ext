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
#ifdef SUPPORT_SYSVPN
#include <vector>
#endif // SUPPORT_SYSVPN

#include "fwmark_client.h"
#include "iservice_registry.h"
#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "system_ability_definition.h"
#include "network_vpn_service_proxy.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
namespace NetManagerStandard {

static constexpr uint32_t WAIT_FOR_SERVICE_TIME_MS = 500;
static constexpr uint32_t MAX_GET_SERVICE_COUNT = 10;

class NetworkVpnClient::SystemAbilityListener : public SystemAbilityStatusChangeStub {
public:
    SystemAbilityListener() = default;
    ~SystemAbilityListener() = default;
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override {};
};

int32_t VpnSetUpEventCallback::OnVpnMultiUserSetUp()
{
    NETMGR_EXT_LOG_I("vpn multiple user setup event.");
    NetworkVpnClient::GetInstance().multiUserSetUpEvent();
    return NETMANAGER_EXT_SUCCESS;
}

int32_t VpnEventCallbackCollection::OnVpnStateChanged(bool isConnected)
{
    std::shared_lock<std::shared_mutex> lock(vpnEventCbMutex_);
    std::list<sptr<IVpnEventCallback>> tmpList = vpnEventCbList_;
    lock.unlock();
    for (auto iter = tmpList.begin(); iter != tmpList.end(); iter++) {
        (*iter)->OnVpnStateChanged(isConnected);
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t VpnEventCallbackCollection::OnMultiVpnStateChanged(
    bool isConnected, const std::string &bundleName, const std::string &vpnId)
{
    std::shared_lock<std::shared_mutex> lock(vpnEventCbMutex_);
    std::list<sptr<IVpnEventCallback>> tmpList = vpnEventCbList_;
    lock.unlock();
    for (auto iter = tmpList.begin(); iter != tmpList.end(); iter++) {
        (*iter)->OnMultiVpnStateChanged(isConnected, bundleName, vpnId);
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t VpnEventCallbackCollection::OnVpnMultiUserSetUp()
{
    std::shared_lock<std::shared_mutex> lock(vpnEventCbMutex_);
    std::list<sptr<IVpnEventCallback>> tmpList = vpnEventCbList_;
    lock.unlock();
    for (auto iter = tmpList.begin(); iter != tmpList.end(); iter++) {
        (*iter)->OnVpnMultiUserSetUp();
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t VpnEventCallbackCollection::RegisterCallback(sptr<IVpnEventCallback> callback)
{
    std::unique_lock<std::shared_mutex> lock(vpnEventCbMutex_);
    for (auto iter = vpnEventCbList_.begin(); iter != vpnEventCbList_.end(); iter++) {
        if ((*iter)->AsObject().GetRefPtr() == callback->AsObject().GetRefPtr()) {
            return NETMANAGER_EXT_ERR_OPERATION_FAILED;
        }
    }
    vpnEventCbList_.push_back(callback);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t VpnEventCallbackCollection::UnregisterCallback(sptr<IVpnEventCallback> callback)
{
    std::unique_lock<std::shared_mutex> lock(vpnEventCbMutex_);
    for (auto iter = vpnEventCbList_.begin(); iter != vpnEventCbList_.end(); iter++) {
        if ((*iter)->AsObject().GetRefPtr() == callback->AsObject().GetRefPtr()) {
            vpnEventCbList_.erase(iter);
            break;
        }
    }
    return NETMANAGER_EXT_SUCCESS;
}

NetworkVpnClient::NetworkVpnClient()
{
    Subscribe();
}

NetworkVpnClient::~NetworkVpnClient()
{
    Unsubscribe();
    UnregisterVpnEventCbCollection();
#ifdef SUPPORT_SYSVPN
    UnregisterMultiVpnEventCbCollection();
#endif
}

NetworkVpnClient &NetworkVpnClient::GetInstance()
{
    static NetworkVpnClient instance;
    return instance;
}

void NetworkVpnClient::Subscribe()
{
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy != nullptr) {
        saStatusChangeListener_ = sptr<SystemAbilityListener>::MakeSptr();
        samgrProxy->SubscribeSystemAbility(COMM_VPN_MANAGER_SYS_ABILITY_ID, saStatusChangeListener_);
    }
}

void NetworkVpnClient::Unsubscribe()
{
    if (saStatusChangeListener_ != nullptr) {
        auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgrProxy != nullptr) {
            samgrProxy->UnSubscribeSystemAbility(COMM_VPN_MANAGER_SYS_ABILITY_ID, saStatusChangeListener_);
        }
    }
}

void NetworkVpnClient::SystemAbilityListener::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    switch (systemAbilityId) {
        case COMM_VPN_MANAGER_SYS_ABILITY_ID:
            NetworkVpnClient::GetInstance().RegisterVpnEventCbCollection();
#ifdef SUPPORT_SYSVPN
            NetworkVpnClient::GetInstance().RegisterMultiVpnEventCbCollection();
#endif
            break;
        default:
            break;
    }
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
    int32_t result = proxy->SetUpVpn(*config, isVpnExtCall);
    if (result != NETMANAGER_EXT_SUCCESS) {
        tunFd = 0;
        return result;
    }
    clientVpnConfig_.first = config;
    clientVpnConfig_.second = isVpnExtCall;
#ifdef SUPPORT_SYSVPN
    vpnInterface_.SetSupportMultiVpn(!config->vpnId_.empty());
#endif // SUPPORT_SYSVPN
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

#ifdef SUPPORT_SYSVPN
int32_t NetworkVpnClient::GetVpnCertData(const int32_t certType, std::vector<int8_t> &certData)
{
    sptr<INetworkVpnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("GetVpnCertData proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->GetVpnCertData(certType, certData);
}

int32_t NetworkVpnClient::DestroyVpn(const std::string &vpnId)
{
    if (vpnId.empty()) {
        NETMGR_EXT_LOG_E("DestroyVpn vpnId is empty");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
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
    return proxy->DestroyVpn(vpnId);
}

int32_t NetworkVpnClient::SetUpVpn(const sptr<SysVpnConfig> &config, bool isVpnExtCall)
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
    NETMGR_EXT_LOG_I("SetUpVpn id=%{public}s", config->vpnId_.c_str());
    return proxy->SetUpSysVpn(config, isVpnExtCall);
}

int32_t NetworkVpnClient::AddSysVpnConfig(sptr<SysVpnConfig> &config)
{
    if (config == nullptr) {
        NETMGR_EXT_LOG_E("AddSysVpnConfig config is null");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    sptr<INetworkVpnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("AddSysVpnConfig proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->AddSysVpnConfig(config);
}

int32_t NetworkVpnClient::DeleteSysVpnConfig(const std::string &vpnId)
{
    if (vpnId.empty()) {
        NETMGR_EXT_LOG_E("DeleteSysVpnConfig vpnId is empty");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    sptr<INetworkVpnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("DeleteSysVpnConfig proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->DeleteSysVpnConfig(vpnId);
}

int32_t NetworkVpnClient::GetConnectedVpnAppInfo(std::vector<std::string> &bundleNameList)
{
    sptr<INetworkVpnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("GetConnectedVpnAppInfo proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->GetConnectedVpnAppInfo(bundleNameList);
}

int32_t NetworkVpnClient::GetSysVpnConfigList(std::vector<sptr<SysVpnConfig>> &vpnList)
{
    sptr<INetworkVpnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("GetSysVpnConfigList proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->GetSysVpnConfigList(vpnList);
}

int32_t NetworkVpnClient::GetSysVpnConfig(sptr<SysVpnConfig> &config, const std::string &vpnId)
{
    if (vpnId.empty()) {
        NETMGR_EXT_LOG_E("DeleteSysVpnConfig vpnId is empty");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    sptr<INetworkVpnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("GetSysVpnConfig proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->GetSysVpnConfig(config, vpnId);
}

int32_t NetworkVpnClient::GetConnectedSysVpnConfig(sptr<SysVpnConfig> &config)
{
    sptr<INetworkVpnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("GetConnectedSysVpnConfig proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->GetConnectedSysVpnConfig(config);
}

int32_t NetworkVpnClient::NotifyConnectStage(const std::string &stage, const int32_t &result)
{
    sptr<INetworkVpnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("NotifyConnectStage proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->NotifyConnectStage(stage, result);
}

int32_t NetworkVpnClient::GetSysVpnCertUri(const int32_t certType, std::string &certUri)
{
    sptr<INetworkVpnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("GetSysVpnCertUri proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->GetSysVpnCertUri(certType, certUri);
}

int32_t NetworkVpnClient::RegisterMultiVpnEvent(sptr<IVpnEventCallback> callback)
{
    if (callback == nullptr) {
        NETMGR_EXT_LOG_E("RegisterMultiVpnEvent callback is null.");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    if (multiVpnEventCbCollection_ == nullptr) {
        multiVpnEventCbCollection_ = sptr<VpnEventCallbackCollection>::MakeSptr();
    }
    return multiVpnEventCbCollection_->RegisterCallback(callback);
}

int32_t NetworkVpnClient::UnregisterMultiVpnEvent(sptr<IVpnEventCallback> callback)
{
    if (callback == nullptr) {
        NETMGR_EXT_LOG_E("UnregisterMultiVpnEvent callback is null.");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    if (multiVpnEventCbCollection_ != nullptr) {
        return multiVpnEventCbCollection_->UnregisterCallback(callback);
    }
    return NETMANAGER_EXT_SUCCESS;
}

void NetworkVpnClient::RegisterMultiVpnEventCbCollection()
{
    if (multiVpnEventCbCollection_ == nullptr) {
        multiVpnEventCbCollection_ = sptr<VpnEventCallbackCollection>::MakeSptr();
    }
    sptr<INetworkVpnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("RegisterMultiVpnEventCbCollection proxy is nullptr");
        return;
    }
    proxy->RegisterMultiVpnEvent(multiVpnEventCbCollection_);
}

void NetworkVpnClient::UnregisterMultiVpnEventCbCollection()
{
    if (multiVpnEventCbCollection_ != nullptr) {
        sptr<INetworkVpnService> proxy = GetProxy();
        if (proxy == nullptr) {
            NETMGR_EXT_LOG_E("UnregisterMultiVpnEventCbCollection proxy is nullptr");
            return;
        }
        proxy->UnregisterMultiVpnEvent(multiVpnEventCbCollection_);
    }
}
#endif // SUPPORT_SYSVPN

int32_t NetworkVpnClient::RegisterVpnEvent(sptr<IVpnEventCallback> callback)
{
    if (callback == nullptr) {
        NETMGR_EXT_LOG_E("RegisterVpnEvent callback is null.");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    if (vpnEventCbCollection_ == nullptr) {
        vpnEventCbCollection_ = sptr<VpnEventCallbackCollection>::MakeSptr();
    }
    return vpnEventCbCollection_->RegisterCallback(callback);
}

int32_t NetworkVpnClient::UnregisterVpnEvent(sptr<IVpnEventCallback> callback)
{
    if (callback == nullptr) {
        NETMGR_EXT_LOG_E("UnregisterVpnEvent callback is null.");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    if (vpnEventCbCollection_ != nullptr) {
        return vpnEventCbCollection_->UnregisterCallback(callback);
    }
    return NETMANAGER_EXT_SUCCESS;
}

void NetworkVpnClient::RegisterVpnEventCbCollection()
{
    if (vpnEventCbCollection_ == nullptr) {
        vpnEventCbCollection_ = sptr<VpnEventCallbackCollection>::MakeSptr();
    }
    sptr<INetworkVpnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("RegisterVpnEventCbCollection proxy is nullptr");
        return;
    }
    proxy->RegisterVpnEvent(vpnEventCbCollection_);
}

void NetworkVpnClient::UnregisterVpnEventCbCollection()
{
    if (vpnEventCbCollection_ != nullptr) {
        sptr<INetworkVpnService> proxy = GetProxy();
        if (proxy == nullptr) {
            NETMGR_EXT_LOG_E("UnregisterVpnEventCbCollection proxy is nullptr");
            return;
        }
        proxy->UnregisterVpnEvent(vpnEventCbCollection_);
    }
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

int32_t NetworkVpnClient::RegisterBundleName(const std::string &bundleName, const std::string &abilityName)
{
    NETMGR_EXT_LOG_D("VpnClient::RegisterBundleName is %{public}s", bundleName.c_str());
    sptr<INetworkVpnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("CreateVpnConnection proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->RegisterBundleName(bundleName, abilityName);
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
    sptr<IRemoteObject> remote = sam->GetSystemAbility(COMM_VPN_MANAGER_SYS_ABILITY_ID);
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
        proxy->SetUpVpn(*clientVpnConfig_.first, clientVpnConfig_.second);
    }
    NETMGR_EXT_LOG_D("Get proxy %{public}s, count: %{public}u", proxy == nullptr ? "failed" : "success", count);
}

void NetworkVpnClient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("remote object is nullptr");
        return;
    }
    {
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

int32_t NetworkVpnClient::GetSelfAppName(std::string &selfAppName, std::string &selfBundleName)
{
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("GetSelfAppName proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->GetSelfAppName(selfAppName, selfBundleName);
}

int32_t NetworkVpnClient::SetSelfVpnPid()
{
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("SetSelfVpnPid proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->SetSelfVpnPid();
}
} // namespace NetManagerStandard
} // namespace OHOS
