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

#include "ipsecvpn_config.h"
#include "netmanager_ext_test_security.h"
#include "network_vpn_service_proxy.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
} // namespace

class MockNetIRemoteObject : public IRemoteObject {
public:
    MockNetIRemoteObject() : IRemoteObject(u"mock_i_remote_object") {}
    ~MockNetIRemoteObject() {}

    int32_t GetObjectRefCount() override
    {
        return 0;
    }

    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        switch (code) {
            case static_cast<uint32_t>(INetworkVpnServiceIpcCode::COMMAND_DELETE_SYS_VPN_CONFIG):
                reply.WriteString("test1");
                break;
            case static_cast<uint32_t>(INetworkVpnServiceIpcCode::COMMAND_GET_SYS_VPN_CONFIG): {
                sptr<SysVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
                if (config == nullptr) {
                    return eCode;
                }
                config->vpnId_ = "test2";
                config->vpnName_ = "test";
                config->vpnType_ = 1;
                config->Marshalling(reply);
                reply.WriteString("test1");
                break;
            }
            case static_cast<uint32_t>(INetworkVpnServiceIpcCode::COMMAND_GET_CONNECTED_SYS_VPN_CONFIG):
            case static_cast<uint32_t>(INetworkVpnServiceIpcCode::COMMAND_ADD_SYS_VPN_CONFIG):
            case static_cast<uint32_t>(INetworkVpnServiceIpcCode::COMMAND_SET_UP_VPN):
            case static_cast<uint32_t>(INetworkVpnServiceIpcCode::COMMAND_SET_UP_SYS_VPN):
            case static_cast<uint32_t>(INetworkVpnServiceIpcCode::COMMAND_GET_SYS_VPN_CONFIG_LIST): {
                sptr<SysVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
                if (config == nullptr) {
                    return eCode;
                }
                config->vpnId_ = "test3";
                config->vpnName_ = "test";
                config->vpnType_ = 1;
                config->Marshalling(reply);
                break;
            }
            case static_cast<uint32_t>(INetworkVpnServiceIpcCode::COMMAND_NOTIFY_CONNECT_STAGE): {
                reply.WriteString("stop");
                reply.WriteInt32(1);
                break;
            }
            case static_cast<uint32_t>(INetworkVpnServiceIpcCode::COMMAND_GET_SYS_VPN_CERT_URI):
                reply.WriteInt32(1);
                break;
            default:
                reply.WriteInt32(1);
                break;
        }
        return eCode;
    }

    bool IsProxyObject() const override
    {
        return true;
    }

    bool CheckObjectLegality() const override
    {
        return true;
    }

    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }

    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }

    bool Marshalling(Parcel &parcel) const override
    {
        return true;
    }

    sptr<IRemoteBroker> AsInterface() override
    {
        return nullptr;
    }

    int Dump(int fd, const std::vector<std::u16string> &args) override
    {
        return 0;
    }

    std::u16string GetObjectDescriptor() const
    {
        std::u16string descriptor = std::u16string();
        return descriptor;
    }

    void SetErrorCode(int errorCode)
    {
        eCode = errorCode;
    }

private:
    int eCode = NETMANAGER_EXT_SUCCESS;
};

class NetworkVpnServiceProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline sptr<MockNetIRemoteObject> remoteObj_ = std::make_unique<MockNetIRemoteObject>().release();
    static inline std::shared_ptr<NetworkVpnServiceProxy> instance_ =
        std::make_shared<NetworkVpnServiceProxy>(remoteObj_);
};

void NetworkVpnServiceProxyTest::SetUpTestCase() {}

void NetworkVpnServiceProxyTest::TearDownTestCase() {}

void NetworkVpnServiceProxyTest::SetUp() {}

void NetworkVpnServiceProxyTest::TearDown() {}

HWTEST_F(NetworkVpnServiceProxyTest, SetUpVpn001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    sptr<SysVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(config, nullptr);
    auto ret = instance_->SetUpVpn(*config, true);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
    config->vpnId_ = "test4";
    config->vpnName_ = "SetUpVpn001";
    config->vpnType_ = 1;
    ret = instance_->SetUpVpn(*config, true);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceProxyTest, SetUpVpn002, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    sptr<SysVpnConfig> config = nullptr;
    auto ret = instance_->SetUpSysVpn(config);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    config = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(config, nullptr);
    ret = instance_->SetUpSysVpn(config);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceProxyTest, AddSysVpnConfig001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    sptr<SysVpnConfig> config = nullptr;
    auto ret = instance_->AddSysVpnConfig(config);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    config = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(config, nullptr);
    ret = instance_->AddSysVpnConfig(config);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceProxyTest, DeleteSysVpnConfig001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    std::string vpnId;
    auto ret = instance_->DeleteSysVpnConfig(vpnId);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetworkVpnServiceProxyTest, GetSysVpnConfig001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    std::string vpnId;
    sptr<SysVpnConfig> resConfig = new (std::nothrow) IpsecVpnConfig();
    auto ret = instance_->GetSysVpnConfig(resConfig, vpnId);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceProxyTest, NotifyConnectStage001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    std::string stage;
    int32_t result = 100;
    auto ret = instance_->NotifyConnectStage(stage, result);
    EXPECT_NE(ret, NETMANAGER_EXT_ERR_WRITE_DATA_FAIL);
}

HWTEST_F(NetworkVpnServiceProxyTest, FactoryResetVpn001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    auto ret = instance_->FactoryResetVpn();
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceProxyTest, Protect001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    auto ret = instance_->Protect(true);
    EXPECT_NE(ret, ERR_NONE);
}

HWTEST_F(NetworkVpnServiceProxyTest, DestroyVpn001, TestSize.Level1)
{
    auto ret = instance_->DestroyVpn(true);
    EXPECT_NE(ret, ERR_NONE);
}

HWTEST_F(NetworkVpnServiceProxyTest, CreateVpnConnection001, TestSize.Level1)
{
    auto ret = instance_->CreateVpnConnection(true);
    EXPECT_NE(ret, ERR_NONE);
}
} // namespace NetManagerStandard
} // namespace OHOS