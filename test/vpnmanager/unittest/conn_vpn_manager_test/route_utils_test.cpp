/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <sys/socket.h>

#include <gtest/gtest.h>

#include "net_conn_types.h"
#include "net_manager_constants.h"
#ifdef GTEST_API_
#define private public
#endif
#include "route_utils.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr uint32_t INVALID_VALUE = 50;

Route GetRoute()
{
    std::string iface("eth0");
    Route route;
    route.iface_ = iface;
    route.rtnType_ = RTN_UNICAST;
    route.hasGateway_ = true;
    route.isDefaultRoute_ = false;
    route.destination_.type_ = INetAddr::IPV4;
    route.destination_.family_ = AF_INET;
    route.destination_.prefixlen_ = 0x18;
    route.destination_.address_ = "192.168.2.10";
    route.destination_.netMask_ = "255.255.255.0";
    route.destination_.hostName_ = "netAddr";
    route.gateway_.type_ = INetAddr::IPV4;
    route.gateway_.family_ = AF_INET;
    route.gateway_.prefixlen_ = 0x18;
    route.gateway_.address_ = "192.168.2.1";
    route.gateway_.netMask_ = "255.255.255.0";
    route.gateway_.hostName_ = "netAddr";
    return route;
}

Route GetRoute2()
{
    std::string iface("eth0");
    Route route;
    route.iface_ = iface;
    route.rtnType_ = RTN_UNREACHABLE;
    route.hasGateway_ = false;
    route.isDefaultRoute_ = false;
    route.destination_.type_ = INetAddr::IPV4;
    route.destination_.family_ = AF_INET;
    route.destination_.prefixlen_ = 0x18;
    route.destination_.address_ = "192.168.2.10";
    route.destination_.netMask_ = "255.255.255.0";
    route.destination_.hostName_ = "netAddr";
    route.gateway_.type_ = INetAddr::IPV4;
    route.gateway_.family_ = AF_INET;
    route.gateway_.prefixlen_ = 0x18;
    route.gateway_.address_ = "192.168.2.1";
    route.gateway_.netMask_ = "255.255.255.0";
    route.gateway_.hostName_ = "netAddr";
    return route;
}

Route GetRoute3()
{
    std::string iface("eth0");
    Route route;
    route.iface_ = iface;
    route.rtnType_ = RTN_UNICAST;
    route.hasGateway_ = false;
    route.isDefaultRoute_ = false;
    route.destination_.type_ = INetAddr::IPV4;
    route.destination_.family_ = AF_INET;
    route.destination_.prefixlen_ = 0x18;
    route.destination_.address_ = "192.168.2.10";
    route.destination_.netMask_ = "255.255.255.0";
    route.destination_.hostName_ = "netAddr";
    route.gateway_.type_ = INetAddr::IPV4;
    route.gateway_.family_ = AF_INET;
    route.gateway_.prefixlen_ = 0x18;
    route.gateway_.address_ = "192.168.2.1";
    route.gateway_.netMask_ = "255.255.255.0";
    route.gateway_.hostName_ = "netAddr";
    return route;
}

Route GetRoute4()
{
    std::string iface("eth0");
    Route route;
    route.iface_ = iface;
    route.rtnType_ = RTN_UNICAST;
    route.hasGateway_ = false;
    route.isDefaultRoute_ = false;
    route.destination_.type_ = INetAddr::IPV4;
    route.destination_.family_ = AF_INET;
    route.destination_.prefixlen_ = 0x18;
    route.destination_.address_ = "192168210";
    route.destination_.netMask_ = "255.255.255.0";
    route.destination_.hostName_ = "netAddr";
    route.gateway_.type_ = INetAddr::IPV4;
    route.gateway_.family_ = AF_INET;
    route.gateway_.prefixlen_ = 0x18;
    route.gateway_.address_ = "192.168.2.1";
    route.gateway_.netMask_ = "255.255.255.0";
    route.gateway_.hostName_ = "netAddr";
    return route;
}

constexpr uint32_t TEST_NETID = 110;
} // namespace

class RouteUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void RouteUtilsTest::SetUpTestCase() {}

void RouteUtilsTest::TearDownTestCase() {}

void RouteUtilsTest::SetUp() {}

void RouteUtilsTest::TearDown() {}

HWTEST_F(RouteUtilsTest, AddRouteToLocal01, TestSize.Level1)
{
    std::list<Route> rList;
    std::string iface("eth0");
    rList.push_back(GetRoute());
    RouteUtils::AddRoutesToLocal(iface, rList);
    EXPECT_FALSE(rList.empty());
}

HWTEST_F(RouteUtilsTest, RemoveRouteFromLocal01, TestSize.Level1)
{
    std::list<Route> rList;
    rList.push_back(GetRoute());

    EXPECT_EQ(0, RouteUtils::RemoveRoutesFromLocal(rList));
}

HWTEST_F(RouteUtilsTest, AddRouteToLocal02, TestSize.Level1)
{
    std::list<Route> rList;
    std::string iface("eth0");
    Route route;
    route.iface_ = iface;
    route.rtnType_ = RTN_UNICAST;
    route.hasGateway_ = true;
    route.isDefaultRoute_ = false;
    route.destination_.type_ = INetAddr::IPV4;
    route.destination_.family_ = AF_INET;
    route.destination_.prefixlen_ = 0;
    route.destination_.address_ = "192.168.2.10";
    route.destination_.netMask_ = "255.255.255.0";
    route.destination_.hostName_ = "netAddr";
    route.gateway_.type_ = INetAddr::IPV4;
    route.gateway_.family_ = AF_INET;
    route.gateway_.prefixlen_ = 0x18;
    route.gateway_.address_ = "192.168.2.1";
    route.gateway_.netMask_ = "255.255.255.0";
    route.gateway_.hostName_ = "netAddr";
    rList.push_back(route);
    int32_t ret = RouteUtils::AddRoutesToLocal(iface, rList);
    EXPECT_NE(ret, 0);
}

HWTEST_F(RouteUtilsTest, AddRouteToLocal03, TestSize.Level1)
{
    std::list<Route> rList;
    std::string iface("eth0");
    Route route;
    route.iface_ = iface;
    route.rtnType_ = RTN_UNREACHABLE;
    route.hasGateway_ = false;
    route.isDefaultRoute_ = false;
    route.destination_.type_ = INetAddr::IPV4;
    route.destination_.family_ = AF_INET;
    route.destination_.prefixlen_ = 0x18;
    route.destination_.address_ = "192.168.2.10";
    route.destination_.netMask_ = "255.255.255.0";
    route.destination_.hostName_ = "netAddr";
    rList.push_back(route);
    int32_t ret = RouteUtils::AddRoutesToLocal(iface, rList);
    EXPECT_NE(ret, 0);
}

HWTEST_F(RouteUtilsTest, AddRouteToLocal04, TestSize.Level1)
{
    std::list<Route> rList;
    std::string iface("eth0");
    Route route;
    route.iface_ = iface;
    route.rtnType_ = RTN_THROW;
    route.hasGateway_ = false;
    route.isDefaultRoute_ = false;
    route.destination_.type_ = INetAddr::IPV4;
    route.destination_.family_ = AF_INET;
    route.destination_.prefixlen_ = 0x18;
    route.destination_.address_ = "192.168.2.10";
    route.destination_.netMask_ = "255.255.255.0";
    route.destination_.hostName_ = "netAddr";
    rList.push_back(route);
    int32_t ret = RouteUtils::AddRoutesToLocal(iface, rList);
    EXPECT_NE(ret, 0);
}

HWTEST_F(RouteUtilsTest, AddRoute01, TestSize.Level1)
{
    EXPECT_GE(0, RouteUtils::AddRoute(TEST_NETID, GetRoute3()));
}

HWTEST_F(RouteUtilsTest, AddRoute02, TestSize.Level1)
{
    EXPECT_GE(0, RouteUtils::AddRoute(TEST_NETID, GetRoute4()));
}

HWTEST_F(RouteUtilsTest, AddRoute03, TestSize.Level1)
{
    EXPECT_GE(0, RouteUtils::AddRoute(TEST_NETID, GetRoute()));
}

HWTEST_F(RouteUtilsTest, RemoveRoute01, TestSize.Level1)
{
    EXPECT_GE(0, RouteUtils::RemoveRoute(TEST_NETID, GetRoute()));
}

HWTEST_F(RouteUtilsTest, UpdateRoutes01, TestSize.Level1)
{
    NetLinkInfo nlio;
    NetLinkInfo nlin;
    nlio.routeList_.push_back(GetRoute());
    nlin.routeList_.push_back(GetRoute2());

    EXPECT_TRUE(RouteUtils::UpdateRoutes(TEST_NETID, nlin, nlio));
}

HWTEST_F(RouteUtilsTest, UpdateRoutes02, TestSize.Level1)
{
    NetLinkInfo nlio;
    NetLinkInfo nlin;
    nlio.routeList_.push_back(GetRoute());

    EXPECT_TRUE(RouteUtils::UpdateRoutes(TEST_NETID, nlin, nlio));
}

HWTEST_F(RouteUtilsTest, RouteUtilsBranchTest001, TestSize.Level1)
{
    Route route = {};
    route.rtnType_ = INVALID_VALUE;
    routeOperateType op = static_cast<routeOperateType>(INVALID_VALUE);
    int32_t netId = 100;
    int32_t ret = RouteUtils::ModifyRoute(op, netId, route);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
    int32_t prefixLen = 0;
    std::string str = RouteUtils::MaskAddress("", prefixLen);
    EXPECT_EQ(str, "");
}
} // namespace NetManagerStandard
} // namespace OHOS
