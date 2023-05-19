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

#include "networkvpn_service_proxy.h"

#include "ipc_types.h"

#include "net_manager_constants.h"
#include "net_manager_ext_constants.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetworkVpnServiceProxy::NetworkVpnServiceProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<INetworkVpnService>(impl)
{
}

int32_t NetworkVpnServiceProxy::SendRequest(INetworkVpnService::MessageCode code, MessageParcel &data,
                                            MessageParcel &reply)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageOption option(MessageOption::TF_SYNC);
    return remote->SendRequest(static_cast<uint32_t>(code), data, reply, option);
}

int32_t NetworkVpnServiceProxy::Prepare(bool &isExistVpn, bool &isRun, std::string &pkg)
{
    return 0;
}

int32_t NetworkVpnServiceProxy::SetUp(const sptr<VpnConfig> &config)
{
    return 0;
}

int32_t NetworkVpnServiceProxy::RegisterVpnEvent(sptr<IVpnEventCallback> callback)
{
    return 0;
}

int32_t NetworkVpnServiceProxy::UnregisterVpnEvent(sptr<IVpnEventCallback> callback)
{
    return 0;
}

int32_t NetworkVpnServiceProxy::DestroyVpn()
{
    return 0;
}
} // namespace NetManagerStandard
} // namespace OHOS
