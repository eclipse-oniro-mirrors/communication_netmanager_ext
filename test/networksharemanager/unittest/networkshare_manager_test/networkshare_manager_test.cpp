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

HWTEST_F(NetworkShareManagerTest, IsSharingSupported, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    int32_t supportedFlag;
    auto ret = DelayedSingleton<NetworkShareClient>::GetInstance()->IsSharingSupported(supportedFlag);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareManagerTest, IsSharing, TestSize.Level1)
{
    int32_t sharingStatus;
    int32_t ret = DelayedSingleton<NetworkShareClient>::GetInstance()->IsSharing(sharingStatus);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareManagerTest, GetWifiSharableRegexs, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    std::vector<std::string> wifiRegexs;
    int32_t ret = DelayedSingleton<NetworkShareClient>::GetInstance()->GetSharableRegexs(
        SharingIfaceType::SHARING_WIFI, wifiRegexs);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    for (auto regex : wifiRegexs) {
        std::cout << "Wifi Sharable Regex: " << regex << std::endl;
    }
}

HWTEST_F(NetworkShareManagerTest, GetUSBSharableRegexs, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    std::vector<std::string> usbRegexs;
    int32_t ret = DelayedSingleton<NetworkShareClient>::GetInstance()->GetSharableRegexs(
        SharingIfaceType::SHARING_USB, usbRegexs);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    for (auto regex : usbRegexs) {
        std::cout << "USB Sharable Regex: " << regex << std::endl;
    }
}

HWTEST_F(NetworkShareManagerTest, GetBluetoothSharableRegexs, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    std::vector<std::string> blueRegexs;
    int32_t ret = DelayedSingleton<NetworkShareClient>::GetInstance()->GetSharableRegexs(
        SharingIfaceType::SHARING_BLUETOOTH, blueRegexs);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    for (auto regex : blueRegexs) {
        std::cout << "Bluetooth Sharable Regex: " << regex << std::endl;
    }
}

HWTEST_F(NetworkShareManagerTest, StartWifiSharing, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->StartSharing(SharingIfaceType::SHARING_WIFI);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    sleep(EIGHT_SECONDS);
    SharingIfaceState state;
    DelayedSingleton<NetworkShareClient>::GetInstance()->GetSharingState(SharingIfaceType::SHARING_WIFI, state);
    SUCCEED();
}

HWTEST_F(NetworkShareManagerTest, StopWifiSharing, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->StopSharing(SharingIfaceType::SHARING_WIFI);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    sleep(TWO_SECONDS);
    SharingIfaceState state;
    DelayedSingleton<NetworkShareClient>::GetInstance()->GetSharingState(SharingIfaceType::SHARING_WIFI, state);
    EXPECT_NE(state, SharingIfaceState::SHARING_NIC_SERVING);
}

HWTEST_F(NetworkShareManagerTest, RegisterSharingEvent001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->RegisterSharingEvent(g_sharingEventCb);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareManagerTest, RegisterSharingEvent002, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    sptr<ISharingEventCallback> callback = nullptr;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->RegisterSharingEvent(callback);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_LOCAL_PTR_NULL);
}

HWTEST_F(NetworkShareManagerTest, UnregisterSharingEvent001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    sptr<ISharingEventCallback> callback = nullptr;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->UnregisterSharingEvent(g_sharingEventCb);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareManagerTest, UnregisterSharingEvent002, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    sptr<ISharingEventCallback> callback = nullptr;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->UnregisterSharingEvent(callback);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_LOCAL_PTR_NULL);
}

HWTEST_F(NetworkShareManagerTest, GetSharingIfaces01, TestSize.Level1)
{
    SharingIfaceState state = SharingIfaceState::SHARING_NIC_SERVING;
    std::vector<std::string> ifaces;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->GetSharingIfaces(state, ifaces);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareManagerTest, GetSharingIfaces02, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    SharingIfaceState state = SharingIfaceState::SHARING_NIC_SERVING;
    std::vector<std::string> ifaces;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->GetSharingIfaces(state, ifaces);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareManagerTest, GetStatsRxBytes01, TestSize.Level1)
{
    int32_t bytes = 0;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->GetStatsRxBytes(bytes);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareManagerTest, GetStatsRxBytes02, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    int32_t bytes = 0;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->GetStatsRxBytes(bytes);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareManagerTest, GetStatsTxBytes01, TestSize.Level1)
{
    int32_t bytes = 0;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->GetStatsTxBytes(bytes);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareManagerTest, GetStatsTxBytes02, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    int32_t bytes = 0;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->GetStatsTxBytes(bytes);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareManagerTest, GetStatsTotalBytes01, TestSize.Level1)
{
    int32_t bytes = 0;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->GetStatsTotalBytes(bytes);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareManagerTest, GetStatsTotalBytes02, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    int32_t bytes = 0;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->GetStatsTotalBytes(bytes);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareManagerTest, OnRemoteDied, TestSize.Level1)
{
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->StartSharing(SharingIfaceType::SHARING_WIFI);
    wptr<IRemoteObject> remote = nullptr;
    DelayedSingleton<NetworkShareClient>::GetInstance()->OnRemoteDied(remote);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}
} // namespace NetManagerStandard
} // namespace OHOS
