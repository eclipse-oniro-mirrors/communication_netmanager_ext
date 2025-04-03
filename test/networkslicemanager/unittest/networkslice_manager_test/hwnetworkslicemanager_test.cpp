/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include <map>
#include <mutex>
#include <set>
#include <vector>
#include <gtest/gtest.h>
#include "networkslicecommconfig.h"
#include "allowednssaiconfig.h"
#include "networksliceutil.h"
#include "urspconfig.h"
#include "nrunsolicitedmsgparser.h"
#include "networkslicemanager.h"
#include "networkslice_service.h"
#include "networkslice_client.h"
#include "hwnetworkslicemanager.h"
 
namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
const std::string REQUEST_NETWORK_SLICE_UID = "uid";
const std::string REQUEST_NETWORK_SLICE_FQDN = "fqdn";
const std::string REQUEST_NETWORK_SLICE_DNN = "dnn";
const std::string REQUEST_NETWORK_SLICE_IP = "ip";
const std::string REQUEST_NETWORK_SLICE_PROTOCOL = "protocolId";
const std::string REQUEST_NETWORK_SLICE_REMOTE_PORT = "remotePort";
class HwNetworkSliceManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};
 
void HwNetworkSliceManagerTest::SetUpTestCase() {}
 
void HwNetworkSliceManagerTest::TearDownTestCase() {}
 
void HwNetworkSliceManagerTest::SetUp() {}
 
void HwNetworkSliceManagerTest::TearDown() {}
 
HWTEST_F(HwNetworkSliceManagerTest, Init001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("Init001");
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->Init();
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, HandleUrspChanged001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleUrspChanged001");
    std::map<std::string, std::string> data;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->HandleUrspChanged(data);
    EXPECT_EQ(sUrspConfig_, nullptr);
}

HWTEST_F(HwNetworkSliceManagerTest, HandleIpReport001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleIpReport001");
    std::map<std::string, std::string> data;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->HandleIpReport(data);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, HandleIpReport002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleIpReport002");
    std::map<std::string, std::string> data = {
        {REQUEST_NETWORK_SLICE_UID, "123"},
        {REQUEST_NETWORK_SLICE_IP, "192.168.1.1"},
        {REQUEST_NETWORK_SLICE_PROTOCOL, "TCP"},
        {REQUEST_NETWORK_SLICE_REMOTE_PORT, "8080"}
    };
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->HandleIpReport(data);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, TryToActivateSliceForForegroundApp001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("TryToActivateSliceForForegroundApp001");
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->TryToActivateSliceForForegroundApp();
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, RequestNetworkSliceForFqdn001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("RequestNetworkSliceForFqdn001");
    int uid = 0;
    std::string fqdn = "";
    std::list<AddrInfo> addresses;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->RequestNetworkSliceForFqdn(uid, fqdn, addresses);
    EXPECT_EQ(sUrspConfig_, nullptr);
}

HWTEST_F(HwNetworkSliceManagerTest, RequestNetworkSliceForIp001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("RequestNetworkSliceForIp001");
    int uid = 0;
    std::string ip = "";
    std::string protocolId = "";
    std::string remotePort = "";
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->RequestNetworkSliceForIp(uid, ip, protocolId, remotePort);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, RequestNetworkSliceForIp002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("RequestNetworkSliceForIp002");
    int uid = 0;
    std::string ip = "1.1.1.1";
    std::string protocolId = "1";
    std::string remotePort = "2";
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->RequestNetworkSliceForIp(uid, ip, protocolId, remotePort);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, RequestNetworkSlice001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("RequestNetworkSlice001");
    std::shared_ptr<TrafficDescriptorsInfo> td;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->RequestNetworkSlice(td);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, RequestNetworkSlice002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("RequestNetworkSlice002");
    TrafficDescriptorsInfo td1;
    td1.mIpv4Num = 1;
    std::shared_ptr<TrafficDescriptorsInfo> td = std::make_shared<TrafficDescriptorsInfo>(td1);
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->RequestNetworkSlice(td);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, HandleRsdRequestAgain001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleRsdRequestAgain001");
    std::shared_ptr<NetworkSliceInfo> requestAgain;
    std::shared_ptr<TrafficDescriptorsInfo> requestTd;
    std::shared_ptr<TrafficDescriptorsInfo> tdsInUrsp;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->HandleRsdRequestAgain(requestAgain, requestTd, tdsInUrsp);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, HandleRsdRequestAgain002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleRsdRequestAgain002");
    std::shared_ptr<NetworkSliceInfo> requestAgain = std::make_shared<NetworkSliceInfo>();
    std::shared_ptr<TrafficDescriptorsInfo> requestTd = std::make_shared<TrafficDescriptorsInfo>();
    std::shared_ptr<TrafficDescriptorsInfo> tdsInUrsp;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->HandleRsdRequestAgain(requestAgain, requestTd, tdsInUrsp);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, HandleRsdRequestAgain003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleRsdRequestAgain003");
    std::shared_ptr<NetworkSliceInfo> requestAgain = std::make_shared<NetworkSliceInfo>();
    std::shared_ptr<TrafficDescriptorsInfo> requestTd = std::make_shared<TrafficDescriptorsInfo>();
    std::shared_ptr<TrafficDescriptorsInfo> tdsInUrsp = std::make_shared<TrafficDescriptorsInfo>();
    tdsInUrsp->mIpv4Num = 1;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->HandleRsdRequestAgain(requestAgain, requestTd, tdsInUrsp);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, isUpToToplimit001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isUpToToplimit001");
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->isUpToToplimit();
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, GetNetworkSliceInfoByParaRsd002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetNetworkSliceInfoByParaRsd002");
    RouteSelectionDescriptorInfo rsd;
    NetworkSliceInfo::ParaType type = NetworkSliceInfo::ParaType(0);
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->GetNetworkSliceInfoByParaRsd(rsd, type);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, GetNetworkSliceInfoByParaNull001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetNetworkSliceInfoByParaNull001");
    NetworkSliceInfo::ParaType type = NetworkSliceInfo::ParaType(0);
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->GetNetworkSliceInfoByParaNull(type);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, GetNetworkSliceInfoByParaNetCap001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetNetworkSliceInfoByParaNetCap001");
    NetCap netCap = NET_CAPABILITY_MMS;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->GetNetworkSliceInfoByParaNetCap(netCap);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, HandleMultipleUrspFirstBind001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleMultipleUrspFirstBind001");
    std::shared_ptr<NetworkSliceInfo> requestAgain;
    std::shared_ptr<TrafficDescriptorsInfo> requestTd;
    std::shared_ptr<TrafficDescriptorsInfo> tdsInUrsp;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->HandleMultipleUrspFirstBind(
        requestAgain, requestTd, tdsInUrsp);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, HandleMultipleUrspFirstBind002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleMultipleUrspFirstBind002");
    std::shared_ptr<NetworkSliceInfo> requestAgain = std::make_shared<NetworkSliceInfo>();
    std::shared_ptr<TrafficDescriptorsInfo> requestTd = std::make_shared<TrafficDescriptorsInfo>();
    std::shared_ptr<TrafficDescriptorsInfo> tdsInUrsp = std::make_shared<TrafficDescriptorsInfo>();
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->HandleMultipleUrspFirstBind(
        requestAgain, requestTd, tdsInUrsp);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, HandleInvalidNetwork001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleInvalidNetwork001");
    std::shared_ptr<NetworkSliceInfo> requestAgain;
    std::shared_ptr<TrafficDescriptorsInfo> requestTd;
    std::shared_ptr<TrafficDescriptorsInfo> tdsInUrsp;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->HandleInvalidNetwork(requestAgain, requestTd, tdsInUrsp);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, HandleInvalidNetwork002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleInvalidNetwork002");
    std::shared_ptr<NetworkSliceInfo> requestAgain = std::make_shared<NetworkSliceInfo>();
    std::shared_ptr<TrafficDescriptorsInfo> requestTd = std::make_shared<TrafficDescriptorsInfo>();
    std::shared_ptr<TrafficDescriptorsInfo> tdsInUrsp = std::make_shared<TrafficDescriptorsInfo>();
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->HandleInvalidNetwork(requestAgain, requestTd, tdsInUrsp);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, TryAddSignedUid001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("TryAddSignedUid001");
    int uid = 0;
    TrafficDescriptorsInfo tds;
    std::shared_ptr<NetworkSliceInfo> nsi;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->TryAddSignedUid(uid, tds, nsi);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, TryAddSignedUid002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("TryAddSignedUid002");
    int uid = 0;
    TrafficDescriptorsInfo tds;
    std::shared_ptr<NetworkSliceInfo> nsi = std::make_shared<NetworkSliceInfo>();
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->TryAddSignedUid(uid, tds, nsi);
    EXPECT_EQ(sUrspConfig_, nullptr);
}

HWTEST_F(HwNetworkSliceManagerTest, BindNetworkSliceProcessToNetwork001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("BindNetworkSliceProcessToNetwork001");
    int uid = 0;
    std::set<int> triggerActivationUids;
    std::shared_ptr<NetworkSliceInfo> nsi;
    std::shared_ptr<FqdnIps> fqdnIps;
    std::shared_ptr<TrafficDescriptorsInfo> tds = std::make_shared<TrafficDescriptorsInfo>();
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->BindNetworkSliceProcessToNetwork(
        uid, triggerActivationUids, nsi, fqdnIps, tds);
    EXPECT_EQ(sUrspConfig_, nullptr);
}

HWTEST_F(HwNetworkSliceManagerTest, BindNetworkSliceProcessToNetwork002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("BindNetworkSliceProcessToNetwork002");
    std::set<int> triggerActivationUids;
    NetworkSliceInfo Nsi;
    Nsi.setNetId(123);
    std::shared_ptr<NetworkSliceInfo> nsi = std::make_shared<NetworkSliceInfo>(Nsi);
    std::shared_ptr<FqdnIps> fqdnIps;
    uint8_t num = 123;
    uint8_t bitmap = 123;
    int uid = 123;
    std::string dnn = "123";
    std::string fqdn = "123";
    INetAddr ip;
    ip.port_ = 0;
    std::string protocolId = "123";
    std::string remotePort = "123";
    int cct = 123;
    std::string appIds = "123";
    TrafficDescriptorsInfo td = TrafficDescriptorsInfo::Builder().setIpv4Num(num).setRouteBitmap(
        bitmap).setUid(uid).setDnn(dnn).setFqdn(fqdn).setIp(ip).setProtocolId(protocolId).setRemotePort(
        remotePort).setCct(cct).setAppIds(appIds).build();
    std::shared_ptr<TrafficDescriptorsInfo> tds = std::make_shared<TrafficDescriptorsInfo>(td);
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->BindNetworkSliceProcessToNetwork(uid,
        triggerActivationUids, nsi, fqdnIps, tds);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, BindNetworkSliceProcessToNetwork005, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("BindNetworkSliceProcessToNetwork005");
    int uid = 0;
    std::set<int> triggerActivationUids;
    NetworkSliceInfo Nsi;
    Nsi.setNetId(123);
    std::shared_ptr<NetworkSliceInfo> nsi = std::make_shared<NetworkSliceInfo>(Nsi);
    std::shared_ptr<FqdnIps> fqdnIps;
    std::shared_ptr<TrafficDescriptorsInfo> tds = std::make_shared<TrafficDescriptorsInfo>();
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->BindNetworkSliceProcessToNetwork(
        uid, triggerActivationUids, nsi, fqdnIps, tds);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, BindNetworkSliceProcessToNetworkForRequestAgain001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("BindNetworkSliceProcessToNetworkForRequestAgain001");
    int uid = 0;
    std::shared_ptr<NetworkSliceInfo> nsi;
    std::shared_ptr<FqdnIps> fqdnIps;
    std::shared_ptr<TrafficDescriptorsInfo> tds;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->BindNetworkSliceProcessToNetworkForRequestAgain(
        uid, nsi, fqdnIps, tds);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, FillUidBindParasForRequestAgain001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("FillUidBindParasForRequestAgain001");
    std::map<std::string, std::string> bindParas;
    bindParas["key"] = "value";
    int uid = 0;
    std::shared_ptr<NetworkSliceInfo> nsi = std::make_shared<NetworkSliceInfo>();
    std::shared_ptr<TrafficDescriptorsInfo> tds = std::make_shared<TrafficDescriptorsInfo>();
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->FillUidBindParasForRequestAgain(bindParas, uid, nsi, tds);
    EXPECT_EQ(sUrspConfig_, nullptr);
}

HWTEST_F(HwNetworkSliceManagerTest, FillIpBindParas001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("FillIpBindParas001");
    std::map<std::string, std::string> bindParas;
    bindParas["key"] = "value";
    std::shared_ptr<TrafficDescriptorsInfo> tds = std::make_shared<TrafficDescriptorsInfo>();
    std::shared_ptr<FqdnIps> fqdnIps = nullptr;
    std::shared_ptr<NetworkSliceInfo> nsi = std::make_shared<NetworkSliceInfo>();
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->FillIpBindParas(bindParas, tds, fqdnIps, nsi);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, FillNetworkSliceRequest001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("FillNetworkSliceRequest001");
    int uid = 123;
    std::string dnn = "123";
    std::string fqdn = "123";
    INetAddr ip;
    ip.port_ = 0;
    std::string protocolId = "123";
    std::string remotePort = "123";
    int cct = 123;
    TrafficDescriptorsInfo td = TrafficDescriptorsInfo::Builder().setUid(uid).setDnn(dnn).setFqdn(fqdn).setIp(ip)
        .setProtocolId(protocolId).setRemotePort(remotePort).setCct(cct).build();
    std::shared_ptr<TrafficDescriptorsInfo> tdinfo = std::make_shared<TrafficDescriptorsInfo>(td);
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->FillNetworkSliceRequest(tdinfo);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, FillIpBindParasForFqdn001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("FillIpBindParasForFqdn001");
    std::map<std::string, std::string> bindParas;
    bindParas["key"] = "value";
    FqdnIps newFqdnIps;
    newFqdnIps.setIpv4AddrAndMask();
    newFqdnIps.setIpv6AddrAndPrefix();
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->FillIpBindParasForFqdn(bindParas, newFqdnIps);
    EXPECT_EQ(sUrspConfig_, nullptr);
}

HWTEST_F(HwNetworkSliceManagerTest, FillIpBindParasForFqdn002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("FillIpBindParasForFqdn001");
    std::map<std::string, std::string> bindParas;
    bindParas["key"] = "value";
    FqdnIps newFqdnIps;
    INetAddr ipv4;
    ipv4.address_ = "0.0.0.0";
    std::set<INetAddr> ipv4Addr;
    ipv4Addr.insert(ipv4);
    newFqdnIps.setIpv4Addr(ipv4Addr);
    INetAddr ipv6;
    ipv6.address_ = "2000:0:0:0:0:0:0:1";
    std::set<INetAddr> ipv6Addr;
    ipv6Addr.insert(ipv6);
    newFqdnIps.setIpv6Addr(ipv6Addr);
    newFqdnIps.setIpv4AddrAndMask();
    newFqdnIps.setIpv6AddrAndPrefix();
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->FillIpBindParasForFqdn(bindParas, newFqdnIps);
    EXPECT_EQ(sUrspConfig_, nullptr);
}

HWTEST_F(HwNetworkSliceManagerTest, FillIpBindParasForIpTriad001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("FillIpBindParasForIpTriad001");
    std::map<std::string, std::string> bindParas;
    bindParas["key"] = "value";
    std::shared_ptr<TrafficDescriptorsInfo> tds = std::make_shared<TrafficDescriptorsInfo>();
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->FillIpBindParasForIpTriad(bindParas, tds);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, GetAutoUids001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetAutoUids001");
    std::shared_ptr<TrafficDescriptorsInfo> tds;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->GetAutoUids(tds);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, GetAutoUids002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetAutoUids002");
    std::shared_ptr<TrafficDescriptorsInfo> tds = std::make_shared<TrafficDescriptorsInfo>();
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->GetAutoUids(tds);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, GetAutoUids004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetAutoUids004");
    int uid = 123;
    std::string dnn = "123";
    std::string fqdn = "123";
    INetAddr ip;
    ip.port_ = 0;
    std::string protocolId = "123";
    std::string remotePort = "123";
    int cct = 123;
    std::string appIds = "123";
    TrafficDescriptorsInfo Tds = TrafficDescriptorsInfo::Builder().setUid(uid).setDnn(dnn).setFqdn(fqdn).setIp(ip)
        .setProtocolId(protocolId).setRemotePort(remotePort).setCct(cct).setAppIds(appIds).build();
    std::shared_ptr<TrafficDescriptorsInfo> tds = std::make_shared<TrafficDescriptorsInfo>(Tds);
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->GetAutoUids(tds);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, RequestNetwork001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("RequestNetwork001");
    int uid = 123;
    std::shared_ptr<NetworkSliceInfo> networkSliceInfo;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->RequestNetwork(uid, networkSliceInfo);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, RequestNetwork2001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("RequestNetwork2001");
    int uid = 123;
    int requestId = 123;
    int timeoutMs = 123;
    std::shared_ptr<NetworkSliceInfo> networkSliceInfo;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->RequestNetwork(uid, networkSliceInfo, requestId, timeoutMs);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, RequestNetwork2002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("RequestNetwork2002");
    int uid = 123;
    int requestId = 123;
    int timeoutMs = 123;
    auto networkSliceInfo = std::make_shared<NetworkSliceInfo>();
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->RequestNetwork(uid, networkSliceInfo, requestId, timeoutMs);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, RequestNetwork2003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("RequestNetwork2003");
    int uid = 123;
    int requestId = 123;
    int timeoutMs = 123;
    auto networkSliceInfo = std::make_shared<NetworkSliceInfo>();
    RouteSelectionDescriptorInfo routeSelectionDescriptor;
    TrafficDescriptorsInfo trafficDescriptorsInfo;
    networkSliceInfo->setRouteSelectionDescriptor(routeSelectionDescriptor);
    networkSliceInfo->setTempTrafficDescriptors(trafficDescriptorsInfo);
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->RequestNetwork(uid, networkSliceInfo, requestId, timeoutMs);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, RequestNetwork2004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("RequestNetwork2004");
    int uid = 123;
    int requestId = 123;
    int timeoutMs = 123;
    auto networkSliceInfo = std::make_shared<NetworkSliceInfo>();
    RouteSelectionDescriptorInfo routeSelectionDescriptor;
    TrafficDescriptorsInfo tds;
    sptr<NetSpecifier> networkRequest = new NetSpecifier();
    networkSliceInfo->setRouteSelectionDescriptor(routeSelectionDescriptor);
    networkSliceInfo->setTempTrafficDescriptors(tds);
    networkSliceInfo->setNetworkRequest(networkRequest);
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->RequestNetwork(uid, networkSliceInfo, requestId, timeoutMs);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, ChangeNetworkSliceCounter001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("ChangeNetworkSliceCounter001");
    int changeType = 1;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->ChangeNetworkSliceCounter(changeType);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, ChangeNetworkSliceCounter002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("ChangeNetworkSliceCounter002");
    int changeType = 2;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->ChangeNetworkSliceCounter(changeType);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, isEnvironmentReady001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isEnvironmentReady001");
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->isEnvironmentReady();
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, SetUrspAvailable001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("SetUrspAvailable001");
    bool urspAvailable = true;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->SetUrspAvailable(urspAvailable);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, isNeedToRequestSliceForAppIdAuto001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isNeedToRequestSliceForAppIdAuto001");
    std::string appId;
    EXPECT_EQ(DelayedSingleton<HwNetworkSliceManager>::GetInstance()->isNeedToRequestSliceForAppIdAuto(appId), false);
}

HWTEST_F(HwNetworkSliceManagerTest, isNeedToRequestSliceForFqdnAuto001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isNeedToRequestSliceForFqdnAuto001");
    std::string fqdn;
    int uid = 123;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->isNeedToRequestSliceForFqdnAuto(fqdn, uid);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, isNeedToRequestSliceForDnnAuto001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isNeedToRequestSliceForDnnAuto001");
    std::string dnn;
    int uid = 123;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->isNeedToRequestSliceForDnnAuto(dnn, uid);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, isCooperativeAppByUid001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isCooperativeAppByUid001");
    int uid = 123;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->isCooperativeAppByUid(uid);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, ReadAppIdWhiteList001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("ReadAppIdWhiteList001");
    TrafficDescriptorWhiteList whiteList;
    whiteList.osAppIds = "123";
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->ReadAppIdWhiteList(whiteList);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, ReadFqdnWhiteList001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("ReadFqdnWhiteList001");
    TrafficDescriptorWhiteList whiteList;
    whiteList.fqdns = "123";
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->ReadFqdnWhiteList(whiteList);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, ReadCctWhiteList001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("ReadCctWhiteList001");
    TrafficDescriptorWhiteList whiteList;
    whiteList.cct = "123";
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->ReadCctWhiteList(whiteList);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, CleanEnvironment001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("CleanEnvironment001");
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->CleanEnvironment();
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, UnbindSingleNetId001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("UnbindSingleNetId001");
    int netId = 123;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->UnbindSingleNetId(netId);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, UnbindUids001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("UnbindUids001");
    uint netId = 2;
    std::string uids;
    uint8_t urspPrecedence = 2;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->UnbindUids(netId, uids, urspPrecedence);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, BindProcessToNetwork001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("BindProcessToNetwork001");
    std::map<std::string, std::string> bindParas;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->BindProcessToNetwork(bindParas);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, UnbindProcessToNetwork001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("UnbindProcessToNetwork001");
    std::string uids;
    int netId = 123;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->UnbindProcessToNetwork(uids, netId);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, OnNetworkAvailable001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("OnNetworkAvailable001");
    NetCap netCap = NET_CAPABILITY_MMS;
    int32_t netId = 2;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->OnNetworkAvailable(netCap, netId);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, OnNetworkAvailable003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("OnNetworkAvailable003");
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->InitNetworkSliceInfos();
    NetCap netCap = NET_CAPABILITY_SNSSAI1;
    int32_t netId = 2;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->OnNetworkAvailable(netCap, netId);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, OnNetworkLost001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("OnNetworkLost001");
    NetCap netCap = NET_CAPABILITY_MMS;
    int32_t netId = 2;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->OnNetworkLost(netCap, netId);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, OnNetworkLost003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("OnNetworkLost003");
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->InitNetworkSliceInfos();
    NetCap netCap = NET_CAPABILITY_SNSSAI1;
    int32_t netId = 2;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->OnNetworkLost(netCap, netId);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, OnNetworkUnavailable001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("OnNetworkUnavailable001");
    NetCap netCap = NET_CAPABILITY_MMS;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->OnNetworkUnavailable(netCap);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, OnNetworkUnavailable003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("OnNetworkUnavailable003");
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->InitNetworkSliceInfos();
    NetCap netCap = NET_CAPABILITY_SNSSAI1;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->OnNetworkUnavailable(netCap);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, RecoveryNetworkSlice001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("RecoveryNetworkSlice001");
    std::shared_ptr<NetworkSliceInfo> networkSliceInfo;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->RecoveryNetworkSlice(networkSliceInfo);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, RecoveryNetworkSlice002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("RecoveryNetworkSlice002");
    std::shared_ptr<NetworkSliceInfo> networkSliceInfo = std::make_shared<NetworkSliceInfo>();
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->RecoveryNetworkSlice(networkSliceInfo);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, ReleaseNetworkSlice001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("ReleaseNetworkSlice001");
    std::shared_ptr<NetworkSliceInfo> networkSliceInfo;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->ReleaseNetworkSlice(networkSliceInfo);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, ReleaseNetworkSlice002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("ReleaseNetworkSlice002");
    std::shared_ptr<NetworkSliceInfo> networkSliceInfo = std::make_shared<NetworkSliceInfo>();
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->ReleaseNetworkSlice(networkSliceInfo);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, CleanRouteSelectionDescriptor001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("CleanRouteSelectionDescriptor001");
    std::shared_ptr<NetworkSliceInfo> networkSliceInfo;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->CleanRouteSelectionDescriptor(networkSliceInfo);
    EXPECT_EQ(sUrspConfig_, nullptr);
}

HWTEST_F(HwNetworkSliceManagerTest, OnNetworkAvailable002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("OnNetworkAvailable002");
    std::map<std::string, std::string> data;
    data["routeBitmap"] = "1";
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->HandleUrspChanged(data);
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->InitNetworkSliceInfos();
    NetCap netCap = NET_CAPABILITY_SNSSAI1;
    int32_t netId = 2;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->OnNetworkAvailable(netCap, netId);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, OnNetworkLost002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("OnNetworkLost002");
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->InitNetworkSliceInfos();
    NetCap netCap = NET_CAPABILITY_SNSSAI1;
    int32_t netId = 2;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->OnNetworkLost(netCap, netId);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, OnNetworkUnavailable002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("OnNetworkUnavailable002");
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->InitNetworkSliceInfos();
    NetCap netCap = NET_CAPABILITY_SNSSAI1;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->OnNetworkUnavailable(netCap);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, BindNetworkSliceProcessToNetworkForRequestAgain002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("BindNetworkSliceProcessToNetworkForRequestAgain002");
    NetworkSliceInfo Nsi;
    Nsi.setNetId(123);
    std::shared_ptr<NetworkSliceInfo> nsi = std::make_shared<NetworkSliceInfo>(Nsi);
    std::shared_ptr<FqdnIps> fqdnIps;
    uint8_t num = 123;
    uint8_t bitmap = 123;
    int uid = 123;
    std::string dnn = "123";
    std::string fqdn = "123";
    INetAddr ip;
    ip.port_ = 0;
    std::string protocolId = "123";
    std::string remotePort = "123";
    int cct = 123;
    std::string appIds = "123";
    TrafficDescriptorsInfo td = TrafficDescriptorsInfo::Builder().setIpv4Num(num).setRouteBitmap(bitmap).setUid(uid)
        .setDnn(dnn).setFqdn(fqdn).setIp(ip).setProtocolId(protocolId).setRemotePort(remotePort).setCct(cct)
        .setAppIds(appIds).build();
    std::shared_ptr<TrafficDescriptorsInfo> tds = std::make_shared<TrafficDescriptorsInfo>(td);
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->BindNetworkSliceProcessToNetworkForRequestAgain(
        uid, nsi, fqdnIps, tds);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, BindNetworkSliceProcessToNetworkForRequestAgain005, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("BindNetworkSliceProcessToNetworkForRequestAgain005");
    NetworkSliceInfo Nsi;
    Nsi.setNetId(123);
    std::shared_ptr<NetworkSliceInfo> nsi = std::make_shared<NetworkSliceInfo>(Nsi);
    std::shared_ptr<FqdnIps> fqdnIps;
    uint8_t num = 123;
    uint8_t bitmap = 123;
    int uid = 123;
    std::string dnn = "123";
    std::string fqdn = "123";
    INetAddr ip;
    ip.port_ = 0;
    std::string protocolId = "123";
    std::string remotePort = "123";
    int cct = 123;
    TrafficDescriptorsInfo td = TrafficDescriptorsInfo::Builder().setIpv4Num(num).setRouteBitmap(bitmap).setUid(uid)
        .setDnn(dnn).setFqdn(fqdn).setIp(ip).setProtocolId(protocolId).setRemotePort(remotePort).setCct(cct).build();
    std::shared_ptr<TrafficDescriptorsInfo> tds = std::make_shared<TrafficDescriptorsInfo>(td);
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->BindNetworkSliceProcessToNetworkForRequestAgain(
        uid, nsi, fqdnIps, tds);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, UnbindProcessToNetworkForSingleUid001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("UnbindProcessToNetworkForSingleUid001");
    int uid = 123;
    std::shared_ptr<NetworkSliceInfo> nsi;
    bool isNeedToRemoveUid = true;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->
        UnbindProcessToNetworkForSingleUid(uid, nsi, isNeedToRemoveUid);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, UnbindProcessToNetworkForSingleUid002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("UnbindProcessToNetworkForSingleUid002");
    int uid = 123;
    std::shared_ptr<NetworkSliceInfo> nsi = std::make_shared<NetworkSliceInfo>();
    bool isNeedToRemoveUid = true;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->
        UnbindProcessToNetworkForSingleUid(uid, nsi, isNeedToRemoveUid);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, HandleUidRemoved001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleUidRemoved001");
    std::string packageName;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->HandleUidRemoved(packageName);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, HandleUidGone001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleUidGone001");
    int uid = 123;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->HandleUidGone(uid);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, ReleaseNetworkSliceByApp001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("ReleaseNetworkSliceByApp001");
    int32_t uid = 123;
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->ReleaseNetworkSliceByApp(uid);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, BindAllProccessToNetwork001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("BindAllProccessToNetwork001");
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->BindAllProccessToNetwork();
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(HwNetworkSliceManagerTest, DumpNetworkSliceInfos001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DumpNetworkSliceInfos001");
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->DumpNetworkSliceInfos();
    EXPECT_EQ(sUrspConfig_, nullptr);
}

}
}