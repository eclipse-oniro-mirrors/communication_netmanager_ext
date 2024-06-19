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
#include "system_ability_definition.h"

#ifdef GTEST_API_
#define private public
#endif
#include "net_manager_constants.h"
#include "netmanager_base_common_utils.h"
#include "networkvpn_client.h"
#include "vpn_event_callback_stub.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
} // namespace

class IVpnEventCallbackTest : public IRemoteStub<IVpnEventCallback> {
public:
    void OnVpnStateChanged(const bool &isConnected) override{};
    void OnVpnMultiUserSetUp() override{};
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
    int32_t tunFd = 0;
    EXPECT_EQ(networkVpnClient_.SetUpVpn(config, tunFd), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    config = new (std::nothrow) VpnConfig();
    EXPECT_EQ(networkVpnClient_.SetUpVpn(config, tunFd), NETMANAGER_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkVpnClientTest, SetUpVpn002, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    int32_t tunFd = 0;
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    EXPECT_EQ(networkVpnClient_.SetUpVpn(config, tunFd), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(networkVpnClient_.DestroyVpn(), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnClientTest, RegisterVpnEvent001, TestSize.Level1)
{
    EXPECT_EQ(networkVpnClient_.RegisterVpnEvent(nullptr), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    callback_ = new (std::nothrow) IVpnEventCallbackTest();
    EXPECT_EQ(networkVpnClient_.RegisterVpnEvent(callback_), NETMANAGER_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkVpnClientTest, RegisterVpnEvent002, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    EXPECT_EQ(networkVpnClient_.RegisterVpnEvent(nullptr), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    callback_ = new (std::nothrow) IVpnEventCallbackTest();
    EXPECT_EQ(networkVpnClient_.RegisterVpnEvent(callback_), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnClientTest, UnregisterVpnEvent001, TestSize.Level1)
{
    EXPECT_EQ(networkVpnClient_.UnregisterVpnEvent(nullptr), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    callback_ = new (std::nothrow) IVpnEventCallbackTest();
    EXPECT_EQ(networkVpnClient_.UnregisterVpnEvent(callback_), NETMANAGER_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkVpnClientTest, UnregisterVpnEvent002, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    EXPECT_EQ(networkVpnClient_.UnregisterVpnEvent(nullptr), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    callback_ = new (std::nothrow) IVpnEventCallbackTest();
    EXPECT_EQ(networkVpnClient_.UnregisterVpnEvent(callback_), NETMANAGER_EXT_ERR_OPERATION_FAILED);
}

HWTEST_F(NetworkVpnClientTest, GetProxy, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> remote = sam->CheckSystemAbility(COMM_VPN_MANAGER_SYS_ABILITY_ID);
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
    remote = sam->CheckSystemAbility(COMM_VPN_MANAGER_SYS_ABILITY_ID);
    networkVpnClient_.networkVpnService_ = nullptr;
    networkVpnClient_.OnRemoteDied(remote);
    networkVpnClient_.networkVpnService_ = iface_cast<INetworkVpnService>(remote);
    networkVpnClient_.OnRemoteDied(remote);
    EXPECT_EQ(networkVpnClient_.networkVpnService_, nullptr);
}

HWTEST_F(NetworkVpnClientTest, NetworkVpnClientBranch001, TestSize.Level1)
{
    callback_ = new (std::nothrow) IVpnEventCallbackTest();
    callback_->OnVpnMultiUserSetUp();
    networkVpnClient_.multiUserSetUpEvent();

    auto ret = networkVpnClient_.CreateVpnConnection();
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnClientTest, RegisterBundleName001, TestSize.Level1)
{
    std::string bundleName = "com.test.test";
    auto ret = networkVpnClient_.RegisterBundleName(bundleName);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
