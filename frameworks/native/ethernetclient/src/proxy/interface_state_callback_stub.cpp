/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "interface_state_callback_stub.h"

#include "string_ex.h"

#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
int32_t InterfaceStateCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
                                                    MessageOption &option)
{
    std::u16string descriptor = data.ReadInterfaceToken();
    if (descriptor != InterfaceStateCallback::GetDescriptor()) {
        NETMGR_EXT_LOG_E("OnRemoteRequest get descriptor error.");
        return NETMANAGER_EXT_ERR_DESCRIPTOR_MISMATCH;
    }
    InterfaceStateCallback::Message msgCode = static_cast<InterfaceStateCallback::Message>(code);
    switch (msgCode) {
        case InterfaceStateCallback::Message::INTERFACE_STATE_ADD: {
            OnInterfaceAdded(data.ReadString());
            break;
        }
        case InterfaceStateCallback::Message::INTERFACE_STATE_REMOVE: {
            OnInterfaceRemoved(data.ReadString());
            break;
        }
        case InterfaceStateCallback::Message::INTERFACE_STATE_CHANGE: {
            OnInterfaceChanged(data.ReadString(), data.ReadBool());
            break;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return NETMANAGER_EXT_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS
