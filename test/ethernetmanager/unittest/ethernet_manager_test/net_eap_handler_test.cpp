/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <filesystem>
#include <gtest/gtest.h>

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "net_eap_handler.h"
#include "net_eap_callback_stub.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr bool TEST_BOOL_VALUE = false;
constexpr int32_t TEST_INT32_VALUE = 1;
constexpr uint32_t TEST_UINT32_VALUE = 1;
constexpr const char *TEST_STRING_VALUE = "test";
constexpr const char *TEST_DOMAIN = "test.com";
} // namespace

class NetRegisterEapCallbackTest : public NetRegisterEapCallbackStub {
public:
    int32_t OnRegisterCustomEapCallback(const std::string &regCmd) override
    {
        return 0;
    }

    int32_t OnReplyCustomEapDataEvent(int result, const sptr<EapData> &eapData) override
    {
        return 0;
    }
};

class EapPostbackCallbackTest : public NetEapPostbackCallbackStub {
public:
    int32_t OnEapSupplicantPostback(NetType netType, const sptr<EapData> &eapData) override
    {
        return 0;
    }
};

static sptr<INetRegisterEapCallback> g_registerEapCallback = new (std::nothrow) NetRegisterEapCallbackTest();
static sptr<INetEapPostbackCallback> g_eapPostbackCallback = new (std::nothrow) EapPostbackCallbackTest();

using namespace testing::ext;
class NetEapHandlerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetEapHandlerTest::SetUpTestCase() {}

void NetEapHandlerTest::TearDownTestCase() {}

void NetEapHandlerTest::SetUp() {}

void NetEapHandlerTest::TearDown() {}

HWTEST_F(NetEapHandlerTest, RegisterCustomEapCallbackTest001, TestSize.Level1)
{
    NetType netType = NetType::INVALID;
    int ret1 = NetEapHandler::GetInstance().RegisterCustomEapCallback(netType, g_registerEapCallback);
    EXPECT_TRUE(ret1 == NETMANAGER_ERR_PARAMETER_ERROR || ret1 == NETMANAGER_SUCCESS);
}

HWTEST_F(NetEapHandlerTest, RegisterCustomEapCallbackTest002, TestSize.Level1)
{
    NetType netType = NetType::WLAN0;
    sptr<INetRegisterEapCallback> callback = nullptr;
    int ret1 = NetEapHandler::GetInstance().RegisterCustomEapCallback(netType, callback);
    EXPECT_TRUE(ret1 == NETMANAGER_ERR_LOCAL_PTR_NULL || ret1 == NETMANAGER_SUCCESS);
}

HWTEST_F(NetEapHandlerTest, RegisterCustomEapCallbackTest003, TestSize.Level1)
{
    NetType netType = NetType::WLAN0;
    int ret1 = NetEapHandler::GetInstance().RegisterCustomEapCallback(netType, g_registerEapCallback);
    EXPECT_EQ(ret1, NETMANAGER_SUCCESS);
}

HWTEST_F(NetEapHandlerTest, UnRegisterCustomEapCallbackTest001, TestSize.Level1)
{
    NetType netType = NetType::INVALID;
    int ret1 = NetEapHandler::GetInstance().UnRegisterCustomEapCallback(netType, g_registerEapCallback);
    EXPECT_TRUE(ret1 == NETMANAGER_ERR_PARAMETER_ERROR || ret1 == NETMANAGER_SUCCESS);
}

HWTEST_F(NetEapHandlerTest, UnRegisterCustomEapCallbackTest002, TestSize.Level1)
{
    NetType netType = NetType::WLAN0;
    sptr<INetRegisterEapCallback> callback = nullptr;
    int ret1 = NetEapHandler::GetInstance().UnRegisterCustomEapCallback(netType, callback);
    EXPECT_TRUE(ret1 == NETMANAGER_ERR_LOCAL_PTR_NULL || ret1 == NETMANAGER_SUCCESS);
}

HWTEST_F(NetEapHandlerTest, UnRegisterCustomEapCallbackTest003, TestSize.Level1)
{
    NetType netType = NetType::WLAN0;
    int ret1 = NetEapHandler::GetInstance().UnRegisterCustomEapCallback(netType, g_registerEapCallback);
    EXPECT_EQ(ret1, NETMANAGER_SUCCESS);
}

HWTEST_F(NetEapHandlerTest, RegCustomEapHandlerTest001, TestSize.Level1)
{
    NetType netType = NetType::INVALID;
    std::string regCmd = "2:277:278";
    int ret1 = NetEapHandler::GetInstance().RegCustomEapHandler(netType, regCmd, g_eapPostbackCallback);
    EXPECT_GE(ret1, NETMANAGER_SUCCESS);
}

HWTEST_F(NetEapHandlerTest, RegCustomEapHandlerTest002, TestSize.Level1)
{
    NetType netType = NetType::WLAN0;
    std::string regCmd = "2:277:278";
    int ret1 = NetEapHandler::GetInstance().RegCustomEapHandler(netType, regCmd, g_eapPostbackCallback);
    EXPECT_GE(ret1, NETMANAGER_SUCCESS);
}

HWTEST_F(NetEapHandlerTest, RegCustomEapHandlerTest003, TestSize.Level1)
{
    NetType netType = NetType::WLAN0;
    std::string regCmd = "2:277:278";
    int ret1 = NetEapHandler::GetInstance().RegisterCustomEapCallback(netType, nullptr);
    EXPECT_GE(ret1, NETMANAGER_SUCCESS);
    int ret2 = NetEapHandler::GetInstance().RegCustomEapHandler(netType, regCmd, g_eapPostbackCallback);
    EXPECT_GE(ret2, NETMANAGER_SUCCESS);
}

HWTEST_F(NetEapHandlerTest, RegCustomEapHandlerTest004, TestSize.Level1)
{
    NetType netType = NetType::WLAN0;
    std::string regCmd = "2:277:278";
    int ret1 = NetEapHandler::GetInstance().RegisterCustomEapCallback(netType, g_registerEapCallback);
    if (ret1 != NETMANAGER_SUCCESS) {
        return;
    }
    int ret2 = NetEapHandler::GetInstance().RegCustomEapHandler(netType, regCmd, g_eapPostbackCallback);
    EXPECT_EQ(ret2, NETMANAGER_SUCCESS);
}

HWTEST_F(NetEapHandlerTest, RegCustomEapHandlerTest005, TestSize.Level1)
{
    NetType netType = NetType::WLAN0;
    std::string regCmd = "2:277:278";
    int ret1 = NetEapHandler::GetInstance().RegCustomEapHandler(netType, regCmd, nullptr);
    EXPECT_GE(ret1, NETMANAGER_SUCCESS);
}

HWTEST_F(NetEapHandlerTest, NotifyWpaEapInterceptInfoTest001, TestSize.Level1)
{
    NetType netType = NetType::WLAN0;
    std::string regCmd = "2:277:278";
    NetEapHandler::GetInstance().RegisterCustomEapCallback(netType, g_registerEapCallback);
    NetEapHandler::GetInstance().RegCustomEapHandler(netType, regCmd, g_eapPostbackCallback);
    sptr<EapData> eapData =new (std::nothrow) EapData();
    eapData->eapCode = 1;
    eapData->eapType = 13;
    eapData->msgId = 56;
    eapData->bufferLen = 4;
    std::vector<uint8_t> tmp = {0x11, 0x12};
    eapData->eapBuffer = tmp;
    int ret1 = NetEapHandler::GetInstance().NotifyWpaEapInterceptInfo(netType, eapData);
    EXPECT_EQ(ret1, NETMANAGER_SUCCESS);
}

HWTEST_F(NetEapHandlerTest, ReplyCustomEapDataTest001, TestSize.Level1)
{
    NetType netType = NetType::WLAN0;
    std::string regCmd = "2:277:278";
    NetEapHandler::GetInstance().RegisterCustomEapCallback(netType, g_registerEapCallback);
    NetEapHandler::GetInstance().RegCustomEapHandler(netType, regCmd, g_eapPostbackCallback);
    sptr<EapData> eapData =new (std::nothrow) EapData();
    eapData->eapCode = 1;
    eapData->eapType = 13;
    eapData->msgId = 55;
    eapData->bufferLen = 4;
    std::vector<uint8_t> tmp = {0x11, 0x12};
    eapData->eapBuffer = tmp;
    NetEapHandler::GetInstance().NotifyWpaEapInterceptInfo(netType, eapData);
    eapData->msgId = 54;
    int ret1 = NetEapHandler::GetInstance().ReplyCustomEapData(1, eapData);
    EXPECT_GE(ret1, NETMANAGER_SUCCESS);
    eapData->msgId = 55;
    int ret2 = NetEapHandler::GetInstance().ReplyCustomEapData(1, eapData);
    EXPECT_EQ(ret2, NETMANAGER_SUCCESS);
}

HWTEST_F(NetEapHandlerTest, RegCustomEapHandlerTest006, TestSize.Level1)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    NetType netType = NetType::ETH0;
    std::string regCmd = "2:277:278:279";
    int ret = NetEapHandler::GetInstance().RegCustomEapHandler(netType, regCmd, g_eapPostbackCallback);
    EXPECT_GE(ret, NETMANAGER_SUCCESS);
    NetEapHandler::GetInstance().eapHdiWpaManager_ = nullptr;
    ret = NetEapHandler::GetInstance().RegCustomEapHandler(netType, regCmd, g_eapPostbackCallback);
    EXPECT_LE(ret, NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    NetEapHandler::GetInstance().eapHdiWpaManager_ = std::make_shared<EapHdiWpaManager>();
    ret = NetEapHandler::GetInstance().RegCustomEapHandler(netType, regCmd, g_eapPostbackCallback);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
#endif
}


HWTEST_F(NetEapHandlerTest, ReplyCustomEapDataTest002, TestSize.Level1)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    NetType netType = NetType::ETH0;
    std::string regCmd = "2:277:278";
    NetEapHandler::GetInstance().RegisterCustomEapCallback(netType, g_registerEapCallback);
    NetEapHandler::GetInstance().RegCustomEapHandler(netType, regCmd, g_eapPostbackCallback);
    int32_t result = 1;
    sptr<EapData> eapData =new (std::nothrow) EapData();
    eapData->eapCode = 2;
    eapData->eapType = 13;
    eapData->msgId = 55;
    eapData->bufferLen = 4;
    std::vector<uint8_t> tmp = {0x11, 0x12};
    eapData->eapBuffer = tmp;
    int ret = NetEapHandler::GetInstance().ReplyCustomEapData(result, eapData);
    EXPECT_GE(ret, NETMANAGER_SUCCESS);
    NetEapHandler::GetInstance().eapHdiWpaManager_ = nullptr;
    ret = NetEapHandler::GetInstance().ReplyCustomEapData(result, eapData);
    EXPECT_GE(ret, NETMANAGER_SUCCESS);
    NetEapHandler::GetInstance().eapHdiWpaManager_ = std::make_shared<EapHdiWpaManager>();
    ret = NetEapHandler::GetInstance().ReplyCustomEapData(result, eapData);
    EXPECT_GE(ret, NETMANAGER_SUCCESS);
#endif
}

HWTEST_F(NetEapHandlerTest, StartEthEapTest001, TestSize.Level1)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    int32_t netId = 100;
    EthEapProfile profile;
    int ret = NetEapHandler::GetInstance().StartEthEap(netId, profile);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
    NetEapHandler::GetInstance().eapHdiWpaManager_ = nullptr;
    ret = NetEapHandler::GetInstance().StartEthEap(netId, profile);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
    NetEapHandler::GetInstance().eapHdiWpaManager_ = std::make_shared<EapHdiWpaManager>();
#endif
}

HWTEST_F(NetEapHandlerTest, LogOffEthEapTest001, TestSize.Level1)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    int32_t netId = 100;
    int ret = NetEapHandler::GetInstance().LogOffEthEap(netId);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
    NetEapHandler::GetInstance().eapHdiWpaManager_ = nullptr;
    ret = NetEapHandler::GetInstance().LogOffEthEap(netId);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
    NetEapHandler::GetInstance().eapHdiWpaManager_ = std::make_shared<EapHdiWpaManager>();
#endif
}

HWTEST_F(NetEapHandlerTest, StartEthEapTest002, TestSize.Level1)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    std::vector<int32_t> netIds = {100, 101, 102, 103, 104, 105};
    EthEapProfile profile;
    for (int32_t netId : netIds) {
        int ret = NetEapHandler::GetInstance().StartEthEap(netId, profile);
        EXPECT_NE(ret, NETMANAGER_SUCCESS);
    }
#endif
}

HWTEST_F(NetEapHandlerTest, LogOffEthEapTest002, TestSize.Level1)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    std::vector<int32_t> netIds = {100, 101, 102, 103, 104, 105};
    for (int32_t netId : netIds) {
        int ret = NetEapHandler::GetInstance().LogOffEthEap(netId);
        EXPECT_NE(ret, NETMANAGER_SUCCESS);
    }
#endif
}

HWTEST_F(NetEapHandlerTest, StartEthEapTest003, TestSize.Level1)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    int32_t netId = -1;
    EthEapProfile profile;
    int ret = NetEapHandler::GetInstance().StartEthEap(netId, profile);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
#endif
}

HWTEST_F(NetEapHandlerTest, LogOffEthEapTest003, TestSize.Level1)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    int32_t netId = -1;
    int ret = NetEapHandler::GetInstance().LogOffEthEap(netId);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
#endif
}

#ifdef NET_EXTENSIBLE_AUTHENTICATION
namespace {
// 伪造 HDI 以太网服务：函数指针全部返回 HDF_SUCCESS，用于打通 StartEthEap 成功链路
struct IEthernet g_fakeIEthernet = {};
struct HDIDeviceManager g_fakeDevMgr = {};

int32_t FakeHdiStartEap(struct IEthernet *self, const char *ifName)
{
    (void)self;
    (void)ifName;
    return HDF_SUCCESS;
}

int32_t FakeHdiStopEap(struct IEthernet *self, const char *ifName)
{
    (void)self;
    (void)ifName;
    return HDF_SUCCESS;
}

int32_t FakeHdiRegisterEapEventCallback(struct IEthernet *self, struct IEthernetCallback *cbFunc,
    const char *ifName)
{
    (void)self;
    (void)cbFunc;
    (void)ifName;
    return HDF_SUCCESS;
}

int32_t FakeHdiEapShellCmd(struct IEthernet *self, const char *ifName, const char *cmd)
{
    (void)self;
    (void)ifName;
    (void)cmd;
    return HDF_SUCCESS;
}

void InitFakeHdiService()
{
    g_fakeIEthernet.StartEap = FakeHdiStartEap;
    g_fakeIEthernet.StopEap = FakeHdiStopEap;
    g_fakeIEthernet.RegisterEapEventCallback = FakeHdiRegisterEapEventCallback;
    g_fakeIEthernet.UnregisterEapEventCallback = FakeHdiRegisterEapEventCallback;
    g_fakeIEthernet.EapShellCmd = FakeHdiEapShellCmd;
}
} // namespace
#endif

/**
 * @tc.name: StartEthEapTest004
 * @tc.desc: Test NetEapHandler StartEthEap success path with fake HDI service.
 * @tc.type: FUNC
 */
HWTEST_F(NetEapHandlerTest, StartEthEapTest004, TestSize.Level1)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    InitFakeHdiService();
    NetEapHandler &handler = NetEapHandler::GetInstance();
    // 注入伪造 HDI 管理器：LoadEthernetHdiService 命中非空早退，StartEap 全链路走 fake 成功返回
    auto fakeManager = std::make_shared<EapHdiWpaManager>();
    fakeManager->iEthernet_ = &g_fakeIEthernet;
    fakeManager->devMgr_ = &g_fakeDevMgr;
    handler.eapHdiWpaManager_ = fakeManager;
    handler.SetEthEapIfName("eth0");
    std::filesystem::create_directories("/data/service/el1/public/eth");
    // 预置注册命令：数字段入库、空段/非数字段与超长段跳过，覆盖注册命令合并的各解析分支
    handler.RegCustomEapHandler(NetType::ETH0, "2:2:277:1024", g_eapPostbackCallback);
    handler.RegCustomEapHandler(NetType::ETH0, "2:2:abc:def", g_eapPostbackCallback);
    handler.RegCustomEapHandler(NetType::ETH0, "2:2:1234567890", g_eapPostbackCallback);
    handler.RegCustomEapHandler(NetType::ETH0, "2:2:", g_eapPostbackCallback);
    EthEapProfile profile;
    profile.eapMethod = EapMethod::EAP_PEAP;
    profile.caPath = "caPath";
    int32_t ret = handler.StartEthEap(-1, profile);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    // 恢复单例真实管理器，避免进程退出析构时走 fake
    handler.eapHdiWpaManager_ = std::make_shared<EapHdiWpaManager>();
    handler.SetEthEapIfName("");
#endif
}

} // namespace NetManagerStandard
} // namespace OHOS