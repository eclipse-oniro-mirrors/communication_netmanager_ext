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
#include "mock_i_network_vpn_service.h"
#include "networkvpn_permission_client.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing;
using namespace testing::ext;
} // namespace

class IVpnEventCallbackTest : public IRemoteStub<IVpnEventCallback> {
public:
    int32_t OnVpnStateChanged(bool isConnected, const sptr<VpnState> &vpnState) override{ return 0; };
    int32_t OnMultiVpnStateChanged(bool isConnected, const std::string &bundleName,
        const std::string &vpnId) override{ return 0; };
    int32_t OnVpnMultiUserSetUp() override{ return 0; };
};

class NetworkVpnClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    sptr<IVpnEventCallback> callback_ = nullptr;
    NetworkVpnClient &networkVpnClient_ = NetworkVpnClient::GetInstance();
};

void NetworkVpnClientTest::SetUpTestCase() {}

void NetworkVpnClientTest::TearDownTestCase() {}

void NetworkVpnClientTest::SetUp() {}

void NetworkVpnClientTest::TearDown() {}

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

HWTEST_F(NetworkVpnClientTest, Prepare001, TestSize.Level1)
{
    bool isExistVpn = false;
    bool isRun = false;
    std::string pkg;
    EXPECT_EQ(networkVpnClient_.Prepare(isExistVpn, isRun, pkg), NETMANAGER_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkVpnClientTest, Prepare002, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    bool isExistVpn = false;
    bool isRun = false;
    std::string pkg;
    EXPECT_EQ(networkVpnClient_.Prepare(isExistVpn, isRun, pkg), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnClientTest, Protect001, TestSize.Level1)
{
    EXPECT_EQ(networkVpnClient_.Protect(0), NETWORKVPN_ERROR_INVALID_FD);
    EXPECT_EQ(networkVpnClient_.Protect(1), NETMANAGER_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkVpnClientTest, Protect002, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    EXPECT_EQ(networkVpnClient_.Protect(1), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnClientTest, SetUpVpn001, TestSize.Level1)
{
    sptr<VpnConfig> config = nullptr;
    int32_t tunFd = INVALID_TUN_FD;
    EXPECT_EQ(networkVpnClient_.SetUpVpn(config, tunFd), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    config = new (std::nothrow) VpnConfig();
    EXPECT_EQ(networkVpnClient_.SetUpVpn(config, tunFd), NETMANAGER_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkVpnClientTest, SetUpVpn002, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    int32_t tunFd = INVALID_TUN_FD;
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    EXPECT_GE(networkVpnClient_.SetUpVpn(config, tunFd), NETMANAGER_EXT_SUCCESS);
    EXPECT_GE(networkVpnClient_.DestroyVpn(), NETMANAGER_EXT_SUCCESS);
}

#ifdef SUPPORT_SYSVPN
HWTEST_F(NetworkVpnClientTest, SetUpVpn003, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    int32_t tunFd = INVALID_TUN_FD;
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    EXPECT_EQ(networkVpnClient_.SetUpVpn(config, tunFd), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(networkVpnClient_.DestroyVpn(), NETMANAGER_EXT_SUCCESS);
	
    config->vpnId_ = "test";
    tunFd = 10;
    EXPECT_EQ(networkVpnClient_.SetUpVpn(config, tunFd), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(networkVpnClient_.DestroyVpn(), NETMANAGER_EXT_SUCCESS);
}
#endif

HWTEST_F(NetworkVpnClientTest, SetUpVpnWithMockProxy001, TestSize.Level1)
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

    int32_t tunFd = INVALID_TUN_FD;
    ON_CALL(*mock, SetUpVpn(_, _, _, _))
        .WillByDefault(DoAll(SetArgReferee<3>(fds[0]), Return(NETMANAGER_EXT_SUCCESS)));
    EXPECT_EQ(client.SetUpVpn(config, tunFd), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(tunFd, fds[0]);
    EXPECT_EQ(client.DestroyVpn(), NETMANAGER_EXT_SUCCESS);

    ON_CALL(*mock, SetUpVpn(_, _, _, _))
        .WillByDefault(DoAll(SetArgReferee<3>(INVALID_TUN_FD), Return(NETMANAGER_EXT_ERR_INTERNAL)));
    EXPECT_EQ(client.SetUpVpn(config, tunFd), NETMANAGER_EXT_ERR_INTERNAL);
    EXPECT_EQ(tunFd, INVALID_TUN_FD);

    ON_CALL(*mock, SetUpVpn(_, _, _, _))
        .WillByDefault(DoAll(SetArgReferee<3>(INVALID_TUN_FD), Return(NETMANAGER_EXT_SUCCESS)));
    EXPECT_EQ(client.SetUpVpn(config, tunFd), NETMANAGER_EXT_ERR_INTERNAL);
    EXPECT_EQ(tunFd, INVALID_TUN_FD);
    EXPECT_EQ(client.DestroyVpn(), NETMANAGER_EXT_SUCCESS);

    client.networkVpnService_ = nullptr;
    close(fds[1]);
}

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

HWTEST_F(NetworkVpnClientTest, RegisterVpnEvent001, TestSize.Level1)
{
    networkVpnClient_.vpnEventCbCollection_ = nullptr;
    EXPECT_EQ(networkVpnClient_.RegisterVpnEvent(nullptr), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    callback_ = sptr<IVpnEventCallbackTest>::MakeSptr();
    EXPECT_EQ(networkVpnClient_.RegisterVpnEvent(callback_), NETMANAGER_EXT_ERR_LOCAL_PTR_NULL);
}

HWTEST_F(NetworkVpnClientTest, StartVpnExtensionAbilityTest_01, TestSize.Level1)
{
    OHOS::AAFwk::Want want;
    int32_t result = networkVpnClient_.StartVpnExtensionAbility(want);
    EXPECT_NE(result, ERR_OK);
}

HWTEST_F(NetworkVpnClientTest, StopVpnExtensionAbilityTest_01, TestSize.Level1)
{
    OHOS::AAFwk::Want want;
    int32_t result = networkVpnClient_.StopVpnExtensionAbility(want);
    EXPECT_NE(result, ERR_OK);
}

HWTEST_F(NetworkVpnClientTest, RegisterVpnEvent002, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    networkVpnClient_.vpnEventCbCollection_ = sptr<VpnEventCallbackCollection>::MakeSptr();
    networkVpnClient_.saStart_ = false;
    callback_ = sptr<IVpnEventCallbackTest>::MakeSptr();
    EXPECT_EQ(networkVpnClient_.RegisterVpnEvent(callback_), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnClientTest, RegisterVpnEvent003, TestSize.Level1)
{
    networkVpnClient_.vpnEventCbCollection_ = sptr<VpnEventCallbackCollection>::MakeSptr();
    callback_ = sptr<IVpnEventCallbackTest>::MakeSptr();
    networkVpnClient_.saStart_ = true;
    EXPECT_EQ(networkVpnClient_.RegisterVpnEvent(callback_), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(networkVpnClient_.vpnEventCbCollection_->GetCallbackNum(), 1);
}

HWTEST_F(NetworkVpnClientTest, RegisterVpnEventCbCollection001, TestSize.Level1)
{
    networkVpnClient_.vpnEventCbCollection_ = nullptr;
    networkVpnClient_.RegisterVpnEventCbCollection();
    
    networkVpnClient_.vpnEventCbCollection_ = sptr<VpnEventCallbackCollection>::MakeSptr();
    networkVpnClient_.RegisterVpnEventCbCollection();
    EXPECT_EQ(networkVpnClient_.vpnEventCbCollection_->GetCallbackNum(), 0);
}

HWTEST_F(NetworkVpnClientTest, UnregisterVpnEvent001, TestSize.Level1)
{
    EXPECT_EQ(networkVpnClient_.UnregisterVpnEvent(nullptr), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    networkVpnClient_.vpnEventCbCollection_ = sptr<VpnEventCallbackCollection>::MakeSptr();
    callback_ = sptr<IVpnEventCallbackTest>::MakeSptr();
    networkVpnClient_.RegisterVpnEvent(callback_);
    EXPECT_EQ(networkVpnClient_.UnregisterVpnEvent(callback_), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnClientTest, UnregisterVpnEvent002, TestSize.Level1)
{
    networkVpnClient_.vpnEventCbCollection_ = nullptr;
    EXPECT_EQ(networkVpnClient_.UnregisterVpnEvent(nullptr), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    callback_ = sptr<IVpnEventCallbackTest>::MakeSptr();
    EXPECT_EQ(networkVpnClient_.UnregisterVpnEvent(callback_), NETMANAGER_EXT_SUCCESS);
}

#ifdef SUPPORT_SYSVPN
HWTEST_F(NetworkVpnClientTest, RegisterMultiVpnEvent001, TestSize.Level1)
{
    networkVpnClient_.multiVpnEventCbCollection_ = nullptr;
    EXPECT_EQ(networkVpnClient_.RegisterMultiVpnEvent(nullptr), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    callback_ = sptr<IVpnEventCallbackTest>::MakeSptr();
    EXPECT_EQ(networkVpnClient_.RegisterMultiVpnEvent(callback_), NETMANAGER_EXT_ERR_LOCAL_PTR_NULL);
}

HWTEST_F(NetworkVpnClientTest, RegisterMultiVpnEvent002, TestSize.Level1)
{
    networkVpnClient_.multiVpnEventCbCollection_ = sptr<VpnEventCallbackCollection>::MakeSptr();
    networkVpnClient_.saStart_ = true;
    callback_ = sptr<IVpnEventCallbackTest>::MakeSptr();
    EXPECT_EQ(networkVpnClient_.RegisterMultiVpnEvent(callback_), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnClientTest, RegisterMultiVpnEventCbCollection001, TestSize.Level1)
{
    networkVpnClient_.multiVpnEventCbCollection_ = nullptr;
    networkVpnClient_.RegisterMultiVpnEventCbCollection();
    
    networkVpnClient_.multiVpnEventCbCollection_ = sptr<VpnEventCallbackCollection>::MakeSptr();
    networkVpnClient_.RegisterMultiVpnEventCbCollection();
    EXPECT_EQ(networkVpnClient_.multiVpnEventCbCollection_->GetCallbackNum(), 0);
}

HWTEST_F(NetworkVpnClientTest, UnregisterMultiVpnEvent001, TestSize.Level1)
{
    EXPECT_EQ(networkVpnClient_.UnregisterMultiVpnEvent(nullptr), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    callback_ = sptr<IVpnEventCallbackTest>::MakeSptr();
    networkVpnClient_.multiVpnEventCbCollection_ = sptr<VpnEventCallbackCollection>::MakeSptr();
    networkVpnClient_.RegisterMultiVpnEvent(callback_);
    EXPECT_EQ(networkVpnClient_.UnregisterMultiVpnEvent(callback_), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnClientTest, UnregisterMultiVpnEvent002, TestSize.Level1)
{
    networkVpnClient_.multiVpnEventCbCollection_ = nullptr;
    EXPECT_EQ(networkVpnClient_.UnregisterMultiVpnEvent(nullptr), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    callback_ = sptr<IVpnEventCallbackTest>::MakeSptr();
    EXPECT_EQ(networkVpnClient_.UnregisterMultiVpnEvent(callback_), NETMANAGER_EXT_SUCCESS);
}
#endif

HWTEST_F(NetworkVpnClientTest, GetConnectedVpnConfigList001, TestSize.Level1)
{
    bool saStartBak = networkVpnClient_.saStart_;
    networkVpnClient_.saStart_ = false;
    std::vector<sptr<VpnConfig>> vpnConfigList;
    sptr<VpnConfig> stale = new (std::nothrow) VpnConfig();
    ASSERT_NE(stale, nullptr);
    vpnConfigList.push_back(stale);
    // early error paths must not clear the caller's vector
    EXPECT_EQ(networkVpnClient_.GetConnectedVpnConfigList(0, vpnConfigList), NETMANAGER_EXT_ERR_GET_PROXY_FAIL);
    ASSERT_EQ(vpnConfigList.size(), 1u);
    EXPECT_EQ(vpnConfigList[0], stale);
    EXPECT_EQ(networkVpnClient_.GetConnectedVpnConfigList(-1, vpnConfigList), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    EXPECT_EQ(vpnConfigList.size(), 1u);
    networkVpnClient_.saStart_ = saStartBak;
}

HWTEST_F(NetworkVpnClientTest, GetConnectedVpnConfigList002, TestSize.Level1)
{
    auto mock = sptr<MockINetworkVpnService>::MakeSptr();
    ASSERT_NE(mock, nullptr);
    NetworkVpnClient client;
    client.networkVpnService_ = mock;
    client.saStart_ = true;
    std::vector<sptr<VpnConfig>> invalidList;
    EXPECT_EQ(client.GetConnectedVpnConfigList(-1, invalidList), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    EXPECT_TRUE(invalidList.empty());
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    ASSERT_NE(config, nullptr);
    std::vector<sptr<VpnConfig>> mockList = { config };
    ON_CALL(*mock, GetConnectedVpnConfigList(_, _))
        .WillByDefault(DoAll(SetArgReferee<1>(mockList), Return(NETMANAGER_EXT_SUCCESS)));
    std::vector<sptr<VpnConfig>> vpnConfigList;
    EXPECT_EQ(client.GetConnectedVpnConfigList(0, vpnConfigList), NETMANAGER_EXT_SUCCESS);
    ASSERT_EQ(vpnConfigList.size(), 1u);
    EXPECT_EQ(vpnConfigList[0], config);

    ON_CALL(*mock, GetConnectedVpnConfigList(_, _))
        .WillByDefault(Return(NETMANAGER_EXT_ERR_INTERNAL));
    EXPECT_EQ(client.GetConnectedVpnConfigList(0, vpnConfigList), NETMANAGER_EXT_ERR_INTERNAL);
    // a failed proxy call must not touch the caller's vector either
    ASSERT_EQ(vpnConfigList.size(), 1u);
    EXPECT_EQ(vpnConfigList[0], config);
    client.networkVpnService_ = nullptr;
}

HWTEST_F(NetworkVpnClientTest, GetProxy, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> remote = sam->GetSystemAbility(COMM_VPN_MANAGER_SYS_ABILITY_ID);
    networkVpnClient_.networkVpnService_ = iface_cast<INetworkVpnService>(remote);
    EXPECT_EQ(networkVpnClient_.GetProxy(), networkVpnClient_.networkVpnService_);
    networkVpnClient_.networkVpnService_ = nullptr;
    EXPECT_NE(networkVpnClient_.GetProxy(), nullptr);
}

HWTEST_F(NetworkVpnClientTest, OnRemoteDied, TestSize.Level1)
{
    sptr<IRemoteObject> remote = nullptr;
    networkVpnClient_.OnRemoteDied(remote);
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    remote = sam->GetSystemAbility(COMM_VPN_MANAGER_SYS_ABILITY_ID);
    networkVpnClient_.networkVpnService_ = nullptr;
    networkVpnClient_.OnRemoteDied(remote);
    networkVpnClient_.networkVpnService_ = iface_cast<INetworkVpnService>(remote);
    networkVpnClient_.OnRemoteDied(remote);
    EXPECT_EQ(networkVpnClient_.networkVpnService_, nullptr);
}

HWTEST_F(NetworkVpnClientTest, NetworkVpnClientBranch001, TestSize.Level1)
{
    callback_ = sptr<IVpnEventCallbackTest>::MakeSptr();
    callback_->OnVpnMultiUserSetUp();
    networkVpnClient_.multiUserSetUpEvent();

    auto ret = networkVpnClient_.CreateVpnConnection();
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnClientTest, VpnEventCallback, TestSize.Level1)
{
    callback_ = sptr<IVpnEventCallbackTest>::MakeSptr();
    auto collection = sptr<VpnEventCallbackCollection>::MakeSptr();
    collection->vpnEventCbList_.push_back(callback_);

    sptr<VpnState> vpnState = new VpnState();
    EXPECT_EQ(collection->OnVpnStateChanged(true, vpnState), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(collection->OnMultiVpnStateChanged(true, "test", "0"), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(collection->OnVpnMultiUserSetUp(), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnClientTest, RequestVpnPermission0001, TestSize.Level1)
{
    bool isAuthorized = false;
    auto ret = RequestVpnPermission(1099, "", "", isAuthorized);
    EXPECT_GE(ret, NETMANAGER_EXT_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetworkVpnClientTest, RequestVpnPermission0002, TestSize.Level1)
{
    bool isAuthorized = false;
    auto ret = RequestVpnPermission(1099, "test", "", isAuthorized);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
