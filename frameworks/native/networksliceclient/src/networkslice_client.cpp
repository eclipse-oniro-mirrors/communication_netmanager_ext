/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include <thread>

#include "hilog/log.h"
#include "iservice_registry.h"
#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "system_ability_definition.h"
#include "networkslice_client.h"

using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace NetManagerStandard {
constexpr int SLICE_SA_ID = 8301;
NetworkSliceClient::NetworkSliceClient() : networksliceService_(nullptr), deathRecipient_(nullptr) {}

NetworkSliceClient::~NetworkSliceClient()
{
    DlCloseRemoveDeathRecipient();
}

int32_t NetworkSliceClient::SetNetworkSliceUePolicy(std::vector<uint8_t> buffer)
{
    NETMGR_EXT_LOG_I("NetworkSliceClient::SetNetworkSliceUePolicy, buffersize = %{public}d", int(buffer.size()));
    sptr<INetworkSliceService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("GetNetworkSliceService proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->SetNetworkSliceUePolicy(buffer);
}

int32_t NetworkSliceClient::NetworkSliceInitUePolicy()
{
    NETMGR_EXT_LOG_I("NetworkSliceClient::NetworkSliceInitUePolicy");
    sptr<INetworkSliceService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("GetNetworkSliceService proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->NetworkSliceInitUePolicy();
}

int32_t NetworkSliceClient::NetworkSliceAllowedNssaiRpt(std::vector<uint8_t> buffer)
{
    NETMGR_EXT_LOG_I("NetworkSliceClient::NetworkSliceAllowedNssaiRpt");
    sptr<INetworkSliceService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("NetworkSliceAllowedNssaiRpt proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->NetworkSliceAllowedNssaiRpt(buffer);
}

int32_t NetworkSliceClient::NetworkSliceEhplmnRpt(std::vector<uint8_t> buffer)
{
    NETMGR_EXT_LOG_I("NetworkSliceClient::NetworkSliceEhplmnRpt");
    sptr<INetworkSliceService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("NetworkSliceEhplmnRpt proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->NetworkSliceEhplmnRpt(buffer);
}

int32_t NetworkSliceClient::GetRouteSelectionDescriptorByDNN(std::string dnn, std::string& snssai, uint8_t& sscMode)
{
    NETMGR_EXT_LOG_I("NetworkSliceClient::GetRouteSelectionDescriptorByDNN");
    sptr<INetworkSliceService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("GetRouteSelectionDescriptorByDNN proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->GetRouteSelectionDescriptorByDNN(dnn, snssai, sscMode);
}

int32_t NetworkSliceClient::GetRSDByNetCap(int32_t netcap, std::map<std::string, std::string>& networkSliceParas)
{
    NETMGR_EXT_LOG_I("NetworkSliceClient::GetRSDByNetCap");
    sptr<INetworkSliceService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("GetRSDByNetCap proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->GetRSDByNetCap(netcap, networkSliceParas);
}

int32_t NetworkSliceClient::SetSaState(bool isSaState)
{
    NETMGR_EXT_LOG_I("NetworkSliceClient::SetSaState");
    sptr<INetworkSliceService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("SetSaState proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    return proxy->SetSaState(isSaState);
}

sptr<INetworkSliceService> NetworkSliceClient::GetProxy()
{
    std::lock_guard lock(mutex_);
    if (networksliceService_) {
        NETMGR_EXT_LOG_D("get proxy is ok");
        return networksliceService_;
    }
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        NETMGR_EXT_LOG_E("GetProxy, get SystemAbilityManager failed");
        return nullptr;
    }
    sptr<IRemoteObject> remote = sam->CheckSystemAbility(SLICE_SA_ID);
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("get Remote service failed");
        return nullptr;
    }
    deathRecipient_ = new (std::nothrow) NetworkSliceDeathRecipient(*this);
    if (deathRecipient_ == nullptr) {
        NETMGR_EXT_LOG_E("Recipient new failed!");
    }
    if ((remote->IsProxyObject()) && (!remote->AddDeathRecipient(deathRecipient_))) {
        NETMGR_EXT_LOG_E("add death recipient failed");
        return nullptr;
    }
    networksliceService_ = iface_cast<INetworkSliceService>(remote);
    if (networksliceService_ == nullptr) {
        NETMGR_EXT_LOG_E("get Remote service proxy failed");
        return nullptr;
    }
    return networksliceService_;
}

void NetworkSliceClient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    NETMGR_EXT_LOG_I("NetworkSlice OnRemoteDied");
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("OnRemoteDied failed, remote is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (networksliceService_ == nullptr) {
        NETMGR_EXT_LOG_E("OnRemoteDied proxy_ is nullptr");
        return;
    }
    sptr<IRemoteObject> serviceRemote = networksliceService_->AsObject();
    if ((serviceRemote != nullptr) && (serviceRemote == remote.promote())) {
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
        networksliceService_ = nullptr;
        NETMGR_EXT_LOG_E("on remote died");
    }
}

void NetworkSliceClient::DlCloseRemoveDeathRecipient()
{
    sptr<INetworkSliceService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("proxy is nullptr");
        return;
    }

    auto serviceRemote = proxy->AsObject();
    if (serviceRemote == nullptr) {
        NETMGR_EXT_LOG_E("serviceRemote is nullptr");
        return;
    }

    serviceRemote->RemoveDeathRecipient(deathRecipient_);
    NETMGR_EXT_LOG_I("RemoveDeathRecipient success");
}

} // namespace NetManagerStandard
} // namespace OHOS
