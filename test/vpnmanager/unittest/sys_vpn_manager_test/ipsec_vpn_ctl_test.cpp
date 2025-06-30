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

#include <memory>

#include <gtest/gtest.h>

#include "net_manager_constants.h"
#include "vpn_config.h"

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "ipsec_vpn_ctl.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;

class IpsecVpnCtlTest : public testing::Test {
public:
    static inline std::unique_ptr<IpsecVpnCtl> ipsecControl_ = nullptr;
    static void SetUpTestSuite();
};

void IpsecVpnCtlTest::SetUpTestSuite()
{
    sptr<IpsecVpnConfig> ipsecConfig = new (std::nothrow) IpsecVpnConfig();
    if (ipsecConfig == nullptr) {
        return;
    }
    int32_t userId = 0;
    std::vector<int32_t> activeUserIds;
    ipsecControl_ = std::make_unique<IpsecVpnCtl>(ipsecConfig, "pkg", userId, activeUserIds);
    if (ipsecControl_ == nullptr) {
        return;
    }
    ipsecControl_->ipsecVpnConfig_ = ipsecConfig;
}

HWTEST_F(IpsecVpnCtlTest, SetUp001, TestSize.Level1)
{
    if (ipsecControl_ == nullptr) {
        return;
    }
    EXPECT_EQ(ipsecControl_->SetUp(), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(IpsecVpnCtlTest, Destroy001, TestSize.Level1)
{
    if (ipsecControl_ == nullptr) {
        return;
    }
    EXPECT_EQ(ipsecControl_->Destroy(), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(IpsecVpnCtlTest, IsInternalVpn001, TestSize.Level1)
{
    if (ipsecControl_ == nullptr) {
        return;
    }
    EXPECT_EQ(ipsecControl_->IsInternalVpn(), true);
}

HWTEST_F(IpsecVpnCtlTest, GetConnectedSysVpnConfigTest001, TestSize.Level1)
{
    if (ipsecControl_ == nullptr) {
        return;
    }
    sptr<SysVpnConfig> resConfig = nullptr;
    int32_t ret = ipsecControl_->GetConnectedSysVpnConfig(resConfig);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    ipsecControl_->state_ = IpsecVpnStateCode::STATE_CONNECTED;
    ret = ipsecControl_->GetConnectedSysVpnConfig(resConfig);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    sptr<IpsecVpnConfig> tmp = ipsecControl_->ipsecVpnConfig_;
    ipsecControl_->ipsecVpnConfig_ = nullptr;
    ret = ipsecControl_->GetConnectedSysVpnConfig(resConfig);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    ipsecControl_->ipsecVpnConfig_ = tmp;
}

HWTEST_F(IpsecVpnCtlTest, NotifyConnectStageTest001, TestSize.Level1)
{
    if (ipsecControl_ == nullptr) {
        return;
    }
    std::string stage;
    int32_t errorCode = 1;
    int32_t ret = ipsecControl_->NotifyConnectStage(stage, errorCode);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PARAMETER_ERROR);

    stage = IPSEC_START_TAG;
    ret = ipsecControl_->NotifyConnectStage(stage, errorCode);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INTERNAL);

    errorCode = NETMANAGER_EXT_SUCCESS;
    ipsecControl_->state_ = IpsecVpnStateCode::STATE_INIT;
    ret = ipsecControl_->NotifyConnectStage(stage, errorCode);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    ipsecControl_->state_ = IpsecVpnStateCode::STATE_STARTED;
    stage = SWANCTL_START_TAG;
    ret = ipsecControl_->NotifyConnectStage(stage, errorCode);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    ipsecControl_->state_ = IpsecVpnStateCode::STATE_CONFIGED;
    stage = IPSEC_CONNECT_TAG;
    ret = ipsecControl_->NotifyConnectStage(stage, errorCode);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    ipsecControl_->state_ = IpsecVpnStateCode::STATE_DISCONNECTED;
    ret = ipsecControl_->NotifyConnectStage(stage, errorCode);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INTERNAL);

    stage = "stageTest";
    ipsecControl_->state_ = IpsecVpnStateCode::STATE_INIT;
    EXPECT_EQ(ipsecControl_->NotifyConnectStage(stage, errorCode), NETMANAGER_EXT_SUCCESS);
    ipsecControl_->state_ = IpsecVpnStateCode::STATE_STARTED;
    EXPECT_EQ(ipsecControl_->NotifyConnectStage(stage, errorCode), NETMANAGER_EXT_SUCCESS);
    ipsecControl_->state_ = IpsecVpnStateCode::STATE_CONFIGED;
    EXPECT_EQ(ipsecControl_->NotifyConnectStage(stage, errorCode), NETMANAGER_EXT_SUCCESS);
    ipsecControl_->state_ = IpsecVpnStateCode::STATE_CONFIGED;
    stage ="{\"updateconfig\":{\"test\":\"192.168.1.1\"}}";
    EXPECT_EQ(ipsecControl_->NotifyConnectStage(stage, errorCode), NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(IpsecVpnCtlTest, NotifyConnectStageTest002, TestSize.Level1)
{
    ASSERT_NE(ipsecControl_, nullptr);
    std::string stage;
    int32_t errorCode = 0;
    int32_t ret = 0;
    ipsecControl_->state_ = IpsecVpnStateCode::STATE_STARTED;
    stage = SWANCTL_START_TAG;
    sptr<MultiVpnInfo> vpnInfo = new (std::nothrow) MultiVpnInfo();
    ASSERT_NE(vpnInfo, nullptr);
    ipsecControl_->multiVpnInfo_ = vpnInfo;
    ret = ipsecControl_->NotifyConnectStage(stage, errorCode);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    ipsecControl_->multiVpnInfo_->ifNameId = 1;
    ret = ipsecControl_->NotifyConnectStage(stage, errorCode);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(IpsecVpnCtlTest, GetSysVpnCertUriTest001, TestSize.Level1)
{
    sptr<IpsecVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    if (config == nullptr) {
        return;
    }
    if (ipsecControl_ == nullptr) {
        return;
    }
    config->ipsecCaCertConf_ = "testCaUri";
    ipsecControl_->ipsecVpnConfig_ = nullptr;
    int32_t certType = 0;
    std::string certUri;
    EXPECT_EQ(ipsecControl_->GetSysVpnCertUri(certType, certUri), NETMANAGER_EXT_ERR_INTERNAL);
    ipsecControl_->ipsecVpnConfig_ = config;
    EXPECT_EQ(ipsecControl_->GetSysVpnCertUri(certType, certUri), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ("testCaUri", certUri);
}

HWTEST_F(IpsecVpnCtlTest, GetSysVpnCertUriTest002, TestSize.Level1)
{
    sptr<IpsecVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    if (config == nullptr) {
        return;
    }
    if (ipsecControl_ == nullptr) {
        return;
    }
    config->ipsecPublicUserCertConf_ = "testUserUri";
    ipsecControl_->ipsecVpnConfig_ = config;
    std::string certUri;
    int32_t certType = 1;
    EXPECT_EQ(ipsecControl_->GetSysVpnCertUri(certType, certUri), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ("testUserUri", certUri);
    certType = 2;
    EXPECT_EQ(ipsecControl_->GetSysVpnCertUri(certType, certUri), NETMANAGER_EXT_SUCCESS);
    certType = -1;
    EXPECT_EQ(ipsecControl_->GetSysVpnCertUri(certType, certUri), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(IpsecVpnCtlTest, GetSysVpnCertUriTest003, TestSize.Level1)
{
    sptr<IpsecVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    if (config == nullptr) {
        return;
    }
    if (ipsecControl_ == nullptr) {
        return;
    }
    config->ipsecPublicUserCertConf_ = "testUserUri";
    ipsecControl_->ipsecVpnConfig_ = config;
    std::string certUri;
    int32_t certType = 1;
    EXPECT_EQ(ipsecControl_->GetSysVpnCertUri(certType, certUri), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ("testUserUri", certUri);
    certType = 3;
    EXPECT_EQ(ipsecControl_->GetSysVpnCertUri(certType, certUri), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(IpsecVpnCtlTest, GetVpnCertData001, TestSize.Level1)
{
    sptr<IpsecVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    if (config == nullptr) {
        return;
    }
    if (ipsecControl_ == nullptr) {
        return;
    }
    config->pkcs12Password_ = "123456";
    ipsecControl_->ipsecVpnConfig_ = config;
    std::vector<int8_t> certData;
    int32_t certType = 1;
    EXPECT_EQ(ipsecControl_->GetVpnCertData(certType, certData), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(certData.size(), 0);
    certType = 3;
    ipsecControl_->ipsecVpnConfig_ = nullptr;
    EXPECT_EQ(ipsecControl_->GetVpnCertData(certType, certData), NETMANAGER_EXT_ERR_INTERNAL);
    std::vector<uint8_t> testCertData{0x30, 0x82, 0x0b, 0xc1, 0x02, 0x01};
    config->pkcs12FileData_ = testCertData;
    ipsecControl_->ipsecVpnConfig_ = config;
    EXPECT_EQ(ipsecControl_->GetVpnCertData(certType, certData), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(certData.size(), 0);
    certType = 6;
    EXPECT_EQ(ipsecControl_->GetVpnCertData(certType, certData), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(certData.size(), testCertData.size());
    certData.clear();
    config->pkcs12FileData_.clear();
    EXPECT_EQ(ipsecControl_->GetVpnCertData(certType, certData), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(certData.size(), 0);
}

HWTEST_F(IpsecVpnCtlTest, GetVpnCertData002, TestSize.Level1)
{
    sptr<IpsecVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    if (config == nullptr) {
        return;
    }
    if (ipsecControl_ == nullptr) {
        return;
    }
    config->pkcs12Password_ = "123456";
    ipsecControl_->ipsecVpnConfig_ = config;
    std::vector<int8_t> certData;
    int32_t certType = 7;
    EXPECT_EQ(ipsecControl_->GetVpnCertData(certType, certData), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(certData.size(), config->pkcs12Password_.length());
    certType = 3;
    EXPECT_EQ(ipsecControl_->GetVpnCertData(certType, certData), NETMANAGER_EXT_SUCCESS);
    certType = 7;
    config->pkcs12Password_ = "";
    EXPECT_EQ(ipsecControl_->GetVpnCertData(certType, certData), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(IpsecVpnCtlTest, InitConfigFileTest001, TestSize.Level1)
{
    sptr<IpsecVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    if (config == nullptr) {
        return;
    }
    if (ipsecControl_ == nullptr) {
        return;
    }
    config->strongswanConf_ = "INVALID_BASE64";
    ipsecControl_->ipsecVpnConfig_ = config;
    EXPECT_EQ(ipsecControl_->InitConfigFile(), NETMANAGER_EXT_SUCCESS);
    config->strongswanConf_ = "SGVsbG8sIFdvcmxkIQ==";
    ipsecControl_->ipsecVpnConfig_ = config;
    EXPECT_EQ(ipsecControl_->InitConfigFile(), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(IpsecVpnCtlTest, InitConfigFileTest002, TestSize.Level1)
{
    if (ipsecControl_ == nullptr) {
        return;
    }
    ipsecControl_->ipsecVpnConfig_ = nullptr;
    EXPECT_EQ(ipsecControl_->InitConfigFile(), NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(IpsecVpnCtlTest, UpdateConfigTest001, TestSize.Level1)
{
    if (ipsecControl_ == nullptr) {
        return;
    }
    ipsecControl_->ipsecVpnConfig_ = nullptr;
    std::string message;
    EXPECT_EQ(ipsecControl_->UpdateConfig(message), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    message = "test";
    EXPECT_EQ(ipsecControl_->UpdateConfig(message), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    message = "updateconfig";
    EXPECT_EQ(ipsecControl_->UpdateConfig(message), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    message = "updateconfig{}";
    EXPECT_EQ(ipsecControl_->UpdateConfig(message), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    message = "{\"updateconfig\"}";
    EXPECT_EQ(ipsecControl_->UpdateConfig(message), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    message = R"({"config":{"address":"192.168.1.1", "netmask":"255.255.255.0",
        "mtu":1400, "phyifname":"xfrm"}})";
    EXPECT_EQ(ipsecControl_->UpdateConfig(message), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    message = "{\"updateconfig\":{\"test\":\"192.168.1.1\"}}";
    EXPECT_EQ(ipsecControl_->UpdateConfig(message), NETMANAGER_EXT_SUCCESS);

    sptr<IpsecVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    if (config == nullptr) {
        return;
    }
    ipsecControl_->ipsecVpnConfig_ = config;
    message = R"({"updateconfig":{"address":"192.168.1.1", "netmask":"255.255.255.0",
        "mtu":1400, "phyifname":"xfrm"}})";
    EXPECT_EQ(ipsecControl_->UpdateConfig(message), NETMANAGER_EXT_SUCCESS);
    message = R"({"updateconfig":{"remoteip":"192.168.1.1","address":"192.168.1.1",
        "netmask":"255.255.255.0", "mtu":1400, "phyifname":"xfrm"}})";
    EXPECT_EQ(ipsecControl_->UpdateConfig(message), NETMANAGER_EXT_SUCCESS);
    ipsecControl_->vpnConfig_ = nullptr;
    message = R"({"updateconfig":{"address":"192.168.1.1", "netmask":"255.255.255.0",
        "mtu":1400, "phyifname":"xfrm"}})";
    EXPECT_EQ(ipsecControl_->UpdateConfig(message), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(IpsecVpnCtlTest, UpdateConfigTest002, TestSize.Level1)
{
    if (ipsecControl_ == nullptr) {
        return;
    }
    sptr<IpsecVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    if (config == nullptr) {
        return;
    }
    std::string message;
    ipsecControl_->ipsecVpnConfig_ = config;

    INetAddr netAddr;
    netAddr.address_ = "10.1.0.12";
    sptr<VpnConfig> vpnConfig = new (std::nothrow) VpnConfig();
    if (vpnConfig == nullptr) {
        return;
    }
    message = R"({"updateconfig":{"remoteip":"192.168.1.21","address":"10.1.0.12",
        "netmask":"255.255.255.0", "mtu":1400, "phyifname":"xfrm"}})";
    EXPECT_EQ(ipsecControl_->UpdateConfig(message), NETMANAGER_EXT_SUCCESS);
    ipsecControl_->vpnConfig_ = vpnConfig;
    ipsecControl_->vpnConfig_->addresses_.push_back(netAddr);
    EXPECT_EQ(ipsecControl_->UpdateConfig(message), NETMANAGER_EXT_SUCCESS);
    MultiVpnHelper::GetInstance().CreateMultiVpnInfo(
        "1000", VpnType::L2TP_IPSEC_PSK, ipsecControl_->multiVpnInfo_);
    ASSERT_NE(ipsecControl_->multiVpnInfo_, nullptr);
    ipsecControl_->multiVpnInfo_->localAddress = "10.1.0.12";
    MultiVpnHelper::GetInstance().AddMultiVpnInfo(ipsecControl_->multiVpnInfo_);
    EXPECT_EQ(ipsecControl_->UpdateConfig(message), NETMANAGER_EXT_ERR_INTERNAL);
    ipsecControl_->vpnConfig_ = nullptr;
    EXPECT_EQ(ipsecControl_->UpdateConfig(message), NETMANAGER_EXT_SUCCESS);
    ipsecControl_->multiVpnInfo_ = nullptr;
    EXPECT_EQ(ipsecControl_->UpdateConfig(message), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(IpsecVpnCtlTest, HandleUpdateConfigTest001, TestSize.Level1)
{
    if (ipsecControl_ == nullptr) {
        return;
    }
    ipsecControl_->ipsecVpnConfig_ = nullptr;
    std::string message;
    EXPECT_EQ(ipsecControl_->HandleUpdateConfig(message), NETMANAGER_EXT_ERR_INTERNAL);
    message = R"({"updateconfig":{"address":"192.168.1.1", "netmask":"255.255.255.0",
        "mtu":1400, "phyifname":"xfrm"}})";
    EXPECT_EQ(ipsecControl_->HandleUpdateConfig(message), NETMANAGER_EXT_ERR_INTERNAL);

    sptr<IpsecVpnConfig> ipsecConfig = new (std::nothrow) IpsecVpnConfig();
    if (ipsecConfig == nullptr) {
        return;
    }
    int32_t userId = 0;
    std::vector<int32_t> activeUserIds;
    ipsecControl_ = std::make_unique<IpsecVpnCtl>(ipsecConfig, "pkg", userId, activeUserIds);
    ipsecControl_->ipsecVpnConfig_ = ipsecConfig;
    message = R"({"updateconfig":{"address":"192.168.1.1", "netmask":"255.255.255.0",
        "mtu":1400, "phyifname":"xfrm"}})";
    EXPECT_EQ(ipsecControl_->HandleUpdateConfig(message), NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(IpsecVpnCtlTest, StartSysVpnTest001, TestSize.Level1)
{
    sptr<IpsecVpnConfig> ipsecConfig = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(ipsecConfig, nullptr);
    int32_t userId = 0;
    std::vector<int32_t> activeUserIds;
    std::unique_ptr<IpsecVpnCtl> ipsecControl1
        = std::make_unique<IpsecVpnCtl>(ipsecConfig, "pkg", userId, activeUserIds);
    ASSERT_NE(ipsecControl1, nullptr);
    EXPECT_EQ(ipsecControl1->SetUp(), NETMANAGER_EXT_SUCCESS);
    std::unique_ptr<IpsecVpnCtl> ipsecControl2
        = std::make_unique<IpsecVpnCtl>(ipsecConfig, "pkg", userId, activeUserIds);
        ASSERT_NE(ipsecControl2, nullptr);
    EXPECT_EQ(ipsecControl2->SetUp(), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(IpsecVpnCtlTest, DestroyTest002, TestSize.Level1)
{
    sptr<IpsecVpnConfig> ipsecConfig = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(ipsecConfig, nullptr);
    int32_t userId = 0;
    std::vector<int32_t> activeUserIds;

    std::unique_ptr<IpsecVpnCtl> ipsecControl
        = std::make_unique<IpsecVpnCtl>(ipsecConfig, "pkg", userId, activeUserIds);
    ASSERT_NE(ipsecControl, nullptr);
    EXPECT_EQ(ipsecControl->Destroy(), NETMANAGER_EXT_SUCCESS);

    std::unique_ptr<IpsecVpnCtl> ipsecControl1
        = std::make_unique<IpsecVpnCtl>(ipsecConfig, "pkg", userId, activeUserIds);
    ASSERT_NE(ipsecControl1, nullptr);
    sptr<MultiVpnInfo> vpnInfo = new (std::nothrow) MultiVpnInfo();
    ASSERT_NE(vpnInfo, nullptr);
    ipsecControl1->multiVpnInfo_ = vpnInfo;
    ipsecControl1->multiVpnInfo_->isVpnExtCall = false;
    EXPECT_EQ(ipsecControl1->Destroy(), NETMANAGER_EXT_SUCCESS);
    std::unique_ptr<IpsecVpnCtl> ipsecControl2
        = std::make_unique<IpsecVpnCtl>(ipsecConfig, "pkg", userId, activeUserIds);
    ASSERT_NE(ipsecControl2, nullptr);
    vpnInfo->isVpnExtCall = 1;
    ipsecControl1->multiVpnInfo_->isVpnExtCall = true;
    ipsecControl1->multiVpnInfo_->ifNameId = 1;
    EXPECT_EQ(ipsecControl2->Destroy(), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(IpsecVpnCtlTest, SetUpVpnTunTest001, TestSize.Level1)
{
    sptr<MultiVpnInfo> vpnInfo = new (std::nothrow) MultiVpnInfo();
    ASSERT_NE(vpnInfo, nullptr);
    vpnInfo->isVpnExtCall = 0;
    vpnInfo->ifNameId = 1;
    sptr<IpsecVpnConfig> ipsecConfig = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(ipsecConfig, nullptr);
    int32_t userId = 0;
    std::vector<int32_t> activeUserIds;
    std::unique_ptr<IpsecVpnCtl> ipsecControl
        = std::make_unique<IpsecVpnCtl>(ipsecConfig, "pkg", userId, activeUserIds);
    ASSERT_NE(ipsecControl, nullptr);
    ipsecControl->multiVpnInfo_ = vpnInfo;
    ipsecControl->multiVpnInfo_->isVpnExtCall = true;
    EXPECT_NE(ipsecControl->SetUpVpnTun(), NETMANAGER_EXT_SUCCESS);
    ipsecControl->multiVpnInfo_->isVpnExtCall = false;
    EXPECT_NE(ipsecControl->SetUpVpnTun(), NETMANAGER_EXT_SUCCESS);
    ipsecConfig->vpnId_ = "123";
    ipsecConfig->vpnName_ = "xfrm-vpn2";
    ipsecConfig->vpnType_ = 2;
    INetAddr netAddr;
    netAddr.address_ = "10.1.0.12";
    ipsecConfig->addresses_.push_back(netAddr);
    ipsecControl->multiVpnInfo_->isVpnExtCall = true;
    EXPECT_NE(ipsecControl->SetUpVpnTun(), NETMANAGER_EXT_SUCCESS);
}

} // namespace NetManagerStandard
} // namespace OHOS