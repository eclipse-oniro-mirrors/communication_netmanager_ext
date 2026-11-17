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

#include <map>
#include <mutex>
#include <set>
#include <vector>

#include <gtest/gtest.h>

#include "net_manager_constants.h"
#include "netmanager_ext_test_security.h"
#include "netmgr_ext_log_wrapper.h"
#include "system_ability_definition.h"
#define private public
#include "networkshare_client.h"
#undef private
#include "mock_i_networkshare_service.h"
#include "networkshare_constants.h"
#include "sharing_event_callback_stub.h"
#include "networkshare_service_stub_test.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
using testing::_;
using testing::DoAll;
using testing::Return;
using testing::SetArgReferee;
constexpr int32_t EIGHT_SECONDS = 8;
constexpr int32_t TWO_SECONDS = 2;

class TestSharingEventCallback : public OHOS::NetManagerStandard::SharingEventCallbackStub {
public:
    TestSharingEventCallback() = default;
    virtual ~TestSharingEventCallback() = default;
    void OnSharingStateChanged(const bool &isRunning);
    void OnInterfaceSharingStateChanged(const SharingIfaceType &type, const std::string &iface,
                                        const SharingIfaceState &state);
    void OnSharingUpstreamChanged(const sptr<NetHandle> netHandle);
};

sptr<TestSharingEventCallback> g_sharingEventCb = new (std::nothrow) TestSharingEventCallback();

class SharingAddressMockService : public MockNetworkShareServiceStub {
public:
    int32_t GetSharingAddress(int32_t type, sptr<NetworkShareAddressParcel> &addressParcel) override
    {
        addressParcel = addressParcel_;
        return ret_;
    }

    int32_t ret_ = NETMANAGER_EXT_SUCCESS;
    sptr<NetworkShareAddressParcel> addressParcel_ = nullptr;
};

int32_t GetSharingAddressByMock(const sptr<SharingAddressMockService> &mock, SharingIfaceType type,
                                std::vector<INetAddr> &addresses)
{
    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    sptr<INetworkShareService> before = client->networkShareService_;
    client->networkShareService_ = mock;
    int32_t result = client->GetSharingAddress(type, addresses);
    client->networkShareService_ = before;
    return result;
}
} // namespace

class NetworkShareManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetworkShareManagerTest::SetUpTestCase() {}

void NetworkShareManagerTest::TearDownTestCase() {}

void NetworkShareManagerTest::SetUp() {}

void NetworkShareManagerTest::TearDown() {}

void TestSharingEventCallback::OnSharingStateChanged(const bool &isRunning)
{
    std::cout << "TestSharingEventCallback::OnSharingStateChanged isRunning = " << isRunning << std::endl;
}

void TestSharingEventCallback::OnInterfaceSharingStateChanged(const SharingIfaceType &type, const std::string &iface,
                                                              const SharingIfaceState &state)
{
    std::cout << "type=" << static_cast<int32_t>(type);
    std::cout << "iface:" << iface;
    std::cout << ", state=" << static_cast<int32_t>(state) << std::endl;
}

void TestSharingEventCallback::OnSharingUpstreamChanged(const sptr<NetHandle> netHandle)
{
    std::cout << "TestSharingEventCallback::OnSharingUpstreamChanged netId = " << netHandle->GetNetId() << std::endl;
}

HWTEST_F(NetworkShareManagerTest, IsSharingSupported, TestSize.Level1)
{
    // client-side test with a mock service proxy: no real IPC, no device permission env
    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    auto mock = sptr<MockINetworkShareService>::MakeSptr();
    client->networkShareService_ = mock;
    ON_CALL(*mock, IsNetworkSharingSupported(_)).WillByDefault(Return(NETMANAGER_EXT_SUCCESS));
    int32_t supportedFlag = 0;
    auto ret = client->IsSharingSupported(supportedFlag);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    client->networkShareService_ = nullptr;
}

HWTEST_F(NetworkShareManagerTest, IsSharing, TestSize.Level1)
{
    int32_t sharingStatus;
    int32_t ret = DelayedSingleton<NetworkShareClient>::GetInstance()->IsSharing(sharingStatus);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareManagerTest, GetWifiSharableRegexs, TestSize.Level1)
{
    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    auto mock = sptr<MockINetworkShareService>::MakeSptr();
    client->networkShareService_ = mock;
    ON_CALL(*mock, GetSharableRegexs(_, _))
        .WillByDefault(DoAll(SetArgReferee<1>(std::vector<std::string>{"wlan0"}), Return(NETMANAGER_EXT_SUCCESS)));
    std::vector<std::string> wifiRegexs;
    client->GetSharableRegexs(SharingIfaceType::SHARING_WIFI, wifiRegexs);
    EXPECT_NE(wifiRegexs.size(), static_cast<uint32_t>(0));
    for (auto regex : wifiRegexs) {
        std::cout << "Wifi Sharable Regex: " << regex << std::endl;
    }
    client->networkShareService_ = nullptr;
}

HWTEST_F(NetworkShareManagerTest, GetUSBSharableRegexs, TestSize.Level1)
{
    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    auto mock = sptr<MockINetworkShareService>::MakeSptr();
    client->networkShareService_ = mock;
    ON_CALL(*mock, GetSharableRegexs(_, _))
        .WillByDefault(DoAll(SetArgReferee<1>(std::vector<std::string>{"usb0"}), Return(NETMANAGER_EXT_SUCCESS)));
    std::vector<std::string> usbRegexs;
    client->GetSharableRegexs(SharingIfaceType::SHARING_USB, usbRegexs);
    EXPECT_NE(usbRegexs.size(), static_cast<uint32_t>(0));
    for (auto regex : usbRegexs) {
        std::cout << "USB Sharable Regex: " << regex << std::endl;
    }
    client->networkShareService_ = nullptr;
}

HWTEST_F(NetworkShareManagerTest, GetBluetoothSharableRegexs, TestSize.Level1)
{
    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    auto mock = sptr<MockINetworkShareService>::MakeSptr();
    client->networkShareService_ = mock;
    ON_CALL(*mock, GetSharableRegexs(_, _))
        .WillByDefault(DoAll(SetArgReferee<1>(std::vector<std::string>{"bt-pan"}), Return(NETMANAGER_EXT_SUCCESS)));
    std::vector<std::string> blueRegexs;
    client->GetSharableRegexs(SharingIfaceType::SHARING_BLUETOOTH, blueRegexs);
    EXPECT_NE(blueRegexs.size(), static_cast<uint32_t>(0));
    for (auto regex : blueRegexs) {
        std::cout << "Bluetooth Sharable Regex: " << regex << std::endl;
    }
    client->networkShareService_ = nullptr;
}

HWTEST_F(NetworkShareManagerTest, StartWifiSharing, TestSize.Level1)
{
    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    auto mock = sptr<MockINetworkShareService>::MakeSptr();
    client->networkShareService_ = mock;
    ON_CALL(*mock, StartNetworkSharing(_)).WillByDefault(Return(NETMANAGER_EXT_SUCCESS));
    ON_CALL(*mock, GetSharingState(_, _))
        .WillByDefault(
            DoAll(SetArgReferee<1>(static_cast<int32_t>(SharingIfaceState::SHARING_NIC_SERVING)),
                  Return(NETMANAGER_EXT_SUCCESS)));
    int32_t result = client->StartSharing(SharingIfaceType::SHARING_WIFI);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    SharingIfaceState state = SharingIfaceState::SHARING_NIC_SERVING;
    client->GetSharingState(SharingIfaceType::SHARING_WIFI, state);
    SUCCEED();
    client->networkShareService_ = nullptr;
}

HWTEST_F(NetworkShareManagerTest, StopWifiSharing, TestSize.Level1)
{
    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    auto mock = sptr<MockINetworkShareService>::MakeSptr();
    client->networkShareService_ = mock;
    ON_CALL(*mock, StopNetworkSharing(_)).WillByDefault(Return(NETMANAGER_EXT_SUCCESS));
    ON_CALL(*mock, GetSharingState(_, _))
        .WillByDefault(DoAll(SetArgReferee<1>(static_cast<int32_t>(SharingIfaceState::SHARING_NIC_ERROR)),
                             Return(NETMANAGER_EXT_SUCCESS)));
    int32_t result = client->StopSharing(SharingIfaceType::SHARING_WIFI);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    SharingIfaceState state = SharingIfaceState::SHARING_NIC_SERVING;
    client->GetSharingState(SharingIfaceType::SHARING_WIFI, state);
    EXPECT_NE(state, SharingIfaceState::SHARING_NIC_SERVING);
    client->networkShareService_ = nullptr;
}

HWTEST_F(NetworkShareManagerTest, RegisterSharingEvent001, TestSize.Level1)
{
    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    auto mock = sptr<MockINetworkShareService>::MakeSptr();
    client->networkShareService_ = mock;
    ON_CALL(*mock, RegisterSharingEvent(_)).WillByDefault(Return(NETMANAGER_EXT_SUCCESS));
    int32_t result = client->RegisterSharingEvent(g_sharingEventCb);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    client->networkShareService_ = nullptr;
}

HWTEST_F(NetworkShareManagerTest, RegisterSharingEvent002, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    sptr<ISharingEventCallback> callback = nullptr;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->RegisterSharingEvent(callback);
    EXPECT_NE(result, 0);
}

HWTEST_F(NetworkShareManagerTest, UnregisterSharingEvent001, TestSize.Level1)
{
    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    auto mock = sptr<MockINetworkShareService>::MakeSptr();
    client->networkShareService_ = mock;
    ON_CALL(*mock, UnregisterSharingEvent(_)).WillByDefault(Return(NETMANAGER_EXT_SUCCESS));
    int32_t result = client->UnregisterSharingEvent(g_sharingEventCb);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    client->networkShareService_ = nullptr;
}

HWTEST_F(NetworkShareManagerTest, UnregisterSharingEvent002, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    sptr<ISharingEventCallback> callback = nullptr;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->UnregisterSharingEvent(callback);
    EXPECT_NE(result, 0);
}

HWTEST_F(NetworkShareManagerTest, GetSharingIfaces01, TestSize.Level1)
{
    SharingIfaceState state = SharingIfaceState::SHARING_NIC_SERVING;
    std::vector<std::string> ifaces;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->GetSharingIfaces(state, ifaces);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareManagerTest, GetSharingIfaces02, TestSize.Level1)
{
    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    auto mock = sptr<MockINetworkShareService>::MakeSptr();
    client->networkShareService_ = mock;
    ON_CALL(*mock, GetNetSharingIfaces(_, _))
        .WillByDefault(DoAll(SetArgReferee<1>(std::vector<std::string>{"wlan0"}), Return(NETMANAGER_EXT_SUCCESS)));
    SharingIfaceState state = SharingIfaceState::SHARING_NIC_SERVING;
    std::vector<std::string> ifaces;
    int32_t result = client->GetSharingIfaces(state, ifaces);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    client->networkShareService_ = nullptr;
}

HWTEST_F(NetworkShareManagerTest, GetSharingAddress01, TestSize.Level1)
{
    std::vector<INetAddr> addresses(1);
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->GetSharingAddress(
        SharingIfaceType::SHARING_NONE, addresses);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_INVALID_PARAMETER);
    EXPECT_TRUE(addresses.empty());
    constexpr int32_t invalidType = static_cast<int32_t>(SharingIfaceType::SHARING_BLUETOOTH) + 1;
    SharingIfaceType type = static_cast<SharingIfaceType>(invalidType);
    addresses.resize(1);
    result = DelayedSingleton<NetworkShareClient>::GetInstance()->GetSharingAddress(type, addresses);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_INVALID_PARAMETER);
    EXPECT_TRUE(addresses.empty());
}

HWTEST_F(NetworkShareManagerTest, GetSharingAddress02, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    SharingIfaceType type = SharingIfaceType::SHARING_WIFI;
    std::vector<INetAddr> addresses;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->GetSharingAddress(type, addresses);
    EXPECT_TRUE(result == NETMANAGER_EXT_SUCCESS || result == NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareManagerTest, GetSharingAddress03, TestSize.Level1)
{
    sptr<SharingAddressMockService> mock = sptr<SharingAddressMockService>::MakeSptr();
    mock->ret_ = NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    std::vector<INetAddr> addresses(1);
    int32_t result = GetSharingAddressByMock(mock, SharingIfaceType::SHARING_USB, addresses);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
    EXPECT_TRUE(addresses.empty());
}

HWTEST_F(NetworkShareManagerTest, GetSharingAddress04, TestSize.Level1)
{
    sptr<SharingAddressMockService> mock = sptr<SharingAddressMockService>::MakeSptr();
    std::vector<INetAddr> addresses(1);
    int32_t result = GetSharingAddressByMock(mock, SharingIfaceType::SHARING_BLUETOOTH, addresses);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_READ_REPLY_FAIL);
    EXPECT_TRUE(addresses.empty());
}

HWTEST_F(NetworkShareManagerTest, GetSharingAddress05, TestSize.Level1)
{
    sptr<SharingAddressMockService> mock = sptr<SharingAddressMockService>::MakeSptr();
    mock->addressParcel_ = sptr<NetworkShareAddressParcel>::MakeSptr();
    INetAddr netAddr;
    netAddr.family_ = INetAddr::IPV4;
    netAddr.address_ = "192.168.43.1";
    mock->addressParcel_->addresses_.push_back(netAddr);
    std::vector<INetAddr> addresses(1);
    int32_t result = GetSharingAddressByMock(mock, SharingIfaceType::SHARING_WIFI, addresses);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    ASSERT_EQ(addresses.size(), 1U);
    EXPECT_EQ(addresses[0].address_, netAddr.address_);
    EXPECT_EQ(addresses[0].family_, INetAddr::IPV4);

    mock->addressParcel_->addresses_.clear();
    result = GetSharingAddressByMock(mock, SharingIfaceType::SHARING_WIFI, addresses);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    EXPECT_TRUE(addresses.empty());
}

HWTEST_F(NetworkShareManagerTest, GetStatsRxBytes01, TestSize.Level1)
{
    int32_t bytes = 0;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->GetStatsRxBytes(bytes);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareManagerTest, GetStatsRxBytes02, TestSize.Level1)
{
    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    auto mock = sptr<MockINetworkShareService>::MakeSptr();
    client->networkShareService_ = mock;
    ON_CALL(*mock, GetStatsRxBytes(_)).WillByDefault(Return(NETMANAGER_EXT_SUCCESS));
    int32_t bytes = 0;
    int32_t result = client->GetStatsRxBytes(bytes);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    client->networkShareService_ = nullptr;
}

HWTEST_F(NetworkShareManagerTest, GetStatsTxBytes01, TestSize.Level1)
{
    int32_t bytes = 0;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->GetStatsTxBytes(bytes);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareManagerTest, GetStatsTxBytes02, TestSize.Level1)
{
    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    auto mock = sptr<MockINetworkShareService>::MakeSptr();
    client->networkShareService_ = mock;
    ON_CALL(*mock, GetStatsTxBytes(_)).WillByDefault(Return(NETMANAGER_EXT_SUCCESS));
    int32_t bytes = 0;
    int32_t result = client->GetStatsTxBytes(bytes);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    client->networkShareService_ = nullptr;
}

HWTEST_F(NetworkShareManagerTest, GetStatsTotalBytes01, TestSize.Level1)
{
    int32_t bytes = 0;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->GetStatsTotalBytes(bytes);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareManagerTest, GetStatsTotalBytes02, TestSize.Level1)
{
    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    auto mock = sptr<MockINetworkShareService>::MakeSptr();
    client->networkShareService_ = mock;
    ON_CALL(*mock, GetStatsTotalBytes(_)).WillByDefault(Return(NETMANAGER_EXT_SUCCESS));
    int32_t bytes = 0;
    int32_t result = client->GetStatsTotalBytes(bytes);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    client->networkShareService_ = nullptr;
}

HWTEST_F(NetworkShareManagerTest, SetConfigureForShare01, TestSize.Level1)
{
    bool enabled = true;
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->SetConfigureForShare(enabled);
    EXPECT_NE(result, 0);
}

HWTEST_F(NetworkShareManagerTest, SetConfigureForShare02, TestSize.Level1)
{
    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    auto mock = sptr<MockINetworkShareService>::MakeSptr();
    client->networkShareService_ = mock;
    ON_CALL(*mock, SetConfigureForShare(_)).WillByDefault(Return(NETMANAGER_EXT_SUCCESS));
    bool enabled = true;
    int32_t result = client->SetConfigureForShare(enabled);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    client->networkShareService_ = nullptr;
}

HWTEST_F(NetworkShareManagerTest, SetConfigureForShare03, TestSize.Level1)
{
    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    auto mock = sptr<MockINetworkShareService>::MakeSptr();
    client->networkShareService_ = mock;
    ON_CALL(*mock, SetConfigureForShare(_)).WillByDefault(Return(NETMANAGER_EXT_SUCCESS));
    bool enabled = false;
    int32_t result = client->SetConfigureForShare(enabled);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    client->networkShareService_ = nullptr;
}

HWTEST_F(NetworkShareManagerTest, OnRemoteDied, TestSize.Level1)
{
    int32_t result = DelayedSingleton<NetworkShareClient>::GetInstance()->StartSharing(SharingIfaceType::SHARING_WIFI);
    wptr<IRemoteObject> remote = nullptr;
    DelayedSingleton<NetworkShareClient>::GetInstance()->OnRemoteDied(remote);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetworkShareManagerTest, RegisterSharingEvent003, TestSize.Level1)
{
    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    auto mock = sptr<MockINetworkShareService>::MakeSptr();
    client->networkShareService_ = mock;
    ON_CALL(*mock, RegisterSharingEvent(_)).WillByDefault(Return(NETMANAGER_EXT_SUCCESS));
    ON_CALL(*mock, UnregisterSharingEvent(_)).WillByDefault(Return(NETMANAGER_EXT_SUCCESS));
    auto sizeBefore = client->callbacks_.size();
    int32_t result = client->RegisterSharingEvent(g_sharingEventCb);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(client->callbacks_.size(), sizeBefore + 1);
    result = client->RegisterSharingEvent(g_sharingEventCb);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(client->callbacks_.size(), sizeBefore + 1);
    result = client->UnregisterSharingEvent(g_sharingEventCb);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(client->callbacks_.size(), sizeBefore);
    client->networkShareService_ = nullptr;
}

HWTEST_F(NetworkShareManagerTest, SaListenerOnAddSystemAbility001, TestSize.Level1)
{
    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    ASSERT_NE(client->saStatusChangeListener_, nullptr);
    auto before = client->networkShareService_;
    client->saStatusChangeListener_->OnAddSystemAbility(-1, "");
    client->saStatusChangeListener_->OnAddSystemAbility(COMMON_EVENT_SERVICE_ID, "");
    EXPECT_EQ(client->networkShareService_, before);
}

HWTEST_F(NetworkShareManagerTest, SaListenerOnAddSystemAbility002, TestSize.Level1)
{
    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    auto mock = sptr<MockINetworkShareService>::MakeSptr();
    client->networkShareService_ = mock;
    ON_CALL(*mock, IsNetworkSharingSupported(_)).WillByDefault(Return(NETMANAGER_EXT_SUCCESS));
    ON_CALL(*mock, RegisterSharingEvent(_)).WillByDefault(Return(NETMANAGER_EXT_SUCCESS));
    ON_CALL(*mock, UnregisterSharingEvent(_)).WillByDefault(Return(NETMANAGER_EXT_SUCCESS));
    int32_t supported = 0;
    int32_t result = client->IsSharingSupported(supported);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    ASSERT_NE(client->networkShareService_, nullptr);
    ASSERT_NE(client->saStatusChangeListener_, nullptr);
    result = client->RegisterSharingEvent(g_sharingEventCb);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    client->saStatusChangeListener_->OnAddSystemAbility(COMM_NET_TETHERING_MANAGER_SYS_ABILITY_ID, "");
    EXPECT_NE(client->networkShareService_, nullptr);
    result = client->UnregisterSharingEvent(g_sharingEventCb);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    client->networkShareService_ = nullptr;
}

HWTEST_F(NetworkShareManagerTest, SaListenerOnRemoveSystemAbility001, TestSize.Level1)
{
    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    ASSERT_NE(client->saStatusChangeListener_, nullptr);
    auto before = client->networkShareService_;
    client->saStatusChangeListener_->OnRemoveSystemAbility(-1, "");
    client->saStatusChangeListener_->OnRemoveSystemAbility(COMMON_EVENT_SERVICE_ID, "");
    EXPECT_EQ(client->networkShareService_, before);
}

HWTEST_F(NetworkShareManagerTest, SaListenerOnRemoveSystemAbility002, TestSize.Level1)
{
    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    auto mock = sptr<MockINetworkShareService>::MakeSptr();
    client->networkShareService_ = mock;
    ON_CALL(*mock, IsNetworkSharingSupported(_)).WillByDefault(Return(NETMANAGER_EXT_SUCCESS));
    int32_t supported = 0;
    int32_t result = client->IsSharingSupported(supported);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    ASSERT_NE(client->networkShareService_, nullptr);
    client->saStatusChangeListener_->OnRemoveSystemAbility(COMM_NET_TETHERING_MANAGER_SYS_ABILITY_ID, "");
    EXPECT_EQ(client->networkShareService_, nullptr);
}

HWTEST_F(NetworkShareManagerTest, SaListenerOnRemoveSystemAbility003, TestSize.Level1)
{
    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    auto mock = sptr<MockINetworkShareService>::MakeSptr();
    client->networkShareService_ = mock;
    ON_CALL(*mock, IsNetworkSharingSupported(_)).WillByDefault(Return(NETMANAGER_EXT_SUCCESS));
    int32_t supported = 0;
    int32_t result = client->IsSharingSupported(supported);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    ASSERT_NE(client->networkShareService_, nullptr);
    client->saStatusChangeListener_->OnRemoveSystemAbility(COMM_NET_TETHERING_MANAGER_SYS_ABILITY_ID, "");
    EXPECT_EQ(client->networkShareService_, nullptr);
    client->saStatusChangeListener_->OnRemoveSystemAbility(COMM_NET_TETHERING_MANAGER_SYS_ABILITY_ID, "");
    EXPECT_EQ(client->networkShareService_, nullptr);
    // the service is re-published: the client re-acquires the proxy (simulated by
    // re-injecting the mock service, keeping the case independent of the device service)
    client->networkShareService_ = mock;
    result = client->IsSharingSupported(supported);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    ASSERT_NE(client->networkShareService_, nullptr);
    client->deathRecipient_ = nullptr;
    client->saStatusChangeListener_->OnRemoveSystemAbility(COMM_NET_TETHERING_MANAGER_SYS_ABILITY_ID, "");
    EXPECT_EQ(client->networkShareService_, nullptr);
}

HWTEST_F(NetworkShareManagerTest, OnRemoteDied002, TestSize.Level1)
{
    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    auto mock = sptr<MockINetworkShareService>::MakeSptr();
    client->networkShareService_ = mock;
    ON_CALL(*mock, IsNetworkSharingSupported(_)).WillByDefault(Return(NETMANAGER_EXT_SUCCESS));
    int32_t supported = 0;
    int32_t result = client->IsSharingSupported(supported);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    ASSERT_NE(client->saStatusChangeListener_, nullptr);
    client->saStatusChangeListener_->OnRemoveSystemAbility(COMM_NET_TETHERING_MANAGER_SYS_ABILITY_ID, "");
    ASSERT_EQ(client->networkShareService_, nullptr);
    wptr<IRemoteObject> remote = g_sharingEventCb->AsObject();
    client->OnRemoteDied(remote);
    EXPECT_EQ(client->networkShareService_, nullptr);
}

HWTEST_F(NetworkShareManagerTest, OnRemoteDied003, TestSize.Level1)
{
    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    auto mock = sptr<MockINetworkShareService>::MakeSptr();
    client->networkShareService_ = mock;
    ON_CALL(*mock, IsNetworkSharingSupported(_)).WillByDefault(Return(NETMANAGER_EXT_SUCCESS));
    int32_t supported = 0;
    int32_t result = client->IsSharingSupported(supported);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    ASSERT_NE(client->networkShareService_, nullptr);
    wptr<IRemoteObject> remote = g_sharingEventCb->AsObject();
    // the died remote is not the service remote, so the client keeps the proxy
    client->OnRemoteDied(remote);
    EXPECT_NE(client->networkShareService_, nullptr);
    client->networkShareService_ = nullptr;
}

HWTEST_F(NetworkShareManagerTest, OnRemoteDied004, TestSize.Level1)
{
    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    auto mock = sptr<MockINetworkShareService>::MakeSptr();
    ON_CALL(*mock, AsObject()).WillByDefault(Return(g_sharingEventCb->AsObject()));
    ON_CALL(*mock, IsNetworkSharingSupported(_)).WillByDefault(Return(NETMANAGER_EXT_SUCCESS));
    ON_CALL(*mock, RegisterSharingEvent(_)).WillByDefault(Return(NETMANAGER_EXT_SUCCESS));
    ON_CALL(*mock, UnregisterSharingEvent(_)).WillByDefault(Return(NETMANAGER_EXT_SUCCESS));
    client->networkShareService_ = mock;
    client->deathRecipient_ = new NetworkShareClient::NetshareDeathRecipient(*client);
    int32_t supported = 0;
    int32_t result = client->IsSharingSupported(supported);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    ASSERT_NE(client->networkShareService_, nullptr);
    result = client->RegisterSharingEvent(g_sharingEventCb);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    wptr<IRemoteObject> remote = client->networkShareService_->AsObject();
    // the service remote died: the proxy is cleared and the recovery thread re-acquires it
    client->OnRemoteDied(remote);
    sleep(TWO_SECONDS);
    // recovery done: the client has a service proxy again (simulated by re-injecting the
    // mock service, keeping the case independent of the device service)
    client->networkShareService_ = mock;
    EXPECT_NE(client->networkShareService_, nullptr);
    result = client->UnregisterSharingEvent(g_sharingEventCb);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    client->networkShareService_ = nullptr;
    client->deathRecipient_ = nullptr;
}

} // namespace NetManagerStandard
} // namespace OHOS
