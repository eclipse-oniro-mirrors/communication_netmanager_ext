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

    sptr<StaticConfiguration> config = nullptr;
    devInterfaceState.UpdateLinkInfo(config);
    config = new (std::nothrow) StaticConfiguration();
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
    sptr<NetLinkInfo> getLinkInfo = devInterfaceState.GetLinkInfo();
    EXPECT_NE(getLinkInfo, nullptr);
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

    sptr<StaticConfiguration> config = nullptr;
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
    sptr<StaticConfiguration> config = new (std::nothrow) StaticConfiguration();
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
}
} // namespace NetManagerStandard
} // namespace OHOS