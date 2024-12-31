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
#include "battery_srv_client.h"
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
};

void WearableDistributedNetAgentTest::SetUpTestCase() {}

void WearableDistributedNetAgentTest::TearDownTestCase() {}

void WearableDistributedNetAgentTest::SetUp() {}

void WearableDistributedNetAgentTest::TearDown() {}

HWTEST_F(WearableDistributedNetAgentTest, EnableWearableDistributedNetForward001, TestSize.Level1)
{
    int32_t ret = WearableDistributedNetAgent::GetInstance().EnableWearableDistributedNetForward(-80, 8002);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INVALID_PARAMETER);
}

HWTEST_F(WearableDistributedNetAgentTest, EnableWearableDistributedNetForward002, TestSize.Level1)
{
    int32_t ret = WearableDistributedNetAgent::GetInstance().EnableWearableDistributedNetForward(8001, 0);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INVALID_PARAMETER);
}

HWTEST_F(WearableDistributedNetAgentTest, EnableWearableDistributedNetForward003, TestSize.Level1)
{
    int32_t ret = WearableDistributedNetAgent::GetInstance().EnableWearableDistributedNetForward(8001, 8002);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    ret = WearableDistributedNetAgent::GetInstance().DisableWearableDistributedNetForward();
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

    WearableDistributedNetAgent::GetInstance().netCaps_.clear();
    bool isMetered = false;
    WearableDistributedNetAgent::GetInstance().ObtainNetCaps(isMetered);
    EXPECT_EQ(WearableDistributedNetAgent::GetInstance().netCaps_, capsNotMetered);

    WearableDistributedNetAgent::GetInstance().netCaps_.clear();
    isMetered = true;
    WearableDistributedNetAgent::GetInstance().ObtainNetCaps(isMetered);
    EXPECT_EQ(WearableDistributedNetAgent::GetInstance().netCaps_, capsMetered);
}

HWTEST_F(WearableDistributedNetAgentTest, GetNetSupplierInfo, TestSize.Level1)
{
    NetSupplierInfo supplierInfo;
    supplierInfo.isAvailable_ = true;
    supplierInfo.isRoaming_ = false;
    supplierInfo.linkUpBandwidthKbps_ = 220;
    supplierInfo.linkDownBandwidthKbps_ = 220;

    NetSupplierInfo getSupplierInfo;
    WearableDistributedNetAgent::GetInstance().SetNetSupplierInfo(getSupplierInfo);
    EXPECT_EQ(supplierInfo.isRoaming_, getSupplierInfo.isRoaming_);
    EXPECT_EQ(supplierInfo.linkUpBandwidthKbps_, getSupplierInfo.linkUpBandwidthKbps_);
    EXPECT_EQ(supplierInfo.linkDownBandwidthKbps_, getSupplierInfo.linkDownBandwidthKbps_);
}

HWTEST_F(WearableDistributedNetAgentTest, GetNetLinkInfo001, TestSize.Level1)
{
    NetLinkInfo getLinkInfo;
    int32_t ret = WearableDistributedNetAgent::GetInstance().SetNetLinkInfo(getLinkInfo);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(WearableDistributedNetAgentTest, GetNetLinkInfo002, TestSize.Level1)
{
    NetLinkInfo getLinkInfo;
    WearableDistributedNetAgent::GetInstance().SetNetLinkInfo(getLinkInfo);
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
    EXPECT_EQ(WearableDistributedNetAgent::GetInstance()
        .SetupWearableDistributedNetwork(TCP_PORT_ID, UDP_PORT_ID, false), NETMANAGER_ERR_PERMISSION_DENIED);
    WearableDistributedNetAgent::GetInstance().TearDownWearableDistributedNetwork();
    EXPECT_EQ(WearableDistributedNetAgent::GetInstance()
        .SetupWearableDistributedNetwork(TCP_PORT_ID, UDP_PORT_ID, true), NETMANAGER_ERR_PERMISSION_DENIED);
    WearableDistributedNetAgent::GetInstance().TearDownWearableDistributedNetwork();
}

HWTEST_F(WearableDistributedNetAgentTest, TearDownWearableDistributedNetwork001, TestSize.Level1)
{
    WearableDistributedNetAgent::GetInstance().SetupWearableDistributedNetwork(TCP_PORT_ID, UDP_PORT_ID, false);
    int32_t ret = WearableDistributedNetAgent::GetInstance().TearDownWearableDistributedNetwork();
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(WearableDistributedNetAgentTest, TearDownWearableDistributedNetwork002, TestSize.Level1)
{
    WearableDistributedNetAgent::GetInstance().netSupplierId_ = 0;
    int32_t ret = WearableDistributedNetAgent::GetInstance().TearDownWearableDistributedNetwork();
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(WearableDistributedNetAgentTest, RegisterNetSupplier001, TestSize.Level1)
{
    WearableDistributedNetAgent::GetInstance().netSupplierId_ = 1;
    bool isMetered = false;
    EXPECT_EQ(WearableDistributedNetAgent::GetInstance().RegisterNetSupplier(isMetered), NETMANAGER_SUCCESS);
    WearableDistributedNetAgent::GetInstance().UnregisterNetSupplier();
    WearableDistributedNetAgent::GetInstance().netSupplierId_ = 0;
    isMetered = true;
    EXPECT_EQ(WearableDistributedNetAgent::GetInstance().RegisterNetSupplier(isMetered),
        NETMANAGER_ERR_PERMISSION_DENIED);
    WearableDistributedNetAgent::GetInstance().UnregisterNetSupplier();
}

HWTEST_F(WearableDistributedNetAgentTest, RegisterNetSupplier002, TestSize.Level1)
{
    bool isMetered = true;
    WearableDistributedNetAgent::GetInstance().netSupplierId_ = 0;
    EXPECT_EQ(WearableDistributedNetAgent::GetInstance().RegisterNetSupplier(isMetered),
        NETMANAGER_ERR_PERMISSION_DENIED);
    WearableDistributedNetAgent::GetInstance().UnregisterNetSupplier();
    isMetered = false;
    EXPECT_EQ(WearableDistributedNetAgent::GetInstance().RegisterNetSupplier(isMetered),
        NETMANAGER_ERR_PERMISSION_DENIED);
    WearableDistributedNetAgent::GetInstance().UnregisterNetSupplier();
}

HWTEST_F(WearableDistributedNetAgentTest, UnregisterNetSupplier001, TestSize.Level1)
{
    WearableDistributedNetAgent::GetInstance().netSupplierId_ = 0;
    int32_t result = WearableDistributedNetAgent::GetInstance().UnregisterNetSupplier();
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(WearableDistributedNetAgentTest, UnregisterNetSupplier002, TestSize.Level1)
{
    bool isMetered = true;
    WearableDistributedNetAgent::GetInstance().netSupplierId_ = 0;
    WearableDistributedNetAgent::GetInstance().RegisterNetSupplier(isMetered);
    int32_t result = WearableDistributedNetAgent::GetInstance().UnregisterNetSupplier();
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(WearableDistributedNetAgentTest, UnregisterNetSupplier003, TestSize.Level1)
{
    bool isMetered = true;
    WearableDistributedNetAgent::GetInstance().netSupplierId_ = 0;
    int32_t ret = WearableDistributedNetAgent::GetInstance().UnregisterNetSupplier();
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(WearableDistributedNetAgentTest, UpdateNetSupplierInfo001, TestSize.Level1)
{
    WearableDistributedNetAgent::GetInstance().netSupplierId_ = 0;
    EXPECT_NE(WearableDistributedNetAgent::GetInstance().UpdateNetSupplierInfo(true), NETMANAGER_SUCCESS);
    WearableDistributedNetAgent::GetInstance().netSupplierId_ = 1;
    EXPECT_EQ(WearableDistributedNetAgent::GetInstance().UpdateNetSupplierInfo(true),
        NETMANAGER_ERR_PERMISSION_DENIED);
}

HWTEST_F(WearableDistributedNetAgentTest, UpdateNetSupplierInfo002, TestSize.Level1)
{
    WearableDistributedNetAgent::GetInstance().netSupplierId_ = 1;
    EXPECT_NE(WearableDistributedNetAgent::GetInstance().UpdateNetSupplierInfo(true), NETMANAGER_SUCCESS);
}

HWTEST_F(WearableDistributedNetAgentTest, UpdateNetLinkInfo001, TestSize.Level1)
{
    WearableDistributedNetAgent::GetInstance().netSupplierId_ = 0;
    EXPECT_NE(WearableDistributedNetAgent::GetInstance().UpdateNetLinkInfo(), 0);
    WearableDistributedNetAgent::GetInstance().netSupplierId_ = 1;
    EXPECT_EQ(WearableDistributedNetAgent::GetInstance().UpdateNetLinkInfo(), NETMANAGER_ERR_PERMISSION_DENIED);
    WearableDistributedNetAgent::GetInstance().netSupplierId_ = 0;
}

HWTEST_F(WearableDistributedNetAgentTest, UpdateNetLinkInfo002, TestSize.Level1)
{
    WearableDistributedNetAgent::GetInstance().netSupplierId_ = 1;
    EXPECT_NE(WearableDistributedNetAgent::GetInstance().UpdateNetLinkInfo(), NETMANAGER_SUCCESS);
    WearableDistributedNetAgent::GetInstance().netSupplierId_ = 0;
}

HWTEST_F(WearableDistributedNetAgentTest, SetupWearableDistributedNetNetwork002, TestSize.Level1)
{
    int32_t tcpPortId = 8001;
    int32_t udpPortId = 0;
    bool isMetered = false;
    WearableDistributedNetAgent::GetInstance().netSupplierId_ = 1;
    int32_t result = WearableDistributedNetAgent::GetInstance()
        .SetupWearableDistributedNetwork(tcpPortId, udpPortId, isMetered);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
}

HWTEST_F(WearableDistributedNetAgentTest, SetInitNetScore001, TestSize.Level1)
{
    OHOS::PowerMgr::BatteryChargeState chargeState = OHOS::PowerMgr::BatteryChargeState::CHARGE_STATE_FULL;
    WearableDistributedNetAgent::GetInstance().SetInitNetScore(chargeState);
    EXPECT_EQ(WearableDistributedNetAgent::GetInstance().score_, NET_SCORE_WITH_CHARGE_STATE);
}

HWTEST_F(WearableDistributedNetAgentTest, SetInitNetScore002, TestSize.Level1)
{
    OHOS::PowerMgr::BatteryChargeState chargeState = OHOS::PowerMgr::BatteryChargeState::CHARGE_STATE_DISABLE;
    WearableDistributedNetAgent::GetInstance().SetInitNetScore(chargeState);
    EXPECT_EQ(WearableDistributedNetAgent::GetInstance().score_, NET_SCORE_WITH_UNCHARGE_STATE);
}

HWTEST_F(WearableDistributedNetAgentTest, SetInitNetScore003, TestSize.Level1)
{
    OHOS::PowerMgr::BatteryChargeState chargeState = OHOS::PowerMgr::BatteryChargeState::CHARGE_STATE_NONE;
    WearableDistributedNetAgent::GetInstance().SetInitNetScore(chargeState);
    EXPECT_EQ(WearableDistributedNetAgent::GetInstance().score_, NET_SCORE_WITH_UNCHARGE_STATE);
}

HWTEST_F(WearableDistributedNetAgentTest, SetScoreBaseNetStatus001, TestSize.Level1)
{
    bool isAvailable = true;
    WearableDistributedNetAgent::GetInstance().SetScoreBaseNetStatus(isAvailable);
    EXPECT_EQ(WearableDistributedNetAgent::GetInstance().netSupplierInfo_.score_, NET_SCORE_WITH_CHARGE_STATE);
}

HWTEST_F(WearableDistributedNetAgentTest, SetScoreBaseNetStatus002, TestSize.Level1)
{
    bool isAvailable = false;
    WearableDistributedNetAgent::GetInstance().SetScoreBaseNetStatus(isAvailable);
    EXPECT_EQ(WearableDistributedNetAgent::GetInstance().netSupplierInfo_.score_, 0);
}

HWTEST_F(WearableDistributedNetAgentTest, UpdateNetScore001, TestSize.Level1)
{
    bool isCharging = true;
    WearableDistributedNetAgent::GetInstance().netSupplierId_ = 0;
    int32_t result = WearableDistributedNetAgent::GetInstance().UpdateNetScore(isCharging);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(WearableDistributedNetAgentTest, UpdateNetScore002, TestSize.Level1)
{
    bool isCharging = false;
    WearableDistributedNetAgent::GetInstance().netSupplierId_ = 1;
    int32_t result = WearableDistributedNetAgent::GetInstance().UpdateNetScore(isCharging);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(WearableDistributedNetAgentTest, UpdateNetCaps001, TestSize.Level1)
{
    bool isMetered = false;
    int32_t result = WearableDistributedNetAgent::GetInstance().UpdateNetCaps(isMetered);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
}

HWTEST_F(WearableDistributedNetAgentTest, UpdateMeteredStatus, TestSize.Level1)
{
    bool isMetered = true;
    int32_t result = WearableDistributedNetAgent::GetInstance().UpdateMeteredStatus(isMetered);
    EXPECT_EQ(result, NETMANAGER_ERR_PERMISSION_DENIED);
}
} // namespace NetManagerStandard
} // namespace OHOS
