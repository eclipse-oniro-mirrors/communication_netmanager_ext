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

#ifndef VIRTUAL_VPN_CTL_H
#define VIRTUAL_VPN_CTL_H

#include <cstdint>

#include "net_vpn_impl.h"
#include "net_supplier_info.h"

namespace OHOS {
namespace NetManagerStandard {
class VirtualVpnCtl : public NetVpnImpl {
public:
    VirtualVpnCtl(sptr<VpnConfig> config, const std::string &pkg, int32_t userId, std::vector<int32_t> &activeUserIds);
    ~VirtualVpnCtl() = default;
    
    bool IsInternalVpn() override;
    int32_t SetUp(bool isInternalChannel = false) override;
    void NotifyConnectState(const VpnConnectState &state) override;
    int32_t Destroy() override;

private:
    /*
     * Send COMMON_EVENT_CONNECTIVITY_CHANGE by ourself;
     * When setup vpn, COMMON_EVENT_CONNECTIVITY_CHANGE will be sent by Network class when update netlink info,
     * but virtual vpn do not have any interface and do not update netlink info, so we should send
     * COMMON_EVENT_CONNECTIVITY_CHANGE ourself.
     */
    void SendConnectionChangedBroadcast(const NetConnState &netConnState);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // VIRTUAL_VPN_CTL_H
