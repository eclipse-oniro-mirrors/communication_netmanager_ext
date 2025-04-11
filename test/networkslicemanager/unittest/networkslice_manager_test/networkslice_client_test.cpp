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

#include "iservice_registry.h"
#include "netmanager_ext_test_security.h"
#include "netmgr_ext_log_wrapper.h"
#include "system_ability_definition.h"

#ifdef GTEST_API_
#define private public
#endif
#include "net_manager_constants.h"
#include "netmanager_base_common_utils.h"
#include "networkslice_client.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr int SLICE_SA_ID = 8301;
using namespace testing::ext;
} // namespace

class NetworkSliceClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<NetworkSliceClient> networkSliceClient_ =  DelayedSingleton<NetworkSliceClient>::GetInstance();
};

void NetworkSliceClientTest::SetUpTestCase() {}

void NetworkSliceClientTest::TearDownTestCase() {}

void NetworkSliceClientTest::SetUp() {}

void NetworkSliceClientTest::TearDown() {}

HWTEST_F(NetworkSliceClientTest, SetNetworkSliceUePolicy001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    std::vector<uint8_t> buffer = {0};
    EXPECT_EQ(networkSliceClient_->SetNetworkSliceUePolicy(buffer), NETMANAGER_EXT_ERR_GET_PROXY_FAIL);
}

HWTEST_F(NetworkSliceClientTest, SetNetworkSliceUePolicy002, TestSize.Level1)
{
    std::vector<uint8_t> buffer = {0};
    EXPECT_EQ(networkSliceClient_->SetNetworkSliceUePolicy(buffer), NETMANAGER_EXT_ERR_GET_PROXY_FAIL);
}

HWTEST_F(NetworkSliceClientTest, NetworkSliceInitUePolicy001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    EXPECT_EQ(networkSliceClient_->NetworkSliceInitUePolicy(), NETMANAGER_EXT_ERR_GET_PROXY_FAIL);
}

HWTEST_F(NetworkSliceClientTest, NetworkSliceInitUePolicy002, TestSize.Level1)
{
    EXPECT_EQ(networkSliceClient_->NetworkSliceInitUePolicy(), NETMANAGER_EXT_ERR_GET_PROXY_FAIL);
}

HWTEST_F(NetworkSliceClientTest, NetworkSliceAllowedNssaiRpt001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    std::vector<uint8_t> buffer = {};
    EXPECT_EQ(networkSliceClient_->NetworkSliceAllowedNssaiRpt(buffer),
        NETMANAGER_EXT_ERR_INVALID_PARAMETER);
}

HWTEST_F(NetworkSliceClientTest, NetworkSliceAllowedNssaiRpt002, TestSize.Level1)
{
    std::vector<uint8_t> buffer = {};
    EXPECT_EQ(networkSliceClient_->NetworkSliceAllowedNssaiRpt(buffer),
        NETMANAGER_EXT_ERR_INVALID_PARAMETER);
}

HWTEST_F(NetworkSliceClientTest, NetworkSliceEhplmnRpt001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    std::vector<uint8_t> buffer = {0};
    EXPECT_EQ(networkSliceClient_->NetworkSliceEhplmnRpt(buffer), NETMANAGER_EXT_ERR_GET_PROXY_FAIL);
}

HWTEST_F(NetworkSliceClientTest, NetworkSliceEhplmnRpt002, TestSize.Level1)
{
    std::vector<uint8_t> buffer = {0};
    EXPECT_EQ(networkSliceClient_->NetworkSliceEhplmnRpt(buffer), NETMANAGER_EXT_ERR_GET_PROXY_FAIL);
}

HWTEST_F(NetworkSliceClientTest, GetRouteSelectionDescriptorByDNN001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    std::string dnn = "dnn";
    std::string snssai = "";
    uint8_t sscMode = 0;
    EXPECT_EQ(networkSliceClient_->GetRouteSelectionDescriptorByDNN(dnn, snssai, sscMode),
        NETMANAGER_EXT_ERR_GET_PROXY_FAIL);
}

HWTEST_F(NetworkSliceClientTest, GetRouteSelectionDescriptorByDNN002, TestSize.Level1)
{
    std::string dnn = "dnn";
    std::string snssai = "";
    uint8_t sscMode = 0;
    EXPECT_EQ(networkSliceClient_->GetRouteSelectionDescriptorByDNN(dnn, snssai, sscMode),
        NETMANAGER_EXT_ERR_GET_PROXY_FAIL);
}

HWTEST_F(NetworkSliceClientTest, GetRSDByNetCap001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    int32_t netcap = 1;
    std::map<std::string, std::string> networkSliceParas;
    EXPECT_EQ(networkSliceClient_->GetRSDByNetCap(netcap, networkSliceParas),
        NETMANAGER_EXT_ERR_GET_PROXY_FAIL);
}

HWTEST_F(NetworkSliceClientTest, GetRSDByNetCap002, TestSize.Level1)
{
    int32_t netcap = 1;
    std::map<std::string, std::string> networkSliceParas;
    EXPECT_EQ(networkSliceClient_->GetRSDByNetCap(netcap, networkSliceParas),
        NETMANAGER_EXT_ERR_GET_PROXY_FAIL);
}

HWTEST_F(NetworkSliceClientTest, SetSaState001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    bool isSaState = false;
    EXPECT_EQ(networkSliceClient_->SetSaState(isSaState), NETMANAGER_EXT_ERR_GET_PROXY_FAIL);
}

HWTEST_F(NetworkSliceClientTest, SetSaState002, TestSize.Level1)
{
    bool isSaState = false;
    EXPECT_EQ(networkSliceClient_->SetSaState(isSaState), NETMANAGER_EXT_ERR_GET_PROXY_FAIL);
}

HWTEST_F(NetworkSliceClientTest, OnRemoteDied, TestSize.Level1)
{
    sptr<IRemoteObject> remote = nullptr;
    networkSliceClient_->OnRemoteDied(remote);
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    remote = sam->CheckSystemAbility(SLICE_SA_ID);
    networkSliceClient_->networksliceService_ = nullptr;
    networkSliceClient_->OnRemoteDied(remote);
    networkSliceClient_->networksliceService_ = iface_cast<INetworkSliceService>(remote);
    networkSliceClient_->OnRemoteDied(remote);
    EXPECT_EQ(networkSliceClient_->networksliceService_, nullptr);
}

}
}
