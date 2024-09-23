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

#include "vpn_config.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;

constexpr uint32_t MAX_SIZE = 65;
} // namespace

class VpnConfigTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void VpnConfigTest::SetUpTestCase() {}

void VpnConfigTest::TearDownTestCase() {}

void VpnConfigTest::SetUp() {}

void VpnConfigTest::TearDown() {}

HWTEST_F(VpnConfigTest, UnmarshallingAddrRoute001, TestSize.Level1)
{
    Parcel parcel;
    VpnConfig info;
    info.isAcceptIPv4_ = true;
    INetAddr ipv4Addr;
    ipv4Addr.type_ = INetAddr::IPV4;
    ipv4Addr.family_ = 0x01;
    ipv4Addr.prefixlen_ = 0x01;
    ipv4Addr.address_ = "172.17.5.234";
    ipv4Addr.netMask_ = "255.255.254.0";
    ipv4Addr.hostName_ = "netAddr";
    info.addresses_.push_back(ipv4Addr);
    Route route1;
    info.routes_.push_back(route1);
    EXPECT_TRUE(info.MarshallingAddrRoute(parcel));
    sptr<VpnConfig> ipsecConfig = new (std::nothrow) VpnConfig();
    EXPECT_TRUE(VpnConfig::UnmarshallingAddrRoute(parcel, ipsecConfig));
}

HWTEST_F(VpnConfigTest, UnmarshallingAddrRoute002, TestSize.Level1)
{
    Parcel parcel;
    VpnConfig info;
    INetAddr ipv4Addr;
    ipv4Addr.type_ = INetAddr::IPV4;
    ipv4Addr.family_ = 0x01;
    ipv4Addr.prefixlen_ = 0x01;
    ipv4Addr.netMask_ = "255.255.254.0";
    ipv4Addr.hostName_ = "netAddr";
    for (int32_t i = 0; i < MAX_SIZE; i++) {
        ipv4Addr.address_ = "172.17.5." + std::to_string(i);
        info.addresses_.push_back(ipv4Addr);
    }
    EXPECT_TRUE(info.MarshallingAddrRoute(parcel));
    sptr<VpnConfig> ipsecConfig = new (std::nothrow) VpnConfig();
    ASSERT_TRUE(ipsecConfig != nullptr);
    EXPECT_FALSE(VpnConfig::UnmarshallingAddrRoute(parcel, ipsecConfig));
}

HWTEST_F(VpnConfigTest, UnmarshallingAddrRoute003, TestSize.Level1)
{
    Parcel parcel;
    VpnConfig info;
    INetAddr ipv4Addr;
    ipv4Addr.type_ = INetAddr::IPV4;
    ipv4Addr.family_ = 0x01;
    ipv4Addr.prefixlen_ = 0x01;
    ipv4Addr.netMask_ = "255.255.254.0";
    ipv4Addr.hostName_ = "netAddr";
    ipv4Addr.address_ = "172.17.5.1";
    info.addresses_.push_back(ipv4Addr);
    Route route;
    for (int32_t i = 0; i < MAX_SIZE; i++) {
        route.iface_ = "ifaceTest" + std::to_string(i);
        info.routes_.push_back(route);
    }
    EXPECT_TRUE(info.MarshallingAddrRoute(parcel));
    sptr<VpnConfig> ipsecConfig = new (std::nothrow) VpnConfig();
    ASSERT_TRUE(ipsecConfig != nullptr);
    EXPECT_FALSE(VpnConfig::UnmarshallingAddrRoute(parcel, ipsecConfig));
}

HWTEST_F(VpnConfigTest, UnmarshallingVectorString001, TestSize.Level1)
{
    Parcel parcel;
    VpnConfig info;
    std::vector<std::string> vec = { "vecTest" };
    EXPECT_TRUE(info.MarshallingVectorString(parcel, vec));
    std::vector<std::string> vec1;
    EXPECT_TRUE(VpnConfig::UnmarshallingVectorString(parcel, vec1));
}

HWTEST_F(VpnConfigTest, UnmarshallingVectorString002, TestSize.Level1)
{
    Parcel parcel;
    VpnConfig info;
    std::vector<std::string> vec;
    for (int32_t i = 0; i < MAX_SIZE; i++) {
        std::string value = "vecTest" + std::to_string(i);
        vec.push_back(value);
    }
    EXPECT_TRUE(info.MarshallingVectorString(parcel, vec));
    std::vector<std::string> vec1;
    EXPECT_FALSE(VpnConfig::UnmarshallingVectorString(parcel, vec1));
}
} // namespace NetManagerStandard
} // namespace OHOS