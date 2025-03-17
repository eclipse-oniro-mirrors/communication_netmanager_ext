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
#include <condition_variable>
#include <mutex>
#include <thread>
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "netmgr_ext_log_wrapper.h"
#include "net_manager_constants.h"
#include "wearable_distributed_net_client.h"

namespace OHOS {
namespace NetManagerStandard {
static std::condition_variable g_cv;
static constexpr uint32_t WAIT_REMOTE_TIME_SEC = 10;
static constexpr uint32_t GET_SERVICE_MAX_TIMES = 5;
static constexpr uint32_t WAIT_FOR_SERVICE_TIME_SEC = 10;

void WearableDistributedNetLoadCallback::OnLoadSystemAbilitySuccess(int32_t systemAbilityId,
                                                                    const sptr<IRemoteObject> &remoteObject)
{
    NETMGR_EXT_LOG_I("Loading system ability succeeded");
    std::unique_lock<std::mutex> lock(loadMutex_);
    remoteObject_ = remoteObject;
    g_cv.notify_one();
}

void WearableDistributedNetLoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    NETMGR_EXT_LOG_I("Loading system ability failed");
    loadSAFailed_ = true;
}

bool WearableDistributedNetLoadCallback::IsFailed()
{
    return loadSAFailed_;
}

const sptr<IRemoteObject> &WearableDistributedNetLoadCallback::GetRemoteObject() const
{
    return remoteObject_;
}

int32_t WearableDistributedNetClient::SetupWearableDistributedNet(const int32_t tcpPortId, const int32_t udpPortId,
                                                                  const bool isMetered)
{
    NETMGR_EXT_LOG_I("set up for WearableDistributedNet, isMetered:%{public}s", isMetered ? "true" : "false");
    sptr<IWearableDistributedNet> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("SetupWearableDistributedNet fail, proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->SetupWearableDistributedNet(tcpPortId, udpPortId, isMetered);
}

int32_t WearableDistributedNetClient::EnableWearableDistributedNet(bool enableFlag)
{
    NETMGR_EXT_LOG_I("enable WearableDistributedNet : %{public}u", enableFlag);
    sptr<IWearableDistributedNet> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("SetupWearableDistributedNet fail, proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->EnableWearableDistributedNet(enableFlag);
}

int32_t WearableDistributedNetClient::TearDownWearableDistributedNet()
{
    NETMGR_EXT_LOG_I("Wearable Distributed Net Client tear down");
    sptr<IWearableDistributedNet> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("Tear down WearableDistributedNet fail, proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->TearDownWearableDistributedNet();
}

int32_t WearableDistributedNetClient::UpdateWearableDistributedNetMeteredStatus(const bool isMetered)
{
    NETMGR_EXT_LOG_I("update wearable distributed net metered status:%{public}s", isMetered ? "true" : "false");
    sptr<IWearableDistributedNet> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("UpdateWearableDistributedNetMeteredStatus fail, proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->UpdateMeteredStatus(isMetered);
}

void WearableDistributedNetClient::RestartWearableDistributedNetManagerSysAbility()
{
    for (uint32_t retryCount = 0; retryCount < GET_SERVICE_MAX_TIMES; ++retryCount) {
        sptr<IWearableDistributedNet> proxy = GetProxy();
        std::this_thread::sleep_for(std::chrono::seconds(WAIT_FOR_SERVICE_TIME_SEC));
        if (proxy) {
            NETMGR_EXT_LOG_I("Restart WearableDistributedNetManager success");
            return;
        }
    }
    NETMGR_EXT_LOG_E("Restart WearableDistributedNetManager failed after %{public}u attempts", GET_SERVICE_MAX_TIMES);
}

void WearableDistributedNetClient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    NETMGR_EXT_LOG_I("WearableDistributedNetClient OnRemoteDied");
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("remote object is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (wearableDistributedNetService_ == nullptr) {
        NETMGR_EXT_LOG_E("wearableDistributedNetService_ is nullptr");
        return;
    }
    sptr<IRemoteObject> local = wearableDistributedNetService_->AsObject();
    if (local != remote.promote()) {
        NETMGR_EXT_LOG_E("proxy and stub is not same remote object");
        return;
    }
    local->RemoveDeathRecipient(deathRecipient_);
    wearableDistributedNetService_= nullptr;

    std::thread([this]() { this->RestartWearableDistributedNetManagerSysAbility(); }).detach();
}

sptr<IWearableDistributedNet> WearableDistributedNetClient::GetProxy()
{
    NETMGR_EXT_LOG_I("WearableDistributedNetClient GetProxy start");
    std::lock_guard<std::mutex> lock(mutex_);
    if (wearableDistributedNetService_) {
        NETMGR_EXT_LOG_D("get proxy is ok");
        return wearableDistributedNetService_;
    }
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        NETMGR_EXT_LOG_E("WearableDistributedNetClient get SystemAbilityManager failed: sam is null");
        return nullptr;
    }
    sptr<WearableDistributedNetLoadCallback> callback = new (std::nothrow) WearableDistributedNetLoadCallback;
    int32_t result = sam->LoadSystemAbility(COMM_WEARABLE_DISTRIBUTED_NET_ABILITY_ID, callback);
    if (result != ERR_OK) {
        NETMGR_EXT_LOG_E("LoadSystemAbility failed : [%{public}d]", result);
        return nullptr;
    }
    {
        std::unique_lock<std::mutex> uniqueLock(loadSaMutex_);
        g_cv.wait_for(uniqueLock, std::chrono::seconds(WAIT_REMOTE_TIME_SEC),
            [&callback]() { return callback->GetRemoteObject() != nullptr || callback->IsFailed(); });
    }

    auto remote = callback->GetRemoteObject();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("get Remote service failed");
        return nullptr;
    }
    deathRecipient_ = new (std::nothrow) WearableDistributedNetDeathRecipient(*this);
    if (deathRecipient_ == nullptr) {
        NETMGR_EXT_LOG_E("deathRecipient_ is nullptr");
        return nullptr;
    }
    if ((remote->IsProxyObject()) && (!remote->AddDeathRecipient(deathRecipient_))) {
        NETMGR_EXT_LOG_E("add death recipient failed");
        return nullptr;
    }
    wearableDistributedNetService_ = iface_cast<IWearableDistributedNet>(remote);
    if (wearableDistributedNetService_ == nullptr) {
        NETMGR_EXT_LOG_E("get Remote service proxy failed");
        return nullptr;
    }
    NETMGR_EXT_LOG_I("WearableDistributedNetClient GetProxy finish");
    return wearableDistributedNetService_;
}

WearableDistributedNetClient::~WearableDistributedNetClient()
{
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        return;
    }

    auto remote = proxy->AsObject();
    if (remote == nullptr) {
        return;
    }

    remote->RemoveDeathRecipient(deathRecipient_);
}
} // namespace NetManagerStandard
} // namespace OHOS
