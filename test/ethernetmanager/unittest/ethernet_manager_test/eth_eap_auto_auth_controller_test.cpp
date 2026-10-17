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

#include <chrono>
#include <thread>

#include "eth_eap_auto_auth_controller.h"
#include "eth_eap_config_store.h"
#include "eth_eap_state_manager.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {

#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE

namespace {
constexpr int32_t CONTROLLER_TEST_MAX_RETRY = 3;
} // namespace

using namespace testing::ext;

class EthEapAutoAuthControllerTest : public testing::Test {
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
        controller.authenticating_.store(false);
        controller.manualRound_.store(false);
        EthEapConfigStore::GetInstance().DeleteConfig();
        auto &mgr = EthEapStateManager::GetInstance();
        mgr.SetRetryCount(0);
        mgr.SetState(EthEapState::STATE_IDLE, "");
    }

    static EthEapConfig MakeEnabledConfig(bool autoAuth)
    {
        EthEapConfig config;
        config.profile.eapMethod = EapMethod::EAP_PEAP;
        config.profile.identity = "controller-dt";
        config.profile.password.assign("ControllerPass@123");
        config.enabled = true;
        config.autoAuth = autoAuth;
        return config;
    }

    /** 轮询等待状态离开指定状态（认证线程为异步执行） */
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

    /** 轮询等待到达指定状态 */
    static bool WaitForState(EthEapState to, int32_t timeoutMs)
    {
        auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
        while (std::chrono::steady_clock::now() < deadline) {
            if (EthEapStateManager::GetInstance().GetState() == to) {
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        return EthEapStateManager::GetInstance().GetState() == to;
    }
};

/**
 * @tc.name  : EnterpriseOccupiedFlagTest001
 * @tc.number: EthEapAutoAuthControllerTest_001
 * @tc.desc  : 企业应用占用/释放标志翻转
 */
HWTEST_F(EthEapAutoAuthControllerTest, EnterpriseOccupiedFlagTest001, TestSize.Level1)
{
    auto &controller = EthEapAutoAuthController::GetInstance();
    EXPECT_FALSE(controller.IsEnterpriseOccupied());
    controller.OnEnterpriseAppOccupied();
    EXPECT_TRUE(controller.IsEnterpriseOccupied());
    controller.OnEnterpriseAppReleased();
    EXPECT_FALSE(controller.IsEnterpriseOccupied());
}

/**
 * @tc.name  : UserPausedFlagTest002
 * @tc.number: EthEapAutoAuthControllerTest_002
 * @tc.desc  : 用户断连暂停/恢复标志翻转
 */
HWTEST_F(EthEapAutoAuthControllerTest, UserPausedFlagTest002, TestSize.Level1)
{
    auto &controller = EthEapAutoAuthController::GetInstance();
    EXPECT_FALSE(controller.IsUserPaused());
    controller.OnUserPaused();
    EXPECT_TRUE(controller.IsUserPaused());
    controller.OnUserResumed();
    EXPECT_FALSE(controller.IsUserPaused());
}

/**
 * @tc.name  : AutoAuthSkipNoConfigTest003
 * @tc.number: EthEapAutoAuthControllerTest_003
 * @tc.desc  : 无已保存配置时自动触发不发起认证（状态保持 IDLE）
 */
HWTEST_F(EthEapAutoAuthControllerTest, AutoAuthSkipNoConfigTest003, TestSize.Level1)
{
    ASSERT_FALSE(EthEapConfigStore::GetInstance().HasConfig());
    EthEapAutoAuthController::GetInstance().OnTriggerAutoAuth();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(EthEapStateManager::GetInstance().GetState(), EthEapState::STATE_IDLE);
}

/**
 * @tc.name  : AutoAuthSkipDisabledTest004
 * @tc.number: EthEapAutoAuthControllerTest_004
 * @tc.desc  : 配置存在但功能未启用（enabled=false）时自动触发跳过
 */
HWTEST_F(EthEapAutoAuthControllerTest, AutoAuthSkipDisabledTest004, TestSize.Level1)
{
    EthEapConfig config = MakeEnabledConfig(true);
    config.enabled = false;
    ASSERT_EQ(EthEapConfigStore::GetInstance().SaveConfig(config), NETMANAGER_EXT_SUCCESS);
    EthEapAutoAuthController::GetInstance().OnTriggerAutoAuth();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(EthEapStateManager::GetInstance().GetState(), EthEapState::STATE_IDLE);
}

/**
 * @tc.name  : AutoAuthSkipAutoAuthDisabledTest005
 * @tc.number: EthEapAutoAuthControllerTest_005
 * @tc.desc  : autoAuth=false 时自动触发跳过，手动连接不受该开关限制
 */
HWTEST_F(EthEapAutoAuthControllerTest, AutoAuthSkipAutoAuthDisabledTest005, TestSize.Level1)
{
    EthEapConfig config = MakeEnabledConfig(false); // enabled=true, autoAuth=false
    ASSERT_EQ(EthEapConfigStore::GetInstance().SaveConfig(config), NETMANAGER_EXT_SUCCESS);
    // 自动触发：被 autoAuth 开关拦截
    EthEapAutoAuthController::GetInstance().OnTriggerAutoAuth();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(EthEapStateManager::GetInstance().GetState(), EthEapState::STATE_IDLE);
    // 手动连接：绕过 autoAuth 开关，进入认证流程
    EthEapAutoAuthController::GetInstance().OnManualConnect();
    EXPECT_TRUE(WaitForStateLeave(EthEapState::STATE_IDLE, 8000));
    EthEapState finalState = EthEapStateManager::GetInstance().GetState();
    EXPECT_TRUE(finalState == EthEapState::STATE_AUTHENTICATING || finalState == EthEapState::STATE_RETRYING ||
                finalState == EthEapState::STATE_FAILED);
}

/**
 * @tc.name  : AutoAuthSkipWhenOccupiedTest006
 * @tc.number: EthEapAutoAuthControllerTest_006
 * @tc.desc  : 企业应用占用期间自动触发与手动连接均跳过认证
 */
HWTEST_F(EthEapAutoAuthControllerTest, AutoAuthSkipWhenOccupiedTest006, TestSize.Level1)
{
    EthEapConfig config = MakeEnabledConfig(true);
    ASSERT_EQ(EthEapConfigStore::GetInstance().SaveConfig(config), NETMANAGER_EXT_SUCCESS);
    auto &controller = EthEapAutoAuthController::GetInstance();
    controller.OnEnterpriseAppOccupied();
    controller.OnTriggerAutoAuth();
    controller.OnManualConnect();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(EthEapStateManager::GetInstance().GetState(), EthEapState::STATE_IDLE);
}

/**
 * @tc.name  : AutoAuthSkipWhenUserPausedTest007
 * @tc.number: EthEapAutoAuthControllerTest_007
 * @tc.desc  : 用户 connect(false) 断连后自动触发跳过（防止自动重连）
 */
HWTEST_F(EthEapAutoAuthControllerTest, AutoAuthSkipWhenUserPausedTest007, TestSize.Level1)
{
    EthEapConfig config = MakeEnabledConfig(true);
    ASSERT_EQ(EthEapConfigStore::GetInstance().SaveConfig(config), NETMANAGER_EXT_SUCCESS);
    auto &controller = EthEapAutoAuthController::GetInstance();
    controller.OnUserPaused();
    controller.OnTriggerAutoAuth();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(EthEapStateManager::GetInstance().GetState(), EthEapState::STATE_IDLE);
}

/**
 * @tc.name  : AutoAuthSkipWhenAuthenticatingTest008
 * @tc.number: EthEapAutoAuthControllerTest_008
 * @tc.desc  : 已有认证轮次进行中时重复触发跳过（防并发重入）
 */
HWTEST_F(EthEapAutoAuthControllerTest, AutoAuthSkipWhenAuthenticatingTest008, TestSize.Level1)
{
    auto &controller = EthEapAutoAuthController::GetInstance();
    controller.authenticating_.store(true);
    EthEapConfig config = MakeEnabledConfig(true);
    ASSERT_EQ(EthEapConfigStore::GetInstance().SaveConfig(config), NETMANAGER_EXT_SUCCESS);
    controller.OnTriggerAutoAuth();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    // 未进入 DoAuthenticate，状态保持 IDLE
    EXPECT_EQ(EthEapStateManager::GetInstance().GetState(), EthEapState::STATE_IDLE);
}

/**
 * @tc.name  : OnLinkUpEthPrefixTest009
 * @tc.number: EthEapAutoAuthControllerTest_009
 * @tc.desc  : 非以太网卡名 link up 不触发；eth 前缀 link up 触发自动认证
 */
HWTEST_F(EthEapAutoAuthControllerTest, OnLinkUpEthPrefixTest009, TestSize.Level1)
{
    auto &controller = EthEapAutoAuthController::GetInstance();
    // wlan0：非 eth 前缀，直接返回
    controller.OnLinkUp("wlan0");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(EthEapStateManager::GetInstance().GetState(), EthEapState::STATE_IDLE);

    EthEapConfig config = MakeEnabledConfig(true);
    ASSERT_EQ(EthEapConfigStore::GetInstance().SaveConfig(config), NETMANAGER_EXT_SUCCESS);
    controller.OnLinkUp("eth0");
    EXPECT_TRUE(WaitForStateLeave(EthEapState::STATE_IDLE, 8000));
}

/**
 * @tc.name  : OnBootCompletedTriggerTest010
 * @tc.number: EthEapAutoAuthControllerTest_010
 * @tc.desc  : 开机完成事件触发一次自动认证周期
 */
HWTEST_F(EthEapAutoAuthControllerTest, OnBootCompletedTriggerTest010, TestSize.Level1)
{
    EthEapConfig config = MakeEnabledConfig(true);
    ASSERT_EQ(EthEapConfigStore::GetInstance().SaveConfig(config), NETMANAGER_EXT_SUCCESS);
    EthEapAutoAuthController::GetInstance().OnBootCompleted();
    EXPECT_TRUE(WaitForStateLeave(EthEapState::STATE_IDLE, 8000));
}

/**
 * @tc.name  : OnAuthSuccessActiveRoundTest011
 * @tc.number: EthEapAutoAuthControllerTest_011
 * @tc.desc  : 认证轮次进行中收到成功事件：状态置已认证、重试计数清零
 */
HWTEST_F(EthEapAutoAuthControllerTest, OnAuthSuccessActiveRoundTest011, TestSize.Level1)
{
    auto &controller = EthEapAutoAuthController::GetInstance();
    controller.authenticating_.store(true);
    EthEapStateManager::GetInstance().SetRetryCount(2);
    EthEapStateManager::GetInstance().SetState(EthEapState::STATE_AUTHENTICATING, "");

    controller.OnAuthSuccess();
    EXPECT_EQ(EthEapStateManager::GetInstance().GetState(), EthEapState::STATE_AUTHENTICATED);
    EXPECT_EQ(EthEapStateManager::GetInstance().GetRetryCount(), 0);
    EXPECT_FALSE(controller.authenticating_.load());
}

/**
 * @tc.name  : OnAuthSuccessStaleEventTest012
 * @tc.number: EthEapAutoAuthControllerTest_012
 * @tc.desc  : 无进行中认证轮次时的陈旧成功事件被忽略（状态不变）
 */
HWTEST_F(EthEapAutoAuthControllerTest, OnAuthSuccessStaleEventTest012, TestSize.Level1)
{
    auto &controller = EthEapAutoAuthController::GetInstance();
    controller.authenticating_.store(false);
    EthEapStateManager::GetInstance().SetState(EthEapState::STATE_FAILED, "OLD_FAILURE");
    controller.OnAuthSuccess();
    EXPECT_EQ(EthEapStateManager::GetInstance().GetState(), EthEapState::STATE_FAILED);
}

/**
 * @tc.name  : OnAuthFailedRetryScheduledTest013
 * @tc.number: EthEapAutoAuthControllerTest_013
 * @tc.desc  : 认证失败且未达最大重试：计数加一、状态置重试中
 */
HWTEST_F(EthEapAutoAuthControllerTest, OnAuthFailedRetryScheduledTest013, TestSize.Level1)
{
    auto &controller = EthEapAutoAuthController::GetInstance();
    controller.authenticating_.store(true);
    EthEapStateManager::GetInstance().SetRetryCount(0);

    controller.OnAuthFailed("DT_AUTH_FAIL");
    EXPECT_EQ(EthEapStateManager::GetInstance().GetState(), EthEapState::STATE_RETRYING);
    EXPECT_EQ(EthEapStateManager::GetInstance().GetRetryCount(), 1);
    EthEapStateInfo info;
    EthEapStateManager::GetInstance().GetStateInfo(info);
    EXPECT_EQ(info.message, "DT_AUTH_FAIL");
    EXPECT_FALSE(controller.authenticating_.load());
}

/**
 * @tc.name  : OnAuthFailedMaxRetryReachedTest014
 * @tc.number: EthEapAutoAuthControllerTest_014
 * @tc.desc  : 达到最大重试次数后失败：状态置最终失败，不再调度重试
 */
HWTEST_F(EthEapAutoAuthControllerTest, OnAuthFailedMaxRetryReachedTest014, TestSize.Level1)
{
    auto &controller = EthEapAutoAuthController::GetInstance();
    controller.authenticating_.store(true);
    EthEapStateManager::GetInstance().SetRetryCount(CONTROLLER_TEST_MAX_RETRY);

    controller.OnAuthFailed("DT_MAX_RETRY");
    EXPECT_EQ(EthEapStateManager::GetInstance().GetState(), EthEapState::STATE_FAILED);
    EXPECT_EQ(EthEapStateManager::GetInstance().GetRetryCount(), CONTROLLER_TEST_MAX_RETRY);
    EXPECT_FALSE(controller.authenticating_.load());
    EXPECT_FALSE(controller.manualRound_.load());
}

/**
 * @tc.name  : OnAuthFailedStaleEventTest015
 * @tc.number: EthEapAutoAuthControllerTest_015
 * @tc.desc  : 无进行中认证轮次时的陈旧失败事件被忽略（状态不变）
 */
HWTEST_F(EthEapAutoAuthControllerTest, OnAuthFailedStaleEventTest015, TestSize.Level1)
{
    auto &controller = EthEapAutoAuthController::GetInstance();
    controller.authenticating_.store(false);
    EthEapStateManager::GetInstance().SetState(EthEapState::STATE_AUTHENTICATED, "");
    controller.OnAuthFailed("STALE_EVENT");
    EXPECT_EQ(EthEapStateManager::GetInstance().GetState(), EthEapState::STATE_AUTHENTICATED);
}

/**
 * @tc.name  : OnAuthFailedOccupiedNoRetryTest016
 * @tc.number: EthEapAutoAuthControllerTest_016
 * @tc.desc  : 企业占用期间认证失败不重试（企业应用保持控制权）
 */
HWTEST_F(EthEapAutoAuthControllerTest, OnAuthFailedOccupiedNoRetryTest016, TestSize.Level1)
{
    auto &controller = EthEapAutoAuthController::GetInstance();
    controller.OnEnterpriseAppOccupied();
    controller.authenticating_.store(true);
    EthEapStateManager::GetInstance().SetRetryCount(0);
    EthEapStateManager::GetInstance().SetState(EthEapState::STATE_AUTHENTICATING, "");

    controller.OnAuthFailed("OCCUPIED_FAIL");
    // 占用期间失败：不重试、不修改状态（状态机由企业应用主导）
    EXPECT_EQ(EthEapStateManager::GetInstance().GetState(), EthEapState::STATE_AUTHENTICATING);
    EXPECT_EQ(EthEapStateManager::GetInstance().GetRetryCount(), 0);
    EXPECT_FALSE(controller.authenticating_.load());
}

/**
 * @tc.name  : ScheduleRetryCancelByUserPausedTest017
 * @tc.number: EthEapAutoAuthControllerTest_017
 * @tc.desc  : 重试等待期间用户断连（connect(false)）后取消后续重试
 */
HWTEST_F(EthEapAutoAuthControllerTest, ScheduleRetryCancelByUserPausedTest017, TestSize.Level1)
{
    auto &controller = EthEapAutoAuthController::GetInstance();
    controller.authenticating_.store(true);
    EthEapStateManager::GetInstance().SetRetryCount(0);
    controller.OnAuthFailed("RETRY_THEN_PAUSE");
    ASSERT_EQ(EthEapStateManager::GetInstance().GetRetryCount(), 1);
    // 重试间隔内用户断连：休眠结束后取消本轮重试
    controller.OnUserPaused();
    std::this_thread::sleep_for(std::chrono::milliseconds(3500));
    EXPECT_EQ(EthEapStateManager::GetInstance().GetRetryCount(), 1);
}

/**
 * @tc.name  : ScheduleRetryCancelByConfigDeleteTest018
 * @tc.number: EthEapAutoAuthControllerTest_018
 * @tc.desc  : 重试等待期间配置被删除后取消后续重试
 */
HWTEST_F(EthEapAutoAuthControllerTest, ScheduleRetryCancelByConfigDeleteTest018, TestSize.Level1)
{
    auto &controller = EthEapAutoAuthController::GetInstance();
    controller.authenticating_.store(true);
    EthEapStateManager::GetInstance().SetRetryCount(0);
    controller.OnAuthFailed("RETRY_THEN_DELETE");
    ASSERT_EQ(EthEapStateManager::GetInstance().GetRetryCount(), 1);
    // 重试间隔内配置被删除：休眠结束后取消本轮重试
    ASSERT_EQ(EthEapConfigStore::GetInstance().DeleteConfig(), NETMANAGER_EXT_SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(3500));
    EXPECT_EQ(EthEapStateManager::GetInstance().GetRetryCount(), 1);
}

/**
 * @tc.name  : StopAutoAuthResetsTest019
 * @tc.number: EthEapAutoAuthControllerTest_019
 * @tc.desc  : StopAutoAuth 复位认证标志与重试计数
 */
HWTEST_F(EthEapAutoAuthControllerTest, StopAutoAuthResetsTest019, TestSize.Level1)
{
    auto &controller = EthEapAutoAuthController::GetInstance();
    controller.authenticating_.store(true);
    controller.manualRound_.store(true);
    EthEapStateManager::GetInstance().SetRetryCount(2);
    controller.StopAutoAuth();
    EXPECT_FALSE(controller.authenticating_.load());
    EXPECT_FALSE(controller.manualRound_.load());
    EXPECT_EQ(EthEapStateManager::GetInstance().GetRetryCount(), 0);
}

/**
 * @tc.name  : EnterpriseReleaseTriggersAutoAuthTest020
 * @tc.number: EthEapAutoAuthControllerTest_020
 * @tc.desc  : 企业应用释放后自动恢复认证周期
 */
HWTEST_F(EthEapAutoAuthControllerTest, EnterpriseReleaseTriggersAutoAuthTest020, TestSize.Level1)
{
    EthEapConfig config = MakeEnabledConfig(true);
    ASSERT_EQ(EthEapConfigStore::GetInstance().SaveConfig(config), NETMANAGER_EXT_SUCCESS);
    auto &controller = EthEapAutoAuthController::GetInstance();
    controller.OnEnterpriseAppOccupied();
    controller.OnEnterpriseAppReleased();
    EXPECT_FALSE(controller.IsEnterpriseOccupied());
    EXPECT_TRUE(WaitForStateLeave(EthEapState::STATE_IDLE, 8000));
}

#endif // NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE

} // namespace NetManagerStandard
} // namespace OHOS
