/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include "ipsecvpn_config.h"
#include "l2tpvpn_config.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;

constexpr const char *TEST_VPNID = "vpnId_";
constexpr const char *TEST_VPN_NAME = "vpnName_";
constexpr int32_t TEST_VPN_TYPE = 1;
constexpr const char *TEST_USER_NAME = "userName_";
constexpr const char *TEST_PASSWORD = "password_";
constexpr bool TEST_SAVE_LOGIN = false;
constexpr int32_t TEST_USERID = 0;
constexpr const char *TEST_FORWARD = "forwardingRoutes_";

IpsecVpnConfig GetIpsecVpnConfigData()
{
    IpsecVpnConfig infoSequence;
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

L2tpVpnConfig GetL2tpVpnConfigData()
{
    L2tpVpnConfig infoSequence;
    infoSequence.vpnId_ = TEST_VPNID;
    infoSequence.vpnName_ = TEST_VPN_NAME;
    infoSequence.vpnType_ = VpnType::L2TP_IPSEC_PSK;
    infoSequence.userName_ = TEST_USER_NAME;
    infoSequence.password_ = TEST_PASSWORD;
    infoSequence.saveLogin_ = TEST_SAVE_LOGIN;
    infoSequence.userId_ = TEST_USERID;
    infoSequence.forwardingRoutes_ = TEST_FORWARD;
    return infoSequence;
}
} // namespace

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

HWTEST_F(SysVpnConfigTest, Unmarshalling001, TestSize.Level1)
{
    Parcel parcel;
    IpsecVpnConfig info = GetIpsecVpnConfigData();
    info.vpnType_ = VpnType::IKEV2_IPSEC_MSCHAPv2;
    EXPECT_TRUE(info.Marshalling(parcel));
    sptr<SysVpnConfig> result = SysVpnConfig::Unmarshalling(parcel);
    EXPECT_TRUE(result != nullptr);
}

HWTEST_F(SysVpnConfigTest, Unmarshalling002, TestSize.Level1)
{
    Parcel parcel;
    IpsecVpnConfig info = GetIpsecVpnConfigData();
    info.vpnType_ = VpnType::IKEV2_IPSEC_PSK;
    EXPECT_TRUE(info.Marshalling(parcel));
    sptr<SysVpnConfig> result = SysVpnConfig::Unmarshalling(parcel);
    EXPECT_TRUE(result != nullptr);
}

HWTEST_F(SysVpnConfigTest, Unmarshalling003, TestSize.Level1)
{
    Parcel parcel;
    IpsecVpnConfig info = GetIpsecVpnConfigData();
    info.vpnType_ = VpnType::IKEV2_IPSEC_RSA;
    EXPECT_TRUE(info.Marshalling(parcel));
    sptr<SysVpnConfig> result = SysVpnConfig::Unmarshalling(parcel);
    EXPECT_TRUE(result != nullptr);
}

HWTEST_F(SysVpnConfigTest, Unmarshalling004, TestSize.Level1)
{
    Parcel parcel;
    L2tpVpnConfig info = GetL2tpVpnConfigData();
    info.vpnType_ = VpnType::L2TP_IPSEC_PSK;
    EXPECT_TRUE(info.Marshalling(parcel));
    sptr<SysVpnConfig> result = SysVpnConfig::Unmarshalling(parcel);
    EXPECT_TRUE(result != nullptr);
}

HWTEST_F(SysVpnConfigTest, Unmarshalling005, TestSize.Level1)
{
    Parcel parcel;
    L2tpVpnConfig info = GetL2tpVpnConfigData();
    info.vpnType_ = VpnType::L2TP_IPSEC_RSA;
    EXPECT_TRUE(info.Marshalling(parcel));
    sptr<SysVpnConfig> result = SysVpnConfig::Unmarshalling(parcel);
    EXPECT_TRUE(result != nullptr);
}

HWTEST_F(SysVpnConfigTest, Unmarshalling006, TestSize.Level1)
{
    Parcel parcel;
    IpsecVpnConfig info = GetIpsecVpnConfigData();
    info.vpnType_ = VpnType::IPSEC_XAUTH_PSK;
    EXPECT_TRUE(info.Marshalling(parcel));
    sptr<SysVpnConfig> result = SysVpnConfig::Unmarshalling(parcel);
    EXPECT_TRUE(result != nullptr);
}

HWTEST_F(SysVpnConfigTest, Unmarshalling007, TestSize.Level1)
{
    Parcel parcel;
    IpsecVpnConfig info = GetIpsecVpnConfigData();
    info.vpnType_ = VpnType::IPSEC_XAUTH_RSA;
    EXPECT_TRUE(info.Marshalling(parcel));
    sptr<SysVpnConfig> result = SysVpnConfig::Unmarshalling(parcel);
    EXPECT_TRUE(result != nullptr);
}

HWTEST_F(SysVpnConfigTest, Unmarshalling008, TestSize.Level1)
{
    Parcel parcel;
    IpsecVpnConfig info = GetIpsecVpnConfigData();
    info.vpnType_ = VpnType::IPSEC_HYBRID_RSA;
    EXPECT_TRUE(info.Marshalling(parcel));
    sptr<SysVpnConfig> result = SysVpnConfig::Unmarshalling(parcel);
    EXPECT_TRUE(result != nullptr);
}

HWTEST_F(SysVpnConfigTest, Unmarshalling009, TestSize.Level1)
{
    Parcel parcel;
    IpsecVpnConfig info = GetIpsecVpnConfigData();
    info.vpnType_ = -1;
    EXPECT_TRUE(info.Marshalling(parcel));
    sptr<SysVpnConfig> result = SysVpnConfig::Unmarshalling(parcel);
    EXPECT_TRUE(result == nullptr);
}
} // namespace NetManagerStandard
} // namespace OHOS