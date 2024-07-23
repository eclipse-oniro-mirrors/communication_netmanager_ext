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
#include "ethernet_configuration.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr const char *IFACE = "iface0";
std::string IFACE_NAME = "iface0";
constexpr const char *FILE_PATH = "./BUILD.gn";
constexpr const char *DIR_PATH = "./BUILD.gn";
std::string REAL_PATH = "./BUILD.gn";
std::string FILE_CONTENT = "./BUILD.gn";
constexpr const char *LAN_STATIC_KEY = "LAN_STATIC";
constexpr const char *STATIC_KEY = "STATIC";
constexpr const char *DHCP_KEY = "DHCP";
constexpr const char *LAN_DHCP_KEY = "LAN_DHCP";
constexpr const char *USER_CONFIG_DIR_TEST = "/data/service/el1/public/netmanager/ethernet";
} // namespace

class EthernetConfigurationTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void EthernetConfigurationTest::SetUpTestCase() {}

void EthernetConfigurationTest::TearDownTestCase() {}

void EthernetConfigurationTest::SetUp() {}

void EthernetConfigurationTest::TearDown() {}

HWTEST_F(EthernetConfigurationTest, EthernetConfiguration001, TestSize.Level1)
{
    EthernetConfiguration ethernetConfiguration;
    std::map<std::string, std::set<NetCap>> devCaps;
    std::map<std::string, sptr<InterfaceConfiguration>> devCfgs;
    bool ret = ethernetConfiguration.ReadSystemConfiguration(devCaps, devCfgs);
    EXPECT_TRUE(ret);
    ret = ethernetConfiguration.ReadUserConfiguration(devCfgs);
    EXPECT_TRUE(ret);
    sptr<InterfaceConfiguration> cfg = (std::make_unique<InterfaceConfiguration>()).release();
    ret = ethernetConfiguration.WriteUserConfiguration(IFACE, cfg);
    EXPECT_TRUE(ret);
    ret = ethernetConfiguration.ClearAllUserConfiguration();
    EXPECT_TRUE(ret);
    EthernetDhcpCallback::DhcpResult dhcpResult;
    sptr<StaticConfiguration> config = (std::make_unique<StaticConfiguration>()).release();
    ret = ethernetConfiguration.ConvertToConfiguration(dhcpResult, config);
    EXPECT_FALSE(ret);
    std::string strRet = ethernetConfiguration.ReadJsonFile(FILE_PATH);
    ret = ethernetConfiguration.IsDirExist(DIR_PATH);
    ret = ethernetConfiguration.CreateDir(DIR_PATH);
    ret = ethernetConfiguration.DelDir(DIR_PATH);
    ret = ethernetConfiguration.IsFileExist(FILE_PATH, REAL_PATH);
    EXPECT_FALSE(ret);
    ret = ethernetConfiguration.ReadFile(FILE_PATH, FILE_CONTENT);
    EXPECT_FALSE(ret);
    ret = ethernetConfiguration.WriteFile(FILE_PATH, FILE_CONTENT);
    EXPECT_TRUE(ret);
    ethernetConfiguration.ParserFileConfig(FILE_CONTENT, IFACE_NAME, cfg);
    ethernetConfiguration.GenCfgContent(IFACE, cfg, FILE_CONTENT);
    sptr<InterfaceConfiguration> cfg2 = nullptr;
    ret = ethernetConfiguration.WriteUserConfiguration(IFACE, cfg2);
    EXPECT_FALSE(ret);
}

HWTEST_F(EthernetConfigurationTest, EthernetConfiguration002, TestSize.Level1)
{
    EthernetConfiguration ethernetConfiguration;
    EthernetDhcpCallback::DhcpResult dhcpResult;
    sptr<StaticConfiguration> config = nullptr;
    bool ret = ethernetConfiguration.ConvertToConfiguration(dhcpResult, config);
    EXPECT_FALSE(ret);

    std::string filePath = "";
    std::string fileContent = "";
    sptr<InterfaceConfiguration> configSptr = nullptr;
    ethernetConfiguration.ParserIfaceIpAndRoute(configSptr, filePath);

    std::string iface = "";
    ethernetConfiguration.GenCfgContent(iface, nullptr, fileContent);

    ret = ethernetConfiguration.IsDirExist(filePath);
    EXPECT_FALSE(ret);

    configSptr = ethernetConfiguration.MakeInterfaceConfiguration(nullptr, nullptr);
    EXPECT_TRUE(configSptr == nullptr);

    ret = ethernetConfiguration.DelDir(filePath);
    EXPECT_FALSE(ret);

    ret = ethernetConfiguration.ReadFile(filePath, fileContent);
    EXPECT_FALSE(ret);

    ret = ethernetConfiguration.WriteFile(filePath, fileContent);
    EXPECT_FALSE(ret);

    ret = ethernetConfiguration.IsValidDhcpResult(dhcpResult, config);
    EXPECT_FALSE(ret);
}

HWTEST_F(EthernetConfigurationTest, EthernetConfiguration003, TestSize.Level1)
{
    EthernetConfiguration ethernetConfiguration;
    EthernetDhcpCallback::DhcpResult dhcpResult;
    dhcpResult.ipAddr = "test";
    sptr<StaticConfiguration> config = (std::make_unique<StaticConfiguration>()).release();
    ethernetConfiguration.IsValidDhcpResult(dhcpResult, config);

    std::string result = ethernetConfiguration.GetIfaceMode(IPSetMode::LAN_STATIC);
    EXPECT_TRUE(result == LAN_STATIC_KEY);

    result = ethernetConfiguration.GetIfaceMode(IPSetMode::LAN_DHCP);
    EXPECT_TRUE(result == LAN_DHCP_KEY);

    result = ethernetConfiguration.GetIfaceMode(IPSetMode::STATIC);
    EXPECT_TRUE(result == STATIC_KEY);

    result = ethernetConfiguration.GetIfaceMode(IPSetMode::DHCP);
    EXPECT_TRUE(result == DHCP_KEY);
}

HWTEST_F(EthernetConfigurationTest, EthernetConfiguration004, TestSize.Level1)
{
    EthernetConfiguration ethernetConfiguration;
    std::map<std::string, sptr<InterfaceConfiguration>> devCfgs;
    std::string fileContent = "";
    bool ret = ethernetConfiguration.CreateDir(USER_CONFIG_DIR_TEST);
    ret = ethernetConfiguration.WriteFile(std::string(USER_CONFIG_DIR_TEST) + "/ethernet_user_interfaces.json",
        fileContent);
    ret = ethernetConfiguration.ReadUserConfiguration(devCfgs);
    EXPECT_TRUE(ret);
    ret = ethernetConfiguration.DelDir(USER_CONFIG_DIR_TEST);
    EXPECT_TRUE(ret);
}

HWTEST_F(EthernetConfigurationTest, EthernetConfiguration005, TestSize.Level1)
{
    EthernetConfiguration ethernetConfiguration;
    std::map<std::string, sptr<InterfaceConfiguration>> devCfgs;
    std::string path = "/data/service/el1/public/netmanager/ethernet_test";
    bool ret = ethernetConfiguration.CreateDir(path);
    std::string fileContent = "";
    ret = ethernetConfiguration.WriteFile(path + "ethernet_user_interfaces.json", fileContent);
    ret = ethernetConfiguration.ReadUserConfiguration(devCfgs);
    EXPECT_TRUE(ret);
    ret = ethernetConfiguration.DelDir(path);
    EXPECT_TRUE(ret);
}

HWTEST_F(EthernetConfigurationTest, EthernetConfiguration006, TestSize.Level1)
{
    EthernetConfiguration ethernetConfiguration;
    std::map<std::string, sptr<InterfaceConfiguration>> devCfgs;
    std::string fileContent = "DEVICE=eth0\nBOOTPROTO=LAN_STATIC\n";
    bool ret = ethernetConfiguration.CreateDir(USER_CONFIG_DIR_TEST);
    ret = ethernetConfiguration.WriteFile(std::string(USER_CONFIG_DIR_TEST) + "/ethernet_user_interfaces.json",
        fileContent);
    ret = ethernetConfiguration.ReadUserConfiguration(devCfgs);
    fileContent = "DEVICE=eth0\nBOOTPROTO=LAN_DHCP\n";
    ret = ethernetConfiguration.WriteFile(std::string(USER_CONFIG_DIR_TEST) + "/ethernet_user_interfaces.json",
        fileContent);
    ret = ethernetConfiguration.ReadUserConfiguration(devCfgs);
    fileContent = "DEVICE=eth0\nBOOTPROTO=STATIC\n";
    ret = ethernetConfiguration.WriteFile(std::string(USER_CONFIG_DIR_TEST) + "/ethernet_user_interfaces.json",
        fileContent);
    ret = ethernetConfiguration.ReadUserConfiguration(devCfgs);
    fileContent = "DEVICE=eth0\nBOOTPROTO=DHCP\n";
    ret = ethernetConfiguration.WriteFile(std::string(USER_CONFIG_DIR_TEST) + "/ethernet_user_interfaces.json",
        fileContent);
    ret = ethernetConfiguration.ReadUserConfiguration(devCfgs);
    EXPECT_TRUE(ret);
    ret = ethernetConfiguration.DelDir(USER_CONFIG_DIR_TEST);
    EXPECT_TRUE(ret);
}

HWTEST_F(EthernetConfigurationTest, EthernetConfiguration007, TestSize.Level1)
{
    EthernetConfiguration ethernetConfiguration;
    std::map<std::string, sptr<InterfaceConfiguration>> devCfgs;
    std::string fileContent =
        "DEVICE=eth0\n\
        BOOTPROTO=STATIC\n\
        IPADDR=123456\n\
        NETMASK=123456\n\
        GATEWAY=123456\n\
        ROUTE=123456\n\
        ROUTE_NETMASK=123456\n\
        DNS=123456\n\
        PROXY_HOST=123456\n\
        PROXY_PORT=123456\n\
        PROXY_EXCLUSIONS=\"127.0.0.1\", \"127.0.0.1\", \"127.0.0.1\"\n";
    bool ret = ethernetConfiguration.CreateDir(USER_CONFIG_DIR_TEST);
    ret = ethernetConfiguration.WriteFile(std::string(USER_CONFIG_DIR_TEST) + "/ethernet_user_interfaces.json",
        fileContent);
    ret = ethernetConfiguration.ReadUserConfiguration(devCfgs);
    EXPECT_TRUE(ret);
    ret = ethernetConfiguration.DelDir(USER_CONFIG_DIR_TEST);
    EXPECT_TRUE(ret);
}

HWTEST_F(EthernetConfigurationTest, EthernetConfiguration008, TestSize.Level1)
{
    EthernetConfiguration ethernetConfiguration;
    EthernetDhcpCallback::DhcpResult dhcpResult;
    dhcpResult.ipAddr = "test1";
    dhcpResult.gateWay = "test2";
    INetAddr ipv4Addr;
    ipv4Addr.address_ = "test1";
    sptr<StaticConfiguration> config = (std::make_unique<StaticConfiguration>()).release();
    config->ipAddrList_.push_back(ipv4Addr);
    bool ret = ethernetConfiguration.IsValidDhcpResult(dhcpResult, config);
    EXPECT_TRUE(ret);

    ipv4Addr.address_ = "test2";
    config->gatewayList_.push_back(ipv4Addr);
    ret = ethernetConfiguration.IsValidDhcpResult(dhcpResult, config);
    EXPECT_FALSE(ret);
}

HWTEST_F(EthernetConfigurationTest, EthernetConfiguration009, TestSize.Level1)
{
    EthernetConfiguration ethernetConfiguration;
    EthernetDhcpCallback::DhcpResult dhcpResult;
    dhcpResult.ipAddr = "test1";
    dhcpResult.gateWay = "test2";
    dhcpResult.route1 = "test3";
    dhcpResult.route2 = "test4";
    sptr<StaticConfiguration> config = (std::make_unique<StaticConfiguration>()).release();
    bool ret = ethernetConfiguration.ConvertToConfiguration(dhcpResult, config);
    EXPECT_TRUE(ret);
}

HWTEST_F(EthernetConfigurationTest, EthernetConfiguration010, TestSize.Level1)
{
    EthernetConfiguration ethernetConfiguration;
    EthernetDhcpCallback::DhcpResult dhcpResult;
    dhcpResult.ipAddr = "test1";
    dhcpResult.gateWay = "test2";
    dhcpResult.route1 = "*";
    dhcpResult.route2 = "test4";
    sptr<StaticConfiguration> config = (std::make_unique<StaticConfiguration>()).release();
    bool ret = ethernetConfiguration.ConvertToConfiguration(dhcpResult, config);
    EXPECT_TRUE(ret);
}

HWTEST_F(EthernetConfigurationTest, EthernetConfiguration011, TestSize.Level1)
{
    EthernetConfiguration ethernetConfiguration;
    EthernetDhcpCallback::DhcpResult dhcpResult;
    dhcpResult.ipAddr = "test1";
    dhcpResult.gateWay = "test2";
    dhcpResult.route1 = "*";
    dhcpResult.route2 = "*";
    sptr<StaticConfiguration> config = (std::make_unique<StaticConfiguration>()).release();
    bool ret = ethernetConfiguration.ConvertToConfiguration(dhcpResult, config);
    EXPECT_TRUE(ret);
}

HWTEST_F(EthernetConfigurationTest, EthernetConfiguration012, TestSize.Level1)
{
    EthernetConfiguration ethernetConfiguration;
    sptr<InterfaceConfiguration> configSptr = nullptr;
    sptr<InterfaceConfiguration> devCfg = (std::make_unique<InterfaceConfiguration>()).release();
    devCfg->mode_ = STATIC;
    sptr<NetLinkInfo> devLinkInfo = (std::make_unique<NetLinkInfo>()).release();
    configSptr = ethernetConfiguration.MakeInterfaceConfiguration(devCfg, devLinkInfo);
    EXPECT_TRUE(configSptr->mode_ == STATIC);
}
} // namespace NetManagerStandard
} // namespace OHOS