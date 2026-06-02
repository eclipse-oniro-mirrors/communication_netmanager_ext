/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef NET_TRAFFICFILTER_ADAPTER_H
#define NET_TRAFFICFILTER_ADAPTER_H

#include <mutex>
#include <string>
#include <map>
#include "net_trafficfilter_type.h"
struct OH_TrafficFilter_Redirector {
};

namespace OHOS {
namespace NetManagerStandard {
constexpr uint32_t CONNECTION_INFO_MIN_SIZE =
    static_cast<uint32_t>(offsetof(OH_TrafficFilter_ConnectionInfo, protocol) + sizeof(uint8_t));
constexpr uint32_t PROCESS_INFO_MIN_SIZE =
    static_cast<uint32_t>(offsetof(OH_TrafficFilter_ProcessInfo, size) + sizeof(uint32_t));
constexpr uint32_t REDIRECT_RULE_MIN_SIZE =
    static_cast<uint32_t>(offsetof(OH_TrafficFilter_RedirectRule, proxyPort) + sizeof(uint16_t));

class RedirectorAdapterManager {
public:
    static RedirectorAdapterManager& GetInstance();

    int32_t CreateRedirector(uint32_t group_id, uint32_t priority, OH_TrafficFilter_Redirector** redirector);

    int32_t DestroyRedirector(OH_TrafficFilter_Redirector* redirector);

    int32_t AddRedirectRule(OH_TrafficFilter_Redirector* redirector, const OH_TrafficFilter_RedirectRule* rule);

    int32_t ClearRedirectRule(OH_TrafficFilter_Redirector* redirector);

    int32_t QueryProcess(const OH_TrafficFilter_ConnectionInfo* connectionInfo,
        OH_TrafficFilter_ProcessInfo* processInfo);

private:
    RedirectorAdapterManager() = default;
    ~RedirectorAdapterManager() = default;

    int32_t AddRedirector(const std::string& redirectorId, OH_TrafficFilter_Redirector** redirector);
    bool GetRedirectorId(OH_TrafficFilter_Redirector* redirector, std::string& redirectorId);
    void RemoveRedirector(OH_TrafficFilter_Redirector* redirector);

    std::mutex mapMutex_;
    std::map<OH_TrafficFilter_Redirector*, std::string> redirectorIdMap_;
};

}
}

#endif /* NET_TRAFFICFILTER_ADAPTER_H */