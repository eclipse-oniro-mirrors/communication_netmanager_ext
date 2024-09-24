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

#include "openvpn_config.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr const char *TEST_VPNID = "vpnId_";
constexpr const char *TEST_VPN_NAME = "vpnName_";
constexpr int32_t TEST_VPN_TYPE = 9;
constexpr const char *TEST_USER_NAME = "userName_";
constexpr const char *TEST_PASSWORD = "password_";
constexpr bool TEST_SAVE_LOGIN = false;
constexpr int32_t TEST_USERID = 0;
constexpr const char *TEST_FORWARD = "forwardingRoutes_";

constexpr const char *TEST_OPENVPN_PORT = "ovpnPort_";
constexpr int32_t TEST_OPENVPN_PROTOVOL = 23;
constexpr const char *TEST_OPENVPN_CONFIG = "ovpnConfig_";
constexpr int32_t TEST_OPENVPN_AUTH_TYPE = 1;
constexpr const char *TEST_ASKPASS = "askpass_";
constexpr const char *TEST_OPENVPN_CONFIG_FILE_PATH = "ovpnConfigFilePath_";
constexpr const char *TEST_OPENVPN_CA_CERT_FILE_PATH = "ovpnCaCertFilePath_";
constexpr const char *TEST_OPENVPN_USER_CERT_FILE_PATH = "ovpnUserCertFilePath_";
constexpr const char *TEST_OPENVPN_PRIVATE_KEY_FILE_PATH = "ovpnPrivateKeyFilePath_";

OpenvpnConfig GetOpenvpnConfigData()
{
    OpenvpnConfig infoSequence;
    infoSequence.ovpnPort_ = TEST_OPENVPN_PORT;
    infoSequence.ovpnProtocol_ = TEST_OPENVPN_PROTOVOL;
    infoSequence.ovpnConfig_ = TEST_OPENVPN_CONFIG;
    infoSequence.ovpnAuthType_ = TEST_OPENVPN_AUTH_TYPE;
    infoSequence.askpass_ = TEST_ASKPASS;
    infoSequence.ovpnConfigFilePath_ = TEST_OPENVPN_CONFIG_FILE_PATH;
    infoSequence.ovpnCaCertFilePath_ = TEST_OPENVPN_CA_CERT_FILE_PATH;
    infoSequence.ovpnUserCertFilePath_ = TEST_OPENVPN_USER_CERT_FILE_PATH;
    infoSequence.ovpnPrivateKeyFilePath_ = TEST_OPENVPN_PRIVATE_KEY_FILE_PATH;

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
class OpenvpnConfigTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void OpenvpnConfigTest::SetUpTestCase() {}

void OpenvpnConfigTest::TearDownTestCase() {}

void OpenvpnConfigTest::SetUp() {}

void OpenvpnConfigTest::TearDown() {}

HWTEST_F(OpenvpnConfigTest, MarshallingUnmarshallingTest001, TestSize.Level1)
{
    Parcel parcel;
    OpenvpnConfig info = GetOpenvpnConfigData();
    EXPECT_TRUE(info.Marshalling(parcel));
    int32_t type;
    parcel.ReadInt32(type);
    sptr<OpenvpnConfig> result = OpenvpnConfig::Unmarshalling(parcel);
    ASSERT_TRUE(result != nullptr);
    EXPECT_EQ(result->ovpnPort_, info.ovpnPort_);
    EXPECT_EQ(result->ovpnProtocol_, info.ovpnProtocol_);
    EXPECT_EQ(result->ovpnConfig_, info.ovpnConfig_);
    EXPECT_EQ(result->ovpnAuthType_, info.ovpnAuthType_);
    EXPECT_EQ(result->askpass_, info.askpass_);
    EXPECT_EQ(result->ovpnConfigFilePath_, info.ovpnConfigFilePath_);
    EXPECT_EQ(result->ovpnCaCertFilePath_, info.ovpnCaCertFilePath_);
    EXPECT_EQ(result->ovpnUserCertFilePath_, info.ovpnUserCertFilePath_);
    EXPECT_EQ(result->ovpnPrivateKeyFilePath_, info.ovpnPrivateKeyFilePath_);
}
} //namespace NetManagerStandard
} //namespace OHOS