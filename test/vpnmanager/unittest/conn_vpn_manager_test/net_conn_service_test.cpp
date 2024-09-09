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

#include <gtest/gtest.h>

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "common_net_conn_callback_test.h"
#include "http_proxy.h"
#include "net_all_capabilities.h"
#include "net_conn_callback_stub.h"
#include "net_conn_client.h"
#include "net_conn_constants.h"
#include "net_conn_service.h"
#include "net_conn_types.h"
#include "net_detection_callback_test.h"
#include "net_factoryreset_callback_stub.h"
#include "net_http_proxy_tracker.h"
#include "net_interface_callback_stub.h"
#include "net_manager_center.h"
#include "net_mgr_log_wrapper.h"
#include "netmanager_base_test_security.h"
#include "netsys_controller.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr uint32_t TEST_TIMEOUTMS = 1000;
constexpr int32_t TEST_NETID = 3;
constexpr int32_t TEST_SOCKETFD = 2;
const int32_t NET_ID = 2;
const int32_t SOCKET_FD = 2;
const int32_t ZERO_VALUE = 0;
const int32_t INVALID_VALUE = 10;
constexpr const char *TEST_IDENT = "testIdent";
constexpr const char *TEST_HOST = "testHost";
constexpr const char *TEST_PROXY_HOST = "testHttpProxy";
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
    "0123456789qwertyuiopasdfghjklzxcvbnm[]:;<>?!@#$%^&()AEFFEqwdqwrtfasfj4897qwe465791qwr87tq4fq7t8qt4654qwr";
constexpr const char *TEST_LONG_EXCLUSION_LIST =
    "www.test0.com,www.test1.com,www.test2.com,www.test3.com,www.test4.com,www.test5.com,www.test6.com,www.test7.com,"
    "www.test8.com,www.test9.com,www.test10.com,www.test11.com,www.test12.com,www.test12.com,www.test12.com,www.test13."
    "com,www.test14.com,www.test15.com,www.test16.com,www.test17.com,www.test18.com,www.test19.com,www.test20.com";
constexpr const char *NET_CONN_MANAGER_WORK_THREAD = "NET_CONN_MANAGER_WORK_THREAD";
constexpr int64_t TEST_UID = 1010;
constexpr uint32_t TEST_NOTEXISTSUPPLIER = 1000;

class TestDnsService : public DnsBaseService {
public:
    int32_t GetAddressesByName(const std::string &hostName, int32_t netId,
                               std::vector<INetAddr> &addrInfo) override
    {
        if (netId == TEST_NOTEXISTSUPPLIER) {
            return NETMANAGER_ERROR;
        } else if (netId == TEST_NETID) {
            INetAddr netAddr;
            netAddr.type_ = INetAddr::IPV4;
            addrInfo.push_back(netAddr);
        }
        return NETSYS_SUCCESS;
    }
};

sptr<INetConnCallback> g_callback = new (std::nothrow) NetConnCallbackStubCb();
sptr<INetDetectionCallback> g_detectionCallback = new (std::nothrow) NetDetectionCallbackTest();
uint32_t g_supplierId = 0;
uint32_t g_vpnSupplierId = 0;
} // namespace

class NetConnServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetConnServiceTest::SetUpTestCase()
{
    NetConnService::GetInstance()->OnStart();
    if (NetConnService::GetInstance()->state_ != NetConnService::STATE_RUNNING) {
        NetConnService::GetInstance()->netConnEventRunner_ =
            AppExecFwk::EventRunner::Create(NET_CONN_MANAGER_WORK_THREAD);
        ASSERT_NE(NetConnService::GetInstance()->netConnEventRunner_, nullptr);
        NetConnService::GetInstance()->netConnEventHandler_ =
            std::make_shared<NetConnEventHandler>(NetConnService::GetInstance()->netConnEventRunner_);
        NetConnService::GetInstance()->serviceIface_ = std::make_unique<NetConnServiceIface>().release();
        NetManagerCenter::GetInstance().RegisterConnService(NetConnService::GetInstance()->serviceIface_);
        NetHttpProxyTracker httpProxyTracker;
        HttpProxy httpProxy;
        httpProxy.SetPort(0);
        httpProxyTracker.ReadFromSettingsData(httpProxy);
        NetConnService::GetInstance()->SendHttpProxyChangeBroadcast(httpProxy);
    }
}

void NetConnServiceTest::TearDownTestCase() {}

void NetConnServiceTest::SetUp() {}

void NetConnServiceTest::TearDown() {}

HWTEST_F(NetConnServiceTest, OnRemoveSystemAbility001, TestSize.Level1)
{
    std::string deviceId = "dev1";
    NetConnService::GetInstance()->OnRemoveSystemAbility(COMM_NETSYS_NATIVE_SYS_ABILITY_ID, deviceId);
    EXPECT_TRUE(NetConnService::GetInstance()->hasSARemoved_);
}

HWTEST_F(NetConnServiceTest, OnAddSystemAbility001, TestSize.Level1)
{
    std::string deviceId = "dev1";
    NetConnService::GetInstance()->OnAddSystemAbility(COMM_NETSYS_NATIVE_SYS_ABILITY_ID, deviceId);
    EXPECT_FALSE(NetConnService::GetInstance()->hasSARemoved_);
}

HWTEST_F(NetConnServiceTest, IsSupplierMatchRequestAndNetworkTest001, TestSize.Level1)
{
    sptr<NetSupplier> supplier = nullptr;
    bool ret = NetConnService::GetInstance()->IsSupplierMatchRequestAndNetwork(supplier);
    EXPECT_FALSE(ret);

    NetConnService::GetInstance()->CreateDefaultRequest();
    ret = NetConnService::GetInstance()->IsSupplierMatchRequestAndNetwork(supplier);
    EXPECT_FALSE(ret);
}

HWTEST_F(NetConnServiceTest, SystemReadyTest001, TestSize.Level1)
{
    if (!NetConnService::GetInstance()->registerToService_) {
        NetConnService::GetInstance()->state_ = NetConnService::STATE_RUNNING;
    }

    NetConnService::GetInstance()->OnStart();
    int32_t ret = NetConnService::GetInstance()->SystemReady();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, SetInternetPermissionTest001, TestSize.Level1)
{
    uint8_t allow = 1;
    int32_t ret = NetConnService::GetInstance()->SetInternetPermission(TEST_UID, allow);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, GetDefaultNetTest000, TestSize.Level1)
{
    int32_t netId = 0;
    auto ret = NetConnService::GetInstance()->GetDefaultNet(netId);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, RegisterNetSupplierTest001, TestSize.Level1)
{
    std::set<NetCap> netCaps;
    netCaps.insert(NetCap::NET_CAPABILITY_MMS);
    netCaps.insert(NetCap::NET_CAPABILITY_INTERNET);
    int32_t ret = NetConnService::GetInstance()->RegisterNetSupplier(NetBearType::BEARER_DEFAULT, TEST_IDENT,
        netCaps, g_supplierId);
    EXPECT_EQ(ret, NET_CONN_ERR_NET_TYPE_NOT_FOUND);

    ret = NetConnService::GetInstance()->RegisterNetSupplier(NetBearType::BEARER_ETHERNET, TEST_IDENT,
        netCaps, g_supplierId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = NetConnService::GetInstance()->RegisterNetSupplier(NetBearType::BEARER_VPN, TEST_IDENT,
        netCaps, g_vpnSupplierId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, RegisterNetSupplierTest002, TestSize.Level1)
{
    std::set<NetCap> netCaps;
    netCaps.insert(NetCap::NET_CAPABILITY_INTERNAL_DEFAULT);
    netCaps.insert(NetCap::NET_CAPABILITY_INTERNET);
    auto ret = NetConnService::GetInstance()->RegisterNetSupplier(NetBearType::BEARER_CELLULAR, TEST_IDENT,
        netCaps, g_supplierId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, RegisterNetSupplierCallbackTest001, TestSize.Level1)
{
    sptr<INetSupplierCallback> callback = new (std::nothrow) NetSupplierCallbackStubTestCb();
    ASSERT_NE(callback, nullptr);
    std::set<NetCap> netCaps;
    auto ret = NetConnService::GetInstance()->RegisterNetSupplierCallback(g_supplierId, callback);
    EXPECT_EQ(ret, NETSYS_SUCCESS);
}

HWTEST_F(NetConnServiceTest, UpdateNetSupplierInfoTest001, TestSize.Level1)
{
    sptr<NetSupplierInfo> netSupplierInfo = nullptr;
    auto ret = NetConnService::GetInstance()->UpdateNetSupplierInfo(g_supplierId, netSupplierInfo);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    netSupplierInfo = new (std::nothrow) NetSupplierInfo();
    ASSERT_NE(netSupplierInfo, nullptr);
    ret = NetConnService::GetInstance()->UpdateNetSupplierInfo(TEST_NOTEXISTSUPPLIER, netSupplierInfo);
    EXPECT_EQ(ret, NET_CONN_ERR_NO_SUPPLIER);

    netSupplierInfo->isAvailable_ = true;
    netSupplierInfo->uid_ = TEST_UID;
    netSupplierInfo->ident_ = "0";
    netSupplierInfo->score_ = 90;
    ret = NetConnService::GetInstance()->UpdateNetSupplierInfo(g_vpnSupplierId, netSupplierInfo);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    EXPECT_EQ(NetConnService::GetInstance()->FindNetSupplier(g_vpnSupplierId)->GetNetScore(), 90);

    netSupplierInfo->isAvailable_ = false;
    netSupplierInfo->ident_ = "";
    netSupplierInfo->score_ = 0;
    ret = NetConnService::GetInstance()->UpdateNetSupplierInfo(g_vpnSupplierId, netSupplierInfo);
    EXPECT_EQ(ret, NETSYS_SUCCESS);
    EXPECT_EQ(NetConnService::GetInstance()->FindNetSupplier(g_vpnSupplierId)->GetNetScore(), 90);
}

HWTEST_F(NetConnServiceTest, UpdateNetLinkInfoTest001, TestSize.Level1)
{
    sptr<NetLinkInfo> netLinkInfo = nullptr;
    auto ret = NetConnService::GetInstance()->UpdateNetLinkInfo(g_supplierId, netLinkInfo);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    netLinkInfo = new (std::nothrow) NetLinkInfo();
    ret = NetConnService::GetInstance()->UpdateNetLinkInfo(TEST_NOTEXISTSUPPLIER, netLinkInfo);
    EXPECT_EQ(ret, NET_CONN_ERR_NO_SUPPLIER);

    ASSERT_NE(netLinkInfo, nullptr);
    netLinkInfo->httpProxy_.SetHost(TEST_HOST);
    ret = NetConnService::GetInstance()->UpdateNetLinkInfo(g_supplierId, netLinkInfo);
    EXPECT_EQ(ret, NETSYS_SUCCESS);
}

HWTEST_F(NetConnServiceTest, UpdateNetLinkInfoTest002, TestSize.Level1)
{
    sptr<NetLinkInfo> netLinkInfo = new (std::nothrow) NetLinkInfo();
    netLinkInfo->ifaceName_ = "rmnet0";
    INetAddr netAddr;
    netAddr.type_ = INetAddr::IPV4;
    netAddr.hostName_ = "testHost";
    netLinkInfo->netAddrList_.push_back(netAddr);
    auto ret = NetConnService::GetInstance()->UpdateNetLinkInfo(g_supplierId, netLinkInfo);
    EXPECT_EQ(ret, NETSYS_SUCCESS);
    ret = NetConnService::GetInstance()->IsIfaceNameInUse("rmnet0", 1);
    EXPECT_FALSE(ret);
    ret = NetConnService::GetInstance()->IsIfaceNameInUse("rmnet0", 100);
    EXPECT_FALSE(ret);

    ret = NetConnService::GetInstance()->IsAddrInOtherNetwork("rmnet0", 1, netAddr);
    EXPECT_FALSE(ret);
    ret = NetConnService::GetInstance()->IsAddrInOtherNetwork("rmnet0", 100, netAddr);
    EXPECT_TRUE(ret);
}

HWTEST_F(NetConnServiceTest, RequestNetConnectionTest001, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    sptr<NetSpecifier> netSpecifier = new (std::nothrow) NetSpecifier();
    netSpecifier->netCapabilities_.bearerTypes_.emplace(NetManagerStandard::BEARER_CELLULAR);
    netSpecifier->netCapabilities_.netCaps_.emplace(NetManagerStandard::NET_CAPABILITY_INTERNAL_DEFAULT);
    ASSERT_NE(netSpecifier, nullptr);
    auto ret = NetConnService::GetInstance()->RequestNetConnection(netSpecifier, g_callback, TEST_TIMEOUTMS);
    EXPECT_EQ(ret, NETSYS_SUCCESS);

    sptr<INetConnCallback> callback = nullptr;
    uint32_t timeoutMS = 0;
    sptr<NetSpecifier> invalidNetSpecifier = nullptr;
    ret = NetConnService::GetInstance()->RequestNetConnection(invalidNetSpecifier, callback, timeoutMS);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);

    NetConnService::RegisterType registerType = NetConnService::RegisterType::INVALIDTYPE;
    uint32_t reqId = 0;
    NetConnService::GetInstance()->FindSameCallback(g_callback, reqId, registerType);
    EXPECT_EQ(registerType, NetConnService::RegisterType::REQUEST);

    ret = NetConnService::GetInstance()->UnregisterNetConnCallback(g_callback);
    EXPECT_EQ(ret, NETSYS_SUCCESS);
}

HWTEST_F(NetConnServiceTest, RequestNetConnectionTest002, TestSize.Level1)
{
    sptr<NetSpecifier> netSpecifier = new (std::nothrow) NetSpecifier();
    int64_t TEST_CALLBACK_UID = 1111;
    auto ret = -1;
    vector<sptr<INetConnCallback>> uidCallbacks;
    for (int32_t i = 1; i <= 2000; ++i) {
        sptr<INetConnCallback> uidCallback = new (std::nothrow) NetConnCallbackStubCb();
        ret = NetConnService::GetInstance()->RequestNetConnectionAsync(netSpecifier, uidCallback, 0,
                                                                                        TEST_CALLBACK_UID);
        EXPECT_EQ(ret, NETMANAGER_SUCCESS);
        uidCallbacks.push_back(uidCallback);
    }
    sptr<INetConnCallback> uidCallback = new (std::nothrow) NetConnCallbackStubCb();
    ret = NetConnService::GetInstance()->RequestNetConnectionAsync(netSpecifier, uidCallback, 0, TEST_CALLBACK_UID);
    EXPECT_EQ(ret, NET_CONN_ERR_NET_OVER_MAX_REQUEST_NUM);
    for (auto& callback : uidCallbacks) {
        ret = NetConnService::GetInstance()->UnregisterNetConnCallbackAsync(callback, TEST_CALLBACK_UID);
        EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    }
}

HWTEST_F(NetConnServiceTest, RegisterNetConnCallbackTest001, TestSize.Level1)
{
    auto ret = NetConnService::GetInstance()->RegisterNetConnCallback(g_callback);
    EXPECT_EQ(ret, NETSYS_SUCCESS);
}

HWTEST_F(NetConnServiceTest, UnregisterNetConnCallbackTest001, TestSize.Level1)
{
    sptr<INetConnCallback> netCallback = new (std::nothrow) NetConnCallbackStubCb();
    auto ret = NetConnService::GetInstance()->UnregisterNetConnCallback(netCallback);
    EXPECT_EQ(ret, NET_CONN_ERR_CALLBACK_NOT_FOUND);

    ret = NetConnService::GetInstance()->UnregisterNetConnCallback(g_callback);
    EXPECT_EQ(ret, NETSYS_SUCCESS);
}

HWTEST_F(NetConnServiceTest, RegisterNetConnCallbackTest002, TestSize.Level1)
{
    sptr<NetSpecifier> netSpecifier = new (std::nothrow) NetSpecifier();
    ASSERT_NE(netSpecifier, nullptr);
    auto ret = NetConnService::GetInstance()->RegisterNetConnCallback(netSpecifier, g_callback,
                                                                                        TEST_TIMEOUTMS);
    EXPECT_EQ(ret, NETSYS_SUCCESS);
}

HWTEST_F(NetConnServiceTest, RegisterNetConnCallbackTest003, TestSize.Level1)
{
    sptr<NetSpecifier> netSpecifier = new (std::nothrow) NetSpecifier();
    int64_t TEST_CALLBACK_UID = 1111;
    auto ret = -1;
    vector<sptr<INetConnCallback>> uidCallbacks;
    for (int32_t i = 1; i <= 2000; ++i) {
        sptr<INetConnCallback> uidCallback = new (std::nothrow) NetConnCallbackStubCb();
        ret = NetConnService::GetInstance()->RegisterNetConnCallbackAsync(netSpecifier, uidCallback, 0,
                                                                                        TEST_CALLBACK_UID);
        EXPECT_EQ(ret, NETMANAGER_SUCCESS);
        uidCallbacks.push_back(uidCallback);
    }
    sptr<INetConnCallback> uidCallback = new (std::nothrow) NetConnCallbackStubCb();
    ret = NetConnService::GetInstance()->RegisterNetConnCallbackAsync(netSpecifier, uidCallback, 0, TEST_CALLBACK_UID);
    EXPECT_EQ(ret, NET_CONN_ERR_NET_OVER_MAX_REQUEST_NUM);
    for (auto& callback : uidCallbacks) {
        ret = NetConnService::GetInstance()->UnregisterNetConnCallbackAsync(callback, TEST_CALLBACK_UID);
        EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    }
}

HWTEST_F(NetConnServiceTest, RegisterNetDetectionCallbackTest001, TestSize.Level1)
{
    sptr<INetDetectionCallback> callback_ = nullptr;
    auto ret = NetConnService::GetInstance()->RegisterNetDetectionCallback(TEST_NETID, callback_);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);

    ret = NetConnService::GetInstance()->RegisterNetDetectionCallback(TEST_NETID, g_detectionCallback);
    EXPECT_EQ(ret, NET_CONN_ERR_NETID_NOT_FOUND);

    ret = NetConnService::GetInstance()->RegisterNetDetectionCallback(MIN_NET_ID, g_detectionCallback);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, UnRegisterNetDetectionCallbackTest001, TestSize.Level1)
{
    auto ret = NetConnService::GetInstance()->UnRegisterNetDetectionCallback(MIN_NET_ID, g_detectionCallback);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, UpdateNetStateForTestTest001, TestSize.Level1)
{
    int32_t netState = 0;
    sptr<NetSpecifier> netSpecifier = nullptr;
    auto ret = NetConnService::GetInstance()->UpdateNetStateForTest(netSpecifier, netState);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);

    netSpecifier = new (std::nothrow) NetSpecifier();
    ret = NetConnService::GetInstance()->UpdateNetStateForTest(netSpecifier, netState);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, GetAllNetsTest001, TestSize.Level1)
{
    std::list<int32_t> netIdList;
    auto ret = NetConnService::GetInstance()->GetAllNets(netIdList);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, GetSpecificUidNetTest001, TestSize.Level1)
{
    int32_t defaultNetId = 5;
    auto ret = NetConnService::GetInstance()->GetDefaultNet(defaultNetId);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
    EXPECT_NE(defaultNetId, 0);

    std::list<int32_t> netIdList;
    ret = NetConnService::GetInstance()->GetSpecificNet(BEARER_VPN, netIdList);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
    EXPECT_GE(netIdList.size(), 0);

    int32_t netID = 0;
    NetConnService::GetInstance()->GetSpecificUidNet(TEST_NOTEXISTSUPPLIER, netID);
    EXPECT_EQ(netID, defaultNetId);

    NetConnService::GetInstance()->GetSpecificUidNet(TEST_UID, netID);
    EXPECT_EQ(netID, *netIdList.begin());
}

HWTEST_F(NetConnServiceTest, GetConnectionPropertiesTest001, TestSize.Level1)
{
    NetLinkInfo info;
    auto ret = NetConnService::GetInstance()->GetConnectionProperties(TEST_NETID, info);
    EXPECT_EQ(ret, NET_CONN_ERR_INVALID_NETWORK);

    int32_t defaultNetId = 0;
    ret = NetConnService::GetInstance()->GetDefaultNet(defaultNetId);

    ret = NetConnService::GetInstance()->GetConnectionProperties(defaultNetId, info);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, GetAddressesByNameTest001, TestSize.Level1)
{
    std::vector<INetAddr> addrList;
    auto ret = NetConnService::GetInstance()->GetAddressesByName(TEST_HOST, TEST_NETID, addrList);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceTest, GetAddressByNameTest001, TestSize.Level1)
{
    int32_t netId = 0;
    auto ret = NetConnService::GetInstance()->GetDefaultNet(netId);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
    EXPECT_NE(netId, 0);

    INetAddr addr;
    ret = NetConnService::GetInstance()->GetAddressByName(TEST_HOST, netId, addr);
    EXPECT_EQ(ret, NETMANAGER_ERROR);

    sptr<TestDnsService> dnsService = new (std::nothrow) TestDnsService();
    NetManagerCenter::GetInstance().RegisterDnsService(dnsService);

    ret = NetConnService::GetInstance()->GetAddressByName(TEST_HOST, netId, addr);
    EXPECT_EQ(ret, NET_CONN_ERR_NO_ADDRESS);

    ret = NetConnService::GetInstance()->GetAddressByName(TEST_HOST, TEST_NETID, addr);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, BindSocketTest001, TestSize.Level1)
{
    auto ret = NetConnService::GetInstance()->BindSocket(TEST_SOCKETFD, TEST_NETID);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, NetDetectionTest001, TestSize.Level1)
{
    auto ret = NetConnService::GetInstance()->NetDetection(TEST_NETID);
    EXPECT_EQ(ret, NET_CONN_ERR_NETID_NOT_FOUND);

    ret = NetConnService::GetInstance()->NetDetection(MIN_NET_ID);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, GetNetIdByIdentifierTest001, TestSize.Level1)
{
    std::list<int32_t> netIdList;
    auto ret = NetConnService::GetInstance()->GetNetIdByIdentifier("", netIdList);
    EXPECT_EQ(ret, NETMANAGER_ERR_INVALID_PARAMETER);

    ret = NetConnService::GetInstance()->GetNetIdByIdentifier(TEST_IDENT, netIdList);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, GetDefaultNetTest001, TestSize.Level1)
{
    int32_t netId = 0;
    auto ret = NetConnService::GetInstance()->GetDefaultNet(netId);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, HasDefaultNetTest001, TestSize.Level1)
{
    bool bFlag = false;
    auto ret = NetConnService::GetInstance()->HasDefaultNet(bFlag);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
    EXPECT_TRUE(bFlag);

    ret = DelayedSingleton<NetConnService>::GetInstance()->HasDefaultNet(bFlag);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    EXPECT_FALSE(bFlag);
}

HWTEST_F(NetConnServiceTest, GetNetCapabilitiesTest001, TestSize.Level1)
{
    int32_t netId = 0;
    int32_t ret = NetConnService::GetInstance()->GetDefaultNet(netId);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);

    NetAllCapabilities netAllCap;
    ret = NetConnService::GetInstance()->GetNetCapabilities(TEST_NETID, netAllCap);
    ASSERT_EQ(ret, NET_CONN_ERR_INVALID_NETWORK);

    ret = NetConnService::GetInstance()->GetNetCapabilities(netId, netAllCap);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, SetAirplaneModeTest001, TestSize.Level1)
{
    auto ret = NetConnService::GetInstance()->SetAirplaneMode(true);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, SetAirplaneModeTest002, TestSize.Level1)
{
    auto ret = NetConnService::GetInstance()->SetAirplaneMode(false);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, IsDefaultNetMeteredTest001, TestSize.Level1)
{
    bool bRes = false;
    auto ret = NetConnService::GetInstance()->IsDefaultNetMetered(bRes);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);

    ret = DelayedSingleton<NetConnService>::GetInstance()->IsDefaultNetMetered(bRes);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
    EXPECT_TRUE(bRes);
}

HWTEST_F(NetConnServiceTest, SetGlobalHttpProxyTest001, TestSize.Level1)
{
    HttpProxy httpProxy = {TEST_PROXY_HOST, 0, {}};
    auto ret = NetConnService::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, SetGlobalHttpProxyTest002, TestSize.Level1)
{
    HttpProxy httpProxy = {TEST_DOMAIN1, 8080, {}};
    auto ret = NetConnService::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, SetGlobalHttpProxyTest003, TestSize.Level1)
{
    HttpProxy httpProxy = {TEST_DOMAIN2, 8080, {}};
    auto ret = NetConnService::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, SetGlobalHttpProxyTest004, TestSize.Level1)
{
    HttpProxy httpProxy = {TEST_DOMAIN3, 8080, {}};
    auto ret = NetConnService::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, SetGlobalHttpProxyTest005, TestSize.Level1)
{
    HttpProxy httpProxy = {TEST_DOMAIN4, 8080, {}};
    auto ret = NetConnService::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, SetGlobalHttpProxyTest006, TestSize.Level1)
{
    HttpProxy httpProxy = {TEST_DOMAIN5, 8080, {}};
    auto ret = NetConnService::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, SetGlobalHttpProxyTest007, TestSize.Level1)
{
    HttpProxy httpProxy = {TEST_DOMAIN6, 8080, {}};
    auto ret = NetConnService::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, SetGlobalHttpProxyTest008, TestSize.Level1)
{
    HttpProxy httpProxy = {TEST_DOMAIN7, 8080, {}};
    auto ret = NetConnService::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, SetGlobalHttpProxyTest009, TestSize.Level1)
{
    HttpProxy httpProxy = {TEST_DOMAIN8, 8080, {}};
    auto ret = NetConnService::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, SetGlobalHttpProxyTest010, TestSize.Level1)
{
    HttpProxy httpProxy = {TEST_DOMAIN9, 8080, {}};
    auto ret = NetConnService::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, SetGlobalHttpProxyTest011, TestSize.Level1)
{
    HttpProxy httpProxy = {TEST_DOMAIN10, 8080, {}};
    auto ret = NetConnService::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, SetGlobalHttpProxyTest012, TestSize.Level1)
{
    HttpProxy httpProxy = {TEST_IPV4_ADDR, 8080, {}};
    auto ret = NetConnService::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, SetGlobalHttpProxyTest013, TestSize.Level1)
{
    HttpProxy httpProxy = {TEST_IPV6_ADDR, 8080, {}};
    auto ret = NetConnService::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, SetGlobalHttpProxyTest014, TestSize.Level1)
{
    HttpProxy httpProxy = {TEST_LONG_HOST, 8080, {TEST_LONG_EXCLUSION_LIST}};
    auto ret = NetConnService::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, GetGlobalHttpProxyTest001, TestSize.Level1)
{
    HttpProxy getGlobalHttpProxy;
    int32_t ret = NetConnService::GetInstance()->GetGlobalHttpProxy(getGlobalHttpProxy);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, GetDefaultHttpProxyTest001, TestSize.Level1)
{
    int32_t bindNetId = 0;
    HttpProxy defaultHttpProxy;
    int32_t ret = NetConnService::GetInstance()->GetDefaultHttpProxy(bindNetId, defaultHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

HWTEST_F(NetConnServiceTest, GetDefaultHttpProxyTest002, TestSize.Level1)
{
    int32_t bindNetId = NET_ID;
    HttpProxy defaultHttpProxy;
    int32_t ret = NetConnService::GetInstance()->GetDefaultHttpProxy(bindNetId, defaultHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

HWTEST_F(NetConnServiceTest, GetCallingUserIdTest001, TestSize.Level1)
{
    int32_t userId;
    int32_t ret = NetConnService::GetInstance()->GetCallingUserId(userId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, GetTest001, TestSize.Level1)
{
    std::list<int32_t> netIdList;
    int32_t ret = NetConnService::GetInstance()->GetSpecificNet(BEARER_CELLULAR, netIdList);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = NetConnService::GetInstance()->GetSpecificNet(BEARER_DEFAULT, netIdList);
    EXPECT_EQ(ret, NET_CONN_ERR_NET_TYPE_NOT_FOUND);

    ret = NetConnService::GetInstance()->RestrictBackgroundChanged(false);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = NetConnService::GetInstance()->RestrictBackgroundChanged(false);
    EXPECT_EQ(ret, NET_CONN_ERR_NET_NO_RESTRICT_BACKGROUND);

    NetConnService::GetInstance()->HandleDetectionResult(TEST_NOTEXISTSUPPLIER, VERIFICATION_STATE);
    NetConnService::GetInstance()->HandleDetectionResult(g_supplierId, VERIFICATION_STATE);

    std::vector<std::u16string> args;
    args.emplace_back(u"dummy data");
    ret = NetConnService::GetInstance()->Dump(SOCKET_FD, args);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = DelayedSingleton<NetConnService>::GetInstance()->Dump(SOCKET_FD, args);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, OnNetActivateTimeOutTest001, TestSize.Level1)
{
    NetConnService::GetInstance()->OnNetActivateTimeOut(NET_ID);
    if (NetConnService::GetInstance()->netActivates_.size() > 0) {
        uint32_t nNetID = NetConnService::GetInstance()->netActivates_.begin()->first;
        NetConnService::GetInstance()->OnNetActivateTimeOut(nNetID);
        for (auto iterSupplier = NetConnService::GetInstance()->netSuppliers_.begin();
             iterSupplier != NetConnService::GetInstance()->netSuppliers_.end(); ++iterSupplier) {
            if (iterSupplier->second == nullptr) {
                continue;
            }
            EXPECT_EQ(iterSupplier->second->requestList_.find(nNetID), iterSupplier->second->requestList_.end());
        }
    }
}

HWTEST_F(NetConnServiceTest, GetIfaceNamesTest001, TestSize.Level1)
{
    std::list<std::string> ifaceNames;
    auto ret = NetConnService::GetInstance()->GetIfaceNames(BEARER_DEFAULT, ifaceNames);
    EXPECT_EQ(ret, NET_CONN_ERR_NET_TYPE_NOT_FOUND);

    ret = NetConnService::GetInstance()->GetIfaceNames(BEARER_VPN, ifaceNames);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, GetIfaceNameByTypeTest001, TestSize.Level1)
{
    std::string ifaceName;
    auto ret = NetConnService::GetInstance()->GetIfaceNameByType(BEARER_DEFAULT, TEST_IDENT, ifaceName);
    EXPECT_EQ(ret, NET_CONN_ERR_NET_TYPE_NOT_FOUND);

    ret = NetConnService::GetInstance()->GetIfaceNameByType(BEARER_BLUETOOTH, TEST_IDENT, ifaceName);
    EXPECT_EQ(ret, NET_CONN_ERR_NO_SUPPLIER);

    ret = NetConnService::GetInstance()->GetIfaceNameByType(BEARER_VPN, TEST_IDENT, ifaceName);
    EXPECT_EQ(ret, NET_CONN_ERR_NO_SUPPLIER);
}

HWTEST_F(NetConnServiceTest, GetIfaceNameIdentMapsTest001, TestSize.Level1)
{
    SafeMap<std::string, std::string> data;
    auto ret = NetConnService::GetInstance()->GetIfaceNameIdentMaps(BEARER_CELLULAR, data);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, SetAppNetTest001, TestSize.Level1)
{
    auto ret = NetConnService::GetInstance()->SetAppNet(TEST_NETID);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, RegisterNetInterfaceCallbackTest001, TestSize.Level1)
{
    sptr<INetInterfaceStateCallback> callback = nullptr;
    auto ret = NetConnService::GetInstance()->RegisterNetInterfaceCallback(callback);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);

    callback = new (std::nothrow) NetInterfaceStateCallbackStub();
    ret = NetConnService::GetInstance()->RegisterNetInterfaceCallback(callback);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);

    NetConnService::GetInstance()->interfaceStateCallback_ =
        new (std::nothrow) NetConnService::NetInterfaceStateCallback();
    ASSERT_NE(NetConnService::GetInstance()->interfaceStateCallback_, nullptr);
    NetsysController::GetInstance().RegisterCallback(NetConnService::GetInstance()->interfaceStateCallback_);

    ret = NetConnService::GetInstance()->RegisterNetInterfaceCallback(callback);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, GetNetInterfaceConfigurationTest001, TestSize.Level1)
{
    NetInterfaceConfiguration config;
    auto ret = NetConnService::GetInstance()->GetNetInterfaceConfiguration("wlan0", config);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, AddNetworkRouteTest001, TestSize.Level1)
{
    int32_t netId = 10;
    std::string ifName = "wlan0";
    std::string destination = "0.0.0.0/0";
    std::string nextHop = "0.0.0.1234";
    int32_t ret = NetConnService::GetInstance()->AddNetworkRoute(netId, ifName, destination, nextHop);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceTest, RemoveNetworkRouteTest001, TestSize.Level1)
{
    int32_t netId = 10;
    std::string ifName = "wlan0";
    std::string destination = "0.0.0.0/0";
    std::string nextHop = "0.0.0.1234";
    int32_t ret = NetConnService::GetInstance()->RemoveNetworkRoute(netId, ifName, destination, nextHop);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceTest, AddInterfaceAddressTest001, TestSize.Level1)
{
    std::string ifName = "wlan0";
    std::string ipAddr = "0.0.0.1";
    int32_t prefixLength = 23;
    int32_t ret = NetConnService::GetInstance()->AddInterfaceAddress(ifName, ipAddr, prefixLength);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, DelInterfaceAddressTest001, TestSize.Level1)
{
    std::string ifName = "wlan0";
    std::string ipAddr = "0.0.0.1";
    int32_t prefixLength = 23;
    int32_t ret = NetConnService::GetInstance()->DelInterfaceAddress(ifName, ipAddr, prefixLength);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, AddStaticArpTest001, TestSize.Level1)
{
    std::string ipAddr = "192.168.1.100";
    std::string macAddr = "aa:bb:cc:dd:ee:ff";
    std::string ifName = "wlan0";
    int32_t ret = NetConnService::GetInstance()->AddStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, DelStaticArpTest001, TestSize.Level1)
{
    std::string ipAddr = "192.168.1.100";
    std::string macAddr = "aa:bb:cc:dd:ee:ff";
    std::string ifName = "wlan0";
    int32_t ret = NetConnService::GetInstance()->DelStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, NetConnServiceBranchTest001, TestSize.Level1)
{
    NetConnService::GetInstance()->OnStart();
    EXPECT_EQ(NetConnService::GetInstance()->state_, NetConnService::STATE_RUNNING);
    NetConnService::GetInstance()->OnStop();
    EXPECT_EQ(NetConnService::GetInstance()->state_, NetConnService::STATE_STOPPED);
    bool result = NetConnService::GetInstance()->Init();
    EXPECT_FALSE(result);

    uint32_t reqId = 0;
    result = NetConnService::GetInstance()->FindSameCallback(nullptr, reqId);
    EXPECT_FALSE(result);

    sptr<NetSupplier> supplier = nullptr;
    std::shared_ptr<NetActivate> netActivateNetwork = nullptr;
    auto ret = NetConnService::GetInstance()->FindBestNetworkForRequest(supplier, netActivateNetwork);
    EXPECT_EQ(ret, ZERO_VALUE);

    NetConnService::GetInstance()->SendAllRequestToNetwork(nullptr);

    NetConnService::GetInstance()->SendRequestToAllNetwork(nullptr);

    sptr<NetLinkInfo> netLinkInfo = new (std::nothrow) NetLinkInfo();
    ASSERT_NE(netLinkInfo, nullptr);
    netLinkInfo->httpProxy_.SetHost(TEST_HOST);
    ret = NetConnService::GetInstance()->UpdateNetLinkInfo(g_supplierId, netLinkInfo);
    EXPECT_EQ(ret, NETMANAGER_ERROR);

    supplier = NetConnService::GetInstance()->FindNetSupplier(g_supplierId);
    ASSERT_NE(supplier, nullptr);

    CallbackType type = CallbackType::CALL_TYPE_LOST;
    NetConnService::GetInstance()->CallbackForSupplier(supplier, type);
    type = CallbackType::CALL_TYPE_UPDATE_CAP;
    NetConnService::GetInstance()->CallbackForSupplier(supplier, type);
    type = CallbackType::CALL_TYPE_UPDATE_LINK;
    NetConnService::GetInstance()->CallbackForSupplier(supplier, type);
    type = CallbackType::CALL_TYPE_BLOCK_STATUS;
    NetConnService::GetInstance()->CallbackForSupplier(supplier, type);

    uint32_t validType = INVALID_VALUE;
    type = static_cast<CallbackType>(validType);
    NetConnService::GetInstance()->CallbackForSupplier(supplier, type);

    ret = NetConnService::GetInstance()->RegisterNetConnCallbackAsync(nullptr, nullptr, 0, TEST_UID);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);
}

HWTEST_F(NetConnServiceTest, NetConnServiceBranchTest002, TestSize.Level1)
{
    auto ret = NetConnService::GetInstance()->UnregisterNetConnCallbackAsync(nullptr, TEST_UID);
    EXPECT_NE(ret, NETSYS_SUCCESS);

    sptr<NetSupplier> supplier = nullptr;
    sptr<NetSupplier> newSupplier = nullptr;
    NetConnService::GetInstance()->MakeDefaultNetWork(supplier, newSupplier);

    ret = NetConnService::GetInstance()->ActivateNetwork(nullptr, nullptr, 0, 0);
    EXPECT_NE(ret, NETSYS_SUCCESS);
}

HWTEST_F(NetConnServiceTest, NetDetectionForDnsHealthTest001, TestSize.Level1)
{
    int32_t netId = 0;
    auto ret = NetConnService::GetInstance()->GetDefaultNet(netId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    bool dnsHealthSuccess = true;
    bool dnsHealthFail = false;
    ret = NetConnService::GetInstance()->NetDetectionForDnsHealth(netId, dnsHealthSuccess);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
    ret = NetConnService::GetInstance()->NetDetectionForDnsHealth(netId, dnsHealthFail);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceTest, FactoryResetNetworkTest001, TestSize.Level1)
{
    NetConnService::GetInstance()->netFactoryResetCallback_ =
        new (std::nothrow) NetFactoryResetCallback();
    ASSERT_NE(NetConnService::GetInstance()->netFactoryResetCallback_, nullptr);
    auto ret = NetConnService::GetInstance()->FactoryResetNetwork();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, RegisterNetFactoryResetCallbackTest001, TestSize.Level1)
{
    sptr<INetFactoryResetCallback> callback = nullptr;
    auto ret = NetConnService::GetInstance()->RegisterNetFactoryResetCallback(callback);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);

    NetConnService::GetInstance()->netFactoryResetCallback_ =
        new (std::nothrow) NetFactoryResetCallback();
    ASSERT_NE(NetConnService::GetInstance()->netFactoryResetCallback_, nullptr);

    callback = new (std::nothrow) NetFactoryResetCallbackStub();
    ret = NetConnService::GetInstance()->RegisterNetFactoryResetCallback(callback);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, NetConnServiceBranchTest003, TestSize.Level1)
{
    HttpProxy proxy;
    uint32_t supplierId = 0;
    std::string testString = "test";
    int32_t testInt = 0;
    std::set<NetCap> netCaps;
    NetConnService::GetInstance()->netConnEventHandler_ = nullptr;
    NetConnService::GetInstance()->RegisterNetSupplier(NetBearType::BEARER_BLUETOOTH, testString, netCaps, supplierId);
    NetConnService::GetInstance()->UnregisterNetSupplier(supplierId);
    NetConnService::GetInstance()->UpdateGlobalHttpProxy(proxy);
    NetConnService::GetInstance()->OnNetActivateTimeOut(testInt);
    NetConnService::GetInstance()->UnregisterNetSupplierAsync(supplierId);
    sptr<NetSupplier> supplier = nullptr;
    NetConnService::GetInstance()->CallbackForSupplier(supplier, CallbackType::CALL_TYPE_AVAILABLE);

    sptr<INetSupplierCallback> supplierCallback = nullptr;
    auto ret = NetConnService::GetInstance()->RegisterNetSupplierCallbackAsync(supplierId, supplierCallback);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);

    ret = NetConnService::GetInstance()->RegisterNetSupplierCallback(supplierId, supplierCallback);
    EXPECT_EQ(ret, NETMANAGER_ERROR);

    sptr<INetConnCallback> callback = nullptr;
    uint32_t timeoutMS = 0;
    sptr<NetSpecifier> netSpecifier = nullptr;
    ret = NetConnService::GetInstance()->RegisterNetConnCallback(netSpecifier, callback, timeoutMS);
    EXPECT_EQ(ret, NETMANAGER_ERROR);

    ret = NetConnService::GetInstance()->RequestNetConnection(netSpecifier, callback, timeoutMS);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);

    ret = NetConnService::GetInstance()->UnregisterNetConnCallback(callback);
    EXPECT_EQ(ret, NETMANAGER_ERROR);

    sptr<INetDetectionCallback> detectionCallback = nullptr;
    ret = NetConnService::GetInstance()->RegUnRegNetDetectionCallback(testInt, detectionCallback, false);
    EXPECT_EQ(ret, NETMANAGER_ERROR);

    ret = NetConnService::GetInstance()->UpdateNetStateForTest(netSpecifier, testInt);
    EXPECT_EQ(ret, NETMANAGER_ERROR);

    sptr<NetSupplierInfo> netSupplierInfo = nullptr;
    ret = NetConnService::GetInstance()->UpdateNetSupplierInfo(testInt, netSupplierInfo);
    EXPECT_EQ(ret, NETMANAGER_ERROR);

    sptr<NetLinkInfo> netLinkInfo = nullptr;
    ret = NetConnService::GetInstance()->UpdateNetLinkInfo(testInt, netLinkInfo);
    EXPECT_EQ(ret, NETMANAGER_ERROR);

    ret = NetConnService::GetInstance()->NetDetection(testInt);
    EXPECT_EQ(ret, NETMANAGER_ERROR);

    ret = NetConnService::GetInstance()->RestrictBackgroundChanged(false);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceTest, NetConnServiceBranchTest004, TestSize.Level1)
{
    NetConnService::GetInstance()->RequestAllNetworkExceptDefault();

    NetConnService::NetInterfaceStateCallback stateCallback;
    std::string testString = "test";
    int32_t testInt = 0;
    auto ret = stateCallback.OnInterfaceAddressUpdated(testString, testString, testInt, testInt);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = stateCallback.OnInterfaceAddressRemoved(testString, testString, testInt, testInt);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = stateCallback.OnInterfaceAdded(testString);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = stateCallback.OnInterfaceRemoved(testString);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = stateCallback.OnInterfaceChanged(testString, false);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = stateCallback.OnInterfaceLinkStateChanged(testString, false);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = stateCallback.OnRouteChanged(false, testString, testString, testString);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    NetsysControllerCallback::DhcpResult dhcpResult;
    ret = stateCallback.OnDhcpSuccess(dhcpResult);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = stateCallback.OnBandwidthReachedLimit(testString, testString);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    sptr<INetInterfaceStateCallback> interfaceStateCallback = nullptr;
    ret = stateCallback.RegisterInterfaceCallback(interfaceStateCallback);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);
}

HWTEST_F(NetConnServiceTest, NetConnServiceBranchTest005, TestSize.Level1)
{
    NetHttpProxyTracker httpProxyTracker;
    std::string exclusions = "";
    NetConnService::GetInstance()->GetPreferredUrl();
    std::list<std::string> list = httpProxyTracker.ParseExclusionList(exclusions);
    EXPECT_TRUE(list.empty());

    std::string result = httpProxyTracker.GetExclusionsAsString(list);
    EXPECT_TRUE(result.empty());

    uint32_t supplierId = 10;
    int32_t type = 0;
    auto ret = NetConnService::GetInstance()->RegisterSlotType(supplierId, type);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    std::string slotType = "";
    ret = NetConnService::GetInstance()->GetSlotType(slotType);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    std::string url = "";
    bool preferCellular = false;
    ret = NetConnService::GetInstance()->IsPreferCellularUrl(url, preferCellular);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    NetConnService::GetInstance()->netFactoryResetCallback_ = nullptr;
    ret = NetConnService::GetInstance()->FactoryResetNetwork();
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);
}

HWTEST_F(NetConnServiceTest, FindSupplierWithInternetByBearerType001, TestSize.Level1)
{
    std::set<NetCap> netCaps;
    netCaps.insert(NetCap::NET_CAPABILITY_MMS);
    netCaps.insert(NetCap::NET_CAPABILITY_INTERNET);
    uint32_t supplierId = 0;
    int32_t ret = NetConnService::GetInstance()->RegisterNetSupplierAsync(NetBearType::BEARER_WIFI, TEST_IDENT,
        netCaps, supplierId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    std::vector<sptr<NetSupplier>> suppliers =
        NetConnService::GetInstance()->FindSupplierWithInternetByBearerType(NetBearType::BEARER_WIFI);
    EXPECT_FALSE(suppliers.empty());
}

HWTEST_F(NetConnServiceTest, UpdateSupplierScore001, TestSize.Level1)
{
    std::set<NetCap> netCaps;
    netCaps.insert(NetCap::NET_CAPABILITY_MMS);
    netCaps.insert(NetCap::NET_CAPABILITY_INTERNET);
    uint32_t supplierId = 0;
    int32_t ret = NetConnService::GetInstance()->RegisterNetSupplierAsync(NetBearType::BEARER_WIFI, TEST_IDENT,
        netCaps, supplierId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    NetConnService::GetInstance()->MakeDefaultNetWork(NetConnService::GetInstance()->defaultNetSupplier_,
        NetConnService::GetInstance()->netSuppliers_[supplierId]);
    bool isBetter = false;
    ret = NetConnService::GetInstance()->UpdateSupplierScoreAsync(NetBearType::BEARER_WIFI, isBetter, supplierId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    auto supplier = NetConnService::GetInstance()->FindNetSupplier(supplierId);
    supplier->SetDetectionDone();
    EXPECT_EQ(supplier->GetRealScore(), supplier->GetNetScore() - DIFF_SCORE_BETWEEN_GOOD_POOR);
}

HWTEST_F(NetConnServiceTest, UpdateSupplierScore002, TestSize.Level1)
{
    std::set<NetCap> netCaps;
    netCaps.insert(NetCap::NET_CAPABILITY_MMS);
    netCaps.insert(NetCap::NET_CAPABILITY_INTERNET);
    uint32_t supplierId = 0;
    int32_t ret = NetConnService::GetInstance()->RegisterNetSupplierAsync(NetBearType::BEARER_WIFI, TEST_IDENT,
        netCaps, supplierId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    bool isBetter = true;
    ret = NetConnService::GetInstance()->UpdateSupplierScoreAsync(NetBearType::BEARER_WIFI, isBetter, supplierId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, EnableVnicNetwork001, TestSize.Level1)
{
    sptr<NetManagerStandard::NetLinkInfo> linkInfo = nullptr;
    std::set<int32_t> uids;

    linkInfo = new (std::nothrow) NetManagerStandard::NetLinkInfo();
    ASSERT_NE(linkInfo, nullptr);

    int32_t ret = NetConnService::GetInstance()->EnableVnicNetworkAsync(linkInfo, uids);
    EXPECT_EQ(ret, NET_CONN_ERR_INVALID_NETWORK);
}

HWTEST_F(NetConnServiceTest, EnableVnicNetwork002, TestSize.Level1)
{
    sptr<NetManagerStandard::NetLinkInfo> linkInfo = nullptr;
    std::set<int32_t> uids;

    linkInfo = new (std::nothrow) NetManagerStandard::NetLinkInfo();
    ASSERT_NE(linkInfo, nullptr);

    NetManagerStandard::INetAddr inetAddr;
    inetAddr.type_ = NetManagerStandard::INetAddr::IpType::IPV4;
    inetAddr.family_ = 0x01;
    inetAddr.address_ = "10.0.0.2";
    inetAddr.netMask_ = "255.255.255.0";
    inetAddr.hostName_ = "localhost";
    inetAddr.port_ = 80;
    inetAddr.prefixlen_ = 24;

    linkInfo->ifaceName_ = "vnic-tun";
    linkInfo->netAddrList_.push_back(inetAddr);
    linkInfo->mtu_ = 1500;

    int32_t ret = NetConnService::GetInstance()->EnableVnicNetworkAsync(linkInfo, uids);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, DisableVnicNetwork001, TestSize.Level1)
{
    int32_t ret = NetConnService::GetInstance()->DisableVnicNetworkAsync();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, CmdCloseSocketsUid001, TestSize.Level1)
{
    int32_t netId = 100;
    uint32_t uid = 20020157;
    int32_t ret = NetConnService::GetInstance()->CloseSocketsUid(netId, uid);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
