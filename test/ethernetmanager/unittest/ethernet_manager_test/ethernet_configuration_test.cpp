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

HWTEST_F(EthernetConfigurationTest, ReadEthernetInterfacesTest001, TestSize.Level1)
{
    EthernetConfiguration ethernetConfiguration;
    std::map<std::string, std::set<NetCap>> devCaps;
    std::map<std::string, sptr<InterfaceConfiguration>> devCfgs;
    sptr<InterfaceConfiguration> preConfig = new InterfaceConfiguration();
    devCfgs["eth0"] = preConfig;
    cJSON *json = cJSON_CreateArray();
    cJSON *item1 = cJSON_CreateObject();
    cJSON *capsArray = cJSON_CreateArray();

    cJSON_AddItemToArray(capsArray, cJSON_CreateNumber(1));
    cJSON_AddItemToArray(capsArray, cJSON_CreateNumber(2));

    std::string CONFIG_KEY_ETH_IFACE = "iface";
    std::string CONFIG_KEY_ETH_CAPS = "caps";
    cJSON_AddItemToObject(item1, CONFIG_KEY_ETH_IFACE.c_str(), cJSON_CreateString("eth0"));
    cJSON_AddItemToObject(item1, CONFIG_KEY_ETH_CAPS.c_str(), capsArray);
    cJSON_AddItemToArray(json, item1);

    bool result = ethernetConfiguration.ReadEthernetInterfaces(devCaps, devCfgs, json);
    EXPECT_EQ(result, true);
    EXPECT_EQ(devCaps.size(), 1);
    EXPECT_EQ(devCaps["eth0"].size(), 2);

    cJSON_Delete(json);
}

HWTEST_F(EthernetConfigurationTest, ReadEthernetInterfacesTest002, TestSize.Level1)
{
    EthernetConfiguration ethernetConfiguration;
    std::map<std::string, std::set<NetCap>> devCaps;
    std::map<std::string, sptr<InterfaceConfiguration>> devCfgs;
    sptr<InterfaceConfiguration> preConfig = new InterfaceConfiguration();
    devCfgs["eth0"] = preConfig;

    cJSON *json = cJSON_CreateArray();
    cJSON *item1 = cJSON_CreateObject();

    std::string CONFIG_KEY_ETH_IFACE = "iface";
    cJSON_AddItemToObject(item1, CONFIG_KEY_ETH_IFACE.c_str(), cJSON_CreateString("eth0"));
    cJSON_AddItemToArray(json, item1);

    bool result = ethernetConfiguration.ReadEthernetInterfaces(devCaps, devCfgs, json);
    EXPECT_EQ(result, true);
    EXPECT_EQ(devCaps.size(), 0);

    cJSON_Delete(json);
}

HWTEST_F(EthernetConfigurationTest, ReadEthernetInterfacesTest003, TestSize.Level1)
{
    class MockEthernetConfiguration : public EthernetConfiguration {
    public:
        sptr<InterfaceConfiguration> ConvertJsonToConfiguration(const cJSON *item, bool isLan) {
            return new InterfaceConfiguration();
        }
    };

    MockEthernetConfiguration ethernetConfiguration;
    std::map<std::string, std::set<NetCap>> devCaps;
    std::map<std::string, sptr<InterfaceConfiguration>> devCfgs;
    sptr<InterfaceConfiguration> preConfig = new InterfaceConfiguration();
    devCfgs["eth0"] = preConfig;
    cJSON *json = cJSON_CreateArray();
    cJSON *item1 = cJSON_CreateObject();

    std::string CONFIG_KEY_ETH_IFACE = "iface";
    std::string CONFIG_KEY_ETH_IP = "ip";
    cJSON_AddItemToObject(item1, CONFIG_KEY_ETH_IFACE.c_str(), cJSON_CreateString("eth0"));
    cJSON_AddItemToObject(item1, CONFIG_KEY_ETH_IP.c_str(), cJSON_CreateString("192.168.1.1"));

    cJSON_AddItemToArray(json, item1);

    bool result = ethernetConfiguration.ReadEthernetInterfaces(devCaps, devCfgs, json);
    EXPECT_EQ(result, true);
    EXPECT_EQ(devCaps.size(), 0);
    EXPECT_EQ(devCfgs.size(), 1);
    EXPECT_NE(devCfgs["eth0"], nullptr);

    cJSON_Delete(json);
}

HWTEST_F(EthernetConfigurationTest, WriteUserConfigurationTest001, TestSize.Level1)
{
    EthernetConfiguration ethernetConfiguration;
    std::string iface = "iface";
    sptr<InterfaceConfiguration> cfg = new InterfaceConfiguration();
    cfg->mode_ = STATIC;
    ethernetConfiguration.WriteUserConfiguration(iface, cfg);
    cfg->mode_ = LAN_STATIC;
    ethernetConfiguration.WriteUserConfiguration(iface, cfg);
    cfg->mode_ = DHCP;
    ethernetConfiguration.WriteUserConfiguration(iface, cfg);
    EXPECT_EQ(iface, "iface");
}

HWTEST_F(EthernetConfigurationTest, GetGatewayFromRouteListTest001, TestSize.Level1)
{
    EthernetConfiguration ethernetConfiguration;
    std::list<Route> routeList;
    Route route;
    INetAddr addr;
    addr.address_ = "0.0";
    route.gateway_ = addr;
    routeList.push_back(route);
    auto result = ethernetConfiguration.GetGatewayFromRouteList(routeList);
    EXPECT_TRUE(!result.empty());
}

HWTEST_F(EthernetConfigurationTest, MakeInterfaceConfigurationTest001, TestSize.Level1)
{
    EthernetConfiguration ethernetConfiguration;
    sptr<InterfaceConfiguration> devCfg = (std::make_unique<InterfaceConfiguration>()).release();
    devCfg->mode_ = STATIC;
    sptr<NetLinkInfo> devLinkInfo = nullptr;
    auto result = ethernetConfiguration.MakeInterfaceConfiguration(devCfg, devLinkInfo);
    EXPECT_EQ(result, nullptr);

    devCfg = nullptr;
    devLinkInfo = (std::make_unique<NetLinkInfo>()).release();
    result = ethernetConfiguration.MakeInterfaceConfiguration(devCfg, devLinkInfo);
    EXPECT_EQ(result, nullptr);
}

HWTEST_F(EthernetConfigurationTest, ParserIfaceIpAndRouteTest001, TestSize.Level1)
{
    EthernetConfiguration ethernetConfiguration;
    sptr<InterfaceConfiguration> cfg = new (std::nothrow) InterfaceConfiguration();

    INetAddr addr1;
    addr1.address_ = "2001:db8::/64";
    INetAddr addr2;
    addr2.address_ = "255.255.255.0";
    cfg->ipStatic_.netMaskList_.push_back(addr1);
    cfg->ipStatic_.netMaskList_.push_back(addr2);

    INetAddr ipAddr1;
    ipAddr1.address_ = "192.168.1.1";
    INetAddr ipAddr2;
    ipAddr2.address_ = "2001:db8::1";
    cfg->ipStatic_.ipAddrList_.push_back(ipAddr1);
    cfg->ipStatic_.ipAddrList_.push_back(ipAddr2);

    INetAddr route1;
    route1.address_ = "192.168.1.0";
    INetAddr route2;
    route2.address_ = "2001:db8::";
    cfg->ipStatic_.routeList_.push_back(route1);
    cfg->ipStatic_.routeList_.push_back(route2);

    std::string rootNetMask = "2001:db8::/64;255.255.255.0";

    ethernetConfiguration.ParserIfaceIpAndRoute(cfg, rootNetMask);

    for (const auto &route : cfg->ipStatic_.routeList_) {
        EXPECT_EQ(route.prefixlen_, 0);
    }
}
} // namespace NetManagerStandard
} // namespace OHOS