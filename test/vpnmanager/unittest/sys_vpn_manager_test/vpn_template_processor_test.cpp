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

#ifdef GTEST_API_
#define private public
#define protected public
#endif
#include <string>
#include <vector>

#include "inet_addr.h"
#include "ipsecvpn_config.h"
#include "ipsec_vpn_ctl.h"
#include "l2tpvpn_config.h"
#include "l2tp_vpn_ctl.h"
#include "vpn_template_processor.h"
#include "net_manager_constants.h"
#include "multi_vpn_helper.h"
#include "networkvpn_service.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
}

class VpnTemplateProcessorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    static inline auto networkVpnService_ = DelayedSingleton<NetworkVpnService>::GetInstance();
    static inline sptr<SysVpnConfig> vpnConfig_ = nullptr;
    static inline std::string vpnId_ = "test001";
    static inline std::string vpnBundleName_ = "testBundleName";
};

void VpnTemplateProcessorTest::SetUpTestCase() {}

void VpnTemplateProcessorTest::TearDownTestCase()
{
    if (vpnConfig_ == nullptr) {
        return;
    }
    networkVpnService_->DeleteSysVpnConfig(vpnId_);
}
 
HWTEST_F(VpnTemplateProcessorTest, BuildConfig001, TestSize.Level1)
{
    std::shared_ptr<NetVpnImpl> vpnObj = nullptr;
    std::map<std::string, std::shared_ptr<NetVpnImpl>> vpnObjMap;
    VpnTemplateProcessor processor;
    EXPECT_EQ(processor.BuildConfig(vpnObj, vpnObjMap), NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(VpnTemplateProcessorTest, BuildConfig002, TestSize.Level1)
{
    sptr<L2tpVpnConfig> config = new (std::nothrow) L2tpVpnConfig();
    ASSERT_NE(config, nullptr);
    int32_t userId = 0;
    std::vector<int32_t> activeUserIds;
    std::shared_ptr<IpsecVpnCtl> sysVpnCtl = std::make_shared<IpsecVpnCtl>(config, "", userId, activeUserIds);
    ASSERT_NE(sysVpnCtl, nullptr);
    std::shared_ptr<NetVpnImpl> vpnObj = sysVpnCtl;
    std::map<std::string, std::shared_ptr<NetVpnImpl>> vpnObjMap;
    VpnTemplateProcessor processor;
    EXPECT_EQ(processor.BuildConfig(vpnObj, vpnObjMap), NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(VpnTemplateProcessorTest, BuildConfig003, TestSize.Level1)
{
    sptr<IpsecVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(config, nullptr);
    sptr<INetAddr> netAddr = new (std::nothrow) INetAddr();
    ASSERT_NE(netAddr, nullptr);
    std::string ip = "1.1.1.1";
    netAddr->address_ = ip;
    netAddr->prefixlen_ = 1;
    config->addresses_.push_back(*netAddr);
    config->vpnId_ = vpnId_;
    config->vpnName_ = "test001";
    config->vpnType_ = 2;
    int32_t userId = AppExecFwk::Constants::UNSPECIFIED_USERID;
    std::vector<int32_t> activeUserIds;
    networkVpnService_->CheckCurrentAccountType(userId, activeUserIds);
    bool isVpnExtCall = true;
    std::shared_ptr<NetVpnImpl> vpnObj = networkVpnService_->CreateSysVpnCtl(
        config, userId, activeUserIds, isVpnExtCall);
    MultiVpnHelper::GetInstance().CreateMultiVpnInfo(config->vpnId_, config->vpnType_,
         vpnObj->multiVpnInfo_);
    VpnTemplateProcessor processor;
    std::map<std::string, std::shared_ptr<NetVpnImpl>> vpnObjMap;
    EXPECT_EQ(processor.BuildConfig(vpnObj, vpnObjMap), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(VpnTemplateProcessorTest, BuildConfig004, TestSize.Level1)
{
    sptr<L2tpVpnConfig> config = new (std::nothrow) L2tpVpnConfig();
    ASSERT_NE(config, nullptr);
    sptr<INetAddr> netAddr = new (std::nothrow) INetAddr();
    ASSERT_NE(netAddr, nullptr);
    std::string ip = "1.1.1.1";
    netAddr->address_ = ip;
    netAddr->prefixlen_ = 1;
    config->addresses_.push_back(*netAddr);
    config->vpnId_ = vpnId_;
    config->vpnName_ = "test001";
    config->vpnType_ = 4;
    int32_t userId = AppExecFwk::Constants::UNSPECIFIED_USERID;
    std::vector<int32_t> activeUserIds;
    networkVpnService_->CheckCurrentAccountType(userId, activeUserIds);
    bool isVpnExtCall = true;
    std::shared_ptr<NetVpnImpl> vpnObj = networkVpnService_->CreateSysVpnCtl(
        config, userId, activeUserIds, isVpnExtCall);
    MultiVpnHelper::GetInstance().CreateMultiVpnInfo(config->vpnId_, config->vpnType_,
         vpnObj->multiVpnInfo_);
    VpnTemplateProcessor processor;
    std::map<std::string, std::shared_ptr<NetVpnImpl>> vpnObjMap;
    EXPECT_EQ(processor.BuildConfig(vpnObj, vpnObjMap), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(VpnTemplateProcessorTest, BuildConfig005, TestSize.Level1)
{
    sptr<L2tpVpnConfig> config = new (std::nothrow) L2tpVpnConfig();
    ASSERT_NE(config, nullptr);
    config->vpnId_ = vpnId_;
    config->vpnName_ = "test001";
    config->vpnType_ = 4;
    int32_t userId = AppExecFwk::Constants::UNSPECIFIED_USERID;
    std::vector<int32_t> activeUserIds;
    networkVpnService_->CheckCurrentAccountType(userId, activeUserIds);
    bool isVpnExtCall = true;
    std::shared_ptr<NetVpnImpl> vpnObj = networkVpnService_->CreateSysVpnCtl(
        config, userId, activeUserIds, isVpnExtCall);
    std::map<std::string, std::shared_ptr<NetVpnImpl>> vpnObjMap;
    VpnTemplateProcessor processor;
    EXPECT_EQ(processor.BuildConfig(vpnObj, vpnObjMap), NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(VpnTemplateProcessorTest, BuildConfig006, TestSize.Level1)
{
    sptr<L2tpVpnConfig> config = new (std::nothrow) L2tpVpnConfig();
    ASSERT_NE(config, nullptr);
    config->vpnId_ = vpnId_;
    config->vpnName_ = "test001";
    config->vpnType_ = 5;
    sptr<INetAddr> netAddr = new (std::nothrow) INetAddr();
    ASSERT_NE(netAddr, nullptr);
    std::string ip = "1.1.1.1";
    netAddr->address_ = ip;
    netAddr->prefixlen_ = 1;
    config->addresses_.push_back(*netAddr);
    int32_t userId = AppExecFwk::Constants::UNSPECIFIED_USERID;
    std::vector<int32_t> activeUserIds;
    networkVpnService_->CheckCurrentAccountType(userId, activeUserIds);
    bool isVpnExtCall = true;
    std::shared_ptr<NetVpnImpl> vpnObj = networkVpnService_->CreateSysVpnCtl(
        config, userId, activeUserIds, isVpnExtCall);
    MultiVpnHelper::GetInstance().CreateMultiVpnInfo(config->vpnId_, config->vpnType_,
        vpnObj->multiVpnInfo_);
    std::map<std::string, std::shared_ptr<NetVpnImpl>> vpnObjMap;
    VpnTemplateProcessor processor;
    EXPECT_EQ(processor.BuildConfig(vpnObj, vpnObjMap), NETMANAGER_EXT_SUCCESS);
    vpnObjMap.insert({config->vpnId_, vpnObj});

    config->vpnId_ = "test4";
    config->vpnType_ = 4;
    std::shared_ptr<NetVpnImpl> vpnObj4 = networkVpnService_->CreateSysVpnCtl(
        config, userId, activeUserIds, isVpnExtCall);
    MultiVpnHelper::GetInstance().CreateMultiVpnInfo(config->vpnId_, config->vpnType_,
        vpnObj4->multiVpnInfo_);
    EXPECT_EQ(processor.BuildConfig(vpnObj4, vpnObjMap), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(VpnTemplateProcessorTest, BuildConfig007, TestSize.Level1)
{
    sptr<IpsecVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(config, nullptr);
    config->vpnId_ = "test3";
    config->vpnName_ = "test001";
    config->vpnType_ = 3;
    sptr<INetAddr> netAddr = new (std::nothrow) INetAddr();
    ASSERT_NE(netAddr, nullptr);
    std::string ip = "1.1.1.1";
    netAddr->address_ = ip;
    netAddr->prefixlen_ = 1;
    config->addresses_.push_back(*netAddr);
    int32_t userId = AppExecFwk::Constants::UNSPECIFIED_USERID;
    std::vector<int32_t> activeUserIds;
    networkVpnService_->CheckCurrentAccountType(userId, activeUserIds);
    bool isVpnExtCall = true;
    std::shared_ptr<NetVpnImpl> vpnObj3 = networkVpnService_->CreateSysVpnCtl(
        config, userId, activeUserIds, isVpnExtCall);
    MultiVpnHelper::GetInstance().CreateMultiVpnInfo(config->vpnId_, config->vpnType_,
        vpnObj3->multiVpnInfo_);
    std::map<std::string, std::shared_ptr<NetVpnImpl>> vpnObjMap;
    VpnTemplateProcessor processor;
    EXPECT_EQ(processor.BuildConfig(vpnObj3, vpnObjMap), NETMANAGER_EXT_SUCCESS);
    vpnObjMap.insert({config->vpnId_, vpnObj3});

    config->vpnType_ = 6;
    std::shared_ptr<NetVpnImpl> vpnObj6 = networkVpnService_->CreateSysVpnCtl(
        config, userId, activeUserIds, isVpnExtCall);
    MultiVpnHelper::GetInstance().CreateMultiVpnInfo(config->vpnId_, config->vpnType_,
        vpnObj6->multiVpnInfo_);
    EXPECT_EQ(processor.BuildConfig(vpnObj6, vpnObjMap), NETMANAGER_EXT_SUCCESS);

    config->vpnType_ = 7;
    std::shared_ptr<NetVpnImpl> vpnObj7 = networkVpnService_->CreateSysVpnCtl(
        config, userId, activeUserIds, isVpnExtCall);
    MultiVpnHelper::GetInstance().CreateMultiVpnInfo(config->vpnId_, config->vpnType_,
        vpnObj7->multiVpnInfo_);
    EXPECT_EQ(processor.BuildConfig(vpnObj7, vpnObjMap), NETMANAGER_EXT_SUCCESS);

    config->vpnType_ = 8;
    std::shared_ptr<NetVpnImpl> vpnObj8 = networkVpnService_->CreateSysVpnCtl(
        config, userId, activeUserIds, isVpnExtCall);
    MultiVpnHelper::GetInstance().CreateMultiVpnInfo(config->vpnId_, config->vpnType_,
        vpnObj8->multiVpnInfo_);
    EXPECT_EQ(processor.BuildConfig(vpnObj8, vpnObjMap), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(VpnTemplateProcessorTest, BuildConfig008, TestSize.Level1)
{
    sptr<L2tpVpnConfig> config = new (std::nothrow) L2tpVpnConfig();
    ASSERT_NE(config, nullptr);
    int32_t userId = 0;
    std::vector<int32_t> activeUserIds;
    std::shared_ptr<IpsecVpnCtl> sysVpnCtl = std::make_shared<IpsecVpnCtl>(config, "", userId, activeUserIds);
    ASSERT_NE(sysVpnCtl, nullptr);
    sptr<MultiVpnInfo> vpnInfo = new (std::nothrow) MultiVpnInfo();
    ASSERT_NE(vpnInfo, nullptr);
    sysVpnCtl->multiVpnInfo_ = vpnInfo;
    std::shared_ptr<NetVpnImpl> vpnObj = sysVpnCtl;
    std::map<std::string, std::shared_ptr<NetVpnImpl>> vpnObjMap;
    VpnTemplateProcessor processor;
    EXPECT_EQ(processor.BuildConfig(vpnObj, vpnObjMap), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(VpnTemplateProcessorTest, GenXl2tpdConf001, TestSize.Level1)
{
    sptr<L2tpVpnConfig> config = nullptr;
    std::map<std::string, std::shared_ptr<NetVpnImpl>> vpnObjMap;
    VpnTemplateProcessor processor;
    processor.GenXl2tpdConf(config, 1, vpnObjMap);
}

HWTEST_F(VpnTemplateProcessorTest, GenOptionsL2tpdClient001, TestSize.Level1)
{
    VpnTemplateProcessor processor;
    sptr<L2tpVpnConfig> config = nullptr;
    processor.GenOptionsL2tpdClient(config);
}

HWTEST_F(VpnTemplateProcessorTest, CreateXl2tpdConf001, TestSize.Level1)
{
    VpnTemplateProcessor processor;
    sptr<L2tpVpnConfig> config = nullptr;
    std::string conf;
    processor.CreateXl2tpdConf(config, 1, conf);
}

HWTEST_F(VpnTemplateProcessorTest, GetSecret001, TestSize.Level1)
{
    VpnTemplateProcessor processor;
    sptr<IpsecVpnConfig> ipsecConfig = nullptr;
    std::string conf;
    processor.GetSecret(ipsecConfig, 1, conf);
}

HWTEST_F(VpnTemplateProcessorTest, GetConnect001, TestSize.Level1)
{
    VpnTemplateProcessor processor;
    sptr<IpsecVpnConfig> ipsecConfig = nullptr;
    std::string conf;
    processor.GetConnect(ipsecConfig, 1, conf);
    ipsecConfig= new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(ipsecConfig, nullptr);
    processor.GetConnect(ipsecConfig, 1, conf);
    sptr<INetAddr> netAddr = new (std::nothrow) INetAddr();
    ASSERT_NE(netAddr, nullptr);
    std::string ip = "1.1.1.1";
    netAddr->address_ = ip;
    netAddr->prefixlen_ = 1;
    ipsecConfig->addresses_.push_back(*netAddr);
    processor.GetConnect(ipsecConfig, 1, conf);
    ipsecConfig->vpnType_ = -1;
    processor.GetConnect(ipsecConfig, 1, conf);
}

} // namespace NetManagerStandard
} // namespace OHOS