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
#include <unordered_map>
#include "net_trafficfilter_type.h"
struct OH_TrafficFilter_Redirector {
};

namespace OHOS {
namespace NetManagerStandard {

class RedirectorAdapterManager {
public:
    static RedirectorAdapterManager& GetInstance();

    int32_t CreateRedirector(uint32_t group_id, uint32_t priority,
        const OH_TrafficFilter_Config* config, OH_TrafficFilter_Redirector** redirector);

    void DestroyRedirector(OH_TrafficFilter_Redirector* redirector);

    int32_t AddRedirectRule(OH_TrafficFilter_Redirector* redirector, const OH_TrafficFilter_RedirectRule* rule);

    int32_t ClearRedirectRule(OH_TrafficFilter_Redirector* redirector);

    int32_t QueryProcess(const char* src_ip, uint16_t src_port, const char* dst_ip,
        uint16_t dst_port, uint8_t protocol, OH_TrafficFilter_ProcessInfo* process_info);

private:
    RedirectorAdapterManager() = default;
    ~RedirectorAdapterManager() = default;

    int32_t AddRedirector(const std::string& redirectorId, OH_TrafficFilter_Redirector** redirector);
    bool GetRedirectorId(OH_TrafficFilter_Redirector* redirector, std::string& redirectorId);
    void RemoveRedirector(OH_TrafficFilter_Redirector* redirector);

    std::mutex mapMutex_;
    std::unordered_map<OH_TrafficFilter_Redirector*, std::string> redirectorIdMap_;
};

}
}

#endif /* NET_TRAFFICFILTER_ADAPTER_H */