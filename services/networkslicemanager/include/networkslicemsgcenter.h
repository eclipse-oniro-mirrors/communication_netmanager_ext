/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef METWORKSLICE_MSGCENTER_H
#define METWORKSLICE_MSGCENTER_H

#include <map>
#include <shared_mutex>
#include "singleton.h"
#include "event_handler.h"
#include "networkslice_submodule.h"
#include "networkslice_event.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
class NetworkSliceMsgCenter final {
    DECLARE_SINGLETON(NetworkSliceMsgCenter);
public:
    void registHandler(NetworkSliceSubModule moduleId, std::shared_ptr<AppExecFwk::EventHandler> handler);
    void unRegistHandler(NetworkSliceSubModule moduleId, std::shared_ptr<AppExecFwk::EventHandler> handler);
    void Subscribe(NetworkSliceEvent event, NetworkSliceSubModule moduleId);
    void UnSubscribe(NetworkSliceEvent event, NetworkSliceSubModule moduleId);

    template<typename... Args>
    void Publish(NetworkSliceEvent event, Args... args);
    template<typename... Args>
    void PublishDelay(NetworkSliceEvent event, int64_t delayTime, Args... args);
private:
    static std::shared_timed_mutex mutex_;
    std::map<NetworkSliceSubModule,
        std::list<std::shared_ptr<AppExecFwk::EventHandler>>> moduleIdHandlerListMap_;
    std::map<NetworkSliceEvent, uint64_t> eventHandlerMap_;
    std::map<uint64_t, uint64_t> eventSubscribeMap_;
};

template<typename... Args>
void NetworkSliceMsgCenter::Publish(NetworkSliceEvent event, Args... args)
{
    NETMGR_EXT_LOG_I("enter event:%{public}d", event);
    std::unique_lock<std::shared_timed_mutex> lock(mutex_);
    auto it = eventHandlerMap_.find(event);
    if (it == eventHandlerMap_.end()) {
        NETMGR_EXT_LOG_I("event:%{public}d, no module subscribed!", event);
        return;
    }
    for (int i = MODULE_COMMON; i < MODULE_BUTT; i++) {
        if ((it->second & (1 << i)) == 0) {
            continue;
        }
        auto itHandlerList = moduleIdHandlerListMap_.find(static_cast<NetworkSliceSubModule>(i));
        if (itHandlerList == moduleIdHandlerListMap_.end()) {
            continue;
        }
        for (auto itHandler : itHandlerList->second) {
            if (sizeof...(args) > 0) {
                itHandler->SendEvent(event, args..., 0);
            } else {
                itHandler->SendEvent(event);
            }
        }
    }
}
 
template<typename... Args>
void NetworkSliceMsgCenter::PublishDelay(NetworkSliceEvent event, int64_t delayTime, Args... args)
{
    NETMGR_EXT_LOG_I("enter event:%{public}d", event);
    std::unique_lock<std::shared_timed_mutex> lock(mutex_);
    auto it = eventHandlerMap_.find(event);
    if (it == eventHandlerMap_.end()) {
        NETMGR_EXT_LOG_I("event:%{public}d, not fined!", event);
        return;
    }
    for (int i = MODULE_COMMON; i < MODULE_BUTT; i++) {
        if ((it->second & (1 << i)) == 0) {
            continue;
        }
        auto itHandlerList = moduleIdHandlerListMap_.find(static_cast<NetworkSliceSubModule>(i));
        if (itHandlerList == moduleIdHandlerListMap_.end()) {
            continue;
        }
        NETMGR_EXT_LOG_I("module:%{public}d SendEvent!", i);
        for (auto itHandler : itHandlerList->second) {
            itHandler->SendEvent(event, args..., delayTime);
        }
    }
}

}
}
#endif
