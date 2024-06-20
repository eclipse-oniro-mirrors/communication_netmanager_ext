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

#include "l2tpvpn_config.h"


namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr const char* TEST_SHARED_KEY = "ipsecPreSharedKey_";
constexpr const char* TEST_IDENRIFIER = "ipsecIdentifier_";
constexpr const char* TEST_STRONG_SWAN_CONFIG = "strongswanConf_";
constexpr const char* TEST_CA_CERT_CONFIG= "ipsecCaCertConf_";
constexpr const char* TEST_PRIVATE_USER_CERT_CONFIG= "ipsecPrivateUserCertConf_";
constexpr const char* TEST_PUBLIC_USER_CERT_CONFIG= "ipsecPublicUserCertConf_";
constexpr const char* TEST_PRIVATE_SERVER_CERT_CONFIG= "ipsecPrivateServerCertConf_";
constexpr const char* TEST_PUBLIC_SERVER_CERT_CONFIG= "ipsecPublicServerCertConf_";
constexpr const char* TEST_CA_CERT_FILE_PATH= "ipsecCaCertFilePath_";
constexpr const char* TEST_PRIVATE_USER_CERT_FILE_PATH= "ipsecPrivateUserCertFilePath_";
constexpr const char* TEST_PUBLIC_USER_CERT_FILE_PATH= "ipsecPublicUserCertFilePath_";
constexpr const char* TEST_PRIVATE_SERVER_CERT_FILE_PATH= "ipsecPrivateServerCertFilePath_";
constexpr const char* TEST_PUBLIC_SERVER_CERT_FILE_PATH= "ipsecPublicServerCertFilePath_";
constexpr const char* TEST_IPSEC_CONFIG ="ipsecConf_";
constexpr const char* TEST_IPSEC_SECRETS ="ipsecSecrets_";
constexpr const char* TEST_OPTION_CLIENT ="optionsL2tpdClient_";
constexpr const char* TEST_XL2TPD_CONFIG ="xl2tpdConf_";
constexpr const char* TEST_L2TP_SHARED_KEY ="l2tpSharedKey_";

L2tpVpnConfig GetL2tpVpnConfigData()
{
    L2tpVpnConfig infoSequence;
    infoSequence.ipsecPreSharedKey_ = TEST_SHARED_KEY;
    infoSequence.ipsecIdentifier_ = TEST_IDENRIFIER;
    infoSequence.strongswanConf_ = TEST_STRONG_SWAN_CONFIG;
    infoSequence.ipsecCaCertConf_ = TEST_CA_CERT_CONFIG;
    infoSequence.ipsecPrivateUserCertConf_ = TEST_PRIVATE_USER_CERT_CONFIG;
    infoSequence.ipsecPublicUserCertConf_ = TEST_PUBLIC_USER_CERT_CONFIG;
    infoSequence.ipsecPrivateServerCertConf_ = TEST_PRIVATE_SERVER_CERT_CONFIG;
    infoSequence.ipsecPublicServerCertConf_ = TEST_PUBLIC_SERVER_CERT_CONFIG;
    infoSequence.ipsecCaCertFilePath_ = TEST_CA_CERT_FILE_PATH;
    infoSequence.ipsecPrivateUserCertFilePath_ = TEST_PRIVATE_USER_CERT_FILE_PATH;
    infoSequence.ipsecPublicUserCertFilePath_ = TEST_PUBLIC_USER_CERT_FILE_PATH;
    infoSequence.ipsecPrivateServerCertFilePath_ = TEST_PRIVATE_SERVER_CERT_FILE_PATH;
    infoSequence.ipsecPublicServerCertFilePath_ = TEST_PUBLIC_SERVER_CERT_FILE_PATH;
    infoSequence.ipsecConf_ = TEST_IPSEC_CONFIG;
    infoSequence.ipsecSecrets_ = TEST_IPSEC_SECRETS;
    infoSequence.optionsL2tpdClient_ = TEST_OPTION_CLIENT;
    infoSequence.xl2tpdConf_ = TEST_XL2TPD_CONFIG;
    infoSequence.l2tpSharedKey_ = TEST_L2TP_SHARED_KEY;
    return infoSequence;
}
} // namespace

using namespace testing::ext;
class L2tpVpnConfigTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void L2tpVpnConfigTest::SetUpTestCase() {}

void L2tpVpnConfigTest::TearDownTestCase() {}

void L2tpVpnConfigTest::SetUp() {}

void L2tpVpnConfigTest::TearDown() {}

/**
* @tc.name: MarshallingUnmarshallingTest001
* @tc.desc: Test L2tpVpnConfig Marshalling and Unmarshalling
* @tc.type: FUNC
*/
HWTEST_F(L2tpVpnConfigTest, MarshallingUnmarshallingTest001, TestSize.Level1)
{
    Parcel parcel;
    L2tpVpnConfig info = GetL2tpVpnConfigData();
    EXPECT_TRUE(info.Marshalling(parcel));
    sptr<L2tpVpnConfig> result;
    result = L2tpVpnConfig::Unmarshalling(parcel);
    if (result != nullptr)
    {
        EXPECT_EQ(result->ipsecPreSharedKey_, info.ipsecPreSharedKey_);
        EXPECT_EQ(result->ipsecIdentifier_, info.ipsecIdentifier_);
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
        EXPECT_EQ(result->ipsecConf_, info.ipsecConf_);
        EXPECT_EQ(result->ipsecSecrets_, info.ipsecSecrets_);
        EXPECT_EQ(result->optionsL2tpdClient_, info.optionsL2tpdClient_);
        EXPECT_EQ(result->xl2tpdConf_, info.xl2tpdConf_);
        EXPECT_EQ(result->l2tpSharedKey_, info.l2tpSharedKey_);
    }
}
}
}