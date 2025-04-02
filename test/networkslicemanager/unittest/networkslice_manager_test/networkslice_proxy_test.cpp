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

#include <gtest/gtest.h>
#include <arpa/inet.h>

#include "gtest/gtest-message.h"
#include "gtest/gtest-test-part.h"
#include "gtest/hwext/gtest-ext.h"
#include "gtest/hwext/gtest-tag.h"
#include "http_proxy.h"
#include "interface_type.h"
#include "net_manager_constants.h"
#include "netmanager_ext_test_security.h"
#include "netmgr_ext_log_wrapper.h"
#include "refbase.h"

#include "networkslice_client.h"
#include "networkslice_proxy.h"
#include "networkslice_service.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
}

class NetworkSliceProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetworkSliceProxyTest::SetUpTestCase() {}

void NetworkSliceProxyTest::TearDownTestCase() {}

void NetworkSliceProxyTest::SetUp() {}

void NetworkSliceProxyTest::TearDown() {}

HWTEST_F(NetworkSliceProxyTest, SetNetworkSliceUePolicy001, TestSize.Level1)
{
    NETMGR_EXT_LOG_I("NetworkSliceProxyTest:SetNetworkSliceUePolicy001");
    NetManagerExtAccessToken token;
    NetworkSliceProxy networkSliceProxy(nullptr);
    std::vector<uint8_t> buffer = {0};
    auto ret = networkSliceProxy.SetNetworkSliceUePolicy(buffer);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL);
}

HWTEST_F(NetworkSliceProxyTest, SetNetworkSliceUePolicy002, TestSize.Level1)
{
    NETMGR_EXT_LOG_I("NetworkSliceProxyTest:SetNetworkSliceUePolicy002");
    NetManagerExtAccessToken token;
    NetworkSliceProxy networkSliceProxy(nullptr);
    std::vector<uint8_t> buffer = {};
    auto ret = networkSliceProxy.SetNetworkSliceUePolicy(buffer);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INVALID_PARAMETER);
}

HWTEST_F(NetworkSliceProxyTest, NetworkSliceAllowedNssaiRpt001, TestSize.Level1)
{
    NETMGR_EXT_LOG_I("NetworkSliceProxyTest:NetworkSliceAllowedNssaiRpt001");
    NetManagerExtAccessToken token;
    NetworkSliceProxy networkSliceProxy(nullptr);
    std::vector<uint8_t> buffer = {0};
    auto ret = networkSliceProxy.NetworkSliceAllowedNssaiRpt(buffer);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL);
}

HWTEST_F(NetworkSliceProxyTest, NetworkSliceAllowedNssaiRpt002, TestSize.Level1)
{
    NETMGR_EXT_LOG_I("NetworkSliceProxyTest:NetworkSliceAllowedNssaiRpt002");
    NetManagerExtAccessToken token;
    NetworkSliceProxy networkSliceProxy(nullptr);
    std::vector<uint8_t> buffer = {};
    auto ret = networkSliceProxy.NetworkSliceAllowedNssaiRpt(buffer);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INVALID_PARAMETER);
}

HWTEST_F(NetworkSliceProxyTest, NetworkSliceEhplmnRpt001, TestSize.Level1)
{
    NETMGR_EXT_LOG_I("NetworkSliceProxyTest:NetworkSliceEhplmnRpt001");
    NetManagerExtAccessToken token;
    NetworkSliceProxy networkSliceProxy(nullptr);
    std::vector<uint8_t> buffer = {0};
    auto ret = networkSliceProxy.NetworkSliceEhplmnRpt(buffer);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL);
}

HWTEST_F(NetworkSliceProxyTest, NetworkSliceEhplmnRpt002, TestSize.Level1)
{
    NETMGR_EXT_LOG_I("NetworkSliceProxyTest:NetworkSliceEhplmnRpt002");
    NetManagerExtAccessToken token;
    NetworkSliceProxy networkSliceProxy(nullptr);
    std::vector<uint8_t> buffer = {};
    auto ret = networkSliceProxy.NetworkSliceEhplmnRpt(buffer);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INVALID_PARAMETER);
}

HWTEST_F(NetworkSliceProxyTest, NetworkSliceInitUePolicy001, TestSize.Level1)
{
    NETMGR_EXT_LOG_I("NetworkSliceProxyTest:NetworkSliceInitUePolicy001");
    NetManagerExtAccessToken token;
    NetworkSliceProxy networkSliceProxy(nullptr);
    auto ret = networkSliceProxy.NetworkSliceInitUePolicy();
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL);
}

HWTEST_F(NetworkSliceProxyTest, GetRouteSelectionDescriptorByDNN001, TestSize.Level1)
{
    NETMGR_EXT_LOG_I("NetworkSliceProxyTest:GetRouteSelectionDescriptorByDNN001");
    NetManagerExtAccessToken token;
    NetworkSliceProxy networkSliceProxy(nullptr);
    std::string dnn = "dnn";
    std::string snssai = "";
    uint8_t sscMode = 0;
    auto ret = networkSliceProxy.GetRouteSelectionDescriptorByDNN(dnn, snssai, sscMode);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL);
}

HWTEST_F(NetworkSliceProxyTest, GetRSDByNetCap001, TestSize.Level1)
{
    NETMGR_EXT_LOG_I("NetworkSliceProxyTest:GetRSDByNetCap001");
    NetManagerExtAccessToken token;
    NetworkSliceProxy networkSliceProxy(nullptr);
    int32_t netcap = 1;
    std::map<std::string, std::string> networkSliceParas;
    auto ret = networkSliceProxy.GetRSDByNetCap(netcap, networkSliceParas);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL);
}

HWTEST_F(NetworkSliceProxyTest, SetSaState001, TestSize.Level1)
{
    NETMGR_EXT_LOG_I("NetworkSliceProxyTest:SetSaState001");
    NetManagerExtAccessToken token;
    NetworkSliceProxy networkSliceProxy(nullptr);
    bool isSaState = false;
    auto ret = networkSliceProxy.SetSaState(isSaState);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL);
}

}
}
