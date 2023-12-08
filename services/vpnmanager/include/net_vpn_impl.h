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

#ifndef NET_VPN_IMPL_H
#define NET_VPN_IMPL_H

#include <cstdint>
#include <memory>
#include <set>
#include <vector>

#include "bundle_mgr_proxy.h"
#include "i_vpn_conn_state_cb.h"
#include "net_all_capabilities.h"
#include "net_conn_client.h"
#include "net_manager_ext_constants.h"
#include "net_specifier.h"
#include "net_supplier_info.h"
#include "networkvpn_hisysevent.h"
#include "vpn_config.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr const char *TUN_CARD_NAME = "vpn-tun";

class NetVpnImpl {
public:
    NetVpnImpl(sptr<VpnConfig> config, const std::string &pkg, int32_t userId, std::vector<int32_t> &activeUserIds);
    virtual ~NetVpnImpl() = default;

    virtual bool IsInternalVpn() = 0;
    virtual int32_t SetUp() = 0;
    virtual int32_t Destroy() = 0;

    int32_t RegisterConnectStateChangedCb(std::shared_ptr<IVpnConnStateCb> callback);
    void NotifyConnectState(const VpnConnectState &state);

public:
    inline sptr<VpnConfig> GetVpnConfig() const
    {
        return vpnConfig_;
    }
    inline std::string GetVpnPkg() const
    {
        return pkgName_;
    }
    inline int32_t GetUserId() const
    {
        return userId_;
    }
    inline bool IsVpnConnecting() const
    {
        return isVpnConnecting_;
    }
    inline std::string GetInterfaceName() const
    {
        return TUN_CARD_NAME;
    }

    int32_t ResumeUids();

private:
    bool RegisterNetSupplier(NetConnClient &netConnClientIns);
    void UnregisterNetSupplier(NetConnClient &netConnClientIns);
    bool UpdateNetSupplierInfo(NetConnClient &netConnClientIns, bool isAvailable);
    bool UpdateNetLinkInfo(NetConnClient &netConnClientIns);
    void DelNetLinkInfo(NetConnClient &netConnClientIns);
    void AdjustRouteInfo(Route &route);
    void SetIpv4DefaultRoute(Route &ipv4DefaultRoute);
    void SetIpv6DefaultRoute(Route &ipv6DefaultRoute);

    void GenerateUidRangesByAcceptedApps(const std::set<int32_t> &uids, std::vector<int32_t> &beginUids,
                                         std::vector<int32_t> &endUids);
    void GenerateUidRangesByRefusedApps(int32_t userId, const std::set<int32_t> &uids, std::vector<int32_t> &beginUids,
                                        std::vector<int32_t> &endUids);
    std::set<int32_t> GetAppsUids(int32_t userId, const std::vector<std::string> &applications);
    int32_t GenerateUidRanges(int32_t userId, std::vector<int32_t> &beginUids, std::vector<int32_t> &endUids);

protected:
    sptr<VpnConfig> vpnConfig_ = nullptr;

private:
    std::string pkgName_;
    int32_t userId_ = -1; // the calling app's user
    std::vector<int32_t> activeUserIds_;
    bool isVpnConnecting_ = false;

    int32_t netId_ = -1;
    uint32_t netSupplierId_ = 0;
    std::vector<int32_t> beginUids_;
    std::vector<int32_t> endUids_;
    std::shared_ptr<IVpnConnStateCb> connChangedCb_;
    sptr<NetSupplierInfo> netSupplierInfo_ = nullptr;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_VPN_IMPL_H
