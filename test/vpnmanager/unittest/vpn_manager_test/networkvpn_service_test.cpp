/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "accesstoken_kit.h"
#include "extended_vpn_ctl.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "parameters.h"

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "net_manager_constants.h"
#include "networkvpn_service.h"
#include "vpn_event_callback_stub.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr const char *NET_ACTIVATE_WORK_THREAD = "VPN_CALLBACK_WORK_THREAD";
class VpnEventTestCallback : public VpnEventCallbackStub {
public:
    int32_t OnVpnStateChanged(bool isConnected, const std::string &vpnIfName, const std::string &vpnIfAddr,
                              const std::string &vpnId, bool isGlobalVpn) override{ return 0; };
    int32_t OnMultiVpnStateChanged(bool isConnected, const std::string &bundleName,
        const std::string &vpnId) override{ return 0; };
    int32_t OnVpnMultiUserSetUp() override{ return 0; };
};
} // namespace

class NetworkVpnServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline auto instance_ = DelayedSingleton<NetworkVpnService>::GetInstance();
    static inline sptr<IVpnEventCallback> eventCallback_ = nullptr;
};

void NetworkVpnServiceTest::SetUpTestCase()
{
    instance_->OnStart();
    eventCallback_ = new (std::nothrow) VpnEventTestCallback();
    ASSERT_NE(eventCallback_, nullptr);
}

void NetworkVpnServiceTest::TearDownTestCase()
{
    instance_->OnStop();
}

void NetworkVpnServiceTest::SetUp() {}

void NetworkVpnServiceTest::TearDown() {}

HWTEST_F(NetworkVpnServiceTest, OnStart, TestSize.Level1)
{
    instance_->state_ = NetworkVpnService::STATE_RUNNING;
    instance_->OnStart();
    instance_->state_ = NetworkVpnService::STATE_STOPPED;
    EXPECT_EQ(instance_->state_, NetworkVpnService::STATE_STOPPED);
}

HWTEST_F(NetworkVpnServiceTest, OnStop, TestSize.Level1)
{
    instance_->OnStop();
    EXPECT_EQ(instance_->state_, NetworkVpnService::STATE_STOPPED);
}

HWTEST_F(NetworkVpnServiceTest, Dump, TestSize.Level1)
{
    int32_t fd = 1;
    std::vector<std::u16string> args = {};
    EXPECT_EQ(instance_->Dump(fd, args), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, Init, TestSize.Level1)
{
    EXPECT_EQ(instance_->Init(), false);
}

HWTEST_F(NetworkVpnServiceTest, GetDumpMessage, TestSize.Level1)
{
    std::string message;
    instance_->vpnObj_ = nullptr;
    instance_->GetDumpMessage(message);
    int32_t userId = AppExecFwk::Constants::UNSPECIFIED_USERID;
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    std::vector<int32_t> activeUserIds;
    instance_->vpnObj_ = std::make_shared<ExtendedVpnCtl>(config, "", userId, activeUserIds);
    instance_->GetDumpMessage(message);
    EXPECT_EQ(message.empty(), false);
}

HWTEST_F(NetworkVpnServiceTest, Prepare001, TestSize.Level1)
{
    bool isExistVpn = false;
    bool isRun = false;
    std::string pkg;
    EXPECT_NE(instance_->Prepare(isExistVpn, isRun, pkg), NETMANAGER_EXT_ERR_OPERATION_FAILED);
}

HWTEST_F(NetworkVpnServiceTest, SetUpVpn001, TestSize.Level1)
{
    int32_t userId = AppExecFwk::Constants::DEFAULT_USERID;
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    std::vector<int32_t> activeUserIds;
    instance_->vpnObj_ = std::make_shared<ExtendedVpnCtl>(config, "", userId, activeUserIds);
    EXPECT_EQ(instance_->SetUpVpn(*config), NETMANAGER_ERR_PERMISSION_DENIED);

    userId = AppExecFwk::Constants::UNSPECIFIED_USERID;
    instance_->vpnObj_ = std::make_shared<ExtendedVpnCtl>(config, "", userId, activeUserIds);
    EXPECT_EQ(instance_->SetUpVpn(*config), NETMANAGER_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkVpnServiceTest, SetUpVpn002, TestSize.Level1)
{
    system::SetParameter("persist.edm.vpn_disable", "true");
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    int32_t ret = instance_->SetUpVpn(*config, true);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
    ret = instance_->SetUpVpn(*config, false);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
    system::SetParameter("persist.edm.vpn_disable", "false");
}

HWTEST_F(NetworkVpnServiceTest, Protect, TestSize.Level1)
{
    EXPECT_NE(instance_->Protect(), NETMANAGER_EXT_ERR_OPERATION_FAILED);
}

HWTEST_F(NetworkVpnServiceTest, DestroyVpn001, TestSize.Level1)
{
    instance_->vpnObj_ = nullptr;
    EXPECT_EQ(instance_->DestroyVpn(), NETMANAGER_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkVpnServiceTest, DestroyVpn002, TestSize.Level1)
{
    const char* permissions[] = { "ohos.permission.MANAGE_VPN", "ohos.permission.MANAGE_EDM_POLICY" };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 2,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = permissions,
        .acls = nullptr,
        .aplStr = "system_basic",
    };

    infoInstance.processName = "vpn_manager_test";
    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    uint64_t tokenIdBak = GetSelfTokenID();
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();

    instance_->vpnObj_ = nullptr;
    EXPECT_EQ(instance_->DestroyVpn(), NETMANAGER_EXT_SUCCESS);

    SetSelfTokenID(tokenIdBak);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

HWTEST_F(NetworkVpnServiceTest, DestroyVpn003, TestSize.Level1)
{
    const char* permissions[] = { "ohos.permission.MANAGE_VPN" };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 1,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = permissions,
        .acls = nullptr,
        .aplStr = "system_basic",
    };

    infoInstance.processName = "vpn_manager_test";
    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    uint64_t tokenIdBak = GetSelfTokenID();
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();

    instance_->vpnObj_ = nullptr;
    EXPECT_EQ(instance_->DestroyVpn(), NETMANAGER_EXT_SUCCESS);

    SetSelfTokenID(tokenIdBak);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

HWTEST_F(NetworkVpnServiceTest, DestroyVpn004, TestSize.Level1)
{
    const char* permissions[] = { "ohos.permission.MANAGE_VPN" };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 1,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = permissions,
        .acls = nullptr,
        .aplStr = "system_basic",
    };

    infoInstance.processName = "vpn_manager_test";
    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    uint64_t tokenIdBak = GetSelfTokenID();
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();

    int32_t uidTmp = instance_->hasOpenedVpnUid_;
    instance_->hasOpenedVpnUid_ = -1;
    instance_->vpnObj_ = nullptr;
    auto ret = instance_->DestroyVpn();
    EXPECT_TRUE(ret == NETMANAGER_EXT_ERR_OPERATION_FAILED || ret == NETMANAGER_EXT_SUCCESS);
    instance_->hasOpenedVpnUid_ = uidTmp;

    SetSelfTokenID(tokenIdBak);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

HWTEST_F(NetworkVpnServiceTest, RegisterSharingEventTest001, TestSize.Level1)
{
    int32_t ret = instance_->RegisterVpnEvent(eventCallback_);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, UnregisterSharingEventTest001, TestSize.Level1)
{
    int32_t ret = instance_->UnregisterVpnEvent(eventCallback_);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, CheckCurrentUser, TestSize.Level1)
{
    int32_t userId = 0;
    std::vector<int32_t> activeUserIds;
    EXPECT_EQ(instance_->CheckCurrentAccountType(userId, activeUserIds), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, SyncRegisterVpnEvent, TestSize.Level1)
{
    instance_->vpnEventCallbacks_.push_back(eventCallback_);
    EXPECT_EQ(instance_->SyncRegisterVpnEvent(eventCallback_), NETMANAGER_EXT_ERR_OPERATION_FAILED);
    instance_->vpnEventCallbacks_.clear();
    sptr<IVpnEventCallback> eventCallback_1 = new (std::nothrow) VpnEventTestCallback();
    instance_->vpnEventCallbacks_.push_back(eventCallback_1);
    sptr<IVpnEventCallback> eventCallback_2 = new (std::nothrow) VpnEventTestCallback();
    instance_->vpnEventCallbacks_.push_back(eventCallback_2);
    EXPECT_EQ(instance_->SyncRegisterVpnEvent(eventCallback_), NETMANAGER_EXT_ERR_OPERATION_FAILED);
}

HWTEST_F(NetworkVpnServiceTest, SyncUnregisterVpnEvent, TestSize.Level1)
{
    EXPECT_EQ(instance_->SyncUnregisterVpnEvent(eventCallback_), NETMANAGER_EXT_ERR_OPERATION_FAILED);
    instance_->vpnEventCallbacks_.clear();
    EXPECT_EQ(instance_->SyncUnregisterVpnEvent(eventCallback_), NETMANAGER_EXT_ERR_OPERATION_FAILED);
}

HWTEST_F(NetworkVpnServiceTest, NetworkVpnServiceBranch, TestSize.Level1)
{
    EXPECT_EQ(instance_->CreateVpnConnection(), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, FactoryResetVpnTest001, TestSize.Level1)
{
    EXPECT_EQ(instance_->FactoryResetVpn(), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, OnAddSystemAbility001, TestSize.Level1)
{
    std::string deviceId = "dev1";
    instance_->OnRemoveSystemAbility(COMM_NETSYS_NATIVE_SYS_ABILITY_ID, deviceId);
    EXPECT_TRUE(instance_->hasSARemoved_);

    instance_->OnAddSystemAbility(COMM_NETSYS_NATIVE_SYS_ABILITY_ID, deviceId);
    EXPECT_FALSE(instance_->hasSARemoved_);
}

HWTEST_F(NetworkVpnServiceTest, NetworkVpnServiceBranchTest001, TestSize.Level1)
{
    instance_->RecoverVpnConfig();
    instance_->RegisterFactoryResetCallback();
    instance_->StartAlwaysOnVpn();
    instance_->SubscribeCommonEvent();
    std::string pkg = "";
    bool enable = false;
    sptr<VpnConfig> vpnCfg = new (std::nothrow) VpnConfig();
    ASSERT_TRUE(vpnCfg != nullptr);
    if (vpnCfg != nullptr) {
        instance_->SetAlwaysOnVpn(pkg, enable);
        std::string jsonString = "";
        instance_->ParseConfigToJson(vpnCfg, jsonString);
        instance_->ParseJsonToConfig(vpnCfg, jsonString);
    }
    int32_t ret = instance_->SetAlwaysOnVpn(pkg, enable);
    EXPECT_NE(ret, NETMANAGER_EXT_ERR_OPERATION_FAILED);
    ret = instance_->GetAlwaysOnVpn(pkg);
    EXPECT_NE(ret, NETMANAGER_EXT_ERR_OPERATION_FAILED);
}

HWTEST_F(NetworkVpnServiceTest, NetworkVpnServiceBranchTest002, TestSize.Level1)
{
    std::string pkg = "testMock";
    bool enable = true;
    sptr<VpnConfig> vpnCfg = new (std::nothrow) VpnConfig();
    if (vpnCfg != nullptr) {
        instance_->SetAlwaysOnVpn(pkg, enable);
    }
    int32_t ret = instance_->GetAlwaysOnVpn(pkg);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}
 
HWTEST_F(NetworkVpnServiceTest, NetworkVpnServiceBranchTest003, TestSize.Level1)
{
    std::string pkg = "testMock";
    bool enable = true;
    int32_t userId = AppExecFwk::Constants::DEFAULT_USERID;
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    std::vector<int32_t> activeUserIds;
    instance_->vpnObj_ = std::make_shared<ExtendedVpnCtl>(config, pkg, userId, activeUserIds);
    instance_->SetAlwaysOnVpn(pkg, enable);
    instance_->vpnObj_ = std::make_shared<ExtendedVpnCtl>(config, "", userId, activeUserIds);
    instance_->SetAlwaysOnVpn(pkg, enable);
    int32_t ret = instance_->GetAlwaysOnVpn(pkg);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, VpnHapObserverTest001, TestSize.Level1)
{
    int32_t userId = AppExecFwk::Constants::DEFAULT_USERID;
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    std::vector<int32_t> activeUserIds;
    instance_->vpnObj_ = std::make_shared<ExtendedVpnCtl>(config, "", userId, activeUserIds);
    AppExecFwk::ProcessData data;
    data.pid = 123456;
    data.uid = userId;
    sptr<NetworkVpnService::VpnHapObserver> vpnHapObserver = new NetworkVpnService::VpnHapObserver(
        *instance_, "testBundleName", "testAbility");
    ASSERT_NE(vpnHapObserver, nullptr);
    instance_->setVpnPidMap_.emplace(userId, 123456);
    vpnHapObserver->OnProcessDied(data);
    EXPECT_FALSE(instance_->vpnObj_ == nullptr);
}

HWTEST_F(NetworkVpnServiceTest, VpnHapObserverTest002, TestSize.Level1)
{
    int32_t userId = AppExecFwk::Constants::DEFAULT_USERID;
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    std::vector<int32_t> activeUserIds;
    instance_->vpnObj_ = std::make_shared<ExtendedVpnCtl>(config, "", userId, activeUserIds);
    AppExecFwk::ProcessData data;
    data.pid = 123456;
    data.uid = userId + 1;  // differs from userId
    sptr<NetworkVpnService::VpnHapObserver> vpnHapObserver = new NetworkVpnService::VpnHapObserver(
        *instance_, "testBundleName");
    ASSERT_NE(vpnHapObserver, nullptr);
    instance_->setVpnPidMap_.emplace(userId, 123456);
    vpnHapObserver->OnProcessDied(data);
    EXPECT_TRUE(instance_->vpnObj_ != nullptr);
}

HWTEST_F(NetworkVpnServiceTest, PublishEventTest001, TestSize.Level1)
{
    OHOS::AAFwk::Want want;
    int eventCode = 100;
    bool isOrdered = true;
    bool isSticky = true;
    std::vector<std::string> permissions = {};
    EXPECT_FALSE(instance_->PublishEvent(want, eventCode, isOrdered, isSticky, permissions));
    std::vector<std::string> permissions2 = {"1", "2", "3"};
    EXPECT_FALSE(instance_->PublishEvent(want, eventCode, isOrdered, isSticky, permissions));
}

HWTEST_F(NetworkVpnServiceTest, PublishVpnConnectionStateEventTest001, TestSize.Level1)
{
    VpnConnectState state = VpnConnectState::VPN_CONNECTED;
    instance_->PublishVpnConnectionStateEvent(state);
    state = VpnConnectState::VPN_DISCONNECTED;
    instance_->PublishVpnConnectionStateEvent(state);
    EXPECT_EQ(state, VpnConnectState::VPN_DISCONNECTED);
}

HWTEST_F(NetworkVpnServiceTest, OnVpnMultiUserSetUpTest001, TestSize.Level1)
{
    instance_->OnVpnMultiUserSetUp();
    EXPECT_EQ(instance_->vpnEventCallbacks_.size(), 0);
}

HWTEST_F(NetworkVpnServiceTest, PrepareTest001, TestSize.Level1)
{
    bool isExistVpn = true;
    bool isRun = true;
    std::string pkg = "123";
    int32_t userId = AppExecFwk::Constants::DEFAULT_USERID;
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    std::vector<int32_t> activeUserIds;
    instance_->vpnObj_ = std::make_shared<ExtendedVpnCtl>(config, "", userId, activeUserIds);
    instance_->Prepare(isExistVpn, isRun, pkg);
    EXPECT_NE(instance_->vpnObj_, nullptr);
}

HWTEST_F(NetworkVpnServiceTest, ConvertRouteToConfigTest001, TestSize.Level1)
{
    Route tmp;
    cJSON *json = cJSON_CreateObject();
    instance_->ConvertRouteToConfig(tmp, json);
    EXPECT_EQ(tmp.iface_, "");
    cJSON_AddStringToObject(json, "iface", "123");
    instance_->ConvertRouteToConfig(tmp, json);
    EXPECT_EQ(tmp.iface_, "123");
}

HWTEST_F(NetworkVpnServiceTest, ConvertRouteToConfigTest002, TestSize.Level1)
{
    Route tmp;
    cJSON *json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "iface", 123);
    instance_->ConvertRouteToConfig(tmp, json);
    EXPECT_EQ(tmp.iface_, "");
}

HWTEST_F(NetworkVpnServiceTest, ConvertRouteToConfigTest003, TestSize.Level1)
{
    Route tmp;
    cJSON *json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "rtnType", 123);
    instance_->ConvertRouteToConfig(tmp, json);
    EXPECT_EQ(tmp.rtnType_, 123);
}

HWTEST_F(NetworkVpnServiceTest, ConvertRouteToConfigTest004, TestSize.Level1)
{
    Route tmp;
    cJSON *json = cJSON_CreateObject();
    cJSON_AddTrueToObject(json, "rtnType");
    instance_->ConvertRouteToConfig(tmp, json);
    EXPECT_EQ(tmp.rtnType_, 1);
}

HWTEST_F(NetworkVpnServiceTest, ConvertRouteToConfigTest005, TestSize.Level1)
{
    Route tmp;
    cJSON *json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "mtu", 123);
    instance_->ConvertRouteToConfig(tmp, json);
    EXPECT_EQ(tmp.mtu_, 123);
}

HWTEST_F(NetworkVpnServiceTest, ConvertRouteToConfigTest006, TestSize.Level1)
{
    Route tmp;
    cJSON *json = cJSON_CreateObject();
    cJSON_AddTrueToObject(json, "mtu");
    instance_->ConvertRouteToConfig(tmp, json);
    EXPECT_EQ(tmp.mtu_, 0);
}

HWTEST_F(NetworkVpnServiceTest, ConvertRouteToConfigTest007, TestSize.Level1)
{
    Route tmp;
    cJSON *json = cJSON_CreateObject();
    cJSON_AddTrueToObject(json, "isHost");
    instance_->ConvertRouteToConfig(tmp, json);
    EXPECT_EQ(tmp.isHost_, true);
}

HWTEST_F(NetworkVpnServiceTest, ConvertRouteToConfigTest008, TestSize.Level1)
{
    Route tmp;
    cJSON *json = cJSON_CreateObject();
    cJSON *isHost = cJSON_CreateObject();
    cJSON_AddItemToObject(json, "isHost", isHost);
    instance_->ConvertRouteToConfig(tmp, json);
    EXPECT_EQ(tmp.isHost_, false);
}

HWTEST_F(NetworkVpnServiceTest, ConvertRouteToConfigTest009, TestSize.Level1)
{
    Route tmp;
    cJSON *json = cJSON_CreateObject();
    cJSON_AddTrueToObject(json, "hasGateway");
    instance_->ConvertRouteToConfig(tmp, json);
    EXPECT_EQ(tmp.hasGateway_, true);
}

HWTEST_F(NetworkVpnServiceTest, ConvertRouteToConfigTest010, TestSize.Level1)
{
    Route tmp;
    cJSON *json = cJSON_CreateObject();
    cJSON *hasGateway = cJSON_CreateObject();
    cJSON_AddItemToObject(json, "hasGateway", hasGateway);
    instance_->ConvertRouteToConfig(tmp, json);
    EXPECT_EQ(tmp.hasGateway_, true);
}

HWTEST_F(NetworkVpnServiceTest, ConvertRouteToConfigTest011, TestSize.Level1)
{
    Route tmp;
    cJSON *json = cJSON_CreateObject();
    cJSON_AddTrueToObject(json, "isDefaultRoute");
    instance_->ConvertRouteToConfig(tmp, json);
    EXPECT_EQ(tmp.isDefaultRoute_, true);
}

HWTEST_F(NetworkVpnServiceTest, ConvertRouteToConfigTest012, TestSize.Level1)
{
    Route tmp;
    cJSON *json = cJSON_CreateObject();
    cJSON *isDefaultRoute = cJSON_CreateObject();
    cJSON_AddItemToObject(json, "isDefaultRoute", isDefaultRoute);
    instance_->ConvertRouteToConfig(tmp, json);
    EXPECT_EQ(tmp.isDefaultRoute_, false);
}

HWTEST_F(NetworkVpnServiceTest, ConvertRouteToConfigTest013, TestSize.Level1)
{
    Route tmp;
    cJSON *json = cJSON_CreateObject();
    cJSON *destination = cJSON_CreateObject();
    cJSON_AddItemToObject(json, "destination", destination);
    instance_->ConvertRouteToConfig(tmp, json);
    EXPECT_EQ(tmp.destination_.port_, 0);
}

HWTEST_F(NetworkVpnServiceTest, ConvertRouteToConfigTest014, TestSize.Level1)
{
    Route tmp;
    cJSON *json = cJSON_CreateObject();
    cJSON_AddTrueToObject(json, "destination");
    instance_->ConvertRouteToConfig(tmp, json);
    EXPECT_EQ(tmp.destination_.port_, 0);
}

HWTEST_F(NetworkVpnServiceTest, ConvertRouteToConfigTest015, TestSize.Level1)
{
    Route tmp;
    cJSON *json = cJSON_CreateObject();
    cJSON *gateway = cJSON_CreateObject();
    cJSON_AddItemToObject(json, "gateway", gateway);
    instance_->ConvertRouteToConfig(tmp, json);
    EXPECT_EQ(tmp.destination_.port_, 0);
}

HWTEST_F(NetworkVpnServiceTest, ConvertRouteToConfigTest016, TestSize.Level1)
{
    Route tmp;
    cJSON *json = cJSON_CreateObject();
    cJSON_AddTrueToObject(json, "gateway");
    instance_->ConvertRouteToConfig(tmp, json);
    EXPECT_EQ(tmp.destination_.port_, 0);
}

HWTEST_F(NetworkVpnServiceTest, ParseJsonToConfigTest001, TestSize.Level1)
{
    sptr<VpnConfig> vpnCfg;
    std::string jsonString = "";
    instance_->ParseJsonToConfig(vpnCfg, jsonString);
    EXPECT_EQ(jsonString, "");
}

HWTEST_F(NetworkVpnServiceTest, CheckCurrentAccountTypeTest001, TestSize.Level1)
{
    int32_t userId = 1;
    std::vector<int32_t> activeUserIds = {};
    instance_->CheckCurrentAccountType(userId, activeUserIds);
    EXPECT_EQ(userId, 0);
}

HWTEST_F(NetworkVpnServiceTest, OnAddSystemAbilityTest001, TestSize.Level1)
{
    int32_t systemAbilityId = COMM_NETSYS_NATIVE_SYS_ABILITY_ID;
    std::string deviceId = "123";
    instance_->OnAddSystemAbility(systemAbilityId, deviceId);
    EXPECT_NE(systemAbilityId, 0);
}

HWTEST_F(NetworkVpnServiceTest, OnNetSysRestartTest001, TestSize.Level1)
{
    int32_t userId = AppExecFwk::Constants::DEFAULT_USERID;
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    std::vector<int32_t> activeUserIds;
    instance_->vpnObj_ = std::make_shared<ExtendedVpnCtl>(config, "", userId, activeUserIds);
    instance_->OnNetSysRestart();
    EXPECT_NE(instance_->vpnObj_, nullptr);
}

HWTEST_F(NetworkVpnServiceTest, SubscribeCommonEventTest001, TestSize.Level1)
{
    EventFwk::CommonEventSubscribeInfo subscriberInfo;
    std::weak_ptr<NetworkVpnService> vpnService;
    instance_->subscriber_ = std::make_shared<NetworkVpnService::ReceiveMessage>(subscriberInfo, vpnService);
    instance_->SubscribeCommonEvent();
    EXPECT_NE(instance_->subscriber_, nullptr);
}

HWTEST_F(NetworkVpnServiceTest, RegisterBundleNameTest001, TestSize.Level1)
{
    std::string bundleName = "";
    std::string abilityName = "";
    instance_->RegisterBundleName(bundleName, abilityName);
    bundleName = "123";
    instance_->RegisterBundleName(bundleName, abilityName);
    EXPECT_NE(instance_->subscriber_, nullptr);
}

HWTEST_F(NetworkVpnServiceTest, RegisterBundleNameTest002, TestSize.Level1)
{
    std::string bundleName = "";
    std::string abilityName = "123";
    instance_->RegisterBundleName(bundleName, abilityName);
    bundleName = "123";
    instance_->RegisterBundleName(bundleName, abilityName);
    EXPECT_NE(instance_->subscriber_, nullptr);
}

HWTEST_F(NetworkVpnServiceTest, GetSelfAppNameTest001, TestSize.Level1)
{
    std::string selfAppName = "";
    std::string selfBundleName = "";
    instance_->GetSelfAppName(selfAppName, selfBundleName);
    EXPECT_NE(instance_->subscriber_, nullptr);
}

HWTEST_F(NetworkVpnServiceTest, IsCurrentVpnPidTest001, TestSize.Level1)
{
    int32_t uid = 1;
    int32_t pid = 2;
    instance_->setVpnPidMap_ = {
        {1, 2},
        {3, 4}};
    EXPECT_TRUE(instance_->IsCurrentVpnPid(uid, pid));
    pid = 1;
    EXPECT_FALSE(instance_->IsCurrentVpnPid(uid, pid));
}

HWTEST_F(NetworkVpnServiceTest, IsNeedNotifyTest001, TestSize.Level1)
{
    VpnConnectState state = VpnConnectState::VPN_DISCONNECTED;
    bool res = instance_->IsNeedNotify(state);
    state = VpnConnectState::VPN_CONNECTED;
    res = instance_->IsNeedNotify(state);
    EXPECT_TRUE(res);
}

HWTEST_F(NetworkVpnServiceTest, IsNeedNotifyTest002, TestSize.Level1)
{
    int32_t userId = AppExecFwk::Constants::DEFAULT_USERID;
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    std::vector<int32_t> activeUserIds;
    instance_->vpnObj_ = std::make_shared<ExtendedVpnCtl>(config, "", userId, activeUserIds);

    VpnConnectState state = VpnConnectState::VPN_DISCONNECTED;
    bool res = instance_->IsNeedNotify(state);
    EXPECT_TRUE(res);
}

HWTEST_F(NetworkVpnServiceTest, VpnExtensionAbilityTest001, TestSize.Level1)
{
    OHOS::AAFwk::Want want;
    EXPECT_NE(instance_->StartVpnExtensionAbility(want), ERR_OK);
    EXPECT_NE(instance_->StopVpnExtensionAbility(want), ERR_OK);
}
} // namespace NetManagerStandard
} // namespace OHOS
