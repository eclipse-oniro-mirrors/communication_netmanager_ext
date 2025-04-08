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
#include "nativetoken_kit.h"
#include "net_manager_constants.h"
#include "system_ability_definition.h"
#include "token_setproc.h"

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
    int32_t OnVpnStateChanged(bool &isConnected) override{ return 0; };
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
    if (vpnConfig_ == nullptr) {
        return;
    }
    EXPECT_EQ(instance_->AddSysVpnConfig(*vpnConfig_), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, AddSysVpnConfigTest002, TestSize.Level1)
{
    sptr<SysVpnConfig> config = nullptr;
    EXPECT_EQ(instance_->AddSysVpnConfig(*config), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
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
    EXPECT_EQ(instance_->AddSysVpnConfig(*config), NETMANAGER_EXT_SUCCESS);

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
    sptr<SysVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(config, nullptr);
    config->vpnId_ = "testId1";
    config->vpnName_ = "testName1";
    config->vpnType_ = 1;
    instance_->AddSysVpnConfig(*config);
    config->vpnId_ = "testId2";
    config->vpnName_ = "testName2";
    instance_->AddSysVpnConfig(*config);
    EXPECT_EQ(instance_->GetSysVpnConfigList(list), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(instance_->DeleteSysVpnConfig("testId1"), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(instance_->DeleteSysVpnConfig("testId2"), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, GetSysVpnConfigTest001, TestSize.Level1)
{
    if (vpnConfig_ == nullptr) {
        return;
    }

    // vpnConfig_ is "test001"
    instance_->AddSysVpnConfig(*vpnConfig_);
    sptr<SysVpnConfig> resultConfig = new (std::nothrow) IpsecVpnConfig();
    EXPECT_EQ(instance_->GetSysVpnConfig(*resultConfig, vpnId_), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, GetSysVpnConfigTest002, TestSize.Level1)
{
    std::string id;
    sptr<SysVpnConfig> resultConfig = new (std::nothrow) IpsecVpnConfig();
    EXPECT_EQ(instance_->GetSysVpnConfig(*resultConfig, id), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetworkVpnServiceTest, GetConnectedSysVpnConfigTest001, TestSize.Level1)
{
    sptr<SysVpnConfig> resultConfig = new (std::nothrow) IpsecVpnConfig();;
    EXPECT_EQ(instance_->GetConnectedSysVpnConfig(*resultConfig), NETMANAGER_EXT_SUCCESS);

    instance_->AddSysVpnConfig(*vpnConfig_);
    instance_->SetUpVpn(*vpnConfig_);
    EXPECT_EQ(instance_->GetConnectedSysVpnConfig(*vpnConfig_), NETMANAGER_EXT_SUCCESS);
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

HWTEST_F(NetworkVpnServiceTest, QueryVpnData001, TestSize.Level1)
{
    SysVpnConfig openvpnConfig;
    sptr<VpnDataBean> vpnBean = nullptr;
    EXPECT_EQ(instance_->QueryVpnData(openvpnConfig, vpnBean), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    vpnBean = new (std::nothrow) VpnDataBean();
    ASSERT_NE(vpnBean, nullptr);
    EXPECT_EQ(instance_->QueryVpnData(openvpnConfig, vpnBean), NETMANAGER_EXT_ERR_INVALID_PARAMETER);
    openvpnConfig.vpnId_ = "test001";
    EXPECT_EQ(instance_->QueryVpnData(openvpnConfig, vpnBean), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, CreateOpenvpnCtl001, TestSize.Level1)
{
    std::vector<int32_t> activeUserIds;
    int32_t userId = TEST_USERID;
    sptr<VpnDataBean> vpnBean = nullptr;
    EXPECT_EQ(instance_->CreateOpenvpnCtl(vpnBean, TEST_USERID, activeUserIds), nullptr);
    vpnBean = new (std::nothrow) VpnDataBean();
    ASSERT_NE(vpnBean, nullptr);
    ASSERT_NE(instance_->CreateOpenvpnCtl(vpnBean, userId, activeUserIds), nullptr);
}

HWTEST_F(NetworkVpnServiceTest, CreateL2tpCtl001, TestSize.Level1)
{
    std::vector<int32_t> activeUserIds;
    int32_t userId = TEST_USERID;
    sptr<VpnDataBean> vpnBean = nullptr;
    EXPECT_EQ(instance_->CreateL2tpCtl(vpnBean, TEST_USERID, activeUserIds), nullptr);
    vpnBean = new (std::nothrow) VpnDataBean();
    ASSERT_NE(vpnBean, nullptr);
    ASSERT_NE(instance_->CreateL2tpCtl(vpnBean, userId, activeUserIds), nullptr);
}

HWTEST_F(NetworkVpnServiceTest, CreateIpsecVpnCtl001, TestSize.Level1)
{
    std::vector<int32_t> activeUserIds;
    int32_t userId = TEST_USERID;
    sptr<VpnDataBean> vpnBean = nullptr;
    EXPECT_EQ(instance_->CreateIpsecVpnCtl(vpnBean, TEST_USERID, activeUserIds), nullptr);
    vpnBean = new (std::nothrow) VpnDataBean();
    ASSERT_NE(vpnBean, nullptr);
    ASSERT_NE(instance_->CreateIpsecVpnCtl(vpnBean, userId, activeUserIds), nullptr);
}

HWTEST_F(NetworkVpnServiceTest, SetUpVpn001, TestSize.Level1)
{
    sptr<SysVpnConfig> config = nullptr;
    int32_t ret = instance_->SetUpVpn(*config);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    config = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(config, nullptr);
    config->vpnId_ = "123";
    std::shared_ptr<NetVpnImpl> tmp = instance_->vpnObj_;
    instance_->vpnObj_ = nullptr;
    ret = instance_->SetUpVpn(*config);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INTERNAL);
    config->vpnName_ = "testSetUpVpn";
    config->vpnType_ = 1;
    ret = instance_->SetUpVpn(*config);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INTERNAL);
    std::string pkg = "test1";
    int32_t userId = AppExecFwk::Constants::UNSPECIFIED_USERID;
    std::vector<int32_t> activeUserIds;
    instance_->CheckCurrentAccountType(userId, activeUserIds);
    instance_->vpnObj_ = std::make_shared<ExtendedVpnCtl>(config, pkg, userId, activeUserIds);
    ret = instance_->SetUpVpn(*config);
    EXPECT_EQ(ret, NETWORKVPN_ERROR_VPN_EXIST);
    instance_->vpnObj_ = nullptr;
    instance_->vpnObj_ = std::make_shared<ExtendedVpnCtl>(config, pkg, 100, activeUserIds);
    EXPECT_EQ(ret, NETWORKVPN_ERROR_VPN_EXIST);
    instance_->vpnObj_ = tmp;
}

HWTEST_F(NetworkVpnServiceTest, SetUpVpn002, TestSize.Level1)
{
    sptr<SysVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(config, nullptr);
    config->vpnId_ = "123";
    int32_t ret = instance_->SetUpVpn(*config, true);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);

    NetManagerExtAccessToken accToken;
    std::string pkg = "test1";
    int32_t userId = AppExecFwk::Constants::UNSPECIFIED_USERID;
    std::vector<int32_t> activeUserIds;
    instance_->CheckCurrentAccountType(userId, activeUserIds);
    instance_->vpnObj_ = std::make_shared<ExtendedVpnCtl>(config, pkg, userId, activeUserIds);
    ret = instance_->SetUpVpn(*config, true);
    EXPECT_EQ(ret, NETWORKVPN_ERROR_VPN_EXIST);

    instance_->vpnObj_ = nullptr;
    ret = instance_->SetUpVpn(*config, true);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    ret = instance_->SetUpVpn(*config, true);
    EXPECT_EQ(ret, NETWORKVPN_ERROR_VPN_EXIST);
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
    SysVpnConfig config;
    int32_t userId = 0;
    std::vector<int32_t> activeUserIds;
    std::shared_ptr<NetVpnImpl> sysVpnCtl = nullptr;
    sysVpnCtl = instance_->CreateSysVpnCtl(config, userId, activeUserIds);
    EXPECT_TRUE(sysVpnCtl == nullptr);

    config.vpnId_ = "1234";
    config.vpnName_ = "test001";
    config.vpnType_ = 1;
    EXPECT_EQ(instance_->AddSysVpnConfig(config), NETMANAGER_EXT_SUCCESS);

    sysVpnCtl = instance_->CreateSysVpnCtl(config, userId, activeUserIds);
    EXPECT_TRUE(sysVpnCtl != nullptr);
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
    EXPECT_EQ(instance_->Prepare(isExistVpn, isRun, pkg), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, DestroyVpn001, TestSize.Level1)
{
    bool isVpnExtCall = false;
    int32_t ret = instance_->DestroyVpn(isVpnExtCall);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
    NetManagerExtAccessToken access;
    ret = instance_->DestroyVpn(isVpnExtCall);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, RegisterVpnEvent001, TestSize.Level1)
{
    sptr<IVpnEventCallback> callback = new (std::nothrow) IVpnEventCallbackTest();
    EXPECT_EQ(instance_->RegisterVpnEvent(callback), NETMANAGER_EXT_ERR_OPERATION_FAILED);
}

HWTEST_F(NetworkVpnServiceTest, UnregisterVpnEvent001, TestSize.Level1)
{
    sptr<IVpnEventCallback> callback = new (std::nothrow) IVpnEventCallbackTest();
    EXPECT_EQ(instance_->UnregisterVpnEvent(callback), NETMANAGER_EXT_ERR_OPERATION_FAILED);
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
    EXPECT_EQ(ret, NETMANAGER_ERR_INTERNAL);
}

HWTEST_F(NetworkVpnServiceTest, GetAlwaysOnVpn001, TestSize.Level1)
{
    std::string pkg = "";
    EXPECT_EQ(instance_->GetAlwaysOnVpn(pkg), NETMANAGER_EXT_SUCCESS);
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
    wptr<IRemoteObject> remote = nullptr;
    int handle = 1;
    sptr<IRemoteObject> result = nullptr;
    std::u16string descriptor = std::u16string();
    result = new (std::nothrow) IPCObjectProxy(handle, descriptor);
    IRemoteObject *object = result.GetRefPtr();
    remote = object;
    instance_->OnRemoteDied(remote);
    EXPECT_TRUE(instance_->vpnObj_ == nullptr);
}

HWTEST_F(NetworkVpnServiceTest, RegisterBundleName001, TestSize.Level1)
{
    instance_->StartAlwaysOnVpn();
    EXPECT_TRUE(instance_->vpnObj_ == nullptr);
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
    instance_->vpnConnCallback_->OnVpnConnStateChanged(state);
}

HWTEST_F(NetworkVpnServiceTest, OnReceiveEvent001, TestSize.Level1)
{
    instance_->SubscribeCommonEvent();
    EXPECT_TRUE(instance_->subscriber_ != nullptr);
    EventFwk::CommonEventData eventData;
    instance_->subscriber_->OnReceiveEvent(eventData);
    EXPECT_TRUE(instance_->subscriber_ != nullptr);
}
} // namespace NetManagerStandard
} // namespace OHOS
