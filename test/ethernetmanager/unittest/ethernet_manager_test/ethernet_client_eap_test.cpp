/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "eth_eap_state_callback_stub.h"
#include "ethernet_client.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {

#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE

using namespace testing::ext;

namespace {
/** 测试用状态回调桩 */
class EthEapStateCallbackStubTestImpl : public EthEapStateCallbackStub {
public:
    int32_t OnEthEapStateChanged(const EthEapStateInfo &stateInfo) override
    {
        lastInfo = stateInfo;
        return NETMANAGER_EXT_SUCCESS;
    }
    EthEapStateInfo lastInfo;
};

/**
 * 单元测试环境中 EthernetService 未随测试进程运行，客户端代理不可用，
 * 各方法返回 EAP_ERRCODE_NETMANAGER_STOP；DT 真机环境若服务可达则返回
 * 业务码。两种环境下返回值都必须是已知业务错误码集合之一。
 */
bool IsKnownClientResult(int32_t ret)
{
    switch (ret) {
        case NETMANAGER_EXT_SUCCESS:
        case EAP_ERRCODE_NETMANAGER_STOP:
        case EAP_ERRCODE_PERMISSION_DENIED:
        case EAP_ERRCODE_INVALID_PROFILE:
        case EAP_ERRCODE_CONFIG_LOCKED:
        case EAP_ERRCODE_CERT_PROCESS_FAILED:
        case NETMANAGER_EXT_ERR_FILE_NOT_FOUND:
            return true;
        default:
            return false;
    }
}
} // namespace

class EthernetClientEapTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name  : RegisterEthEapStateCallbackNullTest001
 * @tc.number: EthernetClientEapTest_001
 * @tc.desc  : 注册空状态回调返回参数错误（客户端侧前置校验）
 */
HWTEST_F(EthernetClientEapTest, RegisterEthEapStateCallbackNullTest001, TestSize.Level1)
{
    auto client = DelayedSingleton<EthernetClient>::GetInstance();
    ASSERT_NE(client, nullptr);
    sptr<IEthEapStateCallback> nullCb = nullptr;
    EXPECT_EQ(client->RegisterEthEapStateCallback(nullCb), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
}

/**
 * @tc.name  : SetEthEapConfigThroughClientTest002
 * @tc.number: EthernetClientEapTest_002
 * @tc.desc  : 客户端 SetEthEapConfig 透传调用返回已知业务码（代理不可用为 33200009）
 */
HWTEST_F(EthernetClientEapTest, SetEthEapConfigThroughClientTest002, TestSize.Level1)
{
    auto client = DelayedSingleton<EthernetClient>::GetInstance();
    ASSERT_NE(client, nullptr);
    EthEapConfig config;
    config.profile.eapMethod = EapMethod::EAP_PEAP;
    config.profile.identity = "client-dt";
    config.enabled = true;
    config.autoAuth = false;
    int32_t ret = client->SetEthEapConfig(config);
    EXPECT_TRUE(IsKnownClientResult(ret));
}

/**
 * @tc.name  : GetEthEapConfigThroughClientTest003
 * @tc.number: EthernetClientEapTest_003
 * @tc.desc  : 客户端 GetEthEapConfig 透传调用返回已知业务码
 */
HWTEST_F(EthernetClientEapTest, GetEthEapConfigThroughClientTest003, TestSize.Level1)
{
    auto client = DelayedSingleton<EthernetClient>::GetInstance();
    ASSERT_NE(client, nullptr);
    EthEapConfig config;
    int32_t ret = client->GetEthEapConfig(config);
    EXPECT_TRUE(IsKnownClientResult(ret));
}

/**
 * @tc.name  : DeleteEthEapConfigThroughClientTest004
 * @tc.number: EthernetClientEapTest_004
 * @tc.desc  : 客户端 DeleteEthEapConfig 透传调用返回已知业务码
 */
HWTEST_F(EthernetClientEapTest, DeleteEthEapConfigThroughClientTest004, TestSize.Level1)
{
    auto client = DelayedSingleton<EthernetClient>::GetInstance();
    ASSERT_NE(client, nullptr);
    int32_t ret = client->DeleteEthEapConfig();
    EXPECT_TRUE(IsKnownClientResult(ret));
}

/**
 * @tc.name  : GetEthEapStateInfoThroughClientTest005
 * @tc.number: EthernetClientEapTest_005
 * @tc.desc  : 客户端 GetEthEapStateInfo 透传调用返回已知业务码
 */
HWTEST_F(EthernetClientEapTest, GetEthEapStateInfoThroughClientTest005, TestSize.Level1)
{
    auto client = DelayedSingleton<EthernetClient>::GetInstance();
    ASSERT_NE(client, nullptr);
    EthEapStateInfo stateInfo;
    int32_t ret = client->GetEthEapStateInfo(stateInfo);
    EXPECT_TRUE(IsKnownClientResult(ret));
}

/**
 * @tc.name  : ConnectEthEapThroughClientTest006
 * @tc.number: EthernetClientEapTest_006
 * @tc.desc  : 客户端 ConnectEthEap 连接/断连透传调用返回已知业务码
 */
HWTEST_F(EthernetClientEapTest, ConnectEthEapThroughClientTest006, TestSize.Level1)
{
    auto client = DelayedSingleton<EthernetClient>::GetInstance();
    ASSERT_NE(client, nullptr);
    int32_t retConnect = client->ConnectEthEap(true);
    EXPECT_TRUE(IsKnownClientResult(retConnect));
    int32_t retDisconnect = client->ConnectEthEap(false);
    EXPECT_TRUE(IsKnownClientResult(retDisconnect));
}

/**
 * @tc.name  : RegisterUnregisterStateCallbackThroughClientTest007
 * @tc.number: EthernetClientEapTest_007
 * @tc.desc  : 客户端状态回调注册/注销透传调用返回已知业务码
 */
HWTEST_F(EthernetClientEapTest, RegisterUnregisterStateCallbackThroughClientTest007, TestSize.Level1)
{
    auto client = DelayedSingleton<EthernetClient>::GetInstance();
    ASSERT_NE(client, nullptr);
    sptr<IEthEapStateCallback> callback = new (std::nothrow) EthEapStateCallbackStubTestImpl();
    ASSERT_NE(callback, nullptr);
    int32_t retRegister = client->RegisterEthEapStateCallback(callback);
    EXPECT_TRUE(IsKnownClientResult(retRegister));
    int32_t retUnregister = client->UnregisterEthEapStateCallback(callback);
    EXPECT_TRUE(IsKnownClientResult(retUnregister));
}

#endif // NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE

} // namespace NetManagerStandard
} // namespace OHOS
