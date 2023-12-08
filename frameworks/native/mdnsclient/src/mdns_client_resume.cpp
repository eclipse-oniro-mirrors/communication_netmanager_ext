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

namespace OHOS {
namespace NetManagerStandard {
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

RegisterServiceMap *MDnsClientResume::GetRegisterServiceMap()
{
    return &registerMap_;
}

DiscoverServiceMap *MDnsClientResume::GetStartDiscoverServiceMap()
{
    return &discoveryMap_;
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