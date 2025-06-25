/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "multi_vpn_helper.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;

class MultiVpnHelperTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
public:
    static inline auto &multiVpnHelper_ = MultiVpnHelper::GetInstance();
};

void MultiVpnHelperTest::SetUpTestCase() {}

void MultiVpnHelperTest::TearDownTestCase() {}

void MultiVpnHelperTest::SetUp() {}

void MultiVpnHelperTest::TearDown() {}

HWTEST_F(MultiVpnHelperTest, GetNewIfNameId001, TestSize.Level1)
{
    multiVpnHelper_.multiVpnInfos_.clear();
    EXPECT_EQ(multiVpnHelper_.GetNewIfNameId(), 1);
    sptr<MultiVpnInfo> info = new (std::nothrow) MultiVpnInfo();
    ASSERT_NE(info, nullptr);
    multiVpnHelper_.multiVpnInfos_.emplace_back(info);
    EXPECT_EQ(multiVpnHelper_.GetNewIfNameId(), 1);
    multiVpnHelper_.multiVpnInfos_.clear();
    info->ifNameId = 1;
    multiVpnHelper_.multiVpnInfos_.emplace_back(info);
    EXPECT_EQ(multiVpnHelper_.GetNewIfNameId(), 2);
}

HWTEST_F(MultiVpnHelperTest, CreateMultiVpnInfo001, TestSize.Level1)
{
    sptr<SysVpnConfig> vpnConfig = nullptr;
    sptr<MultiVpnInfo> info = nullptr;
    std::string bundleName = "test";
    int32_t userId = 1;
    bool isVpnExtCall = true;
    EXPECT_EQ(multiVpnHelper_.CreateMultiVpnInfo(vpnConfig, info, bundleName, userId, isVpnExtCall),
        NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(MultiVpnHelperTest, CreateMultiVpnInfo002, TestSize.Level1)
{
    sptr<SysVpnConfig> vpnConfig = new (std::nothrow) SysVpnConfig();
    ASSERT_NE(vpnConfig, nullptr);
    sptr<MultiVpnInfo> info = nullptr;
    std::string bundleName = "test";
    int32_t userId = 1;
    bool isVpnExtCall = true;
    for (size_t i = 1; i < 21; ++i) {
        sptr<MultiVpnInfo> vpnInfo = new (std::nothrow) MultiVpnInfo();
        ASSERT_NE(vpnInfo, nullptr);;
        vpnInfo->ifNameId = i;
        multiVpnHelper_.multiVpnInfos_.emplace_back(vpnInfo);
    }
    EXPECT_EQ(multiVpnHelper_.CreateMultiVpnInfo(vpnConfig, info, bundleName, userId, isVpnExtCall),
        NETMANAGER_EXT_ERR_INTERNAL);
    multiVpnHelper_.multiVpnInfos_.clear();
}

HWTEST_F(MultiVpnHelperTest, CreateMultiVpnInfo003, TestSize.Level1)
{
    multiVpnHelper_.multiVpnInfos_.clear();
    sptr<MultiVpnInfo> info = nullptr;
    std::string bundleName = "test";
    int32_t userId = 1;
    bool isVpnExtCall = true;
    sptr<MultiVpnInfo> vpnInfo = new (std::nothrow) MultiVpnInfo();
    vpnInfo->ifNameId = 1;
    multiVpnHelper_.multiVpnInfos_.emplace_back(vpnInfo);
    sptr<SysVpnConfig> vpnConfig = new (std::nothrow) SysVpnConfig();
    ASSERT_NE(vpnConfig, nullptr);
    vpnConfig->vpnType_ = VpnType::IKEV2_IPSEC_MSCHAPv2;
    EXPECT_EQ(multiVpnHelper_.CreateMultiVpnInfo(vpnConfig, info, bundleName, userId, isVpnExtCall),
        NETMANAGER_EXT_SUCCESS);
    vpnConfig->vpnType_ = VpnType::IKEV2_IPSEC_PSK;
    EXPECT_EQ(multiVpnHelper_.CreateMultiVpnInfo(vpnConfig, info, bundleName, userId, isVpnExtCall),
        NETMANAGER_EXT_SUCCESS);
    vpnConfig->vpnType_ = VpnType::IKEV2_IPSEC_RSA;
    EXPECT_EQ(multiVpnHelper_.CreateMultiVpnInfo(vpnConfig, info, bundleName, userId, isVpnExtCall),
        NETMANAGER_EXT_SUCCESS);
    vpnConfig->vpnType_ = VpnType::IPSEC_XAUTH_PSK;
    EXPECT_EQ(multiVpnHelper_.CreateMultiVpnInfo(vpnConfig, info, bundleName, userId, isVpnExtCall),
        NETMANAGER_EXT_SUCCESS);
    vpnConfig->vpnType_ = VpnType::IPSEC_XAUTH_RSA;
    EXPECT_EQ(multiVpnHelper_.CreateMultiVpnInfo(vpnConfig, info, bundleName, userId, isVpnExtCall),
        NETMANAGER_EXT_SUCCESS);
    vpnConfig->vpnType_ = VpnType::IPSEC_HYBRID_RSA;
    EXPECT_EQ(multiVpnHelper_.CreateMultiVpnInfo(vpnConfig, info, bundleName, userId, isVpnExtCall),
        NETMANAGER_EXT_SUCCESS);
    vpnConfig->vpnType_ = VpnType::OPENVPN;
    EXPECT_EQ(multiVpnHelper_.CreateMultiVpnInfo(vpnConfig, info, bundleName, userId, isVpnExtCall),
        NETMANAGER_EXT_SUCCESS);
    vpnConfig->vpnType_ = VpnType::L2TP_IPSEC_PSK;
    EXPECT_EQ(multiVpnHelper_.CreateMultiVpnInfo(vpnConfig, info, bundleName, userId, isVpnExtCall),
        NETMANAGER_EXT_SUCCESS);
    vpnConfig->vpnType_ = VpnType::L2TP_IPSEC_RSA;
    EXPECT_EQ(multiVpnHelper_.CreateMultiVpnInfo(vpnConfig, info, bundleName, userId, isVpnExtCall),
        NETMANAGER_EXT_SUCCESS);
    vpnConfig->vpnType_ = VpnType::L2TP;
    EXPECT_EQ(multiVpnHelper_.CreateMultiVpnInfo(vpnConfig, info, bundleName, userId, isVpnExtCall),
        NETMANAGER_EXT_SUCCESS);
    multiVpnHelper_.multiVpnInfos_.clear();
}

HWTEST_F(MultiVpnHelperTest, AddMultiVpnInfo001, TestSize.Level1)
{
    multiVpnHelper_.multiVpnInfos_.clear();
    sptr<MultiVpnInfo> info = nullptr;
    EXPECT_EQ(multiVpnHelper_.AddMultiVpnInfo(info), NETMANAGER_EXT_ERR_INTERNAL);
    info = new (std::nothrow) MultiVpnInfo();
    ASSERT_NE(info, nullptr);
    EXPECT_EQ(multiVpnHelper_.AddMultiVpnInfo(info), NETMANAGER_EXT_ERR_INTERNAL);
    info->ifName = "xfrm-vpn1";
    EXPECT_EQ(multiVpnHelper_.AddMultiVpnInfo(info), NETMANAGER_EXT_SUCCESS);
    sptr<MultiVpnInfo> info1 = new (std::nothrow) MultiVpnInfo();
    ASSERT_NE(info1, nullptr);
    info1->ifName = "xfrm-vpn2";
    EXPECT_EQ(multiVpnHelper_.AddMultiVpnInfo(info1), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(multiVpnHelper_.AddMultiVpnInfo(info), NETMANAGER_EXT_SUCCESS);
    multiVpnHelper_.multiVpnInfos_.clear();
}

HWTEST_F(MultiVpnHelperTest, DelMultiVpnInfo001, TestSize.Level1)
{
    multiVpnHelper_.multiVpnInfos_.clear();
    sptr<MultiVpnInfo> info = nullptr;
    EXPECT_EQ(multiVpnHelper_.DelMultiVpnInfo(info), NETMANAGER_EXT_ERR_INTERNAL);
    info = new (std::nothrow) MultiVpnInfo();
    ASSERT_NE(info, nullptr);
    EXPECT_EQ(multiVpnHelper_.DelMultiVpnInfo(info), NETMANAGER_EXT_SUCCESS);
    info->ifName = "xfrm-vpn1";
    EXPECT_EQ(multiVpnHelper_.AddMultiVpnInfo(info), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(multiVpnHelper_.DelMultiVpnInfo(info), NETMANAGER_EXT_SUCCESS);
    multiVpnHelper_.multiVpnInfos_.clear();
}

HWTEST_F(MultiVpnHelperTest, StartIpsec001, TestSize.Level1)
{
    multiVpnHelper_.ipsecStartedCount_ = -1;
    EXPECT_EQ(multiVpnHelper_.StartIpsec(), true);
    multiVpnHelper_.ipsecStartedCount_ = 0;
    EXPECT_EQ(multiVpnHelper_.StartIpsec(), true);
    multiVpnHelper_.ipsecStartedCount_ = 1;
    EXPECT_EQ(multiVpnHelper_.StartIpsec(), false);
}

HWTEST_F(MultiVpnHelperTest, StopIpsec001, TestSize.Level1)
{
    multiVpnHelper_.ipsecStartedCount_ = 2;
    multiVpnHelper_.StopIpsec();
    EXPECT_EQ(multiVpnHelper_.ipsecStartedCount_, 1);
}

HWTEST_F(MultiVpnHelperTest, StartL2tp001, TestSize.Level1)
{
    multiVpnHelper_.l2tpStartedCount_ = -1;
    EXPECT_EQ(multiVpnHelper_.StartL2tp(), true);
    multiVpnHelper_.l2tpStartedCount_ = 0;
    EXPECT_EQ(multiVpnHelper_.StartL2tp(), true);
    multiVpnHelper_.l2tpStartedCount_ = 1;
    EXPECT_EQ(multiVpnHelper_.StartL2tp(), false);
}

HWTEST_F(MultiVpnHelperTest, StopL2tp001, TestSize.Level1)
{
    multiVpnHelper_.l2tpStartedCount_ = 2;
    multiVpnHelper_.StopL2tp();
    EXPECT_EQ(multiVpnHelper_.l2tpStartedCount_, 1);
}

HWTEST_F(MultiVpnHelperTest, IsConnectedStage001, TestSize.Level1)
{
    std::string stage;
    EXPECT_EQ(multiVpnHelper_.IsConnectedStage(stage), false);
    stage = "test";
    EXPECT_EQ(multiVpnHelper_.IsConnectedStage(stage), false);
    stage = "connect";
    EXPECT_EQ(multiVpnHelper_.IsConnectedStage(stage), true);
    stage = "pppdstart";
    EXPECT_EQ(multiVpnHelper_.IsConnectedStage(stage), true);
    stage ="openvpn{\"updateState\":{\"state\":4}}";
    EXPECT_EQ(multiVpnHelper_.IsConnectedStage(stage), true);
}

HWTEST_F(MultiVpnHelperTest, IsOpenvpnConnectedStage001, TestSize.Level1)
{
    std::string stage;
    EXPECT_EQ(multiVpnHelper_.IsOpenvpnConnectedStage(stage), false);
    stage = "test";
    EXPECT_EQ(multiVpnHelper_.IsOpenvpnConnectedStage(stage), false);
    stage = "openvpn";
    EXPECT_EQ(multiVpnHelper_.IsOpenvpnConnectedStage(stage), false);
    stage = "openvpn{}";
    EXPECT_EQ(multiVpnHelper_.IsOpenvpnConnectedStage(stage), false);
    stage ="openvpn{\"config\":{\"state\":4}}";
    EXPECT_EQ(multiVpnHelper_.IsOpenvpnConnectedStage(stage), false);
    stage ="openvpn{\"updateState\":{}}";
    EXPECT_EQ(multiVpnHelper_.IsOpenvpnConnectedStage(stage), false);
    stage ="openvpn{\"updateState\":{\"test\":4}}";
    EXPECT_EQ(multiVpnHelper_.IsOpenvpnConnectedStage(stage), false);
    stage ="openvpn{\"updateState\":{\"state\":5}}";
    EXPECT_EQ(multiVpnHelper_.IsOpenvpnConnectedStage(stage), false);
    stage ="openvpn{\"updateState\":{\"state\":4}}";
    EXPECT_EQ(multiVpnHelper_.IsOpenvpnConnectedStage(stage), true);
}

HWTEST_F(MultiVpnHelperTest, IsAnyVpnConnecting001, TestSize.Level1)
{
    multiVpnHelper_.multiVpnInfos_.clear();
    sptr<MultiVpnInfo> vpnInfo = new (std::nothrow) MultiVpnInfo();
    ASSERT_NE(vpnInfo, nullptr);
    vpnInfo->ifNameId = 1;
    multiVpnHelper_.multiVpnInfos_.emplace_back(vpnInfo);
    EXPECT_EQ(multiVpnHelper_.IsAnyVpnConnecting(), false);
    sptr<MultiVpnInfo> vpnInfo1 = new (std::nothrow) MultiVpnInfo();
    ASSERT_NE(vpnInfo1, nullptr);
    vpnInfo1->isConnecting = true;
    multiVpnHelper_.multiVpnInfos_.emplace_back(vpnInfo1);
    EXPECT_EQ(multiVpnHelper_.IsAnyVpnConnecting(), true);
    multiVpnHelper_.multiVpnInfos_.clear();
}
}
}