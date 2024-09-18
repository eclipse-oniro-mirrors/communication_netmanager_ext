/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "mdns_client_resume.h"

#include <condition_variable>
#include <mutex>
#include <unistd.h>
#include <thread>

#include "iservice_registry.h"
#include "netmgr_ext_log_wrapper.h"

#include "mdns_common.h"
#include "mdns_client.h"

namespace OHOS {
namespace NetManagerStandard {
static constexpr uint32_t MAX_GET_SERVICE_COUNT = 30;
constexpr uint32_t WAIT_FOR_SERVICE_TIME_S = 1;
void MDnsClientResume::Init()
{
    NETMGR_EXT_LOG_I("MDnsClientResume Init");
    if (initFlag_) {
        NETMGR_EXT_LOG_I("MDnsClientResume initialization is complete");
        return;
    }
    initFlag_ = true;
}

MDnsClientResume &MDnsClientResume::GetInstance()
{
    static MDnsClientResume singleInstance_;
    static std::mutex mutex_;
    std::unique_lock<std::mutex> lock(mutex_);
    if (!singleInstance_.initFlag_) {
        singleInstance_.Init();
    }

    return singleInstance_;
}

void MDnsClientResume::ReRegisterService() 
{
    std::lock_guard<std::recursive_mutex> guard(registerMutex_);
    for (const auto& [key, value]: registerMap_) {
        auto ret = DelayedSingleton<MDnsClient>::GetInstance()->RegisterService(value, key);
        if (ret != NETMANAGER_EXT_SUCCESS) {
            NETMGR_EXT_LOG_E("ReRegisterService error, service name is %{public}s", value.name.c_str());
        }
    }
}

void MDnsClientResume::RestartDiscoverService() 
{
    std::lock_guard<std::recursive_mutex> guard(discoveryMutex_);
    for (const auto& [key, value]: discoveryMap_) {
        uint32_t count = 0;
        while (DelayedSingleton<MDnsClient>::GetInstance()->GetProxy() == nullptr && count < MAX_GET_SERVICE_COUNT) {
            std::this_thread::sleep_for(std::chrono::seconds(WAIT_FOR_SERVICE_TIME_S));
            count++;
        }
        auto proxy = DelayedSingleton<MDnsClient>::GetInstance()->GetProxy();
        NETMGR_EXT_LOG_W("Get proxy %{public}s, count: %{public}u", proxy == nullptr ? "failed" : "success", count);
        if (proxy != nullptr) {
            auto ret = DelayedSingleton<MDnsClient>::GetInstance()->StartDiscoverService(value, key);
            if (ret != NETMANAGER_EXT_SUCCESS) {
                NETMGR_EXT_LOG_E("RestartDiscoverService error, errorCode: %{public}d", ret);
            }
        }
    }
}

int32_t MDnsClientResume::SaveRegisterService(const MDnsServiceInfo &serviceInfo, const sptr<IRegistrationCallback> &cb)
{
    NETMGR_EXT_LOG_D("registerMap_.emplace......");
    std::lock_guard<std::recursive_mutex> guard(registerMutex_);
    registerMap_.emplace(cb, serviceInfo);
    NETMGR_EXT_LOG_D("registerMap_.emplace......[ok]");
    return 0;
}

int32_t MDnsClientResume::RemoveRegisterService(const sptr<IRegistrationCallback> &cb)
{
    std::lock_guard<std::recursive_mutex> guard(registerMutex_);
    auto itr = registerMap_.find(cb);
    if (registerMap_.end() != itr) {
        NETMGR_EXT_LOG_D("registerMap_.erase......");
        registerMap_.erase(itr);
        NETMGR_EXT_LOG_D("registerMap_.erase......[ok]");
    }
    return 0;
}

int32_t MDnsClientResume::SaveStartDiscoverService(const std::string &serviceType, const sptr<IDiscoveryCallback> &cb)
{
    NETMGR_EXT_LOG_D("discoveryMap_.emplace......");
    {
        std::lock_guard<std::recursive_mutex> guard(discoveryMutex_);
        if (discoveryMap_.find(cb) != discoveryMap_.end()) {
            NETMGR_EXT_LOG_D("discoveryMap_.emplace......[ok]");
            return 0;
        }
        discoveryMap_.emplace(cb, serviceType);
    }
    NETMGR_EXT_LOG_D("discoveryMap_.emplace......[ok]");
    
    return 0;
}

int32_t MDnsClientResume::RemoveStopDiscoverService(const sptr<IDiscoveryCallback> &cb)
{
    NETMGR_EXT_LOG_D("discoveryMap_.erase......");

    {
        std::lock_guard<std::recursive_mutex> guard(discoveryMutex_);
        auto itr = discoveryMap_.find(cb);
        if (discoveryMap_.end() != itr) {
            discoveryMap_.erase(itr);
        }
    }

    NETMGR_EXT_LOG_D("discoveryMap_.erase......[ok]");
    
    return 0;
}
} // namespace NetManagerStandard
} // namespace OHOS