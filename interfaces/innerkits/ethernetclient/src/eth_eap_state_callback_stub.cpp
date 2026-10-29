/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "eth_eap_state_callback_stub.h"
#include "netmgr_ext_log_wrapper.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {

enum class EthEapStateCallbackIpcCode {
    ON_ETH_EAP_STATE_CHANGED = 0,
};

EthEapStateCallbackStub::EthEapStateCallbackStub()
{
    memberFuncMap_[static_cast<uint32_t>(EthEapStateCallbackIpcCode::ON_ETH_EAP_STATE_CHANGED)] =
        &EthEapStateCallbackStub::OnEthEapStateChangedInner;
}

int32_t EthEapStateCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
                                                  MessageOption &option)
{
    std::u16string descriptor = data.ReadInterfaceToken();
    if (descriptor != IEthEapStateCallback::GetDescriptor()) {
        NETMGR_EXT_LOG_E("Invalid descriptor");
        return NETMANAGER_EXT_ERR_OPERATION_FAILED;
    }
    auto it = memberFuncMap_.find(code);
    if (it == memberFuncMap_.end()) {
        NETMGR_EXT_LOG_E("Unknown code: %{public}u", code);
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return (this->*(it->second))(data, reply);
}

int32_t EthEapStateCallbackStub::OnEthEapStateChangedInner(MessageParcel &data, MessageParcel &reply)
{
    EthEapStateInfo *info = EthEapStateInfo::Unmarshalling(data);
    if (info == nullptr) {
        NETMGR_EXT_LOG_E("OnEthEapStateChangedInner: failed to unmarshal state info");
        return NETMANAGER_EXT_ERR_LOCAL_PTR_NULL;
    }
    int32_t ret = OnEthEapStateChanged(*info);
    delete info;
    return ret;
}

} // namespace NetManagerStandard
} // namespace OHOS
