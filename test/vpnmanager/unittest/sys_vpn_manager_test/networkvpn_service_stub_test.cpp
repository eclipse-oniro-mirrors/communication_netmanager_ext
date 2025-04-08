/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "ipsecvpn_config.h"
#include "mock_networkvpn_service_stub_test.h"
#include "mock_vpn_event_callback_test.h"
#include "net_manager_constants.h"
#include "network_vpn_service_stub.h"
#include "netmanager_ext_test_security.h"
#include "sharing_event_callback_stub.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
class NetworkVpnServiceStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    static inline std::shared_ptr<NetworkVpnServiceStub> instance_ = std::make_shared<MockNetworkVpnServiceStub>();
    static int32_t SendRemoteRequest(MessageParcel &data, INetworkVpnServiceIpcCode code);
};

void NetworkVpnServiceStubTest::SetUpTestCase() {}

void NetworkVpnServiceStubTest::TearDownTestCase() {}

int32_t NetworkVpnServiceStubTest::SendRemoteRequest(MessageParcel &data, INetworkVpnServiceIpcCode code)
{
    if (instance_ == nullptr) {
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    MessageParcel reply;
    MessageOption option;
    return instance_->OnRemoteRequest(static_cast<uint32_t>(code), data, reply, option);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyAddSysVpnConfigTest001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnServiceIpcCode::COMMAND_ADD_SYS_VPN_CONFIG);

    // config is null
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_READ_DATA_FAIL);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyAddSysVpnConfigTest002, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    sptr<SysVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    if (config == nullptr) {
        return;
    }
    config->vpnId_ = "1234";
    config->vpnName_ = "test";
    config->vpnType_ = 1;
    if (!config->Marshalling(data)) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnServiceIpcCode::COMMAND_ADD_SYS_VPN_CONFIG);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyDeleteSysVpnConfigTest001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnServiceIpcCode::COMMAND_DELETE_SYS_VPN_CONFIG);

    // vpnId is null
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_READ_DATA_FAIL);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyDeleteSysVpnConfigTest002, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    std::string vpnId = "1234";
    if (!data.WriteString(vpnId)) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnServiceIpcCode::COMMAND_DELETE_SYS_VPN_CONFIG);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyGetSysVpnConfigListTest001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnServiceIpcCode::COMMAND_GET_SYS_VPN_CONFIG_LIST);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyGetSysVpnConfigTest001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnServiceIpcCode::COMMAND_GET_SYS_VPN_CONFIG);

    // vpnId is null
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_READ_DATA_FAIL);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyGetSysVpnConfigTest002, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    std::string vpnId = "1234";
    if (!data.WriteString(vpnId)) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnServiceIpcCode::COMMAND_GET_SYS_VPN_CONFIG);

    // vpnId is null
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyGetConnectedSysVpnConfigTest001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnServiceIpcCode::COMMAND_GET_CONNECTED_SYS_VPN_CONFIG);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyNotifyConnectStageTest001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    data.WriteString("stop");
    data.WriteInt32(100);
    int32_t ret = SendRemoteRequest(data, INetworkVpnServiceIpcCode::COMMAND_NOTIFY_CONNECT_STAGE);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, OnRemoteRequest001, TestSize.Level1)
{
    MessageParcel data;
    int32_t ret = SendRemoteRequest(data, INetworkVpnServiceIpcCode::COMMAND_NOTIFY_CONNECT_STAGE);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_DESCRIPTOR_MISMATCH);
    MessageParcel data1;
    if (!data1.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    ret = SendRemoteRequest(data1, INetworkVpnServiceIpcCode::COMMAND_ADD_SYS_VPN_CONFIG);
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}
} // namespace NetManagerStandard
} // namespace OHOS
