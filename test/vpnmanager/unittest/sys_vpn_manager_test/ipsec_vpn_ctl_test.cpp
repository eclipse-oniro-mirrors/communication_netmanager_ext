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
} // namespace NetManagerStandard
} // namespace OHOS