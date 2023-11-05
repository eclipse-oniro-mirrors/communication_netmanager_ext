/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef DEV_INTERFACE_STATE_H
#define DEV_INTERFACE_STATE_H

#include <cstdint>
#include <functional>
#include <iosfwd>
#include <set>
#include <string>
#include <vector>

#include "http_proxy.h"
#include "interface_configuration.h"
#include "net_all_capabilities.h"
#include "net_link_info.h"
#include "net_specifier.h"
#include "net_supplier_info.h"
#include "refbase.h"

namespace OHOS {
namespace NetManagerStandard {
class DevInterfaceState : public virtual RefBase {
public:
    DevInterfaceState();
    ~DevInterfaceState() = default;
    void SetDevName(const std::string &devName);
    void SetNetCaps(const std::set<NetCap> &netCaps);
    void SetLinkUp(bool up);
    void SetlinkInfo(sptr<NetLinkInfo> &linkInfo);
    void SetIfcfg(sptr<InterfaceConfiguration> &ifCfg);
    void SetLancfg(sptr<InterfaceConfiguration> &ifCfg);
    void SetDhcpReqState(bool dhcpReqState);
    void UpdateNetHttpProxy(const HttpProxy &httpProxy);
    void UpdateLinkInfo(const sptr<StaticConfiguration> &config);
    std::string GetDevName() const;
    const std::set<NetCap> &GetNetCaps() const;
    std::set<NetCap> GetNetCaps();
    bool GetLinkUp() const;
    sptr<NetLinkInfo> GetLinkInfo() const;
    sptr<InterfaceConfiguration> GetIfcfg() const;
    IPSetMode GetIPSetMode() const;
    bool GetDhcpReqState() const;
    bool IsLanIface();
    void UpdateLanLinkInfo();
    void UpdateLanLinkInfo(const sptr<StaticConfiguration> &config);

    void RemoteRegisterNetSupplier();
    void RemoteUnregisterNetSupplier();
    void RemoteUpdateNetLinkInfo();
    void RemoteUpdateNetSupplierInfo();

    void GetDumpInfo(std::string &info);

private:
    enum ConnLinkState { REGISTERED, UNREGISTERED, LINK_AVAILABLE, LINK_UNAVAILABLE };
    void UpdateLinkInfo();
    void UpdateSupplierAvailable();
    void CreateLocalRoute(const std::string &iface, const std::vector<INetAddr> &ipAddrList,
                          const std::vector<INetAddr> &netMaskList);
    std::string GetIpv4Prefix(const std::string &ipv4Addr, const std::vector<INetAddr> &netMaskList);
    void GetTargetNetAddrWithSameFamily(const std::string &bySrcAddr, const std::vector<INetAddr> &fromAddrList,
                                        INetAddr &targetNetAddr);
    void GetRoutePrefixlen(const std::string &bySrcAddr, const std::vector<INetAddr> &fromAddrList,
                           INetAddr &targetNetAddr);

private:
    ConnLinkState connLinkState_ = UNREGISTERED;
    uint32_t netSupplier_ = 0;
    std::string devName_;
    bool linkUp_ = false;
    bool dhcpReqState_ = false;
    sptr<NetLinkInfo> linkInfo_ = nullptr;
    sptr<NetSupplierInfo> netSupplierInfo_ = nullptr;
    sptr<InterfaceConfiguration> ifCfg_ = nullptr;
    std::set<NetCap> netCaps_;
    NetBearType bearerType_ = BEARER_ETHERNET;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // DEV_INTERFACE_STATE_H
