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

#include "networkslicemsgcenter.h"

namespace OHOS {
namespace NetManagerStandard {
NetworkSliceMsgCenter::NetworkSliceMsgCenter() = default;
NetworkSliceMsgCenter::~NetworkSliceMsgCenter() = default;

std::shared_timed_mutex NetworkSliceMsgCenter::mutex_ = {};

void NetworkSliceMsgCenter::registHandler(NetworkSliceSubModule moduleId,
    std::shared_ptr<AppExecFwk::EventHandler> handler)
{
    if (handler == nullptr) {
        return;
    }
    std::unique_lock<std::shared_timed_mutex> lock(mutex_);
    auto itHandlerList = moduleIdHandlerListMap_.find(moduleId);
    if (itHandlerList != moduleIdHandlerListMap_.end()) {
        auto &handleList = itHandlerList->second;
        if (std::find(handleList.begin(), handleList.end(), handler) == handleList.end()) {
            handleList.push_back(handler);
        }
    } else {
        moduleIdHandlerListMap_[moduleId] = { handler };
    }
}

void NetworkSliceMsgCenter::unRegistHandler(NetworkSliceSubModule moduleId,
    std::shared_ptr<AppExecFwk::EventHandler> handler)
{
    if (handler == nullptr) {
        return;
    }
    std::unique_lock<std::shared_timed_mutex> lock(mutex_);
    auto itHandlerList = moduleIdHandlerListMap_.find(moduleId);
    if (itHandlerList != moduleIdHandlerListMap_.end()) {
        itHandlerList->second.remove(handler);
    }
    if (itHandlerList == moduleIdHandlerListMap_.end() || itHandlerList->second.empty()) {
        for (auto it = eventHandlerMap_.begin(); it != eventHandlerMap_.end(); it++) {
            it->second &= ~(1 << moduleId);
            uint64_t key = (static_cast<uint64_t>(it->first) << 32) &  static_cast<uint64_t>(moduleId);
            eventSubscribeMap_.erase(key);
        }
        moduleIdHandlerListMap_.erase(moduleId);
    }
}

void NetworkSliceMsgCenter::Subscribe(NetworkSliceEvent event, NetworkSliceSubModule moduleId)
{
    NETMGR_EXT_LOG_I("event:%{public}d, module:%{public}d", event, moduleId);
    std::unique_lock<std::shared_timed_mutex> lock(mutex_);
    auto it = eventHandlerMap_.find(event);
    if (it != eventHandlerMap_.end()) {
        it->second |= (1 << moduleId);
    } else {
        eventHandlerMap_.insert(std::make_pair(event, 1 << moduleId));
    }
    uint64_t key = (static_cast<uint64_t>(event) << 32) &  static_cast<uint64_t>(moduleId);
    eventSubscribeMap_[key]++;
}

void NetworkSliceMsgCenter::UnSubscribe(NetworkSliceEvent event, NetworkSliceSubModule moduleId)
{
    uint64_t key = (static_cast<uint64_t>(event) << 32) &  static_cast<uint64_t>(moduleId);
    auto it = eventSubscribeMap_.find(key);
    if (it == eventSubscribeMap_.end() || it->second <= 1) {
        auto it = eventHandlerMap_.find(event);
        if (it != eventHandlerMap_.end()) {
            it->second &= ~(1 << moduleId);
        }
        eventSubscribeMap_.erase(key);
    } else {
        eventSubscribeMap_[key]--;
    }
}

}
}
