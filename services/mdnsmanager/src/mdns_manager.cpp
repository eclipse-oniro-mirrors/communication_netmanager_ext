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

#include "mdns_manager.h"

#include <unistd.h>

#include "mdns_event_proxy.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {

MDnsManager &MDnsManager::GetInstance()
{
    static MDnsManager sInstance;
    return sInstance;
}

MDnsManager::MDnsManager() {}

void MDnsManager::RestartMDnsProtocolImpl()
{
    NETMGR_EXT_LOG_D("mdns_log Network switching");
    impl.Init();
    RestartDiscoverService();
}

int32_t MDnsManager::RegisterService(const MDnsServiceInfo &serviceInfo, const sptr<IRegistrationCallback> &cb)
{
    NETMGR_EXT_LOG_D("mdns_log RegisterService");
    if (cb == nullptr || cb->AsObject() == nullptr) {
        NETMGR_EXT_LOG_E("mdns_log callback is nullptr");
        return NET_MDNS_ERR_ILLEGAL_ARGUMENT;
    }

    MDnsProtocolImpl::Result result{.serviceName = serviceInfo.name,
                                    .serviceType = serviceInfo.type,
                                    .port = serviceInfo.port,
                                    .txt = serviceInfo.txtRecord};

    int32_t err = impl.Register(result);
    impl.AddTask([this, cb, serviceInfo, err]() {
        cb->HandleRegisterResult(serviceInfo, err);
        return true;
    });

    if (err == NETMANAGER_EXT_SUCCESS) {
        std::lock_guard<std::recursive_mutex> guard(registerMutex_);
        registerMap_.emplace(cb, serviceInfo.name + MDNS_DOMAIN_SPLITER_STR + serviceInfo.type);
    }
    return err;
}

int32_t MDnsManager::UnRegisterService(const sptr<IRegistrationCallback> &cb)
{
    NETMGR_EXT_LOG_D("mdns_log UnRegisterService");
    if (cb == nullptr || cb->AsObject() == nullptr) {
        NETMGR_EXT_LOG_E("mdns_log callback is nullptr");
        return NET_MDNS_ERR_ILLEGAL_ARGUMENT;
    }

    auto itr = registerMap_.find(cb);
    if (registerMap_.end() == itr) {
        NETMGR_EXT_LOG_W("mdns_log find registrer map failed");
        return NET_MDNS_ERR_CALLBACK_NOT_FOUND;
    }

    int32_t err = impl.UnRegister(itr->second);
    if (err == NETMANAGER_EXT_SUCCESS) {
        std::lock_guard<std::recursive_mutex> guard(registerMutex_);
        registerMap_.erase(itr);
    }
    return err;
}

int32_t MDnsManager::StartDiscoverService(const std::string &serviceType, const sptr<IDiscoveryCallback> &cb)
{
    NETMGR_EXT_LOG_D("mdns_log StartDiscoverService");
    if (cb == nullptr || cb->AsObject() == nullptr) {
        NETMGR_EXT_LOG_E("callback is nullptr");
        return NET_MDNS_ERR_ILLEGAL_ARGUMENT;
    }

    if (!IsTypeValid(serviceType)) {
        return NET_MDNS_ERR_ILLEGAL_ARGUMENT;
    }
    std::string name = impl.Decorated(serviceType);
    if (!IsDomainValid(name)) {
        return NET_MDNS_ERR_ILLEGAL_ARGUMENT;
    }

    {
        std::lock_guard<std::recursive_mutex> guard(discoveryMutex_);
        if (discoveryMap_.find(cb) != discoveryMap_.end()) {
            return NET_MDNS_ERR_CALLBACK_DUPLICATED;
        }
        discoveryMap_.emplace(cb, serviceType);
    }
    return impl.Discovery(serviceType, cb);
}

int32_t MDnsManager::StopDiscoverService(const sptr<IDiscoveryCallback> &cb)
{
    NETMGR_EXT_LOG_D("mdns_log StopDiscoverService");
    if (cb == nullptr || cb->AsObject() == nullptr) {
        NETMGR_EXT_LOG_E("mdns_log callback is nullptr");
        return NET_MDNS_ERR_ILLEGAL_ARGUMENT;
    }
    std::string key;
    {
        std::lock_guard<std::recursive_mutex> guard(discoveryMutex_);
        auto local = discoveryMap_.find(cb);
        if (local == discoveryMap_.end()) {
            return NET_MDNS_ERR_CALLBACK_NOT_FOUND;
        }
        key = local->second;
        discoveryMap_.erase(local);
    }
    return impl.StopCbMap(key);
}

void MDnsManager::RestartDiscoverService()
{
    NETMGR_EXT_LOG_D("mdns_log RestartDiscoverService");
    std::lock_guard<std::recursive_mutex> guard(discoveryMutex_);
    for (const auto &it : discoveryMap_) {
        auto cb = it.first;
        if (cb == nullptr || cb->AsObject() == nullptr) {
            NETMGR_EXT_LOG_E("mdns_log callback is nullptr");
            continue;
        }
        auto serviceType = it.second;
        impl.StopCbMap(serviceType);
        impl.Discovery(serviceType, cb);
    }
}

int32_t MDnsManager::ResolveService(const MDnsServiceInfo &serviceInfo, const sptr<IResolveCallback> &cb)
{
    NETMGR_EXT_LOG_D("mdns_log ResolveService");
    if (cb == nullptr || cb->AsObject() == nullptr) {
        NETMGR_EXT_LOG_E("mdns_log callback is nullptr");
        return NET_MDNS_ERR_ILLEGAL_ARGUMENT;
    }

    std::string instance = serviceInfo.name + MDNS_DOMAIN_SPLITER_STR + serviceInfo.type;
    return impl.ResolveInstance(instance, cb);
}

void MDnsManager::GetDumpMessage(std::string &message)
{
    message.append("mDNS Info:\n");
    const auto &config = impl.GetConfig();
    message.append("\tIPv6 Support: " + std::to_string(config.ipv6Support) + "\n");
    message.append("\tAll Iface: " + std::to_string(config.configAllIface) + "\n");
    message.append("\tTop Domain: " + config.topDomain + "\n");
    message.append("\tHostname: " + config.hostname + "\n");
    message.append("\tImpl Service Count: " + std::to_string(impl.srvMap_.size()) + "\n");
    message.append("\tDiscovery Count: " + std::to_string(discoveryMap_.size()) + "\n");
}

bool MDnsManager::IsAvailableCallback(const sptr<IDiscoveryCallback> &cb)
{
    std::lock_guard<std::recursive_mutex> guard(discoveryMutex_);
    return cb != nullptr && discoveryMap_.find(cb) != discoveryMap_.end();
}
} // namespace NetManagerStandard
} // namespace OHOS