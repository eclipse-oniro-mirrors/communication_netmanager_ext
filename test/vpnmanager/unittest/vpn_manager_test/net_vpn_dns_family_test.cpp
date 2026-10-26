/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#ifdef GTEST_API_
#define private public
#define protected public
#endif
#include "net_vpn_impl.h"
#include "net_manager_center.h"
#include "net_manager_constants.h"
#include "netsys_controller.h"
#include "mock_i_network_conn_service.h"
#include "mock_i_net_conn_base_service.h"

namespace OHOS::NetManagerStandard {
namespace {
using namespace testing;
using namespace testing::ext;
constexpr uint16_t TEST_NET_ID = 60125;
constexpr uint32_t TEST_SUPPLIER_ID = 60125;

class DnsFamilyControllerMock {
public:
    MOCK_METHOD(int32_t, SetDns, (uint16_t netId, bool ipv4, bool ipv6));
    MOCK_METHOD(int32_t, AddUids, (int32_t netId));
};

// Enabled only for this fixture, never for unrelated VPN tests.
DnsFamilyControllerMock *g_dnsControllerMock = nullptr;

class DnsFamilyVpn : public NetVpnImpl {
public:
    using NetVpnImpl::NetVpnImpl;
    bool IsInternalVpn() override { return false; }
    int32_t SetUp(bool internal = false) override { return NetVpnImpl::SetUp(internal); }
    int32_t Destroy() override { return NetVpnImpl::Destroy(); }
};
} // namespace

// Linker wrapping keeps fault injection entirely in the test executable. The
// production SetUp and UpdateDnsFamilyCapability implementations are unchanged.
extern "C" int32_t RealSetDns(NetsysController *, uint16_t, bool, bool)
    __asm__("__real__ZN4OHOS18NetManagerStandard16NetsysController29SetNetworkDnsFamilyCapabilityEtbb");
extern "C" int32_t WrapSetDns(NetsysController *, uint16_t, bool, bool)
    __asm__("__wrap__ZN4OHOS18NetManagerStandard16NetsysController29SetNetworkDnsFamilyCapabilityEtbb");
extern "C" int32_t RealAddUids(NetsysController *, int32_t, const std::vector<int32_t> &,
    const std::vector<int32_t> &, uint32_t)
    __asm__("__real__ZN4OHOS18NetManagerStandard16NetsysController14NetworkAddUidsEi"
            "RKNSt3__h6vectorIiNS2_9allocatorIiEEEES8_j");
extern "C" int32_t WrapAddUids(NetsysController *, int32_t, const std::vector<int32_t> &,
    const std::vector<int32_t> &, uint32_t)
    __asm__("__wrap__ZN4OHOS18NetManagerStandard16NetsysController14NetworkAddUidsEi"
            "RKNSt3__h6vectorIiNS2_9allocatorIiEEEES8_j");

int32_t WrapSetDns(NetsysController *controller, uint16_t netId, bool ipv4, bool ipv6)
{
    if (g_dnsControllerMock != nullptr) {
        return g_dnsControllerMock->SetDns(netId, ipv4, ipv6);
    }
    return RealSetDns(controller, netId, ipv4, ipv6);
}

int32_t WrapAddUids(NetsysController *controller, int32_t netId, const std::vector<int32_t> &begin,
    const std::vector<int32_t> &end, uint32_t priority)
{
    if (g_dnsControllerMock != nullptr) {
        return g_dnsControllerMock->AddUids(netId);
    }
    return RealAddUids(controller, netId, begin, end, priority);
}

class NetVpnDnsFamilyTest : public testing::Test {
public:
    void SetUp() override
    {
        config = new VpnConfig();
        config->isAcceptIPv4_ = true;
        config->isAcceptIPv6_ = true;
        vpn = std::make_unique<DnsFamilyVpn>(config, "dns-family-test", 0, activeUsers);
        savedConn = NetConnClient::GetInstance().NetConnService_;
        savedBase = NetManagerCenter::GetInstance().connService_;
        conn = new StrictMock<MockINetConnService>();
        base = new StrictMock<MockNetConnBaseService>();
        NetConnClient::GetInstance().NetConnService_ = conn;
        NetManagerCenter::GetInstance().connService_ = base;
        g_dnsControllerMock = &controller;
    }

    void TearDown() override
    {
        g_dnsControllerMock = nullptr;
        NetConnClient::GetInstance().NetConnService_ = savedConn;
        NetManagerCenter::GetInstance().connService_ = savedBase;
    }

    sptr<NetLinkInfo> MakeLink(bool address4, bool address6, bool route4, bool route6)
    {
        sptr<NetLinkInfo> link = new NetLinkInfo();
        for (const auto &address : {address4 ? "10.0.0.2" : "invalid-v4", address6 ? "2001:db8::2" : "invalid-v6"}) {
            INetAddr item;
            item.address_ = address;
            link->netAddrList_.push_back(item);
        }
        for (const auto &destination : {route4 ? "0.0.0.0" : "invalid-v4", route6 ? "::" : "invalid-v6"}) {
            Route route;
            route.destination_.address_ = destination;
            route.rtnType_ = RTN_UNICAST;
            link->routeList_.push_back(route);
        }
        return link;
    }

    void ExpectSetUpPrerequisites()
    {
        constexpr size_t supplierIdOutputIndex = 3;
        constexpr size_t networkIdsOutputIndex = 1;
        EXPECT_CALL(*conn, RegisterNetSupplier(BEARER_VPN, _, _, _))
            .WillOnce(DoAll(SetArgReferee<supplierIdOutputIndex>(TEST_SUPPLIER_ID),
                Return(NETMANAGER_SUCCESS)));
        EXPECT_CALL(*conn, UpdateNetSupplierInfo(TEST_SUPPLIER_ID, _)).WillOnce(Return(NETMANAGER_SUCCESS));
        EXPECT_CALL(*base, UpdateNetLinkInfo(TEST_SUPPLIER_ID, _)).WillOnce(Return(NETMANAGER_SUCCESS));
        EXPECT_CALL(*conn, GetNetIdByIdentifier(_, _))
            .WillOnce(DoAll(SetArgReferee<networkIdsOutputIndex>(std::list<int32_t>{TEST_NET_ID}),
                Return(NETMANAGER_SUCCESS)));
    }

    std::vector<int32_t> activeUsers;
    sptr<VpnConfig> config;
    std::unique_ptr<DnsFamilyVpn> vpn;
    StrictMock<DnsFamilyControllerMock> controller;
    sptr<StrictMock<MockINetConnService>> conn;
    sptr<StrictMock<MockNetConnBaseService>> base;
    sptr<INetConnService> savedConn;
    sptr<NetConnBaseService> savedBase;
};

HWTEST_F(NetVpnDnsFamilyTest, AcceptanceAddressRouteMatrix, TestSize.Level1)
{
    // All 64 combinations: acceptance, usable address and usable route for each family.
    for (uint32_t bits = 0; bits < 64; ++bits) {
        SCOPED_TRACE(bits);
        config->isAcceptIPv4_ = (bits & 1) != 0;
        config->isAcceptIPv6_ = (bits & 2) != 0;
        auto link = MakeLink(bits & 4, bits & 8, bits & 16, bits & 32);
        vpn->dnsIpv4Enable_ = true;
        vpn->dnsIpv6Enable_ = true;
        vpn->UpdateDnsFamilyCapability(link);
        EXPECT_EQ(vpn->dnsIpv4Enable_, (bits & 21) == 21);
        EXPECT_EQ(vpn->dnsIpv6Enable_, (bits & 42) == 42);
    }
}

HWTEST_F(NetVpnDnsFamilyTest, InitialIpv4CapabilityEnabled, TestSize.Level1)
{
    EXPECT_TRUE(vpn->dnsIpv4Enable_);
    EXPECT_FALSE(vpn->dnsIpv6Enable_);
}

HWTEST_F(NetVpnDnsFamilyTest, EmptyRoutesUseDefaultProxy, TestSize.Level1)
{
    for (uint32_t bits = 0; bits < 16; ++bits) {
        SCOPED_TRACE(bits);
        config->isAcceptIPv4_ = (bits & 1) != 0;
        config->isAcceptIPv6_ = (bits & 2) != 0;
        auto link = MakeLink(bits & 4, bits & 8, false, false);
        link->routeList_.clear();
        vpn->UpdateDnsFamilyCapability(link);
        EXPECT_EQ(vpn->dnsIpv4Enable_, (bits & 5) == 5);
        EXPECT_EQ(vpn->dnsIpv6Enable_, (bits & 10) == 10);
    }
}

HWTEST_F(NetVpnDnsFamilyTest, ExcludedAndNonUnicastRoutes, TestSize.Level1)
{
    for (bool excluded : {false, true}) {
        for (int32_t type : {RTN_UNICAST, RTN_UNREACHABLE, RTN_THROW}) {
            SCOPED_TRACE(type);
            SCOPED_TRACE(excluded);
            auto link = MakeLink(true, true, true, true);
            for (auto &route : link->routeList_) {
                route.isExcludedRoute_ = excluded;
                route.rtnType_ = type;
            }
            vpn->UpdateDnsFamilyCapability(link);
            EXPECT_EQ(vpn->dnsIpv4Enable_, !excluded && type == RTN_UNICAST);
            EXPECT_EQ(vpn->dnsIpv6Enable_, !excluded && type == RTN_UNICAST);
        }
    }
}

HWTEST_F(NetVpnDnsFamilyTest, MissingInputsClearPreviousCapability, TestSize.Level1)
{
    auto link = MakeLink(true, true, true, true);
    vpn->UpdateDnsFamilyCapability(link);
    ASSERT_TRUE(vpn->dnsIpv4Enable_);
    ASSERT_TRUE(vpn->dnsIpv6Enable_);
    vpn->UpdateDnsFamilyCapability(nullptr);
    EXPECT_FALSE(vpn->dnsIpv4Enable_);
    EXPECT_FALSE(vpn->dnsIpv6Enable_);
    vpn->UpdateDnsFamilyCapability(link);
    vpn->vpnConfig_ = nullptr;
    vpn->UpdateDnsFamilyCapability(link);
    EXPECT_FALSE(vpn->dnsIpv4Enable_);
    EXPECT_FALSE(vpn->dnsIpv6Enable_);
    vpn->vpnConfig_ = config;
    vpn->UpdateDnsFamilyCapability(new NetLinkInfo());
    EXPECT_FALSE(vpn->dnsIpv4Enable_);
    EXPECT_FALSE(vpn->dnsIpv6Enable_);
}

HWTEST_F(NetVpnDnsFamilyTest, SetUpDnsFailureContinuesUidInstallation, TestSize.Level1)
{
    auto link = MakeLink(true, true, true, true);
    config->addresses_.assign(link->netAddrList_.begin(), link->netAddrList_.end());
    ExpectSetUpPrerequisites();
    InSequence sequence;
    EXPECT_CALL(controller, SetDns(TEST_NET_ID, true, true)).WillOnce(Return(-ENOENT));
    EXPECT_CALL(controller, AddUids(TEST_NET_ID)).WillOnce(Return(NETMANAGER_SUCCESS));
    EXPECT_EQ(vpn->SetUp(), NETMANAGER_EXT_SUCCESS);
    EXPECT_TRUE(vpn->IsVpnConnecting());
}

HWTEST_F(NetVpnDnsFamilyTest, SetUpDnsFailureDoesNotIgnoreUidFailure, TestSize.Level1)
{
    auto link = MakeLink(true, true, true, true);
    config->addresses_.assign(link->netAddrList_.begin(), link->netAddrList_.end());
    ExpectSetUpPrerequisites();
    InSequence sequence;
    EXPECT_CALL(controller, SetDns(TEST_NET_ID, true, true)).WillOnce(Return(-ENOENT));
    EXPECT_CALL(controller, AddUids(TEST_NET_ID)).WillOnce(Return(-EINVAL));
    EXPECT_EQ(vpn->SetUp(), NETMANAGER_EXT_ERR_INTERNAL);
    EXPECT_FALSE(vpn->IsVpnConnecting());
}

HWTEST_F(NetVpnDnsFamilyTest, SetUpDnsSuccessPrecedesUidInstallation, TestSize.Level1)
{
    auto link = MakeLink(true, true, true, true);
    config->addresses_.assign(link->netAddrList_.begin(), link->netAddrList_.end());
    ExpectSetUpPrerequisites();
    InSequence sequence;
    EXPECT_CALL(controller, SetDns(TEST_NET_ID, true, true)).WillOnce(Return(NETMANAGER_SUCCESS));
    EXPECT_CALL(controller, AddUids(TEST_NET_ID)).WillOnce(Return(NETMANAGER_SUCCESS));
    EXPECT_EQ(vpn->SetUp(), NETMANAGER_EXT_SUCCESS);
    EXPECT_TRUE(vpn->IsVpnConnecting());
}

HWTEST_F(NetVpnDnsFamilyTest, SetUpIpv4OnlyDoesNotEnableAaaa, TestSize.Level1)
{
    auto link = MakeLink(true, false, true, false);
    config->addresses_.assign(link->netAddrList_.begin(), link->netAddrList_.end());
    ExpectSetUpPrerequisites();
    InSequence sequence;
    EXPECT_CALL(controller, SetDns(TEST_NET_ID, true, false)).WillOnce(Return(NETMANAGER_SUCCESS));
    EXPECT_CALL(controller, AddUids(TEST_NET_ID)).WillOnce(Return(NETMANAGER_SUCCESS));
    EXPECT_EQ(vpn->SetUp(), NETMANAGER_EXT_SUCCESS);
    EXPECT_FALSE(vpn->dnsIpv6Enable_);
}
} // namespace OHOS::NetManagerStandard
