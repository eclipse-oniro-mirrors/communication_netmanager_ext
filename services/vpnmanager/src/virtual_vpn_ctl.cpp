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

#include "virtual_vpn_ctl.h"

#include "broadcast_manager.h"
#include "common_event_support.h"
#include "net_manager_constants.h"
#include "net_manager_ext_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "netsys_controller.h"

namespace OHOS {
namespace NetManagerStandard {

VirtualVpnCtl::VirtualVpnCtl(sptr<VpnConfig> config, const std::string &pkg, int32_t userId,
    std::vector<int32_t> &activeUserIds)
    : NetVpnImpl(config, pkg, userId, activeUserIds)
{
}

bool VirtualVpnCtl::IsInternalVpn()
{
    return false;
}

/*
 * virtual vpn ctl is used for distributed modem;
 * when the phone is sharing vpn, the tablet should also show vpn sharing icon,
 * the class VirtualVpnCtl is used for show vpn sharing icon and register the VPN bearer;
 * Here we reuse RegisterNetSupplier, UpdateNetSupplierInfo, NotifyConnectState of base class
 * to implements the vpn icon and vpn bearer type.
 */
int32_t VirtualVpnCtl::SetUp(bool isInternalChannel)
{
    if (vpnConfig_ == nullptr) {
        NETMGR_EXT_LOG_E("VpnConnect vpnConfig_ is nullptr");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }

    NETMGR_EXT_LOG_I("SetUp virtual vpn interface name:%{public}s", GetInterfaceName().c_str());

    VpnEventType legacy = IsInternalVpn() ? VpnEventType::TYPE_LEGACY : VpnEventType::TYPE_EXTENDED;

    auto &netConnClientIns = NetConnClient::GetInstance();
    if (!RegisterNetSupplier(netConnClientIns, isInternalChannel)) {
        VpnHisysEvent::SendFaultEventConnSetting(legacy, VpnEventErrorType::ERROR_REG_NET_SUPPLIER_ERROR,
                                                 "register Supplier failed");
        return NETMANAGER_EXT_ERR_INTERNAL;
    } else if (!UpdateNetSupplierInfo(netConnClientIns, true)) {
        VpnHisysEvent::SendFaultEventConnSetting(legacy, VpnEventErrorType::ERROR_UPDATE_SUPPLIER_INFO_ERROR,
                                                 "update Supplier info failed");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }

    SendConnectionChangedBroadcast(NET_CONN_STATE_CONNECTED);

#ifdef SUPPORT_SYSVPN
    if (!IsSystemVpn()) {
        NotifyConnectState(VpnConnectState::VPN_CONNECTED);
    }
#else
    NotifyConnectState(VpnConnectState::VPN_CONNECTED);
#endif

    return NETMANAGER_EXT_SUCCESS;
}

void VirtualVpnCtl::NotifyConnectState(const VpnConnectState &state)
{
    std::shared_ptr<IVpnConnStateCb> cb = GetConnectStateChangedCb();
    if (cb == nullptr) {
        NETMGR_EXT_LOG_E("NotifyConnectState connect callback is null.");
        return;
    }

    std::string vpnId = "";
#ifdef SUPPORT_SYSVPN
    vpnId = (multiVpnInfo_ != nullptr) ? multiVpnInfo_->vpnId : "";
    if (multiVpnInfo_ != nullptr) {
        multiVpnInfo_->vpnConnectState = state;
        cb->OnMultiVpnConnStateChanged(state, vpnId);
    }
#endif // SUPPORT_SYSVPN
    cb->OnVpnConnStateChanged(state, GetInterfaceName(), GetVpnIfAddr(), vpnId, IsGlobalVpn());
    cb->SendConnStateChanged(state, VpnType::VIRTUAL_VPN);
}

int32_t VirtualVpnCtl::Destroy()
{
    auto &netConnClientIns = NetConnClient::GetInstance();
    UpdateNetSupplierInfo(netConnClientIns, false);
    UnregisterNetSupplier(netConnClientIns);
    
    SendConnectionChangedBroadcast(NET_CONN_STATE_DISCONNECTED);

#ifdef SUPPORT_SYSVPN
    if (!IsSystemVpn()) {
        NotifyConnectState(VpnConnectState::VPN_DISCONNECTED);
        NETMGR_EXT_LOG_I("notify connect state");
    }
#else
    NotifyConnectState(VpnConnectState::VPN_DISCONNECTED);
#endif

    NETMGR_EXT_LOG_I("virtual vpn destroy interface name:%{public}s", GetInterfaceName().c_str());
    return NETMANAGER_EXT_SUCCESS;
}

void VirtualVpnCtl::SendConnectionChangedBroadcast(const NetConnState &netConnState)
{
    BroadcastInfo info;
    info.action = EventFwk::CommonEventSupport::COMMON_EVENT_CONNECTIVITY_CHANGE;
    info.data = "Net Manager Connection State Changed";
    info.code = static_cast<int32_t>(netConnState);
    info.ordered = false;
    std::map<std::string, int32_t> param = {{"NetType", static_cast<int32_t>(BEARER_VPN)}};
    BroadcastManager::GetInstance().SendBroadcast(info, param);
}
} // namespace NetManagerStandard
} // namespace OHOS
