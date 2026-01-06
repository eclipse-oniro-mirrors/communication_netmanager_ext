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
#include "iremote_object.h"
#include "message_option.h"
#include "message_parcel.h"
#include "net_manager_constants.h"
#include "net_manager_ext_constants.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {

int32_t NetFirewallProxy::SetNetFirewallPolicy(const int32_t userId, const sptr<NetFirewallPolicy> &status)
{
    NETMGR_EXT_LOG_I("SetNetFirewallPolicy");
    // LCOV_EXCL_START
    if (status == nullptr) {
        NETMGR_EXT_LOG_E("status is null");
        return NETMANAGER_EXT_ERR_LOCAL_PTR_NULL;
    }
    // LCOV_EXCL_STOP
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

int32_t NetFirewallProxy::GetNetFirewallPolicy(const int32_t userId, sptr<NetFirewallPolicy> &status)
{
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
        NETMGR_EXT_LOG_E("GetNetFirewallPolicy proxy SendRequest failed, error code: [%{public}d]", ret);
        return ret;
    }
    status = NetFirewallPolicy::Unmarshalling(reply);
    if (status == nullptr) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallProxy::AddNetFirewallRule(const sptr<NetFirewallRule> &rule, int32_t &result)
{
    NETMGR_EXT_LOG_I("AddNetFirewallRule");
    // LCOV_EXCL_START
    if (rule == nullptr) {
        NETMGR_EXT_LOG_E("rule is null");
        return NETMANAGER_EXT_ERR_LOCAL_PTR_NULL;
    }
    // LCOV_EXCL_STOP
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
    NETMGR_EXT_LOG_I("UpdateNetFirewallRule");
    // LCOV_EXCL_START
    if (rule == nullptr) {
        NETMGR_EXT_LOG_E("rule is null");
        return NETMANAGER_EXT_ERR_LOCAL_PTR_NULL;
    }
    // LCOV_EXCL_STOP
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
    NETMGR_EXT_LOG_I("DeleteNetFirewallRule");
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
    // LCOV_EXCL_START
    if (requestParam == nullptr) {
        NETMGR_EXT_LOG_E("requestParam is null");
        return NETMANAGER_EXT_ERR_LOCAL_PTR_NULL;
    }
    // LCOV_EXCL_STOP
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
    // LCOV_EXCL_START
    if (requestParam == nullptr) {
        NETMGR_EXT_LOG_E("requestParam is null");
        return NETMANAGER_EXT_ERR_LOCAL_PTR_NULL;
    }
    // LCOV_EXCL_STOP
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

int32_t NetFirewallProxy::RegisterInterceptRecordsCallback(const sptr<INetInterceptRecordCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_EXT_LOG_E("The parameter of callback is nullptr");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }

    MessageParcel data;
    // LCOV_EXCL_START
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        NETMGR_EXT_LOG_E("RegisterInterceptRecordsCallback WriteInterfaceToken failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    // LCOV_EXCL_STOP
    data.WriteRemoteObject(callback->AsObject());

    //LCOV_EXCL_START
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    // LCOV_EXCL_STOP
    MessageParcel reply;
    MessageOption option;
    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(REGISTER_INTERCEPT_RECORDS_CALLBACK), data, reply, option);
    if (ret != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return ret;
    }
    if (!reply.ReadInt32(ret)) {
        return NETMANAGER_ERR_READ_REPLY_FAIL;
    }
    return ret;
}

int32_t NetFirewallProxy::UnregisterInterceptRecordsCallback(const sptr<INetInterceptRecordCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_EXT_LOG_E("The parameter of callback is nullptr");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }

    // LCOV_EXCL_START
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        NETMGR_EXT_LOG_E("UnregisterInterceptRecordsCallback WriteInterfaceToken failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    // LCOV_EXCL_STOP
    data.WriteRemoteObject(callback->AsObject());

    sptr<IRemoteObject> remote = Remote();
    // LCOV_EXCL_START
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    // LCOV_EXCL_STOP
    MessageParcel reply;
    MessageOption option;
    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(UNREGISTER_INTERCEPT_RECORDS_CALLBACK), data, reply, option);
    if (ret != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return ret;
    }
    if (!reply.ReadInt32(ret)) {
        return NETMANAGER_ERR_READ_REPLY_FAIL;
    }
    return ret;
}
} // namespace NetManagerStandard
} // namespace OHOS
