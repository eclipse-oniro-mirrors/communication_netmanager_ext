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

#ifndef ETH_EAP_AUTO_AUTH_CONTROLLER_H
#define ETH_EAP_AUTO_AUTH_CONTROLLER_H

#include <atomic>
#include <cstdint>
#include <mutex>
#include "eth_eap_config.h"

namespace OHOS {
namespace NetManagerStandard {

class EthEapAutoAuthController {
public:
    static EthEapAutoAuthController &GetInstance();

    void OnTriggerAutoAuth();
    void OnManualConnect();
    void OnLinkUp(const std::string &ifName);
    void OnBootCompleted();
    void OnEnterpriseAppOccupied();
    void OnEnterpriseAppReleased();
    void OnAuthSuccess();
    void OnAuthFailed(const std::string &reason);

    bool IsEnterpriseOccupied() const;
    void StopAutoAuth();
    void OnUserPaused();
    void OnUserResumed();
    bool IsUserPaused() const;

private:
    EthEapAutoAuthController() = default;
    ~EthEapAutoAuthController() = default;
    void TryStartAuth(bool manual);
    void ScheduleRetry(const std::string &reason);
    void DoAuthenticate(bool manual);
    void ScheduleAuthTimeoutCheck(uint32_t generation);

    std::mutex mutex_;
    std::atomic<bool> enterpriseOccupied_{false};
    std::atomic<bool> userPaused_{false};
    std::atomic<bool> authenticating_{false};
    std::atomic<bool> manualRound_{false};
    std::atomic<uint32_t> authGeneration_{0};
    std::string ifName_;
};

} // namespace NetManagerStandard
} // namespace OHOS
#endif // ETH_EAP_AUTO_AUTH_CONTROLLER_H
