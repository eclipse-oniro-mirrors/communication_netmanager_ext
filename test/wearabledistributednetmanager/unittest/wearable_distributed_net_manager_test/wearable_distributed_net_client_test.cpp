/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "netmanager_ext_test_security.h"
#include "net_manager_constants.h"
#include "token_setproc.h"

#define private public
#include "wearable_distributed_net_client.h"
#undef private

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr int32_t TCP_PORT_ID = 8080;
constexpr int32_t UDP_PORT_ID = 8081;
} // namespace

class WearableDistributedNetClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void WearableDistributedNetClientTest::SetUpTestCase() {}

void WearableDistributedNetClientTest::TearDownTestCase() {}

void WearableDistributedNetClientTest::SetUp() {}

void WearableDistributedNetClientTest::TearDown() {}

HWTEST_F(WearableDistributedNetClientTest, SetUpWearableDistributedNet001, TestSize.Level1)
{
    auto ret = WearableDistributedNetClient::GetInstance().SetupWearableDistributedNet(TCP_PORT_ID, UDP_PORT_ID, false);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
    WearableDistributedNetClient::GetInstance().TearDownWearableDistributedNet();
    ret = WearableDistributedNetClient::GetInstance().SetupWearableDistributedNet(TCP_PORT_ID, UDP_PORT_ID, true);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
    WearableDistributedNetClient::GetInstance().TearDownWearableDistributedNet();
}

HWTEST_F(WearableDistributedNetClientTest, SetUpWearableDistributedNet002, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    auto ret = WearableDistributedNetClient::GetInstance().SetupWearableDistributedNet(TCP_PORT_ID, UDP_PORT_ID, false);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    WearableDistributedNetClient::GetInstance().TearDownWearableDistributedNet();
    ret = WearableDistributedNetClient::GetInstance().SetupWearableDistributedNet(TCP_PORT_ID, UDP_PORT_ID, true);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    WearableDistributedNetClient::GetInstance().TearDownWearableDistributedNet();
}

HWTEST_F(WearableDistributedNetClientTest, TearDownWearableDistributedNet001, TestSize.Level1)
{
    WearableDistributedNetClient::GetInstance().SetupWearableDistributedNet(TCP_PORT_ID, UDP_PORT_ID, false);
    auto ret = WearableDistributedNetClient::GetInstance().TearDownWearableDistributedNet();
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(WearableDistributedNetClientTest, TearDownWearableDistributedNet002, TestSize.Level1)
{
    WearableDistributedNetClient::GetInstance().SetupWearableDistributedNet(TCP_PORT_ID, UDP_PORT_ID, false);
    auto ret = WearableDistributedNetClient::GetInstance().TearDownWearableDistributedNet();
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(WearableDistributedNetClientTest, OnLoadSystemAbilitySuccess, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    sptr<IRemoteObject> remoteObject;
    DelayedSingleton<WearableDistributedNetLoadCallback>::GetInstance()->OnLoadSystemAbilitySuccess(systemAbilityId,
                                                                                                    remoteObject);
    auto ret = DelayedSingleton<WearableDistributedNetLoadCallback>::GetInstance()->GetRemoteObject();
    EXPECT_EQ(ret, nullptr);
}

HWTEST_F(WearableDistributedNetClientTest, OnLoadSystemAbilityFail, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    DelayedSingleton<WearableDistributedNetLoadCallback>::GetInstance()->OnLoadSystemAbilityFail(systemAbilityId);
    auto ret = DelayedSingleton<WearableDistributedNetLoadCallback>::GetInstance()->IsFailed();
    EXPECT_EQ(ret, true);
}

HWTEST_F(WearableDistributedNetClientTest, OnRemoteDied, TestSize.Level1)
{
    sptr<IRemoteObject> remote = nullptr;
    WearableDistributedNetClient::GetInstance().OnRemoteDied(remote);
    WearableDistributedNetClient::GetInstance().wearableDistributedNetService_ = nullptr;
    EXPECT_EQ(WearableDistributedNetClient::GetInstance().wearableDistributedNetService_, nullptr);
    WearableDistributedNetClient::GetInstance().wearableDistributedNetService_
        = iface_cast<IWearableDistributedNet>(remote);
    WearableDistributedNetClient::GetInstance().OnRemoteDied(remote);
    EXPECT_EQ(WearableDistributedNetClient::GetInstance().wearableDistributedNetService_, nullptr);
}

HWTEST_F(WearableDistributedNetClientTest, UpdateWearableDistributedNetMeteredStatus, TestSize.Level1)
{
    bool isMetered = true;
    WearableDistributedNetClient::GetInstance().RestartWearableDistributedNetManagerSysAbility();
    auto ret = WearableDistributedNetClient::GetInstance().UpdateWearableDistributedNetMeteredStatus(isMetered);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}
} // namespace NetManagerStandard
} // namespace OHOS
