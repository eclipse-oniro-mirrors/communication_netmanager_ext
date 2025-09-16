/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
 
#include "net_eap_handler.h"
#include <utility>
#include <map>
 
#include "netmgr_ext_log_wrapper.h"
#include "net_manager_constants.h"
#ifdef NET_EXTENSIBLE_AUTHENTICATION
#include "net_manager_center.h"
#endif
 
namespace OHOS {
namespace NetManagerStandard {

static constexpr const char* ETH_PREFIX = "eth";
static constexpr const int32_t DEFAULT_ETH_EAP_NETID = -1;

NetEapHandler::NetEapHandler()
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    eapHdiWpaManager_ = std::make_shared<EapHdiWpaManager>();
#endif
}

NetEapHandler &NetEapHandler::GetInstance()
{
    static NetEapHandler gNetEap;
    return gNetEap;
}
 
int32_t NetEapHandler::RegisterCustomEapCallback(const NetType netType, const sptr<INetRegisterEapCallback> &callback)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    if (callback == nullptr) {
        NETMGR_EXT_LOG_E("%{public}s, callback is nullptr", __func__);
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    if (netType < NetType::WLAN0 || netType > NetType::ETH0) {
        NETMGR_EXT_LOG_E("NetEapHandler, RegisterCustomEapCallback invalid netType %{public}d", netType);
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }
    std::unique_lock<std::mutex> lock(mutex_);
    regEapCallBack_[netType] = callback;
    lock.unlock();
 
    NETMGR_EXT_LOG_E("RegisterCustomEapCallback success, netType:%{public}d", static_cast<int>(netType));
#endif
    return NETMANAGER_SUCCESS;
}
 
int32_t NetEapHandler::UnRegisterCustomEapCallback(NetType netType, const sptr<INetRegisterEapCallback> &callback)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    if (callback == nullptr) {
        NETMGR_EXT_LOG_E("%{public}s, postBackCb is nullptr", __func__);
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    if (netType < NetType::WLAN0 || netType > NetType::ETH0) {
        NETMGR_EXT_LOG_E("NetEapHandler, UnRegisterCustomEapCallback invalid netType %{public}d", netType);
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }
    std::unique_lock<std::mutex> lock(mutex_);
    auto iter = regEapCallBack_.find(netType);
    if (iter != regEapCallBack_.end()) {
        regEapCallBack_.erase(iter);
    }
    lock.unlock();
#endif
    return NETMANAGER_SUCCESS;
}
 
int32_t NetEapHandler::RegCustomEapHandler(NetType netType, const std::string &regCmd,
    const sptr<INetEapPostbackCallback> &postBackCb)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    if (postBackCb == nullptr) {
        NETMGR_EXT_LOG_E("%{public}s, postBackCb is nullptr", __func__);
        return EAP_ERRCODE_INTERNAL_ERROR;
    }
    SetPostbackCallback(postBackCb);
    if (netType < NetType::WLAN0 || netType > NetType::ETH0) {
        NETMGR_EXT_LOG_E("NetEapHandler, RegCustomEapHandler invalid netType %{public}d", static_cast<int>(netType));
        return EAP_ERRCODE_INTERNAL_ERROR;
    }
    if (netType == NetType::ETH0) {
        NETMGR_EXT_LOG_I("RegEapHandler eth0 %{public}s", regCmd.c_str());
        ethEapRegCmdVec_.emplace_back(regCmd);
        return EAP_ERRCODE_SUCCESS;
    }
    std::unique_lock<std::mutex> lock(mutex_);
    auto iter = regEapCallBack_.find(netType);
    lock.unlock();
    if (iter == regEapCallBack_.end()) {
        NETMGR_EXT_LOG_E("RegCustomEapHandler not have callback, netType:%{public}d", static_cast<int>(netType));
        return EAP_ERRCODE_INTERNAL_ERROR;
    }
    
    if (iter->second == nullptr) {
        NETMGR_EXT_LOG_E("regEapCallBack ptr is nullptr, netType:%{public}d", static_cast<int>(netType));
        return EAP_ERRCODE_INTERNAL_ERROR;
    }
    iter->second->OnRegisterCustomEapCallback(regCmd);
    NETMGR_EXT_LOG_I("RegCustomEapHandler success.");
#endif
    return NETMANAGER_SUCCESS;
}
 
int32_t NetEapHandler::NotifyWpaEapInterceptInfo(const NetType netType, const sptr<EapData> &eapData)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    if (eapData == nullptr) {
        NETMGR_EXT_LOG_E("%{public}s eapData is nullptr", __func__);
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    if (eapData->eapBuffer.size() == 0) {
        NETMGR_EXT_LOG_E("%{public}s eapData size is 0, %{public}s", __func__, eapData->PrintLogInfo().c_str());
        return NETMANAGER_ERR_INVALID_PARAMETER;
    }
    auto postbackCb = GetPostbackCallback();
    if (postbackCb) {
        nTMapMsgId_[netType] = eapData->msgId;
        postbackCb->OnEapSupplicantPostback(netType, eapData);
    }
#endif
    return NETMANAGER_SUCCESS;
}
 
int32_t NetEapHandler::ReplyCustomEapData(int result, const sptr<EapData> &eapData)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    auto iter = std::find_if(nTMapMsgId_.begin(), nTMapMsgId_.end(),
        [eapData](const std::pair<NetType, int>& p) { return p.second == eapData->msgId; });
    if (iter == nTMapMsgId_.end()) {
        NETMGR_EXT_LOG_E("%{public}s, don't match msgId and type, WALN0:%{public}zu, ETH0:%{public}zu", __func__,
            nTMapMsgId_.count(NetType::WLAN0), nTMapMsgId_.count(NetType::ETH0));
        return EAP_ERRCODE_INTERNAL_ERROR;
    }
    /* ETH0 do not need check regEapCallBack_ */
    if (iter->first == NetType::ETH0) {
        if (eapHdiWpaManager_ == nullptr) {
            return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
        }
        return eapHdiWpaManager_->ReplyCustomEapData(ethEapIfName_, result, eapData);
    }
    std::unique_lock<std::mutex> lock(mutex_);
    auto &callback = regEapCallBack_[iter->first];
    lock.unlock();
    if (callback == nullptr) {
        NETMGR_EXT_LOG_E("%{public}s, callback is nullptr", __func__);
        return EAP_ERRCODE_INTERNAL_ERROR;
    }
    callback->OnReplyCustomEapDataEvent(result, eapData);
#endif
    return NETMANAGER_SUCCESS;
}
 
void NetEapHandler::SetPostbackCallback(const sptr<INetEapPostbackCallback> &postbackCallback)
{
    postbackCallback_ = postbackCallback;
}
 
sptr<INetEapPostbackCallback> NetEapHandler::GetPostbackCallback()
{
    return postbackCallback_;
}

#ifdef NET_EXTENSIBLE_AUTHENTICATION
void NetEapHandler::SetEthEapIfName(const std::string &ifName)
{
    NETMGR_EXT_LOG_I("SetEthEapIfName %{public}s", ifName.c_str());
    ethEapIfName_ = ifName;
}

int32_t NetEapHandler::StartEthEap(int32_t netId, const EthEapProfile& profile)
{
    if (eapHdiWpaManager_ == nullptr) {
        return EAP_ERRCODE_INTERNAL_ERROR;
    }
    if (netId != DEFAULT_ETH_EAP_NETID) {
        GetIfaceNameFromNetId(netId);
        if (ethEapIfName_.find(ETH_PREFIX) == std::string::npos) {
            NETMGR_EXT_LOG_E("StartEthEap invalid netid %{public}d", netId);
            return EAP_ERRCODE_INVALID_NETID;
        }
    }
    int32_t ret = eapHdiWpaManager_->LoadEthernetHdiService();
    if (ret != EAP_ERRCODE_SUCCESS) {
        return ret;
    }
    ret = eapHdiWpaManager_->StartEap(ethEapIfName_, profile);
    for (const std::string &cmd : ethEapRegCmdVec_) {
        eapHdiWpaManager_->RegisterCustomEapCallback(ethEapIfName_, cmd);
    }
    return ret;
}
 
int32_t NetEapHandler::LogOffEthEap(int32_t netId)
{
    if (eapHdiWpaManager_ == nullptr) {
        return EAP_ERRCODE_INTERNAL_ERROR;
    }
    if (netId != DEFAULT_ETH_EAP_NETID) {
        GetIfaceNameFromNetId(netId);
        if (ethEapIfName_.find(ETH_PREFIX) == std::string::npos) {
            NETMGR_EXT_LOG_E("LogOffEthEap invalid netid %{public}d", netId);
            return EAP_ERRCODE_INVALID_NETID;
        }
    }
    return eapHdiWpaManager_->StopEap(ethEapIfName_);
}
 
void NetEapHandler::GetIfaceNameFromNetId(int32_t netId)
{
    NetLinkInfo info;
    NetManagerCenter::GetInstance().GetConnectionProperties(netId, info);
    ethEapIfName_ = info.ifaceName_;
}

#endif // NET_EXTENSIBLE_AUTHENTICATION
 
} // namespace NetManagerStandard
} // namespace OHOS