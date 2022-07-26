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

#include "dhcp_service.h"
#include "ethernet_dhcp_callback.h"
#include "i_dhcp_result_notify.h"
#include "i_dhcp_service.h"
#include "refbase.h"

namespace OHOS {
namespace NetManagerStandard {
class EthernetDhcpController {
public:
    class EthernetDhcpControllerResultNotify : public OHOS::Wifi::IDhcpResultNotify {
    public:
        explicit EthernetDhcpControllerResultNotify(EthernetDhcpController &dhcpController);
        ~EthernetDhcpControllerResultNotify() override;
        void OnSuccess(int status, const std::string &ifname, OHOS::Wifi::DhcpResult &result) override;
        void OnFailed(int status, const std::string &ifname, const std::string &reason) override;
        void OnSerExitNotify(const std::string& ifname) override;

    private:
        EthernetDhcpController &ethDhcpController_;
    };
public:
    EthernetDhcpController();
    ~EthernetDhcpController();

    int32_t RegisterDhcpCallback(sptr<EthernetDhcpCallback> callback);
    void StartDhcpClient(const std::string &iface, bool bIpv6);
    void StopDhcpClient(const std::string &iface, bool bIpv6);
private:
    void OnDhcpSuccess(const std::string &iface, OHOS::Wifi::DhcpResult &result);
    void OnDhcpFailed(int status, const std::string &ifname, const std::string &reason);
private:
    std::unique_ptr<OHOS::Wifi::IDhcpService> dhcpService_ = nullptr;
    std::unique_ptr<EthernetDhcpControllerResultNotify> dhcpResultNotify_ = nullptr;
    sptr<EthernetDhcpCallback> cbObject;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // ETHERNET_DHCP_CONTROLLER_H