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

#include "eth_eap_auto_auth_controller.h"

#include <chrono>
#include <thread>

#include "eth_eap_cert_store.h"
#include "eth_eap_config_store.h"
#include "eth_eap_state_manager.h"
#include "net_eap_handler.h"
#include "netmgr_ext_log_wrapper.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {

static constexpr int32_t MAX_RETRY_COUNT = 3;
static constexpr int32_t RETRY_INTERVAL_MS = 3000;
static constexpr int32_t AUTH_TIMEOUT_MS = 60000;
static constexpr int32_t DEFAULT_ETH_EAP_NETID = -1;
static constexpr const char *ETH_PREFIX = "eth";

EthEapAutoAuthController &EthEapAutoAuthController::GetInstance()
{
    static EthEapAutoAuthController instance;
    return instance;
}

bool EthEapAutoAuthController::IsEnterpriseOccupied() const
{
    return enterpriseOccupied_.load();
}

void EthEapAutoAuthController::OnEnterpriseAppOccupied()
{
    NETMGR_EXT_LOG_I("Enterprise app occupied, stopping auto auth");
    enterpriseOccupied_.store(true);
    StopAutoAuth();
}

void EthEapAutoAuthController::OnEnterpriseAppReleased()
{
    NETMGR_EXT_LOG_I("Enterprise app released, auto auth may resume");
    enterpriseOccupied_.store(false);
    OnTriggerAutoAuth();
}

void EthEapAutoAuthController::OnLinkUp(const std::string &ifName)
{
    if (ifName.find(ETH_PREFIX) == std::string::npos) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    ifName_ = ifName;
    NETMGR_EXT_LOG_I("OnLinkUp: %{public}s", ifName.c_str());
    OnTriggerAutoAuth();
}

void EthEapAutoAuthController::OnBootCompleted()
{
    NETMGR_EXT_LOG_I("OnBootCompleted, triggering auto auth");
    OnTriggerAutoAuth();
}

void EthEapAutoAuthController::OnTriggerAutoAuth()
{
    TryStartAuth(false);
}

void EthEapAutoAuthController::OnManualConnect()
{
    // 手动连接（connect(true)）：不受 autoAuth 开关限制，
    // autoAuth 仅约束网线插入/开机等自动触发场景
    NETMGR_EXT_LOG_I("OnManualConnect, manual auth triggered");
    TryStartAuth(true);
}

void EthEapAutoAuthController::StopAutoAuth()
{
    NETMGR_EXT_LOG_I("StopAutoAuth");
    authenticating_.store(false);
    manualRound_.store(false);
    EthEapStateManager::GetInstance().ResetRetryCount();
}

void EthEapAutoAuthController::OnUserPaused()
{
    NETMGR_EXT_LOG_I("OnUserPaused: user disconnected via connect(false), auto auth paused until user resumes");
    userPaused_.store(true);
}

void EthEapAutoAuthController::OnUserResumed()
{
    if (userPaused_.exchange(false)) {
        NETMGR_EXT_LOG_I("OnUserResumed: user resume, auto auth available again");
    }
}

bool EthEapAutoAuthController::IsUserPaused() const
{
    return userPaused_.load();
}

void EthEapAutoAuthController::TryStartAuth(bool manual)
{
    if (enterpriseOccupied_.load()) {
        NETMGR_EXT_LOG_D("TryStartAuth: enterprise occupied, skip");
        return;
    }
    if (userPaused_.load()) {
        NETMGR_EXT_LOG_D("TryStartAuth: user paused (connect(false)), skip");
        return;
    }
    EthEapConfig config;
    if (EthEapConfigStore::GetInstance().LoadConfig(config) != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_D("TryStartAuth: no config or load failed, skip");
        return;
    }
    if (!config.enabled) {
        NETMGR_EXT_LOG_D("TryStartAuth: 802.1X not enabled, skip");
        return;
    }
    // autoAuth 开关仅约束自动触发场景（插线/开机/企业释放）；
    // 手动连接（connect 按钮）不受该开关限制
    if (!manual && !config.autoAuth) {
        NETMGR_EXT_LOG_D("TryStartAuth: autoAuth disabled, skip");
        return;
    }
    if (authenticating_.load()) {
        NETMGR_EXT_LOG_I("TryStartAuth: already authenticating, skip");
        return;
    }
    DoAuthenticate(manual);
}

void EthEapAutoAuthController::DoAuthenticate(bool manual)
{
    // CAS 保证并发触发（插线/开机/手动/重试线程）时仅启动一个认证轮次
    bool expected = false;
    if (!authenticating_.compare_exchange_strong(expected, true)) {
        NETMGR_EXT_LOG_I("DoAuthenticate: already authenticating, skip");
        return;
    }
    // 记录本轮认证是否为手动发起：失败重试沿用同一模式
    //（手动发起的认证在 autoAuth=false 时仍继续重试，直至成功或耗尽次数）
    manualRound_.store(manual);
    EthEapStateManager::GetInstance().ResetRetryCount();
    EthEapStateManager::GetInstance().SetState(EthEapState::STATE_AUTHENTICATING);

    EthEapConfig config;
    if (EthEapConfigStore::GetInstance().LoadConfig(config) != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("DoAuthenticate: load config failed");
        EthEapStateManager::GetInstance().SetState(EthEapState::STATE_FAILED, "CONFIG_LOAD_FAILED");
        authenticating_.store(false);
        return;
    }
    // 认证前自愈：落盘证书文件缺失时用持久化数据重建（失败仅告警，认证结果兜底反馈）
    if (EthEapCertStore::GetInstance().EnsureCertFiles(config.profile) != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_W("DoAuthenticate: ensure cert files failed, continue auth");
    }
    uint32_t generation = authGeneration_.fetch_add(1) + 1;
    int32_t ret = NetEapHandler::GetInstance().StartEthEap(DEFAULT_ETH_EAP_NETID, config.profile);
    if (ret != EAP_ERRCODE_SUCCESS) {
        NETMGR_EXT_LOG_E("DoAuthenticate: StartEthEap failed: %{public}d", ret);
        OnAuthFailed("INTERNAL_ERROR");
        return;
    }
    ScheduleAuthTimeoutCheck(generation);
}

void EthEapAutoAuthController::OnAuthSuccess()
{
    if (!authenticating_.load() || enterpriseOccupied_.load()) {
        NETMGR_EXT_LOG_I("OnAuthSuccess: no active auth round or enterprise occupied, ignore stale event");
        return;
    }
    NETMGR_EXT_LOG_I("OnAuthSuccess");
    authenticating_.store(false);
    manualRound_.store(false);
    EthEapStateManager::GetInstance().ResetRetryCount();
    EthEapStateManager::GetInstance().SetState(EthEapState::STATE_AUTHENTICATED);
}

void EthEapAutoAuthController::OnAuthFailed(const std::string &reason)
{
    if (!authenticating_.load() && !enterpriseOccupied_.load()) {
        NETMGR_EXT_LOG_I("OnAuthFailed: no active auth round, ignore stale event");
        return;
    }
    NETMGR_EXT_LOG_I("OnAuthFailed: %{public}s", reason.c_str());
    authenticating_.store(false);
    if (enterpriseOccupied_.load()) {
        NETMGR_EXT_LOG_D("OnAuthFailed: enterprise occupied, no retry");
        manualRound_.store(false);
        return;
    }
    int32_t currentRetry = EthEapStateManager::GetInstance().GetRetryCount();
    if (currentRetry >= MAX_RETRY_COUNT) {
        NETMGR_EXT_LOG_I("OnAuthFailed: max retry reached (%{public}d), state=Failed", currentRetry);
        manualRound_.store(false);
        EthEapStateManager::GetInstance().SetState(EthEapState::STATE_FAILED, reason);
        // LogOff 异步化：OnAuthFailed 可能运行在 HDI 事件回调线程（OnEapEventReport）
        // 上下文中，同步反调 HDI（Unregister/StopEap）会与 ethernet_host 的 wpa 线程
        // 构成同线程重入互锁；与 ScheduleRetry 的既有做法一致，登出动作在独立线程执行。
        // 竞态防护：记录发起时的认证代数，执行前若有新一轮认证启动（代数变化或
        // 正在认证中），迟到 LogOff 会误杀新会话，直接跳过
        uint32_t logoffGeneration = authGeneration_.load();
        std::thread([this, logoffGeneration]() {
            if (authGeneration_.load() != logoffGeneration) {
                NETMGR_EXT_LOG_I("AsyncLogOff: new auth round started (generation changed), skip");
                return;
            }
            if (authenticating_.load()) {
                NETMGR_EXT_LOG_I("AsyncLogOff: authentication in progress, skip");
                return;
            }
            NetEapHandler::GetInstance().LogOffEthEap(DEFAULT_ETH_EAP_NETID);
        }).detach();
        return;
    }
    int32_t newRetry = currentRetry + 1;
    EthEapStateManager::GetInstance().SetRetryCount(newRetry);
    EthEapStateManager::GetInstance().SetState(EthEapState::STATE_RETRYING, reason);
    ScheduleRetry(reason);
}

void EthEapAutoAuthController::ScheduleRetry(const std::string &reason)
{
    std::thread([this, reason]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_INTERVAL_MS));
        if (enterpriseOccupied_.load()) {
            NETMGR_EXT_LOG_D("ScheduleRetry: enterprise occupied after sleep, cancel");
            return;
        }
        if (userPaused_.load()) {
            NETMGR_EXT_LOG_D("ScheduleRetry: user paused (connect(false)) after sleep, cancel");
            return;
        }
        if (!EthEapConfigStore::GetInstance().HasConfig()) {
            NETMGR_EXT_LOG_D("ScheduleRetry: config deleted, cancel");
            return;
        }
        EthEapConfig config;
        if (EthEapConfigStore::GetInstance().LoadConfig(config) != NETMANAGER_EXT_SUCCESS) {
            return;
        }
        // 功能关闭取消重试；autoAuth 关闭仅在非手动轮次时取消
        //（手动发起的认证轮次不受 autoAuth 开关影响）
        if (!config.enabled || (!config.autoAuth && !manualRound_.load())) {
            NETMGR_EXT_LOG_D("ScheduleRetry: disabled, cancel");
            return;
        }
        if (authenticating_.load()) {
            NETMGR_EXT_LOG_I("ScheduleRetry: new auth round in progress, cancel stale retry");
            return;
        }
        NETMGR_EXT_LOG_I("ScheduleRetry: retrying authentication (attempt %{public}d)",
            EthEapStateManager::GetInstance().GetRetryCount());
        NetEapHandler::GetInstance().LogOffEthEap(DEFAULT_ETH_EAP_NETID);
        authenticating_.store(true);
        EthEapStateManager::GetInstance().SetState(EthEapState::STATE_AUTHENTICATING);
        uint32_t generation = authGeneration_.fetch_add(1) + 1;
        int32_t ret = NetEapHandler::GetInstance().StartEthEap(DEFAULT_ETH_EAP_NETID, config.profile);
        if (ret != EAP_ERRCODE_SUCCESS) {
            OnAuthFailed(reason);
            return;
        }
        ScheduleAuthTimeoutCheck(generation);
    }).detach();
}

void EthEapAutoAuthController::ScheduleAuthTimeoutCheck(uint32_t generation)
{
    std::thread([this, generation]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(AUTH_TIMEOUT_MS));
        if (authGeneration_.load() != generation || !authenticating_.load()) {
            return;
        }
        NETMGR_EXT_LOG_I("AuthTimeout: no auth result event, generation=%{public}u", generation);
        OnAuthFailed("AUTH_TIMEOUT");
    }).detach();
}

} // namespace NetManagerStandard
} // namespace OHOS
