/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "netshare_result_callback_stub.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
constexpr int32_t RESULT_ZERO = 0;
// constexpr const char *ERROR_MSG = "Test";
class NetShareResultTestCallback : public NetShareResultCallbackStub {
public:
    inline void OnResult(const int32_t &status)
    {
        return;
    }
};

class NetShareResultCallbackStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static inline auto instance_ = std::make_shared<NetShareResultTestCallback>();
};

void NetShareResultCallbackStubTest::SetUpTestCase() {}

void NetShareResultCallbackStubTest::TearDownTestCase() {}

void NetShareResultCallbackStubTest::SetUp() {}

void NetShareResultCallbackStubTest::TearDown() {}

HWTEST_F(NetShareResultCallbackStubTest, OnRemoteRequest, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int32_t result = instance_->OnRemoteRequest(0, data, reply, option);
    EXPECT_NE(result, RESULT_ZERO);
    result = instance_->OnRemoteRequest(1, data, reply, option);
    EXPECT_NE(result, RESULT_ZERO);
}

} // namespace NetManagerStandard
} // namespace OHOS