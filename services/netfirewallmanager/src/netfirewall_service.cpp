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

#include <sys/socket.h>
#include <sys/types.h>

#include "netfirewall_service.h"
#include "ipc_skeleton.h"
#include "bundle_constants.h"
#include "iremote_object.h"
#include "net_event_report.h"
#include "net_manager_constants.h"
#include "net_manager_ext_constants.h"
#include "netfirewall_default_rule_parser.h"
#include "netfirewall_db_helper.h"
#include "netfirewall_hisysevent.h"
#include "netmanager_base_common_utils.h"
#include "netmanager_base_permission.h"
#include "netmanager_hitrace.h"
#include "netmgr_ext_log_wrapper.h"
#include "system_ability_definition.h"
#include "netsys_controller.h"
#include "netfirewall_intercept_recorder.h"
#include "bundle_mgr_interface.h"
#include "iservice_registry.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int64_t QUERY_USER_ID_DELAY_TIME_MS = 300000L;
constexpr int32_t QUERY_USER_MAX_RETRY_TIMES = 100;
constexpr int64_t SET_POLICY_DELAY_TIME_MS = 500000L;

namespace {
const std::string PUSH_RESULT_SUCCESS = "Success";
const std::string PUSH_RESULT_FAILD = "Faild";
const std::string PUSH_RESULT_UNKONW = "Unkonw";
} // namespace

const bool REGISTER_LOCAL_RESULT_NETFIREWALL =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<NetFirewallService>::GetInstance().get());

std::shared_ptr<ffrt::queue> NetFirewallService::ffrtServiceHandler_;

NetFirewallService::NetFirewallService() : SystemAbility(COMM_FIREWALL_MANAGER_SYS_ABILITY_ID, true)
{
    NETMGR_EXT_LOG_I("NetFirewallService()");
}

NetFirewallService::~NetFirewallService()
{
    NETMGR_EXT_LOG_I("~NetFirewallService()");
}

void NetFirewallService::SetCurrentUserId(int32_t userId)
{
    currentUserId_ = userId;
    NetFirewallInterceptRecorder::GetInstance()->SetCurrentUserId(currentUserId_);
    // set current userid to native
    NetFirewallRuleNativeHelper::GetInstance().SetCurrentUserId(currentUserId_);
}

int32_t NetFirewallService::GetCurrentAccountId()
{
    std::vector<int32_t> accountIds;
    auto ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(accountIds);
    if (ret != ERR_OK || accountIds.empty()) {
        NETMGR_EXT_LOG_E("query active user failed errCode=%{public}d", ret);
        return FIREWALL_ERR_INTERNAL;
    }
    SetCurrentUserId(accountIds.front());
    return currentUserId_;
}

bool NetFirewallService::IsSameNetFirewallPolicy(const sptr<NetFirewallPolicy> &inPolicy,
    const sptr<NetFirewallPolicy> &outPolicy)
{
    return (inPolicy->isOpen == outPolicy->isOpen &&
        inPolicy->inAction == outPolicy->inAction &&
        inPolicy->outAction == outPolicy->outAction);
}

/**
 * Turn on or off the firewall
 *
 * @param userId User id
 * @param policy The firewall status to be set
 * @return Returns 0 success. Otherwise fail
 */
int32_t NetFirewallService::SetNetFirewallPolicy(const int32_t userId, const sptr<NetFirewallPolicy> &policy)
{
    // LCOV_EXCL_START
    if (policy == nullptr) {
        NETMGR_EXT_LOG_E("policy is null");
        return FIREWALL_ERR_INTERNAL;
    }
    // LCOV_EXCL_STOP
    NETMGR_EXT_LOG_I("SetNetFirewallPolicy userId=%{public}d isOpen=%{public}d, in=%{public}d, out=%{public}d",
        userId, policy->isOpen, policy->inAction, policy->outAction);
    int32_t ret = CheckUserExist(userId);
    if (ret != FIREWALL_SUCCESS) {
        return ret;
    }

    sptr<NetFirewallPolicy> policyTemp = sptr<NetFirewallPolicy>::MakeSptr();
    NetFirewallPolicyManager::GetInstance().LoadPolicyFormPreference(userId, policyTemp);
    if (IsSameNetFirewallPolicy(policy, policyTemp)) {
        NETMGR_EXT_LOG_W("SetNetFirewallPolicy is same userId=%{public}d, ignore.", userId);
        return FIREWALL_SUCCESS;
    }

    ret = NetFirewallPolicyManager::GetInstance().SetNetFirewallPolicy(userId, policy);
    if (ret != FIREWALL_SUCCESS) {
        return ret;
    }

    // save default action to bpf
    if (policy->isOpen) {
        NetsysController::GetInstance().SetFirewallDefaultAction(userId, policy->inAction, policy->outAction);
    } else {
        NetFirewallPolicyManager::GetInstance().InitNetfirewallPolicy();
    }

    // update rules
    if (policyTemp->isOpen != policy->isOpen) {
        NetFirewallRuleManager::GetInstance().OpenOrCloseNativeFirewall(
            NetFirewallPolicyManager::GetInstance().IsFirewallOpen());
    }
    return ret;
}

/**
 * Query firewall status
 *
 * @param userId User id
 * @param status status of user userId
 * @return Returns 0 success. Otherwise fail
 */
int32_t NetFirewallService::GetNetFirewallPolicy(const int32_t userId, sptr<NetFirewallPolicy> &policy)
{
    NETMGR_EXT_LOG_I("GetNetFirewallPolicy");
    int32_t ret = CheckUserExist(userId);
    if (ret != FIREWALL_SUCCESS) {
        return ret;
    }
    NetFirewallPolicyManager::GetInstance().GetNetFirewallPolicy(userId, policy);
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallService::AddNetFirewallRule(const sptr<NetFirewallRule> &rule, int32_t &ruleId)
{
    return NetFirewallRuleManager::GetInstance().AddNetFirewallRule(rule, ruleId);
}

int32_t NetFirewallService::AddDefaultNetFirewallRule(int32_t userId)
{
    return NetFirewallRuleManager::GetInstance().AddDefaultNetFirewallRule(userId);
}

int32_t NetFirewallService::UpdateNetFirewallRule(const sptr<NetFirewallRule> &rule)
{
    return NetFirewallRuleManager::GetInstance().UpdateNetFirewallRule(rule);
}

int32_t NetFirewallService::DeleteNetFirewallRule(const int32_t userId, const int32_t ruleId)
{
    return NetFirewallRuleManager::GetInstance().DeleteNetFirewallRule(userId, ruleId);
}

int32_t NetFirewallService::GetNetFirewallRules(const int32_t userId, const sptr<RequestParam> &requestParam,
    sptr<FirewallRulePage> &info)
{
    return NetFirewallRuleManager::GetInstance().GetNetFirewallRules(userId, requestParam, info);
}

int32_t NetFirewallService::GetNetFirewallRule(const int32_t userId, const int32_t ruleId, sptr<NetFirewallRule> &rule)
{
    return NetFirewallRuleManager::GetInstance().GetNetFirewallRule(userId, ruleId, rule);
}

int32_t NetFirewallService::GetInterceptRecords(const int32_t userId, const sptr<RequestParam> &requestParam,
    sptr<InterceptRecordPage> &info)
{
    NETMGR_EXT_LOG_I("GetInterceptRecords");
    int32_t ret = CheckUserExist(userId);
    if (ret != FIREWALL_SUCCESS) {
        return ret;
    }
    // Cache data writing to avoid not being able to access new data
    NetFirewallInterceptRecorder::GetInstance()->SyncRecordCache();
    return NetFirewallInterceptRecorder::GetInstance()->GetInterceptRecords(userId, requestParam, info);
}

int32_t NetFirewallService::RegisterInterceptRecordsCallback(const sptr<INetInterceptRecordCallback> &callback)
{
    NETMGR_EXT_LOG_I("RegisterInterceptRecordsCallback");
    return NetFirewallInterceptRecorder::GetInstance()->RegisterInterceptRecordsCallback(callback);
}

int32_t NetFirewallService::UnregisterInterceptRecordsCallback(const sptr<INetInterceptRecordCallback> &callback)
{
    NETMGR_EXT_LOG_I("UnregisterInterceptRecordsCallback");
    return NetFirewallInterceptRecorder::GetInstance()->UnregisterInterceptRecordsCallback(callback);
}

int32_t NetFirewallService::CheckUserExist(const int32_t userId)
{
    AccountSA::OsAccountInfo accountInfo;
    if (AccountSA::OsAccountManager::QueryOsAccountById(userId, accountInfo) != ERR_OK) {
        NETMGR_EXT_LOG_E("QueryOsAccountById error, userId: %{public}d.", userId);
        return FIREWALL_ERR_NO_USER;
    }
    return FIREWALL_SUCCESS;
}

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
    return (state_ == ServiceRunningState::STATE_RUNNING) ? "Running" : "Stop";
}

std::string NetFirewallService::GetLastRulePushTime()
{
    currentSetRuleSecond_ = NetFirewallRuleManager::GetInstance().GetCurrentSetRuleSecond();
    if (currentSetRuleSecond_ == 0) {
        return PUSH_RESULT_UNKONW;
    }
    return std::to_string(currentSetRuleSecond_);
}

std::string NetFirewallService::GetLastRulePushResult()
{
    lastRulePushResult_ = NetFirewallRuleManager::GetInstance().GetLastRulePushResult();
    if (lastRulePushResult_ == FIREWALL_SUCCESS) {
        return PUSH_RESULT_SUCCESS;
    }
    if (lastRulePushResult_ < 0) {
        return PUSH_RESULT_UNKONW;
    }
    return PUSH_RESULT_FAILD;
}

int32_t NetFirewallService::GetAllUserFirewallState(std::map<int32_t, bool> &firewallStateMap)
{
    std::vector<AccountSA::OsAccountInfo> osAccountInfos;
    AccountSA::OsAccountManager::QueryAllCreatedOsAccounts(osAccountInfos);
    size_t size = osAccountInfos.size();
    for (const auto &info : osAccountInfos) {
        int32_t userId = info.GetLocalId();
        firewallStateMap[userId] = NetFirewallPolicyManager::GetInstance().IsNetFirewallOpen(userId);
    }
    return FIREWALL_SUCCESS;
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
        if (!Publish(DelayedSingleton<NetFirewallService>::GetInstance().get())) {
            NETMGR_EXT_LOG_E("Register to sa manager failed");
            return;
        }
        isServicePublished_ = true;
    }

    state_ = ServiceRunningState::STATE_RUNNING;

    AddSystemAbilityListener(COMM_NETSYS_NATIVE_SYS_ABILITY_ID);
    AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
    if (OnInit() != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("Init data failed");
        return;
    }
    serviceSpendTime_ = GetCurrentMilliseconds() - startServiceTime;
}

int32_t NetFirewallService::OnInit()
{
    InitServiceHandler();
    InitQueryUserId(QUERY_USER_MAX_RETRY_TIMES);
    SubscribeCommonEvent();
    NetFirewallInterceptRecorder::GetInstance()->RegisterInterceptCallback();
    return FIREWALL_SUCCESS;
}

void NetFirewallService::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    NETMGR_EXT_LOG_I("OnAddSystemAbility systemAbilityId:%{public}d added!", systemAbilityId);
    if (systemAbilityId == COMM_NETSYS_NATIVE_SYS_ABILITY_ID) {
        if (hasSaRemoved_) {
            NETMGR_EXT_LOG_I("native reboot, reset firewall rules.");
            NetFirewallRuleManager::GetInstance().OpenOrCloseNativeFirewall(
                NetFirewallPolicyManager::GetInstance().IsFirewallOpen());
            NetFirewallInterceptRecorder::GetInstance()->RegisterInterceptCallback();
            hasSaRemoved_ = false;
        }
        // After the universal service is launched, you can register for broadcast monitoring
    } else if (systemAbilityId == COMMON_EVENT_SERVICE_ID && subscriber_ == nullptr) {
        SubscribeCommonEvent();
    }
}

bool NetFirewallService::InitUsersOnBoot()
{
    std::vector<int32_t> userIds;
    if (AccountSA::OsAccountManager::QueryActiveOsAccountIds(userIds) != ERR_OK || userIds.empty()) {
        NETMGR_EXT_LOG_E("PcfirewallService: failed to get current userIds");
        return false;
    }
    SetCurrentUserId(userIds.front());
    NETMGR_EXT_LOG_I("PcfirewallService::get current userIds success, Current userId: %{public}d",
        currentUserId_.load());
    InitQueryNetFirewallRules();
    return true;
}

void NetFirewallService::InitQueryUserId(int32_t times)
{
    times--;
    bool ret = InitUsersOnBoot();
    if (!ret && times > 0) {
        NETMGR_EXT_LOG_I("InitQueryUserId failed");
        ffrtServiceHandler_->submit([this, times]() { InitQueryUserId(times); },
            ffrt::task_attr().delay(QUERY_USER_ID_DELAY_TIME_MS).name("InitQueryUserId"));
    }
}

void NetFirewallService::InitQueryNetFirewallRules()
{
    NetFirewallRuleManager::GetInstance().OpenOrCloseNativeFirewall(
        NetFirewallPolicyManager::GetInstance().IsFirewallOpen());
}

void NetFirewallService::InitServiceHandler()
{
    if (ffrtServiceHandler_ != nullptr) {
        NETMGR_EXT_LOG_E("InitServiceHandler already init.");
        return;
    }
    ffrtServiceHandler_ =
        std::make_shared<ffrt::queue>("NetFirewallService", ffrt::queue_attr().qos(ffrt::qos_utility));
    NETMGR_EXT_LOG_I("InitServiceHandler succeeded.");
}

void NetFirewallService::OnStop()
{
    if (state_ != ServiceRunningState::STATE_RUNNING) {
        return;
    }
    NetFirewallInterceptRecorder::GetInstance()->SyncRecordCache();
    ffrtServiceHandler_.reset();
    ffrtServiceHandler_ = nullptr;
    if (subscriber_ != nullptr) {
        bool unSubscribeResult = OHOS::EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber_);
        subscriber_ = nullptr;
        NETMGR_EXT_LOG_I("UnregisterSubscriber end, unSubscribeResult = %{public}d", unSubscribeResult);
    }
    NetFirewallInterceptRecorder::GetInstance()->UnRegisterInterceptCallback();
    state_ = ServiceRunningState::STATE_NOT_START;
    NETMGR_EXT_LOG_I("OnStop end.");
}

void NetFirewallService::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    NETMGR_EXT_LOG_I("OnRemoveSystemAbility systemAbilityId:%{public}d removed!", systemAbilityId);
    if (systemAbilityId == COMM_NETSYS_NATIVE_SYS_ABILITY_ID) {
        hasSaRemoved_ = true;
    } else if (systemAbilityId == COMMON_EVENT_SERVICE_ID && subscriber_ != nullptr) {
        OHOS::EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber_);
        subscriber_ = nullptr;
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
    subscriber_ = std::make_shared<ReceiveMessage>(subscribeInfo, shared_from_this());
    RegisterSubscribeCommonEvent();
}

void NetFirewallService::RegisterSubscribeCommonEvent()
{
    // If the universal service has not been loaded yet, registering for broadcasting will fail
    if (subscriber_ == nullptr) {
        NETMGR_EXT_LOG_E("subscriber is nullptr");
        return;
    }
    if (!EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_)) {
        NETMGR_EXT_LOG_E("SubscribeCommonEvent fail");
        subscriber_ = nullptr;
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
        NetFirewallRuleManager::GetInstance().DeleteNetFirewallRuleByUserId(userId);
        NetFirewallPolicyManager::GetInstance().ClearFirewallPolicy(userId);
        NetFirewallDbHelper::GetInstance().DeleteInterceptRecord(userId);
        NetFirewallRuleManager::GetInstance().DeleteUserRuleSize(userId);
        return;
    }
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED) {
        // Old user cache cleaning
        NetFirewallInterceptRecorder::GetInstance()->SyncRecordCache();
        netfirewallService_->SetCurrentUserId(userId);
        return;
    }
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED) {
        if (eventData.GetWant().GetIntParam(AppExecFwk::Constants::UID, 0) < 0) {
            NETMGR_EXT_LOG_E("error:deletedUid < 0!,return");
            return;
        }
        uint32_t deletedUid = static_cast<uint32_t>(eventData.GetWant().GetIntParam(AppExecFwk::Constants::UID, 0));
        NETMGR_EXT_LOG_I("NetFirewallService: deletedUid %{public}d", deletedUid);
        NetFirewallRuleManager::GetInstance().DeleteNetFirewallRuleByAppId(deletedUid);
    }
}

int32_t NetFirewallService::CreateRedirector(uint32_t groupId, uint32_t priority,
    std::string& redirectorId)
{
    std::string bundleName = GetBundleName();
    NETMGR_EXT_LOG_I("CreateRedirector:groupId=%{public}u, priority=%{public}u, bundleName=%{public}s",
        groupId, priority, bundleName.c_str());

    int32_t ret = NetTrafficFilterRedirectManager::GetInstance().CreateRedirector(
        bundleName, groupId, priority, redirectorId);
    if (ret != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("CreateRedirector failed, ret: %{public}d", ret);
    } else {
        NETMGR_EXT_LOG_I("Redirector created successfully, redirectorId: %{public}s", redirectorId.c_str());
    }
    return ret;
}

int32_t NetFirewallService::DestroyRedirector(const std::string& redirectorId)
{
    NETMGR_EXT_LOG_I("DestroyRedirector: redirectorId=%{public}s", redirectorId.c_str());
    int32_t ret = NetTrafficFilterRedirectManager::GetInstance().DestroyRedirector(redirectorId);
    if (ret != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("DestroyRedirector failed, ret: %{public}d", ret);
    } else {
        NETMGR_EXT_LOG_I("Redirector destroyed successfully");
    }
    return ret;
}

int32_t NetFirewallService::AddRedirectRule(const std::string& redirectorId,
    const sptr<TrafficFilterRedirectRule> &rule)
{
    NETMGR_EXT_LOG_I("AddRedirectRule: redirectorId=%{public}s", redirectorId.c_str());
    if (redirectorId.empty()) {
        NETMGR_EXT_LOG_E("AddRedirectRule failed: redirectorId is empty");
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    if (rule == nullptr) {
        NETMGR_EXT_LOG_E("AddRedirectRule failed: rule is null");
        return TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    TrafficFilterRedirectRule nativeRule = *rule;
    int32_t ret = NetTrafficFilterRedirectManager::GetInstance().AddRedirectRule(redirectorId, &nativeRule);
    if (ret != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("AddRedirectRule failed, ret: %{public}d", ret);
    } else {
        NETMGR_EXT_LOG_I("Redirect rule added successfully");
    }
    return ret;
}

int32_t NetFirewallService::ClearRedirectRule(const std::string& redirectorId)
{
    NETMGR_EXT_LOG_I("ClearRedirectRule: redirectorId=%{public}s", redirectorId.c_str());
    int32_t ret = NetTrafficFilterRedirectManager::GetInstance().ClearRedirectRule(redirectorId);
    if (ret != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("ClearRedirectRule failed, ret: %{public}d", ret);
    } else {
        NETMGR_EXT_LOG_I("Redirect rules cleared successfully");
    }
    return ret;
}

int32_t NetFirewallService::GlobalEnableTrafficFilter()
{
    NETMGR_EXT_LOG_I("GlobalEnableTrafficFilter");
    int32_t ret = NetTrafficFilterRedirectManager::GetInstance().GlobalEnableTrafficFilter();
    if (ret != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("GlobalEnableTrafficFilter failed, ret: %{public}d", ret);
    } else {
        NETMGR_EXT_LOG_I("Traffic filter globally enabled successfully");
    }
    return ret;
}

int32_t NetFirewallService::GlobalDisableTrafficFilter()
{
    NETMGR_EXT_LOG_I("GlobalDisableTrafficFilter");
    int32_t ret = NetTrafficFilterRedirectManager::GetInstance().GlobalDisableTrafficFilter();
    if (ret != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("GlobalDisableTrafficFilter failed, ret: %{public}d", ret);
    } else {
        NETMGR_EXT_LOG_I("Traffic filter globally disabled successfully");
    }
    return ret;
}

int32_t NetFirewallService::GetTrafficFilterGlobalStatus(bool& isEnabled)
{
    NETMGR_EXT_LOG_I("GetTrafficFilterGlobalStatus");
    int32_t ret = NetTrafficFilterRedirectManager::GetInstance().GetTrafficFilterGlobalStatus(isEnabled);
    if (ret != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("GetTrafficFilterGlobalStatus failed, ret: %{public}d", ret);
    } else {
        NETMGR_EXT_LOG_I("Got traffic filter global status: %{public}d", isEnabled);
    }
    return ret;
}

int32_t NetFirewallService::QueryProcess(const std::string& srcIp, uint16_t srcPort,
    const std::string& dstIp, uint16_t dstPort, uint8_t protocol, uint32_t& uid, uint32_t& pid)
{
    int32_t ret = NetTrafficFilterRedirectManager::GetInstance().QueryProcess(
        srcIp, srcPort, dstIp, dstPort, protocol, uid, pid);
    if (ret != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("QueryProcess failed, ret: %{public}d", ret);
    } else {
        NETMGR_EXT_LOG_I("QueryProcess success: uid=%{public}u, pid=%{public}u", uid, pid);
    }
    return ret;
}

std::string NetFirewallService::GetBundleName()
{
    std::string bundleName;
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        NETMGR_EXT_LOG_E("Get ability manager failed");
        return bundleName;
    }
    sptr<IRemoteObject> object = samgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (object == nullptr) {
        NETMGR_EXT_LOG_E("object is NULL.");
        return bundleName;
    }
    sptr<OHOS::AppExecFwk::IBundleMgr> bms = iface_cast<OHOS::AppExecFwk::IBundleMgr>(object);
    if (bms == nullptr) {
        NETMGR_EXT_LOG_E("bundle manager service is NULL.");
        return bundleName;
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    auto result = bms->GetNameForUid(uid, bundleName);
    if (result != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("Error GetBundleNameForUid fail");
        return bundleName;
    }
    NETMGR_EXT_LOG_I("bundle name is [%{public}s], uid = [%{public}d]", bundleName.c_str(), uid);
    return bundleName;
}
} // namespace NetManagerStandard
} // namespace OHOS
