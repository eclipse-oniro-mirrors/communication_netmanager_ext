/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "networkshare_upstreammonitor.h"

#ifdef SHARE_TRAFFIC_LIMIT_ENABLE
#include "cellular_data_client.h"
#endif
#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "networkshare_constants.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr const char *ERROR_MSG_HAS_NOT_UPSTREAM = "Has not Upstream Network";
constexpr const char *ERROR_MSG_UPSTREAM_ERROR = "Get Upstream Network is Error";
}

NetworkShareUpstreamMonitor::NetConnectionCallback::NetConnectionCallback(
    const std::shared_ptr<NetworkShareUpstreamMonitor> &networkmonitor, int32_t callbackType)
    : NetworkMonitor_(networkmonitor)
{
}

int32_t NetworkShareUpstreamMonitor::NetConnectionCallback::NetAvailable(sptr<NetHandle> &netHandle)
{
    ffrtQueue.submit([weakMonitor = std::weak_ptr(this->NetworkMonitor_), netHandle]() mutable {
        auto networkMonitor = weakMonitor.lock();
        if (networkMonitor) {
            networkMonitor->HandleNetAvailable(netHandle);
        }
    });
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkShareUpstreamMonitor::NetConnectionCallback::NetCapabilitiesChange(sptr<NetHandle> &netHandle,
    const sptr<NetAllCapabilities> &netAllCap)
{
    ffrtQueue.submit([weakMonitor = std::weak_ptr(this->NetworkMonitor_), netHandle, netAllCap]() mutable {
        auto networkMonitor = weakMonitor.lock();
        if (networkMonitor) {
            networkMonitor->HandleNetCapabilitiesChange(netHandle, netAllCap);
        }
    });
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkShareUpstreamMonitor::NetConnectionCallback::NetConnectionPropertiesChange(sptr<NetHandle> &netHandle,
                                                                                          const sptr<NetLinkInfo> &info)
{
    ffrtQueue.submit([weakMonitor = std::weak_ptr(this->NetworkMonitor_), netHandle, info]() mutable {
        auto networkMonitor = weakMonitor.lock();
        if (networkMonitor) {
            networkMonitor->HandleConnectionPropertiesChange(netHandle, info);
        }
    });
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkShareUpstreamMonitor::NetConnectionCallback::NetLost(sptr<NetHandle> &netHandle)
{
    ffrtQueue.submit([weakMonitor = std::weak_ptr(this->NetworkMonitor_), netHandle]() mutable {
        auto networkMonitor = weakMonitor.lock();
        if (networkMonitor) {
            networkMonitor->HandleNetLost(netHandle);
        }
    });
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkShareUpstreamMonitor::NetConnectionCallback::NetUnavailable()
{
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkShareUpstreamMonitor::NetConnectionCallback::NetBlockStatusChange(sptr<NetHandle> &netHandle,
                                                                                 bool blocked)
{
    return NETMANAGER_EXT_SUCCESS;
}

NetworkShareUpstreamMonitor::NetworkShareUpstreamMonitor() : defaultNetworkId_(INVALID_NETID)
{
    netSpecifier_ = sptr<NetSpecifier>::MakeSptr();
    netSpecifier_->netCapabilities_.netCaps_ = {NET_CAPABILITY_INTERNET, NET_CAPABILITY_NOT_VPN};
}

NetworkShareUpstreamMonitor::~NetworkShareUpstreamMonitor()
{
    std::lock_guard lock(networkMapMutex_);
    networkMaps_.clear();
}

void NetworkShareUpstreamMonitor::SetOptionData(int32_t what)
{
    eventId_ = what;
}

void NetworkShareUpstreamMonitor::ListenDefaultNetwork()
{
    std::lock_guard lock(networkCallbackMutex_);
    if (defaultNetworkCallback_ == nullptr) {
        defaultNetworkCallback_ =
            sptr<NetConnectionCallback>::MakeSptr(shared_from_this(), CALLBACK_DEFAULT_INTERNET_NETWORK);
    }
#ifdef SHARE_TRAFFIC_LIMIT_ENABLE
    bool isSupportDun = false;
    Telephony::CellularDataClient::GetInstance().GetIfSupportDunApn(isSupportDun);
    NETMGR_EXT_LOG_I("isSupportDun=%{public}d", isSupportDun);
    if (isSupportDun) {
        netSpecifier_->netCapabilities_.netCaps_ = {NET_CAPABILITY_DUN, NET_CAPABILITY_NOT_VPN};
    } else {
        netSpecifier_->netCapabilities_.netCaps_ = {NET_CAPABILITY_INTERNET, NET_CAPABILITY_NOT_VPN};
    }
#endif
    int32_t result = NetConnClient::GetInstance().RegisterNetConnCallback(netSpecifier_, defaultNetworkCallback_, 0);
    if (result == NETMANAGER_SUCCESS) {
        NETMGR_EXT_LOG_I("Register defaultNetworkCallback_ successful");
    } else {
        NETMGR_EXT_LOG_E("Register defaultNetworkCallback_ failed");
    }
}

void NetworkShareUpstreamMonitor::UnregisterListenDefaultNetwork()
{
    std::lock_guard lock(networkCallbackMutex_);
    int32_t result = NetConnClient::GetInstance().UnregisterNetConnCallback(defaultNetworkCallback_);
    if (result == NETMANAGER_SUCCESS) {
        NETMGR_EXT_LOG_I("UnRegister defaultNetworkCallback_ successful");
    } else {
        NETMGR_EXT_LOG_E("UnRegister defaultNetworkCallback_ failed");
    }
}

void NetworkShareUpstreamMonitor::RegisterUpstreamChangedCallback(
    const std::shared_ptr<NotifyUpstreamCallback> &callback)
{
    notifyUpstreamCallback_ = callback;
}

bool NetworkShareUpstreamMonitor::GetCurrentGoodUpstream(std::shared_ptr<UpstreamNetworkInfo> &upstreamNetInfo)
{
    bool hasDefaultNet = true;
    int32_t result = NetConnClient::GetInstance().HasDefaultNet(hasDefaultNet);
    if (result != NETMANAGER_SUCCESS || !hasDefaultNet) {
        NetworkShareHisysEvent::GetInstance().SendFaultEvent(
            NetworkShareEventOperator::OPERATION_GET_UPSTREAM, NetworkShareEventErrorType::ERROR_GET_UPSTREAM,
            ERROR_MSG_HAS_NOT_UPSTREAM, NetworkShareEventType::SETUP_EVENT);
        NETMGR_EXT_LOG_E("NetConn hasDefaultNet error[%{public}d].", result);
        return false;
    }
    std::lock_guard lock(networkMapMutex_);
    auto iter = networkMaps_.find(defaultNetworkId_);
    if (iter == networkMaps_.end()) {
        return false;
    }
    upstreamNetInfo = iter->second;
    return true;
}

void NetworkShareUpstreamMonitor::NotifyMainStateMachine(int which, const std::shared_ptr<UpstreamNetworkInfo> &obj)
{
    if (notifyUpstreamCallback_ == nullptr) {
        NETMGR_EXT_LOG_E("notifyUpstreamCallback is null.");
    } else {
        notifyUpstreamCallback_->OnUpstreamStateChanged(eventId_, which, 0, obj);
    }
}

void NetworkShareUpstreamMonitor::NotifyMainStateMachine(int which)
{
    if (notifyUpstreamCallback_ == nullptr) {
        NETMGR_EXT_LOG_E("notifyUpstreamCallback is null.");
    } else {
        notifyUpstreamCallback_->OnUpstreamStateChanged(eventId_, which);
    }
}

void NetworkShareUpstreamMonitor::HandleNetAvailable(sptr<NetHandle> &netHandle)
{
    if (netHandle == nullptr) {
        NETMGR_EXT_LOG_E("netHandle is null.");
        return;
    }
    std::lock_guard lock(networkMapMutex_);
    auto iter = networkMaps_.find(netHandle->GetNetId());
    if (iter == networkMaps_.end()) {
        NETMGR_EXT_LOG_I("netHandle[%{public}d] is new.", netHandle->GetNetId());
        sptr<NetAllCapabilities> netCap = sptr<NetAllCapabilities>::MakeSptr();
        sptr<NetLinkInfo> linkInfo = sptr<NetLinkInfo>::MakeSptr();
        std::shared_ptr<UpstreamNetworkInfo> network =
            std::make_shared<UpstreamNetworkInfo>(netHandle, netCap, linkInfo);
        networkMaps_.insert(std::make_pair(netHandle->GetNetId(), network));
    }
}

void NetworkShareUpstreamMonitor::HandleNetCapabilitiesChange(sptr<NetHandle> &netHandle,
                                                              const sptr<NetAllCapabilities> &newNetAllCap)
{
    if (netHandle == nullptr || newNetAllCap == nullptr) {
        NETMGR_EXT_LOG_E("netHandle or netCap is null.");
        return;
    }
    std::lock_guard lock(networkMapMutex_);
    auto iter = networkMaps_.find(netHandle->GetNetId());
    if (iter != networkMaps_.end()) {
        if (iter->second != nullptr && (iter->second)->netAllCap_ != newNetAllCap) {
            NETMGR_EXT_LOG_I("netHandle[%{public}d] Capabilities Changed.", netHandle->GetNetId());
            *((iter->second)->netAllCap_) = *(newNetAllCap);
        }
    }
}

void NetworkShareUpstreamMonitor::HandleConnectionPropertiesChange(sptr<NetHandle> &netHandle,
                                                                   const sptr<NetLinkInfo> &newNetLinkInfo)
{
    if (netHandle == nullptr || newNetLinkInfo == nullptr) {
        NETMGR_EXT_LOG_E("netHandle or netLinkInfo is null.");
        return;
    }
    std::shared_ptr<UpstreamNetworkInfo> currentNetwork = nullptr;
    {
        std::lock_guard lock(networkMapMutex_);
        auto iter = networkMaps_.find(netHandle->GetNetId());
        if (iter != networkMaps_.end()) {
            if (iter->second != nullptr && (iter->second)->netLinkPro_ != newNetLinkInfo) {
                currentNetwork = (iter->second);
                NETMGR_EXT_LOG_I("netHandle[%{public}d] ConnectionProperties Changed.", netHandle->GetNetId());
                currentNetwork->netLinkPro_ = newNetLinkInfo;
            }
        }
    }

    if (currentNetwork != nullptr) {
        if (defaultNetworkId_ == INVALID_NETID || defaultNetworkId_ == netHandle->GetNetId()) {
            defaultNetworkId_ = netHandle->GetNetId();
            NETMGR_EXT_LOG_I("Send MainSM ON_LINKPROPERTY event with netHandle[%{public}d].", netHandle->GetNetId());
            NotifyMainStateMachine(EVENT_UPSTREAM_CALLBACK_ON_LINKPROPERTIES, currentNetwork);
        } else {
            defaultNetworkId_ = netHandle->GetNetId();
            NETMGR_EXT_LOG_I("Send MainSM ON_SWITCH event with netHandle[%{public}d].", netHandle->GetNetId());
            NotifyMainStateMachine(EVENT_UPSTREAM_CALLBACK_DEFAULT_SWITCHED, currentNetwork);
        }
    }
}

void NetworkShareUpstreamMonitor::HandleNetLost(sptr<NetHandle> &netHandle)
{
    if (netHandle == nullptr) {
        return;
    }
    std::shared_ptr<UpstreamNetworkInfo> currentNetInfo = nullptr;
    {
        std::lock_guard lock(networkMapMutex_);
        auto iter = networkMaps_.find(netHandle->GetNetId());
        if (iter != networkMaps_.end()) {
            NETMGR_EXT_LOG_I("netHandle[%{public}d] is lost, defaultNetId[%{public}d].", netHandle->GetNetId(),
                             defaultNetworkId_);
            currentNetInfo = iter->second;
            networkMaps_.erase(netHandle->GetNetId());
        }
    }

    if (currentNetInfo != nullptr && defaultNetworkId_ == netHandle->GetNetId()) {
        NETMGR_EXT_LOG_I("Send MainSM ON_LOST event with netHandle[%{public}d].", defaultNetworkId_);
        NotifyMainStateMachine(EVENT_UPSTREAM_CALLBACK_ON_LOST, currentNetInfo);
        defaultNetworkId_ = INVALID_NETID;
    }
}

void NetworkShareUpstreamMonitor::OnNetworkConnectChange(int32_t state, int32_t bearerType)
{
    if (bearerType != BEARER_CELLULAR) {
        return;
    }
    if (state == NET_CONN_STATE_CONNECTED || state == NET_CONN_STATE_DISCONNECTED) {
        NETMGR_EXT_LOG_I("OnNetworkConnectChange re-Register cell");
        UnregisterListenDefaultNetwork();
        ListenDefaultNetwork();
    }
}

NetworkShareUpstreamMonitor::MonitorEventHandler::MonitorEventHandler(
    const std::shared_ptr<NetworkShareUpstreamMonitor> &networkmonitor,
    const std::shared_ptr<AppExecFwk::EventRunner> &runner)
    : AppExecFwk::EventHandler(runner), networkMonitor_(networkmonitor)
{
}
} // namespace NetManagerStandard
} // namespace OHOS
