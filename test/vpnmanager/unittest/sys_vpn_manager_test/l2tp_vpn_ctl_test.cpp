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

#include "net_manager_constants.h"

#ifdef GTEST_API_
#define private public
#define protected public
#endif
#include "l2tp_vpn_ctl.h"
#include "vpn_config.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;

class L2tpVpnCtlTest : public testing::Test {
public:
    static inline std::unique_ptr<IpsecVpnCtl> l2tpControl_ = nullptr;
    static void SetUpTestSuite();
};

void L2tpVpnCtlTest::SetUpTestSuite()
{
    sptr<L2tpVpnConfig> l2tpVpnconfig = new (std::nothrow) L2tpVpnConfig();
    if (l2tpVpnconfig == nullptr) {
        return;
    }
    int32_t userId = 0;
    std::vector<int32_t> activeUserIds;
    l2tpControl_ = std::make_unique<L2tpVpnCtl>(l2tpVpnconfig, "pkg", userId, activeUserIds);
    if (l2tpControl_ == nullptr) {
        return;
    }
    l2tpControl_->l2tpVpnConfig_ = l2tpVpnconfig;
}

HWTEST_F(L2tpVpnCtlTest, SetUp001, TestSize.Level1)
{
    if (l2tpControl_ == nullptr) {
        return;
    }
    EXPECT_EQ(l2tpControl_->SetUp(), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(L2tpVpnCtlTest, Destroy001, TestSize.Level1)
{
    if (l2tpControl_ == nullptr) {
        return;
    }
    EXPECT_EQ(l2tpControl_->Destroy(), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(L2tpVpnCtlTest, IsInternalVpn001, TestSize.Level1)
{
    if (l2tpControl_ == nullptr) {
        return;
    }
    EXPECT_EQ(l2tpControl_->IsInternalVpn(), true);
}

HWTEST_F(L2tpVpnCtlTest, GetConnectedSysVpnConfigTest001, TestSize.Level1)
{
    if (l2tpControl_ == nullptr) {
        return;
    }
    sptr<SysVpnConfig> resConfig = nullptr;
    int32_t ret = l2tpControl_->GetConnectedSysVpnConfig(resConfig);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    l2tpControl_->state_ = IpsecVpnStateCode::STATE_CONNECTED;
    ret = l2tpControl_->GetConnectedSysVpnConfig(resConfig);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    sptr<L2tpVpnConfig> tmp = l2tpControl_->l2tpVpnConfig_;
    l2tpControl_->l2tpVpnConfig_ = nullptr;
    ret = l2tpControl_->GetConnectedSysVpnConfig(resConfig);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    l2tpControl_->l2tpVpnConfig_ = tmp;
}

HWTEST_F(L2tpVpnCtlTest, NotifyConnectStageTest001, TestSize.Level1)
{
    if (l2tpControl_ == nullptr) {
        return;
    }
    std::string stage;
    int32_t errorCode = 1;
    int32_t ret = l2tpControl_->NotifyConnectStage(stage, errorCode);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    stage = IPSEC_START_TAG;
    ret = l2tpControl_->NotifyConnectStage(stage, errorCode);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INTERNAL);

    errorCode = NETMANAGER_EXT_SUCCESS;
    l2tpControl_->state_ = IpsecVpnStateCode::STATE_INIT;
    ret = l2tpControl_->NotifyConnectStage(stage, errorCode);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    l2tpControl_->state_ = IpsecVpnStateCode::STATE_STARTED;
    stage = L2TP_IPSEC_CONFIGURED_TAG;
    ret = l2tpControl_->NotifyConnectStage(stage, errorCode);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    l2tpControl_->state_ = IpsecVpnStateCode::STATE_CONFIGED;
    stage = IPSEC_CONNECT_TAG;
    ret = l2tpControl_->NotifyConnectStage(stage, errorCode);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    l2tpControl_->state_ = IpsecVpnStateCode::STATE_CONTROLLED;
    stage = L2TP_IPSEC_CONNECTED_TAG;
    ret = l2tpControl_->NotifyConnectStage(stage, errorCode);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    l2tpControl_->state_ = IpsecVpnStateCode::STATE_DISCONNECTED;
    ret = l2tpControl_->NotifyConnectStage(stage, errorCode);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INTERNAL);

    stage = "stageTest";
    l2tpControl_->state_ = IpsecVpnStateCode::STATE_INIT;
    EXPECT_EQ(l2tpControl_->NotifyConnectStage(stage, errorCode), NETMANAGER_EXT_SUCCESS);
    l2tpControl_->state_ = IpsecVpnStateCode::STATE_STARTED;
    EXPECT_EQ(l2tpControl_->NotifyConnectStage(stage, errorCode), NETMANAGER_EXT_SUCCESS);
    l2tpControl_->state_ = IpsecVpnStateCode::STATE_CONFIGED;
    EXPECT_EQ(l2tpControl_->NotifyConnectStage(stage, errorCode), NETMANAGER_EXT_SUCCESS);
    l2tpControl_->state_ = IpsecVpnStateCode::STATE_CONTROLLED;
    EXPECT_EQ(l2tpControl_->NotifyConnectStage(stage, errorCode), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(L2tpVpnCtlTest, GetSysVpnCertUriTest001, TestSize.Level1)
{
    sptr<L2tpVpnConfig> config = new (std::nothrow) L2tpVpnConfig();
    if (config == nullptr) {
        return;
    }
    if (l2tpControl_ == nullptr) {
        return;
    }
    config->ipsecCaCertConf_ = "CaCertUri";
    l2tpControl_->l2tpVpnConfig_ = nullptr;
    std::string certUri;
    int32_t certType = 0;
    int32_t ret = l2tpControl_->GetSysVpnCertUri(certType, certUri);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INTERNAL);
    l2tpControl_->l2tpVpnConfig_ = config;
    ret = l2tpControl_->GetSysVpnCertUri(certType, certUri);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ("CaCertUri", certUri);
}

HWTEST_F(L2tpVpnCtlTest, GetSysVpnCertUriTest002, TestSize.Level1)
{
    sptr<L2tpVpnConfig> config = new (std::nothrow) L2tpVpnConfig();
    if (config == nullptr) {
        return;
    }
    if (l2tpControl_ == nullptr) {
        return;
    }
    config->ipsecPublicUserCertConf_ = "UserCertUri";
    l2tpControl_->l2tpVpnConfig_ = config;
    std::string certUri;
    int32_t certType = 1;
    EXPECT_EQ(l2tpControl_->GetSysVpnCertUri(certType, certUri), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ("UserCertUri", certUri);
    certType = 2;
    EXPECT_EQ(l2tpControl_->GetSysVpnCertUri(certType, certUri), NETMANAGER_EXT_SUCCESS);
    certType = -1;
    EXPECT_EQ(l2tpControl_->GetSysVpnCertUri(certType, certUri), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(L2tpVpnCtlTest, InitConfigFile001, TestSize.Level1)
{
    sptr<L2tpVpnConfig> config = new (std::nothrow) L2tpVpnConfig();
    if (config == nullptr) {
        return;
    }
    if (l2tpControl_ == nullptr) {
        return;
    }
    config->ipsecPublicUserCertConf_ = "testUserUri";
    config->xl2tpdConf_ = "testXl2tpdConf";
    config->strongswanConf_ = "testStrongswanConf";
    config->ipsecConf_ = "ipsecConfTest";
    config->ipsecSecrets_ = "ipsecSecretsTest";
    config->optionsL2tpdClient_ = "optionsL2tpdClientTest";
    l2tpControl_->l2tpVpnConfig_ = nullptr;
    int32_t ret = l2tpControl_->InitConfigFile();
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INTERNAL);
    l2tpControl_->l2tpVpnConfig_ = config;
    ret = l2tpControl_->InitConfigFile();
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(L2tpVpnCtlTest, InitConfigFile002, TestSize.Level1)
{
    sptr<L2tpVpnConfig> config = new (std::nothrow) L2tpVpnConfig();
    if (config == nullptr) {
        return;
    }
    if (l2tpControl_ == nullptr) {
        return;
    }
    config->ipsecPublicUserCertConf_ = "testUserUri";
    config->xl2tpdConf_ = "INVALID_BASE64";
    config->strongswanConf_ = "INVALID_BASE64";
    config->ipsecConf_ = "INVALID_BASE64";
    config->ipsecSecrets_ = "ipsecSecretsTest";
    config->optionsL2tpdClient_ = "optionsL2tpdClientTest";
    l2tpControl_->l2tpVpnConfig_ = config;
    int32_t ret = l2tpControl_->InitConfigFile();
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(L2tpVpnCtlTest, GetSysVpnCertUriTest003, TestSize.Level1)
{
    sptr<L2tpVpnConfig> config = new (std::nothrow) L2tpVpnConfig();
    if (config == nullptr) {
        return;
    }
    if (l2tpControl_ == nullptr) {
        return;
    }
    config->ipsecPublicUserCertConf_ = "UserCertUri";
    l2tpControl_->l2tpVpnConfig_ = config;
    std::string certUri;
    int32_t certType = 1;
    EXPECT_EQ(l2tpControl_->GetSysVpnCertUri(certType, certUri), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ("UserCertUri", certUri);
    certType = 4;
    EXPECT_EQ(l2tpControl_->GetSysVpnCertUri(certType, certUri), NETMANAGER_EXT_SUCCESS);
    certType = 5;
    EXPECT_EQ(l2tpControl_->GetSysVpnCertUri(certType, certUri), NETMANAGER_EXT_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS