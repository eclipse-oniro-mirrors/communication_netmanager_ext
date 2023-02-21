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
#include "netshare_result_callback_stub.h"
#include "networkshare_client.h"
#include "networkshare_constants.h"
#include "sharing_event_callback_stub.h"

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

class TestSharingEventCallback : public OHOS::NetManagerStandard::SharingEventCallbackStub {
public:
    TestSharingEventCallback() = default;
    virtual ~TestSharingEventCallback() = default;
    void OnSharingStateChanged(const bool &isRunning);
    void OnInterfaceSharingStateChanged(const SharingIfaceType &type, const std::string &iface,
                                        const SharingIfaceState &state);
    void OnSharingUpstreamChanged(const sptr<NetHandle> netHandle);
};

sptr<TestSharingEventCallback> g_sharingEventCb = new (std::nothrow) TestSharingEventCallback();
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

void TestSharingEventCallback::OnSharingStateChanged(const bool &isRunning)
{
    std::cout << "TestSharingEventCallback::OnSharingStateChanged isRunning = " << isRunning << std::endl;
}

void TestSharingEventCallback::OnInterfaceSharingStateChanged(const SharingIfaceType &type, const std::string &iface,
                                                              const SharingIfaceState &state)
{
    std::cout << "type=" << static_cast<int32_t>(type);
    std::cout << "iface:" << iface;
    std::cout << ", state=" << static_cast<int32_t>(state) << std::endl;
}

void TestSharingEventCallback::OnSharingUpstreamChanged(const sptr<NetHandle> netHandle)
{
    std::cout << "TestSharingEventCallback::OnSharingUpstreamChanged netId = " << netHandle->GetNetId() << std::endl;
}

HWTEST_F(NetworkShareManagerTest, IsSharingSupported, TestSize.Level1)
{
    AccessToken token;
    int32_t supportedFlag;
    DelayedSingleton<NetworkShareClient>::GetInstance()->IsSharingSupported(supportedFlag);
    EXPECT_EQ(supportedFlag, NETWORKSHARE_IS_SUPPORTED);
}

HWTEST_F(NetworkShareManagerTest, GetWifiSharableRegexs, TestSize.Level1)
{
    AccessToken token;
    std::vector<std::string> wifiRegexs;
    DelayedSingleton<NetworkShareClient>::GetInstance()->GetSharableRegexs(SharingIfaceType::SHARING_WIFI, wifiRegexs);
    EXPECT_NE(wifiRegexs.size(), static_cast<uint32_t>(0));
    for (auto regex : wifiRegexs) {
        std::cout << "Wifi Sharable Regex: " << regex << std::endl;
    }
}

HWTEST_F(NetworkShareManagerTest, GetUSBSharableRegexs, TestSize.Level1)
{
    AccessToken token;
    std::vector<std::string> usbRegexs;
    DelayedSingleton<NetworkShareClient>::GetInstance()->GetSharableRegexs(SharingIfaceType::SHARING_USB, usbRegexs);
    EXPECT_NE(usbRegexs.size(), static_cast<uint32_t>(0));
    for (auto regex : usbRegexs) {
        std::cout << "USB Sharable Regex: " << regex << std::endl;
    }
}

HWTEST_F(NetworkShareManagerTest, GetBluetoothSharableRegexs, TestSize.Level1)
{
    AccessToken token;
    std::vector<std::string> blueRegexs;
    DelayedSingleton<NetworkShareClient>::GetInstance()->GetSharableRegexs(SharingIfaceType::SHARING_BLUETOOTH,
                                                                           blueRegexs);
    EXPECT_NE(blueRegexs.size(), static_cast<uint32_t>(0));
    for (auto regex : blueRegexs) {
        std::cout << "Bluetooth Sharable Regex: " << regex << std::endl;
    }
}

HWTEST_F(NetworkShareManagerTest, StartWifiSharing, TestSize.Level1)
{
    AccessToken token;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->StartSharing(SharingIfaceType::SHARING_WIFI);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    sleep(EIGHT_SECONDS);
    SharingIfaceState state;
    DelayedSingleton<NetworkShareClient>::GetInstance()->GetSharingState(SharingIfaceType::SHARING_WIFI, state);
    SUCCEED();
}

HWTEST_F(NetworkShareManagerTest, StopWifiSharing, TestSize.Level1)
{
    AccessToken token;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->StopSharing(SharingIfaceType::SHARING_WIFI);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    sleep(TWO_SECONDS);
    SharingIfaceState state;
    DelayedSingleton<NetworkShareClient>::GetInstance()->GetSharingState(SharingIfaceType::SHARING_WIFI, state);
    EXPECT_NE(state, SharingIfaceState::SHARING_NIC_SERVING);
}

HWTEST_F(NetworkShareManagerTest, RegisterSharingEvent, TestSize.Level1)
{
    AccessToken token;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->RegisterSharingEvent(g_sharingEventCb);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareManagerTest, UnregisterSharingEvent, TestSize.Level1)
{
    AccessToken token;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->UnregisterSharingEvent(g_sharingEventCb);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
