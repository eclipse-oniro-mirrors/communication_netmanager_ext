/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include <map>
#include <mutex>
#include <set>
#include <vector>

#include <gtest/gtest.h>

#include "net_manager_constants.h"
#include "netmanager_ext_test_security.h"
#include "netmgr_ext_log_wrapper.h"
#include "networkshare_client.h"
#include "networkshare_constants.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr int32_t EIGHT_SECONDS = 8;
constexpr int32_t TWO_SECONDS = 2;
} // namespace

class NetworkShareManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetworkShareManagerTest::SetUpTestCase() {}

void NetworkShareManagerTest::TearDownTestCase() {}

void NetworkShareManagerTest::SetUp() {}

void NetworkShareManagerTest::TearDown() {}

HWTEST_F(NetworkShareManagerTest, StartUsbSharing, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->StartSharing(SharingIfaceType::SHARING_USB);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    sleep(EIGHT_SECONDS);
    SharingIfaceState state;
    DelayedSingleton<NetworkShareClient>::GetInstance()->GetSharingState(SharingIfaceType::SHARING_USB, state);
}

HWTEST_F(NetworkShareManagerTest, StopUsbSharing, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->StopSharing(SharingIfaceType::SHARING_USB);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    sleep(TWO_SECONDS);
    SharingIfaceState state;
    DelayedSingleton<NetworkShareClient>::GetInstance()->GetSharingState(SharingIfaceType::SHARING_USB, state);
    EXPECT_NE(state, SharingIfaceState::SHARING_NIC_SERVING);
}
} // namespace NetManagerStandard
} // namespace OHOS
