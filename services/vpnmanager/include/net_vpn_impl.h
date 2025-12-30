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
#ifdef SUPPORT_SYSVPN
#include "sysvpn_config.h"
#include "multi_vpn_helper.h"
#endif // SUPPORT_SYSVPN
#include "vpn_config.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr const char *TUN_CARD_NAME = "vpn-tun";

class NetVpnImpl {
public:
    NetVpnImpl(sptr<VpnConfig> config, const std::string &pkg, int32_t userId, std::vector<int32_t> &activeUserIds);
    virtual ~NetVpnImpl() = default;

    virtual bool IsInternalVpn() = 0;
    virtual int32_t SetUp(bool isInternalChannel = false) = 0;
    virtual int32_t Destroy() = 0;
#ifdef SUPPORT_SYSVPN
    virtual int32_t GetVpnCertData(const int32_t certType, std::vector<int8_t> &certData);
    virtual int32_t GetConnectedSysVpnConfig(sptr<SysVpnConfig> &sysVpnConfig);
    virtual int32_t NotifyConnectStage(const std::string &stage, const int32_t &result);
    virtual int32_t GetSysVpnCertUri(const int32_t certType, std::string &certUri);
    virtual bool IsSystemVpn();
#endif // SUPPORT_SYSVPN
    int32_t RegisterConnectStateChangedCb(std::shared_ptr<IVpnConnStateCb> callback);
    virtual void NotifyConnectState(const VpnConnectState &state);

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
#ifdef SUPPORT_SYSVPN
        if (multiVpnInfo_ != nullptr && !multiVpnInfo_->ifName.empty()) {
            return multiVpnInfo_->ifName;
        }
#endif
        return TUN_CARD_NAME;
    }
    std::shared_ptr<IVpnConnStateCb> GetConnectStateChangedCb()
    {
        return connChangedCb_;
    }

    int32_t ResumeUids();

    void SetCallingUid(int32_t uid);
    inline int32_t GetCallingUid() const
    {
        return uid_;
    }
    void SetCallingPid(int32_t pid);
    inline int32_t GetCallingPid() const
    {
        return pid_;
    }
    bool IsAppUidInWhiteList(int32_t callingUid, int32_t appUid);

protected:
    bool UpdateNetLinkInfo();
    bool RegisterNetSupplier(NetConnClient &netConnClientIns, bool isInternalChannel = false);
    void UnregisterNetSupplier(NetConnClient &netConnClientIns);
    bool UpdateNetSupplierInfo(NetConnClient &netConnClientIns, bool isAvailable);
    bool IsGlobalVpn();
    std::string GetVpnIfAddr();

private:
    int32_t SetNetId(const VpnEventType &isLegacy, NetConnClient &netConnClientIns);
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
    std::string ConvertVpnIpv4Address(uint32_t addressIpv4);
    uint32_t GetVpnInterffaceToId(const std::string &ifName);
#ifdef SUPPORT_SYSVPN
    void ProcessUpRules(bool isUp);
public:
    sptr<MultiVpnInfo> multiVpnInfo_ = nullptr;
#endif // SUPPORT_SYSVPN

protected:
    sptr<VpnConfig> vpnConfig_ = nullptr;

private:
    std::string pkgName_;
    int32_t userId_ = -1; // the calling app's user
    std::vector<int32_t> activeUserIds_;
    bool isVpnConnecting_ = false;
    bool isInternalChannel_ = false;

    int32_t netId_ = -1;
    uint32_t netSupplierId_ = 0;
    std::vector<int32_t> beginUids_;
    std::vector<int32_t> endUids_;
    std::shared_ptr<IVpnConnStateCb> connChangedCb_;
    sptr<NetSupplierInfo> netSupplierInfo_ = nullptr;
    uint32_t priorityId_ = 0;
    int32_t uid_ = -1;
    int32_t pid_ = -1;

    void SetAllUidRanges();
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_VPN_IMPL_H
