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

#include "networkshare_tracker.h"
#include "sharing_event_callback_stub.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
static constexpr const char *WIFI_AP_DEFAULT_IFACE_NAME = "wlan0";
static constexpr const char *BLUETOOTH_DEFAULT_IFACE_NAME = "bt-pan";
static constexpr int32_t MAX_CALLBACK_COUNT = 100;
std::map<int32_t, sptr<ISharingEventCallback>> g_callbackMap;

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

class NetworkShareTrackerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline sptr<ISharingEventCallback> callback_ = nullptr;
};

void NetworkShareTrackerTest::SetUpTestCase() {}

void NetworkShareTrackerTest::TearDownTestCase() {}

void NetworkShareTrackerTest::SetUp() {}

void NetworkShareTrackerTest::TearDown()
{
    NetworkShareTracker::GetInstance().Uninit();
}

/**
 * @tc.name: Init01
 * @tc.desc: Test NetworkShareTracker Init.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, Init01, TestSize.Level1)
{
    bool ret = NetworkShareTracker::GetInstance().Init();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsNetworkSharingSupported01
 * @tc.desc: Test NetworkShareTracker IsNetworkSharingSupported.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, IsNetworkSharingSupported01, TestSize.Level1)
{
    NetworkShareTracker::GetInstance().Init();
    int32_t supported;
    NetworkShareTracker::GetInstance().IsNetworkSharingSupported(supported);
    EXPECT_EQ(supported, NETWORKSHARE_IS_SUPPORTED);
}

/**
 * @tc.name: IsSharing01
 * @tc.desc: Test NetworkShareTracker IsSharing.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, IsSharing01, TestSize.Level1)
{
    int32_t sharingStatus;
    NetworkShareTracker::GetInstance().IsSharing(sharingStatus);
    EXPECT_EQ(sharingStatus, NETWORKSHARE_IS_UNSHARING);
}

/**
 * @tc.name: StartNetworkSharing01
 * @tc.desc: Test NetworkShareTracker StartNetworkSharing.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, StartNetworkSharing01, TestSize.Level1)
{
    SharingIfaceType type = SharingIfaceType::SHARING_WIFI;
    NetworkShareTracker::GetInstance().StartNetworkSharing(type);
    int32_t sharingStatus;
    int32_t ret = NetworkShareTracker::GetInstance().IsSharing(sharingStatus);
    EXPECT_GE(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: StopNetworkSharing01
 * @tc.desc: Test NetworkShareTracker StopNetworkSharing.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, StopNetworkSharing01, TestSize.Level1)
{
    SharingIfaceType type = SharingIfaceType::SHARING_WIFI;
    int32_t ret = NetworkShareTracker::GetInstance().StopNetworkSharing(type);
    EXPECT_GE(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: StartNetworkSharing03
 * @tc.desc: Test NetworkShareTracker StartNetworkSharing.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, StartNetworkSharing03, TestSize.Level1)
{
    SharingIfaceType type = SharingIfaceType::SHARING_BLUETOOTH;
    int32_t ret = NetworkShareTracker::GetInstance().StartNetworkSharing(type);
    EXPECT_GE(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: StopNetworkSharing03
 * @tc.desc: Test NetworkShareTracker StopNetworkSharing.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, StopNetworkSharing03, TestSize.Level1)
{
    SharingIfaceType type = SharingIfaceType::SHARING_BLUETOOTH;
    int32_t ret = NetworkShareTracker::GetInstance().StopNetworkSharing(type);
    EXPECT_GE(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: GetSharableRegexs01
 * @tc.desc: Test NetworkShareTracker GetSharableRegexs.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, GetSharableRegexs01, TestSize.Level1)
{
    SharingIfaceType type = SharingIfaceType::SHARING_BLUETOOTH;
    std::vector<std::string> ret;
    NetworkShareTracker::GetInstance().GetSharableRegexs(type, ret);
    EXPECT_GE(ret.size(), static_cast<uint32_t>(0));
}

/**
 * @tc.name: GetSharableRegexs02
 * @tc.desc: Test NetworkShareTracker GetSharableRegexs.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, GetSharableRegexs02, TestSize.Level1)
{
    SharingIfaceType type = SharingIfaceType::SHARING_USB;
    std::vector<std::string> ret;
    NetworkShareTracker::GetInstance().GetSharableRegexs(type, ret);
    EXPECT_GE(ret.size(), static_cast<uint32_t>(0));
}

/**
 * @tc.name: GetSharableRegexs03
 * @tc.desc: Test NetworkShareTracker GetSharableRegexs.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, GetSharableRegexs03, TestSize.Level1)
{
    SharingIfaceType type = SharingIfaceType::SHARING_WIFI;
    std::vector<std::string> ret;
    NetworkShareTracker::GetInstance().GetSharableRegexs(type, ret);
    EXPECT_GE(ret.size(), static_cast<uint32_t>(0));
}

/**
 * @tc.name: GetSharingState01
 * @tc.desc: Test NetworkShareTracker GetSharingState.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, GetSharingState01, TestSize.Level1)
{
    SharingIfaceType type = SharingIfaceType::SHARING_WIFI;
    SharingIfaceState state = SharingIfaceState::SHARING_NIC_SERVING;
    int32_t ret = NetworkShareTracker::GetInstance().GetSharingState(type, state);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: GetSharingState02
 * @tc.desc: Test NetworkShareTracker GetSharingState.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, GetSharingState02, TestSize.Level1)
{
    SharingIfaceType type = SharingIfaceType::SHARING_USB;
    SharingIfaceState state = SharingIfaceState::SHARING_NIC_SERVING;
    int32_t ret = NetworkShareTracker::GetInstance().GetSharingState(type, state);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: GetSharingState03
 * @tc.desc: Test NetworkShareTracker GetSharingState.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, GetSharingState03, TestSize.Level1)
{
    SharingIfaceType type = SharingIfaceType::SHARING_BLUETOOTH;
    SharingIfaceState state = SharingIfaceState::SHARING_NIC_SERVING;
    int32_t ret = NetworkShareTracker::GetInstance().GetSharingState(type, state);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: GetNetSharingIfaces01
 * @tc.desc: Test NetworkShareTracker GetNetSharingIfaces.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, GetNetSharingIfaces01, TestSize.Level1)
{
    SharingIfaceState state = SharingIfaceState::SHARING_NIC_ERROR;
    std::vector<std::string> ifaces;
    NetworkShareTracker::GetInstance().GetNetSharingIfaces(state, ifaces);
    EXPECT_GE(ifaces.size(), static_cast<uint32_t>(0));
}

/**
 * @tc.name: GetNetSharingIfaces02
 * @tc.desc: Test NetworkShareTracker GetNetSharingIfaces.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, GetNetSharingIfaces02, TestSize.Level1)
{
    SharingIfaceState state = SharingIfaceState::SHARING_NIC_CAN_SERVER;
    std::vector<std::string> ifaces;
    NetworkShareTracker::GetInstance().GetNetSharingIfaces(state, ifaces);
    EXPECT_GE(ifaces.size(), static_cast<uint32_t>(0));
}

/**
 * @tc.name: GetNetSharingIfaces03
 * @tc.desc: Test NetworkShareTracker GetNetSharingIfaces.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, GetNetSharingIfaces03, TestSize.Level1)
{
    SharingIfaceState state = SharingIfaceState::SHARING_NIC_SERVING;
    std::vector<std::string> ifaces;
    NetworkShareTracker::GetInstance().GetNetSharingIfaces(state, ifaces);
    EXPECT_GE(ifaces.size(), static_cast<uint32_t>(0));
}

/**
 * @tc.name: RegisterSharingEvent01
 * @tc.desc: Test NetworkShareTracker RegisterSharingEvent.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, RegisterSharingEvent01, TestSize.Level1)
{
    sptr<ISharingEventCallback> callback = nullptr;
    int32_t ret = NetworkShareTracker::GetInstance().RegisterSharingEvent(callback);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_LOCAL_PTR_NULL);
}

/**
 * @tc.name: RegisterSharingEvent02
 * @tc.desc: Test NetworkShareTracker RegisterSharingEvent.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, RegisterSharingEvent02, TestSize.Level1)
{
    for (int32_t i = 0; i < MAX_CALLBACK_COUNT; i++) {
        g_callbackMap[i] = new (std::nothrow) SharingEventTestCallback();
    }

    std::for_each(g_callbackMap.begin(), g_callbackMap.end(),
                  [this](const auto &pair) { NetworkShareTracker::GetInstance().RegisterSharingEvent(pair.second); });
    sptr<ISharingEventCallback> callback = new (std::nothrow) SharingEventTestCallback();
    int32_t ret = NetworkShareTracker::GetInstance().RegisterSharingEvent(callback);
    EXPECT_EQ(ret, NETWORKSHARE_ERROR_ISSHARING_CALLBACK_ERROR);
    std::for_each(g_callbackMap.begin(), g_callbackMap.end(),
                  [this](const auto &pair) { NetworkShareTracker::GetInstance().UnregisterSharingEvent(pair.second); });
    NetworkShareTracker::GetInstance().UnregisterSharingEvent(callback);
}

/**
 * @tc.name: RegisterSharingEvent03
 * @tc.desc: Test NetworkShareTracker RegisterSharingEvent.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, RegisterSharingEvent03, TestSize.Level1)
{
    sptr<ISharingEventCallback> callback = new (std::nothrow) SharingEventTestCallback();
    int32_t ret = NetworkShareTracker::GetInstance().RegisterSharingEvent(callback);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: UpstreamWanted01
 * @tc.desc: Test NetworkShareTracker UpstreamWanted.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, UpstreamWanted01, TestSize.Level1)
{
    bool ret = NetworkShareTracker::GetInstance().UpstreamWanted();
    EXPECT_GE(ret, 0);
}

/**
 * @tc.name: ModifySharedSubStateMachineList01
 * @tc.desc: Test NetworkShareTracker ModifySharedSubStateMachineList.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, ModifySharedSubStateMachineList01, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    std::shared_ptr<NetworkShareSubStateMachine> subSm = std::make_shared<NetworkShareSubStateMachine>(
        WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    bool isAdd = true;
    NetworkShareTracker::GetInstance().ModifySharedSubStateMachineList(isAdd, subSm);
    EXPECT_NE(configuration, nullptr);
}

/**
 * @tc.name: ModifySharedSubStateMachineList02
 * @tc.desc: Test NetworkShareTracker ModifySharedSubStateMachineList.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, ModifySharedSubStateMachineList02, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    std::shared_ptr<NetworkShareSubStateMachine> subSm = std::make_shared<NetworkShareSubStateMachine>(
        BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_BLUETOOTH, configuration);
    bool isAdd = false;
    NetworkShareTracker::GetInstance().ModifySharedSubStateMachineList(isAdd, subSm);
    EXPECT_NE(configuration, nullptr);
}

/**
 * @tc.name: GetMainStateMachine01
 * @tc.desc: Test NetworkShareTracker GetMainStateMachine.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, GetMainStateMachine01, TestSize.Level1)
{
    std::shared_ptr<NetworkShareMainStateMachine> mainStateMachine =
        NetworkShareTracker::GetInstance().GetMainStateMachine();
    EXPECT_NE(mainStateMachine, nullptr);
}

/**
 * @tc.name: SetUpstreamNetHandle01
 * @tc.desc: Test NetworkShareTracker SetUpstreamNetHandle.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, SetUpstreamNetHandle01, TestSize.Level1)
{
    std::shared_ptr<UpstreamNetworkInfo> netinfo = nullptr;
    NetworkShareTracker::GetInstance().SetUpstreamNetHandle(netinfo);
    EXPECT_EQ(netinfo, nullptr);
}

/**
 * @tc.name: SetUpstreamNetHandle02
 * @tc.desc: Test NetworkShareTracker SetUpstreamNetHandle.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, SetUpstreamNetHandle02, TestSize.Level1)
{
    sptr<NetHandle> netHandle = nullptr;
    sptr<NetAllCapabilities> netcap = nullptr;
    sptr<NetLinkInfo> netlinkinfo = nullptr;
    std::shared_ptr<UpstreamNetworkInfo> netinfo =
        std::make_shared<UpstreamNetworkInfo>(netHandle, netcap, netlinkinfo);
    NetworkShareTracker::GetInstance().SetUpstreamNetHandle(netinfo);
    EXPECT_EQ(netinfo->netHandle_, nullptr);
}

/**
 * @tc.name: SetUpstreamNetHandle03
 * @tc.desc: Test NetworkShareTracker SetUpstreamNetHandle.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, SetUpstreamNetHandle03, TestSize.Level1)
{
    sptr<NetHandle> netHandle = nullptr;
    sptr<NetAllCapabilities> netcap = nullptr;
    sptr<NetLinkInfo> netlinkinfo = nullptr;
    std::shared_ptr<UpstreamNetworkInfo> netinfo =
        std::make_shared<UpstreamNetworkInfo>(netHandle, netcap, netlinkinfo);
    netinfo->netHandle_ = new (std::nothrow) NetHandle();
    NetworkShareTracker::GetInstance().SetUpstreamNetHandle(netinfo);
    EXPECT_NE(netinfo, nullptr);
}

/**
 * @tc.name: GetUpstreamInfo01
 * @tc.desc: Test NetworkShareTracker GetUpstreamInfo.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, GetUpstreamInfo01, TestSize.Level1)
{
    std::shared_ptr<UpstreamNetworkInfo> upstreamInfo = nullptr;

    NetworkShareTracker::GetInstance().GetUpstreamInfo(upstreamInfo);
    EXPECT_NE(upstreamInfo, nullptr);
}

/**
 * @tc.name: NotifyDownstreamsHasNewUpstreamIface01
 * @tc.desc: Test NetworkShareTracker NotifyDownstreamsHasNewUpstreamIface.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, NotifyDownstreamsHasNewUpstreamIface01, TestSize.Level1)
{
    sptr<NetHandle> netHandle = nullptr;
    sptr<NetAllCapabilities> netcap = nullptr;
    sptr<NetLinkInfo> netlinkinfo = nullptr;
    std::shared_ptr<UpstreamNetworkInfo> netinfo =
        std::make_shared<UpstreamNetworkInfo>(netHandle, netcap, netlinkinfo);

    NetworkShareTracker::GetInstance().NotifyDownstreamsHasNewUpstreamIface(netinfo);
    EXPECT_NE(netinfo, nullptr);
}

/**
 * @tc.name: GetSharedSubSMTraffic01
 * @tc.desc: Test NetworkShareTracker GetSharedSubSMTraffic.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, GetSharedSubSMTraffic01, TestSize.Level1)
{
    TrafficType type = TrafficType::TRAFFIC_ALL;
    int32_t kbByte;
    NetworkShareTracker::GetInstance().GetSharedSubSMTraffic(type, kbByte);
    EXPECT_GE(kbByte, 0);
}

/**
 * @tc.name: GetSharedSubSMTraffic02
 * @tc.desc: Test NetworkShareTracker GetSharedSubSMTraffic.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, GetSharedSubSMTraffic02, TestSize.Level1)
{
    TrafficType type = TrafficType::TRAFFIC_RX;
    int32_t kbByte;
    NetworkShareTracker::GetInstance().GetSharedSubSMTraffic(type, kbByte);
    EXPECT_GE(kbByte, 0);
}

/**
 * @tc.name: GetSharedSubSMTraffic03
 * @tc.desc: Test NetworkShareTracker GetSharedSubSMTraffic.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, GetSharedSubSMTraffic03, TestSize.Level1)
{
    TrafficType type = TrafficType::TRAFFIC_TX;
    int32_t kbByte;
    NetworkShareTracker::GetInstance().GetSharedSubSMTraffic(type, kbByte);
    EXPECT_GE(kbByte, 0);
}
} // namespace NetManagerStandard
} // namespace OHOS
