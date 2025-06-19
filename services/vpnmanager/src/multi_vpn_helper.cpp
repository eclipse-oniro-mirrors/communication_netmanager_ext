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

#include "multi_vpn_helper.h"

#include <string>

#include <fcntl.h>

#include "cJSON.h"
#include "net_manager_constants.h"
#include "net_manager_ext_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "ipc_skeleton.h"
#include "l2tp_vpn_ctl.h"
#include "open_vpn_ctl.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t MAX_VPN_INTERFACE_COUNT = 20;
constexpr const char *PPP_CARD_NAME = "ppp-vpn";
constexpr const char *XFRM_CARD_NAME = "xfrm-vpn";
MultiVpnHelper &MultiVpnHelper::GetInstance()
{
    static MultiVpnHelper instance;
    return instance;
}

MultiVpnHelper::MultiVpnHelper()
{
    multiVpnInfos_.reserve(MAX_VPN_INTERFACE_COUNT);
}

int32_t MultiVpnHelper::GetNewIfNameId()
{
    int32_t newId = 1;
    if (multiVpnInfos_.size() == 0) {
        return newId;
    }
    std::set<int32_t> ifNameIds;
    for (sptr<MultiVpnInfo> info : multiVpnInfos_) {
        if (info != nullptr) {
            ifNameIds.insert(info->ifNameId);
        }
    }
    for (int32_t id : ifNameIds) {
        if (id != newId) {
            return newId;
        }
        newId++;
    }
    return newId;
}

int32_t MultiVpnHelper::CreateMultiVpnInfo(const std::string &vpnId, int32_t vpnType, sptr<MultiVpnInfo> &info)
{
    if (multiVpnInfos_.size() >= MAX_VPN_INTERFACE_COUNT) {
        NETMGR_EXT_LOG_E("CreateMultiVpnInfo failed, MAX_VPN_INTERFACE_COUNT");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    int32_t ifNameId = GetNewIfNameId();
    std::string newIfName;
    switch (vpnType) {
        case VpnType::IKEV2_IPSEC_MSCHAPv2:
        case VpnType::IKEV2_IPSEC_PSK:
        case VpnType::IKEV2_IPSEC_RSA:
        case VpnType::IPSEC_XAUTH_PSK:
        case VpnType::IPSEC_XAUTH_RSA:
        case VpnType::IPSEC_HYBRID_RSA:
            newIfName = XFRM_CARD_NAME + std::to_string(ifNameId);
            break;
        case VpnType::OPENVPN:
            newIfName = TUN_CARD_NAME;
            break;
        case VpnType::L2TP_IPSEC_PSK:
        case VpnType::L2TP_IPSEC_RSA:
        case VpnType::L2TP:
            newIfName = PPP_CARD_NAME + std::to_string(ifNameId);
            break;
        default:
            NETMGR_EXT_LOG_I("other vpnType=%{public}d", vpnType);
            break;
    }
    info = new (std::nothrow) MultiVpnInfo();
    if (info == nullptr) {
        NETMGR_EXT_LOG_E("CreateMultiVpnInfo failed, info is null");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    info->vpnId = vpnId;
    info->ifNameId = ifNameId;
    info->ifName = newIfName;
    info->callingUid = static_cast<uint32_t>(IPCSkeleton::GetCallingUid());
    NETMGR_EXT_LOG_I("CreateMultiVpnInfo %{public}s", newIfName.c_str());
    return NETMANAGER_EXT_SUCCESS;
}

int32_t MultiVpnHelper::AddMultiVpnInfo(const sptr<MultiVpnInfo> &info)
{
    if (info == nullptr || info->ifName.empty()) {
        NETMGR_EXT_LOG_E("AddMultiVpnInfo failed, info invalid");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    if (std::find(multiVpnInfos_.begin(), multiVpnInfos_.end(), info) == multiVpnInfos_.end()) {
        multiVpnInfos_.emplace_back(info);
    }
    NETMGR_EXT_LOG_D("AddMultiVpnInfo %{public}s", info->ifName.c_str());
    return NETMANAGER_EXT_SUCCESS;
}

int32_t MultiVpnHelper::DelMultiVpnInfo(const sptr<MultiVpnInfo> &info)
{
    if (info == nullptr) {
        NETMGR_EXT_LOG_E("DelMultiVpnInfo failed, info invalid");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    multiVpnInfos_.erase(std::remove(multiVpnInfos_.begin(), multiVpnInfos_.end(), info),
        multiVpnInfos_.end());
    NETMGR_EXT_LOG_I("DelMultiVpnInfo %{public}s", info->ifName.c_str());
    return NETMANAGER_EXT_SUCCESS;
}

bool MultiVpnHelper::StartIpsec()
{
    if (ipsecStartedCount_ <= 0) {
        NETMGR_EXT_LOG_I("ipsec start");
        NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_RESTART);
        ipsecStartedCount_ = ipsecStartedCount_ + 1;
        return true;
    }
    ipsecStartedCount_ = ipsecStartedCount_ + 1;
    NETMGR_EXT_LOG_I("ipsec start count %{public}d", ipsecStartedCount_);
    return false;
}

void MultiVpnHelper::StopIpsec()
{
    ipsecStartedCount_ = ipsecStartedCount_ - 1;
    NETMGR_EXT_LOG_I("ipsec stop count %{public}d", ipsecStartedCount_);
}

bool MultiVpnHelper::StartL2tp()
{
    if (l2tpStartedCount_ <= 0) {
        NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_L2TP_LOAD);
        l2tpStartedCount_ = l2tpStartedCount_ + 1;
        return true;
    }
    l2tpStartedCount_ = l2tpStartedCount_ + 1;
    NETMGR_EXT_LOG_I("L2tp start count %{public}d", l2tpStartedCount_);
    return false;
}

void MultiVpnHelper::StopL2tp()
{
    l2tpStartedCount_ = l2tpStartedCount_ - 1;
    NETMGR_EXT_LOG_I("L2tp stop count %{public}d", l2tpStartedCount_);
}

bool MultiVpnHelper::IsConnectedStage(const std::string &stage)
{
    if (stage.empty()) {
        return false;
    }
    if (stage.compare(IPSEC_CONNECT_TAG) == 0 || stage.compare(L2TP_IPSEC_CONNECTED_TAG) == 0 ||
                IsOpenvpnConnectedStage(stage)) {
        // is connected stage
        return true;
    }
    return false;
}

bool MultiVpnHelper::IsOpenvpnConnectedStage(const std::string &msg)
{
    bool openvpnConnected = false;
    if (msg.empty()) {
        NETMGR_EXT_LOG_E("msg is empty");
        return false;
    }
    if (strstr(msg.c_str(), OPENVPN_NODE_ROOT) != 0) {
        const char *ret = strstr(msg.c_str(), "{");
        if (ret == nullptr) {
            NETMGR_EXT_LOG_E("client message format error");
            return false;
        }
        cJSON* message = cJSON_Parse(ret);
        if (message == nullptr) {
            NETMGR_EXT_LOG_E("not json string");
            return false;
        }
        // is state message
        cJSON* state = cJSON_GetObjectItem(message, OPENVPN_NODE_UPDATE_STATE);
        if (state != nullptr && cJSON_IsObject(state)) {
            cJSON* updateState = cJSON_GetObjectItem(state, OPENVPN_NODE_STATE);
            if (updateState != nullptr && cJSON_IsNumber(updateState) &&
                static_cast<int32_t>(cJSON_GetNumberValue(updateState)) == OPENVPN_STATE_CONNECTED) {
                // is openvpn connected stage
                openvpnConnected = true;
            }
        }
        cJSON_Delete(message);
    }
    return openvpnConnected;
}
} // namespace NetManagerStandard
} // namespace OHOS
