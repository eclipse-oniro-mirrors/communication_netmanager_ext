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

#include "netfirewall_proxy.h"
#include "hilog/log.h"
#include "iremote_object.h"
#include "message_option.h"
#include "message_parcel.h"
#include "net_manager_constants.h"
#include "net_manager_ext_constants.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {

int32_t NetFirewallProxy::SetNetFirewallStatus(const int32_t userId, const sptr<NetFirewallStatus> &status)
{
    NETMGR_EXT_LOG_I("NetFirewallProxy set firewall status");
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        NETMGR_EXT_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    data.WriteInt32(userId);
    if (!status->Marshalling(data)) {
        NETMGR_EXT_LOG_E("proxy Marshalling failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(SET_NET_FIREWALL_STATUS), data, reply, option);
    if (ret != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
    }
    return ret;
}

int32_t NetFirewallProxy::GetNetFirewallStatus(const int32_t userId, sptr<NetFirewallStatus> &status)
{
    NETMGR_EXT_LOG_I("NetFirewallProxy get firewall status");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        NETMGR_EXT_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    data.WriteInt32(userId);
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(GET_NET_FIREWALL_STATUS), data, reply, option);
    if (ret != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("GetNetFirewallStatus proxy SendRequest failed, error code: [%{public}d]", ret);
        return ret;
    }
    status = NetFirewallStatus::Unmarshalling(reply);
    if (status == nullptr) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallProxy::AddNetFirewallRule(const sptr<NetFirewallRule> &rule, int32_t &result)
{
    NETMGR_EXT_LOG_I("AddNetFirewallRule set firewall status");
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        NETMGR_EXT_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!rule->Marshalling(data)) {
        NETMGR_EXT_LOG_E("AddNetFirewallRule proxy Marshalling failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(ADD_NET_FIREWALL_RULE), data, reply, option);
    if (ret != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return ret;
    }
    result = reply.ReadInt32();
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallProxy::UpdateNetFirewallRule(const sptr<NetFirewallRule> &rule)
{
    NETMGR_EXT_LOG_I("UpdateNetFirewallRule set firewall status");
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        NETMGR_EXT_LOG_E("UpdateNetFirewallRule WriteInterfaceToken failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!rule->Marshalling(data)) {
        NETMGR_EXT_LOG_E("UpdateNetFirewallRule proxy Marshalling failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(UPDATE_NET_FIREWALL_RULE), data, reply, option);
    if (ret != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
    }
    return ret;
}

int32_t NetFirewallProxy::DeleteNetFirewallRule(const int32_t userId, const int32_t ruleId)
{
    NETMGR_EXT_LOG_I("DeleteNetFirewallRule set firewall status");
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        NETMGR_EXT_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    data.WriteInt32(userId);
    data.WriteInt32(ruleId);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DELETE_NET_FIREWALL_RULE), data, reply, option);
    if (ret != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
    }
    return ret;
}

int32_t NetFirewallProxy::GetNetFirewallRules(const int32_t userId, const sptr<RequestParam> &requestParam,
    sptr<FirewallRulePage> &info)
{
    NETMGR_EXT_LOG_I("GetNetFirewallRules set firewall status");
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        NETMGR_EXT_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    data.WriteInt32(userId);
    if (!requestParam->Marshalling(data)) {
        NETMGR_EXT_LOG_E("GetNetFirewallRules proxy Marshalling failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(GET_ALL_NET_FIREWALL_RULES), data, reply, option);
    if (ret != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return ret;
    }
    info = FirewallRulePage::Unmarshalling(reply);
    if (info == nullptr) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallProxy::GetNetFirewallRule(const int32_t userId, const int32_t ruleId, sptr<NetFirewallRule> &rule)
{
    NETMGR_EXT_LOG_I("GetNetFirewallRule set firewall status");
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        NETMGR_EXT_LOG_E("GetNetFirewallRule WriteInterfaceToken failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    data.WriteInt32(userId);
    data.WriteInt32(ruleId);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("GetNetFirewallRule Remote is null");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(GET_NET_FIREWALL_RULE), data, reply, option);
    if (ret != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return ret;
    }
    rule = NetFirewallRule::Unmarshalling(reply);
    if (rule == nullptr) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallProxy::GetInterceptRecords(const int32_t userId, const sptr<RequestParam> &requestParam,
    sptr<InterceptRecordPage> &info)
{
    NETMGR_EXT_LOG_I("GetInterceptRecords set firewall status");
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        NETMGR_EXT_LOG_E("GetInterceptRecords WriteInterfaceToken failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    data.WriteInt32(userId);
    if (!requestParam->Marshalling(data)) {
        NETMGR_EXT_LOG_E("GetInterceptRecords proxy Marshalling failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(GET_ALL_INTERCEPT_RECORDS), data, reply, option);
    if (ret != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return ret;
    }
    info = InterceptRecordPage::Unmarshalling(reply);
    if (info == nullptr) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    return FIREWALL_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS
