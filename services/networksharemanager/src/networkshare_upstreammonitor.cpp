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

#include "netmgr_ext_log_wrapper.h"
#include "networkshare_constants.h"

namespace OHOS {
namespace NetManagerStandard {
static constexpr const char *ERROR_MSG_HAS_NOT_UPSTREAM = "Has not Upstream Network";
static constexpr const char *ERROR_MSG_UPSTREAM_ERROR = "Get Upstream Network is Error";

NetworkShareUpstreamMonitor::NetConnectionCallback::NetConnectionCallback(
    const std::shared_ptr<NetworkShareUpstreamMonitor> &networkmonitor, int32_t callbackType)
    : NetworkMonitor_(networkmonitor)
{
}

int32_t NetworkShareUpstreamMonitor::NetConnectionCallback::NetAvailable(sptr<NetHandle> &netHandle)
{
    if (NetworkMonitor_) {
        NetworkMonitor_->StoreAvailableNetHandle(netHandle);
    }
    return NETWORKSHARE_SUCCESS;
}

int32_t NetworkShareUpstreamMonitor::NetConnectionCallback::NetCapabilitiesChange(sptr<NetHandle> &netHandle,
    const sptr<NetAllCapabilities> &netAllCap)
{
    if (NetworkMonitor_) {
        NetworkMonitor_->StoreNetCapAndNotify(netHandle, netAllCap);
    }
    return NETWORKSHARE_SUCCESS;
}

int32_t NetworkShareUpstreamMonitor::NetConnectionCallback::NetConnectionPropertiesChange(sptr<NetHandle> &netHandle,
                                                                                          const sptr<NetLinkInfo> &info)
{
    if (NetworkMonitor_) {
        NetworkMonitor_->StoreLinkInfoAndNotify(netHandle, info);
    }
    return NETWORKSHARE_SUCCESS;
}

int32_t NetworkShareUpstreamMonitor::NetConnectionCallback::NetLost(sptr<NetHandle> &netHandle)
{
    if (NetworkMonitor_) {
        NetworkMonitor_->RemoveNetHandleAndNotify(netHandle);
    }
    return NETWORKSHARE_SUCCESS;
}

int32_t NetworkShareUpstreamMonitor::NetConnectionCallback::NetUnavailable()
{
    return NETWORKSHARE_SUCCESS;
}

int32_t NetworkShareUpstreamMonitor::NetConnectionCallback::NetBlockStatusChange(sptr<NetHandle> &netHandle,
                                                                                 bool blocked)
{
    return NETWORKSHARE_SUCCESS;
}

NetworkShareUpstreamMonitor::NetworkShareUpstreamMonitor() : defaultNetHandleNetId_(-1) {}

NetworkShareUpstreamMonitor::~NetworkShareUpstreamMonitor()
{
    networkAndInfoMap_.clear();
    auto netManager = DelayedSingleton<NetConnClient>::GetInstance();
    if (netManager != nullptr) {
        netManager->UnregisterNetConnCallback(defaultNetworkCallback_);
    }
}

void NetworkShareUpstreamMonitor::SetOptionData(int what, std::weak_ptr<MonitorEventHandler> &handler)
{
    eventId_ = what;
    eventHandler_ = handler;
}

void NetworkShareUpstreamMonitor::ListenDefaultNetwork()
{
    auto netManager = DelayedSingleton<NetConnClient>::GetInstance();
    if (netManager == nullptr) {
        NETMGR_EXT_LOG_E("NetConnClient is null.");
        return;
    }
    std::shared_ptr<NetworkShareUpstreamMonitor> networkmonitor = shared_from_this();
    defaultNetworkCallback_ =
        std::make_unique<NetConnectionCallback>(networkmonitor, CALLBACK_DEFAULT_INTERNET_NETWORK).release();
    int32_t result = netManager->RegisterNetConnCallback(defaultNetworkCallback_);
    if (result == NETWORKSHARE_SUCCESS) {
        NETMGR_EXT_LOG_I("Register defaultNetworkCallback_ successful");
    } else {
        NETMGR_EXT_LOG_E("Register defaultNetworkCallback_ failed");
    }
}

void NetworkShareUpstreamMonitor::RegisterUpstreamChangedCallback(
    const std::shared_ptr<NotifyUpstreamCallback> &callback)
{
    notifyUpstreamCallback_ = callback;
}

void NetworkShareUpstreamMonitor::GetCurrentGoodUpstream(std::shared_ptr<UpstreamNetworkInfo> &upstreamNetInfo)
{
    auto netManager = DelayedSingleton<NetConnClient>::GetInstance();
    if (upstreamNetInfo == nullptr || netManager == nullptr) {
        NETMGR_EXT_LOG_E("NetConnClient or upstreamNetInfo is null.");
        return;
    }
    bool hasDefaultNet = true;
    int32_t result = netManager->HasDefaultNet(hasDefaultNet);
    if (result != ERR_NONE || !hasDefaultNet) {
        NetworkShareHisysEvent::GetInstance().SendFaultEvent(
            NetworkShareEventOperator::OPERATION_GET_UPSTREAM, NetworkShareEventErrorType::ERROR_GET_UPSTREAM,
            ERROR_MSG_HAS_NOT_UPSTREAM, NetworkShareEventType::SETUP_EVENT);
        NETMGR_EXT_LOG_E("NetConn hasDefaultNet error[%{public}d].", result);
        return;
    }

    netManager->GetDefaultNet(*(upstreamNetInfo->netHandle_));
    if (upstreamNetInfo->netHandle_ == nullptr) {
        NETMGR_EXT_LOG_E("netHandle_ is null.");
        return;
    }
    NETMGR_EXT_LOG_I("NetConn get defaultNet id[%{public}d].", upstreamNetInfo->netHandle_->GetNetId());
    if (upstreamNetInfo->netHandle_->GetNetId() < 0) {
        NetworkShareHisysEvent::GetInstance().SendFaultEvent(
            NetworkShareEventOperator::OPERATION_GET_UPSTREAM, NetworkShareEventErrorType::ERROR_GET_UPSTREAM,
            ERROR_MSG_UPSTREAM_ERROR, NetworkShareEventType::SETUP_EVENT);
        NETMGR_EXT_LOG_E("NetConn get defaultNet id is error.");
        return;
    }
    if (defaultNetHandleNetId_ == upstreamNetInfo->netHandle_->GetNetId()) {
        std::map<int32_t, std::shared_ptr<UpstreamNetworkInfo>>::iterator iter =
            networkAndInfoMap_.find(defaultNetHandleNetId_);
        if (iter != networkAndInfoMap_.end()) {
            upstreamNetInfo = iter->second;
        }
    } else {
        NETMGR_EXT_LOG_W("defaultNetHandleNetId_[%{public}u] != current default netid.", defaultNetHandleNetId_);
        defaultNetHandleNetId_ = upstreamNetInfo->netHandle_->GetNetId();
        netManager->GetNetCapabilities(*(upstreamNetInfo->netHandle_), *(upstreamNetInfo->netAllCap_));
        netManager->GetConnectionProperties(*(upstreamNetInfo->netHandle_), *(upstreamNetInfo->netLinkPro_));
        if (upstreamNetInfo->netLinkPro_ == nullptr || upstreamNetInfo->netAllCap_ == nullptr) {
            NETMGR_EXT_LOG_E("netLinkPro_ or netAllCap_ is null.");
            return;
        }
        NETMGR_EXT_LOG_W("CapsIsValid[%{public}d], ifaceName[%{public}s].", upstreamNetInfo->netAllCap_->CapsIsValid(),
                         upstreamNetInfo->netLinkPro_->ifaceName_.c_str());
        networkAndInfoMap_.insert(std::make_pair(upstreamNetInfo->netHandle_->GetNetId(), upstreamNetInfo));
    }
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

void NetworkShareUpstreamMonitor::StoreAvailableNetHandle(sptr<NetHandle> &netHandle)
{
    if (netHandle == nullptr) {
        return;
    }
    std::map<int32_t, std::shared_ptr<UpstreamNetworkInfo>>::iterator iter =
        networkAndInfoMap_.find(netHandle->GetNetId());
    if (iter == networkAndInfoMap_.end()) {
        NETMGR_EXT_LOG_I("netHandle[%{public}d] is new.", netHandle->GetNetId());
        sptr<NetAllCapabilities> netCap = new NetAllCapabilities();
        sptr<NetLinkInfo> linkInfo = new NetLinkInfo();
        std::shared_ptr<UpstreamNetworkInfo> networkInfo =
            std::make_shared<UpstreamNetworkInfo>(netHandle, netCap, linkInfo);
        networkAndInfoMap_.insert(std::make_pair(netHandle->GetNetId(), networkInfo));
    }
}

void NetworkShareUpstreamMonitor::StoreNetCapAndNotify(sptr<NetHandle> &netHandle,
                                                       const sptr<NetAllCapabilities> &newNetAllCap)
{
    std::map<int32_t, std::shared_ptr<UpstreamNetworkInfo>>::iterator iter =
        networkAndInfoMap_.find(netHandle->GetNetId());
    if (iter != networkAndInfoMap_.end()) {
        if ((iter->second)->netAllCap_ != newNetAllCap) {
            NETMGR_EXT_LOG_I("netHandle[%{public}d] store new Capabilities, old net[%{public}d].",
                             netHandle->GetNetId(), defaultNetHandleNetId_);
            *((iter->second)->netAllCap_) = *(newNetAllCap);
            if (defaultNetHandleNetId_ != netHandle->GetNetId()) {
                NotifyMainStateMachine(EVENT_UPSTREAM_CALLBACK_ON_LINKPROPERTIES, iter->second);
            }
        }
    }
}

void NetworkShareUpstreamMonitor::StoreLinkInfoAndNotify(sptr<NetHandle> &netHandle,
                                                         const sptr<NetLinkInfo> &newNetLinkInfo)
{
    std::map<int32_t, std::shared_ptr<UpstreamNetworkInfo>>::iterator iter =
        networkAndInfoMap_.find(netHandle->GetNetId());
    if (iter != networkAndInfoMap_.end()) {
        if ((iter->second)->netLinkPro_ != newNetLinkInfo) {
            NETMGR_EXT_LOG_I("netHandle[%{public}d] store new LinkNotify, old net[%{public}d].", netHandle->GetNetId(),
                             defaultNetHandleNetId_);
            (iter->second)->netLinkPro_->ifaceName_ = newNetLinkInfo->ifaceName_;
            if (defaultNetHandleNetId_ != netHandle->GetNetId()) {
                NotifyMainStateMachine(EVENT_UPSTREAM_CALLBACK_ON_LINKPROPERTIES, iter->second);
            }
        }
    }
}

void NetworkShareUpstreamMonitor::StoreLinkInfo(sptr<NetHandle> &netHandle, const sptr<NetLinkInfo> &newNetLinkInfo)
{
    std::map<int32_t, std::shared_ptr<UpstreamNetworkInfo>>::iterator iter =
        networkAndInfoMap_.find(netHandle->GetNetId());
    if (iter != networkAndInfoMap_.end()) {
        if ((iter->second)->netLinkPro_ != newNetLinkInfo) {
            NETMGR_EXT_LOG_I("netHandle[%{public}d] store new LinkInfo, old net[%{public}d].", netHandle->GetNetId(),
                             defaultNetHandleNetId_);
            (iter->second)->netLinkPro_->ifaceName_ = newNetLinkInfo->ifaceName_;
        }
    }
}

void NetworkShareUpstreamMonitor::RemoveNetHandleAndNotify(sptr<NetHandle> &netHandle)
{
    std::map<int32_t, std::shared_ptr<UpstreamNetworkInfo>>::iterator iter =
        networkAndInfoMap_.find(netHandle->GetNetId());
    if (iter != networkAndInfoMap_.end()) {
        std::shared_ptr<UpstreamNetworkInfo> tmpNetInfo = iter->second;
        NETMGR_EXT_LOG_I("netHandle[%{public}d] is lost, defaultNetId=[%{public}d].", netHandle->GetNetId(),
                         defaultNetHandleNetId_);
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
