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

#include "ethernet_client.h"
#include "gtest/gtest-message.h"
#include "gtest/gtest-test-part.h"
#include "gtest/hwext/gtest-ext.h"
#include "gtest/hwext/gtest-tag.h"
#include "http_proxy.h"
#include "interface_configuration.h"
#include "interface_type.h"
#include "net_manager_constants.h"
#include "netmanager_ext_test_security.h"
#include "netmgr_ext_log_wrapper.h"
#include "refbase.h"
#include "static_configuration.h"

#define private public
#define protected public
#include "ethernet_client.h"
#include "ethernet_dhcp_controller.h"
#include "ethernet_service.h"
#include "ethernet_management.h"
#include "ethernet_service_proxy.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr const char *DEV_NAME = "eth0";
constexpr const char *TEST_PROXY_HOST = "127.0.0.1";
constexpr uint16_t TEST_PROXY_PORT = 8080;
}

class EtherNetServiceProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    sptr<InterfaceConfiguration> GetIfaceConfig();
    void SetUp();
    void TearDown();
};

sptr<InterfaceConfiguration> EtherNetServiceProxyTest::GetIfaceConfig()
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
    ic->httpProxy_ = {TEST_PROXY_HOST, TEST_PROXY_PORT, {}};
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

void EtherNetServiceProxyTest::SetUpTestCase() {}

void EtherNetServiceProxyTest::TearDownTestCase() {}

void EtherNetServiceProxyTest::SetUp() {}

void EtherNetServiceProxyTest::TearDown() {}

HWTEST_F(EtherNetServiceProxyTest, SetIfaceConfigTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    EthernetServiceProxy ethernetServiceProxy(nullptr);
    sptr<InterfaceConfiguration> ic = GetIfaceConfig();
    auto ret = ethernetServiceProxy.SetIfaceConfig(DEV_NAME, ic);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL);
}

HWTEST_F(EtherNetServiceProxyTest, GetIfaceConfigTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    EthernetServiceProxy ethernetServiceProxy(nullptr);
    sptr<InterfaceConfiguration> ifaceConfig = new (std::nothrow) InterfaceConfiguration();
    int32_t ret = ethernetServiceProxy.GetIfaceConfig(DEV_NAME, ifaceConfig);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL);
}

HWTEST_F(EtherNetServiceProxyTest, IsIfaceActiveTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    EthernetServiceProxy ethernetServiceProxy(nullptr);
    sptr<InterfaceConfiguration> ifaceConfig = new (std::nothrow) InterfaceConfiguration();
    std::string ifcaeName = "eth0";
    int32_t activeStatus = -1;
    int32_t ret = ethernetServiceProxy.IsIfaceActive(ifcaeName, activeStatus);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL);
}

HWTEST_F(EtherNetServiceProxyTest, GetAllActiveIfacesTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    EthernetServiceProxy ethernetServiceProxy(nullptr);
    std::vector<std::string> result;
    int32_t ret = ethernetServiceProxy.GetAllActiveIfaces(result);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL);
}

HWTEST_F(EtherNetServiceProxyTest, ResetFactoryTest001, TestSize.Level1)
{
    EthernetServiceProxy ethernetServiceProxy(nullptr);
    int32_t ret = ethernetServiceProxy.ResetFactory();
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL);
}

HWTEST_F(EtherNetServiceProxyTest, SetInterfaceUpTest001, TestSize.Level1)
{
    EthernetServiceProxy ethernetServiceProxy(nullptr);
    NetManagerExtAccessToken token;
    int32_t ret = ethernetServiceProxy.SetInterfaceUp(DEV_NAME);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL);
    OHOS::nmd::InterfaceConfigurationParcel cfg;
    ret = ethernetServiceProxy.GetInterfaceConfig(DEV_NAME, cfg);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL);
}

HWTEST_F(EtherNetServiceProxyTest, SetInterfaceDownTest001, TestSize.Level1)
{
    EthernetServiceProxy ethernetServiceProxy(nullptr);
    NetManagerExtAccessToken token;
    int32_t ret = ethernetServiceProxy.SetInterfaceDown(DEV_NAME);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL);
}

HWTEST_F(EtherNetServiceProxyTest, SetInterfaceConfigTest001, TestSize.Level1)
{
    EthernetServiceProxy ethernetServiceProxy(nullptr);
    NetManagerExtAccessToken token;
    OHOS::nmd::InterfaceConfigurationParcel config;
    config.ifName = "eth0";
    config.hwAddr = "";
    config.ipv4Addr = "172.17.5.234";
    config.prefixLength = 24;
    config.flags.push_back("up");
    config.flags.push_back("broadcast");
    int32_t ret = ethernetServiceProxy.SetInterfaceConfig(DEV_NAME, config);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL);
}
} // namespace NetManagerStandard
} // namespace OHOS