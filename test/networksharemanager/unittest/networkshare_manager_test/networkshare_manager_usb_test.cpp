/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "networkshare_client.h"
#include "networkshare_constants.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;
constexpr int32_t EIGHT_SECONDS = 8;
constexpr int32_t TWO_SECONDS = 2;
HapInfoParams testInfoParms = {
    .userID = 1,
    .bundleName = "networkshare_manager_test",
    .instIndex = 0,
    .appIDDesc = "test",
};

PermissionDef testPermDef = {
    .permissionName = "ohos.permission.CONNECTIVITY_INTERNAL",
    .bundleName = "networkshare_manager_test",
    .grantMode = 1,
    .availableLevel = APL_SYSTEM_BASIC,
    .label = "label",
    .labelId = 1,
    .description = "Test network share maneger",
    .descriptionId = 1,
};

PermissionStateFull testState = {
    .permissionName = "ohos.permission.CONNECTIVITY_INTERNAL",
    .isGeneral = true,
    .resDeviceID = {"local"},
    .grantStatus = {PermissionState::PERMISSION_GRANTED},
    .grantFlags = {2},
};

HapPolicyParams testPolicyPrams = {
    .apl = APL_SYSTEM_BASIC,
    .domain = "test.domain",
    .permList = {testPermDef},
    .permStateList = {testState},
};
} // namespace

class AccessToken {
public:
    AccessToken() : currentID_(GetSelfTokenID())
    {
        AccessTokenIDEx tokenIdEx = AccessTokenKit::AllocHapToken(testInfoParms, testPolicyPrams);
        accessID_ = tokenIdEx.tokenIdExStruct.tokenID;
        SetSelfTokenID(accessID_);
    }
    ~AccessToken()
    {
        AccessTokenKit::DeleteToken(accessID_);
        SetSelfTokenID(currentID_);
    }

private:
    AccessTokenID currentID_;
    AccessTokenID accessID_ = 0;
};

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
    AccessToken token;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->StartSharing(SharingIfaceType::SHARING_USB);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    sleep(EIGHT_SECONDS);
    SharingIfaceState state;
    DelayedSingleton<NetworkShareClient>::GetInstance()->GetSharingState(SharingIfaceType::SHARING_USB, state);
}

HWTEST_F(NetworkShareManagerTest, StopUsbSharing, TestSize.Level1)
{
    AccessToken token;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->StopSharing(SharingIfaceType::SHARING_USB);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    sleep(TWO_SECONDS);
    SharingIfaceState state;
    DelayedSingleton<NetworkShareClient>::GetInstance()->GetSharingState(SharingIfaceType::SHARING_USB, state);
    EXPECT_NE(state, SharingIfaceState::SHARING_NIC_SERVING);
}
} // namespace NetManagerStandard
} // namespace OHOS
