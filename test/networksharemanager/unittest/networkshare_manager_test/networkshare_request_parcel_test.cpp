/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "network_share_address_parcel.h"
#include "network_share_request_parcel.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr int32_t NET_WORK_SHARE_TYPE = 1;
constexpr int32_t CONNECTIVITY_SCOPE = 2;
NetworkShareRequestParcel GetNetworkShareRequestData()
{
    NetworkShareRequestParcel info;
    info.networkShareType_ = NET_WORK_SHARE_TYPE;
    info.connectivityScope_ = CONNECTIVITY_SCOPE;
    info.exemptFromEntitlementCheck_ = true;
    info.showProvisioningUi_ = true;
    return info;
}
} // namespace
using namespace testing::ext;
class NetWorkShareRequestParcelTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetWorkShareRequestParcelTest::SetUpTestCase() {}

void NetWorkShareRequestParcelTest::TearDownTestCase() {}

void NetWorkShareRequestParcelTest::SetUp() {}

void NetWorkShareRequestParcelTest::TearDown() {}

/**
 * @tc.name: NetWorkShareRequestParcelTest
 * @tc.desc: Test NetworkShareRequestParcel NetWorkShareRequestParcel01.
 * @tc.type: FUNC
 */
HWTEST_F(NetWorkShareRequestParcelTest, NetWorkShareRequestParcel01, TestSize.Level1)
{
    Parcel parcel;
    NetworkShareRequestParcel data = GetNetworkShareRequestData();
    EXPECT_TRUE(data.Marshalling(parcel));
    auto result = NetworkShareRequestParcel::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->networkShareType_, data.networkShareType_);
    EXPECT_EQ(result->connectivityScope_, data.connectivityScope_);
    EXPECT_EQ(result->exemptFromEntitlementCheck_, data.exemptFromEntitlementCheck_);
    EXPECT_EQ(result->showProvisioningUi_, data.showProvisioningUi_);
}

/**
 * @tc.name: NetworkShareAddressParcel01
 * @tc.desc: Test NetworkShareAddressParcel Marshalling and Unmarshalling.
 * @tc.type: FUNC
 */
HWTEST_F(NetWorkShareRequestParcelTest, NetworkShareAddressParcel01, TestSize.Level1)
{
    Parcel parcel;
    NetworkShareAddressParcel data;
    INetAddr ipv4Addr;
    ipv4Addr.type_ = INetAddr::IPV4;
    ipv4Addr.family_ = INetAddr::IPV4;
    ipv4Addr.address_ = "192.168.43.1";
    INetAddr ipv6Addr;
    ipv6Addr.type_ = INetAddr::IPV6;
    ipv6Addr.family_ = INetAddr::IPV6;
    ipv6Addr.address_ = "2001:db8::1";
    data.addresses_.push_back(ipv4Addr);
    data.addresses_.push_back(ipv6Addr);
    EXPECT_TRUE(data.Marshalling(parcel));
    std::unique_ptr<NetworkShareAddressParcel> result(NetworkShareAddressParcel::Unmarshalling(parcel));
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->addresses_.size(), data.addresses_.size());
    EXPECT_EQ(result->addresses_[0].address_, ipv4Addr.address_);
    EXPECT_EQ(result->addresses_[0].family_, INetAddr::IPV4);
    EXPECT_EQ(result->addresses_[1].address_, ipv6Addr.address_);
    EXPECT_EQ(result->addresses_[1].family_, INetAddr::IPV6);
}

/**
 * @tc.name: NetworkShareAddressParcel02
 * @tc.desc: Test NetworkShareAddressParcel Unmarshalling with empty parcel.
 * @tc.type: FUNC
 */
HWTEST_F(NetWorkShareRequestParcelTest, NetworkShareAddressParcel02, TestSize.Level1)
{
    Parcel parcel;
    std::unique_ptr<NetworkShareAddressParcel> result(NetworkShareAddressParcel::Unmarshalling(parcel));
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: NetworkShareAddressParcel03
 * @tc.desc: Test NetworkShareAddressParcel Unmarshalling with size but without address data.
 * @tc.type: FUNC
 */
HWTEST_F(NetWorkShareRequestParcelTest, NetworkShareAddressParcel03, TestSize.Level1)
{
    Parcel parcel;
    EXPECT_TRUE(parcel.WriteUint32(1));
    std::unique_ptr<NetworkShareAddressParcel> result(NetworkShareAddressParcel::Unmarshalling(parcel));
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: NetworkShareAddressParcel04
 * @tc.desc: Test NetworkShareAddressParcel Marshalling and Unmarshalling with addresses over the max size.
 * @tc.type: FUNC
 */
HWTEST_F(NetWorkShareRequestParcelTest, NetworkShareAddressParcel04, TestSize.Level1)
{
    constexpr uint32_t maxSize = NetworkShareAddressParcel::MAX_ADDRESS_SIZE;
    Parcel parcel;
    NetworkShareAddressParcel data;
    INetAddr netAddr;
    netAddr.type_ = INetAddr::IPV4;
    netAddr.family_ = INetAddr::IPV4;
    netAddr.address_ = "192.168.43.1";
    data.addresses_.assign(maxSize + 1, netAddr);
    EXPECT_TRUE(data.Marshalling(parcel));
    std::unique_ptr<NetworkShareAddressParcel> result(NetworkShareAddressParcel::Unmarshalling(parcel));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->addresses_.size(), maxSize);
}

/**
 * @tc.name: NetworkShareAddressParcel05
 * @tc.desc: Test NetworkShareAddressParcel Unmarshalling with size over the max size but without address data.
 * @tc.type: FUNC
 */
HWTEST_F(NetWorkShareRequestParcelTest, NetworkShareAddressParcel05, TestSize.Level1)
{
    constexpr uint32_t overSize = 1000;
    Parcel parcel;
    EXPECT_TRUE(parcel.WriteUint32(overSize));
    std::unique_ptr<NetworkShareAddressParcel> result(NetworkShareAddressParcel::Unmarshalling(parcel));
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: NetworkShareAddressParcel06
 * @tc.desc: Test NetworkShareAddressParcel Unmarshalling clamps size over the max size with enough address data.
 * @tc.type: FUNC
 */
HWTEST_F(NetWorkShareRequestParcelTest, NetworkShareAddressParcel06, TestSize.Level1)
{
    constexpr uint32_t maxSize = NetworkShareAddressParcel::MAX_ADDRESS_SIZE;
    constexpr uint32_t overSize = 1000;
    Parcel parcel;
    INetAddr netAddr;
    netAddr.type_ = INetAddr::IPV4;
    netAddr.family_ = INetAddr::IPV4;
    netAddr.address_ = "192.168.43.1";
    EXPECT_TRUE(parcel.WriteUint32(overSize));
    for (uint32_t index = 0; index < maxSize + 1; ++index) {
        ASSERT_TRUE(netAddr.Marshalling(parcel));
    }
    std::unique_ptr<NetworkShareAddressParcel> result(NetworkShareAddressParcel::Unmarshalling(parcel));
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->addresses_.size(), maxSize);
    EXPECT_EQ(result->addresses_[0].address_, netAddr.address_);
}
} // namespace NetManagerStandard
} // namespace OHOS