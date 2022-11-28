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

#include "interface_configuration.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
namespace {
InterfaceConfiguration GetInterfaceConfiguration()
{
    InterfaceConfiguration info;
    info.mode_ = STATIC;
    return info;
}
} // namespace

class InterfaceConfigurationTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void InterfaceConfigurationTest::SetUpTestCase() {}

void InterfaceConfigurationTest::TearDownTestCase() {}

void InterfaceConfigurationTest::SetUp() {}

void InterfaceConfigurationTest::TearDown() {}

/**
 * @tc.name: InterfaceConfigurationTest
 * @tc.desc: Test InterfaceConfiguration InterfaceConfiguration01.
 * @tc.type: FUNC
 */
HWTEST_F(InterfaceConfigurationTest, InterfaceConfiguration01, TestSize.Level1)
{
    Parcel parcel;
    InterfaceConfiguration data = GetInterfaceConfiguration();
    EXPECT_TRUE(data.Marshalling(parcel));
    auto result = InterfaceConfiguration::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->mode_, data.mode_);
}
} // namespace NetManagerStandard
} // namespace OHOS
