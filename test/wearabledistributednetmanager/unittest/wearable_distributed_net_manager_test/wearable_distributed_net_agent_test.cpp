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
#include <arpa/inet.h>
#include "net_manager_constants.h"

#define private public
#include "wearable_distributed_net_agent.h"
#undef private

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr int32_t TCP_PORT_ID = 8080;
constexpr int32_t UDP_PORT_ID = 8081;
const std::string WEARABLE_DISTRIBUTED_NET_NAME = "wearabledistributednet";
using namespace testing::ext;
} // namespace

class WearableDistributedNetAgentTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    WearableDistributedNetAgent &agentInstance = WearableDistributedNetAgent::GetInstance();
};

void WearableDistributedNetAgentTest::SetUpTestCase() {}

void WearableDistributedNetAgentTest::TearDownTestCase() {}

void WearableDistributedNetAgentTest::SetUp() {}

void WearableDistributedNetAgentTest::TearDown() {}

HWTEST_F(WearableDistributedNetAgentTest, EnableWearableDistributedNetForward, TestSize.Level1)
{
    int32_t ret = agentInstance.EnableWearableDistributedNetForward(-80, 8002);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INVALID_PARAMETER);
    ret = agentInstance.EnableWearableDistributedNetForward(8001, 0);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INVALID_PARAMETER);
    ret = agentInstance.EnableWearableDistributedNetForward(8001, 8002);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    ret = agentInstance.DisableWearableDistributedNetForward();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(WearableDistributedNetAgentTest, ObtainNetCaps, TestSize.Level1)
{
    std::set<NetCap> capsMetered;
    std::set<NetCap> capsNotMetered;
    capsMetered.insert(NET_CAPABILITY_INTERNET);
    capsMetered.insert(NET_CAPABILITY_NOT_VPN);
    capsNotMetered = capsMetered;
    capsNotMetered.insert(NET_CAPABILITY_NOT_METERED);

    agentInstance.netCaps_.clear();
    bool isMetered = false;
    agentInstance.ObtainNetCaps(isMetered);
    EXPECT_EQ(agentInstance.netCaps_, capsNotMetered);

    agentInstance.netCaps_.clear();
    isMetered = true;
    agentInstance.ObtainNetCaps(isMetered);
    EXPECT_EQ(agentInstance.netCaps_, capsMetered);
}

HWTEST_F(WearableDistributedNetAgentTest, GetNetSupplierInfo, TestSize.Level1)
{
    NetSupplierInfo supplierInfo;
    supplierInfo.isAvailable_ = true;
    supplierInfo.isRoaming_ = false;
    supplierInfo.linkUpBandwidthKbps_ = 220;
    supplierInfo.linkDownBandwidthKbps_ = 220;

    NetSupplierInfo getSupplierInfo;
    agentInstance.SetNetSupplierInfo(getSupplierInfo);
    EXPECT_EQ(supplierInfo.isAvailable_, getSupplierInfo.isAvailable_);
    EXPECT_EQ(supplierInfo.isRoaming_, getSupplierInfo.isRoaming_);
    EXPECT_EQ(supplierInfo.linkUpBandwidthKbps_, getSupplierInfo.linkUpBandwidthKbps_);
    EXPECT_EQ(supplierInfo.linkDownBandwidthKbps_, getSupplierInfo.linkDownBandwidthKbps_);
}

HWTEST_F(WearableDistributedNetAgentTest, GetNetLinkInfo001, TestSize.Level1)
{
    NetLinkInfo getLinkInfo;
    int32_t ret = agentInstance.SetNetLinkInfo(getLinkInfo);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(WearableDistributedNetAgentTest, GetNetLinkInfo002, TestSize.Level1)
{
    NetLinkInfo getLinkInfo;
    agentInstance.SetNetLinkInfo(getLinkInfo);
    EXPECT_EQ(getLinkInfo.ifaceName_, "lo");
    EXPECT_EQ(getLinkInfo.dnsList_.front().address_, "114.114.114.114");
    EXPECT_EQ(getLinkInfo.dnsList_.front().type_, INetAddr::IPV4);

    auto it = std::next(getLinkInfo.dnsList_.begin());
    EXPECT_EQ(it->address_, "114.114.115.115");
    EXPECT_EQ(it->type_, INetAddr::IPV4);

    EXPECT_EQ(getLinkInfo.netAddrList_.front().address_, "127.0.0.2");
    EXPECT_EQ(getLinkInfo.routeList_.front().destination_.address_, "0.0.0.0");
    EXPECT_EQ(getLinkInfo.routeList_.front().gateway_.address_, "127.0.0.1");

    EXPECT_EQ(getLinkInfo.mtu_, 1500);
}

HWTEST_F(WearableDistributedNetAgentTest, SetupWearableDistributedNetNetwork001, TestSize.Level1)
{
    EXPECT_EQ(agentInstance.SetupWearableDistributedNetwork(TCP_PORT_ID, UDP_PORT_ID, false),
        NETMANAGER_ERR_PERMISSION_DENIED);
    agentInstance.TearDownWearableDistributedNetwork();
    EXPECT_EQ(agentInstance.SetupWearableDistributedNetwork(TCP_PORT_ID, UDP_PORT_ID, true),
        NETMANAGER_ERR_PERMISSION_DENIED);
    agentInstance.TearDownWearableDistributedNetwork();
}

HWTEST_F(WearableDistributedNetAgentTest, TearDownWearableDistributedNetwork001, TestSize.Level1)
{
    agentInstance.SetupWearableDistributedNetwork(TCP_PORT_ID, UDP_PORT_ID, false);
    int32_t ret = agentInstance.TearDownWearableDistributedNetwork();
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(WearableDistributedNetAgentTest, TearDownWearableDistributedNetwork002, TestSize.Level1)
{
    agentInstance.netSupplierId_ = 0;
    int32_t ret = agentInstance.TearDownWearableDistributedNetwork();
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(WearableDistributedNetAgentTest, RegisterNetSupplier001, TestSize.Level1)
{
    agentInstance.netSupplierId_ = 1;
    bool isMetered = false;
    EXPECT_EQ(agentInstance.RegisterNetSupplier(isMetered), NET_CONN_SUCCESS);
    agentInstance.UnregisterNetSupplier();
    isMetered = true;
    EXPECT_EQ(agentInstance.RegisterNetSupplier(isMetered), NETMANAGER_ERR_PERMISSION_DENIED);
    agentInstance.UnregisterNetSupplier();
}

HWTEST_F(WearableDistributedNetAgentTest, RegisterNetSupplier002, TestSize.Level1)
{
    bool isMetered = true;
    agentInstance.netSupplierId_ = 0;
    EXPECT_EQ(agentInstance.RegisterNetSupplier(isMetered), NETMANAGER_ERR_PERMISSION_DENIED);
    agentInstance.UnregisterNetSupplier();
    isMetered = false;
    EXPECT_EQ(agentInstance.RegisterNetSupplier(isMetered), NETMANAGER_ERR_PERMISSION_DENIED);
    agentInstance.UnregisterNetSupplier();
}

HWTEST_F(WearableDistributedNetAgentTest, UnregisterNetSupplier001, TestSize.Level1)
{
    agentInstance.netSupplierId_ = 0;
    int32_t result = agentInstance.UnregisterNetSupplier();
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(WearableDistributedNetAgentTest, UnregisterNetSupplier002, TestSize.Level1)
{
    bool isMetered = true;
    agentInstance.netSupplierId_ = 0;
    agentInstance.RegisterNetSupplier(isMetered);
    int32_t result = agentInstance.UnregisterNetSupplier();
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
}

HWTEST_F(WearableDistributedNetAgentTest, UnregisterNetSupplier003, TestSize.Level1)
{
    bool isMetered = true;
    agentInstance.netSupplierId_ = 0;
    int32_t ret = agentInstance.UnregisterNetSupplier();
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(WearableDistributedNetAgentTest, UpdateNetSupplierInfo001, TestSize.Level1)
{
    agentInstance.netSupplierId_ = 0;
    EXPECT_NE(agentInstance.UpdateNetSupplierInfo(true), NETMANAGER_SUCCESS);
    agentInstance.netSupplierId_ = 1;
    EXPECT_EQ(agentInstance.UpdateNetSupplierInfo(true), NETMANAGER_ERR_PERMISSION_DENIED);
}

HWTEST_F(WearableDistributedNetAgentTest, UpdateNetSupplierInfo002, TestSize.Level1)
{
    agentInstance.netSupplierId_ = 1;
    EXPECT_NE(agentInstance.UpdateNetSupplierInfo(true), NETMANAGER_SUCCESS);
}

HWTEST_F(WearableDistributedNetAgentTest, UpdateNetLinkInfo001, TestSize.Level1)
{
    agentInstance.netSupplierId_ = 0;
    EXPECT_NE(agentInstance.UpdateNetLinkInfo(), 0);
    agentInstance.netSupplierId_ = 1;
    EXPECT_EQ(agentInstance.UpdateNetLinkInfo(), NETMANAGER_EXT_ERR_INTERNAL);
    agentInstance.netSupplierId_ = 0;
}

HWTEST_F(WearableDistributedNetAgentTest, UpdateNetLinkInfo002, TestSize.Level1)
{
    agentInstance.netSupplierId_ = 1;
    EXPECT_NE(agentInstance.UpdateNetLinkInfo(), NETMANAGER_SUCCESS);
    agentInstance.netSupplierId_ = 0;
}

HWTEST_F(WearableDistributedNetAgentTest, SetupWearableDistributedNetNetwork002, TestSize.Level1)
{
    int32_t tcpPortId = 8001;
    int32_t udpPortId = 0;
    bool isMetered = false;
    agentInstance.netSupplierId_ = 1;
    int32_t result = agentInstance.SetupWearableDistributedNetwork(tcpPortId, udpPortId, isMetered);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_INVALID_PARAMETER);
}
} // namespace NetManagerStandard
} // namespace OHOS
