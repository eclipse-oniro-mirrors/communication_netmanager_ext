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

#include "extended_vpn_ctl.h"

#include <regex>
#include <string>

#include <fcntl.h>

#include "net_manager_constants.h"
#include "net_manager_ext_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "netsys_controller.h"

namespace OHOS {
namespace NetManagerStandard {

ExtendedVpnCtl::ExtendedVpnCtl(sptr<VpnConfig> config, const std::string &pkg, int32_t userId, std::vector<int32_t> &activeUserIds)
    : NetVpnImpl(config, pkg, userId, activeUserIds)
{
}

bool ExtendedVpnCtl::IsInternalVpn()
{
    return false;
}

int32_t ExtendedVpnCtl::SetUp()
{
    NETMGR_EXT_LOG_I("SetUp virtual network");
#ifdef SUPPORT_SYSVPN
    if (vpnConfig_ != nullptr && !vpnConfig_->vpnId_.empty() && multiVpnInfo_ != nullptr) {
        NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_SET_VPN_CALL_MODE,
            multiVpnInfo_->isVpnExtCall ? "0" : "1");
        std::string addr = vpnConfig_->addresses_.empty() ? "" : vpnConfig_->addresses_.back().address_;
        if (MultiVpnHelper::GetInstance().CheckAndCompareMultiVpnLocalAddress(addr) != NETMANAGER_EXT_SUCCESS) {
            NETMGR_EXT_LOG_E("forbid setup, multi tun check ip address is same");
            return NETMANAGER_EXT_ERR_INTERNAL;
        }
        multiVpnInfo_->localAddress = addr;
    }
#endif // SUPPORT_SYSVPN
    return NetVpnImpl::SetUp();
}

int32_t ExtendedVpnCtl::Destroy()
{
    NETMGR_EXT_LOG_I("Destroy virtual network");
#ifdef SUPPORT_SYSVPN
    if (vpnConfig_ != nullptr && !vpnConfig_->vpnId_.empty() && multiVpnInfo_ != nullptr) {
        NetsysController::GetInstance().ProcessVpnStage(SysVpnStageCode::VPN_STAGE_SET_VPN_CALL_MODE,
            multiVpnInfo_->isVpnExtCall ? "0" : "1");
    }
#endif // SUPPORT_SYSVPN
    return NetVpnImpl::Destroy();
}

} // namespace NetManagerStandard
} // namespace OHOS
