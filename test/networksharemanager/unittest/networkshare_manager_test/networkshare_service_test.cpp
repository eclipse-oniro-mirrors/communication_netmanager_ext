/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "net_manager_constants.h"
#include "netmanager_ext_test_security.h"
#include "networkshare_constants.h"
#include "networkshare_service.h"
#include "sharing_event_callback_stub.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;

class SharingEventTestCallback : public SharingEventCallbackStub {
public:
    inline void OnSharingStateChanged(const bool &isRunning) override
    {
        return;
    }
    inline void OnInterfaceSharingStateChanged(const SharingIfaceType &type, const std::string &iface,
                                               const SharingIfaceState &state) override
    {
        return;
    }
    inline void OnSharingUpstreamChanged(const sptr<NetHandle> netHandle) override
    {
        return;
    }
};
} // namespace

class NetworkShareServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline auto instance_ = DelayedSingleton<NetworkShareService>::GetInstance();
    static inline sptr<ISharingEventCallback> eventCallback_ = nullptr;
};

void NetworkShareServiceTest::SetUpTestCase()
{
    instance_->OnStart();
    eventCallback_ = new (std::nothrow) SharingEventTestCallback();
    ASSERT_NE(eventCallback_, nullptr);
}

void NetworkShareServiceTest::TearDownTestCase()
{
    instance_->OnStop();
}

void NetworkShareServiceTest::SetUp() {}

void NetworkShareServiceTest::TearDown() {}

HWTEST_F(NetworkShareServiceTest, IsNetworkSharingSupportedTest001, TestSize.Level1)
{
    int32_t supported;
    auto ret = instance_->IsNetworkSharingSupported(supported);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareServiceTest, IsNetworkSharingSupportedTest002, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    int32_t supported;
    auto ret = instance_->IsNetworkSharingSupported(supported);
    EXPECT_EQ(supported, NETWORKSHARE_IS_UNSUPPORTED);
    EXPECT_EQ(ret, NETWORKSHARE_ERROR_IFACE_CFG_ERROR);
}

HWTEST_F(NetworkShareServiceTest, IsSharingTest001, TestSize.Level1)
{
    int32_t sharingStatus;
    auto ret = instance_->IsSharing(sharingStatus);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareServiceTest, IsSharingTest002, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    int32_t sharingStatus;
    auto ret = instance_->IsSharing(sharingStatus);
    EXPECT_NE(sharingStatus, NETWORKSHARE_IS_SHARING);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareServiceTest, StartNetworkSharingTest001, TestSize.Level1)
{
    auto ret = instance_->StartNetworkSharing(SharingIfaceType::SHARING_WIFI);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareServiceTest, StartNetworkSharingTest002, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    auto ret = instance_->StartNetworkSharing(SharingIfaceType::SHARING_WIFI);
    EXPECT_EQ(ret, NETWORKSHARE_ERROR_WIFI_SHARING);
}

HWTEST_F(NetworkShareServiceTest, StartNetworkSharingTest003, TestSize.Level1)
{
    auto ret = instance_->StartNetworkSharing(SharingIfaceType::SHARING_USB);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareServiceTest, StopNetworkSharingTest001, TestSize.Level1)
{
    auto ret = instance_->StopNetworkSharing(SharingIfaceType::SHARING_WIFI);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareServiceTest, StopNetworkSharingTest002, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    auto ret = instance_->StopNetworkSharing(SharingIfaceType::SHARING_WIFI);
    EXPECT_EQ(ret, NETWORKSHARE_ERROR_WIFI_SHARING);
}

HWTEST_F(NetworkShareServiceTest, StopNetworkSharingTest003, TestSize.Level1)
{
    auto ret = instance_->StopNetworkSharing(SharingIfaceType::SHARING_USB);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareServiceTest, RegisterSharingEventTest001, TestSize.Level1)
{
    auto ret = instance_->RegisterSharingEvent(eventCallback_);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareServiceTest, RegisterSharingEventTest002, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    auto ret = instance_->RegisterSharingEvent(eventCallback_);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareServiceTest, UnregisterSharingEventTest001, TestSize.Level1)
{
    auto ret = instance_->UnregisterSharingEvent(eventCallback_);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareServiceTest, UnregisterSharingEventTest002, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    auto ret = instance_->UnregisterSharingEvent(eventCallback_);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareServiceTest, GetSharableRegexsTest001, TestSize.Level1)
{
    std::vector<std::string> ifaceRegexs;
    instance_->GetSharableRegexs(SharingIfaceType::SHARING_WIFI, ifaceRegexs);
    EXPECT_TRUE(ifaceRegexs.empty());
}

HWTEST_F(NetworkShareServiceTest, GetSharableRegexsTest002, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    std::vector<std::string> ifaceRegexs;
    instance_->GetSharableRegexs(SharingIfaceType::SHARING_WIFI, ifaceRegexs);
    EXPECT_TRUE(ifaceRegexs.empty());
}

HWTEST_F(NetworkShareServiceTest, GetSharingStateTest001, TestSize.Level1)
{
    SharingIfaceState state;
    auto ret = instance_->GetSharingState(SharingIfaceType::SHARING_WIFI, state);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareServiceTest, GetSharingStateTest002, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    SharingIfaceState state;
    auto ret = instance_->GetSharingState(SharingIfaceType::SHARING_WIFI, state);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareServiceTest, GetNetSharingIfacesTest001, TestSize.Level1)
{
    SharingIfaceType type = SharingIfaceType::SHARING_WIFI;
    std::vector<std::string> ifaceRegexs;
    instance_->GetSharableRegexs(type, ifaceRegexs);
    EXPECT_TRUE(ifaceRegexs.empty());
}

HWTEST_F(NetworkShareServiceTest, GetNetSharingIfacesTest002, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    SharingIfaceType type = SharingIfaceType::SHARING_WIFI;
    std::vector<std::string> ifaceRegexs;
    instance_->GetSharableRegexs(type, ifaceRegexs);
    EXPECT_TRUE(ifaceRegexs.empty());
}

HWTEST_F(NetworkShareServiceTest, GetStatsRxBytesTest001, TestSize.Level1)
{
    int32_t bytes;
    auto ret = instance_->GetStatsRxBytes(bytes);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareServiceTest, GetStatsRxBytesTest002, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    int32_t bytes;
    auto ret = instance_->GetStatsRxBytes(bytes);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareServiceTest, GetStatsTxBytesTest001, TestSize.Level1)
{
    int32_t bytes;
    auto ret = instance_->GetStatsTxBytes(bytes);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareServiceTest, GetStatsTxBytesTest002, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    int32_t bytes;
    auto ret = instance_->GetStatsTxBytes(bytes);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareServiceTest, GetStatsTotalBytesTest001, TestSize.Level1)
{
    int32_t bytes;
    auto ret = instance_->GetStatsTotalBytes(bytes);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareServiceTest, GetStatsTotalBytesTest002, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    int32_t bytes;
    auto ret = instance_->GetStatsTotalBytes(bytes);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareServiceTest, GetDumpMessage001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    std::string message = "";
    instance_->GetDumpMessage(message);
    EXPECT_TRUE(!message.empty());
}

HWTEST_F(NetworkShareServiceTest, GetShareRegexsContent001, TestSize.Level1)
{
    std::string shareRegexsContent = "";
    SharingIfaceType type = SharingIfaceType::SHARING_WIFI;
    instance_->GetShareRegexsContent(type, shareRegexsContent);
    EXPECT_TRUE(shareRegexsContent.empty());
}

HWTEST_F(NetworkShareServiceTest, NetworkShareServiceBranch001, TestSize.Level1)
{
    instance_->state_ = NetworkShareService::ServiceRunningState::STATE_RUNNING;
    instance_->OnStart();

    std::string sharingType;
    std::string typeContent = "wifi;";
    instance_->GetSharingType(SharingIfaceType::SHARING_BLUETOOTH, typeContent, sharingType);

    SharingIfaceState state = SharingIfaceState::SHARING_NIC_CAN_SERVER;
    std::vector<std::string> ifaces;
    int32_t ret = instance_->GetNetSharingIfaces(state, ifaces);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareServiceTest, NetworkShareServiceBranch002, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    SharingIfaceState state = SharingIfaceState::SHARING_NIC_CAN_SERVER;
    std::vector<std::string> ifaces;
    int32_t ret = instance_->GetNetSharingIfaces(state, ifaces);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareServiceTest, OnAddSystemAbility001, TestSize.Level1)
{
    std::string deviceId = "dev1";
    instance_->OnRemoveSystemAbility(COMM_NETSYS_NATIVE_SYS_ABILITY_ID, deviceId);
    EXPECT_TRUE(instance_->hasSARemoved_);

    instance_->OnAddSystemAbility(COMM_NETSYS_NATIVE_SYS_ABILITY_ID, deviceId);
    EXPECT_FALSE(instance_->hasSARemoved_);
}
} // namespace NetManagerStandard
} // namespace OHOS
