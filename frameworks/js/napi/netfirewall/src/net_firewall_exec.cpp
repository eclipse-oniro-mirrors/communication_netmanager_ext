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

#include <cstdint>
#include <securec.h>

#include "napi_utils.h"
#include "net_firewall_exec.h"
#include "net_firewall_rule_parse.h"
#include "net_manager_constants.h"
#include "netmanager_ext_log.h"
#include "netfirewall_client.h"
#include "singleton.h"
#include "hi_app_event_report.h"

namespace OHOS {
namespace NetManagerStandard {
namespace NetFirewallExec {
template <typename ContextT> static inline NetFirewallClient *GetNetFirewallInstance(ContextT *context)
{
    if (context == nullptr) {
        return nullptr;
    }
    auto manager = context->GetManager();
    return (manager == nullptr) ? nullptr : reinterpret_cast<NetFirewallClient *>(manager->GetData());
}

bool ExecSetNetFirewallPolicy(SetNetFirewallPolicyContext *context)
{
    HiAppEventReport hiAppEventReport("NetworkKit", "NetFirewallSetNetFirewallPolicy");
    if (context == nullptr || context->status_ == nullptr) {
        return false;
    }
    if (!context->IsParseOK()) {
        return false;
    }
    int32_t result =
        DelayedSingleton<NetFirewallClient>::GetInstance()->SetNetFirewallPolicy(context->userId_, context->status_);
    if (result != FIREWALL_SUCCESS) {
        NETMANAGER_EXT_LOGE("ExecSetIfaceConfig error, errorCode: %{public}d", result);
        context->SetErrorCode(result);
        hiAppEventReport.ReportSdkEvent(RESULT_SUCCESS, result);
        return false;
    }
    hiAppEventReport.ReportSdkEvent(RESULT_SUCCESS, ERR_NONE);
    return true;
}

napi_value SetNetFirewallPolicyCallback(SetNetFirewallPolicyContext *context)
{
    if (context == nullptr) {
        return nullptr;
    }
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ExecGetNetFirewallPolicy(GetNetFirewallPolicyContext *context)
{
    if (context == nullptr) {
        return false;
    }
    if (!context->IsParseOK()) {
        return false;
    }
    int32_t result =
        DelayedSingleton<NetFirewallClient>::GetInstance()->GetNetFirewallPolicy(context->userId_, context->status_);
    if (result != FIREWALL_SUCCESS || context->status_ == nullptr) {
        NETMANAGER_EXT_LOGE("ExecGetIfaceConfig error, errorCode: %{public}d", result);
        context->SetErrorCode(result);
        return false;
    }
    return true;
}

napi_value GetNetFirewallPolicyCallback(GetNetFirewallPolicyContext *context)
{
    if (context == nullptr || context->status_ == nullptr) {
        return nullptr;
    }
    napi_value firewallStatus = NapiUtils::CreateObject(context->GetEnv());

    NapiUtils::SetBooleanProperty(context->GetEnv(), firewallStatus, NET_FIREWALL_IS_OPEN, context->status_->isOpen);
    NapiUtils::SetInt32Property(context->GetEnv(), firewallStatus, NET_FIREWALL_IN_ACTION,
        static_cast<int32_t>(context->status_->inAction));
    NapiUtils::SetInt32Property(context->GetEnv(), firewallStatus, NET_FIREWALL_OUT_ACTION,
        static_cast<int32_t>(context->status_->outAction));
    return firewallStatus;
}

bool ExecAddNetFirewallRule(AddNetFirewallRuleContext *context)
{
    HiAppEventReport hiAppEventReport("NetworkKit", "NetFirewallAddNetFirewallRule");
    if (context == nullptr || context->rule_ == nullptr) {
        return false;
    }
    if (!context->IsParseOK()) {
        return false;
    }
    int32_t result =
        DelayedSingleton<NetFirewallClient>::GetInstance()->AddNetFirewallRule(context->rule_, context->reslut_);
    if (result != FIREWALL_SUCCESS) {
        NETMANAGER_EXT_LOGE("ExecAddNetFirewallRule error, errorCode: %{public}d", result);
        context->SetErrorCode(result);
        hiAppEventReport.ReportSdkEvent(RESULT_SUCCESS, result);
        return false;
    }
    hiAppEventReport.ReportSdkEvent(RESULT_SUCCESS, ERR_NONE);
    return true;
}

napi_value AddNetFirewallRuleCallback(AddNetFirewallRuleContext *context)
{
    if (context == nullptr) {
        return nullptr;
    }
    // return basic data type
    return NapiUtils::CreateInt32(context->GetEnv(), context->reslut_);
}

bool ExecUpdateNetFirewallRule(UpdateNetFirewallRuleContext *context)
{
    HiAppEventReport hiAppEventReport("NetworkKit", "FirewallUpdateNetFirewallRule");
    if (context == nullptr || context->rule_ == nullptr) {
        return false;
    }
    if (!context->IsParseOK()) {
        return false;
    }
    int32_t result = DelayedSingleton<NetFirewallClient>::GetInstance()->UpdateNetFirewallRule(context->rule_);
    if (result != FIREWALL_SUCCESS) {
        NETMANAGER_EXT_LOGE("ExecUpdateNetFirewallRule error, errorCode: %{public}d", result);
        context->SetErrorCode(result);
        hiAppEventReport.ReportSdkEvent(RESULT_SUCCESS, result);
        return false;
    }
    hiAppEventReport.ReportSdkEvent(RESULT_SUCCESS, ERR_NONE);
    return true;
}

napi_value UpdateNetFirewallRuleCallback(UpdateNetFirewallRuleContext *context)
{
    if (context == nullptr) {
        return nullptr;
    }
    // return basic data type
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ExecDeleteNetFirewallRule(DeleteNetFirewallRuleContext *context)
{
    HiAppEventReport hiAppEventReport("NetworkKit", "NetFirewallremoveNetFirewallRule");
    if (context == nullptr) {
        return false;
    }
    if (!context->IsParseOK()) {
        return false;
    }
    int32_t result =
        DelayedSingleton<NetFirewallClient>::GetInstance()->DeleteNetFirewallRule(context->userId_, context->ruleId_);
    if (result != FIREWALL_SUCCESS) {
        NETMANAGER_EXT_LOGE("ExecDeleteNetFirewallRule error, errorCode: %{public}d", result);
        context->SetErrorCode(result);
        hiAppEventReport.ReportSdkEvent(RESULT_SUCCESS, result);
        return false;
    }
    hiAppEventReport.ReportSdkEvent(RESULT_SUCCESS, ERR_NONE);
    return true;
}

napi_value DeleteNetFirewallRuleCallback(DeleteNetFirewallRuleContext *context)
{
    if (context == nullptr) {
        return nullptr;
    }
    // return basic data type
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ExecGetNetFirewallRules(GetNetFirewallRulesContext *context)
{
    if (context == nullptr || context->requestParam_ == nullptr) {
        return false;
    }
    if (!context->IsParseOK()) {
        return false;
    }
    int32_t result = DelayedSingleton<NetFirewallClient>::GetInstance()->GetNetFirewallRules(context->userId_,
        context->requestParam_, context->pageInfo_);
    if (result != FIREWALL_SUCCESS || context->pageInfo_ == nullptr) {
        NETMANAGER_EXT_LOGE("ExecGetNetFirewallRules error, errorCode: %{public}d", result);
        context->SetErrorCode(result);
        return false;
    }
    return true;
}

napi_value GetNetFirewallRulesCallback(GetNetFirewallRulesContext *context)
{
    if (context == nullptr || context->pageInfo_ == nullptr) {
        return nullptr;
    }
    napi_value pageInfo = NapiUtils::CreateObject(context->GetEnv());
    if (pageInfo == nullptr) {
        return nullptr;
    }
    NapiUtils::SetInt32Property(context->GetEnv(), pageInfo, NET_FIREWALL_PAGE, context->pageInfo_->page);
    NapiUtils::SetInt32Property(context->GetEnv(), pageInfo, NET_FIREWALL_PAGE_SIZE, context->pageInfo_->pageSize);
    NapiUtils::SetInt32Property(context->GetEnv(), pageInfo, NET_FIREWALL_TOTAL_PAGE, context->pageInfo_->totalPage);
    napi_value list = NapiUtils::CreateArray(context->GetEnv(), context->pageInfo_->data.size());
    uint32_t index = 0;
    for (const auto &iface : context->pageInfo_->data) {
        napi_value rule = NapiUtils::CreateObject(context->GetEnv());
        NetFirewallRuleParse::SetRuleParams(context->GetEnv(), rule, iface);
        NapiUtils::SetArrayElement(context->GetEnv(), list, index++, rule);
    }
    NapiUtils::SetNamedProperty(context->GetEnv(), pageInfo, NET_FIREWALL_PAGE_DATA, list);
    return pageInfo;
}

bool ExecGetNetFirewallRule(GetNetFirewallRuleContext *context)
{
    if (context == nullptr) {
        return false;
    }
    if (!context->IsParseOK()) {
        return false;
    }
    int32_t result = DelayedSingleton<NetFirewallClient>::GetInstance()->GetNetFirewallRule(context->userId_,
        context->ruleId_, context->rule_);
    if (result != FIREWALL_SUCCESS || context->rule_ == nullptr) {
        NETMANAGER_EXT_LOGE("ExecGetNetFirewallRule error, errorCode: %{public}d", result);
        context->SetErrorCode(result);
        return false;
    }
    return true;
}

napi_value GetNetFirewallRuleCallback(GetNetFirewallRuleContext *context)
{
    if (context == nullptr || context->rule_ == nullptr) {
        return nullptr;
    }
    napi_value rule = NapiUtils::CreateObject(context->GetEnv());
    NetFirewallRuleParse::SetRuleParams(context->GetEnv(), rule, *(context->rule_));
    return rule;
}

bool ExecGetInterceptRecords(GetInterceptRecordsContext *context)
{
    if (context == nullptr || context->requestParam_ == nullptr) {
        return false;
    }
    if (!context->IsParseOK()) {
        return false;
    }
    int32_t result = DelayedSingleton<NetFirewallClient>::GetInstance()->GetInterceptRecords(context->userId_,
        context->requestParam_, context->pageInfo_);
    if (result != FIREWALL_SUCCESS || context->pageInfo_ == nullptr) {
        NETMANAGER_EXT_LOGE("ExecGetNetFirewallRules error, errorCode: %{public}d", result);
        context->SetErrorCode(result);
        return false;
    }
    return true;
}

napi_value GetInterceptRecordCallbacks(GetInterceptRecordsContext *context)
{
    if (context == nullptr) {
        return nullptr;
    }
    napi_value pageInfo = NapiUtils::CreateObject(context->GetEnv());
    if (context->pageInfo_ == nullptr) {
        return nullptr;
    }
    NapiUtils::SetInt32Property(context->GetEnv(), pageInfo, NET_FIREWALL_PAGE, context->pageInfo_->page);
    NapiUtils::SetInt32Property(context->GetEnv(), pageInfo, NET_FIREWALL_PAGE_SIZE, context->pageInfo_->pageSize);
    NapiUtils::SetInt32Property(context->GetEnv(), pageInfo, NET_FIREWALL_TOTAL_PAGE, context->pageInfo_->totalPage);
    napi_value list = NapiUtils::CreateArray(context->GetEnv(), context->pageInfo_->data.size());

    uint32_t index = 0;
    for (const auto &iface : context->pageInfo_->data) {
        napi_value rule = NapiUtils::CreateObject(context->GetEnv());
        NapiUtils::SetInt32Property(context->GetEnv(), rule, NET_FIREWALL_RECORD_TIME, iface.time);
        NapiUtils::SetStringPropertyUtf8(context->GetEnv(), rule, NET_FIREWALL_RECORD_LOCAL_IP, iface.localIp);
        NapiUtils::SetStringPropertyUtf8(context->GetEnv(), rule, NET_FIREWALL_RECORD_REMOTE_IP, iface.remoteIp);
        NapiUtils::SetInt32Property(context->GetEnv(), rule, NET_FIREWALL_RECORD_LOCAL_PORT, iface.localPort);
        NapiUtils::SetInt32Property(context->GetEnv(), rule, NET_FIREWALL_RECORD_REMOTE_PORT, iface.remotePort);
        NapiUtils::SetInt32Property(context->GetEnv(), rule, NET_FIREWALL_RECORD_PROTOCOL, iface.protocol);
        NapiUtils::SetInt32Property(context->GetEnv(), rule, NET_FIREWALL_RECORD_UID, iface.appUid);
        NapiUtils::SetStringPropertyUtf8(context->GetEnv(), rule, NET_FIREWALL_DOMAIN, iface.domain);

        NapiUtils::SetArrayElement(context->GetEnv(), list, index++, rule);
    }
    NapiUtils::SetNamedProperty(context->GetEnv(), pageInfo, NET_FIREWALL_PAGE_DATA, list);
    return pageInfo;
}
} // namespace NetFirewallExec
} // namespace NetManagerStandard
} // namespace OHOS