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

#include "ipsecvpn_config.h"


namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr const char* TEST_VPNID = "vpnId_";
constexpr const char* TEST_VPN_NAME = "vpnName_";
constexpr int32_t TEST_VPN_TYPE = 1;
constexpr const char* TEST_USER_NAME = "userName_";
constexpr const char* TEST_PASSWORD = "password_";
constexpr bool TEST_SAVE_LOGIN = false;
constexpr int32_t  TEST_USERID = 0;
constexpr const char* TEST_FORWARD = "forwardingRoutes_";

constexpr const char* TEST_PRE_SHARED_KEY = "ipsecPreSharedKey_";
constexpr const char* TEST_IPSEC_IDENTIFIER = "ipsecIdentifier_";
constexpr const char* TEST_IPSEC_SWANCTL_CONF = "swanctlConf_";
constexpr const char* TEST_STRONGSWAN_CONF = "strongswanConf_";
constexpr const char* TEST_CA_CERT_CONF = "ipsecCaCertConf_";
constexpr const char* TEST_PRIVATE_USER_CERT_CONF = "ipsecPrivateUserCertConf_";
constexpr const char* TEST_PUBLIC_USER_CERT_CONF = "ipsecPublicUserCertConf_";
constexpr const char* TEST_PRIVATE_SERVER_CERT_CONF = "ipsecPrivateServerCertConf_";
constexpr const char* TEST_PUBLIC_SERVER_CERT_CONF = "ipsecPublicServerCertConf_";
constexpr const char* TEST_CA_CERT_FILE_PATH = "ipsecCaCertFilePath_";
constexpr const char* TEST_PRIVATE_USER_CERT_FILE_PATH = "ipsecPrivateUserCertFilePath_";
constexpr const char* TEST_PUBLIC_USER_CERT_FILE_PATH = "ipsecPublicUserCertFilePath_";
constexpr const char* TEST_PRIVATE_SERVER_CERT_FILE_PATH = "ipsecPrivateServerCertFilePath_";
constexpr const char* TEST_PUBLIC_ERVER_CERT_FILE_PATH = "ipsecPublicServerCertFilePath_";

IpsecVpnConfig GetIpsecVpnConfigData()
{
    IpsecVpnConfig infoSequence;
    infoSequence.ipsecPreSharedKey_ = TEST_PRE_SHARED_KEY;
    infoSequence.ipsecIdentifier_ = TEST_IPSEC_IDENTIFIER;
    infoSequence.swanctlConf_ = TEST_IPSEC_SWANCTL_CONF;
    infoSequence.strongswanConf_ = TEST_STRONGSWAN_CONF;
    infoSequence.ipsecCaCertConf_ = TEST_CA_CERT_CONF;
    infoSequence.ipsecPrivateUserCertConf_ = TEST_PRIVATE_USER_CERT_CONF;
    infoSequence.ipsecPublicUserCertConf_ = TEST_PUBLIC_USER_CERT_CONF;
    infoSequence.ipsecPrivateServerCertConf_ = TEST_PRIVATE_SERVER_CERT_CONF;
    infoSequence.ipsecPublicServerCertConf_ = TEST_PUBLIC_SERVER_CERT_CONF;
    infoSequence.ipsecCaCertFilePath_ = TEST_CA_CERT_FILE_PATH;
    infoSequence.ipsecPrivateUserCertFilePath_ = TEST_PRIVATE_USER_CERT_FILE_PATH;
    infoSequence.ipsecPublicUserCertFilePath_ = TEST_PUBLIC_USER_CERT_FILE_PATH;
    infoSequence.ipsecPrivateServerCertFilePath_ = TEST_PRIVATE_SERVER_CERT_FILE_PATH;
    infoSequence.ipsecPublicServerCertFilePath_ = TEST_PUBLIC_ERVER_CERT_FILE_PATH;

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
}

using namespace testing::ext;
class IpsecVpnConfigTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void IpsecVpnConfigTest::SetUpTestCase() {}

void IpsecVpnConfigTest::TearDownTestCase() {}

void IpsecVpnConfigTest::SetUp() {}

void IpsecVpnConfigTest::TearDown() {}

/**
* @tc.name: MarshallingUnmarshallingTest001
* @tc.desc: Test IpsecVpnConfig Marshalling and Unmarshalling
* @tc.type: FUNC
*/
HWTEST_F(IpsecVpnConfigTest, MarshallingUnmarshallingTest001, TestSize.Level1)
{
    Parcel parcel;
    IpsecVpnConfig info = GetIpsecVpnConfigData();
    EXPECT_TRUE(info.Marshalling(parcel));
    sptr<IpsecVpnConfig> result;
    result = IpsecVpnConfig::Unmarshalling(parcel);
    if (result != nullptr)
    {
        EXPECT_EQ(result->ipsecPreSharedKey_, info.ipsecPreSharedKey_);
        EXPECT_EQ(result->ipsecIdentifier_, info.ipsecIdentifier_);
        EXPECT_EQ(result->swanctlConf_, info.swanctlConf_);
        EXPECT_EQ(result->strongswanConf_, info.strongswanConf_);
        EXPECT_EQ(result->ipsecCaCertConf_, info.ipsecCaCertConf_);
        EXPECT_EQ(result->ipsecPrivateUserCertConf_, info.ipsecPrivateUserCertConf_);
        EXPECT_EQ(result->ipsecPublicUserCertConf_, info.ipsecPublicUserCertConf_);
        EXPECT_EQ(result->ipsecPrivateServerCertConf_, info.ipsecPrivateServerCertConf_);
        EXPECT_EQ(result->ipsecPublicServerCertConf_, info.ipsecPublicServerCertConf_);
        EXPECT_EQ(result->ipsecCaCertFilePath_, info.ipsecCaCertFilePath_);
        EXPECT_EQ(result->ipsecPrivateUserCertFilePath_, info.ipsecPrivateUserCertFilePath_);
        EXPECT_EQ(result->ipsecPublicUserCertFilePath_, info.ipsecPublicUserCertFilePath_);
        EXPECT_EQ(result->ipsecPrivateServerCertFilePath_, info.ipsecPrivateServerCertFilePath_);
        EXPECT_EQ(result->ipsecPublicServerCertFilePath_, info.ipsecPublicServerCertFilePath_);
    }
}
}