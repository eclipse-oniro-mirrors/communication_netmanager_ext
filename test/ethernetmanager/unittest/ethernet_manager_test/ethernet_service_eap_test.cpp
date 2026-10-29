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

#include <atomic>
#include <chrono>
#include <thread>

#include "eth_eap_auto_auth_controller.h"
#include "eth_eap_config_store.h"
#include "eth_eap_state_callback_stub.h"
#include "eth_eap_state_manager.h"
#include "ethernet_service.h"
#include "netmanager_ext_test_security.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {

#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE

using namespace testing::ext;

namespace {
/** 测试用状态回调：记录最近一次收到的状态信息与回调次数 */
class EthEapStateCallbackStubTestImpl : public EthEapStateCallbackStub {
public:
    int32_t OnEthEapStateChanged(const EthEapStateInfo &stateInfo) override
    {
        lastInfo = stateInfo;
        ++callCount;
        return NETMANAGER_EXT_SUCCESS;
    }

    EthEapStateInfo lastInfo;
    std::atomic<int32_t> callCount{0};
};
} // namespace

class EthernetServiceEapTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp() override
    {
        ResetEnvironment();
    }
    void TearDown() override
    {
        ResetEnvironment();
    }

    static void ResetEnvironment()
    {
        auto &controller = EthEapAutoAuthController::GetInstance();
        controller.OnEnterpriseAppReleased();
        controller.OnUserResumed();
        controller.StopAutoAuth();
        EthEapConfigStore::GetInstance().DeleteConfig();
        auto &mgr = EthEapStateManager::GetInstance();
        mgr.SetRetryCount(0);
        mgr.SetState(EthEapState::STATE_IDLE, "");
    }

    static EthEapConfig MakeServiceConfig()
    {
        EthEapConfig config;
        config.profile.eapMethod = EapMethod::EAP_PEAP;
        config.profile.phase2Method = Phase2Method::PHASE2_MSCHAPV2;
        config.profile.identity = "service-dt-identity";
        config.profile.password.assign("ServicePass@123");
        config.enabled = true;
        config.autoAuth = true;
        config.locked = false;
        return config;
    }

    static bool WaitForStateLeave(EthEapState from, int32_t timeoutMs)
    {
        auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
        while (std::chrono::steady_clock::now() < deadline) {
            if (EthEapStateManager::GetInstance().GetState() != from) {
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        return EthEapStateManager::GetInstance().GetState() != from;
    }
};

/**
 * @tc.name  : EapMethodsPermissionDeniedTest001
 * @tc.number: EthernetServiceEapTest_001
 * @tc.desc  : 无 ENTERPRISE_MANAGE_EAP 权限时全部 EAP 接口返回权限拒绝（201）
 */
HWTEST_F(EthernetServiceEapTest, EapMethodsPermissionDeniedTest001, TestSize.Level1)
{
    NoPermissionAccessToken noPermissionToken;
    EthernetService ethernetservice;
    EthEapConfig config = MakeServiceConfig();
    EthEapConfig outConfig;
    EthEapStateInfo stateInfo;
    // 空回调校验先于权限校验：返回参数错误
    sptr<IEthEapStateCallback> nullCb = nullptr;
    EXPECT_EQ(ethernetservice.RegisterEthEapStateCallback(nullCb), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    EXPECT_EQ(ethernetservice.UnregisterEthEapStateCallback(nullCb), NETMANAGER_EXT_ERR_PARAMETER_ERROR);

    EXPECT_EQ(ethernetservice.SetEthEapConfig(config), EAP_ERRCODE_PERMISSION_DENIED);
    EXPECT_EQ(ethernetservice.GetEthEapConfig(outConfig), EAP_ERRCODE_PERMISSION_DENIED);
    EXPECT_EQ(ethernetservice.DeleteEthEapConfig(), EAP_ERRCODE_PERMISSION_DENIED);
    EXPECT_EQ(ethernetservice.GetEthEapStateInfo(stateInfo), EAP_ERRCODE_PERMISSION_DENIED);
    EXPECT_EQ(ethernetservice.ConnectEthEap(true), EAP_ERRCODE_PERMISSION_DENIED);
    EXPECT_EQ(ethernetservice.ConnectEthEap(false), EAP_ERRCODE_PERMISSION_DENIED);
}

/**
 * @tc.name  : SetAndGetEthEapConfigTest002
 * @tc.number: EthernetServiceEapTest_002
 * @tc.desc  : 有权限时配置保存与回读成功，敏感字段（密码/证书口令/证书内容）不回读
 */
HWTEST_F(EthernetServiceEapTest, SetAndGetEthEapConfigTest002, TestSize.Level1)
{
    NetManagerExtAccessToken accessToken;
    EthernetService ethernetservice;
    EthEapConfig in = MakeServiceConfig();
    ASSERT_EQ(ethernetservice.SetEthEapConfig(in), NETMANAGER_EXT_SUCCESS);

    EthEapConfig out;
    ASSERT_EQ(ethernetservice.GetEthEapConfig(out), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(out.profile.eapMethod, in.profile.eapMethod);
    EXPECT_EQ(out.profile.phase2Method, in.profile.phase2Method);
    EXPECT_EQ(out.profile.identity, in.profile.identity);
    EXPECT_EQ(out.profile.caCertAliases, in.profile.caCertAliases);
    EXPECT_TRUE(out.profile.password.empty());
    EXPECT_TRUE(out.profile.certPassword.empty());
    EXPECT_TRUE(out.profile.certEntry.empty());
    EXPECT_EQ(out.enabled, in.enabled);
    EXPECT_EQ(out.autoAuth, in.autoAuth);
    EXPECT_EQ(out.locked, in.locked);
}

/**
 * @tc.name  : GetEthEapConfigNoConfigTest003
 * @tc.number: EthernetServiceEapTest_003
 * @tc.desc  : 未配置时回读返回文件不存在（2200209），上层据此区分未配置场景
 */
HWTEST_F(EthernetServiceEapTest, GetEthEapConfigNoConfigTest003, TestSize.Level1)
{
    NetManagerExtAccessToken accessToken;
    EthernetService ethernetservice;
    ASSERT_FALSE(EthEapConfigStore::GetInstance().HasConfig());
    EthEapConfig out;
    EXPECT_EQ(ethernetservice.GetEthEapConfig(out), NETMANAGER_EXT_ERR_FILE_NOT_FOUND);
}

/**
 * @tc.name  : SetEthEapConfigLockedTest004
 * @tc.number: EthernetServiceEapTest_004
 * @tc.desc  : locked 配置保护：锁定后再改配置被拒（33200012），企业应用解锁请求放行
 */
HWTEST_F(EthernetServiceEapTest, SetEthEapConfigLockedTest004, TestSize.Level1)
{
    NetManagerExtAccessToken accessToken;
    EthernetService ethernetservice;
    EthEapConfig in = MakeServiceConfig();
    in.locked = true;
    ASSERT_EQ(ethernetservice.SetEthEapConfig(in), NETMANAGER_EXT_SUCCESS);

    // 非解锁请求（locked=true）：拒绝修改
    EthEapConfig modify = MakeServiceConfig();
    modify.locked = true;
    modify.profile.identity = "modified-identity";
    EXPECT_EQ(ethernetservice.SetEthEapConfig(modify), EAP_ERRCODE_CONFIG_LOCKED);
    EthEapConfig out;
    ASSERT_EQ(ethernetservice.GetEthEapConfig(out), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(out.profile.identity, "service-dt-identity");

    // 解锁请求（locked=false）：放行
    EthEapConfig unlock = MakeServiceConfig();
    unlock.locked = false;
    EXPECT_EQ(ethernetservice.SetEthEapConfig(unlock), NETMANAGER_EXT_SUCCESS);
    ASSERT_EQ(ethernetservice.GetEthEapConfig(out), NETMANAGER_EXT_SUCCESS);
    EXPECT_FALSE(out.locked);
}

/**
 * @tc.name  : SetEthEapConfigSensitiveInheritTest005
 * @tc.number: EthernetServiceEapTest_005
 * @tc.desc  : 敏感字段继承：入参密码/证书口令为空时沿用已存凭据（IPC 回读不回传凭据）
 */
HWTEST_F(EthernetServiceEapTest, SetEthEapConfigSensitiveInheritTest005, TestSize.Level1)
{
    NetManagerExtAccessToken accessToken;
    EthernetService ethernetservice;
    EthEapConfig first = MakeServiceConfig();
    ASSERT_EQ(ethernetservice.SetEthEapConfig(first), NETMANAGER_EXT_SUCCESS);

    // 第二次设置：密码与证书口令为空（回读态），应继承第一次的凭据
    EthEapConfig second = MakeServiceConfig();
    second.profile.identity = "service-dt-identity-v2";
    second.profile.password.assign("");
    second.profile.certPassword.assign("");
    ASSERT_EQ(ethernetservice.SetEthEapConfig(second), NETMANAGER_EXT_SUCCESS);

    // 直接读配置存储验证继承结果（服务回读不回传敏感字段）
    EthEapConfig stored;
    ASSERT_EQ(EthEapConfigStore::GetInstance().LoadConfig(stored), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(stored.profile.identity, "service-dt-identity-v2");
    EXPECT_STREQ(stored.profile.password.c_str(), "ServicePass@123");
}

/**
 * @tc.name  : SetEthEapConfigInvalidCertTest006
 * @tc.number: EthernetServiceEapTest_006
 * @tc.desc  : certEntry 非法（非 PKCS12）时 SetEthEapConfig 返回证书处理失败（33200013）
 */
HWTEST_F(EthernetServiceEapTest, SetEthEapConfigInvalidCertTest006, TestSize.Level1)
{
    NetManagerExtAccessToken accessToken;
    EthernetService ethernetservice;
    EthEapConfig in = MakeServiceConfig();
    in.profile.certEntry = {'n', 'o', 't', 'a', 'p', '1', '2'};
    EXPECT_EQ(ethernetservice.SetEthEapConfig(in), EAP_ERRCODE_CERT_PROCESS_FAILED);
    // 保存失败：配置未落盘
    EXPECT_FALSE(EthEapConfigStore::GetInstance().HasConfig());
}

/**
 * @tc.name  : DeleteEthEapConfigTest007
 * @tc.number: EthernetServiceEapTest_007
 * @tc.desc  : 删除配置成功且幂等，删除后状态复位为 IDLE
 */
HWTEST_F(EthernetServiceEapTest, DeleteEthEapConfigTest007, TestSize.Level1)
{
    NetManagerExtAccessToken accessToken;
    EthernetService ethernetservice;
    EthEapConfig in = MakeServiceConfig();
    in.autoAuth = false; // 避免触发自动认证线程
    ASSERT_EQ(ethernetservice.SetEthEapConfig(in), NETMANAGER_EXT_SUCCESS);
    EthEapStateManager::GetInstance().SetState(EthEapState::STATE_AUTHENTICATING, "");

    EXPECT_EQ(ethernetservice.DeleteEthEapConfig(), NETMANAGER_EXT_SUCCESS);
    EXPECT_FALSE(EthEapConfigStore::GetInstance().HasConfig());
    EXPECT_EQ(EthEapStateManager::GetInstance().GetState(), EthEapState::STATE_IDLE);
    // 幂等删除：未配置时同样成功
    EXPECT_EQ(ethernetservice.DeleteEthEapConfig(), NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name  : GetEthEapStateInfoTest008
 * @tc.number: EthernetServiceEapTest_008
 * @tc.desc  : 有权限时查询状态信息返回一致快照（状态/重试计数/消息）
 */
HWTEST_F(EthernetServiceEapTest, GetEthEapStateInfoTest008, TestSize.Level1)
{
    NetManagerExtAccessToken accessToken;
    EthernetService ethernetservice;
    auto &mgr = EthEapStateManager::GetInstance();
    mgr.SetRetryCount(2);
    mgr.SetState(EthEapState::STATE_RETRYING, "service-dt-message");

    EthEapStateInfo stateInfo;
    EXPECT_EQ(ethernetservice.GetEthEapStateInfo(stateInfo), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(stateInfo.state, EthEapState::STATE_RETRYING);
    EXPECT_EQ(stateInfo.retryCount, 2);
    EXPECT_EQ(stateInfo.message, "service-dt-message");
}

/**
 * @tc.name  : ConnectEthEapNoConfigTest009
 * @tc.number: EthernetServiceEapTest_009
 * @tc.desc  : 无已保存配置时手动连接失败（33200003，d.ts 契约）
 */
HWTEST_F(EthernetServiceEapTest, ConnectEthEapNoConfigTest009, TestSize.Level1)
{
    NetManagerExtAccessToken accessToken;
    EthernetService ethernetservice;
    ASSERT_FALSE(EthEapConfigStore::GetInstance().HasConfig());
    EXPECT_EQ(ethernetservice.ConnectEthEap(true), EAP_ERRCODE_INVALID_PROFILE);
}

/**
 * @tc.name  : ConnectEthEapManualConnectTest010
 * @tc.number: EthernetServiceEapTest_010
 * @tc.desc  : 有已保存配置时手动连接受理成功并触发认证周期
 */
HWTEST_F(EthernetServiceEapTest, ConnectEthEapManualConnectTest010, TestSize.Level1)
{
    NetManagerExtAccessToken accessToken;
    EthernetService ethernetservice;
    EthEapConfig in = MakeServiceConfig();
    ASSERT_EQ(ethernetservice.SetEthEapConfig(in), NETMANAGER_EXT_SUCCESS);
    EthEapAutoAuthController::GetInstance().StopAutoAuth();
    EthEapStateManager::GetInstance().SetState(EthEapState::STATE_IDLE, "");

    EXPECT_EQ(ethernetservice.ConnectEthEap(true), NETMANAGER_EXT_SUCCESS);
    EXPECT_TRUE(WaitForStateLeave(EthEapState::STATE_IDLE, 8000));
}

/**
 * @tc.name  : ConnectEthEapDisconnectTest011
 * @tc.number: EthernetServiceEapTest_011
 * @tc.desc  : 断连（connect(false)）置用户暂停标志、停止自动认证并复位状态为 IDLE
 */
HWTEST_F(EthernetServiceEapTest, ConnectEthEapDisconnectTest011, TestSize.Level1)
{
    NetManagerExtAccessToken accessToken;
    EthernetService ethernetservice;
    EthEapConfig in = MakeServiceConfig();
    in.autoAuth = false;
    ASSERT_EQ(ethernetservice.SetEthEapConfig(in), NETMANAGER_EXT_SUCCESS);
    auto &controller = EthEapAutoAuthController::GetInstance();
    controller.OnUserResumed();
    EthEapStateManager::GetInstance().SetState(EthEapState::STATE_AUTHENTICATING, "");
    EthEapStateManager::GetInstance().SetRetryCount(2);

    EXPECT_NO_FATAL_FAILURE(ethernetservice.ConnectEthEap(false));
    EXPECT_TRUE(controller.IsUserPaused());
    EXPECT_EQ(EthEapStateManager::GetInstance().GetRetryCount(), 0);
    EXPECT_EQ(EthEapStateManager::GetInstance().GetState(), EthEapState::STATE_IDLE);
}

/**
 * @tc.name  : SetEthEapConfigDisabledStopsAuthTest012
 * @tc.number: EthernetServiceEapTest_012
 * @tc.desc  : 新配置关闭功能时不触发认证并停止进行中的自动认证
 */
HWTEST_F(EthernetServiceEapTest, SetEthEapConfigDisabledStopsAuthTest012, TestSize.Level1)
{
    NetManagerExtAccessToken accessToken;
    EthernetService ethernetservice;
    EthEapConfig in = MakeServiceConfig();
    in.enabled = false;
    in.autoAuth = false;
    EthEapStateManager::GetInstance().SetRetryCount(2);
    EthEapStateManager::GetInstance().SetState(EthEapState::STATE_RETRYING, "");

    EXPECT_EQ(ethernetservice.SetEthEapConfig(in), NETMANAGER_EXT_SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    EXPECT_EQ(EthEapStateManager::GetInstance().GetState(), EthEapState::STATE_RETRYING); // 状态机不复位，仅停轮次
    EXPECT_EQ(EthEapStateManager::GetInstance().GetRetryCount(), 0);
    EXPECT_EQ(EthEapStateManager::GetInstance().GetState() != EthEapState::STATE_AUTHENTICATING, true);
}

/**
 * @tc.name  : SetEthEapConfigTriggersAutoAuthTest013
 * @tc.number: EthernetServiceEapTest_013
 * @tc.desc  : 启用且自动认证开启的新配置触发一轮自动认证周期
 */
HWTEST_F(EthernetServiceEapTest, SetEthEapConfigTriggersAutoAuthTest013, TestSize.Level1)
{
    NetManagerExtAccessToken accessToken;
    EthernetService ethernetservice;
    EthEapConfig in = MakeServiceConfig(); // enabled=true, autoAuth=true
    EXPECT_EQ(ethernetservice.SetEthEapConfig(in), NETMANAGER_EXT_SUCCESS);
    EXPECT_TRUE(WaitForStateLeave(EthEapState::STATE_IDLE, 8000));
}

/**
 * @tc.name  : RegisterAndUnregisterEthEapStateCallbackTest014
 * @tc.number: EthernetServiceEapTest_014
 * @tc.desc  : 状态回调注册与注销：空回调拒绝，合法回调注册后经状态机收到通知
 */
HWTEST_F(EthernetServiceEapTest, RegisterAndUnregisterEthEapStateCallbackTest014, TestSize.Level1)
{
    // 空回调：参数错误（无权限令牌下同样先校验空回调）
    {
        EthernetService ethernetservice;
        sptr<IEthEapStateCallback> nullCb = nullptr;
        EXPECT_EQ(ethernetservice.RegisterEthEapStateCallback(nullCb), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
        EXPECT_EQ(ethernetservice.UnregisterEthEapStateCallback(nullCb), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    }
    // 有权限：合法回调注册成功并经状态机收到通知，注销后不再通知
    NetManagerExtAccessToken accessToken;
    EthernetService ethernetservice;
    auto *cbImpl = new (std::nothrow) EthEapStateCallbackStubTestImpl();
    ASSERT_NE(cbImpl, nullptr);
    sptr<IEthEapStateCallback> callback = cbImpl;
    EXPECT_EQ(ethernetservice.RegisterEthEapStateCallback(callback), NETMANAGER_EXT_SUCCESS);
    EthEapStateManager::GetInstance().SetState(EthEapState::STATE_AUTHENTICATING, "cb-notify");
    EXPECT_EQ(cbImpl->callCount.load(), 1);
    EXPECT_EQ(cbImpl->lastInfo.state, EthEapState::STATE_AUTHENTICATING);
    EXPECT_EQ(cbImpl->lastInfo.message, "cb-notify");

    EXPECT_EQ(ethernetservice.UnregisterEthEapStateCallback(callback), NETMANAGER_EXT_SUCCESS);
    EthEapStateManager::GetInstance().SetState(EthEapState::STATE_IDLE, "");
    EXPECT_EQ(cbImpl->callCount.load(), 1);
}

/**
 * @tc.name  : DeleteEthEapConfigLockedTest015
 * @tc.number: EthernetServiceEapTest_015
 * @tc.desc  : locked 配置删除保护：锁定期间删除被拒（33200012），解锁后可删除
 */
HWTEST_F(EthernetServiceEapTest, DeleteEthEapConfigLockedTest015, TestSize.Level1)
{
    NetManagerExtAccessToken accessToken;
    EthernetService ethernetservice;
    EthEapConfig in = MakeServiceConfig();
    in.locked = true;
    in.autoAuth = false;
    ASSERT_EQ(ethernetservice.SetEthEapConfig(in), NETMANAGER_EXT_SUCCESS);

    // locked 期间删除被拒
    EXPECT_EQ(ethernetservice.DeleteEthEapConfig(), EAP_ERRCODE_CONFIG_LOCKED);
    EXPECT_TRUE(EthEapConfigStore::GetInstance().HasConfig());

    // 解锁后删除成功
    EthEapConfig unlock = MakeServiceConfig();
    unlock.locked = false;
    unlock.autoAuth = false;
    ASSERT_EQ(ethernetservice.SetEthEapConfig(unlock), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(ethernetservice.DeleteEthEapConfig(), NETMANAGER_EXT_SUCCESS);
    EXPECT_FALSE(EthEapConfigStore::GetInstance().HasConfig());
}

#endif // NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE

} // namespace NetManagerStandard
} // namespace OHOS
