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
#include "netfirewall_service.h"
#include "ipc_skeleton.h"

#include "bundle_constants.h"
#include "iremote_object.h"
#include "net_event_report.h"
#include "net_manager_center.h"
#include "net_manager_constants.h"
#include "net_manager_ext_constants.h"
#include "netfirewall_analysis_json.h"
#include "netfirewall_db_helper.h"
#include "netfirewall_hisysevent.h"
#include "netmanager_base_common_utils.h"
#include "netmanager_base_permission.h"
#include "netmanager_hitrace.h"
#include "netmgr_ext_log_wrapper.h"
#include "netsys_controller.h"
#include "system_ability_definition.h"
#include <sys/socket.h>
#include <sys/types.h>

namespace OHOS {
namespace NetManagerStandard {
constexpr uint32_t MAX_REGISTER_EVENT_TIMES = 10;
constexpr uint32_t AGAIN_REGISTER_CALLBACK_INTERVAL_MS = 500;
constexpr int64_t QUERY_USER_ID_DELAY_TIME_MS = 300L;
constexpr int32_t QUERY_USER_MAX_RETRY_TIMES = 100;
constexpr int32_t RECORD_CACHE_SIZE = 100;
constexpr int64_t RECORD_TASK_DELAY_TIME_MS = 3 * 60 * 1000;
constexpr std::string_view PUSH_RESULT_SUCCESS = "Success";
constexpr std::string_view PUSH_RESULT_FAILD = "Faild";
constexpr std::string_view PUSH_RESULT_UNKONW = "Unkonw";
namespace {
const std::string INTERCEPT_RECORD_TASK = "InterceptRecordTask";
} // namespace

const bool REGISTER_LOCAL_RESULT_NETFIREWALL =
    SystemAbility::MakeAndRegisterAbility(&Singleton<NetFirewallService>::GetInstance());

std::shared_ptr<AppExecFwk::EventHandler> NetFirewallService::serviceHandler_;

NetFirewallService::NetFirewallService() : SystemAbility(COMM_FIREWALL_MANAGER_SYS_ABILITY_ID, true)
{
    NETMGR_EXT_LOG_I("NetFirewallService()");
    referencesUtil_ = NetFirewallPreferencesUtil::GetInstance();
}

NetFirewallService::~NetFirewallService()
{
    NETMGR_EXT_LOG_I("~NetFirewallService()");
}

int32_t NetFirewallService::GetCurrentAccountId()
{
    std::vector<int32_t> accountIds;
    auto ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(accountIds);
    if (ret != ERR_OK || accountIds.empty()) {
        NETMGR_EXT_LOG_E("query active user failed errCode=%{public}d", ret);
        return FIREWALL_ERR_INTERNAL;
    }
    currentUserId_ = accountIds.front();
    for (size_t i = 0; i < accountIds.size(); i++) {
        NETMGR_EXT_LOG_I("query active userId=%{public}d", accountIds[i]);
    }
    return currentUserId_;
}

/*
 * Turn on or off the firewall
 *
 * @param userId User id
 * @param status The firewall status to be set
 * @return Error code
 */
int32_t NetFirewallService::SetNetFirewallStatus(const int32_t userId, const sptr<NetFirewallStatus> &status)
{
    NETMGR_EXT_LOG_I("SetNetFirewallStatus isOpen= %{public}d, inAction=%{public}d", status->isOpen, status->inAction);
    if (referencesUtil_ == nullptr) {
        NETMGR_EXT_LOG_E("SetNetFirewallStatus failed, reference is nullptr.");
        return FIREWALL_ERR_INTERNAL;
    }
    referencesUtil_->GetPreference(FIREWALL_PREFERENCE_PATH + std::to_string(userId) + ".xml");
    referencesUtil_->SaveBool("isOpen", status->isOpen);
    referencesUtil_->SaveInt("inAction", static_cast<int>(status->inAction));
    referencesUtil_->SaveInt("outAction", static_cast<int>(status->outAction));

    if (userId == currentUserId_) {
        GetCurrentFirewallState();
        if (firewallStatus_->isOpen) {
            NetsysController::GetInstance().SetFirewallDefaultAction(status->inAction, status->outAction);
        }
        firewallStatus_->inAction = status->inAction;
        firewallStatus_->outAction = status->outAction;
    }

    return FIREWALL_SUCCESS;
}

void NetFirewallService::initFirewallStatusCache()
{
    // If the current user is not obtained, return directly
    if (currentUserId_ == 0) {
        return;
    }
    if (firewallStatus_ != nullptr) {
        firewallStatus_ = nullptr;
    }
    firewallStatus_ = new (std::nothrow) NetFirewallStatus();
    GetStatusFormPreference(currentUserId_, firewallStatus_);
}

bool NetFirewallService::IsNetFirewallOpen(const int32_t userId)
{
    NETMGR_EXT_LOG_I("GetNetFirewallStatus");
    // Current user fetching cache
    if (userId == currentUserId_) {
        if (firewallStatus_ == nullptr) {
            initFirewallStatusCache();
        }
        return firewallStatus_->isOpen;
    }
    referencesUtil_->GetPreference(FIREWALL_PREFERENCE_PATH + std::to_string(userId) + ".xml");
    return referencesUtil_->ObtainBool("isOpen", true);
}

void NetFirewallService::GetStatusFormPreference(const int32_t userId, sptr<NetFirewallStatus> &status)
{
    referencesUtil_->GetPreference(FIREWALL_PREFERENCE_PATH + std::to_string(userId) + ".xml");
    status->isOpen = referencesUtil_->ObtainBool("isOpen", true);
    status->inAction = static_cast<FirewallRuleAction>(
        referencesUtil_->ObtainInt("inAction", static_cast<int>(FirewallRuleAction::RULE_DENY)));
    status->outAction = static_cast<FirewallRuleAction>(
        referencesUtil_->ObtainInt("outAction", static_cast<int>(FirewallRuleAction::RULE_ALLOW)));
}

/*
 * Query firewall status
 *
 * @param userId User id
 * @param Firewall status of user userId
 * @return Error code
 */
int32_t NetFirewallService::GetNetFirewallStatus(const int32_t userId, sptr<NetFirewallStatus> &status)
{
    NETMGR_EXT_LOG_I("GetNetFirewallStatus");
    // Current user fetching cache
    if (userId == currentUserId_) {
        if (firewallStatus_ == nullptr) {
            initFirewallStatusCache();
        }
        status->isOpen = firewallStatus_->isOpen;
        status->inAction = firewallStatus_->inAction;
        status->outAction = firewallStatus_->outAction;
    } else {
        GetStatusFormPreference(userId, status);
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallService::GetAllInterceptRecords(const int32_t userId, const sptr<RequestParam> &requestParam,
    sptr<InterceptRecordPage> &info)
{
    NETMGR_EXT_LOG_I("GetAllInterceptRecords");
    int32_t ret = ChekcUserExits(userId);
    if (ret != FIREWALL_SUCCESS) {
        return ret;
    }
    // Cache data writing to avoid not being able to access new data
    ClearRecordCache(currentUserId_);
    info->pageSize = requestParam->pageSize;
    std::shared_ptr<NetFirewallDbHelper> helper = NetFirewallDbHelper::GetInstance();
    ret = helper->QueryInterceptRecord(userId, requestParam, info);
    if (ret < 0) {
        NETMGR_EXT_LOG_E("GetAllInterceptRecords error");
        return FIREWALL_ERR_INTERNAL;
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallService::ChekcUserExits(const int32_t userId)
{
    AccountSA::OsAccountInfo accountInfo;
    if (AccountSA::OsAccountManager::QueryOsAccountById(userId, accountInfo) != ERR_OK) {
        NETMGR_EXT_LOG_E("QueryOsAccountById error, userId: %{public}d.", userId);
        return FIREWALL_ERR_NO_USER;
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallService::ClearCurrentNetFirewallPreferences(const int32_t userId)
{
    if (referencesUtil_ == nullptr) {
        NETMGR_EXT_LOG_E("ClearCurrentNetFirewallPreferences failed");
        return FIREWALL_ERR_INTERNAL;
    }
    referencesUtil_->Clear(FIREWALL_PREFERENCE_PATH + std::to_string(userId) + ".xml");

    firewallStatus_ = nullptr;
    return FIREWALL_SUCCESS;
}

// dump netfirewall
int32_t NetFirewallService::Dump(int32_t fd, const std::vector<std::u16string> &args)
{
    std::string result;
    GetDumpMessage(result);
    NETMGR_EXT_LOG_I("NetFirewall dump fd: %{public}d, content: %{public}s", fd, result.c_str());
    int32_t ret = dprintf(fd, "%s\n", result.c_str());
    if (ret < 0) {
        NETMGR_EXT_LOG_E("dprintf failed, errno[%{public}d]", errno);
        return FIREWALL_ERR_INTERNAL;
    }
    return FIREWALL_SUCCESS;
}

void NetFirewallService::GetDumpMessage(std::string &message)
{
    std::unique_lock<std::mutex> locker(netfirewallMutex_);
    message.append("NetFirewall Info:\n");
    message.append("\tServiceRunningState: " + GetServiceState() + "\n");
    message.append("\tSpendTimeMSec: " + std::to_string(serviceSpendTime_) + "ms" + "\n");
    std::map<int32_t, bool> firewallStateMap;
    GetAllUserFirewallState(firewallStateMap);
    message.append("\t");
    for (const auto &pair : firewallStateMap) {
        std::string userId = std::to_string(pair.first);
        std::string state = pair.second ? "Enable" : "Disable";
        message.append("UserId: " + userId + " " + state + ", ");
    }
    message.append("\n");
    message.append("\tLastRulePushTime: " + GetLastRulePushTime() + "\n");
    message.append("\tLastRulePushResult: " + GetLastRulePushResult() + "\n");
}

std::string NetFirewallService::GetServiceState()
{
    return ((state_ == ServiceRunningState::STATE_RUNNING) ? +"Running" : "Stop");
}

std::string NetFirewallService::GetLastRulePushTime()
{
    if (currentSetRuleSecond_ == 0) {
        return static_cast<std::string>(PUSH_RESULT_UNKONW);
    }
    return std::to_string(currentSetRuleSecond_);
}

std::string NetFirewallService::GetLastRulePushResult()
{
    std::string result = "";
    if (lastRulePushResult_ == FIREWALL_SUCCESS) {
        result = static_cast<std::string>(PUSH_RESULT_SUCCESS);
    } else if (lastRulePushResult_ < 0) {
        result = static_cast<std::string>(PUSH_RESULT_UNKONW);
    } else {
        result = static_cast<std::string>(PUSH_RESULT_FAILD);
    }
    return result;
}

int32_t NetFirewallService::GetAllUserFirewallState(std::map<int32_t, bool> &firewallStateMap)
{
    std::vector<AccountSA::OsAccountInfo> osAccountInfos;
    AccountSA::OsAccountManager::QueryAllCreatedOsAccounts(osAccountInfos);
    size_t size = osAccountInfos.size();
    for (size_t i = 0; i < size; i++) {
        int32_t userId = osAccountInfos[i].GetLocalId();
        firewallStateMap[userId] = IsNetFirewallOpen(userId);
    }
    return FIREWALL_SUCCESS;
}

bool NetFirewallService::GetCurrentFirewallState()
{
    if (firewallStatus_ == nullptr) {
        initFirewallStatusCache();
    }
    return firewallStatus_->isOpen;
}

void NetFirewallService::OnStart()
{
    NETMGR_EXT_LOG_I("OnStart()");
    uint64_t startServiceTime = GetCurrentMilliseconds();
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        NETMGR_EXT_LOG_I("PcfirewallService is already running.");
        return;
    }

    if (!REGISTER_LOCAL_RESULT_NETFIREWALL) {
        NETMGR_EXT_LOG_E("Register to local sa manager failed");
        return;
    }
    if (!isServicePublished_) {
        if (!Publish(&Singleton<NetFirewallService>::GetInstance())) {
            NETMGR_EXT_LOG_E("Register to sa manager failed");
            return;
        }
        isServicePublished_ = true;
    }

    state_ = ServiceRunningState::STATE_RUNNING;

    NETMGR_EXT_LOG_I("%{public}s called:AddAbilityListener begin!", __func__);
    AddSystemAbilityListener(COMM_NETSYS_NATIVE_SYS_ABILITY_ID);
    AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
    NETMGR_EXT_LOG_I("%{public}s called:AddAbilityListener end!", __func__);
    if (OnInit() != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("Init data failed");
        return;
    }
    uint64_t endServiceTime = GetCurrentMilliseconds();
    serviceSpendTime_ = endServiceTime - startServiceTime;
}

int32_t NetFirewallService::OnInit()
{
    InitServiceHandler();
    InitQueryUserId(QUERY_USER_MAX_RETRY_TIMES);
    SubscribeCommonEvent();
    if (callback_ == nullptr) {
        callback_ = new (std::nothrow) FirewallCallback(*this);
    }
    NetsysController::GetInstance().RegisterNetFirewallCallback(callback_);
    return FIREWALL_SUCCESS;
}

void NetFirewallService::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    NETMGR_EXT_LOG_I("OnAddSystemAbility systemAbilityId:%{public}d added!", systemAbilityId);
    if (systemAbilityId == COMM_NETSYS_NATIVE_SYS_ABILITY_ID) {
        if (hasSaRemoved_) {
            NETMGR_EXT_LOG_I("native reboot, reset firewall rules.");
            int32_t ret = NetsysController::GetInstance().SetFirewallDefaultAction(firewallStatus_->inAction,
                firewallStatus_->outAction);
            if (ret != FIREWALL_SUCCESS) {
                NETMGR_EXT_LOG_E("native reboot, SetFirewallDefaultAction error");
            }
            hasSaRemoved_ = false;
        }
    }
}

bool NetFirewallService::InitUsersOnBoot()
{
    std::vector<int32_t> userIds;
    if (AccountSA::OsAccountManager::QueryActiveOsAccountIds(userIds) != ERR_OK || userIds.empty()) {
        NETMGR_EXT_LOG_E("PcfirewallService: failed to get current userIds");
        return false;
    }
    currentUserId_ = userIds.front();
    NETMGR_EXT_LOG_I("PcfirewallService::get current userIds success, Current userId: %{public}d", currentUserId_);
    return true;
}

void NetFirewallService::InitQueryUserId(int32_t times)
{
    times--;
    bool ret = InitUsersOnBoot();
    if (!ret && times > 0) {
        NETMGR_EXT_LOG_I("InitQueryUserId failed");
        serviceHandler_->PostTask([this, times]() { InitQueryUserId(times); }, QUERY_USER_ID_DELAY_TIME_MS);
    }
}

bool NetFirewallService::InitQueryNetFirewallRules()
{
    if (firewallStatus_ == nullptr) {
        initFirewallStatusCache();
    }
    int32_t ret =
        NetsysController::GetInstance().SetFirewallDefaultAction(firewallStatus_->inAction, firewallStatus_->outAction);
    if (ret != FIREWALL_SUCCESS) {
        return FIREWALL_ERR_INTERNAL;
        NETMGR_EXT_LOG_E("InitQueryNetFirewallRules, SetFirewallDefaultAction error");
    }
    return FIREWALL_SUCCESS;
}

void NetFirewallService::InitServiceHandler()
{
    NETMGR_EXT_LOG_I("InitServiceHandler started.");
    if (serviceHandler_ != nullptr) {
        NETMGR_EXT_LOG_E("InitServiceHandler already init.");
        return;
    }
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create("NetFirewallService");
    serviceHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);

    NETMGR_EXT_LOG_I("InitServiceHandler succeeded.");
}

void NetFirewallService::OnStop()
{
    NETMGR_EXT_LOG_I("OnStop started.");
    if (state_ != ServiceRunningState::STATE_RUNNING) {
        return;
    }
    ClearRecordCache(currentUserId_);
    serviceHandler_ = nullptr;
    if (subscriber_ != nullptr) {
        bool unSubscribeResult = OHOS::EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber_);
        subscriber_ = nullptr;
        NETMGR_EXT_LOG_I("UnregisterSubscriber end, unSubscribeResult = %{public}d", unSubscribeResult);
    }
    if (callback_ != nullptr) {
        NetsysController::GetInstance().UnRegisterNetFirewallCallback(callback_);
        callback_ = nullptr;
    }
    state_ = ServiceRunningState::STATE_NOT_START;
    NETMGR_EXT_LOG_I("OnStop end.");
}

void NetFirewallService::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    NETMGR_EXT_LOG_I("OnRemoveSystemAbility systemAbilityId:%{public}d removed!", systemAbilityId);
    if (systemAbilityId == COMM_NETSYS_NATIVE_SYS_ABILITY_ID) {
        hasSaRemoved_ = true;
    }
}

void NetFirewallService::SubscribeCommonEvent()
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    // 1 means CORE_EVENT_PRIORITY
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<ReceiveMessage>(subscribeInfo, *this);
    uint32_t tryCount = 0;
    bool subscribeResult = false;
    while (!subscribeResult && tryCount <= MAX_REGISTER_EVENT_TIMES) {
        std::this_thread::sleep_for(std::chrono::milliseconds(AGAIN_REGISTER_CALLBACK_INTERVAL_MS));
        subscribeResult = EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_);
        tryCount++;
        NETMGR_EXT_LOG_I("SubscribeCommonEvent try  %{public}d", tryCount);
    }

    if (!subscribeResult) {
        NETMGR_EXT_LOG_E("SubscribeCommonEvent fail: %{public}d", subscribeResult);
    }
}

void NetFirewallService::ReceiveMessage::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    const auto &action = eventData.GetWant().GetAction();
    const auto &data = eventData.GetData();
    const auto &code = eventData.GetCode();
    NETMGR_EXT_LOG_I("NetVReceiveMessage::OnReceiveEvent(), event:[%{public}s], data:[%{public}s], code:[%{public}d]",
        action.c_str(), data.c_str(), code);
    int32_t userId = code;
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED) {
        NETMGR_EXT_LOG_I("NetFirewallService: COMMON_EVENT_USER_REMOVED");
        _netfirewallService.ClearCurrentNetFirewallPreferences(userId);
        NetFirewallDbHelper::GetInstance()->DeleteInterceptRecord(userId);
        if (!_netfirewallService.userRuleSize_.empty() && _netfirewallService.userRuleSize_.count(userId)) {
            _netfirewallService.userRuleSize_.erase(userId);
        }
        return;
    }
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED) {
        NETMGR_EXT_LOG_I("NetFirewallService: COMMON_EVENT_USER_SWITCHED");
        // Old user cache cleaning
        _netfirewallService.ClearRecordCache(_netfirewallService.currentUserId_);
        _netfirewallService.firewallStatus_ = nullptr;
        _netfirewallService.currentUserId_ = userId;
        return;
    }
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED) {
        NETMGR_EXT_LOG_I("NetFirewallService: COMMON_EVENT_PACKAGE_REMOVED");
        if (eventData.GetWant().GetIntParam(AppExecFwk::Constants::UID, 0) < 0) {
            NETMGR_EXT_LOG_E("error:deletedUid < 0!,return");
            return;
        }
        uint32_t deletedUid = static_cast<uint32_t>(eventData.GetWant().GetIntParam(AppExecFwk::Constants::UID, 0));
        NETMGR_EXT_LOG_I("NetFirewallService: deletedUid %{public}d", deletedUid);
    }
}

int32_t NetFirewallService::FirewallCallback::OnIntercept(sptr<InterceptRecord> &record)
{
    netfirewallService_.recordCache_.emplace_back(record);
    serviceHandler_->RemoveTask(INTERCEPT_RECORD_TASK);
    auto callback = [this]() { netfirewallService_.ClearRecordCache(netfirewallService_.currentUserId_); };
    if (netfirewallService_.recordCache_.size() < RECORD_CACHE_SIZE) {
        // Write every three minutes when dissatisfied
        serviceHandler_->PostTask(callback, INTERCEPT_RECORD_TASK, RECORD_TASK_DELAY_TIME_MS);
    } else {
        serviceHandler_->PostImmediateTask(callback);
    }
    return FIREWALL_SUCCESS;
}

void NetFirewallService::ClearRecordCache(const int32_t userId)
{
    if (!recordCache_.empty()) {
        NetFirewallDbHelper::GetInstance()->AddInterceptRecord(userId, recordCache_);
        recordCache_.clear();
    }
}

bool NetFirewallService::CheckAccountExits(int32_t userId)
{
    AccountSA::OsAccountInfo accountInfo;
    if (AccountSA::OsAccountManager::QueryOsAccountById(userId, accountInfo) != ERR_OK) {
        NETMGR_EXT_LOG_E("QueryOsAccountById error, userId: %{public}d.", userId);
        return false;
    }

    if (accountInfo.GetType() == AccountSA::OsAccountType::GUEST) {
        NETMGR_EXT_LOG_W("The guest account.");
    }
    return true;
}
} // namespace NetManagerStandard
} // namespace OHOS
