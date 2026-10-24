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

#ifndef ETH_EAP_STATE_MANAGER_H
#define ETH_EAP_STATE_MANAGER_H

#include <mutex>
#include <vector>
#include "eth_eap_config.h"
#include "ieth_eap_state_callback.h"

namespace OHOS {
namespace NetManagerStandard {

class EthEapStateManager {
public:
    static EthEapStateManager &GetInstance();

    void SetState(EthEapState state, const std::string &message = "");
    EthEapState GetState();
    /** 原子读取当前完整状态信息（状态、重试计数、最近一次补充消息） */
    void GetStateInfo(EthEapStateInfo &info);
    int32_t GetRetryCount();
    void SetRetryCount(int32_t count);
    void ResetRetryCount();

    int32_t RegisterCallback(const sptr<IEthEapStateCallback> &callback);
    int32_t UnregisterCallback(const sptr<IEthEapStateCallback> &callback);

private:
    EthEapStateManager() = default;
    ~EthEapStateManager() = default;
    void NotifyStateChange(const EthEapStateInfo &info);

    std::mutex mutex_;
    EthEapState currentState_ = EthEapState::STATE_IDLE;
    int32_t retryCount_ = 0;
    std::string lastMessage_;
    std::vector<sptr<IEthEapStateCallback>> callbacks_;
};

} // namespace NetManagerStandard
} // namespace OHOS
#endif // ETH_EAP_STATE_MANAGER_H
