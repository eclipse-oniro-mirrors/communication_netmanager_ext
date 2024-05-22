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

#include <gtest/gtest.h>
#ifdef GTEST_API_
#define private public
#define protected public
#endif
#include "sharing_event_callback_stub.h"
#include "networkshare_tracker.h"
#ifdef BLUETOOTH_MODOULE
#include "bluetooth_pan.h"
#include "bluetooth_remote_device.h"
#endif

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
static constexpr const char *WIFI_AP_DEFAULT_IFACE_NAME = "wlan0";
static constexpr const char *USB_AP_DEFAULT_IFACE_NAME = "usb0";
static constexpr const char *USB_AP_RNDIS_IFACE_NAME = "rndis0";
static constexpr const char *BLUETOOTH_DEFAULT_IFACE_NAME = "bt-pan";
static constexpr const char *TEST_IFACE_NAME = "testIface";
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
    static inline std::shared_ptr<NetworkShareTracker> instance_ = nullptr;
};

void NetworkShareTrackerTest::SetUpTestCase() {}

void NetworkShareTrackerTest::TearDownTestCase() {}

void NetworkShareTrackerTest::SetUp()
{
    instance_ = DelayedSingleton<NetworkShareTracker>::GetInstance();
}

void NetworkShareTrackerTest::TearDown()
{
    NetworkShareTracker::GetInstance().Uninit();
}

HWTEST_F(NetworkShareTrackerTest, IsNetworkSharingSupported00, TestSize.Level1)
{
    int32_t supported;
    auto nret = NetworkShareTracker::GetInstance().IsNetworkSharingSupported(supported);
    EXPECT_EQ(nret, NETWORKSHARE_ERROR_IFACE_CFG_ERROR);
}

HWTEST_F(NetworkShareTrackerTest, GetSharableRegexs00, TestSize.Level1)
{
    SharingIfaceType type = SharingIfaceType::SHARING_BLUETOOTH;
    std::vector<std::string> ret;
    auto nret = NetworkShareTracker::GetInstance().GetSharableRegexs(type, ret);
    EXPECT_EQ(nret, NETWORKSHARE_ERROR_IFACE_CFG_ERROR);
}

HWTEST_F(NetworkShareTrackerTest, SetUpstreamNetHandle00, TestSize.Level1)
{
    sptr<NetHandle> netHandle = new (std::nothrow) NetHandle();
    sptr<NetAllCapabilities> netcap = nullptr;
    sptr<NetLinkInfo> netlinkinfo = nullptr;
    std::shared_ptr<UpstreamNetworkInfo> netinfo =
        std::make_shared<UpstreamNetworkInfo>(netHandle, netcap, netlinkinfo);
    NetworkShareTracker::GetInstance().SetUpstreamNetHandle(netinfo);
    EXPECT_EQ(NetworkShareTracker::GetInstance().mainStateMachine_, nullptr);
}

HWTEST_F(NetworkShareTrackerTest, SendMainSMEvent00, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto subSM = std::make_shared<NetworkShareSubStateMachine>(WIFI_AP_DEFAULT_IFACE_NAME,
                                                               SharingIfaceType::SHARING_WIFI, configuration);

    NetworkShareTracker::GetInstance().SendMainSMEvent(subSM, 0, 0);
    EXPECT_EQ(NetworkShareTracker::GetInstance().mainStateMachine_, nullptr);
}

HWTEST_F(NetworkShareTrackerTest, IsInterfaceMatchType00, TestSize.Level1)
{
    auto ret = NetworkShareTracker::GetInstance().IsInterfaceMatchType(WIFI_AP_DEFAULT_IFACE_NAME,
                                                                       SharingIfaceType::SHARING_WIFI);
    EXPECT_FALSE(ret);
}

HWTEST_F(NetworkShareTrackerTest, InterfaceNameToType00, TestSize.Level1)
{
    std::string iface = TEST_IFACE_NAME;
    SharingIfaceType interfaceType;

    NetworkShareTracker::GetInstance().InterfaceStatusChanged(TEST_IFACE_NAME, false);
    NetworkShareTracker::GetInstance().InterfaceAdded(TEST_IFACE_NAME);
    NetworkShareTracker::GetInstance().InterfaceRemoved(TEST_IFACE_NAME);
    auto ret = NetworkShareTracker::GetInstance().InterfaceNameToType(iface, interfaceType);
    EXPECT_FALSE(ret);
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
    int32_t ret = NetworkShareTracker::GetInstance().StartNetworkSharing(type);
    EXPECT_EQ(ret, NETWORKSHARE_ERROR_WIFI_SHARING);

    ret = NetworkShareTracker::GetInstance().StartNetworkSharing(type);
    EXPECT_EQ(ret, NETWORKSHARE_ERROR_WIFI_SHARING);

    type = SharingIfaceType::SHARING_USB;
    ret = NetworkShareTracker::GetInstance().StartNetworkSharing(type);
    EXPECT_EQ(ret, NETWORKSHARE_ERROR_USB_SHARING);
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
    EXPECT_EQ(ret, NETWORKSHARE_ERROR_WIFI_SHARING);

    type = SharingIfaceType::SHARING_USB;
    ret = NetworkShareTracker::GetInstance().StopNetworkSharing(type);
    EXPECT_EQ(ret, NETWORKSHARE_ERROR_USB_SHARING);
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
    auto nret = NetworkShareTracker::GetInstance().GetSharableRegexs(type, ret);
    EXPECT_EQ(nret, NETMANAGER_EXT_SUCCESS);

    type = SharingIfaceType::SHARING_USB;
    nret = NetworkShareTracker::GetInstance().GetSharableRegexs(type, ret);
    EXPECT_EQ(nret, NETMANAGER_EXT_SUCCESS);

    type = SharingIfaceType::SHARING_WIFI;
    nret = NetworkShareTracker::GetInstance().GetSharableRegexs(type, ret);
    EXPECT_EQ(nret, NETMANAGER_EXT_SUCCESS);
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

    NetworkShareTracker::GetInstance().subStateMachineMap_.clear();
    NetworkShareTracker::GetInstance().subStateMachineMap_.insert(
        std::make_pair(BLUETOOTH_DEFAULT_IFACE_NAME, nullptr));
    NetworkShareTracker::GetInstance().subStateMachineMap_.insert(
        std::make_pair(WIFI_AP_DEFAULT_IFACE_NAME, nullptr));

    int32_t ret = NetworkShareTracker::GetInstance().GetSharingState(type, state);
    NetworkShareTracker::GetInstance().subStateMachineMap_.clear();
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(state, SharingIfaceState::SHARING_NIC_CAN_SERVER);
}

/**
 * @tc.name: GetSharingState02
 * @tc.desc: Test NetworkShareTracker GetSharingState.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, GetSharingState02, TestSize.Level1)
{
    SharingIfaceType type = SharingIfaceType::SHARING_WIFI;
    SharingIfaceState state = SharingIfaceState::SHARING_NIC_SERVING;
    NetworkShareTracker::GetInstance().subStateMachineMap_.clear();
    NetworkShareTracker::GetInstance().CreateSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, type, false);
    auto itfind = NetworkShareTracker::GetInstance().subStateMachineMap_.find(WIFI_AP_DEFAULT_IFACE_NAME);
    ASSERT_NE(itfind, NetworkShareTracker::GetInstance().subStateMachineMap_.end());
    itfind->second->lastState_ = SUB_SM_STATE_UNAVAILABLE;

    int32_t ret = NetworkShareTracker::GetInstance().GetSharingState(type, state);
    NetworkShareTracker::GetInstance().subStateMachineMap_.clear();
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(state, SharingIfaceState::SHARING_NIC_ERROR);

    type = SharingIfaceType::SHARING_USB;
    NetworkShareTracker::GetInstance().CreateSubStateMachine(USB_AP_DEFAULT_IFACE_NAME, type, false);
    itfind = NetworkShareTracker::GetInstance().subStateMachineMap_.find(USB_AP_DEFAULT_IFACE_NAME);
    ASSERT_NE(itfind, NetworkShareTracker::GetInstance().subStateMachineMap_.end());
    itfind->second->lastState_ = SUB_SM_STATE_AVAILABLE;

    ret = NetworkShareTracker::GetInstance().GetSharingState(type, state);
    NetworkShareTracker::GetInstance().subStateMachineMap_.clear();
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(state, SharingIfaceState::SHARING_NIC_CAN_SERVER);

    type = SharingIfaceType::SHARING_BLUETOOTH;
    NetworkShareTracker::GetInstance().CreateSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, type, false);
    itfind = NetworkShareTracker::GetInstance().subStateMachineMap_.find(BLUETOOTH_DEFAULT_IFACE_NAME);
    ASSERT_NE(itfind, NetworkShareTracker::GetInstance().subStateMachineMap_.end());
    itfind->second->lastState_ = SUB_SM_STATE_SHARED;

    ret = NetworkShareTracker::GetInstance().GetSharingState(type, state);
    NetworkShareTracker::GetInstance().subStateMachineMap_.clear();
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(state, SharingIfaceState::SHARING_NIC_SERVING);
}

/**
 * @tc.name: GetSharingState03
 * @tc.desc: Test NetworkShareTracker GetSharingState.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, GetSharingState03, TestSize.Level1)
{
    SharingIfaceType type = static_cast<SharingIfaceType>(3);
    SharingIfaceState state = SharingIfaceState::SHARING_NIC_SERVING;
    int32_t ret = NetworkShareTracker::GetInstance().GetSharingState(type, state);
    EXPECT_EQ(ret, NETWORKSHARE_ERROR_UNKNOWN_TYPE);
}

/**
 * @tc.name: GetNetSharingIfaces01
 * @tc.desc: Test NetworkShareTracker GetNetSharingIfaces.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, GetNetSharingIfaces01, TestSize.Level1)
{
    SharingIfaceType type = SharingIfaceType::SHARING_WIFI;
    SharingIfaceState state = SharingIfaceState::SHARING_NIC_ERROR;
    std::vector<std::string> ifaces;
    NetworkShareTracker::GetInstance().subStateMachineMap_.clear();
    NetworkShareTracker::GetInstance().subStateMachineMap_.insert(
        std::make_pair(BLUETOOTH_DEFAULT_IFACE_NAME, nullptr));

    NetworkShareTracker::GetInstance().GetNetSharingIfaces(state, ifaces);
    EXPECT_EQ(ifaces.size(), 0);

    NetworkShareTracker::GetInstance().subStateMachineMap_.clear();
    NetworkShareTracker::GetInstance().CreateSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, type, false);
    auto itfind = NetworkShareTracker::GetInstance().subStateMachineMap_.find(WIFI_AP_DEFAULT_IFACE_NAME);
    ASSERT_NE(itfind, NetworkShareTracker::GetInstance().subStateMachineMap_.end());
    itfind->second->lastState_ = SUB_SM_STATE_UNAVAILABLE;
    NetworkShareTracker::GetInstance().GetNetSharingIfaces(state, ifaces);
    NetworkShareTracker::GetInstance().subStateMachineMap_.clear();
    EXPECT_EQ(ifaces.at(0), WIFI_AP_DEFAULT_IFACE_NAME);
}

/**
 * @tc.name: GetNetSharingIfaces02
 * @tc.desc: Test NetworkShareTracker GetNetSharingIfaces.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, GetNetSharingIfaces02, TestSize.Level1)
{
    SharingIfaceState state = static_cast<SharingIfaceState>(4);
    std::vector<std::string> ifaces;
    int32_t ret = NetworkShareTracker::GetInstance().GetNetSharingIfaces(state, ifaces);
    EXPECT_EQ(ret, NETWORKSHARE_ERROR_UNKNOWN_TYPE);
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

    std::for_each(g_callbackMap.begin(), g_callbackMap.end(), [this](const auto &pair) {
        NetworkShareTracker::GetInstance().RegisterSharingEvent(pair.second);
        });
    sptr<ISharingEventCallback> callback = new (std::nothrow) SharingEventTestCallback();
    int32_t ret = NetworkShareTracker::GetInstance().RegisterSharingEvent(callback);
    EXPECT_EQ(ret, NETWORKSHARE_ERROR_ISSHARING_CALLBACK_ERROR);
    std::for_each(g_callbackMap.begin(), g_callbackMap.end(), [this](const auto &pair) {
        NetworkShareTracker::GetInstance().UnregisterSharingEvent(pair.second);
        });
    NetworkShareTracker::GetInstance().UnregisterSharingEvent(callback);
    EXPECT_EQ(NetworkShareTracker::GetInstance().sharingEventCallback_.size(), 0);
}

/**
 * @tc.name: UpstreamWanted01
 * @tc.desc: Test NetworkShareTracker UpstreamWanted.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, UpstreamWanted01, TestSize.Level1)
{
    bool ret = NetworkShareTracker::GetInstance().UpstreamWanted();
    EXPECT_TRUE(ret);
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
    auto oldsize = NetworkShareTracker::GetInstance().sharedSubSM_.size();
    NetworkShareTracker::GetInstance().ModifySharedSubStateMachineList(true, subSm);
    auto subsm2 = std::make_shared<NetworkShareSubStateMachine>(
        BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_BLUETOOTH, configuration);
    NetworkShareTracker::GetInstance().ModifySharedSubStateMachineList(true, subsm2);
    EXPECT_EQ(NetworkShareTracker::GetInstance().sharedSubSM_.size(), oldsize + 2);

    NetworkShareTracker::GetInstance().ModifySharedSubStateMachineList(false, subsm2);
    EXPECT_EQ(NetworkShareTracker::GetInstance().sharedSubSM_.size(), oldsize + 1);
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
    sptr<NetHandle> netHandle = new (std::nothrow) NetHandle(-1);
    sptr<NetAllCapabilities> netcap = nullptr;
    sptr<NetLinkInfo> netlinkinfo = nullptr;
    std::shared_ptr<UpstreamNetworkInfo> netinfo =
        std::make_shared<UpstreamNetworkInfo>(netHandle, netcap, netlinkinfo);
    NetworkShareTracker::GetInstance().SetUpstreamNetHandle(netinfo);
    EXPECT_EQ(NetworkShareTracker::GetInstance().mainStateMachine_->errorType_, CMD_SET_DNS_FORWARDERS_ERROR);
}

/**
 * @tc.name: SetUpstreamNetHandle03
 * @tc.desc: Test NetworkShareTracker SetUpstreamNetHandle.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareTrackerTest, SetUpstreamNetHandle03, TestSize.Level1)
{
    sptr<NetHandle> netHandle = new (std::nothrow) NetHandle();
    sptr<NetAllCapabilities> netcap = nullptr;
    sptr<NetLinkInfo> netlinkinfo = nullptr;
    std::shared_ptr<UpstreamNetworkInfo> netinfo =
        std::make_shared<UpstreamNetworkInfo>(netHandle, netcap, netlinkinfo);
    NetworkShareTracker::GetInstance().mainStateMachine_->SwitcheToErrorState(NETWORKSHARING_SHARING_NO_ERROR);
    NetworkShareTracker::GetInstance().SetUpstreamNetHandle(netinfo);
    EXPECT_EQ(NetworkShareTracker::GetInstance().mainStateMachine_->errorType_, NETWORKSHARING_SHARING_NO_ERROR);
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
    EXPECT_EQ(NetworkShareTracker::GetInstance().upstreamInfo_.get(), netinfo.get());
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

#ifdef WIFI_MODOULE
HWTEST_F(NetworkShareTrackerTest, OnWifiHotspotStateChanged01, TestSize.Level1)
{
    int32_t state = 2;
    NetworkShareTracker::GetInstance().OnWifiHotspotStateChanged(state);
    EXPECT_EQ(NetworkShareTracker::GetInstance().curWifiState_, Wifi::ApState::AP_STATE_STARTING);

    state = 3;
    NetworkShareTracker::GetInstance().OnWifiHotspotStateChanged(state);
    EXPECT_EQ(NetworkShareTracker::GetInstance().curWifiState_, Wifi::ApState::AP_STATE_STARTED);

    state = 4;
    NetworkShareTracker::GetInstance().OnWifiHotspotStateChanged(state);
    EXPECT_EQ(NetworkShareTracker::GetInstance().curWifiState_, Wifi::ApState::AP_STATE_CLOSING);

    state = 5;
    NetworkShareTracker::GetInstance().OnWifiHotspotStateChanged(state);
    EXPECT_EQ(NetworkShareTracker::GetInstance().curWifiState_, Wifi::ApState::AP_STATE_CLOSED);

    state = 0;
    NetworkShareTracker::GetInstance().OnWifiHotspotStateChanged(state);
    EXPECT_EQ(NetworkShareTracker::GetInstance().curWifiState_, Wifi::ApState::AP_STATE_NONE);
}
#endif

HWTEST_F(NetworkShareTrackerTest, EnableNetSharingInternal01, TestSize.Level1)
{
    SharingIfaceType type = static_cast<SharingIfaceType>(3);
    auto ret = NetworkShareTracker::GetInstance().EnableNetSharingInternal(type, false);
    EXPECT_EQ(ret, NETWORKSHARE_ERROR_UNKNOWN_TYPE);
}

HWTEST_F(NetworkShareTrackerTest, Sharing01, TestSize.Level1)
{
    std::string iface = "testIface";
    int32_t reqState = 0;
    int32_t ret = NetworkShareTracker::GetInstance().Sharing(iface, reqState);
    EXPECT_EQ(NETWORKSHARE_ERROR_UNKNOWN_IFACE, ret);
}

HWTEST_F(NetworkShareTrackerTest, EnableWifiSubStateMachine01, TestSize.Level1)
{
    NetworkShareTracker::GetInstance().subStateMachineMap_.insert(
        std::make_pair(BLUETOOTH_DEFAULT_IFACE_NAME, nullptr));
    NetworkShareTracker::GetInstance().subStateMachineMap_.insert(
        std::make_pair(WIFI_AP_DEFAULT_IFACE_NAME, nullptr));
    NetworkShareTracker::GetInstance().EnableWifiSubStateMachine();
    auto iter = NetworkShareTracker::GetInstance().subStateMachineMap_.find(WIFI_AP_DEFAULT_IFACE_NAME);
    EXPECT_NE(iter, NetworkShareTracker::GetInstance().subStateMachineMap_.end());
}

HWTEST_F(NetworkShareTrackerTest, EnableBluetoothSubStateMachine01, TestSize.Level1)
{
    NetworkShareTracker::GetInstance().EnableBluetoothSubStateMachine();
    auto iter = NetworkShareTracker::GetInstance().subStateMachineMap_.find(BLUETOOTH_DEFAULT_IFACE_NAME);
    EXPECT_NE(iter, NetworkShareTracker::GetInstance().subStateMachineMap_.end());
}

HWTEST_F(NetworkShareTrackerTest, StopDnsProxy01, TestSize.Level1)
{
    NetworkShareTracker::GetInstance().isStartDnsProxy_ = true;
    NetworkShareTracker::GetInstance().StopDnsProxy();
    EXPECT_FALSE(NetworkShareTracker::GetInstance().isStartDnsProxy_);
}

HWTEST_F(NetworkShareTrackerTest, StopSubStateMachine01, TestSize.Level1)
{
    std::string iface = TEST_IFACE_NAME;
    SharingIfaceType interfaceType = static_cast<SharingIfaceType>(3);
    NetworkShareTracker::GetInstance().StopSubStateMachine(iface, interfaceType);
    auto itfind = NetworkShareTracker::GetInstance().subStateMachineMap_.find(TEST_IFACE_NAME);
    EXPECT_EQ(itfind, NetworkShareTracker::GetInstance().subStateMachineMap_.end());
}

HWTEST_F(NetworkShareTrackerTest, InterfaceNameToType01, TestSize.Level1)
{
    std::string iface = TEST_IFACE_NAME;
    SharingIfaceType interfaceType;
    auto ret = NetworkShareTracker::GetInstance().InterfaceNameToType(iface, interfaceType);
    EXPECT_FALSE(ret);

    iface = WIFI_AP_DEFAULT_IFACE_NAME;
    NetworkShareTracker::GetInstance().InterfaceNameToType(iface, interfaceType);
    EXPECT_EQ(interfaceType, SharingIfaceType::SHARING_WIFI);

    iface = USB_AP_DEFAULT_IFACE_NAME;
    NetworkShareTracker::GetInstance().InterfaceNameToType(iface, interfaceType);
    EXPECT_EQ(interfaceType, SharingIfaceType::SHARING_USB);

    iface = BLUETOOTH_DEFAULT_IFACE_NAME;
    NetworkShareTracker::GetInstance().InterfaceNameToType(iface, interfaceType);
    EXPECT_EQ(interfaceType, SharingIfaceType::SHARING_BLUETOOTH);
}

HWTEST_F(NetworkShareTrackerTest, IsHandleNetlinkEvent01, TestSize.Level1)
{
    SharingIfaceType type;
    bool ret = false;
#ifdef WIFI_MODOULE
    type = SharingIfaceType::SHARING_WIFI;
    NetworkShareTracker::GetInstance().curWifiState_ = Wifi::ApState::AP_STATE_CLOSING;
    ret = NetworkShareTracker::GetInstance().IsHandleNetlinkEvent(type, false);
    EXPECT_TRUE(ret);
#endif
#ifdef USB_MODOULE
    type = SharingIfaceType::SHARING_USB;
    NetworkShareTracker::GetInstance().curUsbState_ = UsbShareState::USB_CLOSING;
    ret = NetworkShareTracker::GetInstance().IsHandleNetlinkEvent(type, false);
    EXPECT_TRUE(ret);
#endif
    NetworkShareTracker::GetInstance().InterfaceStatusChanged(TEST_IFACE_NAME, false);
    NetworkShareTracker::GetInstance().InterfaceStatusChanged(WIFI_AP_DEFAULT_IFACE_NAME, false);
#ifdef WIFI_MODOULE
    NetworkShareTracker::GetInstance().curWifiState_ = Wifi::ApState::AP_STATE_STARTING;
#endif
    NetworkShareTracker::GetInstance().InterfaceStatusChanged(WIFI_AP_DEFAULT_IFACE_NAME, true);
    NetworkShareTracker::GetInstance().InterfaceStatusChanged(USB_AP_RNDIS_IFACE_NAME, true);

    NetworkShareTracker::GetInstance().InterfaceAdded(TEST_IFACE_NAME);
    NetworkShareTracker::GetInstance().InterfaceAdded(WIFI_AP_DEFAULT_IFACE_NAME);

    NetworkShareTracker::GetInstance().InterfaceRemoved(TEST_IFACE_NAME);
    NetworkShareTracker::GetInstance().InterfaceAdded(WIFI_AP_DEFAULT_IFACE_NAME);

#ifdef BLUETOOTH_MODOULE
    type = SharingIfaceType::SHARING_BLUETOOTH;
    NetworkShareTracker::GetInstance().curBluetoothState_ = Bluetooth::BTConnectState::CONNECTING;
    ret = NetworkShareTracker::GetInstance().IsHandleNetlinkEvent(type, true);
    EXPECT_TRUE(ret);
#endif

    type = static_cast<SharingIfaceType>(3);
    ret = NetworkShareTracker::GetInstance().IsHandleNetlinkEvent(type, false);
    EXPECT_FALSE(ret);
}

HWTEST_F(NetworkShareTrackerTest, SendSharingUpstreamChange01, TestSize.Level1)
{
    NetworkShareTracker::GetInstance().sharingEventCallback_.clear();
    NetworkShareTracker::GetInstance().SendSharingUpstreamChange(nullptr);

    sptr<ISharingEventCallback> callback = new (std::nothrow) SharingEventTestCallback();
    NetworkShareTracker::GetInstance().RegisterSharingEvent(callback);
    NetworkShareTracker::GetInstance().SendSharingUpstreamChange(nullptr);
    EXPECT_GE(NetworkShareTracker::GetInstance().sharingEventCallback_.size(), 0);
}

HWTEST_F(NetworkShareTrackerTest, SubSmStateToExportState01, TestSize.Level1)
{
    int state = SUB_SM_STATE_AVAILABLE;
    auto ret = NetworkShareTracker::GetInstance().SubSmStateToExportState(state);
    EXPECT_EQ(ret, SharingIfaceState::SHARING_NIC_CAN_SERVER);

    state = SUB_SM_STATE_SHARED;
    ret = NetworkShareTracker::GetInstance().SubSmStateToExportState(state);
    EXPECT_EQ(ret, SharingIfaceState::SHARING_NIC_SERVING);

    state = SUB_SM_STATE_UNAVAILABLE;
    ret = NetworkShareTracker::GetInstance().SubSmStateToExportState(state);
    EXPECT_EQ(ret, SharingIfaceState::SHARING_NIC_ERROR);

    state = 4;
    ret = NetworkShareTracker::GetInstance().SubSmStateToExportState(state);
    EXPECT_EQ(ret, SharingIfaceState::SHARING_NIC_ERROR);
}

HWTEST_F(NetworkShareTrackerTest, OnChangeSharingState01, TestSize.Level1)
{
    NetworkShareTracker::GetInstance().clientRequestsVector_.push_back(SharingIfaceType::SHARING_WIFI);
    NetworkShareTracker::GetInstance().OnChangeSharingState(SharingIfaceType::SHARING_WIFI, false);
    EXPECT_EQ(NetworkShareTracker::GetInstance().clientRequestsVector_.size(), 0);
}

HWTEST_F(NetworkShareTrackerTest, NetworkShareTrackerBranchTest01, TestSize.Level1)
{
#ifdef BLUETOOTH_MODOULE
    NetworkShareTracker::GetInstance().SetBluetoothState(Bluetooth::BTConnectState::CONNECTING);
#endif

    NetworkShareTracker::NetsysCallback callback;
    std::string testString = "";
    int testNumber = 0;
    auto ret = callback.OnInterfaceAddressUpdated(testString, testString, testNumber, testNumber);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    ret = callback.OnInterfaceAddressRemoved(testString, testString, testNumber, testNumber);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    ret = callback.OnInterfaceAdded(testString);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    ret = callback.OnInterfaceRemoved(testString);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    ret = callback.OnInterfaceChanged(testString, false);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    ret = callback.OnInterfaceLinkStateChanged(testString, false);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    ret = callback.OnRouteChanged(false, testString, testString, testString);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    NetsysControllerCallback::DhcpResult dhcpResult;
    ret = callback.OnDhcpSuccess(dhcpResult);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    ret = callback.OnBandwidthReachedLimit(testString, testString);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

#ifdef BLUETOOTH_MODOULE
    std::shared_ptr<NetworkShareTracker::SharingPanObserver> observer =
        std::make_shared<NetworkShareTracker::SharingPanObserver>();
    Bluetooth::BluetoothRemoteDevice device;
    int32_t cause = 0;
    observer->OnConnectionStateChanged(device, static_cast<int32_t>(Bluetooth::BTConnectState::CONNECTING), cause);
    observer->OnConnectionStateChanged(device, static_cast<int32_t>(Bluetooth::BTConnectState::CONNECTED), cause);
    observer->OnConnectionStateChanged(device, static_cast<int32_t>(Bluetooth::BTConnectState::DISCONNECTING), cause);
    observer->OnConnectionStateChanged(device, static_cast<int32_t>(Bluetooth::BTConnectState::DISCONNECTED), cause);
    int32_t invalidValue = 100;
    observer->OnConnectionStateChanged(device, invalidValue, cause);
#endif
}
} // namespace NetManagerStandard
} // namespace OHOS
