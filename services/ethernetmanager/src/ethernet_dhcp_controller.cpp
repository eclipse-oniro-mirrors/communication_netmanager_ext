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
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t DHCP_TIMEOUT = 300;
EthernetDhcpController::EthernetDhcpControllerResultNotify::EthernetDhcpControllerResultNotify(
    EthernetDhcpController &ethDhcpController)
    : ethDhcpController_(ethDhcpController)
{
}

EthernetDhcpController::EthernetDhcpControllerResultNotify::~EthernetDhcpControllerResultNotify() {}

void EthernetDhcpController::EthernetDhcpControllerResultNotify::OnSuccess(int status, const std::string &ifname,
    OHOS::Wifi::DhcpResult &result)
{
    NETMGR_EXT_LOG_D("Enter EthernetDhcpController::EthernetDhcpControllerResultNotify::OnSuccess "
        "ifname=[%{private}s], iptype=[%{private}d], strYourCli=[%{private}s], "
        "strServer=[%{private}s], strSubnet=[%{private}s], strDns1=[%{private}s], "
        "strDns2=[%{private}s] strRouter1=[%{private}s] strRouter2=[%{private}s]",
        ifname.c_str(), result.iptype, result.strYourCli.c_str(), result.strServer.c_str(),
        result.strSubnet.c_str(), result.strDns1.c_str(), result.strDns2.c_str(), result.strRouter1.c_str(),
        result.strRouter2.c_str());
    ethDhcpController_.OnDhcpSuccess(ifname, result);
}

void EthernetDhcpController::EthernetDhcpControllerResultNotify::OnFailed(int status, const std::string &ifname,
    const std::string &reason)
{
    NETMGR_EXT_LOG_D("Enter EthernetDhcpController::EthernetDhcpControllerResultNotify::OnFailed");
}

void EthernetDhcpController::EthernetDhcpControllerResultNotify::OnSerExitNotify(const std::string& ifname)
{
    NETMGR_EXT_LOG_D("EthernetDhcpController::EthernetDhcpControllerResultNotify::OnSerExitNotify");
}

EthernetDhcpController::EthernetDhcpController()
{
    dhcpService_ = std::make_unique<OHOS::Wifi::DhcpService>();
    dhcpResultNotify_ = std::make_unique<EthernetDhcpControllerResultNotify>(*this);
}

EthernetDhcpController::~EthernetDhcpController() {}

int32_t EthernetDhcpController::RegisterDhcpCallback(sptr<EthernetDhcpCallback> callback)
{
    NETMGR_EXT_LOG_D("EthernetDhcpController RegisterDhcpCallback");
    cbObject = callback;
    return 0;
}

void EthernetDhcpController::StartDhcpClient(const std::string &iface, bool bIpv6)
{
    NETMGR_EXT_LOG_D("EthernetDhcpController StartDhcpClient iface[%{public}s] ipv6[%{public}d]", iface.c_str(), bIpv6);
    dhcpService_->StartDhcpClient(iface, bIpv6);
    if (dhcpService_->GetDhcpResult(iface, dhcpResultNotify_.get(), DHCP_TIMEOUT) != 0) {
        NETMGR_EXT_LOG_D(" Dhcp connection failed.\n");
    }
}

void EthernetDhcpController::StopDhcpClient(const std::string &iface, bool bIpv6)
{
    NETMGR_EXT_LOG_D("EthernetDhcpController StopDhcpClient iface[%{public}s] ipv6[%{public}d]", iface.c_str(), bIpv6);
    dhcpService_->StopDhcpClient(iface, bIpv6);
}

void EthernetDhcpController::OnDhcpSuccess(const std::string &iface, OHOS::Wifi::DhcpResult &result)
{
    NETMGR_EXT_LOG_D("EthernetDhcpController OnDhcpSuccess Enter");
    if (cbObject == nullptr) {
        NETMGR_EXT_LOG_E("Error OnDhcpSuccess No Cb!");
        return;
    }
    EthernetDhcpCallback::DhcpResult dhcpResult;
    dhcpResult.iface = iface;
    dhcpResult.ipAddr = result.strYourCli;
    dhcpResult.gateWay = result.strServer;
    dhcpResult.subNet = result.strSubnet;
    dhcpResult.route1 = result.strRouter1;
    dhcpResult.route2 = result.strRouter2;
    dhcpResult.dns1 = result.strDns1;
    dhcpResult.dns2 = result.strDns2;
    cbObject->OnDhcpSuccess(dhcpResult);
}

void EthernetDhcpController::OnDhcpFailed(int status, const std::string &ifname, const std::string &reason)
{
    NETMGR_EXT_LOG_D("EthernetDhcpController OnDhcpFailed");
}
} // namespace NetManagerStandard
} // namespace OHOS