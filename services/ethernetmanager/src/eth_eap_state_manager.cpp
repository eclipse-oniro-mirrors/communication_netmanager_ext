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

#include "eth_eap_state_manager.h"
#include "eth_eap_state_callback_proxy.h"
#include "netmgr_ext_log_wrapper.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {

EthEapStateManager &EthEapStateManager::GetInstance()
{
    // 静态局部变量的初始化本身线程安全，可直接用作单例
    static EthEapStateManager instance;
    return instance;
}

void EthEapStateManager::SetState(EthEapState state, const std::string &message)
{
    EthEapStateInfo info;
    {
        // 在锁内更新状态并组装通知信息，保证状态与重试计数的一致性
        std::lock_guard<std::mutex> lock(mutex_);
        currentState_ = state;
        lastMessage_ = message;
        info.state = state;
        info.retryCount = retryCount_;
        info.message = message;
    }
    NETMGR_EXT_LOG_I("EthEapState -> %{public}d, retryCount=%{public}d, msg=%{public}s",
        static_cast<int32_t>(state), info.retryCount, message.c_str());
    // 在锁外分发通知：跨进程回调可能阻塞，不能在持锁状态下执行
    NotifyStateChange(info);
}

EthEapState EthEapStateManager::GetState()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return currentState_;
}

void EthEapStateManager::GetStateInfo(EthEapStateInfo &info)
{
    // 锁内一次性读取状态/重试计数/最近消息，保证三者的一致性快照
    std::lock_guard<std::mutex> lock(mutex_);
    info.state = currentState_;
    info.retryCount = retryCount_;
    info.message = lastMessage_;
}

int32_t EthEapStateManager::GetRetryCount()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return retryCount_;
}

void EthEapStateManager::SetRetryCount(int32_t count)
{
    std::lock_guard<std::mutex> lock(mutex_);
    retryCount_ = count;
}

void EthEapStateManager::ResetRetryCount()
{
    std::lock_guard<std::mutex> lock(mutex_);
    retryCount_ = 0;
}

int32_t EthEapStateManager::RegisterCallback(const sptr<IEthEapStateCallback> &callback)
{
    if (callback == nullptr) {
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    // 去重：同一回调对象（同一 remote 对象）只注册一次
    for (auto &cb : callbacks_) {
        if (cb == callback) {
            NETMGR_EXT_LOG_D("RegisterCallback: already registered");
            return NETMANAGER_EXT_SUCCESS;
        }
    }
    callbacks_.push_back(callback);
    NETMGR_EXT_LOG_I("RegisterCallback success, total: %{public}zu", callbacks_.size());
    return NETMANAGER_EXT_SUCCESS;
}

int32_t EthEapStateManager::UnregisterCallback(const sptr<IEthEapStateCallback> &callback)
{
    if (callback == nullptr) {
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = callbacks_.begin(); it != callbacks_.end(); ++it) {
        if (*it == callback) {
            callbacks_.erase(it);
            NETMGR_EXT_LOG_I("UnregisterCallback success, total: %{public}zu", callbacks_.size());
            return NETMANAGER_EXT_SUCCESS;
        }
    }
    // 未找到也返回成功（幂等注销）
    NETMGR_EXT_LOG_D("UnregisterCallback: callback not found");
    return NETMANAGER_EXT_SUCCESS;
}

void EthEapStateManager::NotifyStateChange(const EthEapStateInfo &info)
{
    // 先拷贝回调列表再在锁外遍历分发：
    // 1) 避免持锁调用跨进程 IPC（可能阻塞或死锁）
    // 2) 防止回调内部反向调用 Register/Unregister 时与当前锁冲突
    std::vector<sptr<IEthEapStateCallback>> callbacksCopy;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        callbacksCopy = callbacks_;
    }
    for (auto &cb : callbacksCopy) {
        if (cb != nullptr) {
            cb->OnEthEapStateChanged(info);
        }
    }
}

} // namespace NetManagerStandard
} // namespace OHOS
