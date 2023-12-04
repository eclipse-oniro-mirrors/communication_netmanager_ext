/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "ethernet_dhcp_controller.h"
#include <string>
#include "ethernet_dhcp_callback.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {

EthernetDhcpController *EthernetDhcpController::EthernetDhcpControllerResultNotify::ethDhcpController_ = nullptr;
void EthernetDhcpController::EthernetDhcpControllerResultNotify::OnSuccess(int status, const char *ifname,
                                                                           DhcpResult *result)
{
    if (ifname == nullptr || result == nullptr) {
        NETMGR_EXT_LOG_E("ifname or result is nullptr!");
        return;
    }

    if (ethDhcpController_ != nullptr) {
        NETMGR_EXT_LOG_I("EthernetDhcpControllerResultNotify OnSuccess.");
        ethDhcpController_->OnDhcpSuccess(ifname, result);
    }
}

void EthernetDhcpController::EthernetDhcpControllerResultNotify::OnFailed(int status, const char *ifname,
                                                                          const char *reason)
{
    NETMGR_EXT_LOG_I("EthernetDhcpControllerResultNotify OnFailed.");
    return;
}

void EthernetDhcpController::EthernetDhcpControllerResultNotify::SetEthernetDhcpController(
    EthernetDhcpController *ethDhcpController)
{
    ethDhcpController_ = ethDhcpController;
}

void EthernetDhcpController::RegisterDhcpCallback(sptr<EthernetDhcpCallback> callback)
{
    cbObject_ = callback;
}

void EthernetDhcpController::StartClient(const std::string &iface, bool bIpv6)
{
    clientEvent.OnIpSuccessChanged = EthernetDhcpControllerResultNotify::OnSuccess;
    clientEvent.OnIpFailChanged = EthernetDhcpControllerResultNotify::OnFailed;
    dhcpResultNotify_->SetEthernetDhcpController(this);
    if (RegisterDhcpClientCallBack(iface.c_str(), &clientEvent) != DHCP_SUCCESS) {
        NETMGR_EXT_LOG_E("RegisterDhcpClientCallBack failed.");
        return;
    }
    NETMGR_EXT_LOG_I("Start dhcp client iface[%{public}s] ipv6[%{public}d]", iface.c_str(), bIpv6);
    if (StartDhcpClient(iface.c_str(), bIpv6) != DHCP_SUCCESS) {
        NETMGR_EXT_LOG_E("StartDhcpClient failed.");
    }
}

void EthernetDhcpController::StopClient(const std::string &iface, bool bIpv6)
{
    NETMGR_EXT_LOG_D("StopClient iface[%{public}s] ipv6[%{public}d]", iface.c_str(), bIpv6);
    if (StopDhcpClient(iface.c_str(), bIpv6) != DHCP_SUCCESS) {
        NETMGR_EXT_LOG_E("StopDhcpClient failed.");
    }
}

void EthernetDhcpController::OnDhcpSuccess(const std::string &iface, DhcpResult *result)
{
    if (cbObject_ == nullptr || result == nullptr) {
        NETMGR_EXT_LOG_E("cbObject_ or result is nullptr!");
        return;
    }
    NETMGR_EXT_LOG_I("OnDhcpSuccess, iface[%{public}s]", iface.c_str());
    EthernetDhcpCallback::DhcpResult dhcpResult;
    dhcpResult.iface = iface;
    dhcpResult.ipAddr = result->strOptClientId;
    dhcpResult.gateWay = result->strOptRouter1;
    dhcpResult.subNet = result->strOptSubnet;
    dhcpResult.route1 = result->strOptRouter1;
    dhcpResult.route2 = result->strOptRouter2;
    dhcpResult.dns1 = result->strOptDns1;
    dhcpResult.dns2 = result->strOptDns2;
    cbObject_->OnDhcpSuccess(dhcpResult);
}

void EthernetDhcpController::OnDhcpFailed(int status, const std::string &ifname, const char *reason) {}
} // namespace NetManagerStandard
} // namespace OHOS
