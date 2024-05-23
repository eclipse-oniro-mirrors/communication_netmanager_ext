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
#include "netshare_result_callback_stub.h"
#define private public
#include "networkshare_client.h"
#undef private
#include "networkshare_constants.h"
#include "sharing_event_callback_stub.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr int32_t EIGHT_SECONDS = 8;
constexpr int32_t TWO_SECONDS = 2;

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

HWTEST_F(NetworkShareManagerTest, OnLoadSystemAbilitySuccess, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    sptr<IRemoteObject> remoteObject;
    DelayedSingleton<NetworkShareLoadCallback>::GetInstance()->OnLoadSystemAbilitySuccess(systemAbilityId,
                                                                                            remoteObject);
    auto ret = DelayedSingleton<NetworkShareLoadCallback>::GetInstance()->GetRemoteObject();
    EXPECT_EQ(ret, nullptr);
}

HWTEST_F(NetworkShareManagerTest, OnLoadSystemAbilityFail, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    DelayedSingleton<NetworkShareLoadCallback>::GetInstance()->OnLoadSystemAbilityFail(systemAbilityId);
    auto ret = DelayedSingleton<NetworkShareLoadCallback>::GetInstance()->IsFailed();
    EXPECT_EQ(ret, true);
}

HWTEST_F(NetworkShareManagerTest, GetWifiSharableRegexs, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    std::vector<std::string> wifiRegexs;
    DelayedSingleton<NetworkShareClient>::GetInstance()->GetSharableRegexs(SharingIfaceType::SHARING_WIFI, wifiRegexs);
    EXPECT_EQ(wifiRegexs.size(), static_cast<uint32_t>(0));
    for (auto regex : wifiRegexs) {
        std::cout << "Wifi Sharable Regex: " << regex << std::endl;
    }
}

HWTEST_F(NetworkShareManagerTest, GetUSBSharableRegexs, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    std::vector<std::string> usbRegexs;
    DelayedSingleton<NetworkShareClient>::GetInstance()->GetSharableRegexs(SharingIfaceType::SHARING_USB, usbRegexs);
    EXPECT_EQ(usbRegexs.size(), static_cast<uint32_t>(0));
    for (auto regex : usbRegexs) {
        std::cout << "USB Sharable Regex: " << regex << std::endl;
    }
}

HWTEST_F(NetworkShareManagerTest, GetBluetoothSharableRegexs, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    std::vector<std::string> blueRegexs;
    DelayedSingleton<NetworkShareClient>::GetInstance()->GetSharableRegexs(SharingIfaceType::SHARING_BLUETOOTH,
                                                                           blueRegexs);
    EXPECT_EQ(blueRegexs.size(), static_cast<uint32_t>(0));
    for (auto regex : blueRegexs) {
        std::cout << "Bluetooth Sharable Regex: " << regex << std::endl;
    }
}
} // namespace NetManagerStandard
} // namespace OHOS
