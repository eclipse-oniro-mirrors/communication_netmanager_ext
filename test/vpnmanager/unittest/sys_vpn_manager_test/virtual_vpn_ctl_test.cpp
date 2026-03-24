/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <memory>

#include "route.h"
#include <gtest/gtest.h>

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "net_manager_constants.h"
#include "net_manager_center.h"
#include "virtual_vpn_ctl.h"
#include "vpn_state.h"
#include "networkvpn_client.h"
#include "mock_i_network_conn_service.h"
#include "mock_i_net_conn_base_service.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing;
using namespace testing::ext;
}

class VirtualVpnConnStateCbTest : public IVpnConnStateCb {
public:
    VirtualVpnConnStateCbTest() = default;
    virtual ~VirtualVpnConnStateCbTest() = default;
    void OnVpnConnStateChanged(const VpnConnectState &state, const sptr<VpnState> &vpnState) override;
    void SendConnStateChanged(const VpnConnectState &state, int32_t vpnType = 0,
                              const std::string &vpnId = "") override;
    void OnMultiVpnConnStateChanged(const VpnConnectState &state, const std::string &vpnId) override;
};

void VirtualVpnConnStateCbTest::OnVpnConnStateChanged(const VpnConnectState &state, const sptr<VpnState> &vpnState) {}
void VirtualVpnConnStateCbTest::SendConnStateChanged(const VpnConnectState &state, int32_t vpnType,
                                                     const std::string &vpnId) {}
void VirtualVpnConnStateCbTest::OnMultiVpnConnStateChanged(const VpnConnectState &state, const std::string &vpnId) {}

class VirtualVpnCtlTest : public testing::Test {
public:
    static inline std::unique_ptr<VirtualVpnCtl> control_ = nullptr;
    static void SetUpTestSuite();
};

void VirtualVpnCtlTest::SetUpTestSuite()
{
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    if (config == nullptr) {
        return;
    }
    int32_t userId = 0;
    std::vector<int32_t> activeUserIds;
    control_ = std::make_unique<VirtualVpnCtl>(config, "pkg", userId, activeUserIds);
    if (control_ == nullptr) {
        return;
    }
}

HWTEST_F(VirtualVpnCtlTest, SetUp001, TestSize.Level1)
{
    control_->vpnConfig_ = nullptr;
    EXPECT_EQ(control_->SetUp(true), NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(VirtualVpnCtlTest, SetUp002, TestSize.Level1)
{
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    control_->vpnConfig_ = config;
    EXPECT_EQ(control_->SetUp(true), NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(VirtualVpnCtlTest, SetUp003, TestSize.Level1)
{
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    control_->vpnConfig_ = config;

    auto mockNetConnService = sptr<MockINetConnService>::MakeSptr();
    auto mockNetConnBaseService = sptr<MockNetConnBaseService>::MakeSptr();
    auto &netConnClientIns = NetConnClient::GetInstance();
    auto &netManagerCenter = NetManagerCenter::GetInstance();

    auto oldNetConnService = netConnClientIns.NetConnService_;
    auto oldNetConnBaseService = netManagerCenter.connService_;

    netConnClientIns.NetConnService_ = mockNetConnService;
    netManagerCenter.connService_ = mockNetConnBaseService;

    EXPECT_CALL(*mockNetConnService, GetNetIdByIdentifier(_, _)).
        WillRepeatedly([](const std::string &ident, std::list<int32_t> &netIdList) {
                            netIdList.push_back(123);
                            return 0;
                          });
    
    EXPECT_CALL(*mockNetConnService, RegisterNetSupplier(_, _, _, _)).
        WillRepeatedly([](NetBearType bearerType, const std::string &ident,
                          const std::set<NetCap> &netCaps, uint32_t &supplierId) {
                            supplierId = 1;
                            return 0;
                          });

    EXPECT_CALL(*mockNetConnBaseService, UpdateNetLinkInfo(_, _)).WillRepeatedly(testing::Return(0));
    control_->netSupplierId_ = 0;
    EXPECT_EQ(control_->SetUp(true), NETMANAGER_EXT_ERR_INTERNAL);

    EXPECT_CALL(*mockNetConnBaseService, UpdateNetLinkInfo(_, _)).WillRepeatedly(testing::Return(-1));
    control_->netSupplierId_ = 0;
    EXPECT_EQ(control_->SetUp(true), NETMANAGER_EXT_ERR_INTERNAL);

    EXPECT_CALL(*mockNetConnBaseService, UpdateNetLinkInfo(_, _)).WillRepeatedly(testing::Return(0));
    EXPECT_CALL(*mockNetConnService, GetNetIdByIdentifier(_, _)).
        WillRepeatedly([](const std::string &ident, std::list<int32_t> &netIdList) {
                            return 0;
                          });
    control_->netSupplierId_ = 0;
    EXPECT_EQ(control_->SetUp(true), NETMANAGER_EXT_ERR_INTERNAL);

    EXPECT_CALL(*mockNetConnService, GetNetIdByIdentifier(_, _)).
        WillRepeatedly([](const std::string &ident, std::list<int32_t> &netIdList) {
                            netIdList.push_back(123);
                            return 0;
                          });
    control_->netSupplierId_ = 0;
    EXPECT_EQ(control_->SetUp(true), NETMANAGER_EXT_ERR_INTERNAL);

    control_->netSupplierId_ = 0;
    netConnClientIns.NetConnService_ = oldNetConnService;
    netManagerCenter.connService_ = oldNetConnBaseService;
}

HWTEST_F(VirtualVpnCtlTest, Destroy001, TestSize.Level1)
{
    EXPECT_EQ(control_->Destroy(), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(VirtualVpnCtlTest, NotifyConnectState001, TestSize.Level1)
{
    control_->connChangedCb_ = nullptr;
    control_->NotifyConnectState(VpnConnectState::VPN_CONNECTED);
    EXPECT_EQ(control_->netSupplierId_, 0);
}

HWTEST_F(VirtualVpnCtlTest, NotifyConnectState002, TestSize.Level1)
{
    std::shared_ptr<IVpnConnStateCb> cb = std::make_shared<VirtualVpnConnStateCbTest>();
    control_->connChangedCb_ = cb;
#ifdef SUPPORT_SYSVPN
    control_->multiVpnInfo_ = nullptr;
#endif
    control_->NotifyConnectState(VpnConnectState::VPN_CONNECTED);
    EXPECT_EQ(control_->netSupplierId_, 0);
}

HWTEST_F(VirtualVpnCtlTest, NotifyConnectState003, TestSize.Level1)
{
    std::shared_ptr<IVpnConnStateCb> cb = std::make_shared<VirtualVpnConnStateCbTest>();
    control_->connChangedCb_ = cb;
#ifdef SUPPORT_SYSVPN
    sptr<MultiVpnInfo> vpnInfo = new (std::nothrow) MultiVpnInfo();
    vpnInfo->vpnId = "test";
    control_->multiVpnInfo_ = vpnInfo;
#endif
    control_->NotifyConnectState(VpnConnectState::VPN_CONNECTED);
    EXPECT_EQ(control_->netSupplierId_, 0);

    control_->SendConnectionChangedBroadcast(NET_CONN_STATE_CONNECTED);
    control_->SendConnectionChangedBroadcast(NET_CONN_STATE_DISCONNECTED);
}

HWTEST_F(VirtualVpnCtlTest, GenerateAllowedUids001, TestSize.Level1)
{
    control_->GenerateAllowedUids();
    EXPECT_EQ(control_->beginUids_.size() > 0, true);
    EXPECT_EQ(control_->endUids_.size() > 0, true);
}

HWTEST_F(VirtualVpnCtlTest, UpdateDnsServers001, TestSize.Level1)
{
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    config->isAcceptIPv4_ = true;
    config->dnsAddresses_.push_back("8.8.8.8");
    control_->vpnConfig_ = config;
    bool ret = control_->UpdateDnsServers();
    EXPECT_EQ(ret, false);
    config->isAcceptIPv4_ = false;
    ret = control_->UpdateDnsServers();
    EXPECT_EQ(ret, false);
}

HWTEST_F(VirtualVpnCtlTest, VpnStateTest, TestSize.Level1)
{
    std::vector<Route> routes;
    std::vector<std::string> dnsServers;
    dnsServers.push_back("8.8.8.8");
    routes.push_back(Route());

    Parcel parcel;
    VpnState state("vpn-tun", "1.1.1.1", "virtual-vpn", true, routes, dnsServers);
    bool ret = state.Marshalling(parcel);
    EXPECT_EQ(ret, true);
    VpnState *vpnState = VpnState::Unmarshalling(parcel);
    EXPECT_NE(vpnState, nullptr);
    delete vpnState;

    for (int i = 0; i < 3000; ++i)
    {
        routes.push_back(Route());
    }
    VpnState state2("vpn-tun", "1.1.1.1", "virtual-vpn", true, routes, dnsServers);
    ret = state2.Marshalling(parcel);
    EXPECT_EQ(ret, false);

    routes.clear();
    routes.push_back(Route());

    for (int i = 0; i < 128; ++i)
    {
        dnsServers.push_back("8.8.8.8");
    }
    VpnState state3("vpn-tun", "1.1.1.1", "virtual-vpn", true, routes, dnsServers);
    ret = state3.MarshallingDns(parcel);
    EXPECT_EQ(ret, false);

    Parcel parcel2;
    parcel2.WriteInt32(3000);
    ret = VpnState::UnmarshallingRoute(parcel2, &state3);
    EXPECT_EQ(ret, false);

    Parcel parcel3;
    parcel2.WriteInt32(128);
    ret = VpnState::UnmarshallingDns(parcel2, &state3);
    EXPECT_EQ(ret, false);
}

} // namespace NetManagerStandard
} // namespace OHOS