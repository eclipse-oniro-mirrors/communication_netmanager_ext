/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <sys/socket.h>
#include "cJSON.h"
#include "net_manager_constants.h"

#define private public
#include "wearable_distributed_net_link_info.h"
#undef private

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
const std::string DNSLISTS_FIRST = "dnslistsfirst";
const std::string DNSLISTS_SECOND = "dnslistssecond";
const std::string IFACENAME = "ifacename";
const std::string DEFAULT_NET_MASK = "defaultnetmask";
const std::string NET_IFACE_ADDRESS = "netifaceaddress";
const std::string IPV4_DEFAULT_ROUTE_ADDR = "ipv4defaultrouteaddr";
const std::string DUMMY_ADDRESS = "dummyaddress";
const std::string IPV4_ADDR_NET_MASK = "ipv4addrnetmask";
const std::string ROUTE_DESTINATION_ADDR = "routedestinationaddr";
} // namesapce

class WearableDistributedNetLinkInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();

    void SetUp();
    void TearDown();
};

void WearableDistributedNetLinkInfoTest::SetUpTestCase() {}

void WearableDistributedNetLinkInfoTest::TearDownTestCase() {}

void WearableDistributedNetLinkInfoTest::SetUp() {}

void WearableDistributedNetLinkInfoTest::TearDown() {}

std::string createTempFile(const std::string &content)
{
    std::string filePath = "temp_test_file.json";
    std::ofstream outfile(filePath);
    outfile << content;
    outfile.close();
    return filePath;
}

HWTEST_F(WearableDistributedNetLinkInfoTest, SetInterfaceName, TestSize.Level1)
{
    WearableDistributedNetLinkInfo info;
    NetLinkInfo linkInfo;
    info.SetInterFaceName(linkInfo);
    EXPECT_EQ(linkInfo.ifaceName_, info.GetIfaceName());
}

HWTEST_F(WearableDistributedNetLinkInfoTest, SetDnsLists, TestSize.Level1)
{
    std::list<INetAddr> dnsLists;
    WearableDistributedNetLinkInfo info;
    NetLinkInfo linkInfo;
    info.SetDnsLists(linkInfo);

    EXPECT_EQ(linkInfo.dnsList_.size(), 2);
    ASSERT_EQ(linkInfo.dnsList_.front().address_, "");

    auto it = ++linkInfo.dnsList_.begin();
    ASSERT_EQ(it->address_, "");
    ASSERT_EQ(it->type_, INetAddr::IPV4);
}

HWTEST_F(WearableDistributedNetLinkInfoTest, SetNetLinkIPInfo, TestSize.Level1)
{
    NetLinkInfo linkInfo;
    WearableDistributedNetLinkInfo info;
    info.SetNetLinkIPInfo(linkInfo);
    const auto &netAddr = linkInfo.netAddrList_.front();
    EXPECT_EQ(netAddr.type_, INetAddr::IPV4);
    EXPECT_EQ(netAddr.family_, AF_INET);
    EXPECT_EQ(netAddr.address_, info.GetIpv4DeRouteAddr());
    EXPECT_EQ(netAddr.netMask_, info.GetDefaultNetMask());
}

HWTEST_F(WearableDistributedNetLinkInfoTest, SetNetLinkRouteInfo, TestSize.Level1)
{
    NetLinkInfo linkInfo;
    WearableDistributedNetLinkInfo info;
    info.SetNetLinkRouteInfo(linkInfo);
    const auto &route = linkInfo.routeList_.front();
    EXPECT_EQ(route.iface_, info.GetIfaceName());
    EXPECT_EQ(route.destination_.type_, INetAddr::IPV4);
    EXPECT_EQ(route.destination_.family_, AF_INET);
    EXPECT_EQ(route.destination_.address_, "");
    EXPECT_EQ(route.gateway_.address_, info.GetNetIfaceAddress());
    EXPECT_EQ(route.gateway_.family_, AF_INET);
}

HWTEST_F(WearableDistributedNetLinkInfoTest, SetMtu, TestSize.Level1)
{
    WearableDistributedNetLinkInfo info;
    NetLinkInfo linkInfo;
    info.SetMtu(linkInfo);
    EXPECT_EQ(linkInfo.mtu_, CONSTANTS::WEARABLE_DISTRIBUTED_NET_MTU);
}

HWTEST_F(WearableDistributedNetLinkInfoTest, SetInterfaceDummyUp001, TestSize.Level1)
{
    WearableDistributedNetLinkInfo info;
    int32_t result = info.SetInterfaceDummyUp();
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(WearableDistributedNetLinkInfoTest, ParseDnsLists, TestSize.Level1)
{
    cJSON json;
    cJSON_AddStringToObject(&json, DNSLISTS_FIRST.c_str(), "114.114.114.114");
    WearableDistributedNetLinkInfo info;
    EXPECT_EQ(info.ParseDnsLists(json), false);

    cJSON_AddStringToObject(&json, DNSLISTS_SECOND.c_str(), "114.114.115.115");
    EXPECT_EQ(info.ParseDnsLists(json), true);
}

HWTEST_F(WearableDistributedNetLinkInfoTest, ParseIfaceName, TestSize.Level1)
{
    cJSON json;
    cJSON_AddStringToObject(&json, DNSLISTS_FIRST.c_str(), "114.114.114.114");
    WearableDistributedNetLinkInfo info;
    EXPECT_EQ(info.ParseIfaceName(json), false);

    cJSON_AddStringToObject(&json, IFACENAME.c_str(), "lo");
    EXPECT_EQ(info.ParseIfaceName(json), true);
}

HWTEST_F(WearableDistributedNetLinkInfoTest, ParseDefaultNetMask, TestSize.Level1)
{
    cJSON json;
    cJSON_AddStringToObject(&json, DNSLISTS_FIRST.c_str(), "114.114.114.114");
    WearableDistributedNetLinkInfo info;
    EXPECT_EQ(info.ParseDefaultNetMask(json), false);

    cJSON_AddStringToObject(&json, DEFAULT_NET_MASK.c_str(), "255.255.255.0");
    EXPECT_EQ(info.ParseDefaultNetMask(json), true);
}

HWTEST_F(WearableDistributedNetLinkInfoTest, ParseNetIfaceAddress, TestSize.Level1)
{
    cJSON json;
    cJSON_AddStringToObject(&json, DNSLISTS_FIRST.c_str(), "114.114.114.114");
    WearableDistributedNetLinkInfo info;
    EXPECT_EQ(info.ParseNetIfaceAddress(json), false);

    cJSON_AddStringToObject(&json, NET_IFACE_ADDRESS.c_str(), "127.0.0.1");
    EXPECT_EQ(info.ParseNetIfaceAddress(json), true);
}

HWTEST_F(WearableDistributedNetLinkInfoTest, ParseIpv4DeRouteAddr, TestSize.Level1)
{
    cJSON json;
    cJSON_AddStringToObject(&json, DNSLISTS_FIRST.c_str(), "114.114.114.114");
    WearableDistributedNetLinkInfo info;
    EXPECT_EQ(info.ParseIpv4DeRouteAddr(json), false);

    cJSON_AddStringToObject(&json, IPV4_DEFAULT_ROUTE_ADDR.c_str(), "127.0.0.2");
    EXPECT_EQ(info.ParseIpv4DeRouteAddr(json), true);
}

HWTEST_F(WearableDistributedNetLinkInfoTest, ParseDummyAddress, TestSize.Level1)
{
    cJSON json;
    cJSON_AddStringToObject(&json, DNSLISTS_FIRST.c_str(), "114.114.114.114");
    WearableDistributedNetLinkInfo info;
    EXPECT_EQ(info.ParseDummyAddress(json), false);

    cJSON_AddStringToObject(&json, DUMMY_ADDRESS.c_str(), "192.168.167.239");
    EXPECT_EQ(info.ParseDummyAddress(json), true);
}

HWTEST_F(WearableDistributedNetLinkInfoTest, ParseIpv4AddrNetMask, TestSize.Level1)
{
    cJSON json;
    cJSON_AddStringToObject(&json, DNSLISTS_FIRST.c_str(), "114.114.114.114");
    WearableDistributedNetLinkInfo info;
    EXPECT_EQ(info.ParseIpv4AddrNetMask(json), false);

    cJSON_AddStringToObject(&json, IPV4_ADDR_NET_MASK.c_str(), "255.255.255.0");
    EXPECT_EQ(info.ParseIpv4AddrNetMask(json), true);
}

HWTEST_F(WearableDistributedNetLinkInfoTest, ParseRouteDestinationAddr, TestSize.Level1)
{
    cJSON json;
    cJSON_AddStringToObject(&json, DNSLISTS_FIRST.c_str(), "114.114.114.114");
    WearableDistributedNetLinkInfo info;
    EXPECT_EQ(info.ParseRouteDestinationAddr(json), false);

    cJSON_AddStringToObject(&json, ROUTE_DESTINATION_ADDR.c_str(), "0.0.0.0");
    EXPECT_EQ(info.ParseRouteDestinationAddr(json), true);
}

HWTEST_F(WearableDistributedNetLinkInfoTest, ReadJsonFile001, TestSize.Level1)
{
    WearableDistributedNetLinkInfo info;
    info.configPath_ = createTempFile(R"({"key": "value"})");
    std::string result = info.ReadJsonFile();
    EXPECT_EQ(R"({"key": "value"})", result);
}

HWTEST_F(WearableDistributedNetLinkInfoTest, ReadJsonFile002, TestSize.Level1)
{
    WearableDistributedNetLinkInfo info;
    info.configPath_ = "non_existent_file.json";
    std::string result = info.ReadJsonFile();
    EXPECT_EQ("", result);
}

HWTEST_F(WearableDistributedNetLinkInfoTest, ReadJsonFile003, TestSize.Level1)
{
    WearableDistributedNetLinkInfo info;
    info.configPath_ = createTempFile("");
    std::string result = info.ReadJsonFile();
    EXPECT_EQ("", result);
}
HWTEST_F(WearableDistributedNetLinkInfoTest, ReadJsonFile004, TestSize.Level1)
{
    WearableDistributedNetLinkInfo info;
    info.configPath_ = createTempFile(R"({"key": "value"}\n{\"anotherKey\": \"anotherValue\"})");
    std::string result = info.ReadJsonFile();
    EXPECT_EQ(R"({"key": "value"}\n{\"anotherKey\": \"anotherValue\"})", result);
}

HWTEST_F(WearableDistributedNetLinkInfoTest, ReadSystemNetlinkinfoConfiguration001, TestSize.Level1)
{
    WearableDistributedNetLinkInfo info;
    bool result = info.ReadSystemNetlinkinfoConfiguration();
    EXPECT_TRUE(result);

    cJSON json;
    cJSON_AddStringToObject(&json, DNSLISTS_FIRST.c_str(), "114.114.114.114");
    result = info.ReadNetlinkinfoInterfaces(json);
    EXPECT_FALSE(result);

    cJSON_AddStringToObject(&json, DNSLISTS_SECOND.c_str(), "114.114.115.115");
    result = info.ReadNetlinkinfoInterfaces(json);
    EXPECT_FALSE(result);

    cJSON_AddStringToObject(&json, IFACENAME.c_str(), "lo");
    result = info.ReadNetlinkinfoInterfaces(json);
    EXPECT_FALSE(result);

    cJSON_AddStringToObject(&json, DEFAULT_NET_MASK.c_str(), "255.0.0.0");
    result = info.ReadNetlinkinfoInterfaces(json);
    EXPECT_FALSE(result);

    cJSON_AddStringToObject(&json, NET_IFACE_ADDRESS.c_str(), "127.0.0.1");
    result = info.ReadNetlinkinfoInterfaces(json);
    EXPECT_FALSE(result);

    cJSON_AddStringToObject(&json, IPV4_DEFAULT_ROUTE_ADDR.c_str(), "127.0.0.2");
    result = info.ReadNetlinkinfoInterfaces(json);
    EXPECT_FALSE(result);

    cJSON_AddStringToObject(&json, DUMMY_ADDRESS.c_str(), "192.168.167.239");
    result = info.ReadNetlinkinfoInterfaces(json);
    EXPECT_FALSE(result);

    cJSON_AddStringToObject(&json, IPV4_ADDR_NET_MASK.c_str(), "255.255.255.0");
    result = info.ReadNetlinkinfoInterfaces(json);
    EXPECT_FALSE(result);
}

HWTEST_F(WearableDistributedNetLinkInfoTest, SetInterfaceDummyDown, TestSize.Level1)
{
    int32_t result = SetInterfaceDummyDown();
    EXPECT_EQ(NETMANAGER_EXT_SUCCESS, result);
}

HWTEST_F(WearableDistributedNetLinkInfoTest, ReadSystemNetlinkinfoConfiguration002, TestSize.Level1)
{
    WearableDistributedNetLinkInfo info;
    info.configPath_ = "";
    int32_t result = info.ReadSystemNetlinkinfoConfiguration();
    EXPECT_FALSE(result);
}

HWTEST_F(WearableDistributedNetLinkInfoTest, SetInterfaceDummyUp002, TestSize.Level1)
{
    int32_t result = SetInterfaceDummyUp();
    EXPECT_EQ(NETMANAGER_EXT_ERR_INTERNAL, result);
}

HWTEST_F(WearableDistributedNetLinkInfoTest, CreateNetLinkInfo, TestSize.Level1)
{
    NetLinkInfo linkInfo;
    int32_t result = CreateNetLinkInfo(linkInfo);
    EXPECT_EQ(NETMANAGER_EXT_SUCCESS, result);
}

HWTEST_F(WearableDistributedNetLinkInfoTest, SetInterfaceDummyUp003, TestSize.Level1)
{
    WearableDistributedNetLinkInfo info;
    info.ReadSystemNetlinkinfoConfiguration();
    int32_t result = info.SetInterfaceDummyUp();
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(WearableDistributedNetLinkInfoTest, SetInterfaceDummyDown002, TestSize.Level1)
{
    WearableDistributedNetLinkInfo info;
    info.ReadSystemNetlinkinfoConfiguration();
    int32_t result = info.SetInterfaceDummyDown();
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
