/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "net_quota_policy.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
static constexpr const char *ICCID_1 = "sim_abcdefg_1";
using namespace testing::ext;
NetQuotaPolicy GetQuota()
{
    NetQuotaPolicy policy;
    policy.networkmatchrule.simId = "testIccid";
    policy.networkmatchrule.ident = "testIdent";
    policy.quotapolicy.title = "testTitle";
    return policy;
}
} // namespace
class UtNetQuotaPolicy : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void UtNetQuotaPolicy::SetUpTestCase() {}

void UtNetQuotaPolicy::TearDownTestCase() {}

void UtNetQuotaPolicy::SetUp() {}

void UtNetQuotaPolicy::TearDown() {}

/**
 * @tc.name: GetPeriodStart001
 * @tc.desc: Test NetPolicyQuota GetPeriodStart.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetQuotaPolicy, GetPeriodStart001, TestSize.Level1)
{
    NetQuotaPolicy netQuotaPolicy1;
    netQuotaPolicy1.networkmatchrule.simId = ICCID_1;
    netQuotaPolicy1.quotapolicy.periodDuration = "M1";
    auto result = netQuotaPolicy1.GetPeriodStart();
    std::cout << "result1:" << result << std::endl;

    NetQuotaPolicy netQuotaPolicy2;
    netQuotaPolicy2.networkmatchrule.simId = ICCID_1;
    netQuotaPolicy2.quotapolicy.periodDuration = "Y1";
    auto result2 = netQuotaPolicy2.GetPeriodStart();
    std::cout << "result2:" << result2 << std::endl;

    NetQuotaPolicy netQuotaPolicy3;
    netQuotaPolicy3.networkmatchrule.simId = ICCID_1;
    netQuotaPolicy3.quotapolicy.periodDuration = "D1";
    auto result3 = netQuotaPolicy3.GetPeriodStart();
    std::cout << "result3:" << result3 << std::endl;

    ASSERT_TRUE(result > 0);
    ASSERT_TRUE(result2 > 0);
    ASSERT_TRUE(result3 > 0);
}

/**
 * @tc.name: Marshalling001
 * @tc.desc: Test NetPolicyQuota Marshalling.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetQuotaPolicy, Marshalling001, TestSize.Level1)
{
    Parcel parcel;
    NetQuotaPolicy netQuotaPolicy = GetQuota();
    bool ret = netQuotaPolicy.Marshalling(parcel);
    ASSERT_TRUE(ret);
    NetQuotaPolicy recv1;
    ret = NetQuotaPolicy::Unmarshalling(parcel, recv1);
    ASSERT_TRUE(ret);
    EXPECT_EQ(recv1.networkmatchrule.simId, netQuotaPolicy.networkmatchrule.simId);
    EXPECT_EQ(recv1.networkmatchrule.ident, netQuotaPolicy.networkmatchrule.ident);
}

/**
 * @tc.name: Marshalling002
 * @tc.desc: Test NetPolicyQuota Marshalling.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetQuotaPolicy, Marshalling002, TestSize.Level1)

{
    Parcel parcel;
    NetQuotaPolicy data = GetQuota();
    bool ret = NetQuotaPolicy::Marshalling(parcel, data);
    ASSERT_TRUE(ret);
    NetQuotaPolicy recv;
    ret = NetQuotaPolicy::Unmarshalling(parcel, recv);
    ASSERT_TRUE(ret);
    EXPECT_EQ(recv.networkmatchrule.simId, data.networkmatchrule.simId);
    EXPECT_EQ(recv.networkmatchrule.ident, data.networkmatchrule.ident);
}

/**
 * @tc.name: Marshalling003
 * @tc.desc: Test NetPolicyQuota Marshalling.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetQuotaPolicy, Marshalling003, TestSize.Level1)
{
    Parcel parcel;
    std::vector<NetQuotaPolicy> data;
    const uint32_t dataSize = 12;
    for (uint32_t i = 0; i < dataSize; i++) {
        data.push_back(GetQuota());
    }
    bool ret = NetQuotaPolicy::Marshalling(parcel, data);
    ASSERT_TRUE(ret);
    std::vector<NetQuotaPolicy> recv;
    ret = NetQuotaPolicy::Unmarshalling(parcel, recv);
    ASSERT_TRUE(ret);
    std::for_each(recv.begin(), recv.end(), [this](const auto &cv) {
        EXPECT_EQ(cv.networkmatchrule.simId, GetQuota().networkmatchrule.simId);
        EXPECT_EQ(cv.networkmatchrule.ident, GetQuota().networkmatchrule.ident);
    });
}
} // namespace NetManagerStandard
} // namespace OHOS
