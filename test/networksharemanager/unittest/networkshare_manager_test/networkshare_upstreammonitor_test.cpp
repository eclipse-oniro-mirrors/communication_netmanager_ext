/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#define private public
#include "networkshare_upstreammonitor.h"
#undef private

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
} // namespace

class NetworkShareUpstreamMonitorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::weak_ptr<NetworkShareUpstreamMonitor::MonitorEventHandler> monitorHandler_;
};
void NetworkShareUpstreamMonitorTest::SetUpTestCase() {}
void NetworkShareUpstreamMonitorTest::TearDownTestCase() {}
void NetworkShareUpstreamMonitorTest::SetUp() {}
void NetworkShareUpstreamMonitorTest::TearDown() {}

class UpstreamCallbackTest : public NetworkShareUpstreamMonitor::NotifyUpstreamCallback {
public:
    UpstreamCallbackTest() = default;
    virtual ~UpstreamCallbackTest() = default;

    void OnUpstreamStateChanged(int32_t msgName, int32_t param1) override {}
    void OnUpstreamStateChanged(int32_t msgName, int32_t param1, int32_t param2, const std::any &messageObj) override {}
};
HWTEST_F(NetworkShareUpstreamMonitorTest, UpstreamMonitorTest, TestSize.Level1)
{
    auto monitor = NetworkShareUpstreamMonitor::GetInstance();
    if (monitor == nullptr) {
        return;
    }
    int32_t netId = 0;
    sptr<NetHandle> netHandle = std::make_unique<NetHandle>(netId).release();
    sptr<NetAllCapabilities> netAllCap = std::make_unique<NetAllCapabilities>().release();
    sptr<NetLinkInfo> info = std::make_unique<NetLinkInfo>().release();
    monitor->defaultNetworkCallback_ =
        new (std::nothrow) NetworkShareUpstreamMonitor::NetConnectionCallback(nullptr, netId);
    ASSERT_TRUE(monitor->defaultNetworkCallback_ != nullptr);
    int32_t result = monitor->defaultNetworkCallback_->NetAvailable(netHandle);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    result = monitor->defaultNetworkCallback_->NetCapabilitiesChange(netHandle, netAllCap);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    result = monitor->defaultNetworkCallback_->NetConnectionPropertiesChange(netHandle, info);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    result = monitor->defaultNetworkCallback_->NetLost(netHandle);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    result = monitor->defaultNetworkCallback_->NetUnavailable();
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    result = monitor->defaultNetworkCallback_->NetBlockStatusChange(netHandle, true);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);

    int32_t eventId = 0;
    monitorHandler_ = std::make_shared<NetworkShareUpstreamMonitor::MonitorEventHandler>(monitor, nullptr);
    monitor->SetOptionData(eventId);
    monitor->RegisterUpstreamChangedCallback(nullptr);
}

HWTEST_F(NetworkShareUpstreamMonitorTest, HandleNetAvailableTest, TestSize.Level1)
{
    auto monitor = NetworkShareUpstreamMonitor::GetInstance();
    if (monitor == nullptr) {
        return;
    }
    monitor->ListenDefaultNetwork();
    int32_t netId = 0;
    sptr<NetHandle> netHandle = nullptr;
    int32_t result = monitor->defaultNetworkCallback_->NetAvailable(netHandle);
    netHandle = std::make_unique<NetHandle>(netId).release();
    result = monitor->defaultNetworkCallback_->NetAvailable(netHandle);
    result = monitor->defaultNetworkCallback_->NetAvailable(netHandle);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareUpstreamMonitorTest, HandleNetCapabilitiesChangeTest, TestSize.Level1)
{
    auto monitor = NetworkShareUpstreamMonitor::GetInstance();
    if (monitor == nullptr) {
        return;
    }
    monitor->ListenDefaultNetwork();
    int32_t netId = 0;
    sptr<NetHandle> netHandle = nullptr;
    sptr<NetAllCapabilities> netAllCap = nullptr;
    int32_t result = monitor->defaultNetworkCallback_->NetCapabilitiesChange(netHandle, netAllCap);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);

    netHandle = std::make_unique<NetHandle>(netId).release();
    result = monitor->defaultNetworkCallback_->NetCapabilitiesChange(netHandle, netAllCap);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);

    netAllCap = std::make_unique<NetAllCapabilities>().release();
    result = monitor->defaultNetworkCallback_->NetCapabilitiesChange(netHandle, netAllCap);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);

    netAllCap->linkUpBandwidthKbps_ = 10;
    result = monitor->defaultNetworkCallback_->NetCapabilitiesChange(netHandle, netAllCap);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);

    monitor->networkMaps_.clear();
    result = monitor->defaultNetworkCallback_->NetCapabilitiesChange(netHandle, netAllCap);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareUpstreamMonitorTest, HandleConnectionPropertiesChangeTest, TestSize.Level1)
{
    auto monitor = NetworkShareUpstreamMonitor::GetInstance();
    if (monitor == nullptr) {
        return;
    }
    monitor->ListenDefaultNetwork();
    int32_t netId = 0;
    sptr<NetHandle> netHandle = nullptr;
    sptr<NetLinkInfo> info = nullptr;
    int32_t result = monitor->defaultNetworkCallback_->NetConnectionPropertiesChange(netHandle, info);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);

    netHandle = std::make_unique<NetHandle>(netId).release();
    result = monitor->defaultNetworkCallback_->NetConnectionPropertiesChange(netHandle, info);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);

    info = std::make_unique<NetLinkInfo>().release();
    result = monitor->defaultNetworkCallback_->NetConnectionPropertiesChange(netHandle, info);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);

    result = monitor->defaultNetworkCallback_->NetAvailable(netHandle);
    result = monitor->defaultNetworkCallback_->NetConnectionPropertiesChange(netHandle, info);

    info->ifaceName_ = "eth0";
    result = monitor->defaultNetworkCallback_->NetConnectionPropertiesChange(netHandle, info);
    result = monitor->defaultNetworkCallback_->NetConnectionPropertiesChange(netHandle, info);
    monitor->networkMaps_.clear();
    result = monitor->defaultNetworkCallback_->NetConnectionPropertiesChange(netHandle, info);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareUpstreamMonitorTest, HandleNetLostTest, TestSize.Level1)
{
    auto monitor = NetworkShareUpstreamMonitor::GetInstance();
    if (monitor == nullptr) {
        return;
    }
    monitor->ListenDefaultNetwork();
    int32_t netId = 0;
    sptr<NetHandle> netHandle = nullptr;
    int32_t result = monitor->defaultNetworkCallback_->NetLost(netHandle);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);

    netHandle = std::make_unique<NetHandle>(netId).release();
    result = monitor->defaultNetworkCallback_->NetLost(netHandle);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkShareUpstreamMonitorTest, GetCurrentGoodUpstreamTest, TestSize.Level1)
{
    auto monitor = NetworkShareUpstreamMonitor::GetInstance();
    if (monitor == nullptr) {
        return;
    }

    int32_t netId = 0;
    sptr<NetHandle> netHandle = nullptr;
    sptr<NetAllCapabilities> netAllCap = nullptr;
    sptr<NetLinkInfo> info = nullptr;
    auto upstreamNetInfo = std::make_shared<UpstreamNetworkInfo>(netHandle, netAllCap, info);

    monitor->networkMaps_.clear();
    monitor->defaultNetworkId_ = 100;
    bool result = monitor->GetCurrentGoodUpstream(upstreamNetInfo);
    EXPECT_FALSE(result);

    monitor->networkMaps_.insert(std::make_pair(monitor->defaultNetworkId_, upstreamNetInfo));
    result = monitor->GetCurrentGoodUpstream(upstreamNetInfo);
    EXPECT_TRUE(result);
}

HWTEST_F(NetworkShareUpstreamMonitorTest, NotifyMainStateMachineTest, TestSize.Level1)
{
    auto monitor = NetworkShareUpstreamMonitor::GetInstance();
    if (monitor == nullptr) {
        return;
    }
    monitor->ListenDefaultNetwork();
    int32_t which = 100;
    monitor->NotifyMainStateMachine(which, nullptr);
    monitor->NotifyMainStateMachine(which);
    auto notifyCallback = std::make_shared<UpstreamCallbackTest>();
    monitor->RegisterUpstreamChangedCallback(notifyCallback);
    EXPECT_NE(monitor->notifyUpstreamCallback_, nullptr);
    monitor->NotifyMainStateMachine(which, nullptr);
    monitor->NotifyMainStateMachine(which);
}

HWTEST_F(NetworkShareUpstreamMonitorTest, HandleNetAvailable01, TestSize.Level1)
{
    sptr<NetHandle> netHandle = nullptr;
    NetworkShareUpstreamMonitor::GetInstance()->HandleNetAvailable(netHandle);
    int32_t netId = 0;
    netHandle = std::make_unique<NetHandle>(netId).release();
    NetworkShareUpstreamMonitor::GetInstance()->HandleNetAvailable(netHandle);
    EXPECT_NE(netHandle, nullptr);
}
 
HWTEST_F(NetworkShareUpstreamMonitorTest, HandleNetCapabilitiesChange01, TestSize.Level1)
{
    sptr<NetHandle> netHandle = nullptr;
    sptr<NetAllCapabilities> newNetAllCap = nullptr;
    NetworkShareUpstreamMonitor::GetInstance()->HandleNetCapabilitiesChange(netHandle, newNetAllCap);
    int32_t netId = 0;
    netHandle = std::make_unique<NetHandle>(netId).release();
    NetworkShareUpstreamMonitor::GetInstance()->HandleNetCapabilitiesChange(netHandle, newNetAllCap);
    EXPECT_NE(netHandle, nullptr);
}
 
HWTEST_F(NetworkShareUpstreamMonitorTest, HandleNetCapabilitiesChange02, TestSize.Level1)
{
    sptr<NetHandle> netHandle = nullptr;
    sptr<NetAllCapabilities> newNetAllCap = std::make_unique<NetAllCapabilities>().release();
    NetworkShareUpstreamMonitor::GetInstance()->HandleNetCapabilitiesChange(netHandle, newNetAllCap);
    int32_t netId = 0;
    netHandle = std::make_unique<NetHandle>(netId).release();
    NetworkShareUpstreamMonitor::GetInstance()->HandleNetCapabilitiesChange(netHandle, newNetAllCap);
    EXPECT_NE(netHandle, nullptr);
}
 
HWTEST_F(NetworkShareUpstreamMonitorTest, HandleConnectionPropertiesChange01, TestSize.Level1)
{
    sptr<NetHandle> netHandle = nullptr;
    sptr<NetLinkInfo> newNetLinkInfo = nullptr;
    NetworkShareUpstreamMonitor::GetInstance()->HandleConnectionPropertiesChange(netHandle, newNetLinkInfo);
    int32_t netId = 0;
    netHandle = std::make_unique<NetHandle>(netId).release();
    NetworkShareUpstreamMonitor::GetInstance()->HandleConnectionPropertiesChange(netHandle, newNetLinkInfo);
    EXPECT_NE(netHandle, nullptr);
}
 
HWTEST_F(NetworkShareUpstreamMonitorTest, HandleConnectionPropertiesChange02, TestSize.Level1)
{
    sptr<NetHandle> netHandle = nullptr;
    sptr<NetLinkInfo> newNetLinkInfo = std::make_unique<NetLinkInfo>().release();
    NetworkShareUpstreamMonitor::GetInstance()->HandleConnectionPropertiesChange(netHandle, newNetLinkInfo);
    int32_t netId = 0;
    netHandle = std::make_unique<NetHandle>(netId).release();
    NetworkShareUpstreamMonitor::GetInstance()->HandleConnectionPropertiesChange(netHandle, newNetLinkInfo);
    EXPECT_NE(netHandle, nullptr);
}
 
HWTEST_F(NetworkShareUpstreamMonitorTest, HandleNetLost01, TestSize.Level1)
{
    sptr<NetHandle> netHandle = nullptr;
    NetworkShareUpstreamMonitor::GetInstance()->HandleNetLost(netHandle);
    int32_t netId = 0;
    netHandle = std::make_unique<NetHandle>(netId).release();
    NetworkShareUpstreamMonitor::GetInstance()->HandleNetLost(netHandle);
    EXPECT_NE(netHandle, nullptr);
}

HWTEST_F(NetworkShareUpstreamMonitorTest, OnNetworkConnectChangeTest001, TestSize.Level1)
{
    auto monitor = NetworkShareUpstreamMonitor::GetInstance();
    if (monitor == nullptr) {
        return;
    }
    int32_t state = NET_CONN_STATE_CONNECTED;
    int32_t bearerType = BEARER_WIFI;
    monitor->OnNetworkConnectChange(state, bearerType);
    bearerType = BEARER_CELLULAR;
    monitor->OnNetworkConnectChange(state, bearerType);
    state = NET_CONN_STATE_DISCONNECTED;
    monitor->OnNetworkConnectChange(state, bearerType);
    state = NET_CONN_STATE_IDLE;
    monitor->OnNetworkConnectChange(state, bearerType);
    EXPECT_NE(monitor->defaultNetworkId_, 0);
}
} // namespace NetManagerStandard
} // namespace OHOS
