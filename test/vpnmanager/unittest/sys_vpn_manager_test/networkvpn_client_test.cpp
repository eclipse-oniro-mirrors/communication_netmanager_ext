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

#include <arpa/inet.h>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

#include "fwmark_client.h"
#include "iservice_registry.h"
#include "netmanager_ext_test_security.h"
#include "netmgr_ext_log_wrapper.h"
#include "securec.h"
#include "system_ability_definition.h"

#ifdef GTEST_API_
#define private public
#endif
#include "net_manager_constants.h"
#include "netmanager_base_common_utils.h"
#include "networkvpn_client.h"
#include "vpn_event_callback_stub.h"
#include "ipsecvpn_config.h"
#include "mock_i_network_vpn_service.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing;
using namespace testing::ext;
} // namespace

class NetworkVpnClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    sptr<IVpnEventCallback> callback_ = nullptr;
    NetworkVpnClient &networkVpnClient_ = NetworkVpnClient::GetInstance();
};

void NetworkVpnClientTest::SetUpTestCase() {}

void NetworkVpnClientTest::TearDownTestCase() {}

HWTEST_F(NetworkVpnClientTest, SetUpVpn001, TestSize.Level1)
{
    sptr<SysVpnConfig> config = nullptr;
    int32_t ret = networkVpnClient_.SetUpVpn(config, false);

    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    config = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(config, nullptr);
    sptr<INetAddr> netAddr = new (std::nothrow) INetAddr();
    ASSERT_NE(netAddr, nullptr);
    std::string ip = "1.1.1.1";
    netAddr->address_ = ip;
    netAddr->prefixlen_ = 1;
    config->vpnId_ = "123";
    config->vpnName_ = "testSetUpVpn";
    config->vpnType_ = 1;
    ret = networkVpnClient_.SetUpVpn(config, false);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);

    NetManagerExtAccessToken access;
    ret = networkVpnClient_.SetUpVpn(config, false);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(NetworkVpnClientTest, SetUpVpn002, TestSize.Level1)
{
    sptr<SysVpnConfig> config = nullptr;
    int32_t ret = networkVpnClient_.SetUpVpn(config, true);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PARAMETER_ERROR);

    config = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(config, nullptr);
    config->vpnId_ = "123";
    config->vpnName_ = "testSetUpVpn";
    config->vpnType_ = 1;
    ret = networkVpnClient_.SetUpVpn(config, true);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkVpnClientTest, AddSysVpnConfig001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    std::string id = "1234";
    sptr<SysVpnConfig> config = nullptr;
    int32_t ret = networkVpnClient_.AddSysVpnConfig(config);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    config = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(config, nullptr);
    ret = networkVpnClient_.AddSysVpnConfig(config);
    EXPECT_TRUE(ret == NETMANAGER_EXT_ERR_READ_DATA_FAIL || ret == 5);
    config->vpnId_ = id;
    config->vpnName_ = "test";
    config->vpnType_ = 1;
    ret = networkVpnClient_.AddSysVpnConfig(config);
    EXPECT_TRUE(ret == NETMANAGER_EXT_SUCCESS || ret == NETMANAGER_EXT_ERR_OPERATION_FAILED);
    // delete test config
    auto ret2 = networkVpnClient_.DeleteSysVpnConfig(id);
    EXPECT_TRUE(ret2 == NETMANAGER_EXT_SUCCESS || ret2 == NETMANAGER_EXT_ERR_OPERATION_FAILED);
}

HWTEST_F(NetworkVpnClientTest, AddSysVpnConfig002, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    std::string id = "1234";
    sptr<SysVpnConfig> config = new (std::nothrow) SysVpnConfig();
    ASSERT_NE(config, nullptr);
    config->vpnId_ = id;
    config->vpnName_ = "test";
    config->vpnType_ = 1;
    auto ret = networkVpnClient_.AddSysVpnConfig(config);
    EXPECT_TRUE(ret == NETMANAGER_EXT_ERR_READ_DATA_FAIL || ret == 5);
}

HWTEST_F(NetworkVpnClientTest, AddSysVpnConfig003, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    sptr<SysVpnConfig> config = nullptr;
    EXPECT_EQ(networkVpnClient_.AddSysVpnConfig(config), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetworkVpnClientTest, AddSysVpnConfig004, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    sptr<SysVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(config, nullptr);
    config->vpnId_ = "testId0";
    config->vpnName_ = "test";
    config->vpnType_ = 0;
    config->saveLogin_ = false;
    config->userId_ = 0;
    auto ret = networkVpnClient_.AddSysVpnConfig(config);
    EXPECT_TRUE(ret == NETMANAGER_EXT_ERR_READ_DATA_FAIL || ret == 5);
}

HWTEST_F(NetworkVpnClientTest, DeleteSysVpnConfig001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    std::string id = "1234";
    sptr<SysVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(config, nullptr);
    config->vpnId_ = id;
    config->vpnName_ = "test";
    config->vpnType_ = 1;
    auto ret = networkVpnClient_.AddSysVpnConfig(config);
    EXPECT_TRUE(ret == NETMANAGER_EXT_SUCCESS || ret == NETMANAGER_EXT_ERR_OPERATION_FAILED);
    // delete test config
    std::string emptyStr;
    int32_t ret2 = networkVpnClient_.DeleteSysVpnConfig(emptyStr);
    EXPECT_EQ(ret2, NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    auto ret3 = networkVpnClient_.DeleteSysVpnConfig(id);
    EXPECT_TRUE(ret3 == NETMANAGER_EXT_SUCCESS || ret3 == NETMANAGER_EXT_ERR_OPERATION_FAILED);
}

HWTEST_F(NetworkVpnClientTest, DeleteSysVpnConfig002, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    std::string id;
    EXPECT_EQ(networkVpnClient_.DeleteSysVpnConfig(id), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetworkVpnClientTest, GetSysVpnConfigList001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    std::vector<sptr<SysVpnConfig>> list;
    auto ret = networkVpnClient_.GetSysVpnConfigList(list);
    EXPECT_TRUE(ret == NETMANAGER_EXT_ERR_OPERATION_FAILED || ret == NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnClientTest, GetSysVpnConfigList002, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    std::vector<sptr<SysVpnConfig>> list;
    sptr<SysVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(config, nullptr);
    config->vpnId_ = "testGetList";
    config->vpnName_ = "testList";
    config->vpnType_ = 1;
    auto ret = networkVpnClient_.AddSysVpnConfig(config);
    EXPECT_TRUE(ret == NETMANAGER_EXT_SUCCESS || ret == NETMANAGER_EXT_ERR_OPERATION_FAILED);
    auto ret2 = networkVpnClient_.GetSysVpnConfigList(list);
    EXPECT_TRUE(ret2 == NETMANAGER_EXT_ERR_OPERATION_FAILED || ret2 == NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnClientTest, GetSysVpnConfig001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    std::string id = "1234";
    sptr<SysVpnConfig> resConfig = nullptr;
    auto ret = networkVpnClient_.GetSysVpnConfig(resConfig, id);
    EXPECT_NE(ret, NETMANAGER_EXT_ERR_OPERATION_FAILED);
}

HWTEST_F(NetworkVpnClientTest, GetSysVpnConfig002, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    std::string id;
    sptr<SysVpnConfig> resConfig = nullptr;
    EXPECT_EQ(networkVpnClient_.GetSysVpnConfig(resConfig, id), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetworkVpnClientTest, GetConnectedSysVpnConfig001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    sptr<SysVpnConfig> resConfig = nullptr;
    auto ret = networkVpnClient_.GetConnectedSysVpnConfig(resConfig);
    EXPECT_TRUE(ret == NETMANAGER_EXT_SUCCESS || ret == 5);
}

HWTEST_F(NetworkVpnClientTest, NotifyConnectStage001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    std::string stage = "start";
    int32_t result = NETMANAGER_EXT_SUCCESS;
    EXPECT_EQ(networkVpnClient_.NotifyConnectStage(stage, result), NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL);
}

HWTEST_F(NetworkVpnClientTest, NotifyConnectStage002, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    std::string stage = "config";
    int32_t result = NETMANAGER_EXT_SUCCESS;
    EXPECT_EQ(networkVpnClient_.NotifyConnectStage(stage, result), NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL);
}

HWTEST_F(NetworkVpnClientTest, NotifyConnectStage003, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    std::string stage = "connect";
    int32_t result = NETMANAGER_EXT_SUCCESS;
    EXPECT_EQ(networkVpnClient_.NotifyConnectStage(stage, result), NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL);
}

HWTEST_F(NetworkVpnClientTest, NotifyConnectStage004, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    std::string stage = "xl2tpdstart";
    int32_t result = NETMANAGER_EXT_SUCCESS;
    EXPECT_EQ(networkVpnClient_.NotifyConnectStage(stage, result), NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL);
}

HWTEST_F(NetworkVpnClientTest, NotifyConnectStage005, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    std::string stage = "pppdstart";
    int32_t result = NETMANAGER_EXT_SUCCESS;
    EXPECT_EQ(networkVpnClient_.NotifyConnectStage(stage, result), NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL);
}

HWTEST_F(NetworkVpnClientTest, GetSysVpnCertUri001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    int32_t certType = 0;
    std::string certUri;
    int32_t ret = networkVpnClient_.GetSysVpnCertUri(certType, certUri);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL);
    certType = 2;
    ret = networkVpnClient_.GetSysVpnCertUri(certType, certUri);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL);
    certType = -1;
    ret = networkVpnClient_.GetSysVpnCertUri(certType, certUri);
}

HWTEST_F(NetworkVpnClientTest, GetVpnCertData001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    int32_t certType = 0;
    std::vector<int8_t> certData;
    int32_t ret = networkVpnClient_.GetVpnCertData(certType, certData);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL);
}

HWTEST_F(NetworkVpnClientTest, OnVpnMultiUserSetUp001, TestSize.Level1)
{
    networkVpnClient_.vpnEventCallback_ = new (std::nothrow) VpnSetUpEventCallback();
    ASSERT_NE(networkVpnClient_.vpnEventCallback_, nullptr);
    bool isConnected = false;
    sptr<VpnState> vpnState = new VpnState();
    networkVpnClient_.vpnEventCallback_->OnVpnStateChanged(isConnected, vpnState);
    networkVpnClient_.vpnEventCallback_->OnVpnMultiUserSetUp();
    EXPECT_NE(networkVpnClient_.vpnEventCallback_, nullptr);
}

HWTEST_F(NetworkVpnClientTest, RecoverCallback001, TestSize.Level1)
{
    sptr<VpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(config, nullptr);
    networkVpnClient_.clientVpnConfig_.config = config;
    networkVpnClient_.clientVpnConfig_.isVpnExtCall = true;
    networkVpnClient_.clientVpnConfig_.isInternalChannel = false;
    networkVpnClient_.RecoverCallback();
    EXPECT_NE(networkVpnClient_.clientVpnConfig_.config, nullptr);
}

namespace {
int32_t OpenTestTunFd(const char *ifName)
{
    int32_t fd = open("/dev/tun", O_RDWR | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0) {
        fd = open("/dev/net/tun", O_RDWR | O_NONBLOCK | O_CLOEXEC);
    }
    if (fd < 0) {
        return -1;
    }
    struct ifreq ifr = {};
    if (strcpy_s(ifr.ifr_name, IFNAMSIZ, ifName) != EOK) {
        close(fd);
        return -1;
    }
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    if (ioctl(fd, TUNSETIFF, &ifr) < 0) {
        close(fd);
        return -1;
    }
    return fd;
}

bool IsFdAlive(int32_t fd)
{
    return fcntl(fd, F_GETFD) != -1;
}
} // namespace

HWTEST_F(NetworkVpnClientTest, GetTunIfName001, TestSize.Level1)
{
    char ifName[IFNAMSIZ] = {0};
    EXPECT_FALSE(NetworkVpnClient::GetTunIfName(-1, ifName, IFNAMSIZ));
    int fds[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    // a non-tun fd must be rejected, so a reused fd number can never be misjudged as a live tun fd
    EXPECT_FALSE(NetworkVpnClient::GetTunIfName(fds[0], ifName, IFNAMSIZ));
    EXPECT_FALSE(NetworkVpnClient::GetTunIfName(fds[0], nullptr, IFNAMSIZ));
    EXPECT_FALSE(NetworkVpnClient::GetTunIfName(fds[0], ifName, IFNAMSIZ - 1));
    close(fds[0]);
    close(fds[1]);
}

HWTEST_F(NetworkVpnClientTest, RecoverCallbackKeepAliveTunFd001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    // two fds attached to the same tun device: the old one is still held by the app-side tunnel,
    // the new one is returned by the recovered service (netsysnative reuses the same vpn-tun)
    int32_t oldFd = OpenTestTunFd("vpntestka0");
    int32_t newFd = OpenTestTunFd("vpntestka0");
    if (oldFd < 0 || newFd < 0) {
        if (oldFd >= 0) {
            close(oldFd);
        }
        if (newFd >= 0) {
            close(newFd);
        }
        return; // tun device unavailable in this environment
    }
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    ASSERT_NE(config, nullptr);
    auto mock = sptr<MockINetworkVpnService>::MakeSptr();
    ASSERT_NE(mock, nullptr);
    NetworkVpnClient client;
    client.networkVpnService_ = mock;
    client.clientVpnConfig_.config = config;
    client.tunFd_ = oldFd;

    ON_CALL(*mock, SetUpVpn(_, _, _, _))
        .WillByDefault(DoAll(SetArgReferee<3>(newFd), Return(NETMANAGER_EXT_SUCCESS)));
    client.RecoverCallback();

    // the working old fd must be kept; the extra new dup must be dropped
    EXPECT_EQ(client.tunFd_.load(), oldFd);
    EXPECT_TRUE(IsFdAlive(oldFd));
    EXPECT_FALSE(IsFdAlive(newFd));

    client.clientVpnConfig_.config = nullptr;
    client.networkVpnService_ = nullptr;
    client.CloseTunFd(); // closes oldFd
}

HWTEST_F(NetworkVpnClientTest, RecoverCallbackKeepAliveTunFdDifferentIfName001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    // old and new fds point to different tun devices, but the old fd is still working:
    // the recovery path must keep it as well and never close a working fd
    int32_t oldFd = OpenTestTunFd("vpntestdn0");
    int32_t newFd = OpenTestTunFd("vpntestdn1");
    if (oldFd < 0 || newFd < 0) {
        if (oldFd >= 0) {
            close(oldFd);
        }
        if (newFd >= 0) {
            close(newFd);
        }
        return; // tun device unavailable in this environment
    }
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    ASSERT_NE(config, nullptr);
    auto mock = sptr<MockINetworkVpnService>::MakeSptr();
    ASSERT_NE(mock, nullptr);
    NetworkVpnClient client;
    client.networkVpnService_ = mock;
    client.clientVpnConfig_.config = config;
    client.tunFd_ = oldFd;

    ON_CALL(*mock, SetUpVpn(_, _, _, _))
        .WillByDefault(DoAll(SetArgReferee<3>(newFd), Return(NETMANAGER_EXT_SUCCESS)));
    client.RecoverCallback();

    EXPECT_EQ(client.tunFd_.load(), oldFd);
    EXPECT_TRUE(IsFdAlive(oldFd));
    EXPECT_FALSE(IsFdAlive(newFd));

    client.clientVpnConfig_.config = nullptr;
    client.networkVpnService_ = nullptr;
    client.CloseTunFd(); // closes oldFd
}

HWTEST_F(NetworkVpnClientTest, RecoverCallbackAdoptNewTunFd001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    int fds[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    ASSERT_NE(config, nullptr);
    auto mock = sptr<MockINetworkVpnService>::MakeSptr();
    ASSERT_NE(mock, nullptr);
    NetworkVpnClient client;
    client.networkVpnService_ = mock;
    client.clientVpnConfig_.config = config;
    // the old fd is not a valid tun fd (tun destroyed or the number reused by another open):
    // adopt the new fd, and do not close the stale old number to avoid closing a reused fd
    client.tunFd_ = fds[0];

    ON_CALL(*mock, SetUpVpn(_, _, _, _))
        .WillByDefault(DoAll(SetArgReferee<3>(fds[1]), Return(NETMANAGER_EXT_SUCCESS)));
    client.RecoverCallback();

    EXPECT_EQ(client.tunFd_.load(), fds[1]);
    EXPECT_TRUE(IsFdAlive(fds[1]));
    EXPECT_TRUE(IsFdAlive(fds[0])); // stale old number is not closed

    client.clientVpnConfig_.config = nullptr;
    client.networkVpnService_ = nullptr;
    client.CloseTunFd(); // closes fds[1]
    close(fds[0]);
}

HWTEST_F(NetworkVpnClientTest, RecoverCallbackNoOldTunFd001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    int fds[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    ASSERT_NE(config, nullptr);
    auto mock = sptr<MockINetworkVpnService>::MakeSptr();
    ASSERT_NE(mock, nullptr);
    NetworkVpnClient client;
    client.networkVpnService_ = mock;
    client.clientVpnConfig_.config = config;
    client.tunFd_ = INVALID_TUN_FD;

    ON_CALL(*mock, SetUpVpn(_, _, _, _))
        .WillByDefault(DoAll(SetArgReferee<3>(fds[0]), Return(NETMANAGER_EXT_SUCCESS)));
    client.RecoverCallback();

    EXPECT_EQ(client.tunFd_.load(), fds[0]);
    EXPECT_TRUE(IsFdAlive(fds[0]));

    client.clientVpnConfig_.config = nullptr;
    client.networkVpnService_ = nullptr;
    client.CloseTunFd(); // closes fds[0]
    close(fds[1]);
}

HWTEST_F(NetworkVpnClientTest, RecoverCallbackConcurrentWithSetUpVpn001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    int32_t probe = OpenTestTunFd("vpntestcc0");
    if (probe < 0) {
        return; // tun device unavailable in this environment
    }
    int32_t sentinel = open("/dev/null", O_RDONLY);
    ASSERT_GE(sentinel, 0);
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    ASSERT_NE(config, nullptr);
    auto mock = sptr<MockINetworkVpnService>::MakeSptr();
    ASSERT_NE(mock, nullptr);
    NetworkVpnClient client;
    client.networkVpnService_ = mock;
    client.clientVpnConfig_.config = config;
    ON_CALL(*mock, SetUpVpn(_, _, _, _))
        .WillByDefault(Invoke([](const VpnConfigRawData &, bool, bool, int &tunFd) {
            tunFd = OpenTestTunFd("vpntestcc0");
            return NETMANAGER_EXT_SUCCESS;
        }));

    // the recovery thread and a re-setup loop race on the tun fd critical section:
    // no double close, no mis-close of innocent fds, no crash
    std::thread recoverThread([&client]() {
        for (int i = 0; i < 50; ++i) {
            client.RecoverCallback();
        }
    });
    for (int i = 0; i < 50; ++i) {
        int32_t tunFd = INVALID_TUN_FD;
        client.SetUpVpn(config, tunFd);
    }
    recoverThread.join();

    EXPECT_TRUE(IsFdAlive(sentinel));
    client.clientVpnConfig_.config = nullptr;
    client.networkVpnService_ = nullptr;
    client.CloseTunFd();
    close(sentinel);
    close(probe);
}

HWTEST_F(NetworkVpnClientTest, OnRemoteDied001, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> remote = sam->GetSystemAbility(COMM_VPN_MANAGER_SYS_ABILITY_ID);
    networkVpnClient_.networkVpnService_ = iface_cast<INetworkVpnService>(remote);
    networkVpnClient_.vpnEventCallback_ = new (std::nothrow) VpnSetUpEventCallback();
    ASSERT_NE(networkVpnClient_.vpnEventCallback_, nullptr);
    networkVpnClient_.deathRecipient_ = new (std::nothrow) NetworkVpnClient::MonitorVpnServiceDead(networkVpnClient_);
    ASSERT_NE(networkVpnClient_.deathRecipient_, nullptr);
    networkVpnClient_.deathRecipient_->OnRemoteDied(remote);
    networkVpnClient_.OnRemoteDied(remote);
    EXPECT_TRUE(networkVpnClient_.networkVpnService_ == nullptr);
}

HWTEST_F(NetworkVpnClientTest, multiUserSetUpEvent001, TestSize.Level1)
{
    networkVpnClient_.vpnEventCallback_ = new (std::nothrow) VpnSetUpEventCallback();
    ASSERT_NE(networkVpnClient_.vpnEventCallback_, nullptr);
    networkVpnClient_.multiUserSetUpEvent();
    EXPECT_FALSE(networkVpnClient_.vpnEventCallback_ == nullptr);
}

HWTEST_F(NetworkVpnClientTest, GetSelfAppNameTest_01, TestSize.Level1)
{
    NetworkVpnClient networkVpnClient;
    std::string selfAppName;
    std::string selfBundleName;

    int32_t result = networkVpnClient.GetSelfAppName(selfAppName, selfBundleName);

    // 验证返回值是否为成功
    EXPECT_NE(result, NETMANAGER_EXT_SUCCESS);

    // 验证应用名称和包名是否不为空
    EXPECT_TRUE(selfAppName.empty());
    EXPECT_TRUE(selfBundleName.empty());
}

HWTEST_F(NetworkVpnClientTest, GetConnectedVpnAppInfo_01, TestSize.Level1)
{
    std::vector<std::string> bundleNameList;
    EXPECT_EQ(networkVpnClient_.GetConnectedVpnAppInfo(bundleNameList), NETMANAGER_EXT_ERR_PERMISSION_DENIED);
    NetManagerExtAccessToken access;
    EXPECT_EQ(networkVpnClient_.GetConnectedVpnAppInfo(bundleNameList), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnClientTest, RegisterMultiVpnEvent_01, TestSize.Level1)
{
    sptr<VpnSetUpEventCallback> callback = nullptr;
    EXPECT_EQ(networkVpnClient_.RegisterMultiVpnEvent(callback), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    callback = new (std::nothrow) VpnSetUpEventCallback();
    ASSERT_NE(callback, nullptr);
    auto ret = networkVpnClient_.RegisterMultiVpnEvent(callback);
    EXPECT_TRUE(ret == NETMANAGER_EXT_ERR_PERMISSION_DENIED || ret == NETMANAGER_EXT_SUCCESS);
    NetManagerExtAccessToken access;
    ret = networkVpnClient_.RegisterMultiVpnEvent(callback);
    EXPECT_TRUE(ret == NETMANAGER_EXT_ERR_PARAMETER_ERROR || ret == NETMANAGER_EXT_ERR_OPERATION_FAILED);
}

HWTEST_F(NetworkVpnClientTest, UnregisterMultiVpnEvent_01, TestSize.Level1)
{
    sptr<VpnSetUpEventCallback> callback = nullptr;
    EXPECT_EQ(networkVpnClient_.UnregisterMultiVpnEvent(callback), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    callback = new (std::nothrow) VpnSetUpEventCallback();
    ASSERT_NE(callback, nullptr);
    auto ret = networkVpnClient_.UnregisterMultiVpnEvent(callback);
    EXPECT_TRUE(ret == NETMANAGER_EXT_ERR_PERMISSION_DENIED || ret == NETMANAGER_EXT_SUCCESS);
    NetManagerExtAccessToken access;
    ret = networkVpnClient_.UnregisterMultiVpnEvent(callback);
    EXPECT_TRUE(ret == NETMANAGER_EXT_ERR_OPERATION_FAILED || ret == NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnClientTest, OnMultiVpnStateChanged_01, TestSize.Level1)
{
    networkVpnClient_.vpnEventCallback_ = new (std::nothrow) VpnSetUpEventCallback();
    ASSERT_NE(networkVpnClient_.vpnEventCallback_, nullptr);
    bool isConnected = false;
    std::string vpnId = "testId";
    std::string bundleName = "com.vpn.test";
    int result = networkVpnClient_.vpnEventCallback_->OnMultiVpnStateChanged(isConnected, bundleName, vpnId);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnClientTest, GetVpnConfigToAnco001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    std::vector<std::string> dnsAddresses;
    auto ret = networkVpnClient_.GetVpnConfigToAnco(dnsAddresses);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(NetworkVpnClientTest, GetVpnConfigToAnco002, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    networkVpnClient_.SetVpnSaState(false);
    std::vector<std::string> dnsAddresses;
    auto ret = networkVpnClient_.GetVpnConfigToAnco(dnsAddresses);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
