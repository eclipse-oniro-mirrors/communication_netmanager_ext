/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "net_datashare_utils.h"
#include "net_manager_constants.h"
#include "netmanager_base_test_security.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;

constexpr const char *SHARING_WIFI_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=sharing_wifi";
constexpr const char *SHARING_USB_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=sharing_usb";
constexpr const char *SHARING_BLUETOOTH_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=sharing_bluetooth";

constexpr const char *KEY_SHARING_WIFI = "settings.netmanager.sharing_wifi";
constexpr const char *KEY_SHARING_USB = "settings.netmanager.sharing_usb";
constexpr const char *KEY_SHARING_BLUETOOTH = "settings.netmanager.sharing_bluetooth";

std::unique_ptr<NetDataShareHelperUtils> netDataShareHelperUtils_ = nullptr;

class NetDataShareHelperUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetDataShareHelperUtilsTest::SetUpTestCase()
{
    netDataShareHelperUtils_ = std::make_unique<NetDataShareHelperUtils>();
}

void NetDataShareHelperUtilsTest::TearDownTestCase()
{
    netDataShareHelperUtils_.reset();
}

void NetDataShareHelperUtilsTest::SetUp() {}

void NetDataShareHelperUtilsTest::TearDown() {}

/**
 * @tc.name: InsertTest001
 * @tc.desc: Test NetDataShareHelperUtils::Insert
 * @tc.type: FUNC
 */
HWTEST_F(NetDataShareHelperUtilsTest, InsertTest001, TestSize.Level1)
{
    std::string on = "1";
    Uri wifiUri(SHARING_WIFI_URI);
    int32_t ret = netDataShareHelperUtils_->Insert(wifiUri, KEY_SHARING_WIFI, on);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    std::string off = "0";
    ret = netDataShareHelperUtils_->Insert(wifiUri, KEY_SHARING_WIFI, off);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    Uri usbUri(SHARING_USB_URI);
    ret = netDataShareHelperUtils_->Insert(usbUri, KEY_SHARING_USB, on);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = netDataShareHelperUtils_->Insert(usbUri, KEY_SHARING_USB, off);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    Uri bluetoothUri(SHARING_BLUETOOTH_URI);
    ret = netDataShareHelperUtils_->Insert(bluetoothUri, KEY_SHARING_BLUETOOTH, on);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = netDataShareHelperUtils_->Insert(bluetoothUri, KEY_SHARING_BLUETOOTH, off);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: InsertTest002
 * @tc.desc: Test NetDataShareHelperUtils::Insert
 * @tc.type: FUNC
 */
HWTEST_F(NetDataShareHelperUtilsTest, InsertTest002, TestSize.Level1)
{
    NetManagerBaseDataShareToken token;

    std::string on = "1";
    Uri wifiUri(SHARING_WIFI_URI);
    int32_t ret = netDataShareHelperUtils_->Insert(wifiUri, KEY_SHARING_WIFI, on);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    std::string off = "0";
    ret = netDataShareHelperUtils_->Insert(wifiUri, KEY_SHARING_WIFI, off);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    Uri usbUri(SHARING_USB_URI);
    ret = netDataShareHelperUtils_->Insert(usbUri, KEY_SHARING_USB, on);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = netDataShareHelperUtils_->Insert(usbUri, KEY_SHARING_USB, off);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    Uri bluetoothUri(SHARING_BLUETOOTH_URI);
    ret = netDataShareHelperUtils_->Insert(bluetoothUri, KEY_SHARING_BLUETOOTH, on);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = netDataShareHelperUtils_->Insert(bluetoothUri, KEY_SHARING_BLUETOOTH, off);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: UpdateTest001
 * @tc.desc: Test NetDataShareHelperUtils::Update
 * @tc.type: FUNC
 */
HWTEST_F(NetDataShareHelperUtilsTest, UpdateTest001, TestSize.Level1)
{
    std::string on = "1";
    Uri wifiUri(SHARING_WIFI_URI);
    int32_t ret = netDataShareHelperUtils_->Update(wifiUri, KEY_SHARING_WIFI, on);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    std::string off = "0";
    ret = netDataShareHelperUtils_->Update(wifiUri, KEY_SHARING_WIFI, off);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    Uri usbUri(SHARING_USB_URI);
    ret = netDataShareHelperUtils_->Update(usbUri, KEY_SHARING_USB, on);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = netDataShareHelperUtils_->Update(usbUri, KEY_SHARING_USB, off);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    Uri bluetoothUri(SHARING_BLUETOOTH_URI);
    ret = netDataShareHelperUtils_->Update(bluetoothUri, KEY_SHARING_BLUETOOTH, on);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = netDataShareHelperUtils_->Update(bluetoothUri, KEY_SHARING_BLUETOOTH, off);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: UpdateTest002
 * @tc.desc: Test NetDataShareHelperUtils::Update
 * @tc.type: FUNC
 */
HWTEST_F(NetDataShareHelperUtilsTest, UpdateTest002, TestSize.Level1)
{
    NetManagerBaseDataShareToken token;
    std::string on = "1";
    Uri wifiUri(SHARING_WIFI_URI);
    int32_t ret = netDataShareHelperUtils_->Update(wifiUri, KEY_SHARING_WIFI, on);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    std::string off = "0";
    ret = netDataShareHelperUtils_->Update(wifiUri, KEY_SHARING_WIFI, off);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    Uri usbUri(SHARING_USB_URI);
    ret = netDataShareHelperUtils_->Update(usbUri, KEY_SHARING_USB, on);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = netDataShareHelperUtils_->Update(usbUri, KEY_SHARING_USB, off);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    Uri bluetoothUri(SHARING_BLUETOOTH_URI);
    ret = netDataShareHelperUtils_->Update(bluetoothUri, KEY_SHARING_BLUETOOTH, on);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = netDataShareHelperUtils_->Update(bluetoothUri, KEY_SHARING_BLUETOOTH, off);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: QueryTest001
 * @tc.desc: Test NetDataShareHelperUtils::Query
 * @tc.type: FUNC
 */
HWTEST_F(NetDataShareHelperUtilsTest, QueryTest001, TestSize.Level1)
{
    std::string status;
    Uri wifiUri(SHARING_WIFI_URI);
    int32_t ret = netDataShareHelperUtils_->Query(wifiUri, KEY_SHARING_WIFI, status);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    std::cout << "Wifi QueryTest result: " << status << std::endl;

    Uri usbUri(SHARING_USB_URI);
    ret = netDataShareHelperUtils_->Query(usbUri, KEY_SHARING_USB, status);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    std::cout << "Usb QueryTest result: " << status << std::endl;

    Uri bluetoothUri(SHARING_BLUETOOTH_URI);
    ret = netDataShareHelperUtils_->Query(bluetoothUri, KEY_SHARING_BLUETOOTH, status);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    std::cout << "Bluetooth QueryTest result: " << status << std::endl;
}

/**
 * @tc.name: QueryTest002
 * @tc.desc: Test NetDataShareHelperUtils::Query
 * @tc.type: FUNC
 */
HWTEST_F(NetDataShareHelperUtilsTest, QueryTest002, TestSize.Level1)
{
    NetManagerBaseDataShareToken token;

    std::string status;
    Uri wifiUri(SHARING_WIFI_URI);
    int32_t ret = netDataShareHelperUtils_->Query(wifiUri, KEY_SHARING_WIFI, status);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    std::cout << "Wifi QueryTest result: " << status << std::endl;

    Uri bluetoothUri(SHARING_USB_URI);
    ret = netDataShareHelperUtils_->Query(bluetoothUri, KEY_SHARING_BLUETOOTH, status);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    std::cout << "Bluetooth QueryTest result: " << status << std::endl;

    Uri usbUri(SHARING_BLUETOOTH_URI);
    ret = netDataShareHelperUtils_->Query(usbUri, KEY_SHARING_USB, status);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    std::cout << "Usb QueryTest result: " << status << std::endl;
}
} // namespace NetManagerStandard
} // namespace OHOS
