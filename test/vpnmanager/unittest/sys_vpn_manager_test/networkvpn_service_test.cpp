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

#include "nativetoken_kit.h"
#include "accesstoken_kit.h"
#include "token_setproc.h"

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "ipsecvpn_config.h"
#include "net_manager_constants.h"
#include "networkvpn_service.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
} // namespace
class NetworkVpnServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();

    static inline auto instance_ = &NetworkVpnService::GetInstance();
    static inline sptr<SysVpnConfig> vpnConfig_ = nullptr;
    static inline std::string vpnId_ = "test001";
};

void NetworkVpnServiceTest::SetUpTestCase()
{
    vpnConfig_ = new (std::nothrow) IpsecVpnConfig();
    if (vpnConfig_ == nullptr) {
        return;
    }
    vpnConfig_->vpnId_ = vpnId_;
    vpnConfig_->vpnName_ = vpnId_;
    vpnConfig_->vpnType_ = 1;
}

void NetworkVpnServiceTest::TearDownTestCase()
{
    if (vpnConfig_ == nullptr) {
        return;
    }
    instance_->DeleteSysVpnConfig(vpnId_);
}

HWTEST_F(NetworkVpnServiceTest, AddSysVpnConfigTest001, TestSize.Level1)
{
    if (vpnConfig_ == nullptr) {
        return;
    }
    EXPECT_EQ(instance_->AddSysVpnConfig(vpnConfig_), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, AddSysVpnConfigTest002, TestSize.Level1)
{
    sptr<SysVpnConfig> config = nullptr;
    EXPECT_EQ(instance_->AddSysVpnConfig(config), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetworkVpnServiceTest, DeleteSysVpnConfigTest001, TestSize.Level1)
{
    std::string id = "1234";
    sptr<SysVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    if (config == nullptr) {
        return;
    }
    config->vpnId_ = id;
    config->vpnName_ = "test";
    config->vpnType_ = 1;
    EXPECT_EQ(instance_->AddSysVpnConfig(config), NETMANAGER_EXT_SUCCESS);

    // delete test config
    EXPECT_EQ(instance_->DeleteSysVpnConfig(id), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, DeleteSysVpnConfigTest002, TestSize.Level1)
{
    std::string id;
    EXPECT_EQ(instance_->DeleteSysVpnConfig(id), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetworkVpnServiceTest, GetSysVpnConfigList001, TestSize.Level1)
{
    std::vector<SysVpnConfig> list;
    EXPECT_EQ(instance_->GetSysVpnConfigList(list), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, GetSysVpnConfigTest001, TestSize.Level1)
{
    if (vpnConfig_ == nullptr) {
        return;
    }

    // vpnConfig_ is "test001"
    instance_->AddSysVpnConfig(vpnConfig_);
    sptr<SysVpnConfig> resultConfig = nullptr;
    EXPECT_EQ(instance_->GetSysVpnConfig(resultConfig, vpnId_), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, GetSysVpnConfigTest002, TestSize.Level1)
{
    std::string id;
    sptr<SysVpnConfig> resultConfig = nullptr;
    EXPECT_EQ(instance_->GetSysVpnConfig(resultConfig, id), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetworkVpnServiceTest, GetConnectedSysVpnConfigTest001, TestSize.Level1)
{
    sptr<SysVpnConfig> resultConfig = nullptr;
    EXPECT_EQ(instance_->GetConnectedSysVpnConfig(resultConfig), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, NotifyConnectStageTest001, TestSize.Level1)
{
    std::string stage = "connect";
    int32_t code = 100;
    EXPECT_EQ(instance_->NotifyConnectStage(stage, code), NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL);
}
} // namespace NetManagerStandard
} // namespace OHOS
