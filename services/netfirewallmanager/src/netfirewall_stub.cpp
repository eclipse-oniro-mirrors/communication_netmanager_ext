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

#include "netfirewall_stub.h"

#include "errors.h"
#include "hilog/log.h"
#include "ipc_object_stub.h"
#include "ipc_types.h"
#include "message_parcel.h"
#include "netfirewall_hisysevent.h"
#include "netmanager_base_common_utils.h"
#include "netmanager_base_permission.h"
#include "netmgr_ext_log_wrapper.h"
#include <arpa/inet.h>
#include <sys/socket.h>

namespace OHOS {
namespace NetManagerStandard {
NetFirewallStub::NetFirewallStub()
{
    memberFuncMap_[static_cast<uint32_t>(SET_NET_FIREWALL_STATUS)] = {Permission::MANAGE_NET_STRATEGY,
                                                                      &NetFirewallStub::OnSetNetFirewallStatus};
    memberFuncMap_[static_cast<uint32_t>(GET_NET_FIREWALL_STATUS)] = {Permission::MANAGE_NET_STRATEGY,
                                                                      &NetFirewallStub::OnGetNetFirewallStatus};
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

int32_t NetFirewallStub::OnSetNetFirewallStatus(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId;
    if (!data.ReadInt32(userId)) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }

    sptr<NetFirewallStatus> status = NetFirewallStatus::Unmarshalling(data);
    if (status == nullptr) {
        NETMGR_EXT_LOG_E("status is nullptr.");
        return FIREWALL_ERR_INTERNAL;
    }

    return SetNetFirewallStatus(userId, status);
}

int32_t NetFirewallStub::OnGetNetFirewallStatus(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId;
    if (!data.ReadInt32(userId)) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    sptr<NetFirewallStatus> status = new (std::nothrow) NetFirewallStatus();
    int32_t ret = GetNetFirewallStatus(userId, status);
    if (ret == FIREWALL_SUCCESS) {
        if (!status->Marshalling(reply)) {
            return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
        }
    }
    return ret;
}
} // namespace NetManagerStandard
} // namespace OHOS
