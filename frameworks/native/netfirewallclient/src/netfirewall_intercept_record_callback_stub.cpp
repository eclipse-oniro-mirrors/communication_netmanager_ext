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

#include "netfirewall_intercept_record_callback_stub.h"
#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "i_net_intercept_record_callback.h"

namespace OHOS {
namespace NetManagerStandard {
NetFirewallInterceptRecordCallbackStub::NetFirewallInterceptRecordCallbackStub()
{
    memberFuncMap_[static_cast<uint32_t>(ON_INTERCEPT_RECORD)] =
        &NetFirewallInterceptRecordCallbackStub::CmdOnInterceptRecord;
}

int32_t NetFirewallInterceptRecordCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel &data,
                                                                MessageParcel &reply, MessageOption &option)
{
    NETMGR_EXT_LOG_I("Stub call start, code:[%{public}d]", code);
    std::u16string myDescriptor = NetFirewallInterceptRecordCallbackStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (myDescriptor != remoteDescriptor) {
        NETMGR_EXT_LOG_E("Descriptor checked failed");
        return NETMANAGER_EXT_ERR_DESCRIPTOR_MISMATCH;
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto requestFunc = itFunc->second;
        if (requestFunc != nullptr) {
            return (this->*requestFunc)(data, reply);
        }
    }

    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t NetFirewallInterceptRecordCallbackStub::CmdOnInterceptRecord(MessageParcel &data, MessageParcel &reply)
{
    sptr<NetManagerStandard::InterceptRecord> record = NetManagerStandard::InterceptRecord::Unmarshalling(data);
    int32_t result = OnInterceptRecord(record);
    if (!reply.WriteInt32(result)) {
        return NETMANAGER_EXT_ERR_WRITE_DATA_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS
