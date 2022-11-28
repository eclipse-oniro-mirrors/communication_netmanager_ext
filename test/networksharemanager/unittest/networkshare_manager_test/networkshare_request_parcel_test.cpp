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
} // namespace NetManagerStandard
} // namespace OHOS