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

constexpr const char *WIFI_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=sharing_wifi";
constexpr const char *USB_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=sharing_usb";
constexpr const char *BLUETOOTH_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=sharing_bluetooth";

constexpr const char *KEY_WIFI = "settings.netmanager.sharing_wifi";
constexpr const char *KEY_USB = "settings.netmanager.sharing_usb";
constexpr const char *KEY_BLUETOOTH = "settings.netmanager.sharing_bluetooth";

std::shared_ptr<NetDataShareHelperUtils> netDataShareHelperUtils_ = nullptr;

class DataShareHelperUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void DataShareHelperUtilsTest::SetUpTestCase()
{
    netDataShareHelperUtils_ = std::make_unique<NetDataShareHelperUtils>();
}

void DataShareHelperUtilsTest::TearDownTestCase()
{
    netDataShareHelperUtils_.reset();
}

void DataShareHelperUtilsTest::SetUp() {}

void DataShareHelperUtilsTest::TearDown() {}

HWTEST_F(DataShareHelperUtilsTest, QueryTest001, TestSize.Level1)
{
    std::string status;
    Uri wifiUri(WIFI_URI);
    int32_t ret = netDataShareHelperUtils_->Query(wifiUri, KEY_WIFI, status);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(DataShareHelperUtilsTest, QueryTest002, TestSize.Level1)
{
    std::string status;
    Uri bluetoothUri(USB_URI);
    int ret = netDataShareHelperUtils_->Query(bluetoothUri, KEY_BLUETOOTH, status);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(DataShareHelperUtilsTest, QueryTest003, TestSize.Level1)
{
    std::string status;
    Uri usbUri(BLUETOOTH_URI);
    int ret = netDataShareHelperUtils_->Query(usbUri, KEY_USB, status);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(DataShareHelperUtilsTest, InsertTest001, TestSize.Level1)
{
    std::string on = "1";
    Uri wifiUri(WIFI_URI);
    int32_t ret = netDataShareHelperUtils_->Insert(wifiUri, KEY_WIFI, on);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(DataShareHelperUtilsTest, InsertTest002, TestSize.Level1)
{
    std::string on = "1";
    Uri wifiUri(WIFI_URI);
    int32_t ret = netDataShareHelperUtils_->Insert(wifiUri, KEY_WIFI, on);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(DataShareHelperUtilsTest, InsertTest003, TestSize.Level1)
{
    std::string off = "0";
    Uri usbUri(USB_URI);
    int ret = netDataShareHelperUtils_->Insert(usbUri, KEY_WIFI, off);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(DataShareHelperUtilsTest, InsertTest004, TestSize.Level1)
{
    std::string on = "1";
    Uri usbUri(USB_URI);
    int32_t ret = netDataShareHelperUtils_->Insert(usbUri, KEY_WIFI, on);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(DataShareHelperUtilsTest, InsertTest005, TestSize.Level1)
{
    std::string off = "0";
    Uri blueToothUri(BLUETOOTH_URI);
    int ret = netDataShareHelperUtils_->Insert(blueToothUri, KEY_WIFI, off);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(DataShareHelperUtilsTest, InsertTest006, TestSize.Level1)
{
    std::string on = "1";
    Uri blueToothUri(BLUETOOTH_URI);
    int32_t ret = netDataShareHelperUtils_->Insert(blueToothUri, KEY_WIFI, on);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(DataShareHelperUtilsTest, UpdateTest001, TestSize.Level1)
{
    std::string on = "1";
    Uri wifiUri(WIFI_URI);
    int32_t ret = netDataShareHelperUtils_->Update(wifiUri, KEY_WIFI, on);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(DataShareHelperUtilsTest, UpdateTest002, TestSize.Level1)
{
    std::string off = "0";
    Uri wifiUri(WIFI_URI);
    int ret = netDataShareHelperUtils_->Update(wifiUri, KEY_WIFI, off);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(DataShareHelperUtilsTest, UpdateTest003, TestSize.Level1)
{
    std::string on = "1";
    Uri usbUri(USB_URI);
    int ret = netDataShareHelperUtils_->Update(usbUri, KEY_USB, on);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(DataShareHelperUtilsTest, UpdateTest004, TestSize.Level1)
{
    std::string off = "0";
    Uri usbUri(USB_URI);
    int ret = netDataShareHelperUtils_->Update(usbUri, KEY_USB, off);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(DataShareHelperUtilsTest, UpdateTest005, TestSize.Level1)
{
    std::string on = "1";
    Uri bluetooth(BLUETOOTH_URI);
    int ret = netDataShareHelperUtils_->Update(bluetooth, KEY_USB, on);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(DataShareHelperUtilsTest, UpdateTest006, TestSize.Level1)
{
    std::string off = "0";
    Uri bluetooth(BLUETOOTH_URI);
    int ret = netDataShareHelperUtils_->Update(bluetooth, KEY_USB, off);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

} // namespace NetManagerStandard
} // namespace OHOS
