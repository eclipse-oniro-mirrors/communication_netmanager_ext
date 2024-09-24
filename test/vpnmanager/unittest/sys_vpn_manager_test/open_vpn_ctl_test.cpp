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

#include <memory>

#include <gtest/gtest.h>

#ifdef GTEST_API_
#define private public
#endif
#include "net_manager_constants.h"
#include "netmanager_ext_test_security.h"
#include "open_vpn_ctl.h"
#include "vpn_config.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
constexpr const char *OPENVPN_CONFIG =
    "openvpn{\"config\":{\"address\":\"10.8.0.3\", \"netmask\":\"255.255.255.0\", \"mtu\":1500}}";
constexpr const char *OPENVPN_CONFIG_MASK = "openvpn{\"config***";
constexpr const char *OPENVPN_UPDATE = "openvpn{\"updateState\":{\"state\":4}}";
constexpr const char *OPENVPN_UPDATE2 = "openvpn{\"updateState\":{\"state\":203}}";

class OpenvpnCtlTest : public testing::Test {
public:
    static inline std::unique_ptr<OpenvpnCtl> openvpnControl_ = nullptr;
    // static inline std::unique_ptr<NetVpnImpl> netVpnImpl_ = nullptr;
    static void SetUpTestSuite();
};

void OpenvpnCtlTest::SetUpTestSuite()
{
    sptr<OpenvpnConfig> openvpnConfig = new (std::nothrow) OpenvpnConfig();
    ASSERT_NE(openvpnConfig, nullptr);
    int32_t userId = 0;
    std::vector<int32_t> activeUserIds;
    openvpnControl_ =
        std::make_unique<OpenvpnCtl>(openvpnConfig, "pkg", userId, activeUserIds);
    ASSERT_NE(openvpnControl_, nullptr);
    openvpnControl_->openvpnConfig_ = openvpnConfig;
}

HWTEST_F(OpenvpnCtlTest, SetUp001, TestSize.Level1)
{
    int32_t userId = 0;
    std::vector<int32_t> activeUserIds;
    std::unique_ptr<OpenvpnCtl> openvpnCtl =
        std::make_unique<OpenvpnCtl>(nullptr, "pkg", userId, activeUserIds);
    ASSERT_NE(openvpnCtl, nullptr);
    EXPECT_EQ(openvpnCtl->SetUp(), NETMANAGER_EXT_ERR_INTERNAL);
    ASSERT_NE(openvpnControl_, nullptr);
    EXPECT_EQ(openvpnControl_->SetUp(), NETMANAGER_EXT_ERR_INTERNAL);
    NetManagerExtAccessToken accToken;
    EXPECT_EQ(openvpnControl_->SetUp(), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(OpenvpnCtlTest, Destroy001, TestSize.Level1)
{
    ASSERT_NE(openvpnControl_, nullptr);
    EXPECT_EQ(openvpnControl_->Destroy(), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(OpenvpnCtlTest, IsInternalVpn001, TestSize.Level1)
{
    ASSERT_NE(openvpnControl_, nullptr);
    EXPECT_EQ(openvpnControl_->IsInternalVpn(), true);
}

HWTEST_F(OpenvpnCtlTest, GetConnectedSysVpnConfigTest001, TestSize.Level1)
{
    ASSERT_NE(openvpnControl_, nullptr);
    sptr<SysVpnConfig> resConfig = nullptr;
    EXPECT_EQ(openvpnControl_->GetConnectedSysVpnConfig(resConfig), NETMANAGER_EXT_SUCCESS);
    std::string stage = OPENVPN_UPDATE;
    openvpnControl_->NotifyConnectStage(stage, 0);
    EXPECT_EQ(openvpnControl_->GetConnectedSysVpnConfig(resConfig), NETMANAGER_EXT_SUCCESS);
    ASSERT_NE(resConfig, nullptr);
}

HWTEST_F(OpenvpnCtlTest, NotifyConnectStageTest001, TestSize.Level1)
{
    ASSERT_NE(openvpnControl_, nullptr);
    std::string stage = "connect";
    int32_t errorCode = 100;
    EXPECT_EQ(openvpnControl_->NotifyConnectStage(stage, errorCode), NETMANAGER_EXT_ERR_INTERNAL);
    stage = OPENVPN_CONFIG;
    EXPECT_EQ(openvpnControl_->NotifyConnectStage(stage, errorCode), NETMANAGER_EXT_ERR_INTERNAL);
    errorCode = 0;
    EXPECT_EQ(openvpnControl_->NotifyConnectStage(stage, errorCode), NETMANAGER_EXT_SUCCESS);
    stage = "";
    EXPECT_EQ(openvpnControl_->NotifyConnectStage(stage, errorCode), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
}

HWTEST_F(OpenvpnCtlTest, HandleClientMessage001, TestSize.Level1)
{
    ASSERT_NE(openvpnControl_, nullptr);
    std::string msg = "";
    openvpnControl_->HandleClientMessage(msg);
    msg = "openvpn";
    openvpnControl_->HandleClientMessage(msg);
    msg = "openvpn{test}";
    openvpnControl_->HandleClientMessage(msg);
    msg = OPENVPN_CONFIG;
    openvpnControl_->HandleClientMessage(msg);
    EXPECT_EQ(openvpnControl_->openvpnState_, OPENVPN_STATE_CONNECTED);

    msg = OPENVPN_UPDATE;
    openvpnControl_->HandleClientMessage(msg);
    EXPECT_EQ(openvpnControl_->openvpnState_, OPENVPN_STATE_CONNECTED);

    msg = OPENVPN_UPDATE2;
    openvpnControl_->HandleClientMessage(msg);
    EXPECT_EQ(openvpnControl_->openvpnState_, OPENVPN_STATE_DISCONNECTED);
}

HWTEST_F(OpenvpnCtlTest, UpdateState001, TestSize.Level1)
{
    ASSERT_NE(openvpnControl_, nullptr);
    std::string msg = OPENVPN_UPDATE;
    const char *ret = strstr(msg.c_str(), "{");
    ASSERT_TRUE(ret != nullptr);
    cJSON* message = cJSON_Parse(ret);
    ASSERT_TRUE(message != nullptr);
    cJSON* config = cJSON_GetObjectItem(message, "updateState");
    ASSERT_TRUE(config != nullptr);
    ASSERT_TRUE(cJSON_IsObject(config));
    openvpnControl_->UpdateState(config);
    EXPECT_EQ(openvpnControl_->openvpnState_, OPENVPN_STATE_CONNECTED);
}

HWTEST_F(OpenvpnCtlTest, UpdateState002, TestSize.Level1)
{
    ASSERT_NE(openvpnControl_, nullptr);
    std::string msg = OPENVPN_UPDATE2;
    const char *ret = strstr(msg.c_str(), "{");
    ASSERT_TRUE(ret != nullptr);
    cJSON* message = cJSON_Parse(ret);
    ASSERT_TRUE(message != nullptr);
    cJSON* config = cJSON_GetObjectItem(message, "updateState");
    ASSERT_TRUE(config != nullptr);
    ASSERT_TRUE(cJSON_IsObject(config));
    openvpnControl_->UpdateState(config);
    EXPECT_EQ(openvpnControl_->openvpnState_, OPENVPN_STATE_DISCONNECTED);
}

HWTEST_F(OpenvpnCtlTest, UpdateConfig001, TestSize.Level1)
{
    ASSERT_NE(openvpnControl_, nullptr);
    std::string msg = OPENVPN_CONFIG;
    const char *ret = strstr(msg.c_str(), "{");
    ASSERT_TRUE(ret != nullptr);
    cJSON* message = cJSON_Parse(ret);
    ASSERT_TRUE(message != nullptr);
    cJSON* config = cJSON_GetObjectItem(message, "config");
    ASSERT_TRUE(config != nullptr);
    ASSERT_TRUE(cJSON_IsObject(config));
    openvpnControl_->UpdateConfig(config);
    EXPECT_EQ(openvpnControl_->openvpnState_, OPENVPN_STATE_DISCONNECTED);
}

HWTEST_F(OpenvpnCtlTest, UpdateOpenvpnState001, TestSize.Level1)
{
    ASSERT_NE(openvpnControl_, nullptr);
    openvpnControl_->UpdateOpenvpnState(OPENVPN_STATE_CONNECTED);
    EXPECT_EQ(openvpnControl_->openvpnState_, OPENVPN_STATE_CONNECTED);
    openvpnControl_->UpdateOpenvpnState(OPENVPN_STATE_DISCONNECTED);
    EXPECT_EQ(openvpnControl_->openvpnState_, OPENVPN_STATE_DISCONNECTED);
    openvpnControl_->UpdateOpenvpnState(OPENVPN_STATE_ERROR_PRIVATE_KEY);
    EXPECT_EQ(openvpnControl_->openvpnState_, OPENVPN_STATE_ERROR_PRIVATE_KEY);
    openvpnControl_->UpdateOpenvpnState(OPENVPN_STATE_ERROR_CLIENT_CRT);
    EXPECT_EQ(openvpnControl_->openvpnState_, OPENVPN_STATE_ERROR_CLIENT_CRT);
    openvpnControl_->UpdateOpenvpnState(OPENVPN_STATE_ERROR_CA_CAT);
    EXPECT_EQ(openvpnControl_->openvpnState_, OPENVPN_STATE_ERROR_CA_CAT);
    openvpnControl_->UpdateOpenvpnState(OPENVPN_STATE_ERROR_TIME_OUT);
    EXPECT_EQ(openvpnControl_->openvpnState_, OPENVPN_STATE_ERROR_TIME_OUT);
    openvpnControl_->UpdateOpenvpnState(-1);
    EXPECT_EQ(openvpnControl_->openvpnState_, -1);
}

HWTEST_F(OpenvpnCtlTest, StopOpenvpn001, TestSize.Level1)
{
    ASSERT_NE(openvpnControl_, nullptr);
    openvpnControl_->StopOpenvpn();
    EXPECT_EQ(openvpnControl_->openvpnState_, OPENVPN_STATE_DISCONNECTED);
}

HWTEST_F(OpenvpnCtlTest, GetConnectedSysVpnConfig001, TestSize.Level1)
{
    ASSERT_NE(openvpnControl_, nullptr);
    openvpnControl_->UpdateOpenvpnState(OPENVPN_STATE_CONNECTED);
    sptr<SysVpnConfig> sysVpnConfig = new (std::nothrow) SysVpnConfig();
    ASSERT_NE(sysVpnConfig, nullptr);
    EXPECT_EQ(openvpnControl_->GetConnectedSysVpnConfig(sysVpnConfig), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(OpenvpnCtlTest, IsSystemVpn001, TestSize.Level1)
{
    ASSERT_NE(openvpnControl_, nullptr);
    EXPECT_EQ(openvpnControl_->IsSystemVpn(), true);
}

HWTEST_F(OpenvpnCtlTest, MaskOpenvpnMessage001, TestSize.Level1)
{
    ASSERT_NE(openvpnControl_, nullptr);
    EXPECT_EQ(openvpnControl_->MaskOpenvpnMessage(OPENVPN_UPDATE), OPENVPN_UPDATE);
    EXPECT_EQ(openvpnControl_->MaskOpenvpnMessage(OPENVPN_CONFIG), OPENVPN_CONFIG_MASK);
}
} // namespace NetManagerStandard
} // namespace OHOS