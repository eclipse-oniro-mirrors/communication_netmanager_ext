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
#include "edm_parameter_utils.h"

#include "netmgr_ext_log_wrapper.h"
#include "parameter.h"

#ifdef GTEST_API_
#define private public
#define protected public
#endif

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
}

class EdmParameterUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    EdmParameterUtils &edmparameterutils = EdmParameterUtils::GetInstance();
};

void EdmParameterUtilsTest::SetUpTestCase() {};

void EdmParameterUtilsTest::TearDownTestCase() {};

void EdmParameterUtilsTest::SetUp() {};

void EdmParameterUtilsTest::TearDown() {};

HWTEST_F(EdmParameterUtilsTest, RegisterEdmParameterChangeEventTest001, TestSize.Level1)
{
    char *key;
    ParameterChgPtr callback = nullptr;
    void *context = nullptr;
    edmparameterutils.RegisterEdmParameterChangeEvent(key, callback, context);
    EXPECT_EQ(callback, nullptr);
}

HWTEST_F(EdmParameterUtilsTest, ConvertToInt64Test001, TestSize.Level1)
{
    std::string str = "";
    int64_t value = 123;
    EXPECT_FALSE(edmparameterutils.ConvertToInt64(str, value));
    str = "abc";
    EXPECT_FALSE(edmparameterutils.ConvertToInt64(str, value));
    str = "123";
    EXPECT_TRUE(edmparameterutils.ConvertToInt64(str, value));
}
}
}