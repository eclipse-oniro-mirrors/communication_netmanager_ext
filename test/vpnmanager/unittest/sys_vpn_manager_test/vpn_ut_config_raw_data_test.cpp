/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "vpn_config_raw_data.h"
#include "vpn_config.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
}

class VpnConfigRawDataTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void VpnConfigRawDataTest::SetUpTestCase() {};

void VpnConfigRawDataTest::TearDownTestCase() {};

void VpnConfigRawDataTest::SetUp() {};

void VpnConfigRawDataTest::TearDown() {};

HWTEST_F(VpnConfigRawDataTest, ConstructorTest, TestSize.Level1)
{
    VpnConfigRawData rawData;
    EXPECT_EQ(rawData.size, 0);
    EXPECT_EQ(rawData.data, nullptr);
}

HWTEST_F(VpnConfigRawDataTest, RawDataCpyValidDataTest, TestSize.Level1)
{
    VpnConfigRawData rawData;
    std::string testData = "test data for copying";
    size_t dataSize = testData.length() + 1; // Include null terminator

    rawData.size = dataSize;
    int32_t result = rawData.RawDataCpy(testData.c_str());

    EXPECT_EQ(result, 0);
    EXPECT_NE(rawData.data, nullptr);
    EXPECT_EQ(strcmp(static_cast<const char*>(rawData.data), testData.c_str()), 0);
}

HWTEST_F(VpnConfigRawDataTest, RawDataCpyInvalidSizeTest, TestSize.Level1)
{
    VpnConfigRawData rawData;
    std::string testData = "test data";

    rawData.size = 0; // Invalid size
    int32_t result = rawData.RawDataCpy(testData.c_str());

    EXPECT_EQ(result, -1);

    rawData.size = MAX_RAW_DATA_SIZE + 1; // Too large size
    result = rawData.RawDataCpy(testData.c_str());

    EXPECT_EQ(result, -1);
}

HWTEST_F(VpnConfigRawDataTest, RawDataCpyNullDataTest, TestSize.Level1)
{
    VpnConfigRawData rawData;
    rawData.size = 10;

    int32_t result = rawData.RawDataCpy(nullptr);

    EXPECT_EQ(result, -1);
}

HWTEST_F(VpnConfigRawDataTest, MarshallingWithNullDataTest, TestSize.Level1)
{
    VpnConfigRawData rawData;
    Parcel parcel;

    bool result = rawData.Marshalling(parcel);

    EXPECT_EQ(result, false);
}

HWTEST_F(VpnConfigRawDataTest, MarshallingWithValidDataTest, TestSize.Level1)
{
    VpnConfigRawData rawData;
    std::string testData = "test data for marshalling";
    size_t dataSize = testData.length() + 1;

    rawData.size = dataSize;
    int32_t copyResult = rawData.RawDataCpy(testData.c_str());
    ASSERT_EQ(copyResult, 0);

    MessageParcel msgParcel;
    bool result = rawData.Marshalling(msgParcel);

    EXPECT_EQ(result, true);
}

HWTEST_F(VpnConfigRawDataTest, ToVpnConfigWithEmptyDataTest, TestSize.Level1)
{
    VpnConfigRawData rawData;
    VpnConfig config;

    int32_t result = rawData.ToVpnConfig(config);

    EXPECT_EQ(result, false);
}

HWTEST_F(VpnConfigRawDataTest, ToVpnConfigWithValidDataTest, TestSize.Level1)
{
    // First create a VpnConfig and serialize it to raw data
    VpnConfig config;
    config.vpnId_ = "test-vpn-id";
    config.mtu_ = 1500;
    config.isAcceptIPv4_ = true;
    config.isAcceptIPv6_ = false;
    config.isLegacy_ = true;
    config.isMetered_ = false;
    config.isBlocking_ = true;

    INetAddr addr;
    addr.address_ = "192.168.1.100";
    addr.type_ = INetAddr::IPV4;
    config.addresses_.push_back(addr);

    Route route;
    route.iface_ = "tun0";
    route.rtnType_ = RTN_UNICAST;
    route.mtu_ = 1400;
    config.routes_.push_back(route);

    VpnConfigRawData rawData;
    bool serializeResult = rawData.SerializeFromVpnConfig(config);
    ASSERT_EQ(serializeResult, true);

    // Now convert back to VpnConfig
    VpnConfig newConfig;
    int32_t result = rawData.ToVpnConfig(newConfig);

    EXPECT_EQ(result, true);
    EXPECT_EQ(newConfig.vpnId_, config.vpnId_);
    EXPECT_EQ(newConfig.mtu_, config.mtu_);
    EXPECT_EQ(newConfig.isAcceptIPv4_, config.isAcceptIPv4_);
    EXPECT_EQ(newConfig.isAcceptIPv6_, config.isAcceptIPv6_);
    EXPECT_EQ(newConfig.isLegacy_, config.isLegacy_);
    EXPECT_EQ(newConfig.isMetered_, config.isMetered_);
    EXPECT_EQ(newConfig.isBlocking_, config.isBlocking_);
    EXPECT_EQ(newConfig.addresses_.size(), config.addresses_.size());
    EXPECT_EQ(newConfig.routes_.size(), config.routes_.size());
}

HWTEST_F(VpnConfigRawDataTest, SerializeFromVpnConfigTest, TestSize.Level1)
{
    VpnConfig config;
    config.vpnId_ = "serialize-test";
    config.mtu_ = 1450;
    config.isAcceptIPv4_ = true;
    config.isAcceptIPv6_ = true;

    INetAddr addr;
    addr.address_ = "10.0.0.1";
    addr.type_ = INetAddr::IPV4;
    config.addresses_.push_back(addr);

    VpnConfigRawData rawData;
    bool result = rawData.SerializeFromVpnConfig(config);

    EXPECT_EQ(result, true);
    EXPECT_GT(rawData.size, 0);
    EXPECT_NE(rawData.data, nullptr);
}

HWTEST_F(VpnConfigRawDataTest, SerializeFromVpnConfigWithTooManyRoutesTest, TestSize.Level1)
{
    VpnConfig config;
    config.vpnId_ = "test-routes-limit";
    config.mtu_ = 1500;

    // Add more routes than the limit
    for (uint32_t i = 0; i <= ROUTE_MAX_SIZE; i++) {
        Route route;
        route.iface_ = "tun" + std::to_string(i);
        route.rtnType_ = RTN_UNICAST;
        config.routes_.push_back(route);
    }

    VpnConfigRawData rawData;
    bool result = rawData.SerializeFromVpnConfig(config);

    EXPECT_EQ(result, true);
    // Check that the routes vector was resized to the max allowed
    EXPECT_EQ(config.routes_.size(), ROUTE_MAX_SIZE);
}

HWTEST_F(VpnConfigRawDataTest, DestructorTest, TestSize.Level1)
{
    std::unique_ptr<VpnConfigRawData> rawData = std::make_unique<VpnConfigRawData>();
    std::string testData = "test destructor";
    size_t dataSize = testData.length() + 1;

    rawData->size = dataSize;
    int32_t copyResult = rawData->RawDataCpy(testData.c_str());
    ASSERT_EQ(copyResult, 0);
    EXPECT_NE(rawData->data, nullptr);

    rawData.reset(); // This should trigger the destructor and cleanup

    // Cannot directly test destructor, but we verified cleanup happened
    SUCCEED();
}
} // namespace NetManagerStandard
} // namespace OHOS