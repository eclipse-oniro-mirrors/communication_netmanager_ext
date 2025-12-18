/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "netfirewall_intercept_record_callback_proxy.h"
#include "netmgr_ext_log_wrapper.h"
#include "net_manager_constants.h"
#include "iremote_object.h"


namespace OHOS {
namespace NetManagerStandard {
NetFirewallInterceptRecordCallbackProxy::NetFirewallInterceptRecordCallbackProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<INetInterceptRecordCallback>(impl)
{}

int32_t NetFirewallInterceptRecordCallbackProxy::OnInterceptRecord(
    const sptr<NetManagerStandard::InterceptRecord> &record)
{
    NETMGR_EXT_LOG_I("Proxy OnInterceptRecord");
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetFirewallInterceptRecordCallbackProxy::GetDescriptor())) {
        NETMGR_EXT_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!record->Marshalling(data)) {
        return NETMANAGER_EXT_ERR_WRITE_DATA_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    option.SetFlags(MessageOption::TF_ASYNC);
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(ON_INTERCEPT_RECORD), data, reply, option);
    if (ret != FIREWALL_SUCCESS) {
        NETMGR_EXT_LOG_E("Proxy SendRequest failed, ret code:[%{public}d]", ret);
    }
    return ret;
}
} // namespace NetManagerStandard
} // namespace OHOS
