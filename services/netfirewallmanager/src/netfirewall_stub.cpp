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

#include <arpa/inet.h>
#include <sys/socket.h>

#include "errors.h"
#include "hilog/log.h"
#include "ipc_object_stub.h"
#include "ipc_types.h"
#include "message_parcel.h"
#include "netfirewall_hisysevent.h"
#include "netmanager_base_common_utils.h"
#include "netmanager_base_permission.h"
#include "netmgr_ext_log_wrapper.h"
#include "netfirewall_stub.h"

namespace OHOS {
namespace NetManagerStandard {
NetFirewallStub::NetFirewallStub()
{
    memberFuncMap_[static_cast<uint32_t>(SET_NET_FIREWALL_STATUS)] = {Permission::MANAGE_NET_STRATEGY,
                                                                      &NetFirewallStub::OnSetNetFirewallPolicy};
    memberFuncMap_[static_cast<uint32_t>(GET_NET_FIREWALL_STATUS)] = {Permission::MANAGE_NET_STRATEGY,
                                                                      &NetFirewallStub::OnGetNetFirewallPolicy};
    memberFuncMap_[static_cast<uint32_t>(ADD_NET_FIREWALL_RULE)] = {Permission::MANAGE_NET_STRATEGY,
                                                                    &NetFirewallStub::OnAddNetFirewallRule};
    memberFuncMap_[static_cast<uint32_t>(UPDATE_NET_FIREWALL_RULE)] = {Permission::MANAGE_NET_STRATEGY,
                                                                       &NetFirewallStub::OnUpdateNetFirewallRule};
    memberFuncMap_[static_cast<uint32_t>(DELETE_NET_FIREWALL_RULE)] = {Permission::MANAGE_NET_STRATEGY,
                                                                       &NetFirewallStub::OnDeleteNetFirewallRule};
    memberFuncMap_[static_cast<uint32_t>(GET_ALL_NET_FIREWALL_RULES)] = {Permission::MANAGE_NET_STRATEGY,
                                                                        &NetFirewallStub::OnGetNetFirewallRules};
    memberFuncMap_[static_cast<uint32_t>(GET_NET_FIREWALL_RULE)] = {Permission::MANAGE_NET_STRATEGY,
                                                                    &NetFirewallStub::OnGetNetFirewallRule};
    memberFuncMap_[static_cast<uint32_t>(GET_ALL_INTERCEPT_RECORDS)] = {Permission::MANAGE_NET_STRATEGY,
                                                                       &NetFirewallStub::OnGetInterceptRecords};
}

int32_t NetFirewallStub::CheckFirewallPermission(std::string &strPermission)
{
    if (!strPermission.empty() && !NetManagerPermission::CheckPermission(strPermission)) {
        NETMGR_EXT_LOG_E("Permission denied permission: %{public}s", strPermission.c_str());
        return FIREWALL_ERR_PERMISSION_DENIED;
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    std::u16string myDescripter = NetFirewallStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (myDescripter != remoteDescripter) {
        NETMGR_EXT_LOG_E("descriptor checked fail");
        return NETMANAGER_EXT_ERR_DESCRIPTOR_MISMATCH;
    }
    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        NETMGR_EXT_LOG_I("enter OnRemoteRequest code %{public}d:", code);
        int32_t checkResult = CheckFirewallPermission(itFunc->second.strPermission);
        if (checkResult != FIREWALL_SUCCESS) {
            return checkResult;
        }
        auto serviceFunc = itFunc->second.serviceFunc;
        if (serviceFunc != nullptr) {
            return (this->*serviceFunc)(data, reply);
        }
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t NetFirewallStub::OnSetNetFirewallPolicy(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId;
    if (!data.ReadInt32(userId)) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }

    sptr<NetFirewallPolicy> status = NetFirewallPolicy::Unmarshalling(data);
    if (status == nullptr) {
        NETMGR_EXT_LOG_E("status is nullptr.");
        return FIREWALL_ERR_INTERNAL;
    }

    return SetNetFirewallPolicy(userId, status);
}

int32_t NetFirewallStub::OnGetNetFirewallPolicy(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId;
    if (!data.ReadInt32(userId)) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    sptr<NetFirewallPolicy> status = new (std::nothrow) NetFirewallPolicy();
    int32_t ret = GetNetFirewallPolicy(userId, status);
    if (ret == FIREWALL_SUCCESS) {
        if (!status->Marshalling(reply)) {
            return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
        }
    }
    return ret;
}

int32_t NetFirewallStub::OnAddNetFirewallRule(MessageParcel &data, MessageParcel &reply)
{
    sptr<NetFirewallRule> rule = NetFirewallRule::Unmarshalling(data);
    if (rule == nullptr) {
        NETMGR_EXT_LOG_E("rule is nullptr.");
        return FIREWALL_ERR_INTERNAL;
    }

    if (rule->localIps.size() > MAX_RULE_IP_COUNT || rule->remoteIps.size() > MAX_RULE_IP_COUNT) {
        NETMGR_EXT_LOG_E("ip invalid, size is too long.");
        return FIREWALL_ERR_EXCEED_MAX_IP;
    }

    if (rule->localPorts.size() > MAX_RULE_PORT_COUNT || rule->remotePorts.size() > MAX_RULE_PORT_COUNT) {
        NETMGR_EXT_LOG_E("port invalid, size is too long.");
        return FIREWALL_ERR_EXCEED_MAX_PORT;
    }

    if (rule->domains.size() > MAX_RULE_DOMAIN_COUNT) {
        NETMGR_EXT_LOG_E("domain invalid, size is too long.");
        return FIREWALL_ERR_EXCEED_MAX_DOMAIN;
    }

    int32_t result = 0;
    int32_t ret = AddNetFirewallRule(rule, result);
    if (ret == FIREWALL_SUCCESS) {
        if (!reply.WriteUint32(result)) {
            ret = NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
        }
    }
    NetFirewallHisysEvent::SendFirewallConfigReport(rule, ret);
    return ret;
}

int32_t NetFirewallStub::OnUpdateNetFirewallRule(MessageParcel &data, MessageParcel &reply)
{
    sptr<NetFirewallRule> rule = NetFirewallRule::Unmarshalling(data);
    if (rule == nullptr) {
        NETMGR_EXT_LOG_E("rule is nullptr.");
        return FIREWALL_ERR_INTERNAL;
    }

    if (rule->localIps.size() > MAX_RULE_IP_COUNT || rule->remoteIps.size() > MAX_RULE_IP_COUNT) {
        NETMGR_EXT_LOG_E("ip invalid, size is too long.");
        return FIREWALL_ERR_EXCEED_MAX_IP;
    }

    if (rule->localPorts.size() > MAX_RULE_PORT_COUNT || rule->remotePorts.size() > MAX_RULE_PORT_COUNT) {
        NETMGR_EXT_LOG_E("port invalid, size is too long.");
        return FIREWALL_ERR_EXCEED_MAX_PORT;
    }

    if (rule->domains.size() > MAX_RULE_DOMAIN_COUNT) {
        NETMGR_EXT_LOG_E("domain invalid, size is too long.");
        return FIREWALL_ERR_EXCEED_MAX_DOMAIN;
    }

    int32_t ret = UpdateNetFirewallRule(rule);
    NetFirewallHisysEvent::SendFirewallConfigReport(rule, ret);
    return ret;
}

int32_t NetFirewallStub::OnDeleteNetFirewallRule(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId;
    if (!data.ReadInt32(userId)) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    if (userId <= 0) {
        NETMGR_EXT_LOG_E("Parameter error.");
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    int32_t ruleId;
    if (!data.ReadInt32(ruleId)) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    if (ruleId <= 0) {
        NETMGR_EXT_LOG_E("Parameter error.");
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    int32_t ret = DeleteNetFirewallRule(userId, ruleId);
    NetFirewallHisysEvent::SendFirewallRequestReport(userId, "ruleId=" + std::to_string(ruleId), ret);
    return ret;
}

int32_t NetFirewallStub::OnGetNetFirewallRules(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId;
    if (!data.ReadInt32(userId)) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    if (userId <= 0) {
        NETMGR_EXT_LOG_E("Parameter error.");
        return FIREWALL_ERR_INVALID_PARAMETER;
    }
    sptr<RequestParam> param = RequestParam::Unmarshalling(data);
    if (param == nullptr) {
        NETMGR_EXT_LOG_E("param is nullptr.");
        return FIREWALL_ERR_INTERNAL;
    }
    sptr<FirewallRulePage> info = new (std::nothrow) FirewallRulePage();
    int32_t ret = GetNetFirewallRules(userId, param, info);
    if (ret == FIREWALL_SUCCESS) {
        if (!info->Marshalling(reply)) {
            return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
        }
    }
    NetFirewallHisysEvent::SendFirewallRequestReport(userId, param->ToString(), ret);
    return ret;
}

int32_t NetFirewallStub::OnGetNetFirewallRule(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId;
    if (!data.ReadInt32(userId)) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    int32_t ruleId;
    if (!data.ReadInt32(ruleId)) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }

    sptr<NetFirewallRule> rule = new (std::nothrow) NetFirewallRule();
    int32_t ret = GetNetFirewallRule(userId, ruleId, rule);
    if (ret == FIREWALL_SUCCESS) {
        if (!rule->Marshalling(reply)) {
            return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
        }
    }
    NetFirewallHisysEvent::SendFirewallRequestReport(userId, "ruleId=" + std::to_string(ruleId), ret);
    return ret;
}

int32_t NetFirewallStub::OnGetInterceptRecords(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId;
    if (!data.ReadInt32(userId)) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }

    sptr<RequestParam> param = RequestParam::Unmarshalling(data);
    if (param == nullptr) {
        NETMGR_EXT_LOG_E("param is nullptr.");
        return FIREWALL_ERR_INTERNAL;
    }
    sptr<InterceptRecordPage> info = new (std::nothrow) InterceptRecordPage();
    int32_t ret = GetInterceptRecords(userId, param, info);
    if (ret == FIREWALL_SUCCESS) {
        if (!info->Marshalling(reply)) {
            return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
        }
    }
    NetFirewallHisysEvent::SendRecordRequestReport(userId, ret);
    return ret;
}
} // namespace NetManagerStandard
} // namespace OHOS
