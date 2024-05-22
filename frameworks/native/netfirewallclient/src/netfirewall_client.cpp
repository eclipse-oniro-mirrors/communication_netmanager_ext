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

#include "netfirewall_client.h"

#include "hilog/log.h"
#include "iservice_registry.h"
#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "system_ability_definition.h"

#include <thread>

using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr size_t WAIT_REMOTE_TIME_SEC = 15;
constexpr uint32_t WAIT_FOR_SERVICE_TIME_S = 1;
constexpr uint32_t MAX_GET_SERVICE_COUNT = 10;
std::condition_variable g_cv;
std::mutex g_mutexCv;
} // namespace

void NetFirewallLoadCallback::OnLoadSystemAbilitySuccess(int32_t systemAbilityId,
    const sptr<IRemoteObject> &remoteObject)
{
    NETMGR_EXT_LOG_D("OnLoadSystemAbilitySuccess systemAbilityId: [%{public}d]", systemAbilityId);
    std::unique_lock<std::mutex> lock(g_mutexCv);
    remoteObject_ = remoteObject;
    g_cv.notify_one();
}

void NetFirewallLoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    NETMGR_EXT_LOG_D("OnLoadSystemAbilityFail: [%{public}d]", systemAbilityId);
    loadSAFailed_ = true;
}

bool NetFirewallLoadCallback::IsFailed()
{
    return loadSAFailed_;
}

const sptr<IRemoteObject> &NetFirewallLoadCallback::GetRemoteObject() const
{
    return remoteObject_;
}


NetFirewallClient &NetFirewallClient::GetInstance()
{
    static NetFirewallClient instance;
    return instance;
}

int32_t NetFirewallClient::SetNetFirewallStatus(const int32_t userId, const sptr<NetFirewallStatus> &status)
{
    sptr<INetFirewallService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("SetNetFirewallStatus proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->SetNetFirewallStatus(userId, status);
}

int32_t NetFirewallClient::GetNetFirewallStatus(const int32_t userId, sptr<NetFirewallStatus> &status)
{
    sptr<INetFirewallService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("GetNetFirewallStatus proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->GetNetFirewallStatus(userId, status);
}

int32_t NetFirewallClient::AddNetFirewallRule(const sptr<NetFirewallRule> &rule, int32_t &result)
{
    sptr<INetFirewallService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("AddNetFirewallRule proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->AddNetFirewallRule(rule, result);
}

int32_t NetFirewallClient::UpdateNetFirewallRule(const sptr<NetFirewallRule> &rule)
{
    sptr<INetFirewallService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("UpdateNetFirewallRule proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->UpdateNetFirewallRule(rule);
}

int32_t NetFirewallClient::DeleteNetFirewallRule(const int32_t userId, const int32_t ruleId)
{
    sptr<INetFirewallService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("DeleteNetFirewallRule proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->DeleteNetFirewallRule(userId, ruleId);
}

int32_t NetFirewallClient::GetAllNetFirewallRules(const int32_t userId, const sptr<RequestParam> &requestParam,
    sptr<FirewallRulePage> &info)
{
    sptr<INetFirewallService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("GetAllNetFirewallRules proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->GetAllNetFirewallRules(userId, requestParam, info);
}

int32_t NetFirewallClient::GetNetFirewallRule(const int32_t userId, const int32_t ruleId, sptr<NetFirewallRule> &rule)
{
    sptr<INetFirewallService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("GetNetFirewallRule proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->GetNetFirewallRule(userId, ruleId, rule);
}

int32_t NetFirewallClient::GetAllInterceptRecords(const int32_t userId, const sptr<RequestParam> &requestParam,
    sptr<InterceptRecordPage> &info)
{
    sptr<INetFirewallService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("GetAllInterceptRecords proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->GetAllInterceptRecords(userId, requestParam, info);
}

sptr<INetFirewallService> NetFirewallClient::GetProxy()
{
    NETMGR_EXT_LOG_I("NetFirewallClient getproxy");
    std::lock_guard lock(mutex_);
    if (netfirewallService_ != nullptr) {
        return netfirewallService_;
    }
    loadCallback_ = new (std::nothrow) NetFirewallLoadCallback;
    if (loadCallback_ == nullptr) {
        NETMGR_EXT_LOG_E("loadCallback_ is nullptr");
        return nullptr;
    }
    sptr<IRemoteObject> remote = LoadSaOnDemand();
    if (remote == nullptr || !remote->IsProxyObject()) {
        NETMGR_EXT_LOG_E("get Remote service failed");
        return nullptr;
    }

    deathRecipient_ = new (std::nothrow) MonitorPcfirewallServiceDead(*this);
    if (deathRecipient_ == nullptr) {
        NETMGR_EXT_LOG_E("deathRecipient_ is nullptr");
        return nullptr;
    }
    if ((remote->IsProxyObject()) && (!remote->AddDeathRecipient(deathRecipient_))) {
        NETMGR_EXT_LOG_E("add death recipient failed");
        return nullptr;
    }
    netfirewallService_ = iface_cast<INetFirewallService>(remote);
    if (netfirewallService_ == nullptr) {
        NETMGR_EXT_LOG_E("get Remote service proxy failed");
        return nullptr;
    }
    return netfirewallService_;
}

sptr<IRemoteObject> NetFirewallClient::LoadSaOnDemand()
{
    NETMGR_EXT_LOG_D("NetFirewallClient OnRemoteDied");
    if (loadCallback_->GetRemoteObject() == nullptr) {
        sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (sam == nullptr) {
            NETMGR_EXT_LOG_E("GetSystemAbilityManager failed");
            return nullptr;
        }
        int32_t result = sam->LoadSystemAbility(COMM_FIREWALL_MANAGER_SYS_ABILITY_ID, loadCallback_);
        if (result != ERR_OK) {
            NETMGR_EXT_LOG_E("LoadSystemAbility failed : [%{public}d]", result);
            return nullptr;
        }
        std::unique_lock<std::mutex> lk(g_mutexCv);
        if (!g_cv.wait_for(lk, std::chrono::seconds(WAIT_REMOTE_TIME_SEC),
            [this]() { return loadCallback_->GetRemoteObject() != nullptr; })) {
            NETMGR_EXT_LOG_E("LoadSystemAbility timeout");
            lk.unlock();
            return nullptr;
        }
        lk.unlock();
    }
    return loadCallback_->GetRemoteObject();
}

void NetFirewallClient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    NETMGR_EXT_LOG_D("NetFirewallClient OnRemoteDied");
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("remote object is nullptr");
        return;
    }
    std::lock_guard lock(mutex_);
    if (netfirewallService_ == nullptr) {
        NETMGR_EXT_LOG_E("netfirewallService_ is nullptr");
        return;
    }
    sptr<IRemoteObject> local = netfirewallService_->AsObject();
    if (local != remote.promote()) {
        NETMGR_EXT_LOG_E("proxy and stub is not same remote object");
        return;
    }
    local->RemoveDeathRecipient(deathRecipient_);
    netfirewallService_ = nullptr;

    std::thread([this]() { this->RestartNetFirewallManagerSysAbility(); }).detach();
}

void NetFirewallClient::RestartNetFirewallManagerSysAbility()
{
    for (uint32_t i = 0; i < MAX_GET_SERVICE_COUNT; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(WAIT_FOR_SERVICE_TIME_S));
        sptr<INetFirewallService> proxy = GetProxy();
        if (proxy) {
            NETMGR_EXT_LOG_I("Restart NetFirewallManager success.");
            return;
        }
    }
    NETMGR_EXT_LOG_E("Restart NetFirewallManager failed.");
}

} // namespace NetManagerStandard
} // namespace OHOS
