/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "token_setproc.h"

#include "fwmark_client.h"
#include "iservice_registry.h"
#include "netmgr_ext_log_wrapper.h"

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
class VpnEventTestCallback : public VpnEventCallbackStub {
public:
    inline void OnVpnStateChanged(const bool &isConnected) override
    {
        std::cout << std::endl;
        std::cout << "OnVpnStateChanged::isConnected: " << isConnected << std::endl;
        return;
    }
};
} // namespace

class IVpnEventCallbackTest : public IRemoteStub<IVpnEventCallback> {
public:
    void OnVpnStateChanged(const bool &isConnected) override
    {
        return;
    }
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

HWTEST_F(NetworkVpnClientTest, Prepare, TestSize.Level1)
{
    bool isExistVpn = false;
    bool isRun = false;
    std::string pkg;
    EXPECT_EQ(networkVpnClient_.Prepare(isExistVpn, isRun, pkg), NETMANAGER_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkVpnClientTest, Protect, TestSize.Level1)
{
    int32_t socketFd = 0;
    EXPECT_EQ(networkVpnClient_.Protect(socketFd), NETWORKVPN_ERROR_INVALID_FD);
    socketFd = 1;
    EXPECT_EQ(networkVpnClient_.Protect(socketFd), NETMANAGER_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkVpnClientTest, SetUpVpn, TestSize.Level1)
{
    sptr<VpnConfig> config = nullptr;
    int32_t tunFd = 0;
    EXPECT_EQ(networkVpnClient_.SetUpVpn(config, tunFd), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    config = new (std::nothrow) VpnConfig();
    networkVpnClient_.vpnInterface_.tunFd_ = 0;
    EXPECT_EQ(networkVpnClient_.SetUpVpn(config, tunFd), NETMANAGER_EXT_ERR_PERMISSION_DENIED);
    networkVpnClient_.vpnInterface_.tunFd_ = 1;
    EXPECT_EQ(networkVpnClient_.SetUpVpn(config, tunFd), NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkVpnClientTest, RegisterVpnEvent, TestSize.Level1)
{
    callback_ = nullptr;
    EXPECT_EQ(networkVpnClient_.RegisterVpnEvent(callback_), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    callback_ = new (std::nothrow) IVpnEventCallbackTest();
    EXPECT_EQ(networkVpnClient_.RegisterVpnEvent(callback_), NETMANAGER_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkVpnClientTest, UnregisterVpnEvent, TestSize.Level1)
{
    callback_ = nullptr;
    EXPECT_EQ(networkVpnClient_.UnregisterVpnEvent(callback_), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    callback_ = new (std::nothrow) IVpnEventCallbackTest();
    EXPECT_EQ(networkVpnClient_.UnregisterVpnEvent(callback_), NETMANAGER_ERR_PERMISSION_DENIED);
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
    NETMGR_EXT_LOG_E("dmdebug OnRemoteDied ");
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
} // namespace NetManagerStandard
} // namespace OHOS
