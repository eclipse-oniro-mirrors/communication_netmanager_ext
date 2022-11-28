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

#include "static_configuration.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
namespace {
constexpr const char *DOMAIN = "test";
StaticConfiguration GetStaticConfiguration()
{
    StaticConfiguration info;
    info.domain_ = DOMAIN;
    return info;
}
} // namespace

class StaticConfigurationTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void StaticConfigurationTest::SetUpTestCase() {}

void StaticConfigurationTest::TearDownTestCase() {}

void StaticConfigurationTest::SetUp() {}

void StaticConfigurationTest::TearDown() {}

/**
 * @tc.name: StaticConfigurationTest
 * @tc.desc: Test StaticConfiguration StaticConfiguration01.
 * @tc.type: FUNC
 */
HWTEST_F(StaticConfigurationTest, StaticConfiguration01, TestSize.Level1)
{
    Parcel parcel;
    StaticConfiguration data = GetStaticConfiguration();
    EXPECT_TRUE(data.Marshalling(parcel));
    auto result = StaticConfiguration::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->domain_, data.domain_);
}
} // namespace NetManagerStandard
} // namespace OHOS