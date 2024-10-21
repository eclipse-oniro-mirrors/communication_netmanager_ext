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
#include "wearable_distributed_net_supplier_info.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
} // namesapce

class WearableDistributedNetSupplierInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();

    void SetUp();
    void TearDown();
};

void WearableDistributedNetSupplierInfoTest::SetUpTestCase() {}

void WearableDistributedNetSupplierInfoTest::TearDownTestCase() {}

void WearableDistributedNetSupplierInfoTest::SetUp() {}

void WearableDistributedNetSupplierInfoTest::TearDown() {}

HWTEST_F(WearableDistributedNetSupplierInfoTest, SetAvailable, TestSize.Level1)
{
    WearableDistributedNetSupplierInfo info;
    sptr<NetSupplierInfo> supplierInfo = new (std::nothrow) NetSupplierInfo();
    info.SetAvailable(supplierInfo);
    EXPECT_EQ(supplierInfo->isAvailable_, CONSTANTS::AVALIABLE);
}

HWTEST_F(WearableDistributedNetSupplierInfoTest, SetRoamingStatus, TestSize.Level1)
{
    WearableDistributedNetSupplierInfo info;
    sptr<NetSupplierInfo> supplierInfo = new (std::nothrow) NetSupplierInfo();
    info.SetRoamingStatus(supplierInfo);
    EXPECT_EQ(supplierInfo->isRoaming_, CONSTANTS::ROAMING_STATUS);
}

HWTEST_F(WearableDistributedNetSupplierInfoTest, SetLinkUpBandwidthKbps, TestSize.Level1)
{
    WearableDistributedNetSupplierInfo info;
    sptr<NetSupplierInfo> supplierInfo = new (std::nothrow) NetSupplierInfo();
    info.SetLinkUpBandwidthKbps(supplierInfo);
    EXPECT_EQ(supplierInfo->linkUpBandwidthKbps_, CONSTANTS::LINKUP_BAND_WIDTH_KBPS);
}

HWTEST_F(WearableDistributedNetSupplierInfoTest, SetLinkDownBandwidthKbps, TestSize.Level1)
{
    WearableDistributedNetSupplierInfo info;
    sptr<NetSupplierInfo> supplierInfo = new (std::nothrow) NetSupplierInfo();
    info.SetLinkDownBandwidthKbps(supplierInfo);
    EXPECT_EQ(supplierInfo->linkDownBandwidthKbps_, CONSTANTS::LINKDOWN_BAND_WIDTH_KBPS);
}
} // namespace NetManagerStandard
} // namespace OHOS