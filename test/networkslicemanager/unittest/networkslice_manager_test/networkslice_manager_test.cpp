/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include "networkslicecommconfig.h"
#include "allowednssaiconfig.h"
#include "networksliceutil.h"
#include "urspconfig.h"
#include "nrunsolicitedmsgparser.h"
#include "networkslicemanager.h"
#include "networkslice_service.h"
#include "networkslice_client.h"
#include "parameters.h"
#include "networkslice_kernel_proxy.h"
 
namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
 
class NetworkSliceManagerTest : public testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

HWTEST_F(NetworkSliceManagerTest, ProcessEvent001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("ProcessEvent001");
    auto networkSliceManager = DelayedSingleton<NetworkSliceManager>::GetInstance();
    std::shared_ptr<GetSlicePara> getSlicePara = std::make_shared<GetSlicePara>();
    getSlicePara->data["appId"] = "123";
    std::vector<uint8_t> buffer1 = {0x01};
    std::map<std::string, std::string> buffer2;
    buffer2["uids"] = "123";
    std::shared_ptr<std::vector<uint8_t>> msg1 = std::make_shared<std::vector<uint8_t>>(buffer1);
    std::shared_ptr<std::map<std::string, std::string>> msg2 =
        std::make_shared<std::map<std::string, std::string>>(buffer2);
    AppExecFwk::InnerEvent::Pointer event1 = AppExecFwk::InnerEvent::Get(
        NetworkSliceEvent::EVENT_GET_SLICE_PARA, getSlicePara);
    AppExecFwk::InnerEvent::Pointer event2 = AppExecFwk::InnerEvent::Get(
        NetworkSliceEvent::EVENT_HANDLE_ALLOWED_NSSAI, msg1);
    AppExecFwk::InnerEvent::Pointer event3 = AppExecFwk::InnerEvent::Get(
        NetworkSliceEvent::EVENT_HANDLE_UE_POLICY, msg1);
    AppExecFwk::InnerEvent::Pointer event4 = AppExecFwk::InnerEvent::Get(
        NetworkSliceEvent::EVENT_KERNEL_IP_ADDR_REPORT, msg1);
    AppExecFwk::InnerEvent::Pointer event5 = AppExecFwk::InnerEvent::Get(
        NetworkSliceEvent::EVENT_BIND_TO_NETWORK, msg2);
    AppExecFwk::InnerEvent::Pointer event6 = AppExecFwk::InnerEvent::Get(
        NetworkSliceEvent::EVENT_DEL_BIND_TO_NETWORK, msg2);
    AppExecFwk::InnerEvent::Pointer event7 = AppExecFwk::InnerEvent::Get(
        NetworkSliceEvent::EVENT_HANDLE_EHPLMN, msg1);
    networkSliceManager->ProcessEvent(event1);
    networkSliceManager->ProcessEvent(event2);
    networkSliceManager->ProcessEvent(event3);
    networkSliceManager->ProcessEvent(event4);
    networkSliceManager->ProcessEvent(event5);
    networkSliceManager->ProcessEvent(event6);
    networkSliceManager->ProcessEvent(event7);
    EXPECT_EQ(sUrspConfig_, nullptr);
}

HWTEST_F(NetworkSliceManagerTest, ProcessEvent002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("ProcessEvent002");
    auto networkSliceManager = DelayedSingleton<NetworkSliceManager>::GetInstance();
    bool flag = true;
    std::map<std::string, std::string> buffer;
    std::shared_ptr<std::map<std::string, std::string>> msg =
        std::make_shared<std::map<std::string, std::string>>(buffer);
    AppExecFwk::AppStateData appStateData;
    appStateData.bundleName = "browser";
    std::shared_ptr<AppExecFwk::AppStateData> stateData = std::make_shared<AppExecFwk::AppStateData>(appStateData);
    AppExecFwk::InnerEvent::Pointer event1 = AppExecFwk::InnerEvent::Get(NetworkSliceEvent::EVENT_URSP_CHANGED, msg);
    AppExecFwk::InnerEvent::Pointer event2 = AppExecFwk::InnerEvent::Get(
        NetworkSliceEvent::EVENT_NETWORK_STATE_CHANGED);
    AppExecFwk::InnerEvent::Pointer event3 = AppExecFwk::InnerEvent::Get(NetworkSliceEvent::EVENT_CONNECTIVITY_CHANGE);
    AppExecFwk::InnerEvent::Pointer event4 = AppExecFwk::InnerEvent::Get(
        NetworkSliceEvent::EVENT_FOREGROUND_APP_CHANGED, stateData);
    AppExecFwk::InnerEvent::Pointer event5 = AppExecFwk::InnerEvent::Get(
        NetworkSliceEvent::EVENT_AIR_MODE_CHANGED, flag);
    AppExecFwk::InnerEvent::Pointer event6 = AppExecFwk::InnerEvent::Get(
        NetworkSliceEvent::EVENT_WIFI_CONN_CHANGED, flag);
    AppExecFwk::InnerEvent::Pointer event7 = AppExecFwk::InnerEvent::Get(
        NetworkSliceEvent::EVENT_VPN_MODE_CHANGED, flag);
    AppExecFwk::InnerEvent::Pointer event8 = AppExecFwk::InnerEvent::Get(NetworkSliceEvent::EVENT_SCREEN_ON);
    AppExecFwk::InnerEvent::Pointer event9 = AppExecFwk::InnerEvent::Get(NetworkSliceEvent::EVENT_SCREEN_OFF);
    AppExecFwk::InnerEvent::Pointer event10 = AppExecFwk::InnerEvent::Get(
        NetworkSliceEvent::EVENT_HANDLE_SIM_STATE_CHANGED);
    networkSliceManager->ProcessEvent(event1);
    networkSliceManager->ProcessEvent(event2);
    networkSliceManager->ProcessEvent(event3);
    networkSliceManager->ProcessEvent(event4);
    networkSliceManager->ProcessEvent(event5);
    networkSliceManager->ProcessEvent(event6);
    networkSliceManager->ProcessEvent(event7);
    networkSliceManager->ProcessEvent(event8);
    networkSliceManager->ProcessEvent(event9);
    networkSliceManager->ProcessEvent(event10);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, HandleForegroundAppChanged001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleForegroundAppChanged001");
    std::shared_ptr<AppExecFwk::AppStateData> msg;
    DelayedSingleton<NetworkSliceManager>::GetInstance()->HandleForegroundAppChanged(msg);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, HandleForegroundAppChanged002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleForegroundAppChanged002");
    auto msg = std::make_shared<AppExecFwk::AppStateData>();
    msg->bundleName = "com.ohos.sceneboard";
    DelayedSingleton<NetworkSliceManager>::GetInstance()->HandleForegroundAppChanged(msg);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, HandleForegroundAppChanged003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleForegroundAppChanged003");
    auto msg = std::make_shared<AppExecFwk::AppStateData>();
    msg->state = 2;
    msg->isFocused = 1;
    DelayedSingleton<NetworkSliceManager>::GetInstance()->HandleForegroundAppChanged(msg);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, HandleForegroundAppChanged004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleForegroundAppChanged004");
    auto msg = std::make_shared<AppExecFwk::AppStateData>();
    msg->state = 4;
    DelayedSingleton<NetworkSliceManager>::GetInstance()->HandleForegroundAppChanged(msg);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, HandleForegroundAppChanged005, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleForegroundAppChanged005");
    auto msg = std::make_shared<AppExecFwk::AppStateData>();
    msg->state = 5;
    DelayedSingleton<NetworkSliceManager>::GetInstance()->HandleForegroundAppChanged(msg);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, HandleUrspChanged001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleUrspChanged001");
    std::shared_ptr<std::map<std::string, std::string>> msg;
    DelayedSingleton<NetworkSliceManager>::GetInstance()->HandleUrspChanged(msg);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, HandleUrspChanged002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleUrspChanged002");
    std::map<std::string, std::string> data;
    data["key"] = "value";
    std::shared_ptr<std::map<std::string, std::string>> msg =
        std::make_shared<std::map<std::string, std::string>>(data);
    DelayedSingleton<NetworkSliceManager>::GetInstance()->HandleUrspChanged(msg);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, HandleAirModeChanged001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleAirModeChanged001");
    int32_t mode = 0;
    DelayedSingleton<NetworkSliceManager>::GetInstance()->HandleAirModeChanged(mode);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, HandleAirModeChanged002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleAirModeChanged002");
    int32_t mode = 1;
    DelayedSingleton<NetworkSliceManager>::GetInstance()->HandleAirModeChanged(mode);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, HandleWifiConnChanged001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleWifiConnChanged001");
    int32_t state = 4;
    DelayedSingleton<NetworkSliceManager>::GetInstance()->HandleWifiConnChanged(state);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, HandleWifiConnChanged002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleWifiConnChanged002");
    int32_t state = 6;
    DelayedSingleton<NetworkSliceManager>::GetInstance()->HandleWifiConnChanged(state);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, SendUrspUpdateMsg001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("SendUrspUpdateMsg001");
    DelayedSingleton<NetworkSliceManager>::GetInstance()->SendUrspUpdateMsg();
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, HandleSimStateChanged001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleSimStateChanged001");
    DelayedSingleton<NetworkSliceManager>::GetInstance()->HandleSimStateChanged();
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, HandleAllowedNssaiFromUnsolData001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleAllowedNssaiFromUnsolData001");
    auto networkslicemanager = std::make_shared<NetworkSliceManager>();
    std::vector<uint8_t> msgData = {};
    std::shared_ptr<std::vector<uint8_t>> msg;
    networkslicemanager->HandleAllowedNssaiFromUnsolData(msg);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, HandleAllowedNssaiFromUnsolData002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleAllowedNssaiFromUnsolData002");
    auto networkslicemanager = std::make_shared<NetworkSliceManager>();
    std::vector<uint8_t> msgData = {};
    std::shared_ptr<std::vector<uint8_t>> msg = std::make_shared<std::vector<uint8_t>>(msgData);
    networkslicemanager->HandleAllowedNssaiFromUnsolData(msg);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, HandleAllowedNssaiFromUnsolData003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleAllowedNssaiFromUnsolData003");
    auto networkslicemanager = std::make_shared<NetworkSliceManager>();
    std::vector<uint8_t> msgData = {0x41, 0x2E, 0x42, 0x3A, 0x43, 0x2E, 0x44};
    std::shared_ptr<std::vector<uint8_t>> msg = std::make_shared<std::vector<uint8_t>>(msgData);
    networkslicemanager->HandleAllowedNssaiFromUnsolData(msg);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, HandleUrspFromUnsolData001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleUrspFromUnsolData001");
    auto networkslicemanager = std::make_shared<NetworkSliceManager>();
    std::vector<uint8_t> msgData = {};
    std::shared_ptr<std::vector<uint8_t>> msg;
    networkslicemanager->HandleUrspFromUnsolData(msg);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, HandleUrspFromUnsolData002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleUrspFromUnsolData002");
    auto networkslicemanager = std::make_shared<NetworkSliceManager>();
    std::vector<uint8_t> msgData = {};
    std::shared_ptr<std::vector<uint8_t>> msg = std::make_shared<std::vector<uint8_t>>(msgData);
    networkslicemanager->HandleUrspFromUnsolData(msg);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, HandleUrspFromUnsolData003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleUrspFromUnsolData003");
    std::vector<uint8_t> msgData = {0x01, 0x01, 0x01, 0xE6, 0x05, 0x01, 0x01, 0x44, 0x00, 0x42, 0x00, 0x00, 0x01,
    0x01, 0x3D, 0x00, 0x09, 0x03, 0x39, 0x00, 0x01, 0x37, 0x02, 0x20, 0x00, 0x08, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x08, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x0E, 0x63, 0x6F, 0x6D, 0x2E, 0x74, 0x65, 0x6E, 0x63,
    0x65, 0x6E, 0x74, 0x2E, 0x6D, 0x6D, 0x12, 0x00, 0x11, 0x03, 0x0E, 0x00, 0x01, 0x01, 0x04, 0x06, 0x05, 0x63, 0x6D,
    0x6E, 0x65, 0x74, 0x02, 0x02, 0x0A, 0x00};
    std::shared_ptr<std::vector<uint8_t>> msg = std::make_shared<std::vector<uint8_t>>(msgData);
    auto networkslicemanager = std::make_shared<NetworkSliceManager>();
    networkslicemanager->HandleUrspFromUnsolData(msg);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, HandleEhplmnFromUnsolData001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleEhplmnFromUnsolData001");
    std::shared_ptr<std::vector<uint8_t>> msg;
    DelayedSingleton<NetworkSliceManager>::GetInstance()->HandleEhplmnFromUnsolData(msg);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, HandleEhplmnFromUnsolData002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleEhplmnFromUnsolData002");
    std::shared_ptr<std::vector<uint8_t>> msg = std::make_shared<std::vector<uint8_t>>();
    DelayedSingleton<NetworkSliceManager>::GetInstance()->HandleEhplmnFromUnsolData(msg);
    EXPECT_EQ(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, InitUePolicy001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("InitUePolicy001");
    DelayedSingleton<NetworkSliceManager>::GetInstance()->InitUePolicy();
    EXPECT_NE(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, HandleIpRpt001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleIpRpt001");
    std::shared_ptr<std::vector<uint8_t>> msg;
    DelayedSingleton<NetworkSliceManager>::GetInstance()->HandleIpRpt(msg);
    EXPECT_NE(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, HandleIpRpt002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleIpRpt002");
    std::vector<uint8_t> Msg = {0x01};
    std::shared_ptr<std::vector<uint8_t>> msg = std::make_shared<std::vector<uint8_t>>(Msg);
    DelayedSingleton<NetworkSliceManager>::GetInstance()->HandleIpRpt(msg);
    EXPECT_NE(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, HandleIpRpt003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleIpRpt003");
    std::vector<uint8_t> Msg = {0x01, 0x02};
    std::shared_ptr<std::vector<uint8_t>> msg = std::make_shared<std::vector<uint8_t>>(Msg);
    DelayedSingleton<NetworkSliceManager>::GetInstance()->HandleIpRpt(msg);
    EXPECT_NE(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, HandleIpRpt004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleIpRpt004");
    std::vector<uint8_t> Msg = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    std::shared_ptr<std::vector<uint8_t>> msg = std::make_shared<std::vector<uint8_t>>(Msg);
    DelayedSingleton<NetworkSliceManager>::GetInstance()->HandleIpRpt(msg);
    EXPECT_NE(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, HandleIpRpt005, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleIpRpt005");
    std::vector<uint8_t> Msg = {0x01, 0x02, 0x0f, 0x00, 0x05, 0x06, 0x07, 0x08, 0x09};
    std::shared_ptr<std::vector<uint8_t>> msg = std::make_shared<std::vector<uint8_t>>(Msg);
    DelayedSingleton<NetworkSliceManager>::GetInstance()->HandleIpRpt(msg);
    EXPECT_NE(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, HandleIpRpt006, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleIpRpt006");
    std::vector<uint8_t> Msg = {0x01, 0x02, 0x1b, 0x00, 0x05, 0x06, 0x07, 0x08, 0x09};
    std::shared_ptr<std::vector<uint8_t>> msg = std::make_shared<std::vector<uint8_t>>(Msg);
    DelayedSingleton<NetworkSliceManager>::GetInstance()->HandleIpRpt(msg);
    EXPECT_NE(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, HandleIpRpt007, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleIpRpt007");
    std::vector<uint8_t> Msg = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
    std::shared_ptr<std::vector<uint8_t>> msg = std::make_shared<std::vector<uint8_t>>(Msg);
    DelayedSingleton<NetworkSliceManager>::GetInstance()->HandleIpRpt(msg);
    EXPECT_NE(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, BindProcessToNetworkByFullPara001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("BindProcessToNetworkByFullPara001");
    std::shared_ptr<std::map<std::string, std::string>> msg;
    DelayedSingleton<NetworkSliceManager>::GetInstance()->BindProcessToNetworkByFullPara(msg);
    EXPECT_NE(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, BindProcessToNetworkByFullPara002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("BindProcessToNetworkByFullPara002");
    std::map<std::string, std::string> buffer;
    buffer["uids"] = "123";
    std::shared_ptr<std::map<std::string, std::string>> msg =
        std::make_shared<std::map<std::string, std::string>>(buffer);
    DelayedSingleton<NetworkSliceManager>::GetInstance()->BindProcessToNetworkByFullPara(msg);
    EXPECT_NE(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, GetUidArray001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetUidArray001");
    std::string uids = "123";
    DelayedSingleton<NetworkSliceManager>::GetInstance()->GetUidArray(uids);
    EXPECT_NE(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, GetPrecedenceArray001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetPrecedenceArray001");
    std::string precedences = "123";
    DelayedSingleton<NetworkSliceManager>::GetInstance()->GetPrecedenceArray(precedences);
    EXPECT_NE(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, GetUidRoutePara001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetUidRoutePara001");
    AddRoutePara addRoutePara;
    std::map<std::string, std::string> data;
    data["uids"] = "123";
    EXPECT_TRUE(DelayedSingleton<NetworkSliceManager>::GetInstance()->GetUidRoutePara(addRoutePara, data));
    EXPECT_NE(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, GetRoutePara002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetRoutePara002");
    AddRoutePara addRoutePara;
    std::map<std::string, std::string> data;
    data["ipv4AddrAndMask"] = "123";
    data["ipv6Num"] = "123";
    data["ipv6AddrAndPrefix"] = "123";
    data["protocolIds"] = "123";
    data["remotePorts"] = "123";
    data["netId"] = "123";
    data["urspPrecedence"] = "123";
    EXPECT_FALSE(DelayedSingleton<NetworkSliceManager>::GetInstance()->GetRoutePara(addRoutePara, data));
}
 
HWTEST_F(NetworkSliceManagerTest, GetRoutePara003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetRoutePara003");
    AddRoutePara addRoutePara;
    std::map<std::string, std::string> data;
    data["ipv4AddrAndMask"] = "123";
    data["ipv6AddrAndPrefix"] = "123";
    data["protocolIds"] = "123";
    data["remotePorts"] = "123";
    data["netId"] = "123";
    data["urspPrecedence"] = "123";
    EXPECT_TRUE(DelayedSingleton<NetworkSliceManager>::GetInstance()->GetRoutePara(addRoutePara, data));
}
 
HWTEST_F(NetworkSliceManagerTest, GetRoutePara004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetRoutePara004");
    AddRoutePara addRoutePara;
    addRoutePara.ipv4Num = 123;
    std::map<std::string, std::string> data;
    data["ipv4Num"] = "123";
    EXPECT_FALSE(DelayedSingleton<NetworkSliceManager>::GetInstance()->GetRoutePara(addRoutePara, data));
}
 
HWTEST_F(NetworkSliceManagerTest, GetRoutePara005, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetRoutePara005");
    AddRoutePara addRoutePara;
    addRoutePara.ipv6Num = 123;
    std::map<std::string, std::string> data;
    EXPECT_FALSE(DelayedSingleton<NetworkSliceManager>::GetInstance()->GetRoutePara(addRoutePara, data));
}
 
HWTEST_F(NetworkSliceManagerTest, CalculateParaLen001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("CalculateParaLen001");
    AddRoutePara addRoutePara;
    addRoutePara.uidNum = 65536;
    EXPECT_FALSE(DelayedSingleton<NetworkSliceManager>::GetInstance()->CalculateParaLen(addRoutePara));
}

HWTEST_F(NetworkSliceManagerTest, DeleteNetworkBindByFullPara001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DeleteNetworkBindByFullPara001");
    std::shared_ptr<std::map<std::string, std::string>> msg;
    DelayedSingleton<NetworkSliceManager>::GetInstance()->DeleteNetworkBindByFullPara(msg);
    EXPECT_NE(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, DeleteNetworkBindByFullPara002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DeleteNetworkBindByFullPara002");
    std::shared_ptr<std::map<std::string, std::string>> msg = std::make_shared<std::map<std::string, std::string>>();
    DelayedSingleton<NetworkSliceManager>::GetInstance()->DeleteNetworkBindByFullPara(msg);
    EXPECT_NE(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, DeleteNetworkBindByFullPara003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DeleteNetworkBindByFullPara003");
    std::map<std::string, std::string> data;
    data["uids"] = "123";
    std::shared_ptr<std::map<std::string, std::string>> msg =
        std::make_shared<std::map<std::string, std::string>>(data);
    DelayedSingleton<NetworkSliceManager>::GetInstance()->DeleteNetworkBindByFullPara(msg);
    EXPECT_NE(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, DeleteNetworkBindByFullPara004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DeleteNetworkBindByFullPara004");
    std::map<std::string, std::string> data;
    data["type"] = "0";
    data["uids"] = "123";
    data["netId"] = "123";
    std::shared_ptr<std::map<std::string, std::string>> msg =
        std::make_shared<std::map<std::string, std::string>>(data);
    DelayedSingleton<NetworkSliceManager>::GetInstance()->DeleteNetworkBindByFullPara(msg);
    EXPECT_NE(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, IpParaReportControl001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("IpParaReportControl001");
    DelayedSingleton<NetworkSliceManager>::GetInstance()->IpParaReportControl();
    EXPECT_NE(sUrspConfig_, nullptr);
}

HWTEST_F(NetworkSliceManagerTest, HandleIpv4Rpt001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleIpv4Rpt001");
    int startIndex = 123;
    std::vector<uint8_t> buffer = {};
    std::map<std::string, std::string> bundle;
    AppDescriptor appDescriptor;
    DelayedSingleton<NetworkSliceManager>::GetInstance()->HandleIpv4Rpt(startIndex, buffer, bundle, appDescriptor);
    EXPECT_NE(sUrspConfig_, nullptr);
}

HWTEST_F(NetworkSliceManagerTest, HandleIpv4Rpt002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleIpv4Rpt002");
    int startIndex = 0;
    std::vector<uint8_t> buffer = {0x01, 0x02, 0x03, 0x04};
    std::map<std::string, std::string> bundle;
    AppDescriptor appDescriptor;
    DelayedSingleton<NetworkSliceManager>::GetInstance()->HandleIpv4Rpt(startIndex, buffer, bundle, appDescriptor);
    EXPECT_NE(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, HandleIpv4Rpt003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleIpv4Rpt003");
    int startIndex = 0;
    std::vector<uint8_t> buffer = {0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03};
    std::map<std::string, std::string> bundle;
    AppDescriptor appDescriptor;
    DelayedSingleton<NetworkSliceManager>::GetInstance()->HandleIpv4Rpt(startIndex, buffer, bundle, appDescriptor);
    EXPECT_NE(sUrspConfig_, nullptr);
}

HWTEST_F(NetworkSliceManagerTest, HandleIpv6Rpt001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleIpv6Rpt001");
    int startIndex = 123;
    std::vector<uint8_t> buffer = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
         0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
    std::map<std::string, std::string> bundle;
    AppDescriptor appDescriptor;
    DelayedSingleton<NetworkSliceManager>::GetInstance()->HandleIpv6Rpt(startIndex, buffer, bundle, appDescriptor);
    EXPECT_NE(sUrspConfig_, nullptr);
}

HWTEST_F(NetworkSliceManagerTest, HandleIpv6Rpt002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleIpv6Rpt001");
    int startIndex = 123;
    std::vector<uint8_t> buffer = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
    std::map<std::string, std::string> bundle;
    AppDescriptor appDescriptor;
    DelayedSingleton<NetworkSliceManager>::GetInstance()->HandleIpv6Rpt(startIndex, buffer, bundle, appDescriptor);
    EXPECT_NE(sUrspConfig_, nullptr);
}

HWTEST_F(NetworkSliceManagerTest, SetAppId001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("SetAppId001");
    AppDescriptor appDescriptor;
    std::vector<std::string> values = {"1", "2"};
    std::string appId;
    DelayedSingleton<NetworkSliceManager>::GetInstance()->SetAppId(appDescriptor, values, appId);
    EXPECT_NE(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, SetAppId002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("SetAppId002");
    AppDescriptor appDescriptor;
    std::vector<std::string> values = {"1"};
    std::string appId;
    DelayedSingleton<NetworkSliceManager>::GetInstance()->SetAppId(appDescriptor, values, appId);
    EXPECT_NE(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, GetAppDescriptor001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetAppDescriptor001");
    std::map<std::string, std::string> data;
    AppDescriptor appDescriptor;
    EXPECT_FALSE(DelayedSingleton<NetworkSliceManager>::GetInstance()->GetAppDescriptor(data, appDescriptor));
    EXPECT_NE(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, GetAppDescriptor002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetAppDescriptor002");
    std::map<std::string, std::string> data;
    data["appId"] = "123";
    data["dnn"] = "123";
    data["fqdn"] = "123";
    data["ip"] = "123";
    data["protocolId"] = "123";
    data["remotePort"] = "123";
    AppDescriptor appDescriptor;
    DelayedSingleton<NetworkSliceManager>::GetInstance()->GetAppDescriptor(data, appDescriptor);
    EXPECT_NE(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, FillRouteSelectionDescriptor001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("FillRouteSelectionDescriptor001");
    std::map<std::string, std::string> ret;
    SelectedRouteDescriptor routeRule;
    routeRule.setPduSessionType(0);
    std::string str = "123";
    uint8_t ipv4Num = 123;
    routeRule.setDnn(str);
    routeRule.setAppIds(str);
    routeRule.setProtocolIds(str);
    routeRule.setRemotePorts(str);
    routeRule.setIpv4Num(123);
    routeRule.setIpv6Num(123);
    DelayedSingleton<NetworkSliceManager>::GetInstance()->FillRouteSelectionDescriptor(ret, routeRule);
    EXPECT_NE(sUrspConfig_, nullptr);
}
 
HWTEST_F(NetworkSliceManagerTest, FillRouteSelectionDescriptor002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("FillRouteSelectionDescriptor002");
    std::map<std::string, std::string> ret;
    SelectedRouteDescriptor routeRule;
    routeRule.setPduSessionType(-1);
    std::string null;
    routeRule.setDnn(null);
    routeRule.setAppIds(null);
    routeRule.setProtocolIds(null);
    routeRule.setRemotePorts(null);
    routeRule.setIpv4Num(0);
    routeRule.setIpv6Num(0);
    DelayedSingleton<NetworkSliceManager>::GetInstance()->FillRouteSelectionDescriptor(ret, routeRule);
    EXPECT_NE(sUrspConfig_, nullptr);
}

HWTEST_F(NetworkSliceManagerTest, FillRoutePara001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("FillRoutePara001");
    AddRoutePara addRoutePara;
    addRoutePara.netId = 0;
    addRoutePara.uidNum = 1;
    addRoutePara.protocolIdNum = 1;
    addRoutePara.remotePortNum = 1;
    addRoutePara.remotePortRangeNum = 1;
    addRoutePara.singleRemotePortNum = 1;
    addRoutePara.len = 0;
    addRoutePara.urspPrecedence = 0;
    addRoutePara.ipv4Num = 1;
    addRoutePara.ipv6Num = 1;
    addRoutePara.uidArrays.push_back(1234);
    addRoutePara.protocolIdArrays.push_back("1");
    addRoutePara.remotePortsArrays.push_back("80");
    addRoutePara.ipv4AddrAndMasks.push_back(24);
    addRoutePara.ipv6AddrAndPrefixs.push_back(64);
    std::vector<uint8_t> buffer;
    EXPECT_TRUE(DelayedSingleton<NetworkSliceManager>::GetInstance()->FillRoutePara(buffer, addRoutePara));
}

HWTEST_F(NetworkSliceManagerTest, FillRoutePara002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("FillRoutePara002");
    AddRoutePara addRoutePara;
    addRoutePara.netId = 0;
    addRoutePara.uidNum = 1;
    addRoutePara.protocolIdNum = 1;
    addRoutePara.remotePortNum = 1;
    addRoutePara.remotePortRangeNum = 1;
    addRoutePara.singleRemotePortNum = 1;
    addRoutePara.len = 0;
    addRoutePara.urspPrecedence = 0;
    addRoutePara.ipv4Num = 1;
    addRoutePara.ipv6Num = 1;
    addRoutePara.uidArrays.push_back(1234);
    addRoutePara.protocolIdArrays.push_back("1");
    addRoutePara.remotePortsArrays.push_back("80-90");
    addRoutePara.ipv4AddrAndMasks.push_back(24);
    addRoutePara.ipv6AddrAndPrefixs.push_back(64);
    std::vector<uint8_t> buffer;
    EXPECT_TRUE(DelayedSingleton<NetworkSliceManager>::GetInstance()->FillRoutePara(buffer, addRoutePara));
}

HWTEST_F(NetworkSliceManagerTest, isWifiConnected001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isWifiConnected");
    EXPECT_FALSE(DelayedSingleton<NetworkSliceManager>::GetInstance()->isWifiConnected());
}

HWTEST_F(NetworkSliceManagerTest, isAirPlaneModeOn001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isAirPlaneModeOn001");
    EXPECT_TRUE(DelayedSingleton<NetworkSliceManager>::GetInstance()->isAirPlaneModeOn());
}

HWTEST_F(NetworkSliceManagerTest, isInVpnMode001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isInVpnMode001");
    EXPECT_TRUE(DelayedSingleton<NetworkSliceManager>::GetInstance()->isInVpnMode());
}

HWTEST_F(NetworkSliceManagerTest, isDefaultDataOnMainCard001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isDefaultDataOnMainCard001");
    EXPECT_TRUE(DelayedSingleton<NetworkSliceManager>::GetInstance()->isDefaultDataOnMainCard());
}
 
HWTEST_F(NetworkSliceManagerTest, DumpSelectedRouteDescriptor001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DumpSelectedRouteDescriptor001");
    SelectedRouteDescriptor routeRule;
    routeRule.mSscMode = 0;
    routeRule.mPduSessionType = -1;
    routeRule.mSnssai = "110011";
    routeRule.mDnn = "test";
    DelayedSingleton<NetworkSliceManager>::GetInstance()->DumpSelectedRouteDescriptor(routeRule);
    EXPECT_NE(sUrspConfig_, nullptr);
}

}
}
