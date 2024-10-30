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

#include "message_parcel.h"
#ifdef GTEST_API_
#define private public
#endif
#include "common_net_conn_callback_test.h"
#include "i_net_conn_callback.h"
#include "net_conn_client.h"
#include "net_conn_constants.h"
#include "net_conn_types.h"
#include "net_factoryreset_callback_stub.h"
#include "net_interface_callback_stub.h"
#include "net_interface_config.h"
#include "net_manager_constants.h"
#include "netmanager_base_test_security.h"
#include "network.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;

constexpr const char *TEST_IPV4_ADDR = "127.0.0.1";
constexpr const char *TEST_IPV6_ADDR = "240C:1:1:1::1";
constexpr const char *TEST_DOMAIN1 = ".com";
constexpr const char *TEST_DOMAIN2 = "test.com";
constexpr const char *TEST_DOMAIN3 = "testcom";
constexpr const char *TEST_DOMAIN4 = "com.test";
constexpr const char *TEST_DOMAIN5 = "test.co.uk";
constexpr const char *TEST_DOMAIN6 = "test.com.com";
constexpr const char *TEST_DOMAIN7 = "test1.test2.test3.test4.test5.com";
constexpr const char *TEST_DOMAIN8 = "http://www.example.com";
constexpr const char *TEST_DOMAIN9 = "https://www.example.com";
constexpr const char *TEST_DOMAIN10 = "httpd://www.example.com";
constexpr const char *TEST_LONG_HOST =
    "0123456789qwertyuiopasdfghjklzxcvbnm[]:;<>?!@#$%^&*()qwdqwrtfasfj4897qwe465791qwr87tq4fq7t8qt4654qwr";
constexpr const char *TEST_LONG_EXCLUSION_LIST =
    "www.test0.com,www.test1.com,www.test2.com,www.test3.com,www.test4.com,www.test5.com,www.test6.com,www.test7.com,"
    "www.test8.com,www.test9.com,www.test10.com,www.test11.com,www.test12.com,www.test12.com,www.test12.com,www.test13."
    "com,www.test14.com,www.test15.com,www.test16.com,www.test17.com,www.test18.com,www.test19.com,www.test20.com";
constexpr const char *TEST_IFACE = "eth0";
constexpr const char *PROXY_NAME = "123456789";
constexpr const int32_t PROXY_NAME_SIZE = 9;
constexpr const int32_t INVALID_VALUE = 100;
} // namespace

class NetConnClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetConnClientTest::SetUpTestCase() {}

void NetConnClientTest::TearDownTestCase() {}

void NetConnClientTest::SetUp() {}

void NetConnClientTest::TearDown() {}

class INetFactoryResetCallbackTest : public IRemoteStub<INetFactoryResetCallback> {
public:
    INetFactoryResetCallbackTest() = default;

    int32_t OnNetFactoryReset()
    {
        return 0;
    }
};

/**
 * @tc.name: GetDefaultNetTest001
 * @tc.desc: Test NetConnClient::GetDefaultNet, not applying for
 * permission,return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetDefaultNetTest001, TestSize.Level1)
{
    std::cout << "GetDefaultNetTest001 In" << std::endl;
    NetHandle handle;
    auto ret = NetConnClient::GetInstance().GetDefaultNet(handle);
    ASSERT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: GetDefaultNetTest002
 * @tc.desc: Test NetConnClient::GetDefaultNet, not applying for
 * permission,return NETMANAGER_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetDefaultNetTest002, TestSize.Level1)
{
    std::cout << "GetDefaultNetTest002 In" << std::endl;
    NetManagerBaseAccessToken token;
    NetHandle handle;
    int32_t netId = 0;
    auto ret = NetConnClient::GetInstance().GetDefaultNet(handle);
    netId = handle.GetNetId();
    if (netId == 0) {
        std::cout << "No network" << std::endl;
        ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
    } else if (netId >= 100 && netId <= MAX_NET_ID) {
        std::cout << "Get default network id:" << netId << std::endl;
        ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
    } else {
        ASSERT_FALSE(ret == NETMANAGER_SUCCESS);
    }
}

/**
 * @tc.name: HasDefaultNetTest001
 * @tc.desc: Test NetConnClient::HasDefaultNet,not applying for
 * permission, return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, HasDefaultNetTest001, TestSize.Level1)
{
    bool bFlag = false;
    auto ret = NetConnClient::GetInstance().HasDefaultNet(bFlag);
    ASSERT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: HasDefaultNetTest002
 * @tc.desc: Test NetConnClient::HasDefaultNet, applying for
 * permission, return NETMANAGER_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, HasDefaultNetTest002, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    bool bFlag = false;
    auto ret = NetConnClient::GetInstance().HasDefaultNet(bFlag);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetNetCapabilitiesTest001
 * @tc.desc: Test NetConnClient::GetNetCapabilities, In the absence of
 * permission, GetDefaultNet return NETMANAGER_ERR_PERMISSION_DENIED and
 * GetNetCapabilities return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetNetCapabilitiesTest001, TestSize.Level1)
{
    NetHandle handle;
    int32_t ret = NetConnClient::GetInstance().GetDefaultNet(handle);
    ASSERT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);

    NetAllCapabilities netAllCap;
    ret = NetConnClient::GetInstance().GetNetCapabilities(handle, netAllCap);
    ASSERT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: GetNetCapabilitiesTest002
 * @tc.desc: Test NetConnClient::GetNetCapabilities:In the absence of
 * permission, GetDefaultNet return NETMANAGER_ERR_PERMISSION_DENIED, and
 * after add permission GetNetCapabilities return NET_CONN_ERR_INVALID_NETWORK
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetNetCapabilitiesTest002, TestSize.Level1)
{
    NetHandle handle;
    int32_t ret = NetConnClient::GetInstance().GetDefaultNet(handle);
    ASSERT_TRUE(ret == NETMANAGER_ERR_PERMISSION_DENIED);

    NetManagerBaseAccessToken token;
    NetAllCapabilities netAllCap;
    ret = NetConnClient::GetInstance().GetNetCapabilities(handle, netAllCap);
    ASSERT_TRUE(ret == NET_CONN_ERR_INVALID_NETWORK);
}

/**
 * @tc.name: GetNetCapabilitiesTest003
 * @tc.desc: Test NetConnClient::GetNetCapabilities:Apply for permission at
 * first, when net is connected,return NET_CONN_SUCCESS, or net is not connected,return
 * NET_CONN_ERR_INVALID_NETWORK
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetNetCapabilitiesTest003, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    NetHandle handle;
    int32_t ret = NetConnClient::GetInstance().GetDefaultNet(handle);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);

    NetAllCapabilities netAllCap;
    ret = NetConnClient::GetInstance().GetNetCapabilities(handle, netAllCap);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS || ret == NET_CONN_ERR_INVALID_NETWORK);
}

/**
 * @tc.name: SetAirplaneModeTest001
 * @tc.desc: Test NetConnClient::SetAirplaneMode
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetAirplaneModeTest001, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    auto ret = NetConnClient::GetInstance().SetAirplaneMode(true);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetAirplaneModeTest002
 * @tc.desc: Test NetConnClient::SetAirplaneMode
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetAirplaneModeTest002, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    auto ret = NetConnClient::GetInstance().SetAirplaneMode(false);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: IsDefaultNetMeteredTest001
 * @tc.desc: if no permission,NetConnClient::IsDefaultNetMetered return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, IsDefaultNetMeteredTest001, TestSize.Level1)
{
    bool bRes = false;
    auto ret = NetConnClient::GetInstance().IsDefaultNetMetered(bRes);
    ASSERT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
    ASSERT_TRUE(bRes == false);
}

/**
 * @tc.name: IsDefaultNetMeteredTest002
 * @tc.desc: Test NetConnClient::IsDefaultNetMetered
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, IsDefaultNetMeteredTest002, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    bool bRes = false;
    auto ret = NetConnClient::GetInstance().IsDefaultNetMetered(bRes);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
    ASSERT_TRUE(bRes == true);
}

/**
 * @tc.name: SetGlobalHttpProxyTest001
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest001, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    HttpProxy httpProxy = {"testHttpProxy", 0, {}};
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest002
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest002, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN1, 8080, {}};
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest003
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest003, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN2, 8080, {}};
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest004
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest004, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN3, 8080, {}};
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest005
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest005, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN4, 8080, {}};
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest006
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest006, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN5, 8080, {}};
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest007
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest007, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN6, 8080, {}};
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest008
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest008, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN7, 8080, {}};
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest09
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest09, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN8, 8080, {}};
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest10
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest10, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN9, 8080, {}};
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest11
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest11, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN10, 8080, {}};
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest012
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest012, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    HttpProxy httpProxy = {TEST_IPV4_ADDR, 8080, {}};
    SecureData name;
    name.append(PROXY_NAME, PROXY_NAME_SIZE);
    SecureData pwd;
    pwd.append(PROXY_NAME, PROXY_NAME_SIZE);
    httpProxy.SetUserName(name);
    httpProxy.SetPassword(pwd);
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest013
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest013, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    HttpProxy httpProxy = {TEST_IPV6_ADDR, 8080, {}};
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest14
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest14, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    HttpProxy httpProxy = {TEST_LONG_HOST, 8080, {TEST_LONG_EXCLUSION_LIST}};
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest015
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest015, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    HttpProxy httpProxy;
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest016
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy.not applying for permission,return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest016, TestSize.Level1)
{
    HttpProxy httpProxy = {TEST_IPV4_ADDR, 8080, {}};
    SecureData name;
    SecureData pwd;
    name.append(PROXY_NAME, PROXY_NAME_SIZE);
    pwd.append(PROXY_NAME, PROXY_NAME_SIZE);
    httpProxy.SetUserName(name);
    httpProxy.SetPassword(pwd);
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: GetGlobalHttpProxyTest001
 * @tc.desc: Test NetConnClient::GetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetGlobalHttpProxyTest001, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    HttpProxy httpProxy = {TEST_IPV4_ADDR, 8080, {}};
    int32_t ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);

    HttpProxy getGlobalHttpProxy;
    ret = NetConnClient::GetInstance().GetGlobalHttpProxy(getGlobalHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
    ASSERT_TRUE(getGlobalHttpProxy.GetHost() == TEST_IPV4_ADDR);
}

/**
 * @tc.name: GetGlobalHttpProxyTest002
 * @tc.desc: Test NetConnClient::GetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetGlobalHttpProxyTest002, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    HttpProxy httpProxy = {TEST_IPV6_ADDR, 8080, {}};
    int32_t ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);

    HttpProxy getGlobalHttpProxy;
    ret = NetConnClient::GetInstance().GetGlobalHttpProxy(getGlobalHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
    ASSERT_TRUE(getGlobalHttpProxy.GetHost() == TEST_IPV6_ADDR);
}

/**
 * @tc.name: GetGlobalHttpProxyTest003
 * @tc.desc: Test NetConnClient::GetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetGlobalHttpProxyTest003, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN2, 8080, {}};
    int32_t ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);

    HttpProxy getGlobalHttpProxy;
    ret = NetConnClient::GetInstance().GetGlobalHttpProxy(getGlobalHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
    ASSERT_TRUE(getGlobalHttpProxy.GetHost() == TEST_DOMAIN2);
}

/**
 * @tc.name: GetGlobalHttpProxyTest004
 * @tc.desc: Test NetConnClient::GetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetGlobalHttpProxyTest004, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    HttpProxy validHttpProxy = {TEST_IPV4_ADDR, 8080, {}};
    int32_t ret = NetConnClient::GetInstance().SetGlobalHttpProxy(validHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);

    HttpProxy getGlobalHttpProxy;
    ret = NetConnClient::GetInstance().GetGlobalHttpProxy(getGlobalHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
    ASSERT_TRUE(getGlobalHttpProxy.GetHost() == TEST_IPV4_ADDR);
}

/**
 * @tc.name: GetGlobalHttpProxyTest005
 * @tc.desc: Test NetConnClient::GetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetGlobalHttpProxyTest005, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    HttpProxy httpProxy;
    int32_t ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);

    HttpProxy getGlobalHttpProxy;
    ret = NetConnClient::GetInstance().GetGlobalHttpProxy(getGlobalHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
    ASSERT_TRUE(getGlobalHttpProxy.GetHost().empty());
}

/**
 * @tc.name: GetDefaultHttpProxyTest001
 * @tc.desc: Test NetConnClient::GetDefaultHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetDefaultHttpProxyTest001, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    HttpProxy validHttpProxy = {TEST_IPV4_ADDR, 8080, {}};
    int32_t ret = NetConnClient::GetInstance().SetGlobalHttpProxy(validHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);

    HttpProxy defaultHttpProxy;
    ret = NetConnClient::GetInstance().GetDefaultHttpProxy(defaultHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
    ASSERT_TRUE(defaultHttpProxy.GetHost() == TEST_IPV4_ADDR);
}

/**
 * @tc.name: GetDefaultHttpProxyTest002
 * @tc.desc: Test NetConnClient::GetDefaultHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetDefaultHttpProxyTest002, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    HttpProxy globalHttpProxy;
    int32_t ret = NetConnClient::GetInstance().SetGlobalHttpProxy(globalHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);

    HttpProxy defaultHttpProxy;
    ret = NetConnClient::GetInstance().GetDefaultHttpProxy(defaultHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: GetDefaultHttpProxyTest003
 * @tc.desc: Test NetConnClient::SetAppNet and NetConnClient::GetDefaultHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetDefaultHttpProxyTest003, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    int32_t netId = 102;
    int32_t ret = NetConnClient::GetInstance().SetAppNet(netId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    HttpProxy defaultHttpProxy;
    ret = NetConnClient::GetInstance().GetDefaultHttpProxy(defaultHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);

    int32_t cancelNetId = 0;
    ret = NetConnClient::GetInstance().SetAppNet(cancelNetId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    ret = NetConnClient::GetInstance().GetDefaultHttpProxy(defaultHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: RegisterNetSupplier001
 * @tc.desc: Test NetConnClient::RegisterNetSupplier
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetSupplier001, TestSize.Level1)
{
    uint32_t supplierId = 100;
    NetBearType netBearType = BEARER_WIFI;
    const std::string ident = "";
    std::set<NetCap> netCaps = {NET_CAPABILITY_INTERNET};
    auto ret = NetConnClient::GetInstance().RegisterNetSupplier(netBearType, ident, netCaps, supplierId);
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: RegisterNetSupplier002
 * @tc.desc: Test NetConnClient::RegisterNetSupplier
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetSupplier002, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    uint32_t supplierId = 100;
    NetBearType netBearType = BEARER_WIFI;
    const std::string ident = "";
    std::set<NetCap> netCaps = {NET_CAPABILITY_INTERNET};
    auto ret = NetConnClient::GetInstance().RegisterNetSupplier(netBearType, ident, netCaps, supplierId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: UnregisterNetSupplier001
 * @tc.desc: Test NetConnClient::UnregisterNetSupplier
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, UnregisterNetSupplier001, TestSize.Level1)
{
    uint32_t supplierId = 100;
    auto ret = NetConnClient::GetInstance().UnregisterNetSupplier(supplierId);
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: UnregisterNetSupplier002
 * @tc.desc: Test NetConnClient::UnregisterNetSupplier
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, UnregisterNetSupplier002, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    uint32_t supplierId = 100;
    auto ret = NetConnClient::GetInstance().UnregisterNetSupplier(supplierId);
    EXPECT_EQ(ret, NET_CONN_ERR_NO_SUPPLIER);
}

/**
 * @tc.name: RegisterNetSupplierCallbackTest001
 * @tc.desc: Test NetConnClient::RegisterNetSupplierCallback
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetSupplierCallbackTest001, TestSize.Level1)
{
    uint32_t supplierId = 100;
    sptr<NetSupplierCallbackBase> callback = new (std::nothrow) NetSupplierCallbackBase();
    ASSERT_NE(callback, nullptr);
    auto ret = NetConnClient::GetInstance().RegisterNetSupplierCallback(supplierId, callback);
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: RegisterNetSupplierCallbackTest002
 * @tc.desc: Test NetConnClient::RegisterNetSupplierCallback
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetSupplierCallbackTest002, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    uint32_t supplierId = 100;
    sptr<NetSupplierCallbackBase> callback = new (std::nothrow) NetSupplierCallbackBase();
    ASSERT_NE(callback, nullptr);
    auto ret = NetConnClient::GetInstance().RegisterNetSupplierCallback(supplierId, callback);
    EXPECT_EQ(ret, NET_CONN_ERR_NO_SUPPLIER);
}

/**
 * @tc.name: RegisterNetSupplierCallbackTest003
 * @tc.desc: Test NetConnClient::RegisterNetSupplierCallback
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetSupplierCallbackTest003, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    NetBearType bearerType = BEARER_CELLULAR;
    std::set<NetCap> netCaps{NET_CAPABILITY_INTERNET};
    std::string ident = "ident";
    uint32_t supplierId = 0;
    int32_t result = NetConnClient::GetInstance().RegisterNetSupplier(bearerType, ident, netCaps, supplierId);
    ASSERT_TRUE(result == NETMANAGER_SUCCESS);
    sptr<NetSupplierCallbackBase> callback = new (std::nothrow) NetSupplierCallbackBase();
    ASSERT_NE(callback, nullptr);
    auto ret = NetConnClient::GetInstance().RegisterNetSupplierCallback(supplierId, callback);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: RegisterNetSupplierCallbackTest004
 * @tc.desc: Test NetConnClient::RegisterNetSupplierCallback
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetSupplierCallbackTest004, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    uint32_t supplierId = 0;
    sptr<NetSupplierCallbackBase> callback;
    auto ret = NetConnClient::GetInstance().RegisterNetSupplierCallback(supplierId, callback);
    EXPECT_EQ(ret, NET_CONN_ERR_NO_SUPPLIER);
}

/**
 * @tc.name: SetAppNetTest001
 * @tc.desc: Test NetConnClient::SetAppNet, if param is invalid, SetAppNet return NET_CONN_ERR_INVALID_NETWORK
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetAppNetTest001, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    int32_t netId = 99;
    auto ret = NetConnClient::GetInstance().SetAppNet(netId);
    EXPECT_EQ(ret, NET_CONN_ERR_INVALID_NETWORK);
}

/**
 * @tc.name: SetAppNetTest002
 * @tc.desc: Test NetConnClient::SetAppNet, if param is valid, SetAppNet return NETMANAGER_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetAppNetTest002, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    int32_t netId = 102;
    auto ret = NetConnClient::GetInstance().SetAppNet(netId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    int32_t cancelNetId = 0;
    ret = NetConnClient::GetInstance().SetAppNet(cancelNetId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetAppNetTest001
 * @tc.desc: Test NetConnClient::GetAppNet, return NetId set by SetAppNet
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetAppNetTest001, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    int32_t netId = 102;
    auto ret = NetConnClient::GetInstance().SetAppNet(netId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    int32_t getNetId = 0;
    NetConnClient::GetInstance().GetAppNet(getNetId);
    EXPECT_EQ(getNetId, netId);

    int32_t cancelNetId = 0;
    ret = NetConnClient::GetInstance().SetAppNet(cancelNetId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: RegisterNetConnCallback001
 * @tc.desc: Test NetConnClient::RegisterNetConnCallback, not applying for
 * permission,return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetConnCallback001, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    sptr<INetConnCallbackTest> callback = new (std::nothrow) INetConnCallbackTest();
    int32_t ret = NetConnClient::GetInstance().RegisterNetConnCallback(callback);
    ret = NetConnClient::GetInstance().UnregisterNetConnCallback(callback);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: RegisterNetConnCallback002
 * @tc.desc: Test NetConnClient::RegisterNetConnCallback, not applying for
 * permission,return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetConnCallback002, TestSize.Level1)
{
    sptr<NetSpecifier> netSpecifier = nullptr;
    sptr<INetConnCallbackTest> callback = new (std::nothrow) INetConnCallbackTest();
    uint32_t timesOut = 1;
    auto ret = NetConnClient::GetInstance().RegisterNetConnCallback(netSpecifier, callback, timesOut);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

/**
 * @tc.name: RegisterNetConnCallback002
 * @tc.desc: Test NetConnClient::RegisterNetConnCallback, not applying for
 * permission,return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetConnCallback003, TestSize.Level1)
{
    sptr<NetSpecifier> netSpecifier = new (std::nothrow) NetSpecifier();
    sptr<INetConnCallbackTest> callback = new (std::nothrow) INetConnCallbackTest();
    uint32_t timesOut = 1;
    auto ret = NetConnClient::GetInstance().RegisterNetConnCallback(netSpecifier, callback, timesOut);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

/**
 * @tc.name: RegisterNetConnCallback001
 * @tc.desc: Test NetConnClient::RegisterNetConnCallback, not applying for
 * permission,return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, UnRegisterNetConnCallback001, TestSize.Level1)
{
    sptr<INetConnCallbackTest> callback = new (std::nothrow) INetConnCallbackTest();
    int32_t ret = NetConnClient::GetInstance().RegisterNetConnCallback(callback);
    ret = NetConnClient::GetInstance().UnregisterNetConnCallback(callback);
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: RequestNetConnection001
 * @tc.desc: Test NetConnClient::RequestNetConnection, not applying for
 * permission,return NETMANAGER_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RequestNetConnection001, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    sptr<NetSpecifier> netSpecifier = new (std::nothrow) NetSpecifier();
    netSpecifier->netCapabilities_.bearerTypes_.emplace(NetManagerStandard::BEARER_CELLULAR);
    netSpecifier->netCapabilities_.netCaps_.emplace(NetManagerStandard::NET_CAPABILITY_INTERNAL_DEFAULT);
    sptr<INetConnCallbackTest> callback = new (std::nothrow) INetConnCallbackTest();
    uint32_t timesOut = 0;
    auto ret = NetConnClient::GetInstance().RequestNetConnection(netSpecifier, callback, timesOut);
    ret = NetConnClient::GetInstance().UnregisterNetConnCallback(callback);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: RequestNetConnection002
 * @tc.desc: Test NetConnClient::RequestNetConnection, not applying for
 * permission,return NETMANAGER_ERR_PARAMETER_ERROR
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RequestNetConnection002, TestSize.Level1)
{
    sptr<NetSpecifier> netSpecifier = nullptr;
    sptr<INetConnCallbackTest> callback = new (std::nothrow) INetConnCallbackTest();
    uint32_t timesOut = 1;
    auto ret = NetConnClient::GetInstance().RequestNetConnection(netSpecifier, callback, timesOut);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

/**
 * @tc.name: RequestNetConnection003
 * @tc.desc: Test NetConnClient::RequestNetConnection, not applying for
 * permission,return NETMANAGER_ERR_PARAMETER_ERROR
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RequestNetConnection003, TestSize.Level1)
{
    sptr<NetSpecifier> netSpecifier = new (std::nothrow) NetSpecifier();
    sptr<INetConnCallbackTest> callback = new (std::nothrow) INetConnCallbackTest();
    uint32_t timesOut = 1;
    auto ret = NetConnClient::GetInstance().RequestNetConnection(netSpecifier, callback, timesOut);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

/**
 * @tc.name: RequestNetConnection004
 * @tc.desc: Test NetConnClient::RequestNetConnection, not applying for
 * permission,return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RequestNetConnection004, TestSize.Level1)
{
    sptr<NetSpecifier> netSpecifier = new (std::nothrow) NetSpecifier();
    netSpecifier->netCapabilities_.bearerTypes_.emplace(NetManagerStandard::BEARER_CELLULAR);
    netSpecifier->netCapabilities_.netCaps_.emplace(NetManagerStandard::NET_CAPABILITY_INTERNAL_DEFAULT);
    sptr<INetConnCallbackTest> callback = new (std::nothrow) INetConnCallbackTest();
    uint32_t timesOut = 0;
    auto ret = NetConnClient::GetInstance().RequestNetConnection(netSpecifier, callback, timesOut);
    ret = NetConnClient::GetInstance().UnregisterNetConnCallback(callback);
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: UpdateNetSupplierInfo001
 * @tc.desc: Test NetConnClient::UpdateNetSupplierInfo, not applying for
 * permission,return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, UpdateNetSupplierInfo001, TestSize.Level1)
{
    auto &client = NetConnClient::GetInstance();
    uint32_t supplierId = 1;
    sptr<NetSupplierInfo> netSupplierInfo = new (std::nothrow) NetSupplierInfo;
    int32_t ret = client.UpdateNetSupplierInfo(supplierId, netSupplierInfo);
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: UpdateNetSupplierInfo002
 * @tc.desc: Test NetConnClient::UpdateNetSupplierInfo, not applying for
 * permission,return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, UpdateNetSupplierInfo002, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    auto &client = NetConnClient::GetInstance();
    uint32_t supplierId = 1;
    sptr<NetSupplierInfo> netSupplierInfo = new NetSupplierInfo;
    netSupplierInfo->isAvailable_ = true;
    netSupplierInfo->isRoaming_ = true;
    netSupplierInfo->strength_ = 0x64;
    netSupplierInfo->frequency_ = 0x10;
    int32_t ret = client.UpdateNetSupplierInfo(supplierId, netSupplierInfo);
    EXPECT_EQ(ret, NET_CONN_ERR_NO_SUPPLIER);
}

/**
 * @tc.name: UpdateNetSupplierInfo003
 * @tc.desc: Test NetConnClient::UpdateNetSupplierInfo,return NETMANAGER_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, UpdateNetSupplierInfo003, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    NetBearType netBearType = BEARER_CELLULAR;
    std::set<NetCap> netCaps{NET_CAPABILITY_INTERNET};
    std::string ident = "ident";
    uint32_t supplierId = 0;
    auto &client = NetConnClient::GetInstance();
    client.RegisterNetSupplier(netBearType, ident, netCaps, supplierId);

    sptr<NetSupplierInfo> netSupplierInfo = new NetSupplierInfo;
    netSupplierInfo->isAvailable_ = true;
    netSupplierInfo->isRoaming_ = true;
    netSupplierInfo->strength_ = 0x64;
    netSupplierInfo->frequency_ = 0x10;
    netSupplierInfo->score_ = 55;
    int32_t ret = client.UpdateNetSupplierInfo(supplierId, netSupplierInfo);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetNetInterfaceConfigurationTest001
 * @tc.desc: Test NetConnClient::GetNetInterfaceConfiguration
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetNetInterfaceConfigurationTest001, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    NetInterfaceConfiguration config;
    auto ret = NetConnClient::GetInstance().GetNetInterfaceConfiguration(TEST_IFACE, config);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetNetInterfaceConfigurationTest001
 * @tc.desc: Test NetConnClient::GetNetInterfaceConfiguration
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetNetInterfaceConfigurationTest002, TestSize.Level1)
{
    NetInterfaceConfiguration config;
    auto ret = NetConnClient::GetInstance().GetNetInterfaceConfiguration(TEST_IFACE, config);
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: RegisterNetInterfaceCallbackTest001
 * @tc.desc: Test NetConnClient::RegisterNetInterfaceCallback
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetInterfaceCallbackTest001, TestSize.Level1)
{
    sptr<INetInterfaceStateCallback> callback = new (std::nothrow) NetInterfaceStateCallbackStub();
    int32_t ret = NetConnClient::GetInstance().RegisterNetInterfaceCallback(callback);
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: RegisterNetInterfaceCallbackTest002
 * @tc.desc: Test NetConnClient::RegisterNetInterfaceCallback
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetInterfaceCallbackTest002, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    sptr<INetInterfaceStateCallback> callback = new (std::nothrow) NetInterfaceStateCallbackStub();
    int32_t ret = NetConnClient::GetInstance().RegisterNetInterfaceCallback(callback);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SystemReadyTest002
 * @tc.desc: Test NetConnClient::SystemReady
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SystemReadyTest002, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->SystemReady();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: UpdateNetLinkInfoTest002
 * @tc.desc: Test NetConnClient::UpdateNetLinkInfo
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, UpdateNetLinkInfoTest002, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    uint32_t supplierId = 1;
    sptr<NetLinkInfo> netLinkInfo = std::make_unique<NetLinkInfo>().release();
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->UpdateNetLinkInfo(supplierId, netLinkInfo);
    EXPECT_EQ(ret, NET_CONN_ERR_NO_SUPPLIER);
}

/**
 * @tc.name: GetAllNetsTest002
 * @tc.desc: Test NetConnClient::GetAllNets
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetAllNetsTest002, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    std::list<sptr<NetHandle>> netList;
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->GetAllNets(netList);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetConnectionPropertiesTest002
 * @tc.desc: Test NetConnClient::GetConnectionProperties
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetConnectionPropertiesTest002, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    NetHandle netHandle;
    NetLinkInfo info;
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->GetConnectionProperties(netHandle, info);
    EXPECT_EQ(ret, NET_CONN_ERR_INVALID_NETWORK);
}

/**
 * @tc.name: GetAddressesByNameTest002
 * @tc.desc: Test NetConnClient::GetAddressesByName
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetAddressesByNameTest002, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    const std::string host = "ipaddr";
    int32_t netId = 1;
    std::vector<INetAddr> addrList = {};
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->GetAddressesByName(host, netId, addrList);
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name: GetAddressByNameTest002
 * @tc.desc: Test NetConnClient::GetAddressByName
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetAddressByNameTest002, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    std::string host = "ipaddr";
    int32_t netId = 1;
    INetAddr addr;
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->GetAddressByName(host, netId, addr);
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name: GetIfaceNameIdentMapsTest001
 * @tc.desc: Test NetConnClient::GetIfaceNameIdentMaps
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetIfaceNameIdentMapsTest001, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    SafeMap<std::string, std::string> data;
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->GetIfaceNameIdentMaps(NetBearType::BEARER_CELLULAR,
                                                                                        data);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: BindSocketTest002
 * @tc.desc: Test NetConnClient::BindSocket
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, BindSocketTest002, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    NetConnClient::NetConnDeathRecipient deathRecipient(*DelayedSingleton<NetConnClient>::GetInstance());
    sptr<IRemoteObject> remote = nullptr;
    deathRecipient.OnRemoteDied(remote);
    int32_t socketFd = 0;
    int32_t netId = 99;
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->BindSocket(socketFd, netId);
    EXPECT_EQ(ret, NET_CONN_ERR_INVALID_NETWORK);
    netId = 101;
    ret = DelayedSingleton<NetConnClient>::GetInstance()->BindSocket(socketFd, netId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetDetectionTest002
 * @tc.desc: Test NetConnClient::NetDetection
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, NetDetectionTest002, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    NetHandle netHandle;
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->NetDetection(netHandle);
    EXPECT_EQ(ret, NET_CONN_ERR_NETID_NOT_FOUND);
}

HWTEST_F(NetConnClientTest, NetworkRouteTest001, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    int32_t netId = 10;
    std::string ifName = "wlan0";
    std::string destination = "0.0.0.0/0";
    std::string nextHop = "0.0.0.1234";

    int32_t ret = NetConnClient::GetInstance().AddNetworkRoute(netId, ifName, destination, nextHop);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
    ret = NetConnClient::GetInstance().RemoveNetworkRoute(netId, ifName, destination, nextHop);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnClientTest, InterfaceAddressTest001, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    std::string ifName = "wlan0";
    std::string ipAddr = "0.0.0.1";
    int32_t prefixLength = 23;

    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->AddInterfaceAddress(ifName, ipAddr, prefixLength);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    ret = DelayedSingleton<NetConnClient>::GetInstance()->DelInterfaceAddress(ifName, ipAddr, prefixLength);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnClientTest, StaticArpTest001, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    std::string ifName = "wlan0";
    std::string ipAddr = "123.12.12.123";
    std::string macAddr = "12:23:34:12:12:11";
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->AddStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ipAddr = "1234";
    macAddr = "12:23:34:12:12:11";
    ret = DelayedSingleton<NetConnClient>::GetInstance()->AddStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    ipAddr = "123.12.12.123";
    macAddr = "12:234:34";
    ret = DelayedSingleton<NetConnClient>::GetInstance()->AddStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetConnClientTest, StaticArpTest002, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    std::string ipAddr = "123.12.12.123";
    std::string macAddr = "12:23:34:12:12:11";
    std::string ifName = "wlan0";
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->DelStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ipAddr = "123.12.12.123";
    macAddr = "12:23:34:12:12:11";
    ret = DelayedSingleton<NetConnClient>::GetInstance()->DelStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, NETMANAGER_ERR_OPERATION_FAILED);

    ipAddr = "123.12.12.1235678";
    macAddr = "12:23:34:12:12:11";
    ret = DelayedSingleton<NetConnClient>::GetInstance()->DelStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    ipAddr = "123.12.12.123";
    macAddr = "12:23:34:12:12";
    ret = DelayedSingleton<NetConnClient>::GetInstance()->DelStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    ipAddr = "123.12.12.123";
    macAddr = "12:23:34:12:12:11";
    ifName = "";
    ret = DelayedSingleton<NetConnClient>::GetInstance()->DelStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, NETMANAGER_ERR_OPERATION_FAILED);
}

HWTEST_F(NetConnClientTest, NetConnClientBranchTest001, TestSize.Level1)
{
    int32_t uid = 0;
    uint8_t allow = 0;
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->SetInternetPermission(uid, allow);
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);

    uint32_t supplierId = 0;
    sptr<NetSupplierInfo> netSupplierInfo = nullptr;
    ret = DelayedSingleton<NetConnClient>::GetInstance()->UpdateNetSupplierInfo(supplierId, netSupplierInfo);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);

    sptr<NetLinkInfo> netLinkInfo = nullptr;
    ret = DelayedSingleton<NetConnClient>::GetInstance()->UpdateNetLinkInfo(supplierId, netLinkInfo);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);
}

HWTEST_F(NetConnClientTest, SetInternetPermissionTest001, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    int32_t uid = 0;
    uint8_t allow = 0;
    NetConnClient::GetInstance().netPermissionMap_.EnsureInsert(0, 0);
    auto ret = NetConnClient::GetInstance().SetInternetPermission(uid, allow);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    EXPECT_EQ(NetConnClient::GetInstance().netPermissionMap_.Size(), 1);

    NetManagerBaseAccessToken access;
    ret = NetConnClient::GetInstance().SetInternetPermission(uid, allow);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    EXPECT_EQ(NetConnClient::GetInstance().netPermissionMap_.Size(), 1);
}

HWTEST_F(NetConnClientTest, RegisterSlotTypeTest001, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    uint32_t supplierId = 100;
    int32_t tech = 2;
    int32_t ret = NetConnClient::GetInstance().RegisterSlotType(supplierId, tech);
    EXPECT_EQ(ret, NETMANAGER_ERR_INVALID_PARAMETER);

    std::string type = "";
    ret = NetConnClient::GetInstance().GetSlotType(type);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);
    EXPECT_TRUE(type.empty());
}

HWTEST_F(NetConnClientTest, RegisterSlotTypeTest002, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    NetBearType netBearType = BEARER_CELLULAR;
    std::set<NetCap> netCaps{NET_CAPABILITY_INTERNET};
    std::string ident = "ident";
    uint32_t supplierId = 0;
    auto ret = NetConnClient::GetInstance().RegisterNetSupplier(netBearType, ident, netCaps, supplierId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    int32_t tech = 2;
    ret = NetConnClient::GetInstance().RegisterSlotType(supplierId, tech);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnClientTest, GetPinSetForHostName001, TestSize.Level1)
{
    std::string hostname("www.example.com");
    std::string pins;
    auto ret = NetConnClient::GetInstance().GetPinSetForHostName(hostname, pins);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnClientTest, GetTrustAnchorsForHostName001, TestSize.Level1)
{
    std::string hostname("www.example.com");
    std::vector<std::string> certs;
    auto ret = NetConnClient::GetInstance().GetTrustAnchorsForHostName(hostname, certs);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: FactoryResetNetworkTest001
 * @tc.desc: Test NetConnClient::FactoryResetNetwork
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, FactoryResetNetworkTest001, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    int32_t ret = NetConnClient::GetInstance().FactoryResetNetwork();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: RegisterNetFactoryResetCallbackTest001
 * @tc.desc: Test NetConnClient::RegisterNetFactoryResetCallback
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetFactoryResetCallbackTest001, TestSize.Level1)
{
    sptr<INetFactoryResetCallbackTest> callback = new (std::nothrow) INetFactoryResetCallbackTest();
    int32_t ret = NetConnClient::GetInstance().RegisterNetFactoryResetCallback(callback);
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: RegisterNetFactoryResetCallbackTest002
 * @tc.desc: Test NetConnClient::RegisterNetFactoryResetCallback
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetFactoryResetCallbackTest002, TestSize.Level1)
{
    NetManagerBaseAccessToken token;

    sptr<INetFactoryResetCallback> callback = nullptr;
    int32_t ret = NetConnClient::GetInstance().RegisterNetFactoryResetCallback(callback);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);

    callback = new (std::nothrow) INetFactoryResetCallbackTest();
    ASSERT_NE(callback, nullptr);
    ret = NetConnClient::GetInstance().RegisterNetFactoryResetCallback(callback);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnClientTest, RegisterAppHttpProxyCallback001, TestSize.Level1)
{
    std::function<void(const HttpProxy &httpProxy)> callback;
    uint32_t callbackid = 0;
    NetConnClient::GetInstance().RegisterAppHttpProxyCallback(callback, callbackid);
    EXPECT_EQ(callbackid, 0);

    NetConnClient::GetInstance().UnregisterAppHttpProxyCallback(callbackid);

    NetConnClient::GetInstance().RegisterAppHttpProxyCallback(callback, callbackid);
    EXPECT_EQ(callbackid, 1);
}

HWTEST_F(NetConnClientTest, SetAppHttpProxy001, TestSize.Level1)
{
    HttpProxy httpProxy;
    auto ret = NetConnClient::GetInstance().SetAppHttpProxy(httpProxy);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnClientTest, IsPreferCellularUrl, TestSize.Level1)
{
    std::string url = "www.testPreferCellularUrl.com";
    bool preferCellular = false;
    NetConnClient::GetInstance().IsPreferCellularUrl(url, preferCellular);
    EXPECT_FALSE(preferCellular);
}

HWTEST_F(NetConnClientTest, RegisterPreAirplaneCallback, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    sptr<PreAirplaneCallbackTest> callback = new (std::nothrow) PreAirplaneCallbackTest();
    int32_t ret = NetConnClient::GetInstance().RegisterPreAirplaneCallback(callback);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnClientTest, RegisterPreAirplaneCallback2, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    sptr<PreAirplaneCallbackTest> callback = nullptr;
    int32_t ret = NetConnClient::GetInstance().RegisterPreAirplaneCallback(callback);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);
}

HWTEST_F(NetConnClientTest, UnregisterPreAirplaneCallback, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    sptr<PreAirplaneCallbackTest> callback = new (std::nothrow) PreAirplaneCallbackTest();
    int32_t ret = NetConnClient::GetInstance().UnregisterPreAirplaneCallback(callback);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnClientTest, UpdateSupplierScore001, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    uint32_t supplierId = 100;
    int32_t ret = NetConnClient::GetInstance().UpdateSupplierScore(NetBearType::BEARER_WIFI,
        QUALITY_POOR_STATE, supplierId);
    EXPECT_NE(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetConnClientTest, UpdateSupplierScore002, TestSize.Level1)
{
    uint32_t supplierId = 100;
    int32_t ret = NetConnClient::GetInstance().UpdateSupplierScore(NetBearType::BEARER_WIFI,
        QUALITY_POOR_STATE, supplierId);
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetConnClientTest, GetIfaceNameIdentMaps001, TestSize.Level1)
{
    uint32_t invalidValue = INVALID_VALUE;
    NetBearType bearerType = static_cast<NetBearType>(invalidValue);
    SafeMap<std::string, std::string> ifaceNameIdentMaps;
    int32_t ret = NetConnClient::GetInstance().GetIfaceNameIdentMaps(bearerType, ifaceNameIdentMaps);
    EXPECT_EQ(ret, NETMANAGER_ERR_INTERNAL);

    bearerType = NetBearType::BEARER_BLUETOOTH;
    ret = NetConnClient::GetInstance().GetIfaceNameIdentMaps(bearerType, ifaceNameIdentMaps);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnClientTest, IsAPIVersionSupported001, TestSize.Level1)
{
    int targetApiVersion = 1;
    EXPECT_TRUE(NetConnClient::IsAPIVersionSupported(targetApiVersion));
}

HWTEST_F(NetConnClientTest, ObtainBundleNameForSelf001, TestSize.Level1)
{
    auto result = NetConnClient::ObtainBundleNameForSelf();
    EXPECT_EQ(result, std::nullopt);
}

HWTEST_F(NetConnClientTest, CloseSocketsUid001, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    int32_t netId = 100;
    uint32_t uid = 20020157;
    int32_t ret = NetConnClient::GetInstance().CloseSocketsUid(netId, uid);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnClientTest, CloseSocketsUid002, TestSize.Level1)
{
    int32_t netId = 100;
    uint32_t uid = 20020157;
    int32_t ret = NetConnClient::GetInstance().CloseSocketsUid(netId, uid);
    NetConnClient::GetInstance().DlCloseRemoveDeathRecipient();
    EXPECT_NE(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}
} // namespace NetManagerStandard
} // namespace OHOS
