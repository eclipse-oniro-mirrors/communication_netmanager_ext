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
    EXPECT_TRUE(ret1 == NETMANAGER_ERR_PARAMETER_ERROR || ret1 == NETMANAGER_SUCCESS);
}
 
HWTEST_F(NetEapHandlerTest, RegCustomEapHandlerTest002, TestSize.Level1)
{
    NetType netType = NetType::WLAN0;
    std::string regCmd = "2:277:278";
    int ret1 = NetEapHandler::GetInstance().RegCustomEapHandler(netType, regCmd, g_eapPostbackCallback);
    EXPECT_TRUE(ret1 == NETMANAGER_ERR_INVALID_PARAMETER || ret1 == NETMANAGER_SUCCESS);
}
 
HWTEST_F(NetEapHandlerTest, RegCustomEapHandlerTest003, TestSize.Level1)
{
    NetType netType = NetType::WLAN0;
    std::string regCmd = "2:277:278";
    int ret1 = NetEapHandler::GetInstance().RegisterCustomEapCallback(netType, nullptr);
    if (ret1 != NETMANAGER_SUCCESS) {
        return;
    }
    int ret2 = NetEapHandler::GetInstance().RegCustomEapHandler(netType, regCmd, g_eapPostbackCallback);
    EXPECT_TRUE(ret2 == NETMANAGER_ERR_INVALID_PARAMETER || ret2 == NETMANAGER_SUCCESS);
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
    EXPECT_TRUE(ret1 == NETMANAGER_ERR_LOCAL_PTR_NULL || ret1 == NETMANAGER_SUCCESS);
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
    eapData->msgId = 55;
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
    EXPECT_TRUE(ret1 == NETMANAGER_ERR_OPERATION_FAILED || ret1 == NETMANAGER_SUCCESS);
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
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
    NetEapHandler::GetInstance().eapHdiWpaManager_ = nullptr;
    ret = NetEapHandler::GetInstance().RegCustomEapHandler(netType, regCmd, g_eapPostbackCallback);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    NetEapHandler::GetInstance().eapHdiWpaManager_ = std::make_shared<EapHdiWpaManager>();
    ret = NetEapHandler::GetInstance().RegCustomEapHandler(netType, regCmd, g_eapPostbackCallback);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
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
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
    NetEapHandler::GetInstance().eapHdiWpaManager_ = nullptr;
    ret = NetEapHandler::GetInstance().ReplyCustomEapData(result, eapData);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
    NetEapHandler::GetInstance().eapHdiWpaManager_ = std::make_shared<EapHdiWpaManager>();
    ret = NetEapHandler::GetInstance().ReplyCustomEapData(result, eapData);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
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

} // namespace NetManagerStandard
} // namespace OHOS