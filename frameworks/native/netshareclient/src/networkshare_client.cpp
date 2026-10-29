/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "networkshare_client.h"

#include <thread>

#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "net_manager_constants.h"
#include "netmanager_base_permission.h"
#include "netmgr_ext_log_wrapper.h"
#include "network_share_service_proxy.h"
#include "networkshare_constants.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr uint32_t WAIT_FOR_SERVICE_TIME_MS = 500;
constexpr uint32_t MAX_GET_SERVICE_COUNT = 10;
} // namespace

NetworkShareClient::NetworkShareClient()
    : networkShareService_(nullptr), deathRecipient_(nullptr)
{
    SubscribeSaStatus();
}

NetworkShareClient::~NetworkShareClient()
{
    isDestroyed_ = true;
    NETMGR_EXT_LOG_I("~NetworkShareClient : Destroy NetworkShareClient");
    UnsubscribeSaStatus();
    std::lock_guard lock(mutex_);
    if (networkShareService_ == nullptr || deathRecipient_ == nullptr) {
        return;
    }
    sptr<IRemoteObject> serviceRemote = networkShareService_->AsObject();
    if (serviceRemote == nullptr) {
        return;
    }

    if (deathRecipient_) {
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
    }
}

void NetworkShareClient::SubscribeSaStatus()
{
    // LCOV_EXCL_START
    if (saStatusChangeListener_ != nullptr) {
        return;
    }
    // LCOV_EXCL_STOP
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    // LCOV_EXCL_START
    if (samgrProxy == nullptr) {
        NETMGR_EXT_LOG_E("SubscribeSaStatus samgrProxy is nullptr");
        return;
    }
    // LCOV_EXCL_STOP
    saStatusChangeListener_ = sptr<NetworkShareSaListener>::MakeSptr(*this);
    samgrProxy->SubscribeSystemAbility(COMM_NET_TETHERING_MANAGER_SYS_ABILITY_ID, saStatusChangeListener_);
}

void NetworkShareClient::UnsubscribeSaStatus()
{
    // LCOV_EXCL_START
    if (saStatusChangeListener_ == nullptr) {
        return;
    }
    // LCOV_EXCL_STOP
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy != nullptr) {
        samgrProxy->UnSubscribeSystemAbility(COMM_NET_TETHERING_MANAGER_SYS_ABILITY_ID, saStatusChangeListener_);
    }
    saStatusChangeListener_ = nullptr;
}

int32_t NetworkShareClient::StartSharing(const SharingIfaceType &type)
{
    NETMGR_EXT_LOG_I("NetworkShare StartSharing type= %{public}d", type);
    sptr<INetworkShareService> proxy = GetProxy(true);
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("StartSharing proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->StartNetworkSharing(static_cast<int32_t>(type));
}

int32_t NetworkShareClient::StopSharing(const SharingIfaceType &type)
{
    NETMGR_EXT_LOG_I("NetworkShare StopSharing type= %{public}d", type);
    sptr<INetworkShareService> proxy = GetProxy(false);
    // LCOV_EXCL_START
    if (proxy == nullptr) {
        return NETMANAGER_EXT_SUCCESS;
    }
    // LCOV_EXCL_STOP
    return proxy->StopNetworkSharing(static_cast<int32_t>(type));
}

int32_t NetworkShareClient::IsSharingSupported(int32_t &supported)
{
    NETMGR_EXT_LOG_I("NetworkShare IsSharingSupported.");
    sptr<INetworkShareService> proxy = GetProxy(true);
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("IsSharingSupported proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->IsNetworkSharingSupported(supported);
}

int32_t NetworkShareClient::IsSharing(int32_t &sharingStatus)
{
    NETMGR_EXT_LOG_I("NetworkShare IsSharing.");
    sptr<INetworkShareService> proxy = GetProxy(false);
    // LCOV_EXCL_START
    if (proxy == nullptr) {
        sharingStatus = NETWORKSHARE_IS_UNSHARING;
        return NETMANAGER_EXT_SUCCESS;
    }
    // LCOV_EXCL_STOP
    return proxy->IsSharing(sharingStatus);
}

int32_t NetworkShareClient::RegisterSharingEvent(sptr<ISharingEventCallback> callback)
{
    NETMGR_EXT_LOG_I("NetworkShare RegisterSharingEvent.");
    if (callback == nullptr) {
        NETMGR_EXT_LOG_E("callback is null");
        return NETMANAGER_EXT_ERR_LOCAL_PTR_NULL;
    }
    std::lock_guard<std::mutex> lock(callbackMutex_);
    for (auto &cb : callbacks_) {
        if (callback->AsObject().GetRefPtr() == cb->AsObject().GetRefPtr()) {
            NETMGR_EXT_LOG_I("RegisterSharingEvent find same callback");
            return NETMANAGER_EXT_SUCCESS;
        }
    }
    callbacks_.push_back(callback);
    sptr<INetworkShareService> proxy = GetProxy(false);
    // LCOV_EXCL_START
    if (proxy == nullptr) {
        return NETMANAGER_EXT_SUCCESS;
    }
    // LCOV_EXCL_STOP
    return proxy->RegisterSharingEvent(callback);
}

int32_t NetworkShareClient::UnregisterSharingEvent(sptr<ISharingEventCallback> callback)
{
    NETMGR_EXT_LOG_I("NetworkShare UnregisterSharingEvent.");
    if (callback == nullptr) {
        NETMGR_EXT_LOG_E("callback is null");
        return NETMANAGER_EXT_ERR_LOCAL_PTR_NULL;
    }
    std::lock_guard<std::mutex> lock(callbackMutex_);
    for (auto iter = callbacks_.begin(); iter != callbacks_.end(); ++iter) {
        if (callback->AsObject().GetRefPtr() == (*iter)->AsObject().GetRefPtr()) {
            callbacks_.erase(iter);
            break;
        }
    }
    sptr<INetworkShareService> proxy = GetProxy(false);
    // LCOV_EXCL_START
    if (proxy == nullptr) {
        return NETMANAGER_EXT_SUCCESS;
    }
    // LCOV_EXCL_STOP
    return proxy->UnregisterSharingEvent(callback);
}

int32_t NetworkShareClient::GetSharableRegexs(const SharingIfaceType &type, std::vector<std::string> &ifaceRegexs)
{
    NETMGR_EXT_LOG_I("NetworkShare GetSharableRegexs type= %{public}d.", type);
    sptr<INetworkShareService> proxy = GetProxy(false);
    // LCOV_EXCL_START
    if (proxy == nullptr) {
        return NETMANAGER_EXT_SUCCESS;
    }
    // LCOV_EXCL_STOP
    return proxy->GetSharableRegexs(static_cast<int32_t>(type), ifaceRegexs);
}

int32_t NetworkShareClient::GetSharingState(const SharingIfaceType &type, SharingIfaceState &state)
{
    NETMGR_EXT_LOG_I("NetworkShare GetSharingState type= %{public}d.", type);
    sptr<INetworkShareService> proxy = GetProxy(false);
    // LCOV_EXCL_START
    if (proxy == nullptr) {
        state = SharingIfaceState::SHARING_NIC_CAN_SERVER;
        return NETMANAGER_EXT_SUCCESS;
    }
    // LCOV_EXCL_STOP
    int32_t stateInt = static_cast<int32_t>(state);
    int32_t ret = proxy->GetSharingState(static_cast<int32_t>(type), stateInt);
    state = SharingIfaceState(stateInt);
    return ret;
}

int32_t NetworkShareClient::GetSharingIfaces(const SharingIfaceState &state, std::vector<std::string> &ifaces)
{
    NETMGR_EXT_LOG_I("NetworkShare GetSharingIfaces type= %{public}d.", state);
    sptr<INetworkShareService> proxy = GetProxy(false);
    // LCOV_EXCL_START
    if (proxy == nullptr) {
        return NETMANAGER_EXT_SUCCESS;
    }
    // LCOV_EXCL_STOP
    return proxy->GetNetSharingIfaces(static_cast<int32_t>(state), ifaces);
}

int32_t NetworkShareClient::GetSharingAddress(const SharingIfaceType &type, std::vector<INetAddr> &addresses)
{
    addresses.clear();
    NETMGR_EXT_LOG_I("NetworkShare GetSharingAddress type= %{public}d.", static_cast<int32_t>(type));
    if (type != SharingIfaceType::SHARING_WIFI && type != SharingIfaceType::SHARING_USB &&
        type != SharingIfaceType::SHARING_BLUETOOTH) {
        NETMGR_EXT_LOG_E("GetSharingAddress invalid type.");
        return NETMANAGER_EXT_ERR_INVALID_PARAMETER;
    }
    sptr<INetworkShareService> proxy = GetProxy(false);
    // LCOV_EXCL_START
    if (proxy == nullptr) {
        if (!NetManagerPermission::CheckPermission(Permission::GET_NETWORK_INFO)) {
            NETMGR_EXT_LOG_E("GetSharingAddress permission denied.");
            return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
        }
        return NETMANAGER_EXT_SUCCESS;
    }
    // LCOV_EXCL_STOP
    sptr<NetworkShareAddressParcel> addressParcel = nullptr;
    int32_t ret = proxy->GetSharingAddress(static_cast<int32_t>(type), addressParcel);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("GetSharingAddress failed, ret[%{public}d].", ret);
        return ret;
    }
    if (addressParcel == nullptr) {
        NETMGR_EXT_LOG_E("GetSharingAddress addressParcel is nullptr");
        return NETMANAGER_EXT_ERR_READ_REPLY_FAIL;
    }
    addresses = addressParcel->addresses_;
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkShareClient::GetStatsRxBytes(int32_t &bytes)
{
    NETMGR_EXT_LOG_I("NetworkShare GetStatsRxBytes.");
    sptr<INetworkShareService> proxy = GetProxy(false);
    // LCOV_EXCL_START
    if (proxy == nullptr) {
        bytes = 0;
        return NETMANAGER_EXT_SUCCESS;
    }
    // LCOV_EXCL_STOP
    return proxy->GetStatsRxBytes(bytes);
}

int32_t NetworkShareClient::GetStatsTxBytes(int32_t &bytes)
{
    NETMGR_EXT_LOG_I("NetworkShare GetStatsTxBytes.");
    sptr<INetworkShareService> proxy = GetProxy(false);
    // LCOV_EXCL_START
    if (proxy == nullptr) {
        bytes = 0;
        return NETMANAGER_EXT_SUCCESS;
    }
    // LCOV_EXCL_STOP
    return proxy->GetStatsTxBytes(bytes);
}

int32_t NetworkShareClient::GetStatsTotalBytes(int32_t &bytes)
{
    NETMGR_EXT_LOG_I("NetworkShare GetStatsTotalBytes.");
    sptr<INetworkShareService> proxy = GetProxy(false);
    // LCOV_EXCL_START
    if (proxy == nullptr) {
        bytes = 0;
        return NETMANAGER_EXT_SUCCESS;
    }
    // LCOV_EXCL_STOP
    return proxy->GetStatsTotalBytes(bytes);
}

int32_t NetworkShareClient::SetConfigureForShare(bool enabled)
{
    NETMGR_EXT_LOG_I("SetConfigureForShare NetworkShare.");
    sptr<INetworkShareService> proxy = GetProxy(true);
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("SetConfigureForShare proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->SetConfigureForShare(enabled);
}

sptr<INetworkShareService> NetworkShareClient::GetProxy(bool triggerLoad)
{
    std::lock_guard locker(mutex_);
    if (networkShareService_ != nullptr) {
        return networkShareService_;
    }
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        NETMGR_EXT_LOG_E("get SystemAbilityManager failed");
        return nullptr;
    }
    sptr<IRemoteObject> remote = triggerLoad
        ? sam->GetSystemAbility(COMM_NET_TETHERING_MANAGER_SYS_ABILITY_ID)
        : sam->CheckSystemAbility(COMM_NET_TETHERING_MANAGER_SYS_ABILITY_ID);
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("get Remote service failed");
        return nullptr;
    }

    deathRecipient_ = new NetshareDeathRecipient(*this);
    if (deathRecipient_ == nullptr) {
        NETMGR_EXT_LOG_E("deathRecipient_ is nullptr");
        return nullptr;
    }
    if (remote->IsProxyObject() && !remote->AddDeathRecipient(deathRecipient_)) {
        NETMGR_EXT_LOG_E("add death recipient failed");
        return nullptr;
    }
    networkShareService_ = iface_cast<INetworkShareService>(remote);
    if (networkShareService_ == nullptr) {
        NETMGR_EXT_LOG_E("get Remote service proxy failed");
        if (remote->IsProxyObject() && deathRecipient_ != nullptr) {
            remote->RemoveDeathRecipient(deathRecipient_);
        }
        deathRecipient_ = nullptr;
        return nullptr;
    }
    return networkShareService_;
}

void NetworkShareClient::RecoverCallback()
{
    uint32_t count = 0;
    while (GetProxy(false) == nullptr && count < MAX_GET_SERVICE_COUNT) {
        // LCOV_EXCL_START
        if (isDestroyed_) {
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_FOR_SERVICE_TIME_MS));
        count++;
        // LCOV_EXCL_STOP
    }
    if (isDestroyed_) {
        return;
    }
    auto proxy = GetProxy(false);
    NETMGR_EXT_LOG_D("Get proxy %{public}s, count: %{public}u", proxy == nullptr ? "failed" : "success", count);
    if (proxy != nullptr) {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        for (auto &callback : callbacks_) {
            proxy->RegisterSharingEvent(callback);
        }
    }
}

void NetworkShareClient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    NETMGR_EXT_LOG_I("NetworkShareClient:OnRemoteDied enter");
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("remote object is nullptr");
        return;
    }
    sptr<IRemoteObject> local = nullptr;
    {
        std::lock_guard lock(mutex_);
        if (networkShareService_ == nullptr) {
            NETMGR_EXT_LOG_E("networkShareService_ is nullptr");
            return;
        }
        local = networkShareService_->AsObject();
        if (local != remote.promote()) {
            NETMGR_EXT_LOG_E("proxy and stub is not same remote object");
            return;
        }
        local->RemoveDeathRecipient(deathRecipient_);
        networkShareService_ = nullptr;
    }

    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (!callbacks_.empty()) {
        NETMGR_EXT_LOG_D("on remote died recover callback");
        std::thread t([this]() {
            RecoverCallback();
        });
        std::string threadName = "networkshareRecoverCallback";
        pthread_setname_np(t.native_handle(), threadName.c_str());
        t.detach();
    }
}

void NetworkShareClient::NetworkShareSaListener::OnAddSystemAbility(
    int32_t systemAbilityId, const std::string &deviceId)
{
    NETMGR_EXT_LOG_I("NetworkShareSaListener::OnAddSystemAbility systemAbilityId[%{public}d]", systemAbilityId);
    if (systemAbilityId != COMM_NET_TETHERING_MANAGER_SYS_ABILITY_ID) {
        return;
    }
    sptr<INetworkShareService> proxy = client_.GetProxy(false);
    // LCOV_EXCL_START
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("NetworkShareSaListener::OnAddSystemAbility get proxy failed");
        return;
    }
    // LCOV_EXCL_STOP
    std::lock_guard<std::mutex> lock(client_.callbackMutex_);
    for (auto &callback : client_.callbacks_) {
        proxy->RegisterSharingEvent(callback);
    }
}

void NetworkShareClient::NetworkShareSaListener::OnRemoveSystemAbility(
    int32_t systemAbilityId, const std::string &deviceId)
{
    NETMGR_EXT_LOG_I("NetworkShareSaListener::OnRemoveSystemAbility systemAbilityId[%{public}d]", systemAbilityId);
    if (systemAbilityId != COMM_NET_TETHERING_MANAGER_SYS_ABILITY_ID) {
        return;
    }
    std::lock_guard locker(client_.mutex_);
    if (client_.networkShareService_ != nullptr) {
        auto remote = client_.networkShareService_->AsObject();
        if (remote != nullptr && client_.deathRecipient_ != nullptr) {
            remote->RemoveDeathRecipient(client_.deathRecipient_);
        }
        client_.networkShareService_ = nullptr;
    }
}
} // namespace NetManagerStandard
} // namespace OHOS
