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
#include "net_manager_constants.h"
#include "wearable_distributed_net_config_forward.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr int32_t TCP_PORT_ID = 8080;
constexpr int32_t UDP_PORT_ID = 8081;
} // namesapce

class WearableDistributedNetConfigForwardTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();

    void SetUp();
    void TearDown();
};

void WearableDistributedNetConfigForwardTest::SetUpTestCase() {}

void WearableDistributedNetConfigForwardTest::TearDownTestCase() {}

void WearableDistributedNetConfigForwardTest::SetUp() {}

void WearableDistributedNetConfigForwardTest::TearDown() {}

HWTEST_F(WearableDistributedNetConfigForwardTest, EnableWearableDistributedNetForward, TestSize.Level1)
{
    WearableDistributedNetForward config;
    int32_t ret = config.EnableWearableDistributedNetForward(TCP_PORT_ID, UDP_PORT_ID);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    ret = config.EnableWearableDistributedNetForward(0, UDP_PORT_ID);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INVALID_PARAMETER);

    ret = config.EnableWearableDistributedNetForward(65536, UDP_PORT_ID);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INVALID_PARAMETER);

    ret = config.EnableWearableDistributedNetForward(65535, UDP_PORT_ID);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    ret = config.EnableWearableDistributedNetForward(-1, UDP_PORT_ID);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INVALID_PARAMETER);

    ret = config.EnableWearableDistributedNetForward(TCP_PORT_ID, 0);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INVALID_PARAMETER);

    ret = config.EnableWearableDistributedNetForward(TCP_PORT_ID, 65536);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INVALID_PARAMETER);

    ret = config.EnableWearableDistributedNetForward(TCP_PORT_ID, 65535);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    ret = config.EnableWearableDistributedNetForward(TCP_PORT_ID, -1);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INVALID_PARAMETER);
}

HWTEST_F(WearableDistributedNetConfigForwardTest, DisableWearableDistributedNetForward, TestSize.Level1)
{
    WearableDistributedNetForward config;
    int32_t result = config.DisableWearableDistributedNetForward();
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS