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
#include <gmock/gmock.h>
#include <cerrno>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <unistd.h>

#include <securec.h>

#include "accesstoken_kit.h"
#ifdef GTEST_API_
#define private public
#define protected public
#endif
#include "extended_vpn_ctl.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "parameters.h"

#include "net_manager_constants.h"
#include "networkvpn_service.h"
#include "protect_context.h"
#include "vpn_event_callback_stub.h"
#include "vpn_fd_channel.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing;
using namespace testing::ext;
constexpr const char *NET_ACTIVATE_WORK_THREAD = "VPN_CALLBACK_WORK_THREAD";
class VpnEventTestCallback : public VpnEventCallbackStub {
public:
    int32_t OnVpnStateChanged(bool isConnected, const sptr<VpnState> &vpnState) override{ return 0; };
    int32_t OnMultiVpnStateChanged(bool isConnected, const std::string &bundleName,
        const std::string &vpnId) override{ return 0; };
    int32_t OnVpnMultiUserSetUp() override{ return 0; };
};

class MockNetworkVpnService : public NetworkVpnService {
public:
    MOCK_METHOD(bool, ShowVpnDialog,
        (const std::string &bundleName, const std::string &abilityName, const std::string &appName, int32_t uid),
        (override));
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
    VpnConfigRawData rawdata;
    EXPECT_EQ(true, rawdata.SerializeFromVpnConfig(*config));
    int32_t tunFd = INVALID_TUN_FD;
    EXPECT_EQ(instance_->SetUpVpn(rawdata, false, false, tunFd), NETMANAGER_ERR_PERMISSION_DENIED);

    userId = AppExecFwk::Constants::UNSPECIFIED_USERID;
    instance_->vpnObj_ = std::make_shared<ExtendedVpnCtl>(config, "", userId, activeUserIds);
    EXPECT_EQ(instance_->SetUpVpn(rawdata, false, false, tunFd), NETMANAGER_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkVpnServiceTest, SetUpVpn002, TestSize.Level1)
{
    system::SetParameter("persist.edm.vpn_disable", "true");
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    VpnConfigRawData rawdata;
    EXPECT_EQ(true, rawdata.SerializeFromVpnConfig(*config));
    int32_t tunFd = INVALID_TUN_FD;
    int32_t ret = instance_->SetUpVpn(rawdata, true, false, tunFd);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
    ret = instance_->SetUpVpn(rawdata, false, false, tunFd);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
    system::SetParameter("persist.edm.vpn_disable", "false");
}

// Regression of the FDSAN abort in CloseHeldVpnFd: the generated stub writes the [out]
// FileDescriptor to the reply parcel (WriteFileDescriptor dups the fd) and then closes
// the out fd, so the fd returned to IPC is owned by the stub. heldVpnFdMap_ must hold an
// independent dup; sharing one fd number left a stale entry and CloseHeldVpnFd closed an
// fd number already reused by another FILE*.
HWTEST_F(NetworkVpnServiceTest, HoldVpnInterfaceFdOwnership001, TestSize.Level1)
{
    instance_->CloseHeldVpnFd("");
    EXPECT_TRUE(instance_->heldVpnFdMap_.empty());

    int32_t ipcFd = INVALID_TUN_FD;
    EXPECT_EQ(instance_->HoldVpnInterfaceFd("", -1, ipcFd), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    EXPECT_EQ(ipcFd, INVALID_TUN_FD);
    EXPECT_TRUE(instance_->heldVpnFdMap_.empty());

    int fds[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    ASSERT_EQ(instance_->HoldVpnInterfaceFd("", fds[0], ipcFd), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(ipcFd, fds[0]);
    int32_t heldFd = instance_->heldVpnFdMap_[""];
    EXPECT_GE(heldFd, 0);
    EXPECT_NE(heldFd, ipcFd);

    close(ipcFd); // simulate the stub closing the out fd after the reply is written
    EXPECT_NE(fcntl(heldFd, F_GETFD), -1); // the held dup must stay valid

    instance_->CloseHeldVpnFd(""); // destroy path closes the held dup exactly once
    EXPECT_EQ(fcntl(heldFd, F_GETFD), -1);
    instance_->CloseHeldVpnFd(""); // idempotent
    EXPECT_TRUE(instance_->heldVpnFdMap_.empty());

    // re-setup of the same vpnId replaces the held fd and closes the previous one
    int fds2[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds2), 0);
    ASSERT_EQ(instance_->HoldVpnInterfaceFd("", fds2[0], ipcFd), NETMANAGER_EXT_SUCCESS);
    int32_t heldFd2 = instance_->heldVpnFdMap_[""];
    EXPECT_NE(heldFd2, heldFd);
    EXPECT_NE(fcntl(heldFd2, F_GETFD), -1);
    instance_->CloseHeldVpnFd("");
    close(ipcFd);
    close(fds[1]);
    close(fds2[1]);
}

// HoldVpnInterfaceFd dup 失败分支：dup 仅在进程 fd 耗尽（EMFILE）时失败，
// 压低 RLIMIT_NOFILE 模拟，验证失败路径接管并关闭入参 fd（不泄漏）且不写脏 map
HWTEST_F(NetworkVpnServiceTest, HoldVpnInterfaceFdDupFail001, TestSize.Level1)
{
    instance_->CloseHeldVpnFd("");
    int fds[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    struct rlimit origin = {};
    ASSERT_EQ(getrlimit(RLIMIT_NOFILE, &origin), 0);
    struct rlimit zero = {0, origin.rlim_max};
    ASSERT_EQ(setrlimit(RLIMIT_NOFILE, &zero), 0);
    int32_t ipcFd = INVALID_TUN_FD;
    EXPECT_EQ(instance_->HoldVpnInterfaceFd("", fds[0], ipcFd), NETMANAGER_EXT_ERR_INTERNAL);
    EXPECT_EQ(ipcFd, INVALID_TUN_FD);
    EXPECT_TRUE(instance_->heldVpnFdMap_.empty());
    ASSERT_EQ(setrlimit(RLIMIT_NOFILE, &origin), 0);
    EXPECT_EQ(fcntl(fds[0], F_GETFD), -1);
    close(fds[1]);
}

// CloseHeldVpnFdOfVpn 取 key 行为：vpnObj 为空 / config 为空按单 VPN key，正常按 config.vpnId_
HWTEST_F(NetworkVpnServiceTest, CloseHeldVpnFdOfVpn001, TestSize.Level1)
{
    instance_->CloseHeldVpnFd("");
    int32_t userId = AppExecFwk::Constants::DEFAULT_USERID;
    std::vector<int32_t> activeUserIds;
    int32_t ipcFd = INVALID_TUN_FD;

    int chan[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, chan), 0);
    ASSERT_EQ(instance_->HoldVpnInterfaceFd("", chan[0], ipcFd), NETMANAGER_EXT_SUCCESS);
    int32_t heldFd = instance_->heldVpnFdMap_[""];
    close(ipcFd);
    instance_->CloseHeldVpnFdOfVpn(nullptr);
    EXPECT_EQ(fcntl(heldFd, F_GETFD), -1);
    EXPECT_TRUE(instance_->heldVpnFdMap_.empty());
    close(chan[1]);

    sptr<VpnConfig> nullConfig = nullptr;
    std::shared_ptr<NetVpnImpl> nullConfigVpn =
        std::make_shared<ExtendedVpnCtl>(nullConfig, "", userId, activeUserIds);
    int chan2[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, chan2), 0);
    ASSERT_EQ(instance_->HoldVpnInterfaceFd("", chan2[0], ipcFd), NETMANAGER_EXT_SUCCESS);
    heldFd = instance_->heldVpnFdMap_[""];
    close(ipcFd);
    instance_->CloseHeldVpnFdOfVpn(nullConfigVpn);
    EXPECT_EQ(fcntl(heldFd, F_GETFD), -1);
    EXPECT_TRUE(instance_->heldVpnFdMap_.empty());
    close(chan2[1]);

    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    config->vpnId_ = "holdfdvpn001";
    std::shared_ptr<NetVpnImpl> vpnObj = std::make_shared<ExtendedVpnCtl>(config, "", userId, activeUserIds);
    int chan3[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, chan3), 0);
    ASSERT_EQ(instance_->HoldVpnInterfaceFd(config->vpnId_, chan3[0], ipcFd), NETMANAGER_EXT_SUCCESS);
    heldFd = instance_->heldVpnFdMap_[config->vpnId_];
    close(ipcFd);
    instance_->CloseHeldVpnFdOfVpn(vpnObj);
    EXPECT_EQ(fcntl(heldFd, F_GETFD), -1);
    EXPECT_TRUE(instance_->heldVpnFdMap_.empty());
    close(chan3[1]);
}

// OnProcessDied 单 VPN 销毁分支：currSetUpVpnPid_ 命中时同步关闭钉持 fd（对齐 4f6fff0 清理路径）
HWTEST_F(NetworkVpnServiceTest, OnProcessDiedHeldVpnFd001, TestSize.Level1)
{
    instance_->CloseHeldVpnFd("");
    int32_t userId = AppExecFwk::Constants::DEFAULT_USERID;
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    std::vector<int32_t> activeUserIds;
    instance_->vpnObj_ = std::make_shared<ExtendedVpnCtl>(config, "", userId, activeUserIds);
    int32_t ipcFd = INVALID_TUN_FD;
    int chan[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, chan), 0);
    ASSERT_EQ(instance_->HoldVpnInterfaceFd("", chan[0], ipcFd), NETMANAGER_EXT_SUCCESS);
    int32_t heldFd = instance_->heldVpnFdMap_[""];
    close(ipcFd);
    AppExecFwk::ProcessData data;
    data.pid = 123456;
    data.uid = userId;
    sptr<NetworkVpnService::VpnHapObserver> vpnHapObserver = new NetworkVpnService::VpnHapObserver(
        *instance_, "testBundleName", "testAbility");
    instance_->setVpnPidMap_.emplace(userId, 123456);
    instance_->currSetUpVpnPid_ = 123456;
    vpnHapObserver->OnProcessDied(data);
    EXPECT_EQ(fcntl(heldFd, F_GETFD), -1);
    EXPECT_TRUE(instance_->heldVpnFdMap_.empty());
    close(chan[1]);
}

// OnRemoteDied 单 VPN 销毁分支：远端事件回调宿主死亡注销成功时同步关闭钉持 fd
HWTEST_F(NetworkVpnServiceTest, OnRemoteDiedHeldVpnFd001, TestSize.Level1)
{
    instance_->CloseHeldVpnFd("");
    int32_t userId = AppExecFwk::Constants::DEFAULT_USERID;
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    std::vector<int32_t> activeUserIds;
    instance_->vpnObj_ = std::make_shared<ExtendedVpnCtl>(config, "", userId, activeUserIds);
    int32_t ipcFd = INVALID_TUN_FD;
    int chan[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, chan), 0);
    ASSERT_EQ(instance_->HoldVpnInterfaceFd("", chan[0], ipcFd), NETMANAGER_EXT_SUCCESS);
    int32_t heldFd = instance_->heldVpnFdMap_[""];
    close(ipcFd);
    instance_->vpnEventCallbacks_.push_back(eventCallback_);
    wptr<IRemoteObject> remote = eventCallback_->AsObject().GetRefPtr();
    instance_->OnRemoteDied(remote);
    EXPECT_EQ(fcntl(heldFd, F_GETFD), -1);
    EXPECT_TRUE(instance_->heldVpnFdMap_.empty());
    close(chan[1]);
}

// NotifyConnectStage DISCONNECT 单 VPN 分支：配置本地地址且销毁成功时同步关闭钉持 fd
#ifdef SUPPORT_SYSVPN
HWTEST_F(NetworkVpnServiceTest, NotifyConnectStageHeldVpnFd001, TestSize.Level1)
{
    instance_->CloseHeldVpnFd("");
    int32_t userId = AppExecFwk::Constants::DEFAULT_USERID;
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    INetAddr addr;
    addr.address_ = "10.10.10.1";
    config->addresses_.push_back(addr);
    config->addresses_.push_back(addr); // CheckVpnHasLocalAddr 要求 addresses_.size() > VPN_LOCAL_IP_INDEX(1)
    std::vector<int32_t> activeUserIds;
    instance_->vpnObj_ = std::make_shared<ExtendedVpnCtl>(config, "", userId, activeUserIds);
    int32_t ipcFd = INVALID_TUN_FD;
    int chan[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, chan), 0);
    ASSERT_EQ(instance_->HoldVpnInterfaceFd("", chan[0], ipcFd), NETMANAGER_EXT_SUCCESS);
    int32_t heldFd = instance_->heldVpnFdMap_[""];
    close(ipcFd);
    EXPECT_EQ(instance_->NotifyConnectStage("disconnect", 0), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(fcntl(heldFd, F_GETFD), -1);
    EXPECT_TRUE(instance_->heldVpnFdMap_.empty());
    close(chan[1]);
}
#endif // SUPPORT_SYSVPN

HWTEST_F(NetworkVpnServiceTest, ProcessVpnConfigTest001, TestSize.Level1)
{
    VpnConfigRawData configData;
    sptr<VpnConfig> configPtr = new (std::nothrow) VpnConfig();
    ASSERT_NE(configPtr, nullptr);
    EXPECT_EQ(true, configData.SerializeFromVpnConfig(*configPtr));

    std::string vpnBundleName;
    int32_t userId = AppExecFwk::Constants::UNSPECIFIED_USERID;
    std::vector<int32_t> activeUserIds;
    VpnConfig config;
    int32_t result = instance_->ProcessVpnConfig(configData, vpnBundleName, userId, activeUserIds, config);
    EXPECT_NE(result, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, ProcessVpnConfigTest002, TestSize.Level1)
{
    VpnConfigRawData configData;
    std::string vpnBundleName;
    int32_t userId = AppExecFwk::Constants::UNSPECIFIED_USERID;
    std::vector<int32_t> activeUserIds;
    VpnConfig config;
    int32_t result = instance_->ProcessVpnConfig(configData, vpnBundleName, userId, activeUserIds, config);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(NetworkVpnServiceTest, ProcessVpnConfigTest003, TestSize.Level1)
{
    VpnConfigRawData configData;
    sptr<VpnConfig> configPtr = new (std::nothrow) VpnConfig();
    ASSERT_NE(configPtr, nullptr);
    configPtr->vpnId_ = "invalid_vpn_id";
    EXPECT_EQ(true, configData.SerializeFromVpnConfig(*configPtr));

    std::string vpnBundleName;
    int32_t userId = AppExecFwk::Constants::UNSPECIFIED_USERID;
    std::vector<int32_t> activeUserIds;
    VpnConfig config;
    int32_t result = instance_->ProcessVpnConfig(configData, vpnBundleName, userId, activeUserIds, config);
    EXPECT_NE(result, NETMANAGER_EXT_SUCCESS);
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
    EXPECT_LE(instance_->SyncRegisterVpnEvent(eventCallback_), NETMANAGER_EXT_ERR_OPERATION_FAILED);
}

HWTEST_F(NetworkVpnServiceTest, SyncUnregisterVpnEvent, TestSize.Level1)
{
    EXPECT_LE(instance_->SyncUnregisterVpnEvent(eventCallback_), NETMANAGER_EXT_ERR_OPERATION_FAILED);
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
    EXPECT_LE(ret, NETMANAGER_EXT_ERR_OPERATION_FAILED);
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
    EXPECT_TRUE(instance_->vpnObj_ == nullptr);
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
    bool isMainProc = false;
    instance_->setVpnPidMap_ = {
        {1, 2},
        {3, 4}};
    EXPECT_TRUE(instance_->IsCurrentVpnPid(uid, pid, isMainProc));
    pid = 1;
    EXPECT_FALSE(instance_->IsCurrentVpnPid(uid, pid, isMainProc));
}

HWTEST_F(NetworkVpnServiceTest, IsCurrentVpnPidTest002, TestSize.Level1)
{
    constexpr int32_t testUid = 200;
    constexpr int32_t testPid = 123456;
    constexpr int32_t extPid = 9999;
    bool isMainProc = false;
    instance_->setVpnPidMap_.clear();
    instance_->currSetUpVpnPid_ = extPid;
    instance_->hasOpenedVpnUid_ = testUid;
    isMainProc = false;
    EXPECT_FALSE(instance_->IsCurrentVpnPid(testUid, testPid, isMainProc,
        AppExecFwk::ProcessType::EXTENSION, AppExecFwk::ExtensionAbilityType::VPN));
    EXPECT_FALSE(isMainProc);
}

HWTEST_F(NetworkVpnServiceTest, IsCurrentVpnPidTest003, TestSize.Level1)
{
    constexpr int32_t testUid = 200;
    constexpr int32_t testPid = 123456;
    constexpr int32_t extPid = 9999;
    bool isMainProc = false;
    instance_->setVpnPidMap_.clear();
    instance_->currSetUpVpnPid_ = extPid;
    instance_->hasOpenedVpnUid_ = testUid;
    isMainProc = false;
    EXPECT_FALSE(instance_->IsCurrentVpnPid(testUid, testPid, isMainProc,
        AppExecFwk::ProcessType::RENDER, AppExecFwk::ExtensionAbilityType::UNSPECIFIED));
    EXPECT_FALSE(isMainProc);
}

HWTEST_F(NetworkVpnServiceTest, IsCurrentVpnPidTest004, TestSize.Level1)
{
    constexpr int32_t testUid = 200;
    constexpr int32_t testPid = 123456;
    constexpr int32_t extPid = 9999;
    bool isMainProc = false;
    instance_->setVpnPidMap_.clear();
    instance_->currSetUpVpnPid_ = extPid;
    instance_->hasOpenedVpnUid_ = testUid;
    isMainProc = false;
    EXPECT_TRUE(instance_->IsCurrentVpnPid(testUid, testPid, isMainProc,
        AppExecFwk::ProcessType::NORMAL, AppExecFwk::ExtensionAbilityType::UNSPECIFIED));
    EXPECT_TRUE(isMainProc);
}

HWTEST_F(NetworkVpnServiceTest, VpnExtensionAbilityTest001, TestSize.Level1)
{
    OHOS::AAFwk::Want want;
    EXPECT_NE(instance_->StartVpnExtensionAbility(want), ERR_OK);
    EXPECT_NE(instance_->StopVpnExtensionAbility(want), ERR_OK);
}

/**
 * @tc.name: ProtectSyncContextParseParams001
 * @tc.desc: Verify protectSync rejects a call without a socket fd.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkVpnServiceTest, ProtectSyncContextParseParams001, TestSize.Level1)
{
    std::shared_ptr<EventManager> manager;
    ProtectSyncContext context(nullptr, manager);

    context.ParseParams(nullptr, 0);

    EXPECT_FALSE(context.IsParseOK());
    EXPECT_EQ(context.socketFd_, -1);
}

/**
 * @tc.name: ProtectSyncContextParseParams002
 * @tc.desc: Verify protectSync rejects the asynchronous callback overload.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkVpnServiceTest, ProtectSyncContextParseParams002, TestSize.Level1)
{
    std::shared_ptr<EventManager> manager;
    ProtectSyncContext context(nullptr, manager);
    napi_value params[] = { nullptr, nullptr };

    context.ParseParams(params, 2);

    EXPECT_FALSE(context.IsParseOK());
    EXPECT_EQ(context.socketFd_, -1);
}

/**
 * @tc.name: ProtectSyncContextParseParams003
 * @tc.desc: Verify protectSync delegates single-argument type validation.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkVpnServiceTest, ProtectSyncContextParseParams003, TestSize.Level1)
{
    std::shared_ptr<EventManager> manager;
    ProtectSyncContext context(nullptr, manager);
    napi_value params[] = { nullptr };

    context.ParseParams(params, 1);

    EXPECT_FALSE(context.IsParseOK());
    EXPECT_EQ(context.socketFd_, -1);
}

HWTEST_F(NetworkVpnServiceTest, IsNeedNotifyTest001, TestSize.Level1)
{
    std::string vpnId = "";
    VpnConnectState state = VpnConnectState::VPN_DISCONNECTED;
    bool res = instance_->IsNeedNotify(state, vpnId);
    vpnId = "test";
    res = instance_->IsNeedNotify(state, vpnId);
    state = VpnConnectState::VPN_CONNECTED;
    res = instance_->IsNeedNotify(state, vpnId);
    vpnId = "";
    res = instance_->IsNeedNotify(state, vpnId);
    EXPECT_TRUE(res);
}
 
HWTEST_F(NetworkVpnServiceTest, IsNeedNotifyTest002, TestSize.Level1)
{
    int32_t userId = AppExecFwk::Constants::DEFAULT_USERID;
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    std::vector<int32_t> activeUserIds;
    instance_->vpnObj_ = std::make_shared<ExtendedVpnCtl>(config, "", userId, activeUserIds);
 
    VpnConnectState state = VpnConnectState::VPN_DISCONNECTED;
    std::string vpnId = "test";
    bool res = instance_->IsNeedNotify(state, vpnId);
    vpnId = "";
    res = instance_->IsNeedNotify(state, vpnId);
    EXPECT_TRUE(res);
}

HWTEST_F(NetworkVpnServiceTest, RequestVpnPermissionTest001, TestSize.Level1)
{
    auto vpnService = std::make_shared<NetworkVpnService>();
    int32_t uid = 1000;
    std::string bundleName = "com.test.bundle";
    std::string abilityName = "VpnServiceExtAbility";
    bool isAuthorized = true;
    int32_t result = vpnService->RequestVpnPermission(uid, bundleName, abilityName, isAuthorized);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
    EXPECT_FALSE(isAuthorized);
}

HWTEST_F(NetworkVpnServiceTest, RequestVpnPermissionTest002, TestSize.Level1)
{
    auto vpnService = std::make_shared<NetworkVpnService>();
    int32_t uid = 1099;
    std::string bundleName = "";
    std::string abilityName = "VpnServiceExtAbility";
    bool isAuthorized = true;
    int32_t result = vpnService->RequestVpnPermission(uid, bundleName, abilityName, isAuthorized);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    EXPECT_FALSE(isAuthorized);
}

HWTEST_F(NetworkVpnServiceTest, RequestVpnPermissionTest003, TestSize.Level1)
{
    auto vpnService = std::make_shared<MockNetworkVpnService>();
    EXPECT_CALL(*vpnService, ShowVpnDialog(_, _, _, _)).WillRepeatedly(Return(false));
    int32_t uid = 1099;
    std::string bundleName = "";
    std::string abilityName = "VpnServiceExtAbility";
    bool isAuthorized = false;
    vpnService->allowVpnStartWithoutCheckPermissions_.emplace("");
    int32_t result = vpnService->RequestVpnPermission(uid, bundleName, abilityName, isAuthorized);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    EXPECT_TRUE(isAuthorized);
}

HWTEST_F(NetworkVpnServiceTest, RequestVpnPermissionTest004, TestSize.Level1)
{
    auto vpnService = std::make_shared<MockNetworkVpnService>();
    EXPECT_CALL(*vpnService, ShowVpnDialog(_, _, _, _)).WillRepeatedly(Return(true));
    int32_t uid = 1099;
    std::string bundleName = "";
    std::string abilityName = "VpnServiceExtAbility";
    bool isAuthorized = false;
    vpnService->allowVpnStartWithoutCheckPermissions_.emplace("");
    int32_t result = vpnService->RequestVpnPermission(uid, bundleName, abilityName, isAuthorized);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    EXPECT_TRUE(isAuthorized);
}

HWTEST_F(NetworkVpnServiceTest, OnAbilityDisconnectDone001, TestSize.Level1)
{
    auto vpnService = std::make_shared<NetworkVpnService>();
    auto vpnAbilityConnect = std::make_shared<NetworkVpnService::VpnAbilityConnect>(vpnService, "",
        "VpnServiceExtAbility", 1099);
    AppExecFwk::ElementName elememt;
    elememt.SetBundleName("com.test.bundle");
    vpnAbilityConnect->OnAbilityDisconnectDone(elememt, 0);
    EXPECT_NE(elememt.GetBundleName(), "com.huawei.hmos.vpndialog");
}

HWTEST_F(NetworkVpnServiceTest, OnAbilityDisconnectDone002, TestSize.Level1)
{
    auto vpnService = std::make_shared<NetworkVpnService>();
    auto vpnAbilityConnect = std::make_shared<NetworkVpnService::VpnAbilityConnect>(vpnService, "",
        "VpnServiceExtAbility", 1099);
    AppExecFwk::ElementName elememt;
    elememt.SetBundleName("com.huawei.hmos.vpndialog");
    vpnAbilityConnect->OnAbilityDisconnectDone(elememt, 0);
    EXPECT_EQ(elememt.GetBundleName(), "com.huawei.hmos.vpndialog");
    EXPECT_NE(vpnAbilityConnect->service_.lock(), nullptr);
}

HWTEST_F(NetworkVpnServiceTest, OnAbilityDisconnectDone003, TestSize.Level1)
{
    std::shared_ptr<NetworkVpnService> vpnService = nullptr;
    auto vpnAbilityConnect = std::make_shared<NetworkVpnService::VpnAbilityConnect>(vpnService, "",
        "VpnServiceExtAbility", 1099);
    AppExecFwk::ElementName elememt;
    elememt.SetBundleName("com.huawei.hmos.vpndialog");
    vpnAbilityConnect->OnAbilityDisconnectDone(elememt, 0);
    EXPECT_EQ(elememt.GetBundleName(), "com.huawei.hmos.vpndialog");
    EXPECT_EQ(vpnAbilityConnect->service_.lock(), nullptr);
}

HWTEST_F(NetworkVpnServiceTest, CheckVpnExtPermission001, TestSize.Level1)
{
    auto vpnService = std::make_shared<NetworkVpnService>();
    vpnService->vpnExtPermissionCache_.emplace("test_0", true);
    auto ret = vpnService->CheckVpnExtPermission(1099, "test");
    EXPECT_TRUE(ret);
}

HWTEST_F(NetworkVpnServiceTest, CheckVpnExtPermission002, TestSize.Level1)
{
    auto vpnService = std::make_shared<NetworkVpnService>();
    auto ret = vpnService->CheckVpnExtPermission(1099, "test");
    EXPECT_FALSE(ret);
}

HWTEST_F(NetworkVpnServiceTest, VpnFdChannelConnectControl001, TestSize.Level1)
{
    VpnFdChannel vpnFdChannel;
    int32_t fd = -1;
    int32_t nsec = 1;
    int32_t ret = vpnFdChannel.ConnectControl(fd, nsec);
    EXPECT_EQ(NETMANAGER_EXT_ERR_INTERNAL, ret);

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    ASSERT_GE(fd, 0);
    ret = vpnFdChannel.ConnectControl(fd, nsec);
    EXPECT_GE(ret, NETMANAGER_EXT_SUCCESS);
    close(fd);
}

HWTEST_F(NetworkVpnServiceTest, VpnFdChannelRecvMsgFromUnixServer001, TestSize.Level1)
{
    VpnFdChannel vpnFdChannel;
    int32_t fd = -1;
    int32_t ret = vpnFdChannel.RecvMsgFromUnixServer(fd);
    EXPECT_EQ(NETMANAGER_EXT_ERR_INTERNAL, ret);
}

HWTEST_F(NetworkVpnServiceTest, VpnFdChannelGetVpnInterfaceFd001, TestSize.Level1)
{
    VpnFdChannel vpnFdChannel;
    int32_t fd = vpnFdChannel.GetVpnInterfaceFd();
    EXPECT_GE(fd, -1);
}

HWTEST_F(NetworkVpnServiceTest, VpnFdChannelRecvMsgFromUnixServer002, TestSize.Level1)
{
    VpnFdChannel vpnFdChannel;
    int chan[2] = {-1, -1};
    int payload[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, chan), 0);
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, payload), 0);

    // 正确的 SCM_RIGHTS 单 fd 递交：成功且 tunFd_ 被赋值
    char data = 'x';
    iovec iov = {&data, sizeof(data)};
    union {
        cmsghdr align;
        char buf[CMSG_SPACE(sizeof(int32_t))];
    } cmsgu = {};
    msghdr msg = {};
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsgu.buf;
    msg.msg_controllen = sizeof(cmsgu.buf);
    cmsghdr *cmsgh = CMSG_FIRSTHDR(&msg);
    ASSERT_NE(cmsgh, nullptr);
    cmsgh->cmsg_level = SOL_SOCKET;
    cmsgh->cmsg_type = SCM_RIGHTS;
    cmsgh->cmsg_len = CMSG_LEN(sizeof(int32_t));
    ASSERT_EQ(memcpy_s(CMSG_DATA(cmsgh), sizeof(int32_t), &payload[0], sizeof(int32_t)), EOK);
    ASSERT_EQ(sendmsg(chan[0], &msg, 0), static_cast<ssize_t>(1));
    EXPECT_EQ(vpnFdChannel.RecvMsgFromUnixServer(chan[1]), NETMANAGER_EXT_SUCCESS);
    // SCM_RIGHTS 接收侧由内核生成新的 fd 号，不可能等于发送侧仍打开的 payload[0]
    EXPECT_GE(vpnFdChannel.tunFd_.load(), 0);

    // 无辅助数据：CMSG_FIRSTHDR 为空分支
    msghdr plainMsg = {};
    plainMsg.msg_iov = &iov;
    plainMsg.msg_iovlen = 1;
    ASSERT_EQ(sendmsg(chan[0], &plainMsg, 0), static_cast<ssize_t>(1));
    EXPECT_EQ(vpnFdChannel.RecvMsgFromUnixServer(chan[1]), NETMANAGER_EXT_ERR_INTERNAL);

    close(chan[0]);
    close(chan[1]);
    close(payload[1]);
    vpnFdChannel.CloseVpnInterfaceFd();
}

HWTEST_F(NetworkVpnServiceTest, VpnFdChannelCloseVpnInterfaceFd001, TestSize.Level1)
{
    VpnFdChannel vpnFdChannel;
    int fds[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    vpnFdChannel.tunFd_ = fds[0];
    vpnFdChannel.CloseVpnInterfaceFd();
    EXPECT_EQ(vpnFdChannel.tunFd_.load(), INVALID_TUN_FD);
    EXPECT_EQ(write(fds[0], "x", 1), static_cast<ssize_t>(-1));
    EXPECT_EQ(errno, EBADF);
    close(fds[1]);
}

HWTEST_F(NetworkVpnServiceTest, FetchVpnInterfaceFdTest001, TestSize.Level1)
{
    NetworkVpnService service;
    EXPECT_GE(service.FetchVpnInterfaceFd("test-vpn-id", "multitun-vpn0"), INVALID_TUN_FD);
    EXPECT_GE(service.FetchVpnInterfaceFd("", ""), INVALID_TUN_FD);
}

HWTEST_F(NetworkVpnServiceTest, HeldVpnFdTest001, TestSize.Level1)
{
    NetworkVpnService service;
    int fds1[2] = {-1, -1};
    int fds2[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds1), 0);
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds2), 0);
    const std::string vpnId = "held-vpn-test";

    service.StoreHeldVpnFd(vpnId, fds1[0]);
    service.StoreHeldVpnFd(vpnId, fds2[0]);
    EXPECT_EQ(service.heldVpnFdMap_.size(), static_cast<size_t>(1));
    EXPECT_EQ(service.heldVpnFdMap_[vpnId], fds2[0]);
    service.CloseHeldVpnFd(vpnId);
    EXPECT_TRUE(service.heldVpnFdMap_.empty());
    service.CloseHeldVpnFd(vpnId);
    EXPECT_TRUE(service.heldVpnFdMap_.empty());

    service.StoreHeldVpnFd(vpnId, INVALID_TUN_FD);
    service.CloseHeldVpnFd(vpnId);
    EXPECT_TRUE(service.heldVpnFdMap_.empty());

    close(fds1[1]);
    close(fds2[1]);
}

HWTEST_F(NetworkVpnServiceTest, GetConnectedVpnConfigList001, TestSize.Level1)
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
    std::vector<sptr<VpnConfig>> vpnConfigList;
    EXPECT_EQ(instance_->GetConnectedVpnConfigList(0, vpnConfigList), NETMANAGER_EXT_SUCCESS);
    EXPECT_TRUE(vpnConfigList.empty());
    EXPECT_EQ(instance_->GetConnectedVpnConfigList(-1, vpnConfigList), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    EXPECT_TRUE(vpnConfigList.empty());

    SetSelfTokenID(tokenIdBak);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

HWTEST_F(NetworkVpnServiceTest, GetConnectedVpnConfigList002, TestSize.Level1)
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

    int32_t userId = AppExecFwk::Constants::DEFAULT_USERID;
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    ASSERT_NE(config, nullptr);
    std::vector<int32_t> activeUserIds;
    instance_->vpnObj_ = std::make_shared<ExtendedVpnCtl>(config, "", userId, activeUserIds);
    std::vector<sptr<VpnConfig>> vpnConfigList;
    // the connected state is recorded in NetVpnImpl::SetUp(): before it the vpn is not reported
    EXPECT_EQ(instance_->GetConnectedVpnConfigList(userId, vpnConfigList), NETMANAGER_EXT_SUCCESS);
    EXPECT_TRUE(vpnConfigList.empty());
    instance_->vpnObj_->isVpnConnecting_ = true;
    EXPECT_EQ(instance_->GetConnectedVpnConfigList(userId, vpnConfigList), NETMANAGER_EXT_SUCCESS);
    ASSERT_EQ(vpnConfigList.size(), 1u);
    EXPECT_EQ(vpnConfigList[0], config);
    // reusing the out vector must replace the previous result, not append to it
    EXPECT_EQ(instance_->GetConnectedVpnConfigList(userId, vpnConfigList), NETMANAGER_EXT_SUCCESS);
    ASSERT_EQ(vpnConfigList.size(), 1u);
    EXPECT_EQ(vpnConfigList[0], config);

    // a different user must not see this vpn
    std::vector<sptr<VpnConfig>> otherUserList;
    EXPECT_EQ(instance_->GetConnectedVpnConfigList(userId + 1, otherUserList), NETMANAGER_EXT_SUCCESS);
    EXPECT_TRUE(otherUserList.empty());

    // a global vpn without a valid config must be skipped even when connected
    auto nullConfigObj = std::make_shared<ExtendedVpnCtl>(nullptr, "", userId, activeUserIds);
    nullConfigObj->isVpnConnecting_ = true;
    instance_->vpnObj_ = nullConfigObj;
    std::vector<sptr<VpnConfig>> nullConfigList;
    EXPECT_EQ(instance_->GetConnectedVpnConfigList(userId, nullConfigList), NETMANAGER_EXT_SUCCESS);
    EXPECT_TRUE(nullConfigList.empty());
    instance_->vpnObj_ = nullptr;
    SetSelfTokenID(tokenIdBak);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

#ifdef SUPPORT_SYSVPN
HWTEST_F(NetworkVpnServiceTest, GetConnectedVpnConfigList003, TestSize.Level1)
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

    int32_t userId = AppExecFwk::Constants::DEFAULT_USERID;
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    ASSERT_NE(config, nullptr);
    std::vector<int32_t> activeUserIds;
    std::shared_ptr<NetVpnImpl> vpnObj = std::make_shared<ExtendedVpnCtl>(config, "", userId, activeUserIds);

    // a nullptr entry must be skipped
    instance_->vpnObjMap_.insert({"GetConnectedVpnConfigList003", nullptr});
    std::vector<sptr<VpnConfig>> vpnConfigList;
    EXPECT_EQ(instance_->GetConnectedVpnConfigList(userId, vpnConfigList), NETMANAGER_EXT_SUCCESS);
    EXPECT_TRUE(vpnConfigList.empty());
    instance_->vpnObjMap_.erase("GetConnectedVpnConfigList003");

    // an entry without multiVpnInfo_ must be skipped
    instance_->vpnObjMap_.insert({"GetConnectedVpnConfigList003", vpnObj});
    EXPECT_EQ(instance_->GetConnectedVpnConfigList(userId, vpnConfigList), NETMANAGER_EXT_SUCCESS);
    EXPECT_TRUE(vpnConfigList.empty());

    // a disconnected entry must be skipped
    sptr<MultiVpnInfo> multiVpnInfo = new (std::nothrow) MultiVpnInfo();
    ASSERT_NE(multiVpnInfo, nullptr);
    multiVpnInfo->vpnId = "GetConnectedVpnConfigList003";
    multiVpnInfo->userId = userId;
    vpnObj->multiVpnInfo_ = multiVpnInfo;
    EXPECT_EQ(instance_->GetConnectedVpnConfigList(userId, vpnConfigList), NETMANAGER_EXT_SUCCESS);
    EXPECT_TRUE(vpnConfigList.empty());

    // a connected entry of another user must be skipped
    multiVpnInfo->vpnConnectState = VpnConnectState::VPN_CONNECTED;
    EXPECT_EQ(instance_->GetConnectedVpnConfigList(userId + 1, vpnConfigList), NETMANAGER_EXT_SUCCESS);
    EXPECT_TRUE(vpnConfigList.empty());

    // a connected entry of the requested user is returned
    EXPECT_EQ(instance_->GetConnectedVpnConfigList(userId, vpnConfigList), NETMANAGER_EXT_SUCCESS);
    instance_->vpnObjMap_.erase("GetConnectedVpnConfigList003");
    ASSERT_EQ(vpnConfigList.size(), 1u);
    EXPECT_EQ(vpnConfigList[0], config);
    SetSelfTokenID(tokenIdBak);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

HWTEST_F(NetworkVpnServiceTest, GetConnectedVpnConfigList004, TestSize.Level1)
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

    int32_t userId = AppExecFwk::Constants::DEFAULT_USERID;
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    ASSERT_NE(config, nullptr);
    std::vector<int32_t> activeUserIds;
    std::shared_ptr<NetVpnImpl> vpnObj = std::make_shared<ExtendedVpnCtl>(config, "", userId, activeUserIds);
    sptr<MultiVpnInfo> multiVpnInfo = new (std::nothrow) MultiVpnInfo();
    ASSERT_NE(multiVpnInfo, nullptr);
    multiVpnInfo->vpnId = "GetConnectedVpnConfigList004";
    multiVpnInfo->userId = userId;
    multiVpnInfo->vpnConnectState = VpnConnectState::VPN_CONNECTED;
    vpnObj->multiVpnInfo_ = multiVpnInfo;
    instance_->vpnObjMap_.insert({"GetConnectedVpnConfigList004", vpnObj});

    // a connected entry whose config is invalid must be skipped: only the valid one is returned
    std::shared_ptr<NetVpnImpl> invalidObj = std::make_shared<ExtendedVpnCtl>(nullptr, "", userId, activeUserIds);
    sptr<MultiVpnInfo> invalidVpnInfo = new (std::nothrow) MultiVpnInfo();
    ASSERT_NE(invalidVpnInfo, nullptr);
    invalidVpnInfo->vpnId = "GetConnectedVpnConfigList004Invalid";
    invalidVpnInfo->userId = userId;
    invalidVpnInfo->vpnConnectState = VpnConnectState::VPN_CONNECTED;
    invalidObj->multiVpnInfo_ = invalidVpnInfo;
    instance_->vpnObjMap_.insert({"GetConnectedVpnConfigList004Invalid", invalidObj});
    std::vector<sptr<VpnConfig>> vpnConfigList;
    EXPECT_EQ(instance_->GetConnectedVpnConfigList(userId, vpnConfigList), NETMANAGER_EXT_SUCCESS);
    instance_->vpnObjMap_.erase("GetConnectedVpnConfigList004");
    instance_->vpnObjMap_.erase("GetConnectedVpnConfigList004Invalid");
    ASSERT_EQ(vpnConfigList.size(), 1u);
    EXPECT_EQ(vpnConfigList[0], config);
    SetSelfTokenID(tokenIdBak);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}
#endif // SUPPORT_SYSVPN

HWTEST_F(NetworkVpnServiceTest, GetConnectedVpnConfigList005, TestSize.Level1)
{
    // a system caller without the MANAGE_VPN permission must be rejected by the permission gate
    const char* permissions[] = { "ohos.permission.GET_NETWORK_INFO" };
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

    std::vector<sptr<VpnConfig>> vpnConfigList;
    sptr<VpnConfig> stale = new (std::nothrow) VpnConfig();
    ASSERT_NE(stale, nullptr);
    vpnConfigList.push_back(stale);
    // the permission gate must not touch the caller's out param
    EXPECT_EQ(instance_->GetConnectedVpnConfigList(0, vpnConfigList), NETMANAGER_ERR_PERMISSION_DENIED);
    ASSERT_EQ(vpnConfigList.size(), 1u);
    EXPECT_EQ(vpnConfigList[0], stale);

    SetSelfTokenID(tokenIdBak);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}
} // namespace NetManagerStandard
} // namespace OHOS
