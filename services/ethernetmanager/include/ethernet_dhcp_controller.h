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

#ifndef ETHERNET_DHCP_CONTROLLER_H
#define ETHERNET_DHCP_CONTROLLER_H

#include <cstdint>
#include <iosfwd>
#include <memory>

#include "ethernet_dhcp_callback.h"
#include "refbase.h"
#include "dhcp_c_api.h"

namespace OHOS {
namespace NetManagerStandard {
class EthernetDhcpController {
public:
    class EthernetDhcpControllerResultNotify {
    public:
        EthernetDhcpControllerResultNotify()
        {
        }
        ~EthernetDhcpControllerResultNotify()  = default;
        static void OnSuccess(int status, const char *ifname, DhcpResult *result);
        static void OnFailed(int status, const char *ifname, const char *reason);
        static void SetEthernetDhcpController(EthernetDhcpController *ethDhcpController);
    private:
        static EthernetDhcpController *ethDhcpController_;
    };
public:
    EthernetDhcpController() : dhcpResultNotify_(std::make_unique<EthernetDhcpControllerResultNotify>())
    {
        clientEvent.OnIpSuccessChanged = EthernetDhcpControllerResultNotify::OnSuccess;
        clientEvent.OnIpFailChanged = EthernetDhcpControllerResultNotify::OnFailed;
    }
    ~EthernetDhcpController() = default;
    void RegisterDhcpCallback(sptr<EthernetDhcpCallback> callback);
    void StartClient(const std::string &iface, bool bIpv6);
    void StopClient(const std::string &iface, bool bIpv6);
private:
    void OnDhcpSuccess(const std::string &iface, DhcpResult *result);
    void OnDhcpFailed(int status, const std::string &ifname, const char *reason);
private:
    ClientCallBack clientEvent;
    std::unique_ptr<EthernetDhcpControllerResultNotify> dhcpResultNotify_ = nullptr;
    sptr<EthernetDhcpCallback> cbObject_ = nullptr;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // ETHERNET_DHCP_CONTROLLER_H