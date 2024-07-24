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

#include "netfirewall_common.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
}

class NetFirewallPolicyTest : public testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

class RequestParamTest : public testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

class FirewallRulePageTest : public testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

class InterceptRecordPageTest : public testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

HWTEST_F(NetFirewallPolicyTest, Unmarshalling001, TestSize.Level0)
{
    Parcel parcel;
    sptr<NetFirewallPolicy> ptr = NetFirewallPolicy::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(NetFirewallPolicyTest, Unmarshalling002, TestSize.Level0)
{
    Parcel parcel;
    bool isOpen = true;
    parcel.WriteBool(isOpen);
    sptr<NetFirewallPolicy> ptr = NetFirewallPolicy::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(NetFirewallPolicyTest, Unmarshalling003, TestSize.Level0)
{
    Parcel parcel;
    bool isOpen = true;
    parcel.WriteBool(isOpen);
    int32_t inAction = 1;
    parcel.WriteInt32(inAction);
    sptr<NetFirewallPolicy> ptr = NetFirewallPolicy::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(RequestParamTest, Unmarshalling001, TestSize.Level0)
{
    Parcel parcel;
    sptr<RequestParam> ptr = RequestParam::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(RequestParamTest, Unmarshalling002, TestSize.Level0)
{
    Parcel parcel;
    int32_t page = 1;
    parcel.WriteInt32(page);
    sptr<RequestParam> ptr = RequestParam::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(RequestParamTest, Unmarshalling003, TestSize.Level0)
{
    Parcel parcel;
    int32_t page = 1;
    parcel.WriteInt32(page);
    int32_t pageSize = 2;
    parcel.WriteInt32(pageSize);
    sptr<RequestParam> ptr = RequestParam::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(RequestParamTest, Unmarshalling004, TestSize.Level0)
{
    Parcel parcel;
    int32_t page = 1;
    parcel.WriteInt32(page);
    int32_t pageSize = 2;
    parcel.WriteInt32(pageSize);
    int32_t orderField = 1;
    parcel.WriteInt32(orderField);
    sptr<RequestParam> ptr = RequestParam::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(FirewallRulePageTest, Unmarshalling001, TestSize.Level0)
{
    Parcel parcel;
    sptr<FirewallRulePage> ptr = FirewallRulePage::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(FirewallRulePageTest, Unmarshalling002, TestSize.Level0)
{
    Parcel parcel;
    int32_t page = 1;
    parcel.WriteInt32(page);
    sptr<FirewallRulePage> ptr = FirewallRulePage::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(FirewallRulePageTest, Unmarshalling003, TestSize.Level0)
{
    Parcel parcel;
    int32_t page = 1;
    parcel.WriteInt32(page);
    int32_t pageSize = 2;
    parcel.WriteInt32(pageSize);
    sptr<FirewallRulePage> ptr = FirewallRulePage::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(FirewallRulePageTest, Unmarshalling004, TestSize.Level0)
{
    Parcel parcel;
    int32_t page = 1;
    parcel.WriteInt32(page);
    int32_t pageSize = 2;
    parcel.WriteInt32(pageSize);
    int32_t totalPage = 50;
    parcel.WriteInt32(totalPage);
    sptr<FirewallRulePage> ptr = FirewallRulePage::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(FirewallRulePageTest, Unmarshalling005, TestSize.Level0)
{
    Parcel parcel;
    int32_t page = 1;
    parcel.WriteInt32(page);
    int32_t pageSize = 2;
    parcel.WriteInt32(pageSize);
    int32_t totalPage = 50;
    parcel.WriteInt32(totalPage);
    uint32_t size = 2;
    parcel.WriteUint32(size);
    sptr<FirewallRulePage> ptr = FirewallRulePage::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(InterceptRecordPageTest, Unmarshalling001, TestSize.Level0)
{
    Parcel parcel;
    sptr<InterceptRecordPage> ptr = InterceptRecordPage::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(InterceptRecordPageTest, Unmarshalling002, TestSize.Level0)
{
    Parcel parcel;
    int32_t page = 1;
    parcel.WriteInt32(page);
    sptr<InterceptRecordPage> ptr = InterceptRecordPage::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(InterceptRecordPageTest, Unmarshalling003, TestSize.Level0)
{
    Parcel parcel;
    int32_t page = 1;
    parcel.WriteInt32(page);
    int32_t pageSize = 2;
    parcel.WriteInt32(pageSize);
    sptr<InterceptRecordPage> ptr = InterceptRecordPage::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(InterceptRecordPageTest, Unmarshalling004, TestSize.Level0)
{
    Parcel parcel;
    int32_t page = 1;
    parcel.WriteInt32(page);
    int32_t pageSize = 2;
    parcel.WriteInt32(pageSize);
    int32_t totalPage = 50;
    parcel.WriteInt32(totalPage);
    sptr<InterceptRecordPage> ptr = InterceptRecordPage::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(InterceptRecordPageTest, Unmarshalling005, TestSize.Level0)
{
    Parcel parcel;
    int32_t page = 1;
    parcel.WriteInt32(page);
    int32_t pageSize = 2;
    parcel.WriteInt32(pageSize);
    int32_t totalPage = 50;
    parcel.WriteInt32(totalPage);
    uint32_t size = 2;
    parcel.WriteUint32(size);
    sptr<InterceptRecordPage> ptr = InterceptRecordPage::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}
}
}