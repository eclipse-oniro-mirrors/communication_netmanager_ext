/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
 
#include "eth_eap_profile.h"
#include "netmgr_ext_log_wrapper.h"
#include "parcel.h"
#include "refbase.h"
 
#ifdef GTEST_API_
#define private public
#define protected public
#endif
 
namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
}
 
class EthEapProfileTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};
 
void EthEapProfileTest::SetUpTestCase() {};
 
void EthEapProfileTest::TearDownTestCase() {};
 
void EthEapProfileTest::SetUp() {};
 
void EthEapProfileTest::TearDown() {};
 
HWTEST_F(EthEapProfileTest, MarshallingTest001, TestSize.Level1)
{
    EthEapProfile profile;
    Parcel parcel;
    EXPECT_TRUE(profile.Marshalling(parcel));
}
 
HWTEST_F(EthEapProfileTest, UnmarshallingTest001, TestSize.Level1)
{
    EthEapProfile profile;
    Parcel parcel;
    std::string testStr = "test";
    int32_t testInt32 = 0;
    EXPECT_EQ(profile.Unmarshalling(parcel), nullptr);
    parcel.WriteInt32(testInt32);
    EXPECT_EQ(profile.Unmarshalling(parcel), nullptr);
    parcel.WriteInt32(testInt32);
    parcel.WriteInt32(testInt32);
    EXPECT_EQ(profile.Unmarshalling(parcel), nullptr);
    parcel.WriteInt32(testInt32);
    parcel.WriteInt32(testInt32);
    parcel.WriteString(testStr);
    EXPECT_EQ(profile.Unmarshalling(parcel), nullptr);
    parcel.WriteInt32(testInt32);
    parcel.WriteInt32(testInt32);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    EXPECT_EQ(profile.Unmarshalling(parcel), nullptr);
    parcel.WriteInt32(testInt32);
    parcel.WriteInt32(testInt32);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    EXPECT_EQ(profile.Unmarshalling(parcel), nullptr);
    parcel.WriteInt32(testInt32);
    parcel.WriteInt32(testInt32);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    EXPECT_EQ(profile.Unmarshalling(parcel), nullptr);
    parcel.WriteInt32(testInt32);
    parcel.WriteInt32(testInt32);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    EXPECT_EQ(profile.Unmarshalling(parcel), nullptr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteInt32(testInt32);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    EXPECT_EQ(profile.Unmarshalling(parcel), nullptr);
}
 
HWTEST_F(EthEapProfileTest, UnmarshallingTest002, TestSize.Level1)
{
    EthEapProfile profile;
    Parcel parcel;
    std::string testStr = "test";
    int32_t testInt32 = 0;
    parcel.WriteInt32(testInt32);
    parcel.WriteInt32(testInt32);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    EXPECT_EQ(profile.Unmarshalling(parcel), nullptr);
    parcel.WriteInt32(testInt32);
    parcel.WriteInt32(testInt32);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    EXPECT_EQ(profile.Unmarshalling(parcel), nullptr);
    parcel.WriteInt32(testInt32);
    parcel.WriteInt32(testInt32);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    EXPECT_EQ(profile.Unmarshalling(parcel), nullptr);
    parcel.WriteInt32(testInt32);
    parcel.WriteInt32(testInt32);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    EXPECT_EQ(profile.Unmarshalling(parcel), nullptr);
}
 
HWTEST_F(EthEapProfileTest, UnmarshallingTest003, TestSize.Level1)
{
    EthEapProfile profile;
    Parcel parcel;
    std::string testStr = "test";
    int32_t testInt32 = 0;
    parcel.WriteInt32(testInt32);
    parcel.WriteInt32(testInt32);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    EXPECT_EQ(profile.Unmarshalling(parcel), nullptr);
    parcel.WriteInt32(testInt32);
    parcel.WriteInt32(testInt32);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    EXPECT_EQ(profile.Unmarshalling(parcel), nullptr);
    parcel.WriteInt32(testInt32);
    parcel.WriteInt32(testInt32);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    EXPECT_EQ(profile.Unmarshalling(parcel), nullptr);
}
 
HWTEST_F(EthEapProfileTest, UnmarshallingTest004, TestSize.Level1)
{
    EthEapProfile profile;
    Parcel parcel;
    std::string testStr = "test";
    int32_t testInt32 = 0;
    parcel.WriteInt32(testInt32);
    parcel.WriteInt32(testInt32);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteInt32(testInt32);
    EXPECT_EQ(profile.Unmarshalling(parcel), nullptr);
    parcel.WriteInt32(testInt32);
    parcel.WriteInt32(testInt32);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteInt32(testInt32);
    parcel.WriteInt32(testInt32);
    EXPECT_NE(profile.Unmarshalling(parcel), nullptr);
}
 
HWTEST_F(EthEapProfileTest, UnmarshallingTest005, TestSize.Level1)
{
    EthEapProfile profile;
    Parcel parcel;
    std::string testStr = "test";
    int32_t testInt32 = 0;
    int32_t testInt2 = 1;
    parcel.WriteInt32(testInt32);
    parcel.WriteInt32(testInt32);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteInt32(testInt32);
    parcel.WriteInt32(testInt2);
    EXPECT_EQ(profile.Unmarshalling(parcel), nullptr);
    parcel.WriteInt32(testInt32);
    parcel.WriteInt32(testInt32);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteString(testStr);
    parcel.WriteInt32(testInt32);
    parcel.WriteInt32(testInt2);
    parcel.WriteInt32(testInt32);
    EXPECT_NE(profile.Unmarshalling(parcel), nullptr);
}

} // namespace NetManagerStandard
} // namespace OHOS