/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "http_proxy.h"
#include "iservice_registry.h"
#include "native_net_conn_api.h"
#include "native_net_conn_type.h"
#include "net_conn_client.h"
#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"
#include "netmanager_base_test_security.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr const char *LONG_HOST =
    "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
    "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
    "1111111111111111111111111111";
constexpr const char *HOST_NAME = "127.0.0.1";
constexpr uint16_t PORT = 8080;
} // namespace

class NativeNetConnTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    sptr<INetAddr> GetINetAddrSample() const;
    sptr<Route> GetRouteSample() const;
    HttpProxy GetHttpProxySample() const;
    sptr<NetLinkInfo> GetNetLinkInfoSample() const;
    int32_t AddNetwork(const std::string ident) const;
    void LogINetAddr(const OH_NetConn_INetAddr netAddr) const;
    void LogRoute(const OH_NetConn_Route route) const;
    void LogHttpProxy(const OH_NetConn_HttpProxy httpProxy) const;
    void LogNetLinkInfo(const OH_NetConn_NetLinkInfo info) const;
    void LogNetAllCapabilities(const OH_NetConn_NetAllCapabilities netAllCaps) const;
};

void NativeNetConnTest::SetUpTestCase() {}

void NativeNetConnTest::TearDownTestCase() {}

void NativeNetConnTest::SetUp() {}

void NativeNetConnTest::TearDown() {}

sptr<INetAddr> NativeNetConnTest::GetINetAddrSample() const
{
    sptr<INetAddr> netAddr = (std::make_unique<INetAddr>()).release();
    netAddr->type_ = INetAddr::IPV4;
    netAddr->family_ = 0x10;
    netAddr->prefixlen_ = 0x17;
    netAddr->address_ = "192.168.2.0";
    netAddr->netMask_ = "192.255.255.255";
    netAddr->hostName_ = "netAddr";
    return netAddr;
}
sptr<Route> NativeNetConnTest::GetRouteSample() const
{
    sptr<Route> route = (std::make_unique<Route>()).release();
    route->iface_ = "iface0";
    route->destination_.type_ = INetAddr::IPV4;
    route->destination_.family_ = 0x10;
    route->destination_.prefixlen_ = 0x17;
    route->destination_.address_ = "192.168.2.0";
    route->destination_.netMask_ = "192.255.255.255";
    route->destination_.hostName_ = "netAddr";
    route->gateway_.type_ = INetAddr::IPV4;
    route->gateway_.family_ = 0x10;
    route->gateway_.prefixlen_ = 0x17;
    route->gateway_.address_ = "192.168.2.0";
    route->gateway_.netMask_ = "192.255.255.255";
    route->gateway_.hostName_ = "netAddr";
    return route;
}
HttpProxy NativeNetConnTest::GetHttpProxySample() const
{
    HttpProxy httpProxy = HttpProxy();
    httpProxy.SetHost("testHost");
    httpProxy.SetPort(PORT);
    std::list<std::string> exclusionList;
    exclusionList.push_back("testExclusion");
    httpProxy.SetExclusionList(exclusionList);
    return httpProxy;
}

sptr<NetLinkInfo> NativeNetConnTest::GetNetLinkInfoSample() const
{
    sptr<NetLinkInfo> netLinkInfo = (std::make_unique<NetLinkInfo>()).release();
    netLinkInfo->ifaceName_ = "test";
    netLinkInfo->domain_ = "test";
    netLinkInfo->mtu_ = 0x5DC;
    sptr<INetAddr> netAddr = GetINetAddrSample();
    netLinkInfo->netAddrList_.push_back(*netAddr);
    sptr<INetAddr> dns = GetINetAddrSample();
    netLinkInfo->dnsList_.push_back(*dns);
    sptr<Route> route = GetRouteSample();
    netLinkInfo->routeList_.push_back(*route);
    HttpProxy httpProxy = GetHttpProxySample();
    netLinkInfo->httpProxy_ = httpProxy;
    return netLinkInfo;
}

int32_t NativeNetConnTest::AddNetwork(const std::string ident) const
{
    NetBearType bearerType = BEARER_CELLULAR;
    std::set<NetCap> netCaps{NET_CAPABILITY_INTERNET, NET_CAPABILITY_MMS};
    uint32_t supplierId = 0;
    return NetConnClient::GetInstance().RegisterNetSupplier(bearerType, ident, netCaps, supplierId);
}

void NativeNetConnTest::LogINetAddr(const OH_NetConn_INetAddr netAddr) const
{
    std::cout << "iptype: " << netAddr.type << std::endl;
    std::cout << "family: " << netAddr.family << std::endl;
    std::cout << "prefixlen: " << netAddr.prefixlen << std::endl;
    std::cout << "port: " << netAddr.port << std::endl;
    std::cout << "address: " << netAddr.address << std::endl;
    std::cout << "hostName: " << netAddr.hostName << std::endl;
}

void NativeNetConnTest::LogRoute(const OH_NetConn_Route route) const
{
    std::cout << "iface: " << route.iface << std::endl;
    LogINetAddr(route.destination);
    LogINetAddr(route.gateway);
    std::cout << "rtnType: " << route.rtnType << std::endl;
    std::cout << "mtu: " << route.mtu << std::endl;
    std::cout << "isHost: " << route.isHost << std::endl;
    std::cout << "hasGateway: " << route.hasGateway << std::endl;
    std::cout << "isDefaultRoute: " << route.isDefaultRoute << std::endl;
}

void NativeNetConnTest::LogHttpProxy(const OH_NetConn_HttpProxy httpProxy) const
{
    std::cout << "host: " << httpProxy.host << std::endl;
    for (int32_t i = 0; i < httpProxy.exclusionListSize; i++) {
        std::cout << "exclusion: " << httpProxy.exclusionList[i] << std::endl;
    }
    std::cout << "exclusionListSize: " << httpProxy.exclusionListSize << std::endl;
    std::cout << "port: " << httpProxy.port << std::endl;
}

void NativeNetConnTest::LogNetLinkInfo(const OH_NetConn_NetLinkInfo info) const
{
    std::cout << "ifaceName: " << info.ifaceName << std::endl;
    std::cout << "domain: " << info.domain << std::endl;
    std::cout << "tcpBufferSizes: " << info.tcpBufferSizes << std::endl;
    std::cout << "mtu: " << info.mtu << std::endl;
    for (int32_t i = 0; i < info.netAddrListSize; i++) {
        LogINetAddr(info.netAddrList[i]);
    }
    std::cout << "netAddrListSize: " << info.netAddrListSize << std::endl;
    for (int32_t i = 0; i < info.dnsListSize; i++) {
        LogINetAddr(info.dnsList[i]);
    }
    std::cout << "dnsListSize: " << info.dnsListSize << std::endl;
    for (int32_t i = 0; i < info.routeListSize; i++) {
        LogRoute(info.routeList[i]);
    }
    std::cout << "routeListSize: " << info.routeListSize << std::endl;
}

void NativeNetConnTest::LogNetAllCapabilities(const OH_NetConn_NetAllCapabilities netAllCaps) const
{
    std::cout << "linkUpBandwidthKbps: " << netAllCaps.linkUpBandwidthKbps << std::endl;
    std::cout << "linkDownBandwidthKbps: " << netAllCaps.linkDownBandwidthKbps << std::endl;
    for (int32_t i = 0; i < netAllCaps.netCapsSize; i++) {
        std::cout << "netCap: " << netAllCaps.netCaps[i] << std::endl;
    }
    std::cout << "netCapsSize: " << netAllCaps.netCapsSize << std::endl;
    for (int32_t i = 0; i < netAllCaps.bearerTypesSize; i++) {
        std::cout << "bearType: " << netAllCaps.bearerTypes[i] << std::endl;
    }
    std::cout << "bearerTypesSize: " << netAllCaps.bearerTypesSize << std::endl;
}

/**
 * @tc.name: NativeNetConnTest001
 * @tc.desc: Test OH_NetConn_HasDefaultNet, not applying for
 * permission, return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type FUNC
 */
HWTEST_F(NativeNetConnTest, NativeNetConnTest001, TestSize.Level1)
{
    int32_t flag = 0;
    auto ret = OH_NetConn_HasDefaultNet(&flag);
    ASSERT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: NativeNetConnTest002
 * @tc.desc: Test OH_NetConn_HasDefaultNet, applied for
 * permission, return NETMANAGER_SUCCESS
 * @tc.type FUNC
 */
HWTEST_F(NativeNetConnTest, NativeNetConnTest002, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    int32_t flag = 0;
    int32_t ret = OH_NetConn_HasDefaultNet(&flag);
    std::cout << "has Default Net: " << flag << std::endl;
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NativeNetConnTest003
 * @tc.desc: Test OH_NetConn_HasDefaultNet, giving a nullptr,
 * return NETMANAGER_ERR_INVALID_PARAMETER
 * @tc.type FUNC
 */
HWTEST_F(NativeNetConnTest, NativeNetConnTest003, TestSize.Level1)
{
    int32_t *flag = nullptr;
    int32_t ret = OH_NetConn_HasDefaultNet(flag);
    ASSERT_EQ(ret, NETMANAGER_ERR_INVALID_PARAMETER);
}

/**
 * @tc.name: NativeNetConnTest004
 * @tc.desc: Test OH_NetConn_GetDefaultNet, applied for
 * permission, return NETMANAGER_SUCCESS
 * @tc.type FUNC
 */
HWTEST_F(NativeNetConnTest, NativeNetConnTest004, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    int32_t ret = AddNetwork("ident");
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
    OH_NetConn_NetHandle netHandle = OH_NetConn_NetHandle();
    ret = OH_NetConn_GetDefaultNet(&netHandle);
    std::cout << "DefaultNet ID: " << netHandle.netId << std::endl;
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NativeNetConnTest005
 * @tc.desc: Test OH_NetConn_GetDefaultNet, giving a nullptr,
 * return NETMANAGER_ERR_INVALID_PARAMETER
 * @tc.type FUNC
 */
HWTEST_F(NativeNetConnTest, NativeNetConnTest005, TestSize.Level1)
{
    OH_NetConn_NetHandle *netHandle = nullptr;
    int32_t ret = OH_NetConn_GetDefaultNet(netHandle);
    ASSERT_EQ(ret, NETMANAGER_ERR_INVALID_PARAMETER);
}

/**
 * @tc.name: NativeNetConnTest006
 * @tc.desc: Test OH_NetConn_IsDefaultNetMetered, applied for
 * permission, return NETMANAGER_SUCCESS
 * @tc.type FUNC
 */
HWTEST_F(NativeNetConnTest, NativeNetConnTest006, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    int32_t flag = 0;
    int32_t ret = OH_NetConn_HasDefaultNet(&flag);
    std::cout << "is defaultNet metered: " << flag << std::endl;
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NativeNetConnTest007
 * @tc.desc: Test OH_NetConn_IsDefaultNetMetered, giving a nullptr,
 * return NETMANAGER_ERR_INVALID_PARAMETER
 * @tc.type FUNC
 */
HWTEST_F(NativeNetConnTest, NativeNetConnTest007, TestSize.Level1)
{
    int *flag = nullptr;
    int32_t ret = OH_NetConn_HasDefaultNet(flag);
    ASSERT_EQ(ret, NETMANAGER_ERR_INVALID_PARAMETER);
}

/**
 * @tc.name: NativeNetConnTest008
 * @tc.desc: Test OH_NetConn_GetAllNets, applied for
 * permission, return NETMANAGER_SUCCESS
 * @tc.type FUNC
 */
HWTEST_F(NativeNetConnTest, NativeNetConnTest008, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    OH_NetConn_NetHandleList netHandleList = OH_NetConn_NetHandleList();
    int32_t ret = OH_NetConn_GetAllNets(&netHandleList);
    for (int32_t i = 0; i < netHandleList.netHandleListSize; i++) {
        std::cout << " netId: " << netHandleList.netHandleList[i].netId << std::endl;
    }
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NativeNetConnTest009
 * @tc.desc: Test OH_NetConn_GetAllNets, giving a nullptr,
 * return NETMANAGER_ERR_INVALID_PARAMETER
 * @tc.type FUNC
 */
HWTEST_F(NativeNetConnTest, NativeNetConnTest009, TestSize.Level1)
{
    OH_NetConn_NetHandleList *netHandleList = nullptr;
    int32_t ret = OH_NetConn_GetAllNets(netHandleList);
    ASSERT_EQ(ret, NETMANAGER_ERR_INVALID_PARAMETER);
}

/**
 * @tc.name: NativeNetConnTest010
 * @tc.desc: Test OH_NetConn_GetAllNets, having more networks
 * than struct memory limit, return NETMANAGER_ERR_INTERNAL
 * @tc.type FUNC
 */
HWTEST_F(NativeNetConnTest, NativeNetConnTest010, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    int32_t ret;
    for (int32_t i = 0; i < OH_NETCONN_MAX_NET_SIZE + 1; i++) {
        std::string ident = "ident" + std::to_string(i);
        ret = AddNetwork(ident);
        ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
    }
    OH_NetConn_NetHandleList netHandleList;
    ret = OH_NetConn_GetAllNets(&netHandleList);
    ASSERT_EQ(ret, NETMANAGER_ERR_INTERNAL);
}

/**
 * @tc.name: NativeNetConnTest011
 * @tc.desc: Test OH_NetConn_GetConnectionProperties, applied for
 * permission, return NETMANAGER_SUCCESS
 * @tc.type FUNC
 */
HWTEST_F(NativeNetConnTest, NativeNetConnTest011, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    int32_t ret = AddNetwork("ident");
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
    sptr<NetLinkInfo> netLinkInfo = GetNetLinkInfoSample();
    ret = NetConnClient::GetInstance().UpdateNetLinkInfo(0, netLinkInfo);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);

    OH_NetConn_NetHandle netHandle = OH_NetConn_NetHandle();
    ret = OH_NetConn_GetDefaultNet(&netHandle);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
    OH_NetConn_NetLinkInfo info = OH_NetConn_NetLinkInfo();
    ret = OH_NetConn_GetConnectionProperties(&netHandle, &info);
    LogNetLinkInfo(info);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NativeNetConnTest012
 * @tc.desc: Test OH_NetConn_GetConnectionProperties, giving nullptr(s),
 * return NETMANAGER_ERR_INVALID_PARAMETER
 * @tc.type FUNC
 */
HWTEST_F(NativeNetConnTest, NativeNetConnTest012, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    OH_NetConn_NetHandle *netHandle = nullptr;
    OH_NetConn_NetLinkInfo info = OH_NetConn_NetLinkInfo();
    int32_t ret = OH_NetConn_GetConnectionProperties(netHandle, &info);
    ASSERT_EQ(ret, NETMANAGER_ERR_INVALID_PARAMETER);
}

/**
 * @tc.name: NativeNetConnTest013
 * @tc.desc: Test OH_NetConn_GetConnectionProperties, having more address
 * than struct memory limit, return NETMANAGER_ERR_INTERNAL
 * @tc.type FUNC
 */
HWTEST_F(NativeNetConnTest, NativeNetConnTest013, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    int32_t ret = AddNetwork("ident");
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
    sptr<NetLinkInfo> netLinkInfo = GetNetLinkInfoSample();
    for (int32_t i = 0; i < OH_NETCONN_MAX_ADDR_SIZE + 1; i++) {
        sptr<INetAddr> netAddr = GetINetAddrSample();
        netLinkInfo->netAddrList_.push_back(*netAddr);
    }
    ret = NetConnClient::GetInstance().UpdateNetLinkInfo(0, netLinkInfo);
    ASSERT_EQ(ret, NETMANAGER_ERR_INTERNAL);
    OH_NetConn_NetHandle netHandle = OH_NetConn_NetHandle();
    OH_NetConn_NetLinkInfo info = OH_NetConn_NetLinkInfo();
    ret = OH_NetConn_GetConnectionProperties(&netHandle, &info);
    ASSERT_EQ(ret, NETMANAGER_ERR_INTERNAL);
}

/**
 * @tc.name: NativeNetConnTest014
 * @tc.desc: Test OH_NetConn_GetConnectionProperties, having more route
 * than struct memory limit, return NETMANAGER_ERR_INTERNAL
 * @tc.type FUNC
 */
HWTEST_F(NativeNetConnTest, NativeNetConnTest014, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    int32_t ret = AddNetwork("ident");
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
    sptr<NetLinkInfo> netLinkInfo = GetNetLinkInfoSample();
    for (int32_t i = 0; i < OH_NETCONN_MAX_ROUTE_SIZE + 1; i++) {
        sptr<Route> route = GetRouteSample();
        netLinkInfo->routeList_.push_back(*route);
    }
    ret = NetConnClient::GetInstance().UpdateNetLinkInfo(0, netLinkInfo);
    ASSERT_EQ(ret, NETMANAGER_ERR_INTERNAL);
    OH_NetConn_NetHandle netHandle = OH_NetConn_NetHandle();
    OH_NetConn_NetLinkInfo info = OH_NetConn_NetLinkInfo();
    ret = OH_NetConn_GetConnectionProperties(&netHandle, &info);
    ASSERT_EQ(ret, NETMANAGER_ERR_INTERNAL);
}

/**
 * @tc.name: NativeNetConnTest015
 * @tc.desc: Test OH_NetConn_GetConnectionProperties, httpProxy having more
 * exclusion than struct memory limit, return NETMANAGER_ERR_INTERNAL
 * @tc.type FUNC
 */
HWTEST_F(NativeNetConnTest, NativeNetConnTest015, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    int32_t ret = AddNetwork("ident");
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
    sptr<NetLinkInfo> netLinkInfo = GetNetLinkInfoSample();
    std::list<std::string> exclusionList;
    for (int32_t i = 0; i < OH_NETCONN_MAX_EXCLUSION_SIZE + 1; i++) {
        exclusionList.push_back("testExclusion");
    }
    netLinkInfo->httpProxy_.SetExclusionList(exclusionList);
    ret = NetConnClient::GetInstance().UpdateNetLinkInfo(0, netLinkInfo);
    ASSERT_EQ(ret, NETMANAGER_ERR_INTERNAL);
    OH_NetConn_NetHandle netHandle = OH_NetConn_NetHandle();
    OH_NetConn_NetLinkInfo info = OH_NetConn_NetLinkInfo();
    ret = OH_NetConn_GetConnectionProperties(&netHandle, &info);
    ASSERT_EQ(ret, NETMANAGER_ERR_INTERNAL);
}

/**
 * @tc.name: NativeNetConnTest016
 * @tc.desc: Test OH_NetConn_GetNetCapabilities, applied for
 * permission, return NETMANAGER_SUCCESS
 * @tc.type FUNC
 */
HWTEST_F(NativeNetConnTest, NativeNetConnTest016, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    int32_t ret = AddNetwork("ident");
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
    OH_NetConn_NetHandle netHandle = OH_NetConn_NetHandle();
    OH_NetConn_NetAllCapabilities netAllCaps = OH_NetConn_NetAllCapabilities();
    ret = OH_NetConn_GetNetCapabilities(&netHandle, &netAllCaps);
    LogNetAllCapabilities(netAllCaps);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NativeNetConnTest017
 * @tc.desc: Test OH_NetConn_GetNetCapabilities, giving nullptr(s),
 * return NETMANAGER_ERR_INVALID_PARAMETER
 * @tc.type FUNC
 */
HWTEST_F(NativeNetConnTest, NativeNetConnTest017, TestSize.Level1)
{
    OH_NetConn_NetHandle *netHandle = nullptr;
    OH_NetConn_NetAllCapabilities netAllCaps = OH_NetConn_NetAllCapabilities();
    int32_t ret = OH_NetConn_GetNetCapabilities(netHandle, &netAllCaps);
    ASSERT_TRUE(ret == NETMANAGER_ERR_INVALID_PARAMETER);
}

/**
 * @tc.name: NativeNetConnTest018
 * @tc.desc: Test OH_NetConn_GetDefaultHttpProxy, giving nullptr(s),
 * return NETMANAGER_ERR_INVALID_PARAMETER
 * @tc.type FUNC
 */
HWTEST_F(NativeNetConnTest, NativeNetConnTest018, TestSize.Level1)
{
    OH_NetConn_HttpProxy *httpProxy = nullptr;
    int32_t ret = OH_NetConn_GetDefaultHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NETMANAGER_ERR_INVALID_PARAMETER);
}

/**
 * @tc.name: NativeNetConnTest019
 * @tc.desc: Test OH_NetConn_GetDefaultHttpProxy, return NETMANAGER_SUCCESS
 * @tc.type FUNC
 */
HWTEST_F(NativeNetConnTest, NativeNetConnTest019, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    HttpProxy validHttpProxy = {HOST_NAME, PORT, {}};
    int32_t ret = NetConnClient::GetInstance().SetGlobalHttpProxy(validHttpProxy);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
    OH_NetConn_HttpProxy httpProxy = OH_NetConn_HttpProxy();
    ret = OH_NetConn_GetDefaultHttpProxy(&httpProxy);
    LogHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NativeNetConnTest020
 * @tc.desc: Test OH_NetConn_GetDefaultHttpProxy, string length more
 * than struct char array limit, return NETMANAGER_ERR_INTERNAL
 * @tc.type FUNC
 */
HWTEST_F(NativeNetConnTest, NativeNetConnTest020, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    HttpProxy validHttpProxy = {LONG_HOST, PORT, {}};
    int32_t ret = NetConnClient::GetInstance().SetGlobalHttpProxy(validHttpProxy);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
    OH_NetConn_HttpProxy httpProxy = OH_NetConn_HttpProxy();
    ret = OH_NetConn_GetDefaultHttpProxy(&httpProxy);
    ASSERT_TRUE(ret == NETMANAGER_ERR_INTERNAL);
}

} // namespace NetManagerStandard
} // namespace OHOS
