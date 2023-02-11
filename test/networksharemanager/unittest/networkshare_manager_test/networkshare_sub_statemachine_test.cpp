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

#include "networkshare_sub_statemachine.h"

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
 * @tc.name: GetNetShareType03
 * @tc.desc: Test NetworkShareSubStateMachine GetNetShareType.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareSubStateMachineTest, GetNetShareType03, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(USB_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_USB, configuration);
    SharingIfaceType netShareType = networkShareSubStateMachine->GetNetShareType();
    EXPECT_EQ(netShareType, SharingIfaceType::SHARING_USB);
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
 * @tc.name: SubSmEventHandle01
 * @tc.desc: Test NetworkShareSubStateMachine SubSmEventHandle.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareSubStateMachineTest, SubSmEventHandle01, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(WIFI_AP_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_WIFI, configuration);
    int eventId = CMD_NETSHARE_REQUESTED;
    std::any messageObj = "";
    networkShareSubStateMachine->SubSmEventHandle(eventId, messageObj);
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
 * @tc.name: GetDownIfaceName03
 * @tc.desc: Test NetworkShareSubStateMachine GetDownIfaceName.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkShareSubStateMachineTest, GetDownIfaceName03, TestSize.Level1)
{
    auto configuration = std::make_shared<NetworkShareConfiguration>();
    auto networkShareSubStateMachine = new (std::nothrow)
        NetworkShareSubStateMachine(USB_DEFAULT_IFACE_NAME, SharingIfaceType::SHARING_USB, configuration);
    std::string downIface;
    networkShareSubStateMachine->GetDownIfaceName(downIface);
    EXPECT_EQ(downIface, USB_DEFAULT_IFACE_NAME);
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
} // namespace NetManagerStandard
} // namespace OHOS