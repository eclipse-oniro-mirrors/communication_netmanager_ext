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

#include "ethernet_client.h"
#include "dev_interface_state.h"
#include "ethernet_lan_management.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
} // namespace

class EthernetLanManagementTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    sptr<InterfaceConfiguration> GetNewIfaceConfig();
};

void EthernetLanManagementTest::SetUpTestCase() {}

void EthernetLanManagementTest::TearDownTestCase() {}

void EthernetLanManagementTest::SetUp() {}

void EthernetLanManagementTest::TearDown() {}

constexpr const char *DEV_NAME = "eth2";

sptr<InterfaceConfiguration> EthernetLanManagementTest::GetNewIfaceConfig()
{
    sptr<InterfaceConfiguration> ic = (std::make_unique<InterfaceConfiguration>()).release();
    if (!ic) {
        return ic;
    }

    INetAddr ipv4Addr;
    ipv4Addr.type_ = INetAddr::IPV4;
    ipv4Addr.address_ = "10.14.0.99";
    ipv4Addr.netMask_ = "255.255.255.0";
    ic->ipStatic_.ipAddrList_.push_back(ipv4Addr);
    INetAddr route;
    route.type_ = INetAddr::IPV4;
    route.address_ = "0.0.0.0";
    route.netMask_ = "0.0.0.0";
    ic->ipStatic_.routeList_.push_back(route);
    INetAddr gateway;
    gateway.type_ = INetAddr::IPV4;
    gateway.address_ = "10.14.0.1";
    ic->ipStatic_.gatewayList_.push_back(gateway);
    return ic;
}

HWTEST_F(EthernetLanManagementTest, EthernetLanManagement001, TestSize.Level1)
{
    EthernetLanManagement ethernetLanManager;
    sptr<DevInterfaceState> devState = nullptr;
    int32_t ret = ethernetLanManager.UpdateLanLinkInfo(devState);
    ASSERT_EQ(ret, NETMANAGER_ERR_INTERNAL);

    sptr<DevInterfaceState> newDevState = new (std::nothrow) DevInterfaceState();
    sptr<InterfaceConfiguration> newIc = GetNewIfaceConfig();
    newIc->mode_ = LAN_STATIC;
    newDevState->SetDevName(DEV_NAME);
    newDevState->SetLancfg(newIc);
    ret = ethernetLanManager.UpdateLanLinkInfo(newDevState);
    ASSERT_EQ(ret, ETHERNET_ERR_DEVICE_NOT_LINK);

    newDevState->SetLinkUp(true);
    ret = ethernetLanManager.UpdateLanLinkInfo(newDevState);
    ASSERT_EQ(ret, (NETMANAGER_ERR_PERMISSION_DENIED + NETMANAGER_ERR_PERMISSION_DENIED));

    sptr<NetLinkInfo> linkInfo = nullptr;
    newDevState->SetlinkInfo(linkInfo);
    ret = ethernetLanManager.UpdateLanLinkInfo(newDevState);
    ASSERT_EQ(ret, NETMANAGER_ERR_INTERNAL);
}

HWTEST_F(EthernetLanManagementTest, EthernetLanManagement002, TestSize.Level1)
{
    EthernetLanManagement ethernetLanManager;
    NetLinkInfo netLinkInfo;
    int32_t ret = ethernetLanManager.DelIp(netLinkInfo);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
    ret = ethernetLanManager.SetIp(netLinkInfo);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
    ret = ethernetLanManager.DelRoute(netLinkInfo);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
    ret = ethernetLanManager.SetRoute(netLinkInfo);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);

    sptr<InterfaceConfiguration> newIc = GetNewIfaceConfig();
    sptr<DevInterfaceState> setDevState = new (std::nothrow) DevInterfaceState();
    newIc->mode_ = LAN_STATIC;
    setDevState->SetDevName(DEV_NAME);
    setDevState->SetLinkUp(true);
    setDevState->SetLancfg(newIc);
    setDevState->GetLinkInfo(netLinkInfo);
    ret = ethernetLanManager.DelIp(netLinkInfo);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
    ret = ethernetLanManager.SetIp(netLinkInfo);
    ASSERT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
    ret = ethernetLanManager.DelRoute(netLinkInfo);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
    ret = ethernetLanManager.SetRoute(netLinkInfo);
    ASSERT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

HWTEST_F(EthernetLanManagementTest, EthernetLanManagement003, TestSize.Level1)
{
    EthernetLanManagement ethernetLanManager;
    NetLinkInfo netLinkInfo;
    int32_t ret = ethernetLanManager.DelIp(netLinkInfo);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
    ret = ethernetLanManager.SetIp(netLinkInfo);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
    ret = ethernetLanManager.DelIp(netLinkInfo);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(EthernetLanManagementTest, EthernetLanManagement004, TestSize.Level1)
{
    EthernetLanManagement ethernetLanManager;
    NetLinkInfo netLinkInfo;
    int32_t ret = ethernetLanManager.DelRoute(netLinkInfo);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
    ret = ethernetLanManager.SetRoute(netLinkInfo);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
    ret = ethernetLanManager.DelRoute(netLinkInfo);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(EthernetLanManagementTest, EthernetLanManagementBranchTest001, TestSize.Level1)
{
    EthernetLanManagement ethernetLanManager;
    sptr<DevInterfaceState> devState = nullptr;
    ethernetLanManager.GetOldLinkInfo(devState);
    int32_t ret = ethernetLanManager.ReleaseLanNetLink(devState);
    EXPECT_EQ(ret, NETMANAGER_ERR_INTERNAL);

    devState = new (std::nothrow) DevInterfaceState();
    ethernetLanManager.GetOldLinkInfo(devState);
    ret = ethernetLanManager.ReleaseLanNetLink(devState);
    EXPECT_EQ(ret, NETMANAGER_ERR_INTERNAL);

    sptr<NetLinkInfo> linkInfo = new (std::nothrow) NetLinkInfo();
    devState->SetlinkInfo(linkInfo);
    ethernetLanManager.GetOldLinkInfo(devState);
    ret = ethernetLanManager.ReleaseLanNetLink(devState);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(EthernetLanManagementTest, ReleaseLanNetLinkTest001, TestSize.Level1)
{
    EthernetLanManagement ethernetLanManager;
    sptr<DevInterfaceState> devState = new (std::nothrow) DevInterfaceState();
    sptr<NetLinkInfo> linkInfo = new (std::nothrow) NetLinkInfo();
    INetAddr addr;
    addr.address_ = "192.168.1.1";
    addr.prefixlen_ = 24;
    linkInfo->netAddrList_.push_back(addr);

    Route route;
    route.destination_.address_ = "192.168.1.0";
    route.destination_.prefixlen_ = 24;
    route.gateway_.address_ = "192.168.1.254";
    linkInfo->routeList_.push_back(route);

    devState->SetlinkInfo(linkInfo);
    int32_t ret = ethernetLanManager.ReleaseLanNetLink(devState);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(EthernetLanManagementTest, SetIpTest, TestSize.Level1)
{
    EthernetLanManagement ethernetLanManager;

    NetLinkInfo newNetLinkInfo;
    INetAddr existingAddr;
    existingAddr.address_ = "192.168.1.1";
    existingAddr.prefixlen_ = 24;
    newNetLinkInfo.netAddrList_.push_back(existingAddr);

    INetAddr addr;
    addr.address_ = "192.168.1.1";
    addr.prefixlen_ = 24;
    ethernetLanManager.netLinkInfo_.netAddrList_.push_back(addr);

    int32_t ret = ethernetLanManager.SetIp(newNetLinkInfo);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(EthernetLanManagementTest, DelIpTest, TestSize.Level1)
{
    EthernetLanManagement ethernetLanManager;

    NetLinkInfo newNetLinkInfo;
    INetAddr existingAddr;
    existingAddr.address_ = "192.168.1.1";
    existingAddr.prefixlen_ = 24;
    newNetLinkInfo.netAddrList_.push_back(existingAddr);
    INetAddr addr;
    addr.address_ = "192.168.1.1";
    addr.prefixlen_ = 24;
    ethernetLanManager.netLinkInfo_.netAddrList_.push_back(addr);
    int32_t ret = ethernetLanManager.DelIp(newNetLinkInfo);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    INetAddr newAddr;
    newAddr.address_ = "192.168.1.2";
    newAddr.prefixlen_ = 24;
    newNetLinkInfo.netAddrList_.push_back(newAddr);
    ret = ethernetLanManager.DelIp(newNetLinkInfo);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(EthernetLanManagementTest, SetAndDelRouteTest, TestSize.Level1)
{
    EthernetLanManagement ethernetLanManager;
    NetLinkInfo newNetLinkInfo;
    Route existingRoute;
    existingRoute.destination_.address_ = "192.168.1.0";
    existingRoute.destination_.prefixlen_ = 24;
    existingRoute.gateway_.address_ = "192.168.1.254";
    newNetLinkInfo.routeList_.push_back(existingRoute);
    Route route;
    route.destination_.address_ = "192.168.1.0";
    route.destination_.prefixlen_ = 24;
    route.gateway_.address_ = "192.168.1.254";
    ethernetLanManager.netLinkInfo_.routeList_.push_back(route);

    int32_t ret = ethernetLanManager.SetRoute(newNetLinkInfo);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    ret = ethernetLanManager.DelRoute(newNetLinkInfo);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    Route newRoute;
    newRoute.destination_.address_ = "192.168.2.0";
    newRoute.destination_.prefixlen_ = 24;
    newRoute.gateway_.address_ = "192.168.2.254";
    newNetLinkInfo.routeList_.push_back(newRoute);
    ret = ethernetLanManager.DelRoute(newNetLinkInfo);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
