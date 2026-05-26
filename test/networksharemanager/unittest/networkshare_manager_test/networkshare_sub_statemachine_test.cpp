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

#define private public
#include "networkshare_sub_statemachine.h"
#undef private

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
static constexpr const char *WIFI_AP_DEFAULT_IFACE_NAME = "wlan0";
static constexpr const char *BLUETOOTH_DEFAULT_IFACE_NAME = "bt-pan";
static constexpr const char *USB_DEFAULT_IFACE_NAME = "usb0";
static constexpr const char *EMPTY_UPSTREAM_IFACENAME = "";
constexpr int32_t SUBSTATE_TEST_ILLEGAL_VALUE = 0;
} // namespace

class NetworkShareSubStateMachineTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetworkShareSubStateMachineTest::SetUpTestCase() {}

void NetworkShareSubStateMachineTest::TearDownTestCase() {}

void NetworkShareSubStateMachineTest::SetUp() {}

void NetworkShareSubStateMachineTest::TearDown() {}

class SubStateMachineCallbackTest : public NetworkShareSubStateMachine::SubStateMachineCallback {
public:
    SubStateMachineCallbackTest() = default;
    virtual ~SubStateMachineCallbackTest() = default;
    void OnUpdateInterfaceState(const std::shared_ptr<NetworkShareSubStateMachine> &paraSubStateMachine, int state,
                                int lastError) override
    {
        return;
    }
};

/**
 * @tc.name: GetNetShareType01
 * @tc.desc: Test NetworkShareSubStateMachine GetNetShareType.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareSubStateMachineTest, GetNetShareType01, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    SharingIfaceType netShareType = networkShareSubStateMachine->GetNetShareType();
    EXPECT_EQ(netShareType, SharingIfaceType::SHARING_WIFI);
}

/**
 * @tc.name: GetNetShareType02
 * @tc.desc: Test NetworkShareSubStateMachine GetNetShareType.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareSubStateMachineTest, GetNetShareType02, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_BLUETOOTH, configuration);
    SharingIfaceType netShareType = networkShareSubStateMachine->GetNetShareType();
    EXPECT_EQ(netShareType, SharingIfaceType::SHARING_BLUETOOTH);
}

/**
 * @tc.name: GetInterfaceName01
 * @tc.desc: Test NetworkShareSubStateMachine GetInterfaceName.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareSubStateMachineTest, GetInterfaceName01, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    std::string ifaceName = networkShareSubStateMachine->GetInterfaceName();
    EXPECT_EQ(ifaceName, WIFI_AP_DEFAULT_IFACE_NAME);
}

/**
 * @tc.name: GetInterfaceName02
 * @tc.desc: Test NetworkShareSubStateMachine GetInterfaceName.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareSubStateMachineTest, GetInterfaceName02, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_BLUETOOTH, configuration);
    std::string ifaceName = networkShareSubStateMachine->GetInterfaceName();
    EXPECT_EQ(ifaceName, BLUETOOTH_DEFAULT_IFACE_NAME);
}

/**
 * @tc.name: RegisterSubSMCallback01
 * @tc.desc: Test NetworkShareSubStateMachine RegisterSubSMCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareSubStateMachineTest, RegisterSubSMCallback01, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);

    std::shared_ptr<NetworkShareSubStateMachine::SubStateMachineCallback> callback =
        std::make_shared<SubStateMachineCallbackTest>();
    networkShareSubStateMachine->RegisterSubSMCallback(callback);
    EXPECT_NE(callback, nullptr);
}

/**
 * @tc.name: SubSmStateSwitch01
 * @tc.desc: Test NetworkShareSubStateMachine SubSmStateSwitch.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareSubStateMachineTest, SubSmStateSwitch01, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    int newState = SUBSTATE_INIT;
    networkShareSubStateMachine->SubSmStateSwitch(newState);

    newState = SUBSTATE_SHARED;
    networkShareSubStateMachine->SubSmStateSwitch(newState);

    newState = SUBSTATE_UNAVAILABLE;
    networkShareSubStateMachine->SubSmStateSwitch(newState);

    newState = SUBSTATE_TEST_ILLEGAL_VALUE;
    networkShareSubStateMachine->SubSmStateSwitch(newState);
    networkShareSubStateMachine->SubSmStateSwitch(newState);
    EXPECT_NE(networkShareSubStateMachine, nullptr);
}

/**
 * @tc.name: GetDownIfaceName01
 * @tc.desc: Test NetworkShareSubStateMachine GetDownIfaceName.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareSubStateMachineTest, GetDownIfaceName01, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    int eventId = CMD_NETSHARE_REQUESTED;
    std::any messageObj = "";
    networkShareSubStateMachine->SubSmEventHandle(eventId, messageObj);

    std::string downIface;
    networkShareSubStateMachine->GetDownIfaceName(downIface);
    EXPECT_EQ(downIface, WIFI_AP_DEFAULT_IFACE_NAME);
}

/**
 * @tc.name: GetDownIfaceName02
 * @tc.desc: Test NetworkShareSubStateMachine GetDownIfaceName.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareSubStateMachineTest, GetDownIfaceName02, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_BLUETOOTH, configuration);
    std::string downIface;
    networkShareSubStateMachine->GetDownIfaceName(downIface);
    EXPECT_EQ(downIface, BLUETOOTH_DEFAULT_IFACE_NAME);
}

/**
 * @tc.name: HandleSharedUnrequest01
 * @tc.desc: Test NetworkShareSubStateMachine HandleSharedUnrequest.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareSubStateMachineTest, HandleSharedUnrequest01, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    std::any messageObj = "";
    int ret = networkShareSubStateMachine->HandleSharedUnrequest(messageObj);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: HandleSharedInterfaceDown01
 * @tc.desc: Test NetworkShareSubStateMachine HandleSharedInterfaceDown.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareSubStateMachineTest, HandleSharedInterfaceDown01, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    std::any messageObj = "";
    int ret = networkShareSubStateMachine->HandleSharedInterfaceDown(messageObj);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: GetBtDestinationAddr01
 * @tc.desc: Test NetworkShareSubStateMachine GetBtDestinationAddr.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareSubStateMachineTest, GetBtDestinationAddr01, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_BLUETOOTH, configuration);
    std::string addrStr;
    int ret = networkShareSubStateMachine->GetBtDestinationAddr(addrStr);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: GetUsbDestinationAddr01
 * @tc.desc: Test NetworkShareSubStateMachine GetUsbDestinationAddr.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareSubStateMachineTest, GetUsbDestinationAddr01, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(USB_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_USB, configuration);
    std::string addrStr;
    int ret = networkShareSubStateMachine->GetUsbDestinationAddr(addrStr);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: StartDhcp01
 * @tc.desc: Test NetworkShareSubStateMachine StartDhcp.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareSubStateMachineTest, StartDhcp01, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    EXPECT_NE(configuration, nullptr);
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_BLUETOOTH, configuration);
    std::shared_ptr<INetAddr> ipv4Address = nullptr;
    networkShareSubStateMachine->RequestIpv4Address(ipv4Address);
    networkShareSubStateMachine->StartDhcp(ipv4Address);
}

/**
 * @tc.name: StartDhcp02
 * @tc.desc: Test NetworkShareSubStateMachine StartDhcp.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareSubStateMachineTest, StartDhcp02, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_BLUETOOTH, configuration);
    std::shared_ptr<INetAddr> ipv4Address = nullptr;
    bool ret = networkShareSubStateMachine->StartDhcp(ipv4Address);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: StartDhcp03
 * @tc.desc: Test NetworkShareSubStateMachine StartDhcp.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareSubStateMachineTest, StartDhcp03, TestSize.Level1)
{
    std::shared_ptr<NetworkShareConfiguration> configuration = nullptr;
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_BLUETOOTH, configuration);
    std::shared_ptr<INetAddr> ipv4Address = nullptr;
    bool ret = networkShareSubStateMachine->StartDhcp(ipv4Address);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: RequestIpv4Address01
 * @tc.desc: Test NetworkShareSubStateMachine RequestIpv4Address.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareSubStateMachineTest, RequestIpv4Address01, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(USB_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_USB, configuration);
    std::shared_ptr<INetAddr> ipv4Address = nullptr;
    bool ret = networkShareSubStateMachine->RequestIpv4Address(ipv4Address);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: GetUpIfaceName01
 * @tc.desc: Test NetworkShareSubStateMachine GetUpIfaceName.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareSubStateMachineTest, GetUpIfaceName01, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    std::string upIface;
    networkShareSubStateMachine->GetUpIfaceName(upIface);
    EXPECT_EQ(upIface, EMPTY_UPSTREAM_IFACENAME);
}

/**
 * @tc.name: HasChangeUpstreamIfaceSet01
 * @tc.desc: Test NetworkShareSubStateMachine HasChangeUpstreamIfaceSet.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareSubStateMachineTest, HasChangeUpstreamIfaceSet01, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    networkShareSubStateMachine->upstreamIfaceName_ = "";
    std::string newUpstreamIface = "";
    bool ret = networkShareSubStateMachine->HasChangeUpstreamIfaceSet(newUpstreamIface);
    EXPECT_EQ(ret, false);
    newUpstreamIface = "Usb0";
    networkShareSubStateMachine->upstreamIfaceName_ = "Usb0";
    ret = networkShareSubStateMachine->HasChangeUpstreamIfaceSet(newUpstreamIface);
    EXPECT_EQ(ret, false);
    networkShareSubStateMachine->upstreamIfaceName_ = "Wlan0";
    ret = networkShareSubStateMachine->HasChangeUpstreamIfaceSet(newUpstreamIface);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: HandleConnectionChanged001
 * @tc.desc: Test NetworkShareSubStateMachine HandleConnectionChanged.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareSubStateMachineTest, HandleConnectionChanged001, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    ASSERT_NE(configuration, nullptr);
    ASSERT_NE(networkShareSubStateMachine, nullptr);
    sptr<NetHandle> netHandle = nullptr;
    sptr<NetAllCapabilities> netcap = nullptr;
    sptr<NetLinkInfo> netlinkinfo = nullptr;
    std::shared_ptr<UpstreamNetworkInfo> netinfo =
        std::make_shared<UpstreamNetworkInfo>(netHandle, netcap, netlinkinfo);
    networkShareSubStateMachine->HandleConnectionChanged(netinfo);
}

/**
 * @tc.name: HandleConnectionChanged002
 * @tc.desc: Test NetworkShareSubStateMachine HandleConnectionChanged.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareSubStateMachineTest, HandleConnectionChanged002, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    ASSERT_NE(configuration, nullptr);
    ASSERT_NE(networkShareSubStateMachine, nullptr);
    sptr<NetHandle> handle = nullptr;
    sptr<NetAllCapabilities> netcap = new (std::nothrow) NetManagerStandard::NetAllCapabilities();
    sptr<NetLinkInfo> link = new (std::nothrow) NetManagerStandard::NetLinkInfo();
    std::shared_ptr<UpstreamNetworkInfo> upstreamNetInfo = std::make_shared<UpstreamNetworkInfo>(handle, netcap, link);
    upstreamNetInfo->netLinkPro_->ifaceName_ = "";
    networkShareSubStateMachine->HandleConnectionChanged(upstreamNetInfo);
}

/**
 * @tc.name: HandleConnectionChanged003
 * @tc.desc: Test NetworkShareSubStateMachine HandleConnectionChanged.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareSubStateMachineTest, HandleConnectionChanged003, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    ASSERT_NE(configuration, nullptr);
    ASSERT_NE(networkShareSubStateMachine, nullptr);
    std::shared_ptr<UpstreamNetworkInfo> upstreamNetInfo = nullptr;
    networkShareSubStateMachine->HandleConnectionChanged(upstreamNetInfo);
}

/**
 * @tc.name: HandleConnectionChanged004
 * @tc.desc: Test NetworkShareSubStateMachine HandleConnectionChanged.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareSubStateMachineTest, HandleConnectionChanged004, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = std::make_shared<NetworkShareSubStateMachine>(
        WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    sptr<NetHandle> handle = nullptr;
    sptr<NetAllCapabilities> netcap = new (std::nothrow) NetManagerStandard::NetAllCapabilities();
    sptr<NetLinkInfo> link = new (std::nothrow) NetManagerStandard::NetLinkInfo();
    auto upstreamNetInfo = std::make_shared<UpstreamNetworkInfo>(handle, netcap, link);
    networkShareSubStateMachine->upstreamIfaceName_ = "test0";
    upstreamNetInfo->netLinkPro_->ifaceName_ = "test1";
    EXPECT_TRUE(networkShareSubStateMachine->HasChangeUpstreamIfaceSet(upstreamNetInfo->netLinkPro_->ifaceName_));
    networkShareSubStateMachine->HandleConnectionChanged(upstreamNetInfo);
}

/**
 * @tc.name: AddRoutesToLocalNetwork001
 * @tc.desc: Test NetworkShareSubStateMachine AddRoutesToLocalNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareSubStateMachineTest, AddRoutesToLocalNetwork001, TestSize.Level1)
{
    std::shared_ptr<NetworkShareConfiguration> configuration = nullptr;
    auto networkShareWifiSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    ASSERT_NE(networkShareWifiSubStateMachine, nullptr);
    networkShareWifiSubStateMachine->AddRoutesToLocalNetwork();
    auto networkShareBtSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_BLUETOOTH, configuration);
    ASSERT_NE(networkShareBtSubStateMachine, nullptr);
    networkShareBtSubStateMachine->AddRoutesToLocalNetwork();
    auto networkShareUsbSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(USB_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_USB, configuration);
    ASSERT_NE(networkShareUsbSubStateMachine, nullptr);
    networkShareUsbSubStateMachine->AddRoutesToLocalNetwork();
}

/**
 * @tc.name: AddRoutesToLocalNetwork002
 * @tc.desc: Test NetworkShareSubStateMachine AddRoutesToLocalNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareSubStateMachineTest, AddRoutesToLocalNetwork002, TestSize.Level1)
{
    auto configurationWifi = std::make_shared<NetworkShareConfiguration>();
    auto networkShareWifiSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configurationWifi);
    ASSERT_NE(networkShareWifiSubStateMachine, nullptr);
    networkShareWifiSubStateMachine->AddRoutesToLocalNetwork();
    auto configurationBt = std::make_shared<NetworkShareConfiguration>();
    auto networkShareBtSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_BLUETOOTH, configurationBt);
    ASSERT_NE(networkShareBtSubStateMachine, nullptr);
    networkShareBtSubStateMachine->AddRoutesToLocalNetwork();
    auto configurationUsb = std::make_shared<NetworkShareConfiguration>();
    auto networkShareUsbSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(USB_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_USB, configurationUsb);
    ASSERT_NE(networkShareUsbSubStateMachine, nullptr);
    networkShareUsbSubStateMachine->AddRoutesToLocalNetwork();
}

/**
 * @tc.name: NetworkShareSubStateMachineBranch001
 * @tc.desc: Test NetworkShareSubStateMachine Branch.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareSubStateMachineTest, NetworkShareSubStateMachineBranch001, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareWifiSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    ASSERT_NE(networkShareWifiSubStateMachine, nullptr);

    networkShareWifiSubStateMachine->HandleConnection();

    std::any messageObj = "";
    auto ret = networkShareWifiSubStateMachine->HandleInitInterfaceDown(messageObj);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    ret = networkShareWifiSubStateMachine->HandleSharedErrors(messageObj);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    networkShareWifiSubStateMachine->UnavailableStateEnter();

    bool result = networkShareWifiSubStateMachine->StopDhcp();
    ASSERT_TRUE(result);

    std::shared_ptr<INetAddr> netAddr = nullptr;
    result = networkShareWifiSubStateMachine->StartDhcp(netAddr);
    ASSERT_FALSE(result);

    result = networkShareWifiSubStateMachine->StopDhcp();
    ASSERT_TRUE(result);
}

/**
 * @tc.name: NetworkShareSubStateMachineBranch002
 * @tc.desc: Test NetworkShareSubStateMachine Branch.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareSubStateMachineTest, NetworkShareSubStateMachineBranch002, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareWifiSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    ASSERT_NE(networkShareWifiSubStateMachine, nullptr);

    networkShareWifiSubStateMachine->configuration_ = nullptr;
    std::string testName = "";
    auto result = networkShareWifiSubStateMachine->GetWifiApDestinationAddr(testName);
    ASSERT_FALSE(result);

    result = networkShareWifiSubStateMachine->GetWifiHotspotDhcpFlag();
    ASSERT_FALSE(result);

    result = networkShareWifiSubStateMachine->GetBtDestinationAddr(testName);
    ASSERT_FALSE(result);

    result = networkShareWifiSubStateMachine->GetUsbDestinationAddr(testName);
    ASSERT_FALSE(result);

    result = networkShareWifiSubStateMachine->CheckConfig(testName, testName);
    ASSERT_FALSE(result);

    std::shared_ptr<INetAddr> netAddr = nullptr;
    result = networkShareWifiSubStateMachine->RequestIpv4Address(netAddr);
    ASSERT_FALSE(result);
}

HWTEST_F(NetworkShareSubStateMachineTest, GetShareIpv6Prefix001, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    std::string iface = "123";
    EXPECT_TRUE(networkShareSubStateMachine->GetShareIpv6Prefix(iface));
}
 
HWTEST_F(NetworkShareSubStateMachineTest, GetShareIpv6Prefix002, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    std::string iface;
    EXPECT_TRUE(networkShareSubStateMachine->GetShareIpv6Prefix(iface));
}
 
HWTEST_F(NetworkShareSubStateMachineTest, GetShareIpv6Prefix003, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    std::string iface = "-1";
    EXPECT_TRUE(networkShareSubStateMachine->GetShareIpv6Prefix(iface));
}
 
HWTEST_F(NetworkShareSubStateMachineTest, MacToEui64Addr001, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    std::string mac = "123";
    EXPECT_NE(networkShareSubStateMachine->MacToEui64Addr(mac), "");
}
 
HWTEST_F(NetworkShareSubStateMachineTest, MacToEui64Addr002, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    std::string mac;
    EXPECT_NE(networkShareSubStateMachine->MacToEui64Addr(mac), "");
}
 
HWTEST_F(NetworkShareSubStateMachineTest, StartIpv6001, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    networkShareSubStateMachine->StartIpv6();
    IpPrefix ipprefix;
    ipprefix.prefixesLength = 123;
    networkShareSubStateMachine->lastRaParams_.prefixes_.push_back(ipprefix);
    networkShareSubStateMachine->StartIpv6();
    networkShareSubStateMachine->raDaemon_ = std::make_shared<RouterAdvertisementDaemon>();
    networkShareSubStateMachine->StartIpv6();
}
 
HWTEST_F(NetworkShareSubStateMachineTest, StopIpv6001, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    networkShareSubStateMachine->raDaemon_ = std::make_shared<RouterAdvertisementDaemon>();
    networkShareSubStateMachine->StopIpv6();
}
 
HWTEST_F(NetworkShareSubStateMachineTest, ConfigureShareIpv4001, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    NetLinkInfo netlinkinfo;
    INetAddr inetaddr;
    inetaddr.family_ = 1;
    netlinkinfo.netAddrList_.push_back(inetaddr);
    auto upstreamLinkInfo = new NetLinkInfo(netlinkinfo);
    networkShareSubStateMachine->ConfigureShareIpv4(upstreamLinkInfo);
}
 
HWTEST_F(NetworkShareSubStateMachineTest, ConfigureShareIpv4002, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    NetLinkInfo netlinkinfo;
    INetAddr inetaddr;
    inetaddr.family_ = 2;
    netlinkinfo.netAddrList_.push_back(inetaddr);
    auto upstreamLinkInfo = new NetLinkInfo(netlinkinfo);
    networkShareSubStateMachine->ConfigureShareIpv4(upstreamLinkInfo);
}
 
HWTEST_F(NetworkShareSubStateMachineTest, ConfigureShareIpv6001, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    NetLinkInfo netlinkinfo;
    INetAddr inetaddr;
    inetaddr.family_ = 2;
    netlinkinfo.netAddrList_.push_back(inetaddr);
    auto upstreamLinkInfo = new NetLinkInfo(netlinkinfo);
    networkShareSubStateMachine->ConfigureShareIpv6(upstreamLinkInfo);
}
 
HWTEST_F(NetworkShareSubStateMachineTest, ConfigureShareIpv6002, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    NetLinkInfo netlinkinfo;
    INetAddr inetaddr;
    inetaddr.family_ = 1;
    netlinkinfo.netAddrList_.push_back(inetaddr);
    auto upstreamLinkInfo = new NetLinkInfo(netlinkinfo);
    networkShareSubStateMachine->ConfigureShareIpv6(upstreamLinkInfo);
}
 
HWTEST_F(NetworkShareSubStateMachineTest, AddIpv6AddrToLocalNetwork001, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    networkShareSubStateMachine->AddIpv6AddrToLocalNetwork();
    IpPrefix ipprefix;
    ipprefix.prefixesLength = 123;
    networkShareSubStateMachine->lastRaParams_.prefixes_.push_back(ipprefix);
    networkShareSubStateMachine->AddIpv6AddrToLocalNetwork();
    networkShareSubStateMachine->ifaceName_ = "wlan0";
    networkShareSubStateMachine->AddIpv6AddrToLocalNetwork();
}
 
HWTEST_F(NetworkShareSubStateMachineTest, AddIpv6InfoToLocalNetwork001, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    networkShareSubStateMachine->AddIpv6InfoToLocalNetwork();
    EXPECT_FALSE(networkShareSubStateMachine->GetWifiApDstIpv6Addr());
}
 
HWTEST_F(NetworkShareSubStateMachineTest, FindDestinationAddr001, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_NONE, configuration);
    std::string destination = "";
    EXPECT_FALSE(networkShareSubStateMachine->FindDestinationAddr(destination));
}
 
HWTEST_F(NetworkShareSubStateMachineTest, GetWifiHotspotDhcpFlag001, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    EXPECT_EQ(networkShareSubStateMachine->GetWifiHotspotDhcpFlag(), false);
}
 
HWTEST_F(NetworkShareSubStateMachineTest, GetBtDestinationAddr002, TestSize.Level1)
{
    std::string addrStr;
    NetworkShareConfiguration networkshareconfiguration;
    networkshareconfiguration.btPanIpv4Str_ = "";
    auto configuration = std::make_shared<NetworkShareConfiguration>(networkshareconfiguration);
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    EXPECT_EQ(networkShareSubStateMachine->GetBtDestinationAddr(addrStr), false);
}
 
HWTEST_F(NetworkShareSubStateMachineTest, GetBtDestinationAddr003, TestSize.Level1)
{
    std::string addrStr;
    NetworkShareConfiguration networkshareconfiguration;
    networkshareconfiguration.routeSuffix_ = "";
    auto configuration = std::make_shared<NetworkShareConfiguration>(networkshareconfiguration);
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    EXPECT_EQ(networkShareSubStateMachine->GetBtDestinationAddr(addrStr), false);
}
 
HWTEST_F(NetworkShareSubStateMachineTest, GetWifiApDestinationAddr002, TestSize.Level1)
{
    std::string addrStr;
    NetworkShareConfiguration networkshareconfiguration;
    networkshareconfiguration.wifiIpv4Str_ = "";
    auto configuration = std::make_shared<NetworkShareConfiguration>(networkshareconfiguration);
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    EXPECT_EQ(networkShareSubStateMachine->GetWifiApDestinationAddr(addrStr), false);
}
 
HWTEST_F(NetworkShareSubStateMachineTest, GetWifiApDestinationAddr003, TestSize.Level1)
{
    std::string addrStr;
    NetworkShareConfiguration networkshareconfiguration;
    networkshareconfiguration.routeSuffix_ = "";
    auto configuration = std::make_shared<NetworkShareConfiguration>(networkshareconfiguration);
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    EXPECT_EQ(networkShareSubStateMachine->GetWifiApDestinationAddr(addrStr), false);
}
 
HWTEST_F(NetworkShareSubStateMachineTest, GetUsbDestinationAddr002, TestSize.Level1)
{
    std::string addrStr;
    NetworkShareConfiguration networkshareconfiguration;
    networkshareconfiguration.usbIpv4Str_ = "";
    auto configuration = std::make_shared<NetworkShareConfiguration>(networkshareconfiguration);
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    EXPECT_EQ(networkShareSubStateMachine->GetUsbDestinationAddr(addrStr), false);
}
 
HWTEST_F(NetworkShareSubStateMachineTest, GetUsbDestinationAddr003, TestSize.Level1)
{
    std::string addrStr;
    NetworkShareConfiguration networkshareconfiguration;
    networkshareconfiguration.routeSuffix_ = "";
    auto configuration = std::make_shared<NetworkShareConfiguration>(networkshareconfiguration);
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    EXPECT_EQ(networkShareSubStateMachine->GetUsbDestinationAddr(addrStr), false);
}
 
HWTEST_F(NetworkShareSubStateMachineTest, StopDhcp001, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    EXPECT_NE(configuration, nullptr);
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(BLUETOOTH_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_USB, configuration);
    networkShareSubStateMachine->ifaceName_ = "123";
}
 
HWTEST_F(NetworkShareSubStateMachineTest, ConfigureShareDhcp001, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(USB_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    bool ret = networkShareSubStateMachine->ConfigureShareDhcp(true);
    EXPECT_EQ(ret, true);
}
 
HWTEST_F(NetworkShareSubStateMachineTest, ConfigureShareDhcp002, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(USB_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_NONE, configuration);
    bool ret = networkShareSubStateMachine->ConfigureShareDhcp(true);
    EXPECT_EQ(ret, false);
}
 
HWTEST_F(NetworkShareSubStateMachineTest, ConfigureShareDhcp003, TestSize.Level1)
{
    auto configuration = nullptr;
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(USB_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    bool ret = networkShareSubStateMachine->ConfigureShareDhcp(true);
    EXPECT_EQ(ret, false);
}
 
HWTEST_F(NetworkShareSubStateMachineTest, ConfigureShareDhcp004, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    EXPECT_NE(configuration, nullptr);
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(USB_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_USB, configuration);
    networkShareSubStateMachine->ConfigureShareDhcp(true);
}
 
HWTEST_F(NetworkShareSubStateMachineTest, RequestIpv4Address002, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(USB_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    std::shared_ptr<INetAddr> ipv4Address = nullptr;
    bool ret = networkShareSubStateMachine->RequestIpv4Address(ipv4Address);
    EXPECT_EQ(ret, true);
}
 
HWTEST_F(NetworkShareSubStateMachineTest, RequestIpv4Address003, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(USB_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_NONE, configuration);
    std::shared_ptr<INetAddr> ipv4Address = nullptr;
    bool ret = networkShareSubStateMachine->RequestIpv4Address(ipv4Address);
    EXPECT_NE(ret, true);
}

/**
 * @tc.number: NetworkShareSubStateMachine_HandleConnection_IpfwdAddInterfaceForwardFail
 * @tc.name: Test HandleConnection when IpfwdAddInterfaceForward returns failure
 * @tc.desc: Verify that HandleConnection switches to INIT state when IpfwdAddInterfaceForward fails
 */
HWTEST_F(NetworkShareSubStateMachineTest, HandleConnection_IpfwdAddInterfaceForwardFail, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    ASSERT_NE(networkShareSubStateMachine, nullptr);
    networkShareSubStateMachine->upstreamIfaceName_ = "test_upstream";
    networkShareSubStateMachine->curState_ = SUBSTATE_SHARED;
    networkShareSubStateMachine->HandleConnection();
    EXPECT_GE(networkShareSubStateMachine->curState_, SUBSTATE_INIT);
}

/**
 * @tc.number: NetworkShareSubStateMachine_HandleConnection_NetworkAddInterfaceFail
 * @tc.name: Test HandleConnection when NetworkAddInterface returns failure
 * @tc.desc: Verify that HandleConnection switches to INIT state when NetworkAddInterface fails
 */
HWTEST_F(NetworkShareSubStateMachineTest, HandleConnection_NetworkAddInterfaceFail, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    ASSERT_NE(networkShareSubStateMachine, nullptr);
    networkShareSubStateMachine->upstreamIfaceName_ = "test_upstream";
    networkShareSubStateMachine->curState_ = SUBSTATE_SHARED;
    networkShareSubStateMachine->HandleConnection();
    EXPECT_GE(networkShareSubStateMachine->curState_, SUBSTATE_INIT);
}

/**
 * @tc.number: NetworkShareSubStateMachine_HandleConnection_Success
 * @tc.name: Test HandleConnection when both IpfwdAddInterfaceForward and NetworkAddInterface succeed
 * @tc.desc: Verify that HandleConnection maintains SHARED state when both operations succeed
 */
HWTEST_F(NetworkShareSubStateMachineTest, HandleConnection_Success, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    ASSERT_NE(networkShareSubStateMachine, nullptr);
    networkShareSubStateMachine->upstreamIfaceName_ = "test_upstream";
    networkShareSubStateMachine->curState_ = SUBSTATE_SHARED;
    networkShareSubStateMachine->HandleConnection();
}

/**
 * @tc.number: NetworkShareSubStateMachine_CleanupUpstreamInterface
 * @tc.name: Test CleanupUpstreamInterface
 * @tc.desc: Verify that CleanupUpstreamInterface removes routes and interface forward
 */
HWTEST_F(NetworkShareSubStateMachineTest, CleanupUpstreamInterface, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    ASSERT_NE(networkShareSubStateMachine, nullptr);
    networkShareSubStateMachine->upstreamIfaceName_ = "test_upstream";
    networkShareSubStateMachine->CleanupUpstreamInterface();
    EXPECT_NE(networkShareSubStateMachine->upstreamIfaceName_, "TEST");
}

/**
 * @tc.number: NetworkShareSubStateMachine_HandleConnection_Tunv4Exists_AddSuccess
 * @tc.name: Test HandleConnection when tunv4 interface exists and IpfwdAddInterfaceForward succeeds
 * @tc.desc: Verify that HandleConnection handles tunv4 interface addition successfully
 */
HWTEST_F(NetworkShareSubStateMachineTest, HandleConnection_Tunv4Exists_AddSuccess, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    ASSERT_NE(networkShareSubStateMachine, nullptr);
    networkShareSubStateMachine->upstreamIfaceName_ = "rmnet0";
    networkShareSubStateMachine->curState_ = SUBSTATE_SHARED;
    networkShareSubStateMachine->HandleConnection();
}

/**
 * @tc.number: NetworkShareSubStateMachine_HandleConnection_Tunv4NotExists
 * @tc.name: Test HandleConnection when tunv4 interface does not exist (tunv4IfIndex == 0)
 * @tc.desc: Verify that HandleConnection skips tunv4 handling when interface doesn't exist
 */
HWTEST_F(NetworkShareSubStateMachineTest, HandleConnection_Tunv4NotExists, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    ASSERT_NE(networkShareSubStateMachine, nullptr);
    networkShareSubStateMachine->upstreamIfaceName_ = "nonexistent_iface";
    networkShareSubStateMachine->curState_ = SUBSTATE_SHARED;
    networkShareSubStateMachine->HandleConnection();
}

/**
 * @tc.number: NetworkShareSubStateMachine_HandleConnection_Tunv4AddFail_UpstreamSuccess
 * @tc.name: Test HandleConnection when tunv4 add fails but upstream add succeeds
 * @tc.desc: Verify that HandleConnection continues when tunv4 fails but upstream succeeds
 */
HWTEST_F(NetworkShareSubStateMachineTest, HandleConnection_Tunv4AddFail_UpstreamSuccess, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    ASSERT_NE(networkShareSubStateMachine, nullptr);
    networkShareSubStateMachine->upstreamIfaceName_ = "test_upstream";
    networkShareSubStateMachine->curState_ = SUBSTATE_SHARED;
    networkShareSubStateMachine->HandleConnection();
}

/**
 * @tc.number: NetworkShareSubStateMachine_HandleConnection_UpstreamEmpty
 * @tc.name: Test HandleConnection when upstreamIfaceName is empty
 * @tc.desc: Verify that HandleConnection handles empty upstream interface name
 */
HWTEST_F(NetworkShareSubStateMachineTest, HandleConnection_UpstreamEmpty, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    ASSERT_NE(networkShareSubStateMachine, nullptr);
    networkShareSubStateMachine->upstreamIfaceName_ = "";
    networkShareSubStateMachine->curState_ = SUBSTATE_SHARED;
    networkShareSubStateMachine->HandleConnection();
}

/**
 * @tc.number: NetworkShareSubStateMachine_HandleConnection_Tunv4PrefixGeneration
 * @tc.name: Test HandleConnection generates correct tunv4 interface name
 * @tc.desc: Verify that tunv4UpstreamIfaceName_ is correctly generated as "tunv4-" + upstreamIfaceName_
 */
HWTEST_F(NetworkShareSubStateMachineTest, HandleConnection_Tunv4PrefixGeneration, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    ASSERT_NE(networkShareSubStateMachine, nullptr);
    networkShareSubStateMachine->upstreamIfaceName_ = "rmnet_data0";
    networkShareSubStateMachine->curState_ = SUBSTATE_SHARED;
    networkShareSubStateMachine->HandleConnection();
    EXPECT_EQ(networkShareSubStateMachine->tunv4UpstreamIfaceName_, "tunv4-rmnet_data0");
}

/**
 * @tc.number: NetworkShareSubStateMachine_HandleConnection_ErrorStateSwitch
 * @tc.name: Test HandleConnection switches to INIT state on error
 * @tc.desc: Verify that HandleConnection correctly switches state when error occurs
 */
HWTEST_F(NetworkShareSubStateMachineTest, HandleConnection_ErrorStateSwitch, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    ASSERT_NE(networkShareSubStateMachine, nullptr);
    networkShareSubStateMachine->upstreamIfaceName_ = "error_test";
    networkShareSubStateMachine->curState_ = SUBSTATE_SHARED;
    networkShareSubStateMachine->HandleConnection();
    EXPECT_GE(networkShareSubStateMachine->curState_, SUBSTATE_INIT);
}

/**
 * @tc.number: NetworkShareSubStateMachine_HandleConnection_LastErrorSet
 * @tc.name: Test HandleConnection sets lastError_ on failure
 * @tc.desc: Verify that lastError_ is set to NETWORKSHARE_ERROR_ENABLE_FORWARDING_ERROR on failure
 */
HWTEST_F(NetworkShareSubStateMachineTest, HandleConnection_LastErrorSet, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    ASSERT_NE(networkShareSubStateMachine, nullptr);
    networkShareSubStateMachine->upstreamIfaceName_ = "error_test";
    networkShareSubStateMachine->curState_ = SUBSTATE_SHARED;
    networkShareSubStateMachine->HandleConnection();
    EXPECT_LE(networkShareSubStateMachine->lastError_, NETWORKSHARE_ERROR_ENABLE_FORWARDING_ERROR);
}

/**
 * @tc.number: NetworkShareSubStateMachine_HandleConnection_Tunv4CleanupOnUpstreamFail
 * @tc.name: Test HandleConnection cleans up tunv4 interface when upstream fails
 * @tc.desc: Verify that tunv4 interface is removed when upstream IpfwdAddInterfaceForward fails
 */
HWTEST_F(NetworkShareSubStateMachineTest, HandleConnection_Tunv4CleanupOnUpstreamFail, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    ASSERT_NE(networkShareSubStateMachine, nullptr);
    networkShareSubStateMachine->upstreamIfaceName_ = "test_fail";
    networkShareSubStateMachine->curState_ = SUBSTATE_SHARED;
    networkShareSubStateMachine->HandleConnection();
    EXPECT_NE(networkShareSubStateMachine->tunv4UpstreamIfaceName_, "TEST");
}

/**
 * @tc.number: NetworkShareSubStateMachine_CleanupUpstreamInterface_Tunv4NotEmpty
 * @tc.name: Test CleanupUpstreamInterface when tunv4UpstreamIfaceName_ is not empty
 * @tc.desc: removes tunv4 interface forward when tunv4UpstreamIfaceName_ is not empty
 */
HWTEST_F(NetworkShareSubStateMachineTest, CleanupUpstreamInterface_Tunv4NotEmpty, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    ASSERT_NE(networkShareSubStateMachine, nullptr);
    networkShareSubStateMachine->upstreamIfaceName_ = "rmnet0";
    networkShareSubStateMachine->tunv4UpstreamIfaceName_ = "tunv4-rmnet0";
    networkShareSubStateMachine->CleanupUpstreamInterface();
    EXPECT_EQ(networkShareSubStateMachine->tunv4UpstreamIfaceName_, "");
}

/**
 * @tc.number: NetworkShareSubStateMachine_CleanupUpstreamInterface_Tunv4Empty
 * @tc.name: Test CleanupUpstreamInterface when tunv4UpstreamIfaceName_ is empty
 * @tc.desc: Verify that CleanupUpstreamInterface skips tunv4 cleanup when tunv4UpstreamIfaceName_ is empty
 */
HWTEST_F(NetworkShareSubStateMachineTest, CleanupUpstreamInterface_Tunv4Empty, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    ASSERT_NE(networkShareSubStateMachine, nullptr);
    networkShareSubStateMachine->upstreamIfaceName_ = "rmnet0";
    networkShareSubStateMachine->tunv4UpstreamIfaceName_ = "";
    networkShareSubStateMachine->CleanupUpstreamInterface();
    EXPECT_EQ(networkShareSubStateMachine->tunv4UpstreamIfaceName_, "");
}

/**
 * @tc.number: NetworkShareSubStateMachine_CleanupUpstreamInterface_UpstreamEmpty
 * @tc.name: Test CleanupUpstreamInterface when upstreamIfaceName_ is empty
 * @tc.desc: Verify that CleanupUpstreamInterface handles empty upstream interface name
 */
HWTEST_F(NetworkShareSubStateMachineTest, CleanupUpstreamInterface_UpstreamEmpty, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    ASSERT_NE(networkShareSubStateMachine, nullptr);
    networkShareSubStateMachine->upstreamIfaceName_ = "";
    networkShareSubStateMachine->tunv4UpstreamIfaceName_ = "";
    networkShareSubStateMachine->CleanupUpstreamInterface();
    EXPECT_EQ(networkShareSubStateMachine->upstreamIfaceName_, "");
}

/**
 * @tc.number: NetworkShareSubStateMachine_CleanupUpstreamInterface_BothIfacesSet
 * @tc.name: Test CleanupUpstreamInterface when both upstream and tunv4 interfaces are set
 * @tc.desc: Verify that CleanupUpstreamInterface removes both upstream and tunv4 interface forwards
 */
HWTEST_F(NetworkShareSubStateMachineTest, CleanupUpstreamInterface_BothIfacesSet, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    ASSERT_NE(networkShareSubStateMachine, nullptr);
    networkShareSubStateMachine->upstreamIfaceName_ = "usb0";
    networkShareSubStateMachine->tunv4UpstreamIfaceName_ = "tunv4-usb0";
    networkShareSubStateMachine->CleanupUpstreamInterface();
    EXPECT_EQ(networkShareSubStateMachine->tunv4UpstreamIfaceName_, "");
    EXPECT_EQ(networkShareSubStateMachine->upstreamIfaceName_, "usb0");
}

/**
 * @tc.number: NetworkShareSubStateMachine_CleanupUpstreamInterface_RoutesRemoved
 * @tc.name: Test CleanupUpstreamInterface removes routes to local network
 * @tc.desc: Verify that CleanupUpstreamInterface calls RemoveRoutesToLocalNetwork
 */
HWTEST_F(NetworkShareSubStateMachineTest, CleanupUpstreamInterface_RoutesRemoved, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    ASSERT_NE(networkShareSubStateMachine, nullptr);
    networkShareSubStateMachine->upstreamIfaceName_ = "wlan0";
    networkShareSubStateMachine->tunv4UpstreamIfaceName_ = "";
    networkShareSubStateMachine->CleanupUpstreamInterface();
}

/**
 * @tc.number: NetworkShareSubStateMachine_CleanupUpstreamInterface_NetworkRemoveInterface
 * @tc.name: Test CleanupUpstreamInterface calls NetworkRemoveInterface
 * @tc.desc: Verify that CleanupUpstreamInterface calls NetworkRemoveInterface to remove interface from network
 */
HWTEST_F(NetworkShareSubStateMachineTest, CleanupUpstreamInterface_NetworkRemoveInterface, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(USB_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_USB, configuration);
    ASSERT_NE(networkShareSubStateMachine, nullptr);
    networkShareSubStateMachine->upstreamIfaceName_ = "rmnet0";
    networkShareSubStateMachine->tunv4UpstreamIfaceName_ = "tunv4-rmnet0";
    networkShareSubStateMachine->CleanupUpstreamInterface();
    EXPECT_EQ(networkShareSubStateMachine->tunv4UpstreamIfaceName_, "");
}

/**
 * @tc.number: NetworkShareSubStateMachine_HandleConnection_Tunv4InterfaceExists
 * @tc.name: Test HandleConnection when tunv4 interface exists (tunv4IfIndex != 0)
 * @tc.desc: Verify that HandleConnection calls IpfwdAddInterfaceForward for tunv4 interface when it exists
 * This test creates a virtual tunv4 interface to cover the branch: if (tunv4IfIndex != 0)
 */
HWTEST_F(NetworkShareSubStateMachineTest, HandleConnection_Tunv4InterfaceExists, TestSize.Level1)
{
    // Create a virtual tunv4 interface for testing
    // The interface name format is "tunv4-" + upstream interface name
    const std::string testUpstreamIface = "rmnet0";
    const std::string tunv4IfaceName = "tunv4-" + testUpstreamIface;

    // Create a dummy tunv4 interface using ip command
    // We use a dummy interface and rename it to simulate tunv4
    system("ip link add dummy0 type dummy");
    system(("ip link set dummy0 name " + tunv4IfaceName).c_str());
    system(("ip link set " + tunv4IfaceName + " up").c_str());

    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    ASSERT_NE(networkShareSubStateMachine, nullptr);

    networkShareSubStateMachine->upstreamIfaceName_ = testUpstreamIface;
    networkShareSubStateMachine->curState_ = SUBSTATE_SHARED;

    // Call HandleConnection - this should now take the branch where tunv4IfIndex != 0
    networkShareSubStateMachine->HandleConnection();

    // Verify that tunv4UpstreamIfaceName_ is correctly set
    EXPECT_EQ(networkShareSubStateMachine->tunv4UpstreamIfaceName_, tunv4IfaceName);

    // Clean up the virtual interface
    system(("ip link del " + tunv4IfaceName).c_str());
}

/**
 * @tc.number: NetworkShareSubStateMachine_HandleConnection_Tunv4AddInterfaceForwardFail
 * @tc.name: Test HandleConnection when tunv4 interface exists but IpfwdAddInterfaceForward fails
 * @tc.desc: Verify that HandleConnection handles failure when adding tunv4 interface forward fails
 */
HWTEST_F(NetworkShareSubStateMachineTest, HandleConnection_Tunv4AddInterfaceForwardFail, TestSize.Level1)
{
    const std::string testUpstreamIface = "rmnet0";
    const std::string tunv4IfaceName = "tunv4-" + testUpstreamIface;

    // Create a virtual tunv4 interface
    system("ip link add dummy0 type dummy");
    system(("ip link set dummy0 name " + tunv4IfaceName).c_str());
    system(("ip link set " + tunv4IfaceName + " up").c_str());

    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    ASSERT_NE(networkShareSubStateMachine, nullptr);

    networkShareSubStateMachine->upstreamIfaceName_ = testUpstreamIface;
    networkShareSubStateMachine->curState_ = SUBSTATE_SHARED;

    // Call HandleConnection - this should take the branch where tunv4IfIndex != 0
    // and call IpfwdAddInterfaceForward for tunv4
    networkShareSubStateMachine->HandleConnection();

    // Clean up the virtual interface
    system(("ip link del " + tunv4IfaceName).c_str());
}
} // namespace NetManagerStandard
} // namespace OHOS