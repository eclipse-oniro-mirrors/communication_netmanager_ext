/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#include "ethernet_client.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
namespace {
using namespace testing::ext;
constexpr const char *DEV_NAME = "eth0";
constexpr const char *IFACE_NAME = "wlan0";
} // namespace

class EthernetClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    sptr<InterfaceConfiguration> GetIfaceConfig();
    void SetUp();
    void TearDown();
};

sptr<InterfaceConfiguration> EthernetClientTest::GetIfaceConfig()
{
    sptr<InterfaceConfiguration> ic = (std::make_unique<InterfaceConfiguration>()).release();
    if (!ic) {
        return ic;
    }
    INetAddr ipv4Addr;
    ipv4Addr.type_ = INetAddr::IPV4;
    ipv4Addr.family_ = 0x01;
    ipv4Addr.prefixlen_ = 0x01;
    ipv4Addr.address_ = "172.17.5.234";
    ipv4Addr.netMask_ = "255.255.254.0";
    ipv4Addr.hostName_ = "netAddr";
    ic->ipStatic_.ipAddrList_.push_back(ipv4Addr);
    INetAddr route;
    route.type_ = INetAddr::IPV4;
    route.family_ = 0x01;
    route.prefixlen_ = 0x01;
    route.address_ = "0.0.0.0";
    route.netMask_ = "0.0.0.0";
    route.hostName_ = "netAddr";
    ic->ipStatic_.routeList_.push_back(route);
    INetAddr gateway;
    gateway.type_ = INetAddr::IPV4;
    gateway.family_ = 0x01;
    gateway.prefixlen_ = 0x01;
    gateway.address_ = "172.17.4.1";
    gateway.netMask_ = "0.0.0.0";
    gateway.hostName_ = "netAddr";
    ic->ipStatic_.gatewayList_.push_back(gateway);
    INetAddr netMask;
    netMask.type_ = INetAddr::IPV4;
    netMask.family_ = 0x01;
    netMask.address_ = "255.255.255.0";
    netMask.hostName_ = "netAddr";
    ic->ipStatic_.netMaskList_.push_back(netMask);
    INetAddr dns1;
    dns1.type_ = INetAddr::IPV4;
    dns1.family_ = 0x01;
    dns1.address_ = "8.8.8.8";
    dns1.hostName_ = "netAddr";
    INetAddr dns2;
    dns2.type_ = INetAddr::IPV4;
    dns2.family_ = 0x01;
    dns2.address_ = "114.114.114.114";
    dns2.hostName_ = "netAddr";
    ic->ipStatic_.dnsServers_.push_back(dns1);
    ic->ipStatic_.dnsServers_.push_back(dns2);
    return ic;
}

void EthernetClientTest::SetUpTestCase() {}

void EthernetClientTest::TearDownTestCase() {}

void EthernetClientTest::SetUp() {}

void EthernetClientTest::TearDown() {}

/**
 * @tc.name: GetMacAddressTest001
 * @tc.desc: Test GetMacAddress.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetClientTest, GetMacAddressTest001, TestSize.Level1)
{
    auto ethernetClient = DelayedSingleton<EthernetClient>::GetInstance();
    std::vector<MacAddressInfo> macAddrList;
    int32_t ret = ethernetClient->GetMacAddress(macAddrList);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: SetIfaceConfigTest001
 * @tc.desc: Test SetIfaceConfig.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetClientTest, SetIfaceConfigTest001, TestSize.Level1)
{
    auto ethernetClient = DelayedSingleton<EthernetClient>::GetInstance();
    sptr<InterfaceConfiguration> ic = GetIfaceConfig();
    int32_t ret = ethernetClient->SetIfaceConfig(DEV_NAME, ic);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: GetIfaceConfigTest001
 * @tc.desc: Test GetIfaceConfig.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetClientTest, GetIfaceConfigTest001, TestSize.Level1)
{
    auto ethernetClient = DelayedSingleton<EthernetClient>::GetInstance();
    std::string iface = DEV_NAME;
    sptr<InterfaceConfiguration> ifaceConfig = GetIfaceConfig();
    int32_t ret = ethernetClient->GetIfaceConfig(iface, ifaceConfig);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: ResetFactoryTest001
 * @tc.desc: Test ResetFactory.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetClientTest, ResetFactoryTest001, TestSize.Level1)
{
    auto ethernetClient = DelayedSingleton<EthernetClient>::GetInstance();
    int32_t ret = ethernetClient->ResetFactory();
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: SetInterfaceUpTest001
 * @tc.desc: Test SetInterfaceUp.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetClientTest, SetInterfaceUpTest001, TestSize.Level1)
{
    auto ethernetClient = DelayedSingleton<EthernetClient>::GetInstance();
    int32_t ret = ethernetClient->SetInterfaceUp(DEV_NAME);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: SetInterfaceDownTest001
 * @tc.desc: Test SetInterfaceDown.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetClientTest, SetInterfaceDownTest001, TestSize.Level1)
{
    auto ethernetClient = DelayedSingleton<EthernetClient>::GetInstance();
    int32_t ret = ethernetClient->SetInterfaceDown(DEV_NAME);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: SetInterfaceConfigTest001
 * @tc.desc: Test SetInterfaceConfig.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetClientTest, SetInterfaceConfigTest001, TestSize.Level1)
{
    auto ethernetClient = DelayedSingleton<EthernetClient>::GetInstance();
    std::string iface = DEV_NAME;
    OHOS::nmd::InterfaceConfigurationParcel cfg;
    cfg.ifName = "eth0";
    cfg.hwAddr = "";
    cfg.ipv4Addr = "172.17.5.234";
    cfg.prefixLength = 24;
    cfg.flags.push_back("up");
    cfg.flags.push_back("broadcast");
    int32_t ret = ethernetClient->SetInterfaceConfig(iface, cfg);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: GetDeviceInformationTest001
 * @tc.desc: Test GetDeviceInformation.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetClientTest, GetDeviceInformationTest001, TestSize.Level1)
{
    auto ethernetClient = DelayedSingleton<EthernetClient>::GetInstance();
    std::vector<EthernetDeviceInfo> devInfoList;
    int32_t ret = ethernetClient->GetDeviceInformation(devInfoList);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS