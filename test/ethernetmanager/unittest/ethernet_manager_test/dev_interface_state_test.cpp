/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "gtest/gtest-message.h"
#include "gtest/gtest-test-part.h"
#include "gtest/hwext/gtest-ext.h"
#include "gtest/hwext/gtest-tag.h"
#define private public
#include "dev_interface_state.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr const char *DEVICE_NAME = "ahaha";
} // namespace

class DevInterfaceStateTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void DevInterfaceStateTest::SetUpTestCase() {}

void DevInterfaceStateTest::TearDownTestCase() {}

void DevInterfaceStateTest::SetUp() {}

void DevInterfaceStateTest::TearDown() {}

HWTEST_F(DevInterfaceStateTest, DevInterfaceStateTest001, TestSize.Level1)
{
    DevInterfaceState devInterfaceState;
    devInterfaceState.SetDevName(DEVICE_NAME);
    std::set<NetCap> netCaps;
    netCaps.insert(NET_CAPABILITY_MMS);
    devInterfaceState.SetNetCaps(netCaps);
    devInterfaceState.SetLinkUp(true);
    sptr<NetLinkInfo> linkInfo;
    devInterfaceState.SetlinkInfo(linkInfo);
    devInterfaceState.SetDhcpReqState(true);
    bool getDhcpReqState = devInterfaceState.GetDhcpReqState();
    EXPECT_TRUE(getDhcpReqState);

    StaticConfiguration config;
    devInterfaceState.linkInfo_ = nullptr;
    devInterfaceState.UpdateLinkInfo(config);
    devInterfaceState.linkInfo_ = new (std::nothrow) NetLinkInfo();
    devInterfaceState.UpdateLinkInfo(config);
    std::string devName = devInterfaceState.GetDevName();
    EXPECT_STREQ(devName.c_str(), "ahaha");
    std::set<NetCap> getNetCaps = devInterfaceState.GetNetCaps();
    EXPECT_FALSE(getNetCaps.empty());
    bool linkUp = devInterfaceState.GetLinkUp();
    EXPECT_TRUE(linkUp);
}

HWTEST_F(DevInterfaceStateTest, DevInterfaceStateTest002, TestSize.Level1)
{
    DevInterfaceState devInterfaceState;
    devInterfaceState.ifCfg_ = nullptr;
    IPSetMode ipSetMod = devInterfaceState.GetIPSetMode();
    EXPECT_EQ(ipSetMod, IPSetMode::STATIC);
    devInterfaceState.connLinkState_ = DevInterfaceState::ConnLinkState::UNREGISTERED;
    devInterfaceState.RemoteRegisterNetSupplier();
    devInterfaceState.connLinkState_ = DevInterfaceState::ConnLinkState::REGISTERED;
    devInterfaceState.RemoteUnregisterNetSupplier();
    devInterfaceState.connLinkState_ = DevInterfaceState::ConnLinkState::LINK_UNAVAILABLE;
    devInterfaceState.RemoteUpdateNetLinkInfo();
    devInterfaceState.connLinkState_ = DevInterfaceState::ConnLinkState::LINK_AVAILABLE;
    devInterfaceState.linkInfo_ = nullptr;
    devInterfaceState.RemoteUpdateNetLinkInfo();
    sptr<NetLinkInfo> linkInfo;
    devInterfaceState.linkInfo_ = linkInfo;
    devInterfaceState.RemoteUpdateNetLinkInfo();
    devInterfaceState.connLinkState_ = DevInterfaceState::ConnLinkState::UNREGISTERED;
    devInterfaceState.RemoteUpdateNetSupplierInfo();
    devInterfaceState.connLinkState_ = DevInterfaceState::ConnLinkState::REGISTERED;
    devInterfaceState.netSupplierInfo_ = nullptr;
    devInterfaceState.RemoteUpdateNetSupplierInfo();
    sptr<NetSupplierInfo> netSupplierInfo;
    devInterfaceState.netSupplierInfo_ = netSupplierInfo;
    devInterfaceState.RemoteUpdateNetSupplierInfo();
}

HWTEST_F(DevInterfaceStateTest, SetIfcfgTest002, TestSize.Level1)
{
    DevInterfaceState devInterfaceState;
    devInterfaceState.ifCfg_ = nullptr;
    sptr<InterfaceConfiguration> ifCfg = new (std::nothrow) InterfaceConfiguration();
    ifCfg->mode_ = STATIC;
    devInterfaceState.SetIfcfg(ifCfg);
    EXPECT_NE(devInterfaceState.GetIfcfg(), nullptr);
    ifCfg->mode_ = DHCP;
    devInterfaceState.SetIfcfg(ifCfg);
    devInterfaceState.UpdateLinkInfo();
    EXPECT_EQ(devInterfaceState.GetIfcfg()->mode_, DHCP);
}

HWTEST_F(DevInterfaceStateTest, DevInterfaceStateBranchTest001, TestSize.Level1)
{
    DevInterfaceState devInterfaceState;
    sptr<InterfaceConfiguration> ifCfg = new (std::nothrow) InterfaceConfiguration();
    ifCfg->mode_ = STATIC;
    devInterfaceState.SetLinkUp(true);
    devInterfaceState.UpdateSupplierAvailable();
    devInterfaceState.SetIfcfg(ifCfg);
    EXPECT_TRUE(devInterfaceState.linkUp_);

    IPSetMode mode = devInterfaceState.GetIPSetMode();
    EXPECT_EQ(mode, STATIC);

    devInterfaceState.SetLinkUp(false);
    devInterfaceState.UpdateSupplierAvailable();
    devInterfaceState.SetIfcfg(ifCfg);
    EXPECT_FALSE(devInterfaceState.linkUp_);

    devInterfaceState.RemoteRegisterNetSupplier();
    devInterfaceState.RemoteUnregisterNetSupplier();
    devInterfaceState.RemoteUpdateNetSupplierInfo();
    HttpProxy httpProxy;
    devInterfaceState.UpdateNetHttpProxy(httpProxy);

    sptr<NetLinkInfo> linkInfo = nullptr;
    devInterfaceState.SetlinkInfo(linkInfo);
    devInterfaceState.UpdateNetHttpProxy(httpProxy);
    EXPECT_TRUE(devInterfaceState.linkInfo_ == nullptr);

    std::vector<INetAddr> ipAddrList;
    std::vector<INetAddr> netMaskList;
    devInterfaceState.CreateLocalRoute("", ipAddrList, netMaskList);
    EXPECT_TRUE(devInterfaceState.linkInfo_ == nullptr);

    std::string testString = "";
    auto result = devInterfaceState.GetIpv4Prefix(testString, netMaskList);
    EXPECT_TRUE(result.empty());

    StaticConfiguration config;
    devInterfaceState.UpdateLanLinkInfo(config);

    INetAddr targetNetAddr = {};
    devInterfaceState.GetTargetNetAddrWithSameFamily("", ipAddrList, targetNetAddr);
    devInterfaceState.GetDumpInfo(testString);
    EXPECT_TRUE(result.empty());
}

HWTEST_F(DevInterfaceStateTest, DevInterfaceStateBranchTest002, TestSize.Level1)
{
    DevInterfaceState devInterfaceState;
    devInterfaceState.GetNetCaps();

    sptr<NetLinkInfo> linkInfo = nullptr;
    devInterfaceState.SetlinkInfo(linkInfo);
    StaticConfiguration config;
    devInterfaceState.UpdateLanLinkInfo(config);

    sptr<InterfaceConfiguration> ifCfg = new (std::nothrow) InterfaceConfiguration();
    ifCfg->mode_ = STATIC;
    devInterfaceState.SetLancfg(ifCfg);
    bool ret = devInterfaceState.IsLanIface();
    EXPECT_FALSE(ret);

    ifCfg->mode_ = LAN_STATIC;
    devInterfaceState.SetLancfg(ifCfg);
    ret = devInterfaceState.IsLanIface();
    EXPECT_TRUE(ret);

    ifCfg->mode_ = LAN_DHCP;
    devInterfaceState.SetLancfg(ifCfg);
    ret = devInterfaceState.IsLanIface();
    EXPECT_TRUE(ret);
}

HWTEST_F(DevInterfaceStateTest, IsLanIfaceTest001, TestSize.Level1)
{
    DevInterfaceState devInterfaceState;
    devInterfaceState.ifCfg_ = nullptr;
    bool ret = devInterfaceState.IsLanIface();
    EXPECT_FALSE(ret);
}

HWTEST_F(DevInterfaceStateTest, RemoteRegisterNetSupplierTest001, TestSize.Level0)
{
    DevInterfaceState devInterfaceState;
    devInterfaceState.RemoteRegisterNetSupplier();
    EXPECT_EQ(devInterfaceState.connLinkState_, DevInterfaceState::UNREGISTERED);
    devInterfaceState.RemoteUnregisterNetSupplier();
    EXPECT_EQ(devInterfaceState.connLinkState_, DevInterfaceState::UNREGISTERED);
}

HWTEST_F(DevInterfaceStateTest, RemoteRegisterAndUnregisterNetSupplierTest, TestSize.Level0)
{
    DevInterfaceState devInterfaceState;
    devInterfaceState.netCaps_.clear();
    EXPECT_TRUE(devInterfaceState.netCaps_.empty());

    devInterfaceState.connLinkState_ = DevInterfaceState::ConnLinkState::UNREGISTERED;
    devInterfaceState.RemoteRegisterNetSupplier();

    std::set<NetCap> getNetCaps = devInterfaceState.GetNetCaps();
    EXPECT_FALSE(getNetCaps.empty());
    EXPECT_TRUE(getNetCaps.find(NET_CAPABILITY_INTERNET) != getNetCaps.end());
    devInterfaceState.RemoteUnregisterNetSupplier();
}

HWTEST_F(DevInterfaceStateTest, RemoteUpdateNetLinkInfoTest001, TestSize.Level0)
{
    DevInterfaceState devInterfaceState;
    sptr<NetLinkInfo> linkInfo = new NetLinkInfo();
    devInterfaceState.SetlinkInfo(linkInfo);
    devInterfaceState.RemoteUpdateNetLinkInfo();
    EXPECT_EQ(devInterfaceState.connLinkState_, DevInterfaceState::UNREGISTERED);
    sptr<NetLinkInfo> linkInfo1 = new NetLinkInfo();
    INetAddr ipv6Addr;
    ipv6Addr.family_ = 2;
    linkInfo1->netAddrList_.push_back(ipv6Addr);
    devInterfaceState.SetlinkInfo(linkInfo1);
    devInterfaceState.RemoteUpdateNetLinkInfo();
    EXPECT_EQ(devInterfaceState.connLinkState_, DevInterfaceState::UNREGISTERED);
}

HWTEST_F(DevInterfaceStateTest, UpdateNetHttpProxyTest001, TestSize.Level0)
{
    DevInterfaceState devInterfaceState;
    sptr<InterfaceConfiguration> ifCfg = new (std::nothrow) InterfaceConfiguration();
    ifCfg->httpProxy_.SetUserId(123);
    devInterfaceState.SetIfcfg(ifCfg);
    HttpProxy httpProxy;
    sptr<NetLinkInfo> linkInfo = new NetLinkInfo();
    devInterfaceState.SetlinkInfo(linkInfo);
    httpProxy.SetUserId(1234);
    httpProxy.SetPort(8710);
    devInterfaceState.UpdateNetHttpProxy(httpProxy);
    EXPECT_EQ(devInterfaceState.connLinkState_, DevInterfaceState::UNREGISTERED);

    ifCfg = new (std::nothrow) InterfaceConfiguration();
    ifCfg->httpProxy_.SetUserId(123);
    devInterfaceState.SetIfcfg(ifCfg);
    devInterfaceState.connLinkState_ = DevInterfaceState::ConnLinkState::LINK_AVAILABLE;
    devInterfaceState.UpdateNetHttpProxy(httpProxy);
    EXPECT_NE(devInterfaceState.linkInfo_, nullptr);

    ifCfg = new (std::nothrow) InterfaceConfiguration();
    ifCfg->httpProxy_.SetUserId(123);
    devInterfaceState.SetIfcfg(ifCfg);
    sptr<NetLinkInfo> linkInfo1 = nullptr;
    devInterfaceState.SetlinkInfo(linkInfo1);
    devInterfaceState.UpdateNetHttpProxy(httpProxy);
    EXPECT_EQ(devInterfaceState.linkInfo_, nullptr);
}

HWTEST_F(DevInterfaceStateTest, UpdateLinkInfoTest001, TestSize.Level0)
{
    DevInterfaceState devInterfaceState;
    sptr<InterfaceConfiguration> ifCfg = new (std::nothrow) InterfaceConfiguration();
    ifCfg->mode_ = DHCP;
    devInterfaceState.SetIfcfg(ifCfg);
    devInterfaceState.UpdateLinkInfo();
    EXPECT_NE(devInterfaceState.ifCfg_->mode_, STATIC);
    EXPECT_EQ(devInterfaceState.connLinkState_, DevInterfaceState::UNREGISTERED);
}

HWTEST_F(DevInterfaceStateTest, UpdateLanLinkInfoTest001, TestSize.Level0)
{
    DevInterfaceState devInterfaceState;
    devInterfaceState.ifCfg_ = nullptr;
    bool ret = devInterfaceState.IsLanIface();
    EXPECT_FALSE(ret);
    devInterfaceState.UpdateLanLinkInfo();

    sptr<InterfaceConfiguration> ifCfg = new (std::nothrow) InterfaceConfiguration();
    ifCfg->mode_ = DHCP;
    devInterfaceState.SetIfcfg(ifCfg);
    devInterfaceState.UpdateLanLinkInfo();
    EXPECT_NE(devInterfaceState.ifCfg_->mode_, STATIC);
}

HWTEST_F(DevInterfaceStateTest, UpdateLanLinkInfoTest002, TestSize.Level0)
{
    DevInterfaceState devInterfaceState;
    StaticConfiguration config;
    devInterfaceState.linkInfo_ = nullptr;
    devInterfaceState.UpdateLanLinkInfo(config);
    EXPECT_EQ(devInterfaceState.connLinkState_, DevInterfaceState::UNREGISTERED);
}

HWTEST_F(DevInterfaceStateTest, UpdateLinkInfoTest002, TestSize.Level0)
{
    DevInterfaceState devInterfaceState;
    StaticConfiguration config;
    sptr<NetLinkInfo> linkInfo = new NetLinkInfo();
    devInterfaceState.SetlinkInfo(linkInfo);
    sptr<InterfaceConfiguration> ifCfg = new (std::nothrow) InterfaceConfiguration();
    ifCfg->httpProxy_.SetUserId(123);
    devInterfaceState.SetIfcfg(ifCfg);
    devInterfaceState.UpdateLinkInfo(config);
    EXPECT_EQ(devInterfaceState.connLinkState_, DevInterfaceState::UNREGISTERED);
}

HWTEST_F(DevInterfaceStateTest, UpdateLinkInfoTest003, TestSize.Level0)
{
    DevInterfaceState devInterfaceState;
    sptr<InterfaceConfiguration> ifCfg = new (std::nothrow) InterfaceConfiguration();
    ifCfg->mode_ = DHCP;
    EXPECT_TRUE(ifCfg->ipStatic_.dnsServers_.empty());
    NetManagerStandard::INetAddr Emptydns;
    Emptydns.type_ = NetManagerStandard::INetAddr::IPV4;
    Emptydns.address_ = "";
    ifCfg->ipStatic_.dnsServers_.push_back(Emptydns);
    devInterfaceState.SetIfcfg(ifCfg);
    devInterfaceState.UpdateLinkInfo();
    EXPECT_FALSE(ifCfg->ipStatic_.dnsServers_.empty());
}

HWTEST_F(DevInterfaceStateTest, UpdateLinkInfoTest004, TestSize.Level0)
{
    DevInterfaceState devInterfaceState;
    StaticConfiguration config = StaticConfiguration();
    sptr<NetLinkInfo> linkInfo = new NetLinkInfo();
    devInterfaceState.SetlinkInfo(linkInfo);
    sptr<InterfaceConfiguration> ifCfg = new (std::nothrow) InterfaceConfiguration();
    EXPECT_TRUE(config.dnsServers_.empty());
    NetManagerStandard::INetAddr Emptydns;
    Emptydns.type_ = NetManagerStandard::INetAddr::IPV4;
    Emptydns.address_ = "";
    config.dnsServers_.push_back(Emptydns);
    devInterfaceState.UpdateLinkInfo(config);
    EXPECT_FALSE(config.dnsServers_.empty());
}

HWTEST_F(DevInterfaceStateTest, GetRoutePrefixlenTest001, TestSize.Level0)
{
    DevInterfaceState devInterfaceState;
    std::string bySrcAddr = "123";
    INetAddr inetaddr;
    inetaddr.address_ = "192.168.1.1";
    std::vector<INetAddr> fromAddrList = {inetaddr};
    INetAddr targetNetAddr;
    devInterfaceState.GetRoutePrefixlen(bySrcAddr, fromAddrList, targetNetAddr);
    bySrcAddr = "192.168.1.1";
    devInterfaceState.GetRoutePrefixlen(bySrcAddr, fromAddrList, targetNetAddr);
    EXPECT_NE(targetNetAddr.prefixlen_, 0);
}

HWTEST_F(DevInterfaceStateTest, GetDumpInfoTest001, TestSize.Level0)
{
    DevInterfaceState devInterfaceState;
    sptr<NetLinkInfo> linkInfo = new NetLinkInfo();
    devInterfaceState.SetlinkInfo(linkInfo);
    devInterfaceState.netSupplierInfo_ = nullptr;
    devInterfaceState.ifCfg_ = nullptr;
    std::string info = "";
    devInterfaceState.GetDumpInfo(info);
    sptr<InterfaceConfiguration> ifCfg = new (std::nothrow) InterfaceConfiguration();
    ifCfg->mode_ = DHCP;
    devInterfaceState.SetIfcfg(ifCfg);
    devInterfaceState.GetDumpInfo(info);
    EXPECT_NE(info, "");
}

HWTEST_F(DevInterfaceStateTest, GetIpType001, TestSize.Level0)
{
    DevInterfaceState devInterfaceState;
    std::string ip1 = "192.168.1.1";
    EXPECT_EQ(devInterfaceState.GetIpType(ip1), 1);
    std::string ip2 = "::1";
    EXPECT_EQ(devInterfaceState.GetIpType(ip2), 2);
    std::string ip3 = "wrong ip";
    EXPECT_EQ(devInterfaceState.GetIpType(ip3), 0);
}

HWTEST_F(DevInterfaceStateTest, CreateDefaultRouteTest001, TestSize.Level0)
{
    DevInterfaceState devInterfaceState;
    sptr<NetLinkInfo> linkInfo = new NetLinkInfo();
    devInterfaceState.SetlinkInfo(linkInfo);
    std::vector<NetManagerStandard::INetAddr> gatewayList;
    devInterfaceState.CreateDefaultRoute(gatewayList, true, true);
    EXPECT_TRUE(linkInfo->routeList_.empty());
}

HWTEST_F(DevInterfaceStateTest, CreateDefaultRouteTest002, TestSize.Level0)
{
    DevInterfaceState devInterfaceState;
    sptr<NetLinkInfo> linkInfo = new NetLinkInfo();
    devInterfaceState.SetlinkInfo(linkInfo);
    std::vector<NetManagerStandard::INetAddr> gatewayList;
    NetManagerStandard::INetAddr router1, router2;
    router1.address_ = "0.0.0.0";
    router2.address_ = "::";
    gatewayList.push_back(router1);
    gatewayList.push_back(router2);
    devInterfaceState.CreateDefaultRoute(gatewayList, true, true);
    EXPECT_FALSE(linkInfo->routeList_.empty());

    linkInfo->routeList_.clear();
    devInterfaceState.CreateDefaultRoute(gatewayList, true, false);
    EXPECT_FALSE(linkInfo->routeList_.empty());

    linkInfo->routeList_.clear();
    devInterfaceState.CreateDefaultRoute(gatewayList, false, true);
    EXPECT_FALSE(linkInfo->routeList_.empty());

    linkInfo->routeList_.clear();
    devInterfaceState.CreateDefaultRoute(gatewayList, false, false);
    EXPECT_TRUE(linkInfo->routeList_.empty());
}
} // namespace NetManagerStandard
} // namespace OHOS