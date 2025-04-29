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
#include "vpn_config.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
}

class VpnConfigTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void VpnConfigTest::SetUpTestCase() {};

void VpnConfigTest::TearDownTestCase() {};

void VpnConfigTest::SetUp() {};

void VpnConfigTest::TearDown() {};

HWTEST_F(VpnConfigTest, MarshallingAddrRouteTest001, TestSize.Level1)
{
    VpnConfig vpnconfig;
    Parcel parcel;
    INetAddr inetaddr;
    inetaddr.address_ = "123";
    vpnconfig.addresses_.push_back(inetaddr);
    Route route;
    route.iface_ = "123";
    vpnconfig.routes_.push_back(route);
    auto result = vpnconfig.MarshallingAddrRoute(parcel);
    EXPECT_EQ(result, true);
}

HWTEST_F(VpnConfigTest, MarshallingVectorStringTest001, TestSize.Level1)
{
    VpnConfig vpnconfig;
    Parcel parcel;
    std::vector<std::string> vec = {"123"};
    auto result = vpnconfig.MarshallingVectorString(parcel, vec);
    EXPECT_EQ(result, true);
}

HWTEST_F(VpnConfigTest, UnmarshallingTest001, TestSize.Level1)
{
    VpnConfig vpnconfig;
    Parcel parcel;
    auto result = vpnconfig.Unmarshalling(parcel);
    EXPECT_EQ(result, nullptr);
}

HWTEST_F(VpnConfigTest, UnmarshallingVpnConfigTest001, TestSize.Level1)
{
    VpnConfig vpnconfig;
    Parcel parcel;
    auto result = vpnconfig.UnmarshallingVpnConfig(parcel, nullptr);
    EXPECT_EQ(result, false);
}

HWTEST_F(VpnConfigTest, UnmarshallingAddrRouteTest001, TestSize.Level1)
{
    VpnConfig vpnconfig;
    Parcel parcel;
    int32_t largeAddrSize = 4097;
    parcel.WriteInt32(largeAddrSize);
    auto result = vpnconfig.UnmarshallingAddrRoute(parcel, &vpnconfig);
    EXPECT_EQ(result, false);
}

HWTEST_F(VpnConfigTest, UnmarshallingAddrRouteTest002, TestSize.Level1)
{
    VpnConfig vpnconfig;
    Parcel parcel;
    int32_t addrSize = 1;
    parcel.WriteInt32(addrSize);
    parcel.WriteInt32(-1);
    auto result = vpnconfig.UnmarshallingAddrRoute(parcel, &vpnconfig);
    EXPECT_EQ(result, false);
}

HWTEST_F(VpnConfigTest, UnmarshallingAddrRouteTest003, TestSize.Level1)
{
    VpnConfig vpnconfig;
    Parcel parcel;
    int32_t addrSize = 1;
    parcel.WriteInt32(addrSize);
    sptr<INetAddr> validAddress = new INetAddr();
    validAddress->type_ = INetAddr::IPV4;
    validAddress->address_ = "192.168.1.1";
    validAddress->Marshalling(parcel);

    int32_t routeSize = 1;
    parcel.WriteInt32(routeSize);
    parcel.WriteInt32(-1);
    auto result = vpnconfig.UnmarshallingAddrRoute(parcel, &vpnconfig);
    EXPECT_EQ(result, false);
}

HWTEST_F(VpnConfigTest, UnmarshallingAddrRouteTest004, TestSize.Level1)
{
    VpnConfig vpnconfig;
    Parcel parcel;
    int32_t addrSize = 1;
    parcel.WriteInt32(addrSize);
    sptr<INetAddr> validAddress = new INetAddr();
    validAddress->type_ = INetAddr::IPV4;
    validAddress->address_ = "192.168.1.1";
    validAddress->Marshalling(parcel);

    int32_t largeRouteSize = 2001;
    parcel.WriteInt32(largeRouteSize);
    auto result = vpnconfig.UnmarshallingAddrRoute(parcel, &vpnconfig);
    EXPECT_EQ(result, false);
}

HWTEST_F(VpnConfigTest, UnmarshallingAddrRouteTest005, TestSize.Level1)
{
    VpnConfig vpnconfig;
    Parcel parcel;

    int32_t addrSize = 1;
    parcel.WriteInt32(addrSize);

    sptr<INetAddr> validAddress1 = new INetAddr();
    validAddress1->type_ = INetAddr::IPV4;
    validAddress1->address_ = "192.168.1.1";
    validAddress1->Marshalling(parcel);

    int32_t routeSize = 1;
    parcel.WriteInt32(routeSize);

    sptr<INetAddr> destination = new INetAddr();
    destination->type_ = INetAddr::IPV4;
    destination->address_ = "0.0.0.0";

    sptr<INetAddr> gateway = new INetAddr();
    gateway->type_ = INetAddr::IPV4;
    gateway->address_ = "192.168.1.1";

    sptr<Route> validRoute1 = new Route();
    validRoute1->iface_ = "eth0";
    validRoute1->destination_ = *destination;
    validRoute1->gateway_ = *gateway;
    validRoute1->rtnType_ = RTN_UNICAST;
    validRoute1->mtu_ = 1500;
    validRoute1->isHost_ = false;
    validRoute1->hasGateway_ = true;
    validRoute1->isDefaultRoute_ = true;

    validRoute1->Marshalling(parcel);

    auto result = vpnconfig.UnmarshallingAddrRoute(parcel, &vpnconfig);

    EXPECT_EQ(result, true);
    EXPECT_EQ(vpnconfig.addresses_.size(), 1);
    EXPECT_EQ(vpnconfig.addresses_[0].address_, "192.168.1.1");
    EXPECT_EQ(vpnconfig.routes_.size(), 1);
    EXPECT_EQ(vpnconfig.routes_[0].iface_, "eth0");
    EXPECT_EQ(vpnconfig.routes_[0].destination_.address_, "0.0.0.0");
    EXPECT_EQ(vpnconfig.routes_[0].gateway_.address_, "192.168.1.1");
    EXPECT_EQ(vpnconfig.routes_[0].rtnType_, RTN_UNICAST);
    EXPECT_EQ(vpnconfig.routes_[0].mtu_, 1500);
    EXPECT_EQ(vpnconfig.routes_[0].isHost_, false);
    EXPECT_EQ(vpnconfig.routes_[0].hasGateway_, true);
    EXPECT_EQ(vpnconfig.routes_[0].isDefaultRoute_, true);
}

HWTEST_F(VpnConfigTest, UnmarshallingVectorStringTest001, TestSize.Level1)
{
    VpnConfig vpnconfig;
    Parcel parcel;
    std::vector<std::string> vec;

    uint32_t largeSize = 4097;
    parcel.WriteInt32(largeSize);

    auto result = vpnconfig.UnmarshallingVectorString(parcel, vec);
    EXPECT_EQ(result, false);
    EXPECT_EQ(vec.size(), 0);
}

HWTEST_F(VpnConfigTest, UnmarshallingVectorStringTest002, TestSize.Level1)
{
    VpnConfig vpnconfig;
    Parcel parcel;
    std::vector<std::string> vec;

    int32_t size = 2;
    parcel.WriteInt32(size);

    std::string str1 = "string1";
    std::string str2 = "string2";
    parcel.WriteString(str1);
    parcel.WriteString(str2);

    auto result = vpnconfig.UnmarshallingVectorString(parcel, vec);
    EXPECT_EQ(result, true);
    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(vec[0], str1);
    EXPECT_EQ(vec[1], str2);
}
}
}