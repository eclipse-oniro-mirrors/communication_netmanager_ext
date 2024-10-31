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
#include "net_manager_constants.h"

#define private public
#include "wearable_distributed_net_static_configuration.h"
#undef private

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
} // namesapce

class WearableDistributedNetStaticConfigurationTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();

    void SetUp();
    void TearDown();
};

void WearableDistributedNetStaticConfigurationTest::SetUpTestCase() {}

void WearableDistributedNetStaticConfigurationTest::TearDownTestCase() {}

void WearableDistributedNetStaticConfigurationTest::SetUp() {}

void WearableDistributedNetStaticConfigurationTest::TearDown() {}

HWTEST_F(WearableDistributedNetStaticConfigurationTest, SetNetLinkInfo, TestSize.Level1)
{
    WearableDistributedNetStaticConfiguration wearableDistributedNetStaticConfig;
    NetSupplierInfo supplierInfo;
    SetNetSupplierInfo(supplierInfo);
    NetLinkInfo linkInfo;
    EXPECT_EQ(wearableDistributedNetStaticConfig.SetNetLinkInfo(linkInfo), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(WearableDistributedNetStaticConfigurationTest, GetNetLinkInfo, TestSize.Level1)
{
    WearableDistributedNetStaticConfiguration wearableDistributedNetStaticConfig;
    NetLinkInfo linkInfo;
    int32_t ret = wearableDistributedNetStaticConfig.GetNetLinkInfo(linkInfo);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(WearableDistributedNetStaticConfigurationTest, GetNetSupplierInfo, TestSize.Level1)
{
    WearableDistributedNetStaticConfiguration wearableDistributedNetStaticConfig;
    NetSupplierInfo supplierInfo;
    wearableDistributedNetStaticConfig.GetNetSupplierInfo(supplierInfo);
    EXPECT_EQ(supplierInfo.isAvailable_, CONSTANTS::AVALIABLE);
}

HWTEST_F(WearableDistributedNetStaticConfigurationTest, WearableDistributedNetStaticConfiguration001, TestSize.Level1)
{
    NetSupplierInfo supplierInfo;
    supplierInfo.isAvailable_ = true;
    supplierInfo.isRoaming_ = false;
    supplierInfo.linkUpBandwidthKbps_ = 220;
    supplierInfo.linkDownBandwidthKbps_ = 220;

    WearableDistributedNetStaticConfiguration wearableDistributedNetStaticConfig;

    NetLinkInfo retrievedLinkInfo;
    wearableDistributedNetStaticConfig.GetNetLinkInfo(retrievedLinkInfo);
    EXPECT_EQ(retrievedLinkInfo.ifaceName_, "lo");
    EXPECT_EQ(retrievedLinkInfo.mtu_, 1500);
    EXPECT_EQ(retrievedLinkInfo.dnsList_.front().address_, "114.114.114.114");
    EXPECT_EQ(retrievedLinkInfo.dnsList_.front().type_, INetAddr::IPV4);

    auto it = std::next(retrievedLinkInfo.dnsList_.begin());
    EXPECT_EQ(it->address_, "114.114.115.115");
    EXPECT_EQ(it->type_, INetAddr::IPV4);

    EXPECT_EQ(retrievedLinkInfo.netAddrList_.front().address_, "127.0.0.2");
    EXPECT_EQ(retrievedLinkInfo.routeList_.front().destination_.address_, "0.0.0.0");
    EXPECT_EQ(retrievedLinkInfo.routeList_.front().gateway_.address_, "127.0.0.1");

    NetSupplierInfo retrievedSupplierInfo;
    wearableDistributedNetStaticConfig.GetNetSupplierInfo(retrievedSupplierInfo);

    EXPECT_EQ(supplierInfo.isAvailable_, retrievedSupplierInfo.isAvailable_);
    EXPECT_EQ(supplierInfo.isRoaming_, retrievedSupplierInfo.isRoaming_);
    EXPECT_EQ(supplierInfo.linkUpBandwidthKbps_, retrievedSupplierInfo.linkUpBandwidthKbps_);
    EXPECT_EQ(supplierInfo.linkDownBandwidthKbps_, retrievedSupplierInfo.linkDownBandwidthKbps_);
}

HWTEST_F(WearableDistributedNetStaticConfigurationTest, EnableWearableDistributedNetForward001, TestSize.Level1)
{
    WearableDistributedNetStaticConfiguration wearableDistributedNetStaticConfig;

    int32_t ret = wearableDistributedNetStaticConfig.EnableWearableDistributedNetForward(8080, 8081);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    wearableDistributedNetStaticConfig.DisableWearableDistributedNetForward();
}

HWTEST_F(WearableDistributedNetStaticConfigurationTest, EnableWearableDistributedNetForward002, TestSize.Level1)
{
    WearableDistributedNetStaticConfiguration wearableDistributedNetStaticConfig;
    int32_t ret = wearableDistributedNetStaticConfig.EnableWearableDistributedNetForward(0, 8081);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INVALID_PARAMETER);
    wearableDistributedNetStaticConfig.DisableWearableDistributedNetForward();
}

HWTEST_F(WearableDistributedNetStaticConfigurationTest, EnableWearableDistributedNetForward003, TestSize.Level1)
{
    WearableDistributedNetStaticConfiguration wearableDistributedNetStaticConfig;
    int32_t ret = wearableDistributedNetStaticConfig.EnableWearableDistributedNetForward(65536, 8081);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INVALID_PARAMETER);
    wearableDistributedNetStaticConfig.DisableWearableDistributedNetForward();
}

HWTEST_F(WearableDistributedNetStaticConfigurationTest, EnableWearableDistributedNetForward004, TestSize.Level1)
{
    WearableDistributedNetStaticConfiguration wearableDistributedNetStaticConfig;
    int32_t ret = wearableDistributedNetStaticConfig.EnableWearableDistributedNetForward(8080, 0);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INVALID_PARAMETER);
    wearableDistributedNetStaticConfig.DisableWearableDistributedNetForward();
}

HWTEST_F(WearableDistributedNetStaticConfigurationTest, EnableWearableDistributedNetForward005, TestSize.Level1)
{
    WearableDistributedNetStaticConfiguration wearableDistributedNetStaticConfig;
    int32_t ret = wearableDistributedNetStaticConfig.EnableWearableDistributedNetForward(8080, 65536);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INVALID_PARAMETER);
    wearableDistributedNetStaticConfig.DisableWearableDistributedNetForward();
}

HWTEST_F(WearableDistributedNetStaticConfigurationTest, DisableWearableDistributedNetForward, TestSize.Level1)
{
    WearableDistributedNetStaticConfiguration wearableDistributedNetStaticConfig;
    int32_t ret = wearableDistributedNetStaticConfig.DisableWearableDistributedNetForward();
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(WearableDistributedNetStaticConfigurationTest, GetNetCaps, TestSize.Level1)
{
    WearableDistributedNetStaticConfiguration wearableDistributedNetStaticConfig;
    std::set<NetCap> capsMetered;
    std::set<NetCap> capsNotMetered;
    capsMetered.insert(NET_CAPABILITY_INTERNET);
    capsMetered.insert(NET_CAPABILITY_NOT_VPN);
    capsNotMetered = capsMetered;
    capsNotMetered.insert(NET_CAPABILITY_NOT_METERED);

    std::set<NetCap> getNetCaps = wearableDistributedNetStaticConfig.GetNetCaps(false);
    EXPECT_EQ(getNetCaps, capsNotMetered);

    getNetCaps = wearableDistributedNetStaticConfig.GetNetCaps(true);
    EXPECT_EQ(getNetCaps, capsMetered);
}
} // namespace NetManagerStandard
} // namespace OHOS
