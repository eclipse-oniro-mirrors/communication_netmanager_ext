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

using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace NetManagerStandard {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, 0xD001800, "NetFirewallProxy" };

int32_t NetFirewallProxy::SetNetFirewallStatus(const int32_t userId, const sptr<NetFirewallStatus> &status)
{
    HiLog::Info(LABEL, "NetFirewallProxy set firewall status");
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
    HiLog::Info(LABEL, "NetFirewallProxy get firewall status");
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
} // namespace NetManagerStandard
} // namespace OHOS
