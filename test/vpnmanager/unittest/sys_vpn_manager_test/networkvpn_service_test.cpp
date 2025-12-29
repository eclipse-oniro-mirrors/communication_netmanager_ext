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

#include "accesstoken_kit.h"
#include "extended_vpn_ctl.h"
#include "ipsecvpn_config.h"
#include "openvpn_config.h"
#include "l2tpvpn_config.h"
#include "nativetoken_kit.h"
#include "net_manager_constants.h"
#include "system_ability_definition.h"
#include "token_setproc.h"
#include "parameters.h"

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "networkvpn_service.h"
#include "netmanager_ext_test_security.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
    constexpr int32_t TEST_USERID = 0;
} // namespace
class IVpnEventCallbackTest : public IRemoteStub<IVpnEventCallback> {
public:
    int32_t OnVpnStateChanged(bool isConnected, const std::string &vpnIfName, const std::string &vpnIfAddr,
                              const std::string &vpnId, bool isGlobalVpn) override{ return 0; };
    int32_t OnMultiVpnStateChanged(bool isConnected, const std::string &bundleName,
        const std::string &vpnId) override{ return 0; };
    int32_t OnVpnMultiUserSetUp() override{ return 0; };
};
class NetworkVpnServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();

    static inline auto instance_ = DelayedSingleton<NetworkVpnService>::GetInstance();
    static inline sptr<SysVpnConfig> vpnConfig_ = nullptr;
    static inline std::string vpnId_ = "test001";
};

void NetworkVpnServiceTest::SetUpTestCase()
{
    vpnConfig_ = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(vpnConfig_, nullptr);
    vpnConfig_->vpnId_ = vpnId_;
    vpnConfig_->vpnName_ = vpnId_;
    vpnConfig_->vpnType_ = 1;
}

void NetworkVpnServiceTest::TearDownTestCase()
{
    ASSERT_NE(vpnConfig_, nullptr);
    instance_->DeleteSysVpnConfig(vpnId_);
}

HWTEST_F(NetworkVpnServiceTest, OnStart001, TestSize.Level1)
{
    instance_->state_ = NetworkVpnService::ServiceRunningState::STATE_RUNNING;
    instance_->OnStart();

    instance_->state_ = NetworkVpnService::ServiceRunningState::STATE_STOPPED;
    instance_->OnStart();

    EXPECT_EQ(instance_->state_, NetworkVpnService::ServiceRunningState::STATE_STOPPED);
}

HWTEST_F(NetworkVpnServiceTest, OnStop001, TestSize.Level1)
{
    instance_->OnStop();
    EXPECT_EQ(instance_->state_, NetworkVpnService::ServiceRunningState::STATE_STOPPED);
}

HWTEST_F(NetworkVpnServiceTest, AddSysVpnConfigTest001, TestSize.Level1)
{
    ASSERT_NE(vpnConfig_, nullptr);
    auto ret = instance_->AddSysVpnConfig(vpnConfig_);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, AddSysVpnConfigTest002, TestSize.Level1)
{
    sptr<SysVpnConfig> config = nullptr;
    auto ret = instance_->AddSysVpnConfig(config);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetworkVpnServiceTest, DeleteSysVpnConfigTest001, TestSize.Level1)
{
    std::string id = "1234";
    sptr<SysVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(config, nullptr);
    config->vpnId_ = id;
    config->vpnName_ = "test";
    config->vpnType_ = 1;
    auto ret = instance_->AddSysVpnConfig(config);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);

    // delete test config
    auto ret2 = instance_->DeleteSysVpnConfig(id);
    EXPECT_EQ(ret2, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, DeleteSysVpnConfigTest002, TestSize.Level1)
{
    std::string id;
    auto ret = instance_->DeleteSysVpnConfig(id);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetworkVpnServiceTest, GetSysVpnConfigList001, TestSize.Level1)
{
    std::vector<sptr<SysVpnConfig>> list;
    auto ret = instance_->GetSysVpnConfigList(list);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
    sptr<SysVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(config, nullptr);
    config->vpnId_ = "testId1";
    config->vpnName_ = "testName1";
    config->vpnType_ = 1;
    instance_->AddSysVpnConfig(config);
    config->vpnId_ = "testId2";
    config->vpnName_ = "testName2";
    instance_->AddSysVpnConfig(config);
    ret = instance_->GetSysVpnConfigList(list);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    auto ret2 = instance_->DeleteSysVpnConfig("testId1");
    EXPECT_EQ(ret2, NETMANAGER_EXT_SUCCESS);
    auto ret3 = instance_->DeleteSysVpnConfig("testId2");
    EXPECT_EQ(ret3, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, GetSysVpnConfigTest001, TestSize.Level1)
{
    ASSERT_NE(vpnConfig_, nullptr);

    // vpnConfig_ is "test001"
    instance_->AddSysVpnConfig(vpnConfig_);
    sptr<SysVpnConfig> resultConfig = new (std::nothrow) IpsecVpnConfig();
    auto ret = instance_->GetSysVpnConfig(resultConfig, vpnId_);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, GetSysVpnConfigTest002, TestSize.Level1)
{
    std::string id;
    sptr<SysVpnConfig> resultConfig = new (std::nothrow) IpsecVpnConfig();
    auto ret = instance_->GetSysVpnConfig(resultConfig, id);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetworkVpnServiceTest, GetConnectedSysVpnConfigTest001, TestSize.Level1)
{
    sptr<SysVpnConfig> resultConfig = new (std::nothrow) IpsecVpnConfig();
    auto ret = instance_->GetConnectedSysVpnConfig(resultConfig);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    instance_->AddSysVpnConfig(vpnConfig_);
    instance_->SetUpSysVpn(vpnConfig_);
    auto ret2 = instance_->GetConnectedSysVpnConfig(vpnConfig_);
    EXPECT_EQ(ret2, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, NotifyConnectStageTest001, TestSize.Level1)
{
    std::string stage = "connect";
    int32_t code = 100;
    EXPECT_EQ(instance_->NotifyConnectStage(stage, code), NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL);
}

HWTEST_F(NetworkVpnServiceTest, GetSysVpnCertUriTest001, TestSize.Level1)
{
    std::string certUri;
    int32_t certType = 0;
    EXPECT_EQ(instance_->GetSysVpnCertUri(certType, certUri), NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL);
}

HWTEST_F(NetworkVpnServiceTest, GetVpnCertData001, TestSize.Level1)
{
    std::vector<int8_t> certData;
    ASSERT_NE(instance_, nullptr);
    EXPECT_EQ(instance_->GetVpnCertData(6, certData), NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL);
    NetManagerExtAccessToken access;
    EXPECT_EQ(instance_->GetVpnCertData(6, certData), NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL);
}

HWTEST_F(NetworkVpnServiceTest, QueryVpnData001, TestSize.Level1)
{
    sptr<SysVpnConfig> openvpnConfig = nullptr;
    sptr<VpnDataBean> vpnBean = nullptr;
    EXPECT_EQ(instance_->QueryVpnData(openvpnConfig, vpnBean), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    openvpnConfig = new (std::nothrow) OpenvpnConfig();
    EXPECT_EQ(instance_->QueryVpnData(openvpnConfig, vpnBean), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    vpnBean = new (std::nothrow) VpnDataBean();
    ASSERT_NE(openvpnConfig, nullptr);
    ASSERT_NE(vpnBean, nullptr);
    EXPECT_EQ(instance_->QueryVpnData(openvpnConfig, vpnBean), NETMANAGER_EXT_ERR_INVALID_PARAMETER);
    openvpnConfig->vpnId_ = "test001";
    auto ret = instance_->QueryVpnData(openvpnConfig, vpnBean);
    EXPECT_TRUE(ret == NETMANAGER_EXT_SUCCESS || ret == NETMANAGER_EXT_ERR_OPERATION_FAILED);
}

HWTEST_F(NetworkVpnServiceTest, SetUpSysVpn001, TestSize.Level1)
{
    sptr<SysVpnConfig> config = nullptr;
    int32_t ret = instance_->SetUpSysVpn(config, false);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PARAMETER_ERROR);

    config = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(config, nullptr);
    config->vpnId_ = "123";
    config->vpnName_ = "testSetUpVpn";
    config->vpnType_ = 1;
    ret = instance_->SetUpSysVpn(config, false);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INTERNAL);

    NetManagerExtAccessToken access;
    std::shared_ptr<NetVpnImpl> tmp = instance_->vpnObj_;
    instance_->vpnObj_ = nullptr;
    std::string pkg = "test1";
    int32_t userId = AppExecFwk::Constants::UNSPECIFIED_USERID;
    std::vector<int32_t> activeUserIds;
    instance_->CheckCurrentAccountType(userId, activeUserIds);
    instance_->vpnObj_ = std::make_shared<ExtendedVpnCtl>(config, pkg, userId, activeUserIds);
    ret = instance_->SetUpSysVpn(config, false);
    EXPECT_TRUE(ret == NETWORKVPN_ERROR_VPN_EXIST || ret == NETMANAGER_EXT_ERR_INTERNAL);
    instance_->vpnObj_ = nullptr;
    instance_->vpnObj_ = std::make_shared<ExtendedVpnCtl>(config, pkg, 100, activeUserIds);
    ret = instance_->SetUpSysVpn(config, false);
    EXPECT_TRUE(ret == NETWORKVPN_ERROR_VPN_EXIST || ret == NETMANAGER_EXT_ERR_INTERNAL);
    instance_->vpnObj_ = tmp;
}

HWTEST_F(NetworkVpnServiceTest, SetUpSysVpn002, TestSize.Level1)
{
    sptr<SysVpnConfig> config = nullptr;
    int32_t ret = instance_->SetUpSysVpn(config, true);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PARAMETER_ERROR);

    config = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(config, nullptr);
    sptr<INetAddr> netAddr = new (std::nothrow) INetAddr();
    ASSERT_NE(netAddr, nullptr);
    std::string ip = "1.1.1.1";
    netAddr->address_ = ip;
    netAddr->prefixlen_ = 1;
    config->addresses_.push_back(*netAddr);
    config->vpnId_ = "123";
    config->vpnName_ = "testSetUpVpn";
    config->vpnType_ = 1;
    ret = instance_->SetUpSysVpn(config, true);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    NetManagerExtAccessToken access;
    std::shared_ptr<NetVpnImpl> tmp = instance_->vpnObj_;
    instance_->vpnObj_ = nullptr;
    std::string pkg = "test1";
    int32_t userId = AppExecFwk::Constants::UNSPECIFIED_USERID;
    std::vector<int32_t> activeUserIds;
    instance_->CheckCurrentAccountType(userId, activeUserIds);
    instance_->vpnObj_ = std::make_shared<ExtendedVpnCtl>(config, pkg, userId, activeUserIds);
    ret = instance_->SetUpSysVpn(config, true);
    EXPECT_TRUE(ret == NETWORKVPN_ERROR_VPN_EXIST || ret == NETMANAGER_EXT_SUCCESS);
    instance_->vpnObj_ = nullptr;
    instance_->vpnObj_ = std::make_shared<ExtendedVpnCtl>(config, pkg, 100, activeUserIds);
    ret = instance_->SetUpSysVpn(config, true);
    EXPECT_TRUE(ret == NETWORKVPN_ERROR_VPN_EXIST || ret == NETMANAGER_EXT_SUCCESS);
    instance_->vpnObj_ = tmp;
}

HWTEST_F(NetworkVpnServiceTest, SetUpSysVpn003, TestSize.Level1)
{
    system::SetParameter("persist.edm.vpn_disable", "true");
    sptr<SysVpnConfig> config = nullptr;
    int32_t ret = instance_->SetUpSysVpn(config, true);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    ret = instance_->SetUpSysVpn(config, false);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    system::SetParameter("persist.edm.vpn_disable", "false");
}

HWTEST_F(NetworkVpnServiceTest, SetUpVpn001, TestSize.Level1)
{
    sptr<SysVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(config, nullptr);
    config->vpnId_ = "123";
    int32_t ret = instance_->SetUpVpn(*config, true);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INTERNAL);

    NetManagerExtAccessToken accToken;
    std::string pkg = "test1";
    int32_t userId = AppExecFwk::Constants::UNSPECIFIED_USERID;
    std::vector<int32_t> activeUserIds;
    instance_->CheckCurrentAccountType(userId, activeUserIds);
    instance_->vpnObj_ = std::make_shared<ExtendedVpnCtl>(config, pkg, userId, activeUserIds);
    ret = instance_->SetUpVpn(*config, true);
    EXPECT_NE(ret, NETMANAGER_EXT_ERR_INTERNAL);

    instance_->vpnObj_ = nullptr;
    ret = instance_->SetUpVpn(*config, true);
    EXPECT_NE(ret, NETMANAGER_EXT_ERR_INTERNAL);

    ret = instance_->SetUpVpn(*config, true);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(NetworkVpnServiceTest, DumpTest001, TestSize.Level1)
{
    int32_t fd = 0;
    std::vector<std::u16string> args;
    int32_t ret = instance_->Dump(fd, args);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INTERNAL);

    fd = 1;
    ret = instance_->Dump(fd, args);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, CreateSysVpnCtl001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    sptr<SysVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(config, nullptr);
    int32_t userId = 0;
    std::vector<int32_t> activeUserIds;
    std::shared_ptr<NetVpnImpl> sysVpnCtl = nullptr;
    config->vpnId_ = "1234";
    config->vpnName_ = "test001";
    config->vpnType_ = 1;
    auto ret = instance_->AddSysVpnConfig(config);
    EXPECT_TRUE(ret == NETMANAGER_EXT_SUCCESS || ret == NETMANAGER_EXT_ERR_OPERATION_FAILED);
    sysVpnCtl = instance_->CreateSysVpnCtl(config, userId, activeUserIds, false);
    EXPECT_FALSE(sysVpnCtl != nullptr);
}

HWTEST_F(NetworkVpnServiceTest, CreateSysVpnCtl002, TestSize.Level1)
{
    sptr<SysVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(config, nullptr);
    int32_t userId = 0;
    std::vector<int32_t> activeUserIds;
    std::shared_ptr<NetVpnImpl> sysVpnCtl = nullptr;
    sptr<INetAddr> netAddr = new (std::nothrow) INetAddr();
    ASSERT_NE(netAddr, nullptr);
    std::string ip = "1.1.1.1";
    netAddr->address_ = ip;
    netAddr->prefixlen_ = 1;
    config->addresses_.push_back(*netAddr);
    config->vpnId_ = "1234";
    config->vpnName_ = "test001";
    config->vpnType_ = 1;
    sysVpnCtl = instance_->CreateSysVpnCtl(config, userId, activeUserIds, true);
    EXPECT_TRUE(sysVpnCtl != nullptr);
}

HWTEST_F(NetworkVpnServiceTest, CreateSysVpnCtl003, TestSize.Level1)
{
    sptr<SysVpnConfig> config = new (std::nothrow) L2tpVpnConfig();
    ASSERT_NE(config, nullptr);
    int32_t userId = 0;
    std::vector<int32_t> activeUserIds;
    std::shared_ptr<NetVpnImpl> sysVpnCtl = nullptr;
    sptr<INetAddr> netAddr = new (std::nothrow) INetAddr();
    ASSERT_NE(netAddr, nullptr);
    std::string ip = "1.1.1.1";
    netAddr->address_ = ip;
    netAddr->prefixlen_ = 1;
    config->addresses_.push_back(*netAddr);
    config->vpnId_ = "1234";
    config->vpnName_ = "test001";
    config->vpnType_ = 4;
    sysVpnCtl = instance_->CreateSysVpnCtl(config, userId, activeUserIds, true);
    EXPECT_TRUE(sysVpnCtl != nullptr);
}

HWTEST_F(NetworkVpnServiceTest, CreateSysVpnCtl004, TestSize.Level1)
{
    sptr<SysVpnConfig> config = new (std::nothrow) OpenvpnConfig();
    ASSERT_NE(config, nullptr);
    int32_t userId = 0;
    std::vector<int32_t> activeUserIds;
    std::shared_ptr<NetVpnImpl> sysVpnCtl = nullptr;
    sptr<INetAddr> netAddr = new (std::nothrow) INetAddr();
    ASSERT_NE(netAddr, nullptr);
    std::string ip = "1.1.1.1";
    netAddr->address_ = ip;
    netAddr->prefixlen_ = 1;
    config->addresses_.push_back(*netAddr);
    config->vpnId_ = "1234";
    config->vpnName_ = "test001";
    config->vpnType_ = 9;
    sysVpnCtl = instance_->CreateSysVpnCtl(config, userId, activeUserIds, true);
    EXPECT_TRUE(sysVpnCtl != nullptr);
}

HWTEST_F(NetworkVpnServiceTest, CreateSysVpnCtl005, TestSize.Level1)
{
    sptr<SysVpnConfig> config = new (std::nothrow) OpenvpnConfig();
    ASSERT_NE(config, nullptr);
    int32_t userId = 0;
    std::vector<int32_t> activeUserIds;
    std::shared_ptr<NetVpnImpl> sysVpnCtl = nullptr;
    sptr<INetAddr> netAddr = new (std::nothrow) INetAddr();
    ASSERT_NE(netAddr, nullptr);
    std::string ip = "1.1.1.1";
    netAddr->address_ = ip;
    netAddr->prefixlen_ = 1;
    config->addresses_.push_back(*netAddr);
    config->vpnId_ = "1234";
    config->vpnName_ = "test001";
    config->vpnType_ = 12;
    sysVpnCtl = instance_->CreateSysVpnCtl(config, userId, activeUserIds, true);
    EXPECT_TRUE(sysVpnCtl != nullptr);
    
    sysVpnCtl = instance_->CreateSysVpnCtl(config, userId, activeUserIds, false);
    EXPECT_TRUE(sysVpnCtl == nullptr);
}

HWTEST_F(NetworkVpnServiceTest, Init001, TestSize.Level1)
{
    EXPECT_EQ(instance_->Init(), false);
}

HWTEST_F(NetworkVpnServiceTest, PublishVpnConnectionStateEvent001, TestSize.Level1)
{
    VpnConnectState state = VpnConnectState::VPN_CONNECTED;
    EXPECT_NE(instance_, nullptr);
    instance_->PublishVpnConnectionStateEvent(state);
}

HWTEST_F(NetworkVpnServiceTest, Prepare001, TestSize.Level1)
{
    bool isExistVpn = false;
    bool isRun = false;
    std::string pkg = "";
    auto ret = instance_->Prepare(isExistVpn, isRun, pkg);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, DestroyVpn001, TestSize.Level1)
{
    bool isVpnExtCall = false;
    int32_t ret = instance_->DestroyVpn(isVpnExtCall);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    NetManagerExtAccessToken access;
    ret = instance_->DestroyVpn(isVpnExtCall);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, DestroyVpn002, TestSize.Level1)
{
    std::string vpnId = "";
    int32_t ret = instance_->DestroyVpn(vpnId);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_INVALID);
    vpnId = "testVpnId";
    ret = instance_->DestroyVpn(vpnId);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    NetManagerExtAccessToken access;
    ret = instance_->DestroyVpn(vpnId);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, DestroyMultiVpn001, TestSize.Level1)
{
    std::string vpnId = "testVpnId";
    NetManagerExtAccessToken access;
    sptr<SysVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(config, nullptr);
    int32_t userId = 0;
    std::vector<int32_t> activeUserIds;
    std::shared_ptr<NetVpnImpl> sysVpnCtl = nullptr;
    config->vpnId_ = vpnId;
    config->vpnName_ = "test001";
    config->vpnType_ = 1;
    sysVpnCtl = instance_->CreateSysVpnCtl(config, userId, activeUserIds, true);
    ASSERT_NE(sysVpnCtl, nullptr);
    instance_->vpnObjMap_.insert({vpnId, sysVpnCtl});
    int32_t ret = instance_->DestroyMultiVpn(0);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INTERNAL);
    sptr<MultiVpnInfo> multiVpnInterface = new (std::nothrow) MultiVpnInfo();
    ASSERT_NE(multiVpnInterface, nullptr);
    multiVpnInterface->vpnId = vpnId;
    multiVpnInterface->userId = userId;
    multiVpnInterface->callingUid = 100;
    sysVpnCtl->multiVpnInfo_ = multiVpnInterface;
    instance_->vpnObjMap_.insert({vpnId, sysVpnCtl});
    ret = instance_->DestroyMultiVpn(100);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, DestroyMultiVpn002, TestSize.Level1)
{
    int32_t ret = instance_->DestroyMultiVpn(nullptr);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INTERNAL);

    std::string vpnId = "testVpnId";
    NetManagerExtAccessToken access;
    sptr<SysVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(config, nullptr);
    int32_t userId = 0;
    std::vector<int32_t> activeUserIds;
    std::shared_ptr<NetVpnImpl> sysVpnCtl = nullptr;
    config->vpnId_ = vpnId;
    config->vpnName_ = "test001";
    config->vpnType_ = 1;
    sysVpnCtl = instance_->CreateSysVpnCtl(config, userId, activeUserIds, true);
    ASSERT_NE(sysVpnCtl, nullptr);
    instance_->vpnObjMap_.insert({vpnId, sysVpnCtl});
    instance_->vpnObj_ = sysVpnCtl;
    ret = instance_->DestroyMultiVpn(sysVpnCtl, true);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INTERNAL);

    sptr<MultiVpnInfo> multiVpnInterface = new (std::nothrow) MultiVpnInfo();
    ASSERT_NE(multiVpnInterface, nullptr);
    multiVpnInterface->vpnId = vpnId;
    multiVpnInterface->userId = userId;
    sysVpnCtl->multiVpnInfo_ = multiVpnInterface;
    ret = instance_->DestroyMultiVpn(sysVpnCtl, true);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, GetConnectedVpnAppInfo001, TestSize.Level1)
{
    std::vector<std::string> appInfos;
    int32_t ret = instance_->GetConnectedVpnAppInfo(appInfos);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    NetManagerExtAccessToken access;
    ret = instance_->GetConnectedVpnAppInfo(appInfos);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, RegisterVpnEvent001, TestSize.Level1)
{
    sptr<IVpnEventCallback> callback = new (std::nothrow) IVpnEventCallbackTest();
    auto ret = instance_->RegisterVpnEvent(callback);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_OPERATION_FAILED);
}

HWTEST_F(NetworkVpnServiceTest, UnregisterVpnEvent001, TestSize.Level1)
{
    sptr<IVpnEventCallback> callback = new (std::nothrow) IVpnEventCallbackTest();
    auto ret = instance_->UnregisterVpnEvent(callback);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_OPERATION_FAILED);
}

HWTEST_F(NetworkVpnServiceTest, RegisterMultiVpnEvent001, TestSize.Level1)
{
    sptr<IVpnEventCallback> callback = new (std::nothrow) IVpnEventCallbackTest();
    ASSERT_NE(callback, nullptr);
    EXPECT_EQ(instance_->RegisterMultiVpnEvent(callback), NETMANAGER_ERR_SYSTEM_INTERNAL);
    NetManagerExtAccessToken access;
    EXPECT_EQ(instance_->RegisterMultiVpnEvent(callback), NETMANAGER_ERR_SYSTEM_INTERNAL);
}

HWTEST_F(NetworkVpnServiceTest, UnregisterMultiVpnEvent001, TestSize.Level1)
{
    sptr<IVpnEventCallback> callback = new (std::nothrow) IVpnEventCallbackTest();
    ASSERT_NE(callback, nullptr);
    EXPECT_EQ(instance_->UnregisterMultiVpnEvent(callback), NETMANAGER_ERR_SYSTEM_INTERNAL);
    NetManagerExtAccessToken access;
    EXPECT_EQ(instance_->UnregisterMultiVpnEvent(callback), NETMANAGER_ERR_SYSTEM_INTERNAL);
}

HWTEST_F(NetworkVpnServiceTest, CreateVpnConnection001, TestSize.Level1)
{
    EXPECT_EQ(instance_->CreateVpnConnection(true), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, CheckCurrentAccountType001, TestSize.Level1)
{
    int32_t userId = 1;
    std::vector<int32_t> activeUserIds;
    EXPECT_EQ(instance_->CheckCurrentAccountType(userId, activeUserIds), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, SyncRegisterVpnEvent001, TestSize.Level1)
{
    sptr<IVpnEventCallback> callback = new (std::nothrow) IVpnEventCallbackTest();

    int32_t ret = instance_->SyncRegisterVpnEvent(callback);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);

    instance_->vpnEventCallbacks_.push_back(callback);
    ret = instance_->SyncRegisterVpnEvent(callback);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_OPERATION_FAILED);
}

HWTEST_F(NetworkVpnServiceTest, SyncUnregisterVpnEvent001, TestSize.Level1)
{
    sptr<IVpnEventCallback> callback = new (std::nothrow) IVpnEventCallbackTest();
    int32_t ret = instance_->SyncUnregisterVpnEvent(callback);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_OPERATION_FAILED);

    instance_->vpnEventCallbacks_.push_back(callback);
    ret = instance_->SyncUnregisterVpnEvent(callback);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, SyncRegisterMultiVpnEvent001, TestSize.Level1)
{
    sptr<IVpnEventCallback> callback = new (std::nothrow) IVpnEventCallbackTest();
    ASSERT_NE(callback, nullptr);
    std::string pkg = "";
    int32_t ret = instance_->SyncRegisterMultiVpnEvent(callback, pkg);
    EXPECT_EQ(ret, NETMANAGER_ERR_SYSTEM_INTERNAL);
    pkg = "com.vpn.test";
    ret = instance_->SyncRegisterMultiVpnEvent(callback, pkg);
    EXPECT_EQ(ret, NETMANAGER_ERR_SYSTEM_INTERNAL);

    sptr<NetworkVpnService::MultiVpnEventCallback> multiVpnEventCallback =
        new (std::nothrow) NetworkVpnService::MultiVpnEventCallback();
    ASSERT_NE(multiVpnEventCallback, nullptr);
    multiVpnEventCallback->userId = 100;
    multiVpnEventCallback->bundleName = "com.vpn.test";
    multiVpnEventCallback->callback = callback;
    instance_->multiVpnEventCallbacks_.push_back(multiVpnEventCallback);
    ret = instance_->SyncRegisterMultiVpnEvent(callback, pkg);
    EXPECT_EQ(ret, NETMANAGER_ERR_SYSTEM_INTERNAL);
}

HWTEST_F(NetworkVpnServiceTest, SyncUnregisterMultiVpnEvent001, TestSize.Level1)
{
    sptr<IVpnEventCallback> callback = new (std::nothrow) IVpnEventCallbackTest();
    ASSERT_NE(callback, nullptr);
    int32_t ret = instance_->SyncUnregisterMultiVpnEvent(callback);
    EXPECT_EQ(ret, NETMANAGER_ERR_SYSTEM_INTERNAL);

    sptr<NetworkVpnService::MultiVpnEventCallback> multiVpnEventCallback =
        new (std::nothrow) NetworkVpnService::MultiVpnEventCallback();
    ASSERT_NE(multiVpnEventCallback, nullptr);
    multiVpnEventCallback->userId = 100;
    multiVpnEventCallback->bundleName = "com.vpn.test";
    multiVpnEventCallback->callback = callback;
    instance_->multiVpnEventCallbacks_.push_back(multiVpnEventCallback);
    ret = instance_->SyncUnregisterMultiVpnEvent(callback);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, FactoryResetVpn001, TestSize.Level1)
{
    EXPECT_EQ(instance_->FactoryResetVpn(), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, SetAlwaysOnVpn001, TestSize.Level1)
{
    std::string pkg = "vpn";
    bool enable = true;
    int32_t ret = instance_->SetAlwaysOnVpn(pkg, enable);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    NoPermissionAccessToken accToken;
    enable = false;
    pkg = "";
    ret = instance_->SetAlwaysOnVpn(pkg, enable);
    EXPECT_TRUE(ret == NETMANAGER_ERR_INTERNAL || ret == NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, GetAlwaysOnVpn001, TestSize.Level1)
{
    std::string pkg = "";
    EXPECT_EQ(instance_->GetAlwaysOnVpn(pkg), NETMANAGER_ERR_INTERNAL);
}

HWTEST_F(NetworkVpnServiceTest, ParseJsonToConfig001, TestSize.Level1)
{
    sptr<VpnConfig> vpnCfg = new (std::nothrow) VpnConfig();
    INetAddr netAddr;
    netAddr.address_ = "1";
    netAddr.netMask_ = "1";
    netAddr.hostName_ = "1";
    vpnCfg->addresses_.push_back(netAddr);
    std::string jsonString;
    instance_->ParseConfigToJson(vpnCfg, jsonString);
    EXPECT_FALSE(jsonString.empty());
    sptr<VpnConfig> vpnCfgnew = new (std::nothrow) VpnConfig();
    instance_->ParseJsonToConfig(vpnCfgnew, jsonString);
    EXPECT_GT(vpnCfgnew->addresses_.size(), 0);
}

HWTEST_F(NetworkVpnServiceTest, OnRemoteDied001, TestSize.Level1)
{
    auto networkVpnService = std::make_shared<NetworkVpnService>();
    int handle = 1;
    std::u16string descriptor = std::u16string();
    sptr<IRemoteObject> result = new (std::nothrow) IPCObjectProxy(handle, descriptor);
    wptr<IRemoteObject> remote = result.GetRefPtr();
    networkVpnService->OnRemoteDied(remote);
    EXPECT_TRUE(networkVpnService->vpnObj_ == nullptr);
}

HWTEST_F(NetworkVpnServiceTest, RegisterBundleName001, TestSize.Level1)
{
    instance_->StartAlwaysOnVpn();
    EXPECT_FALSE(instance_->vpnObj_ == nullptr);
    instance_->SubscribeCommonEvent();
    instance_->RegisterFactoryResetCallback();
    int32_t systemAbilityId = 1;
    std::string deviceId = "testvpn";
    instance_->OnAddSystemAbility(systemAbilityId, deviceId);
    instance_->OnRemoveSystemAbility(systemAbilityId, deviceId);

    std::string bundleName = "vpntest";
    std::string abilityName = "vpnAbility";
    EXPECT_EQ(instance_->RegisterBundleName(bundleName, abilityName), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, OnExtensionStateChanged001, TestSize.Level1)
{
    int ret = 0;
    if (instance_->vpnHapObserver_ == nullptr) {
        instance_->vpnHapObserver_ = new NetworkVpnService::VpnHapObserver(*instance_);
        ret = NETMANAGER_EXT_SUCCESS;
    }
    ASSERT_NE(instance_->vpnHapObserver_, nullptr);
    AppExecFwk::AbilityStateData abilityStateData;
    instance_->vpnHapObserver_->OnExtensionStateChanged(abilityStateData);
    AppExecFwk::ProcessData processData;
    instance_->vpnHapObserver_->OnProcessCreated(processData);
    instance_->vpnHapObserver_->OnProcessStateChanged(processData);
    instance_->vpnHapObserver_->OnProcessDied(processData);
}

HWTEST_F(NetworkVpnServiceTest, OnVpnConnStateChanged001, TestSize.Level1)
{
    int ret = 0;
    if (instance_->vpnConnCallback_ == nullptr) {
        instance_->vpnConnCallback_ = std::make_shared<NetworkVpnService::VpnConnStateCb>(*instance_);
        ret = NETMANAGER_EXT_SUCCESS;
    }
    ASSERT_NE(instance_->vpnConnCallback_, nullptr);
    VpnConnectState state = VpnConnectState::VPN_CONNECTED;
    instance_->vpnConnCallback_->OnVpnConnStateChanged(state, "vpn-tun", "192.168.10.2", "", false);
}

HWTEST_F(NetworkVpnServiceTest, OnMultiVpnConnStateChanged001, TestSize.Level1)
{
    int ret = 0;
    std::string vpnId = "testId";
    if (instance_->vpnConnCallback_ == nullptr) {
        instance_->vpnConnCallback_ = std::make_shared<NetworkVpnService::VpnConnStateCb>(*instance_);
        ret = NETMANAGER_EXT_SUCCESS;
    }
    ASSERT_NE(instance_->vpnConnCallback_, nullptr);
    VpnConnectState state = VpnConnectState::VPN_CONNECTED;
    instance_->vpnConnCallback_->OnMultiVpnConnStateChanged(state, vpnId);
}

HWTEST_F(NetworkVpnServiceTest, OnReceiveEvent001, TestSize.Level1)
{
    instance_->SubscribeCommonEvent();
    EXPECT_TRUE(instance_->subscriber_ != nullptr);
    EventFwk::CommonEventData eventData;
    instance_->subscriber_->OnReceiveEvent(eventData);
    EXPECT_TRUE(instance_->subscriber_ != nullptr);
}

HWTEST_F(NetworkVpnServiceTest, TryParseSysRemoteAddrTest001, TestSize.Level1)
{
    sptr<VpnDataBean> vpnBean = nullptr;
    instance_->TryParseSysRemoteAddr(vpnBean);
    vpnBean = new (std::nothrow) VpnDataBean();
    ASSERT_NE(vpnBean, nullptr);
    instance_->TryParseSysRemoteAddr(vpnBean);
    EXPECT_TRUE(vpnBean->vpnAddress_.empty());
    vpnBean->remoteAddr_ = "notexistvpnbean.com";
    instance_->TryParseSysRemoteAddr(vpnBean);
    EXPECT_TRUE(vpnBean->vpnAddress_.empty());
    vpnBean->remoteAddr_ = "www.baidu.com";
    instance_->TryParseSysRemoteAddr(vpnBean);
    EXPECT_FALSE(vpnBean->vpnAddress_.empty());
}

HWTEST_F(NetworkVpnServiceTest, IsSetUpReady001, TestSize.Level1)
{
    NetManagerExtAccessToken accToken;
    sptr<SysVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(config, nullptr);
    sptr<INetAddr> netAddr = new (std::nothrow) INetAddr();
    ASSERT_NE(netAddr, nullptr);
    std::string id = "123";
    std::string ip = "1.1.1.1";
    netAddr->address_ = ip;
    netAddr->prefixlen_ = 1;
    config->addresses_.push_back(*netAddr);
    config->vpnId_ = id;
    config->vpnName_ = "testSetUpVpn";
    config->vpnType_ = 1;

    std::string vpnBundleName;
    int32_t userId = AppExecFwk::Constants::UNSPECIFIED_USERID;
    std::vector<int32_t> activeUserIds;
    std::shared_ptr<IpsecVpnCtl> vpnCtl = std::make_shared<IpsecVpnCtl>(config, "", userId, activeUserIds);
    ASSERT_NE(vpnCtl, nullptr);
    instance_->vpnObj_ = vpnCtl;
    int32_t ret = instance_->IsSetUpReady(id, vpnBundleName, userId, activeUserIds);
    instance_->vpnObj_ = nullptr;
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    instance_->vpnObjMap_.insert({id, vpnCtl});
    ret = instance_->IsSetUpReady(id, vpnBundleName, userId, activeUserIds);
    EXPECT_EQ(ret, NETWORKVPN_ERROR_VPN_EXIST);
    ret = instance_->IsSetUpReady("", vpnBundleName, userId, activeUserIds);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
    config->vpnId_ = "1234";
    ret = instance_->SetUpSysVpn(config, false);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
    instance_->vpnObjMap_.erase(id);
}

HWTEST_F(NetworkVpnServiceTest, CreateVirtualCtl001, TestSize.Level1)
{
    sptr<SysVpnConfig> config = nullptr;
    std::vector<int32_t> ids;
    std::shared_ptr<NetVpnImpl> vpn = instance_->CreateVirtualCtl(config, 0, ids);
    EXPECT_EQ(vpn, nullptr);
}
} // namespace NetManagerStandard
} // namespace OHOS
