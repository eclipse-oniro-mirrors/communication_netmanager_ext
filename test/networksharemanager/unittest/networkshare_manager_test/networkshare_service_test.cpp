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

#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

#include "net_manager_constants.h"
#include "networkshare_constants.h"
#include "networkshare_service.h"
#include "sharing_event_callback_stub.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;
HapInfoParams testInfoParms = {
    .userID = 1,
    .bundleName = "networkshare_service_test",
    .instIndex = 0,
    .appIDDesc = "test",
};

PermissionDef testPermDef = {
    .permissionName = "ohos.permission.CONNECTIVITY_INTERNAL",
    .bundleName = "networkshare_service_test",
    .grantMode = 1,
    .availableLevel = APL_SYSTEM_BASIC,
    .label = "label",
    .labelId = 1,
    .description = "Test network share manager",
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

class SharingEventTestCallback : public SharingEventCallbackStub {
public:
    inline void OnSharingStateChanged(const bool &isRunning) override
    {
        return;
    }
    inline void OnInterfaceSharingStateChanged(const SharingIfaceType &type, const std::string &iface,
                                               const SharingIfaceState &state) override
    {
        return;
    }
    inline void OnSharingUpstreamChanged(const sptr<NetHandle> netHandle) override
    {
        return;
    }
};
} // namespace

class NetworkShareServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline auto instance_ = DelayedSingleton<NetworkShareService>::GetInstance();
    static inline sptr<ISharingEventCallback> eventCallback_ = nullptr;
};

void NetworkShareServiceTest::SetUpTestCase()
{
    instance_->OnStart();
    eventCallback_ = new (std::nothrow) SharingEventTestCallback();
    ASSERT_NE(eventCallback_, nullptr);
}

void NetworkShareServiceTest::TearDownTestCase()
{
    instance_->OnStop();
}

void NetworkShareServiceTest::SetUp() {}

void NetworkShareServiceTest::TearDown() {}

HWTEST_F(NetworkShareServiceTest, IsNetworkSharingSupportedTest001, TestSize.Level1)
{
    int32_t supported;
    auto ret = instance_->IsNetworkSharingSupported(supported);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareServiceTest, IsSharingTest001, TestSize.Level1)
{
    int32_t sharingStatus;
    auto ret = instance_->IsSharing(sharingStatus);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareServiceTest, StartNetworkSharingTest001, TestSize.Level1)
{
    auto ret = instance_->StartNetworkSharing(SharingIfaceType::SHARING_WIFI);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareServiceTest, StartNetworkSharingTest002, TestSize.Level1)
{
    AccessToken token;
    auto ret = instance_->StartNetworkSharing(SharingIfaceType::SHARING_WIFI);
    EXPECT_EQ(ret, NETWORKSHARE_ERROR_WIFI_SHARING);
}

HWTEST_F(NetworkShareServiceTest, StartNetworkSharingTest003, TestSize.Level1)
{
    auto ret = instance_->StartNetworkSharing(SharingIfaceType::SHARING_USB);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareServiceTest, StopNetworkSharingTest001, TestSize.Level1)
{
    auto ret = instance_->StopNetworkSharing(SharingIfaceType::SHARING_WIFI);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareServiceTest, StopNetworkSharingTest002, TestSize.Level1)
{
    AccessToken token;
    auto ret = instance_->StopNetworkSharing(SharingIfaceType::SHARING_WIFI);
    EXPECT_EQ(ret, NETWORKSHARE_ERROR_WIFI_SHARING);
}

HWTEST_F(NetworkShareServiceTest, StopNetworkSharingTest003, TestSize.Level1)
{
    auto ret = instance_->StopNetworkSharing(SharingIfaceType::SHARING_USB);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareServiceTest, RegisterSharingEventTest001, TestSize.Level1)
{
    auto ret = instance_->RegisterSharingEvent(eventCallback_);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareServiceTest, UnregisterSharingEventTest001, TestSize.Level1)
{
    auto ret = instance_->UnregisterSharingEvent(eventCallback_);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareServiceTest, GetSharableRegexsTest001, TestSize.Level1)
{
    std::vector<std::string> ifaceRegexs;
    instance_->GetSharableRegexs(SharingIfaceType::SHARING_WIFI, ifaceRegexs);
    EXPECT_TRUE(ifaceRegexs.empty());
}

HWTEST_F(NetworkShareServiceTest, GetSharableRegexsTest002, TestSize.Level1)
{
    AccessToken token;
    std::vector<std::string> ifaceRegexs;
    instance_->GetSharableRegexs(SharingIfaceType::SHARING_WIFI, ifaceRegexs);
    EXPECT_TRUE(ifaceRegexs.empty());
}

HWTEST_F(NetworkShareServiceTest, GetSharingStateTest001, TestSize.Level1)
{
    SharingIfaceState state;
    auto ret = instance_->GetSharingState(SharingIfaceType::SHARING_WIFI, state);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareServiceTest, GetNetSharingIfacesTest001, TestSize.Level1)
{
    SharingIfaceType type = SharingIfaceType::SHARING_WIFI;
    std::vector<std::string> ifaceRegexs;
    instance_->GetSharableRegexs(type, ifaceRegexs);
    EXPECT_TRUE(ifaceRegexs.empty());
}

HWTEST_F(NetworkShareServiceTest, GetNetSharingIfacesTest002, TestSize.Level1)
{
    AccessToken token;
    SharingIfaceType type = SharingIfaceType::SHARING_WIFI;
    std::vector<std::string> ifaceRegexs;
    instance_->GetSharableRegexs(type, ifaceRegexs);
    EXPECT_TRUE(ifaceRegexs.empty());
}

HWTEST_F(NetworkShareServiceTest, GetStatsRxBytesTest001, TestSize.Level1)
{
    int32_t bytes;
    auto ret = instance_->GetStatsRxBytes(bytes);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareServiceTest, GetStatsTxBytesTest001, TestSize.Level1)
{
    int32_t bytes;
    auto ret = instance_->GetStatsTxBytes(bytes);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareServiceTest, GetStatsTotalBytesTest001, TestSize.Level1)
{
    int32_t bytes;
    auto ret = instance_->GetStatsTotalBytes(bytes);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

} // namespace NetManagerStandard
} // namespace OHOS
