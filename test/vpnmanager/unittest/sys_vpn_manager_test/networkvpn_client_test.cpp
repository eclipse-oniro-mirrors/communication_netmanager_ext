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
#include "ipsecvpn_config.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
} // namespace
class NetworkVpnClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();

    NetworkVpnClient &networkVpnClient_ = NetworkVpnClient::GetInstance();
};

void NetworkVpnClientTest::SetUpTestCase() {}

void NetworkVpnClientTest::TearDownTestCase() {}

HWTEST_F(NetworkVpnClientTest, SetUpVpn001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    sptr<SysVpnConfig> config = nullptr;
    int32_t ret = networkVpnClient_.SetUpVpn(config);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    config = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(config, nullptr);
    config->vpnId_ = "123";
    ret = networkVpnClient_.SetUpVpn(config);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INTERNAL);
    config->vpnName_ = "testSetUpVpn";
    config->vpnType_ = 1;
    ret = networkVpnClient_.SetUpVpn(config);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INTERNAL);
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
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_READ_DATA_FAIL);
    config->vpnId_ = id;
    config->vpnName_ = "test";
    config->vpnType_ = 1;
    EXPECT_EQ(networkVpnClient_.AddSysVpnConfig(config), NETMANAGER_EXT_SUCCESS);
    // delete test config
    EXPECT_EQ(networkVpnClient_.DeleteSysVpnConfig(id), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnClientTest, AddSysVpnConfig002, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    std::string id = "1234";
    sptr<SysVpnConfig> config = new (std::nothrow) SysVpnConfig();
    if (config == nullptr) {
        return;
    }
    config->vpnId_ = id;
    config->vpnName_ = "test";
    config->vpnType_ = 1;
    EXPECT_EQ(networkVpnClient_.AddSysVpnConfig(config), NETMANAGER_EXT_ERR_READ_DATA_FAIL);
}

HWTEST_F(NetworkVpnClientTest, AddSysVpnConfig003, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    sptr<SysVpnConfig> config = nullptr;
    EXPECT_EQ(networkVpnClient_.AddSysVpnConfig(config), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetworkVpnClientTest, DeleteSysVpnConfig001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    std::string id = "1234";
    sptr<SysVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    if (config == nullptr) {
        return;
    }
    config->vpnId_ = id;
    config->vpnName_ = "test";
    config->vpnType_ = 1;
    EXPECT_EQ(networkVpnClient_.AddSysVpnConfig(config), NETMANAGER_EXT_SUCCESS);
    // delete test config
    std::string emptyStr;
    int32_t ret = networkVpnClient_.DeleteSysVpnConfig(emptyStr);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    ret = networkVpnClient_.DeleteSysVpnConfig(id);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
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
    std::vector<SysVpnConfig> list;
    EXPECT_EQ(networkVpnClient_.GetSysVpnConfigList(list), NETMANAGER_EXT_ERR_OPERATION_FAILED);
}

HWTEST_F(NetworkVpnClientTest, GetSysVpnConfig001, TestSize.Level1)
{
    NetManagerExtAccessToken access;
    std::string id = "1234";
    sptr<SysVpnConfig> resConfig = nullptr;
    EXPECT_EQ(networkVpnClient_.GetSysVpnConfig(resConfig, id), NETMANAGER_EXT_ERR_OPERATION_FAILED);
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
    EXPECT_EQ(networkVpnClient_.GetConnectedSysVpnConfig(resConfig), NETMANAGER_EXT_SUCCESS);
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
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL);
}
} // namespace NetManagerStandard
} // namespace OHOS
