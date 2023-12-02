/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <memory>

#include "message_parcel.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "networkshare_constants.h"
#include "net_manager_constants.h"
#include "net_datashare_utils.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;

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

class NetDataShareAccessToken {
public:
    NetDataShareAccessToken();
    ~NetDataShareAccessToken();

private:
    Security::AccessToken::AccessTokenID currentID_ = 0;
    Security::AccessToken::AccessTokenID accessID_ = 0;
};

namespace {
HapInfoParams netDataShareInfo = {
    .userID = 100,
    .bundleName = "networkshare_manager_test",
    .instIndex = 0,
    .appIDDesc = "test",
    .isSystemApp = true,
};

PermissionDef testNetworkShareSettingsPermDef = {
    .permissionName = "ohos.permission.MANAGE_SECURE_SETTINGS",
    .bundleName = "networkshare_manager_test",
    .grantMode = 1,
    .label = "label",
    .labelId = 1,
    .description = "Test net data share",
    .descriptionId = 1,
    .availableLevel = APL_SYSTEM_BASIC,
};

PermissionStateFull testNetworkShareSettingsState = {
    .grantFlags = {2},
    .grantStatus = {PermissionState::PERMISSION_GRANTED},
    .isGeneral = true,
    .permissionName = "ohos.permission.MANAGE_SECURE_SETTINGS",
    .resDeviceID = {"local"},
};

HapPolicyParams netDataSharePolicy = {
    .apl = APL_SYSTEM_BASIC,
    .domain = "test.domain",
    .permList = {testNetworkShareSettingsPermDef},
    .permStateList = {testNetworkShareSettingsState},
};

}


NetDataShareAccessToken::NetDataShareAccessToken()
{
    currentID_ = GetSelfTokenID();
    AccessTokenIDEx tokenIdEx = AccessTokenKit::AllocHapToken(netDataShareInfo, netDataSharePolicy);
    accessID_ = tokenIdEx.tokenIdExStruct.tokenID;
    SetSelfTokenID(tokenIdEx.tokenIDEx);
}

NetDataShareAccessToken::~NetDataShareAccessToken()
{
    AccessTokenKit::DeleteToken(accessID_);
    SetSelfTokenID(currentID_);
}

/**
 * @tc.name: InsertTest001
 * @tc.desc: Test NetDataShareHelperUtils::Insert
 * @tc.type: FUNC
 */
HWTEST_F(NetDataShareHelperUtilsTest, InsertTest001, TestSize.Level1)
{
    // insert wifi
    {
        std::string on = "1";
        Uri uri(SHARING_WIFI_URI);
        int32_t ret = netDataShareHelperUtils_->Insert(uri, KEY_SHARING_WIFI, on);
        ASSERT_TRUE(ret == NETMANAGER_ERROR);

        on = "0";
        ret = netDataShareHelperUtils_->Insert(uri, KEY_SHARING_WIFI, on);
        ASSERT_TRUE(ret == NETMANAGER_ERROR);
    }
    // insert usb
    {
        std::string on = "1";
        Uri uri(SHARING_USB_URI);
        int32_t ret = netDataShareHelperUtils_->Insert(uri, KEY_SHARING_USB, on);
        ASSERT_TRUE(ret == NETMANAGER_ERROR);

        on = "0";
        ret = netDataShareHelperUtils_->Insert(uri, KEY_SHARING_USB, on);
        ASSERT_TRUE(ret == NETMANAGER_ERROR);
    }
    // insert bluetooth
    {
        std::string on = "1";
        Uri uri(SHARING_BLUETOOTH_URI);
        int32_t ret = netDataShareHelperUtils_->Insert(uri, KEY_SHARING_BLUETOOTH, on);
        ASSERT_TRUE(ret == NETMANAGER_ERROR);

        on = "0";
        ret = netDataShareHelperUtils_->Insert(uri, KEY_SHARING_BLUETOOTH, on);
        ASSERT_TRUE(ret == NETMANAGER_ERROR);
    }
}

/**
 * @tc.name: InsertTest002
 * @tc.desc: Test NetDataShareHelperUtils::Insert
 * @tc.type: FUNC
 */
HWTEST_F(NetDataShareHelperUtilsTest, InsertTest002, TestSize.Level1)
{
    NetDataShareAccessToken token;
    // insert wifi
    {
        std::string on = "1";
        Uri uri(SHARING_WIFI_URI);
        int32_t ret = netDataShareHelperUtils_->Insert(uri, KEY_SHARING_WIFI, on);
        ASSERT_TRUE(ret == NETMANAGER_SUCCESS);

        on = "0";
        ret = netDataShareHelperUtils_->Insert(uri, KEY_SHARING_WIFI, on);
        ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
    }
    // insert usb
    {
        std::string on = "1";
        Uri uri(SHARING_USB_URI);
        int32_t ret = netDataShareHelperUtils_->Insert(uri, KEY_SHARING_USB, on);
        ASSERT_TRUE(ret == NETMANAGER_SUCCESS);

        on = "0";
        ret = netDataShareHelperUtils_->Insert(uri, KEY_SHARING_USB, on);
        ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
    }
    // insert bluetooth
    {
        std::string on = "1";
        Uri uri(SHARING_BLUETOOTH_URI);
        int32_t ret = netDataShareHelperUtils_->Insert(uri, KEY_SHARING_BLUETOOTH, on);
        ASSERT_TRUE(ret == NETMANAGER_SUCCESS);

        on = "0";
        ret = netDataShareHelperUtils_->Insert(uri, KEY_SHARING_BLUETOOTH, on);
        ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
    }
}

/**
 * @tc.name: UpdateTest001
 * @tc.desc: Test NetDataShareHelperUtils::Update
 * @tc.type: FUNC
 */
HWTEST_F(NetDataShareHelperUtilsTest, UpdateTest001, TestSize.Level1)
{
    // Update wifi
    {
        std::string on = "1";
        Uri uri(SHARING_WIFI_URI);
        int32_t ret = netDataShareHelperUtils_->Update(uri, KEY_SHARING_WIFI, on);
        ASSERT_TRUE(ret == NETMANAGER_ERROR);

        on = "0";
        ret = netDataShareHelperUtils_->Update(uri, KEY_SHARING_WIFI, on);
        ASSERT_TRUE(ret == NETMANAGER_ERROR);
    }
    // Update usb
    {
        std::string on = "1";
        Uri uri(SHARING_USB_URI);
        int32_t ret = netDataShareHelperUtils_->Update(uri, KEY_SHARING_USB, on);
        ASSERT_TRUE(ret == NETMANAGER_ERROR);

        on = "0";
        ret = netDataShareHelperUtils_->Update(uri, KEY_SHARING_USB, on);
        ASSERT_TRUE(ret == NETMANAGER_ERROR);
    }
    // Update bluetooth
    {
        std::string on = "1";
        Uri uri(SHARING_BLUETOOTH_URI);
        int32_t ret = netDataShareHelperUtils_->Update(uri, KEY_SHARING_BLUETOOTH, on);
        ASSERT_TRUE(ret == NETMANAGER_ERROR);

        on = "0";
        ret = netDataShareHelperUtils_->Update(uri, KEY_SHARING_BLUETOOTH, on);
        ASSERT_TRUE(ret == NETMANAGER_ERROR);
    }
}

/**
 * @tc.name: UpdateTest002
 * @tc.desc: Test NetDataShareHelperUtils::Update
 * @tc.type: FUNC
 */
HWTEST_F(NetDataShareHelperUtilsTest, UpdateTest002, TestSize.Level1)
{
    NetDataShareAccessToken token;
    // Update wifi
    {
        std::string on = "1";
        Uri uri(SHARING_WIFI_URI);
        int32_t ret = netDataShareHelperUtils_->Update(uri, KEY_SHARING_WIFI, on);
        ASSERT_TRUE(ret == NETMANAGER_SUCCESS);

        on = "0";
        ret = netDataShareHelperUtils_->Update(uri, KEY_SHARING_WIFI, on);
        ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
    }
    // Update usb
    {
        std::string on = "1";
        Uri uri(SHARING_USB_URI);
        int32_t ret = netDataShareHelperUtils_->Update(uri, KEY_SHARING_USB, on);
        ASSERT_TRUE(ret == NETMANAGER_SUCCESS);

        on = "0";
        ret = netDataShareHelperUtils_->Update(uri, KEY_SHARING_USB, on);
        ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
    }
    // Update bluetooth
    {
        std::string on = "1";
        Uri uri(SHARING_BLUETOOTH_URI);
        int32_t ret = netDataShareHelperUtils_->Update(uri, KEY_SHARING_BLUETOOTH, on);
        ASSERT_TRUE(ret == NETMANAGER_SUCCESS);

        on = "0";
        ret = netDataShareHelperUtils_->Update(uri, KEY_SHARING_BLUETOOTH, on);
        ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
    }
}

/**
 * @tc.name: QueryTest001
 * @tc.desc: Test NetDataShareHelperUtils::Query
 * @tc.type: FUNC
 */
HWTEST_F(NetDataShareHelperUtilsTest, QueryTest001, TestSize.Level1)
{
    // Query wifi
    {
        std::string on;
        Uri uri(SHARING_WIFI_URI);
        int32_t ret = netDataShareHelperUtils_->Query(uri, KEY_SHARING_WIFI, on);
        ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
        std::cout << "wifi QueryTest result:" << on << std::endl;
    }
    // Query usb
    {
        std::string on;
        Uri uri(SHARING_USB_URI);
        int32_t ret = netDataShareHelperUtils_->Query(uri, KEY_SHARING_USB, on);
        ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
        std::cout << "usb QueryTest result:" << on << std::endl;
    }
    // Query bluetooth
    {
        std::string on;
        Uri uri(SHARING_BLUETOOTH_URI);
        int32_t ret = netDataShareHelperUtils_->Query(uri, KEY_SHARING_BLUETOOTH, on);
        ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
        std::cout << "bluetooth QueryTest result:" << on << std::endl;
    }
}

/**
 * @tc.name: QueryTest002
 * @tc.desc: Test NetDataShareHelperUtils::Query
 * @tc.type: FUNC
 */
HWTEST_F(NetDataShareHelperUtilsTest, QueryTest002, TestSize.Level1)
{
    NetDataShareAccessToken token;
    // Query wifi
    {
        std::string on;
        Uri uri(SHARING_WIFI_URI);
        int32_t ret = netDataShareHelperUtils_->Query(uri, KEY_SHARING_WIFI, on);
        ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
        std::cout << "wifi QueryTest result:" << on << std::endl;
    }
    // Query usb
    {
        std::string on;
        Uri uri(SHARING_USB_URI);
        int32_t ret = netDataShareHelperUtils_->Query(uri, KEY_SHARING_USB, on);
        ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
        std::cout << "usb QueryTest result:" << on << std::endl;
    }
    // Query bluetooth
    {
        std::string on;
        Uri uri(SHARING_BLUETOOTH_URI);
        int32_t ret = netDataShareHelperUtils_->Query(uri, KEY_SHARING_BLUETOOTH, on);
        ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
        std::cout << "bluetooth QueryTest result:" << on << std::endl;
    }
}
} // namespace NetManagerStandard
} // namespace OHOS
