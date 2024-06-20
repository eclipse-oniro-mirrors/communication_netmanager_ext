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

#include "sysvpn_config.h"


namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr const char* TEST_VPNID = "vpnId_";
constexpr const char* TEST_VPN_NAME = "vpnName_";
constexpr int32_t TEST_VPN_TYPE = 1;
constexpr const char* TEST_USER_NAME= "userName_";
constexpr const char* TEST_PASSWORD= "password_";
constexpr bool TEST_SAVE_LOGIN = false;
constexpr int32_t  TEST_USERID = 0;
constexpr const char* TEST_FORWARD ="forwardingRoutes_";

SysVpnConfig GetSysVpnConfigData()
{
    SysVpnConfig infoSequence;
    infoSequence.vpnId_ = TEST_VPNID;
    infoSequence.vpnName_ = TEST_VPN_NAME;
    infoSequence.vpnType_ = TEST_VPN_TYPE;
    infoSequence.userName_ = TEST_USER_NAME;
    infoSequence.password_ = TEST_PASSWORD;
    infoSequence.saveLogin_ = TEST_SAVE_LOGIN;
    infoSequence.userId_ = TEST_USERID;
    infoSequence.forwardingRoutes_ = TEST_FORWARD;
    return infoSequence;
}
} // namespace

using namespace testing::ext;
class SysVpnConfigTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void SysVpnConfigTest::SetUpTestCase() {}

void SysVpnConfigTest::TearDownTestCase() {}

void SysVpnConfigTest::SetUp() {}

void SysVpnConfigTest::TearDown() {}

/**
* @tc.name: MarshallingUnmarshallingTest001
* @tc.desc: Test SysVpnConfig Marshalling and Unmarshalling
* @tc.type: FUNC
*/
HWTEST_F(SysVpnConfigTest, MarshallingUnmarshallingTest001, TestSize.Level1)
{
    Parcel parcel;
    SysVpnConfig info = GetSysVpnConfigData();
    EXPECT_TRUE(info.Marshalling(parcel));
    sptr<SysVpnConfig> result = new (std::nothrow) SysVpnConfig();
    SysVpnConfig::Unmarshalling(parcel, result);
    if (result != nullptr)
    {
        EXPECT_EQ(result->vpnId_, info.vpnId_);
        EXPECT_EQ(result->vpnName_, info.vpnName_);
        EXPECT_EQ(result->vpnType_, info.vpnType_);
        EXPECT_EQ(result->userName_, info.userName_);
        EXPECT_EQ(result->password_, info.password_);
        EXPECT_EQ(result->saveLogin_, info.saveLogin_);
        EXPECT_EQ(result->userId_, info.userId_);
        EXPECT_EQ(result->forwardingRoutes_, info.forwardingRoutes_);
    }
}
}
}