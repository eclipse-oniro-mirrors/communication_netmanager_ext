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

#include <thread>

#include "iservice_registry.h"
#include "net_manager_constants.h"
#include "netfirewall_client.h"
#include "netmgr_ext_log_wrapper.h"
#include "system_ability_definition.h"
#include "i_net_intercept_record_callback.h"

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

int32_t NetFirewallClient::SetNetFirewallPolicy(const int32_t userId, const sptr<NetFirewallPolicy> &status)
{
    sptr<INetFirewallService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("SetNetFirewallPolicy proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->SetNetFirewallPolicy(userId, status);
}

int32_t NetFirewallClient::GetNetFirewallPolicy(const int32_t userId, sptr<NetFirewallPolicy> &status)
{
    sptr<INetFirewallService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("GetNetFirewallPolicy proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->GetNetFirewallPolicy(userId, status);
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

int32_t NetFirewallClient::GetNetFirewallRules(const int32_t userId, const sptr<RequestParam> &requestParam,
    sptr<FirewallRulePage> &info)
{
    sptr<INetFirewallService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("GetNetFirewallRules proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->GetNetFirewallRules(userId, requestParam, info);
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

int32_t NetFirewallClient::RegisterInterceptRecordsCallback(const sptr<INetInterceptRecordCallback> &callback)
{
    sptr<INetFirewallService> proxy = GetProxy();
    // LCOV_EXCL_START
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("GetAllNetMessageInterceptInfoRecords proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    // LCOV_EXCL_STOP
    return proxy->RegisterInterceptRecordsCallback(callback);
}

int32_t NetFirewallClient::UnregisterInterceptRecordsCallback(const sptr<INetInterceptRecordCallback> &callback)
{
    sptr<INetFirewallService> proxy = GetProxy();
    // LCOV_EXCL_START
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("GetAllNetMessageInterceptInfoRecords proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    // LCOV_EXCL_STOP
    return proxy->UnregisterInterceptRecordsCallback(callback);
}

int32_t NetFirewallClient::GetInterceptRecords(const int32_t userId, const sptr<RequestParam> &requestParam,
    sptr<InterceptRecordPage> &info)
{
    sptr<INetFirewallService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("GetInterceptRecords proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->GetInterceptRecords(userId, requestParam, info);
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
    sptr<OHOS::IRemoteObject> result = sam->GetSystemAbility(COMM_FIREWALL_MANAGER_SYS_ABILITY_ID);
    return result;
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

bool NetFirewallClient::RestartNetFirewallManagerSysAbility()
{
    for (uint32_t i = 0; i < MAX_GET_SERVICE_COUNT; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(WAIT_FOR_SERVICE_TIME_S));
        sptr<INetFirewallService> proxy = GetProxy();
        if (proxy) {
            NETMGR_EXT_LOG_I("Restart NetFirewallManager success.");
            return true;
        }
    }
    NETMGR_EXT_LOG_E("Restart NetFirewallManager failed.");
    return false;
}

int32_t NetFirewallClient::CreateRedirector(uint32_t groupId, uint32_t priority,
    std::string& redirectorId)
{
    sptr<INetFirewallService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("CreateRedirector proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->CreateRedirector(groupId, priority, redirectorId);
}

int32_t NetFirewallClient::DestroyRedirector(const std::string& redirectorId)
{
    sptr<INetFirewallService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("DestroyRedirector proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->DestroyRedirector(redirectorId);
}

int32_t NetFirewallClient::AddRedirectRule(const std::string& redirectorId,
    const sptr<TrafficFilterRedirectRule> &rule)
{
    sptr<INetFirewallService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("AddRedirectRule proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->AddRedirectRule(redirectorId, rule);
}

int32_t NetFirewallClient::ClearRedirectRule(const std::string& redirectorId)
{
    sptr<INetFirewallService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("ClearRedirectRule proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->ClearRedirectRule(redirectorId);
}

int32_t NetFirewallClient::GlobalEnableTrafficFilter()
{
    sptr<INetFirewallService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("GlobalEnableTrafficFilter proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->GlobalEnableTrafficFilter();
}

int32_t NetFirewallClient::GlobalDisableTrafficFilter()
{
    sptr<INetFirewallService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("GlobalDisableTrafficFilter proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->GlobalDisableTrafficFilter();
}

int32_t NetFirewallClient::GetTrafficFilterGlobalStatus(bool& isEnabled)
{
    sptr<INetFirewallService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("GetTrafficFilterGlobalStatus proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->GetTrafficFilterGlobalStatus(isEnabled);
}

int32_t NetFirewallClient::QueryProcess(const std::string& srcIp, uint16_t srcPort,
    const std::string& dstIp, uint16_t dstPort, uint8_t protocol, uint32_t& uid, uint32_t& pid)
{
    sptr<INetFirewallService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("QueryProcess proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    int32_t ret = proxy->QueryProcess(srcIp, srcPort, dstIp, dstPort, protocol, uid, pid);
    if (ret != 0) {
        NETMGR_EXT_LOG_E("QueryProcess: service call failed, ret=%{public}d", ret);
    }
    return ret;
}
} // namespace NetManagerStandard
} // namespace OHOS
