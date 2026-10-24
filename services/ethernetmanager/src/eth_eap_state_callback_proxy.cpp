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

#include "eth_eap_state_callback_proxy.h"
#include "netmgr_ext_log_wrapper.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {

int32_t EthEapStateCallbackProxy::OnEthEapStateChanged(const EthEapStateInfo &stateInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    // 异步发送：状态通知不等待对端处理返回，避免客户端阻塞拖死服务端状态机
    option.SetFlags(MessageOption::TF_ASYNC);

    if (!data.WriteInterfaceToken(IEthEapStateCallback::GetDescriptor())) {
        NETMGR_EXT_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_EXT_ERR_OPERATION_FAILED;
    }

    if (!stateInfo.Marshalling(data)) {
        NETMGR_EXT_LOG_E("Marshal stateInfo failed");
        return NETMANAGER_EXT_ERR_OPERATION_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is nullptr");
        return NETMANAGER_EXT_ERR_LOCAL_PTR_NULL;
    }

    int32_t ret = remote->SendRequest(ON_ETH_EAP_STATE_CHANGED, data, reply, option);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("SendRequest failed, ret: %{public}d", ret);
    }
    // 发送失败时仅记录日志，仍把错误码返回给调用方（服务端状态机逻辑继续执行）
    return ret;
}

} // namespace NetManagerStandard
} // namespace OHOS
