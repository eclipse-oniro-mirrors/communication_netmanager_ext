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

HWTEST_F(L2tpVpnCtlTest, SetUp002, TestSize.Level1)
{
    sptr<L2tpVpnConfig> l2tpVpnconfig = new (std::nothrow) L2tpVpnConfig();
    ASSERT_NE(l2tpVpnconfig, nullptr);
    sptr<INetAddr> netAddr = new (std::nothrow) INetAddr();
    ASSERT_NE(netAddr, nullptr);
    std::string ip = "1.1.1.1";
    netAddr->address_ = ip;
    netAddr->prefixlen_ = 1;
    l2tpVpnconfig->addresses_.push_back(*netAddr);
    l2tpVpnconfig->vpnId_ = "123";
    l2tpVpnconfig->vpnName_ = "testSetUpVpn";
    l2tpVpnconfig->vpnType_ = 1;
    int32_t userId = 0;
    std::vector<int32_t> activeUserIds;
    std::unique_ptr<L2tpVpnCtl> l2tpControl =
        std::make_unique<L2tpVpnCtl>(l2tpVpnconfig, "pkg", userId, activeUserIds);
    ASSERT_NE(l2tpControl, nullptr);
    l2tpControl->l2tpVpnConfig_ = l2tpVpnconfig;
    EXPECT_EQ(l2tpControl->SetUp(), NETMANAGER_EXT_SUCCESS);
    l2tpControl->l2tpVpnConfig_->vpnType_ = 10;
    EXPECT_EQ(l2tpControl->SetUp(), NETMANAGER_EXT_SUCCESS);
    l2tpControl->l2tpVpnConfig_->vpnType_ = 4;
    EXPECT_EQ(l2tpControl->SetUp(), NETMANAGER_EXT_SUCCESS);
    sptr<MultiVpnInfo> vpnInfo = new (std::nothrow) MultiVpnInfo();
    ASSERT_NE(vpnInfo, nullptr);
    l2tpControl->multiVpnInfo_ = vpnInfo;
    EXPECT_EQ(l2tpControl->SetUp(), NETMANAGER_EXT_SUCCESS);
    l2tpControl->l2tpVpnConfig_->vpnType_ = 10;
    EXPECT_EQ(l2tpControl->SetUp(), NETMANAGER_EXT_SUCCESS);
    l2tpControl->l2tpVpnConfig_->vpnType_ = 4;
    EXPECT_EQ(l2tpControl->SetUp(), NETMANAGER_EXT_SUCCESS);
    l2tpControl->l2tpVpnConfig_->vpnType_ = 5;
    EXPECT_EQ(l2tpControl->SetUp(), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(L2tpVpnCtlTest, SetUpTest003, TestSize.Level1)
{
    sptr<L2tpVpnConfig> l2tpVpnconfig = new (std::nothrow) L2tpVpnConfig();
    ASSERT_NE(l2tpVpnconfig, nullptr);
    l2tpVpnconfig->vpnType_ == 10;
    int32_t userId = 0;
    std::vector<int32_t> activeUserIds;
    std::unique_ptr<L2tpVpnCtl> l2tpControl =
        std::make_unique<L2tpVpnCtl>(l2tpVpnconfig, "pkg", userId, activeUserIds);
    ASSERT_NE(l2tpControl, nullptr);
    l2tpControl->l2tpVpnConfig_ = l2tpVpnconfig;
    l2tpControl->l2tpVpnConfig_->vpnType_ = 10;
    EXPECT_EQ(l2tpControl->SetUp(), NETMANAGER_EXT_SUCCESS);
    l2tpControl->l2tpVpnConfig_->vpnType_ = 4;
    EXPECT_EQ(l2tpControl->SetUp(), NETMANAGER_EXT_SUCCESS);
    l2tpControl->l2tpVpnConfig_->vpnType_ = 10;
    EXPECT_EQ(l2tpControl->SetUp(), NETMANAGER_EXT_SUCCESS);
    sptr<MultiVpnInfo> vpnInfo = new (std::nothrow) MultiVpnInfo();
    ASSERT_NE(vpnInfo, nullptr);
    l2tpControl->multiVpnInfo_ = vpnInfo;
    EXPECT_EQ(l2tpControl->SetUp(), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(L2tpVpnCtlTest, StartSysVpnTest001, TestSize.Level1)
{
    sptr<L2tpVpnConfig> l2tpVpnconfig = new (std::nothrow) L2tpVpnConfig();
    ASSERT_NE(l2tpVpnconfig, nullptr);
    sptr<INetAddr> netAddr = new (std::nothrow) INetAddr();
    ASSERT_NE(netAddr, nullptr);
    std::string ip = "1.1.1.1";
    netAddr->address_ = ip;
    netAddr->prefixlen_ = 1;
    l2tpVpnconfig->addresses_.push_back(*netAddr);
    l2tpVpnconfig->vpnId_ = "123";
    l2tpVpnconfig->vpnName_ = "testSetUpVpn";
    l2tpVpnconfig->vpnType_ = 1;
    int32_t userId = 0;
    std::vector<int32_t> activeUserIds;
    std::unique_ptr<L2tpVpnCtl> l2tpControl =
        std::make_unique<L2tpVpnCtl>(l2tpVpnconfig, "pkg", userId, activeUserIds);
    ASSERT_NE(l2tpControl, nullptr);
    l2tpControl->l2tpVpnConfig_ = l2tpVpnconfig;
    l2tpControl->l2tpVpnConfig_->vpnType_ = 4;
    EXPECT_EQ(l2tpControl->StartSysVpn(), NETMANAGER_EXT_SUCCESS);
    l2tpControl->l2tpVpnConfig_->vpnType_ = 5;
    EXPECT_EQ(l2tpControl->StartSysVpn(), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(L2tpVpnCtlTest, Destroy001, TestSize.Level1)
{
    if (l2tpControl_ == nullptr) {
        return;
    }
    EXPECT_EQ(l2tpControl_->Destroy(), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(L2tpVpnCtlTest, Destroy002, TestSize.Level1)
{
    sptr<L2tpVpnConfig> l2tpVpnconfig = new (std::nothrow) L2tpVpnConfig();
    ASSERT_NE(l2tpVpnconfig, nullptr);
    int32_t userId = 0;
    std::vector<int32_t> activeUserIds;
    std::unique_ptr<L2tpVpnCtl> l2tpControl =
        std::make_unique<L2tpVpnCtl>(l2tpVpnconfig, "pkg", userId, activeUserIds);
    ASSERT_NE(l2tpControl, nullptr);
    EXPECT_EQ(l2tpControl->Destroy(), NETMANAGER_EXT_SUCCESS);
    l2tpControl->l2tpVpnConfig_->vpnType_ == VpnType::L2TP;
    EXPECT_EQ(l2tpControl->Destroy(), NETMANAGER_EXT_SUCCESS);
    l2tpControl->l2tpVpnConfig_->vpnType_ == VpnType::L2TP_IPSEC_PSK;
    EXPECT_EQ(l2tpControl->Destroy(), NETMANAGER_EXT_SUCCESS);
    sptr<MultiVpnInfo> vpnInfo = new (std::nothrow) MultiVpnInfo();
    ASSERT_NE(vpnInfo, nullptr);
    l2tpControl->multiVpnInfo_ = vpnInfo;
    EXPECT_EQ(l2tpControl->Destroy(), NETMANAGER_EXT_SUCCESS);
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

    l2tpControl_->state_ = IpsecVpnStateCode::STATE_L2TP_STARTED;
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

HWTEST_F(L2tpVpnCtlTest, NotifyConnectStageTest002, TestSize.Level1)
{
    if (l2tpControl_ == nullptr) {
        return;
    }
    std::string stage;
    int32_t errorCode = NETMANAGER_EXT_SUCCESS;
    int32_t ret;

    l2tpControl_->state_ = IpsecVpnStateCode::STATE_STARTED;
    stage = SWANCTL_START_TAG;
    ret = l2tpControl_->NotifyConnectStage(stage, errorCode);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    l2tpControl_->state_ = IpsecVpnStateCode::STATE_CONFIGED;
    stage = IPSEC_CONNECT_TAG;
    ret = l2tpControl_->NotifyConnectStage(stage, errorCode);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    l2tpControl_->state_ = IpsecVpnStateCode::STATE_CONFIGED;
    stage = L2TP_IPSEC_CONFIGURED_TAG;
    sptr<L2tpVpnConfig> l2tpVpnconfig = new (std::nothrow) L2tpVpnConfig();
    ASSERT_NE(l2tpVpnconfig, nullptr);
    l2tpControl_->l2tpVpnConfig_ = l2tpVpnconfig;
    l2tpControl_->l2tpVpnConfig_->vpnType_ = 10;
    sptr<MultiVpnInfo> vpnInfo = new (std::nothrow) MultiVpnInfo();
    ASSERT_NE(vpnInfo, nullptr);
    l2tpControl_->multiVpnInfo_ = vpnInfo;
    ret = l2tpControl_->NotifyConnectStage(stage, errorCode);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    l2tpControl_->state_ = IpsecVpnStateCode::STATE_CONNECTED;
    stage = R"({"updateconfig":{"address":"192.168.1.1", "netmask":"255.255.255.0",
        "mtu":1400, "phyifname":"xfrm"}})";
    ret = l2tpControl_->NotifyConnectStage(stage, errorCode);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(L2tpVpnCtlTest, NotifyConnectStageTest003, TestSize.Level1)
{
    ASSERT_NE(l2tpControl_, nullptr);
    std::string stage;
    int32_t errorCode = NETMANAGER_EXT_SUCCESS;
    l2tpControl_->state_ = IpsecVpnStateCode::STATE_CONFIGED;
    stage = L2TP_IPSEC_CONFIGURED_TAG;
    sptr<L2tpVpnConfig> l2tpVpnconfig = new (std::nothrow) L2tpVpnConfig();
    ASSERT_NE(l2tpVpnconfig, nullptr);
    l2tpControl_->l2tpVpnConfig_ = l2tpVpnconfig;
    l2tpControl_->l2tpVpnConfig_->vpnType_ = 4;
    EXPECT_EQ(l2tpControl_->NotifyConnectStage(stage, errorCode), NETMANAGER_EXT_SUCCESS);
    sptr<MultiVpnInfo> vpnInfo = new (std::nothrow) MultiVpnInfo();
    ASSERT_NE(vpnInfo, nullptr);
    l2tpControl_->multiVpnInfo_ = vpnInfo;
    EXPECT_EQ(l2tpControl_->NotifyConnectStage(stage, errorCode), NETMANAGER_EXT_SUCCESS);

    l2tpControl_->state_ = IpsecVpnStateCode::STATE_L2TP_STARTED;
    stage = IPSEC_CONNECT_TAG;
    EXPECT_EQ(l2tpControl_->NotifyConnectStage(stage, errorCode), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(L2tpVpnCtlTest, NotifyConnectStageTest004, TestSize.Level1)
{
    ASSERT_NE(l2tpControl_, nullptr);
    std::string stage;
    int32_t errorCode = NETMANAGER_EXT_SUCCESS;
    l2tpControl_->state_ = IpsecVpnStateCode::STATE_STARTED;
    stage = SWANCTL_START_TAG;
    sptr<L2tpVpnConfig> l2tpVpnconfig = new (std::nothrow) L2tpVpnConfig();
    ASSERT_NE(l2tpVpnconfig, nullptr);
    l2tpControl_->l2tpVpnConfig_ = l2tpVpnconfig;
    MultiVpnHelper::GetInstance().StartL2tp();
    l2tpControl_->l2tpVpnConfig_->vpnType_ = 4;
    EXPECT_EQ(l2tpControl_->NotifyConnectStage(stage, errorCode), NETMANAGER_EXT_SUCCESS);
    l2tpControl_->l2tpVpnConfig_->vpnType_ = 5;
    EXPECT_EQ(l2tpControl_->NotifyConnectStage(stage, errorCode), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(L2tpVpnCtlTest, NotifyConnectStageTest005, TestSize.Level1)
{
    ASSERT_NE(l2tpControl_, nullptr);
    std::string stage = SWANCTL_START_TAG;
    int32_t errorCode = 1;
    int32_t ret = l2tpControl_->NotifyConnectStage(stage, errorCode);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INTERNAL);
    sptr<MultiVpnInfo> vpnInfo = new (std::nothrow) MultiVpnInfo();
    ASSERT_NE(vpnInfo, nullptr);
    l2tpControl_->multiVpnInfo_ = vpnInfo;
    ret = l2tpControl_->NotifyConnectStage(stage, errorCode);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INTERNAL);
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
    certType = 3;
    EXPECT_EQ(l2tpControl_->GetSysVpnCertUri(certType, certUri), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(L2tpVpnCtlTest, GetVpnCertData001, TestSize.Level1)
{
    sptr<L2tpVpnConfig> config = new (std::nothrow) L2tpVpnConfig();
    if (config == nullptr) {
        return;
    }
    if (l2tpControl_ == nullptr) {
        return;
    }
    config->pkcs12Password_ = "123456";
    l2tpControl_->l2tpVpnConfig_ = config;
    std::vector<int8_t> certData;
    int32_t certType = 1;
    EXPECT_EQ(l2tpControl_->GetVpnCertData(certType, certData), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(certData.size(), 0);
    certType = 3;
    l2tpControl_->l2tpVpnConfig_ = nullptr;
    EXPECT_EQ(l2tpControl_->GetVpnCertData(certType, certData), NETMANAGER_EXT_ERR_INTERNAL);
    std::vector<uint8_t> testCertData{0x30, 0x82, 0x0b, 0xc1, 0x02, 0x01};
    config->pkcs12FileData_ = testCertData;
    l2tpControl_->l2tpVpnConfig_ = config;
    EXPECT_EQ(l2tpControl_->GetVpnCertData(certType, certData), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(certData.size(), 0);
    certType = 6;
    EXPECT_EQ(l2tpControl_->GetVpnCertData(certType, certData), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(certData.size(), testCertData.size());
    certData.clear();
    config->pkcs12FileData_.clear();
    EXPECT_EQ(l2tpControl_->GetVpnCertData(certType, certData), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(certData.size(), 0);
}

HWTEST_F(L2tpVpnCtlTest, GetVpnCertData002, TestSize.Level1)
{
    sptr<L2tpVpnConfig> config = new (std::nothrow) L2tpVpnConfig();
    if (config == nullptr) {
        return;
    }
    if (l2tpControl_ == nullptr) {
        return;
    }
    config->pkcs12Password_ = "123456";
    l2tpControl_->l2tpVpnConfig_ = config;
    std::vector<int8_t> certData;
    int32_t certType = 7;
    EXPECT_EQ(l2tpControl_->GetVpnCertData(certType, certData), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(certData.size(), config->pkcs12Password_.length());
    certType = 3;
    EXPECT_EQ(l2tpControl_->GetVpnCertData(certType, certData), NETMANAGER_EXT_SUCCESS);
    certType = 7;
    config->pkcs12Password_ = "";
    EXPECT_EQ(l2tpControl_->GetVpnCertData(certType, certData), NETMANAGER_EXT_SUCCESS);
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

HWTEST_F(L2tpVpnCtlTest, ProcessUpdateL2tpConfig001, TestSize.Level1)
{
    sptr<L2tpVpnConfig> l2tpVpnconfig = new (std::nothrow) L2tpVpnConfig();
    ASSERT_NE(l2tpVpnconfig, nullptr);
    int32_t userId = 0;
    std::vector<int32_t> activeUserIds;
    std::unique_ptr<L2tpVpnCtl> l2tpControl =
        std::make_unique<L2tpVpnCtl>(l2tpVpnconfig, "pkg", userId, activeUserIds);
    ASSERT_NE(l2tpVpnconfig, nullptr);
    std::string message;
    EXPECT_EQ(l2tpControl->ProcessUpdateConfig(message), NETMANAGER_EXT_ERR_INTERNAL);
    message = R"({"updateconfig":{"address":"192.168.1.1", "netmask":"255.255.255.0",
        "mtu":1400, "phyifname":"xfrm"}})";
    EXPECT_EQ(l2tpControl->ProcessUpdateConfig(message), NETMANAGER_EXT_ERR_INTERNAL);

    l2tpControl->l2tpVpnConfig_ = l2tpVpnconfig;
    EXPECT_EQ(l2tpControl->ProcessUpdateConfig(message), NETMANAGER_EXT_ERR_INTERNAL);
    message =R"({"updateconfig":{"address":"192.168.1.1", "netmask":"255.255.255.0",
        "mtu":1400, "phyifname":"xfrm"}})";
    EXPECT_EQ(l2tpControl->ProcessUpdateConfig(message), NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(L2tpVpnCtlTest, HandleConnectFailed001, TestSize.Level1)
{
    sptr<L2tpVpnConfig> l2tpVpnconfig = new (std::nothrow) L2tpVpnConfig();
    ASSERT_NE(l2tpVpnconfig, nullptr);
    int32_t userId = 0;
    std::vector<int32_t> activeUserIds;
    std::unique_ptr<L2tpVpnCtl> l2tpControl =
        std::make_unique<L2tpVpnCtl>(l2tpVpnconfig, "pkg", userId, activeUserIds);
    ASSERT_NE(l2tpControl, nullptr);
    l2tpControl->state_ = IpsecVpnStateCode::STATE_STARTED;
    l2tpControl->l2tpVpnConfig_ = l2tpVpnconfig;
    sptr<MultiVpnInfo> vpnInfo = new (std::nothrow) MultiVpnInfo();
    ASSERT_NE(vpnInfo, nullptr);
    l2tpControl->multiVpnInfo_ = vpnInfo;
    int32_t code = VpnErrorCode::CONNECT_TIME_OUT;
    l2tpControl->HandleConnectFailed(code);
    EXPECT_TRUE(l2tpControl->state_ == IpsecVpnStateCode::STATE_DISCONNECTED);
    code = VpnErrorCode::IKEV1_KEY_ERROR;
    l2tpControl->state_ = IpsecVpnStateCode::STATE_STARTED;
    l2tpControl->HandleConnectFailed(code);
    EXPECT_TRUE(l2tpControl->state_ ==IpsecVpnStateCode::STATE_DISCONNECTED);
    code = VpnErrorCode::PASSWORD_ERROR;
    l2tpControl->state_ = IpsecVpnStateCode::STATE_STARTED;
    l2tpControl->HandleConnectFailed(code);
    EXPECT_TRUE(l2tpControl->state_ == IpsecVpnStateCode::STATE_DISCONNECTED);
    code = 300;
    l2tpControl->state_ = IpsecVpnStateCode::STATE_STARTED;
    l2tpControl->HandleConnectFailed(code);
    EXPECT_TRUE(l2tpControl->state_ == IpsecVpnStateCode::STATE_DISCONNECTED);
    code = 400;
    l2tpControl->state_ = IpsecVpnStateCode::STATE_DISCONNECTED;
    l2tpControl->HandleConnectFailed(code);
    EXPECT_TRUE(l2tpControl->state_ == IpsecVpnStateCode::STATE_DISCONNECTED);
}

} // namespace NetManagerStandard
} // namespace OHOS