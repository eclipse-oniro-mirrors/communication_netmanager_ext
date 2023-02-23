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

namespace {
constexpr int DEFAULT_RESOLVE_TIMEOUT = 5000;
}

MDnsManager::MDnsManager()
{
    InitHandler();
}

int32_t MDnsManager::RegisterService(const MDnsServiceInfo &serviceInfo, const sptr<IRegistrationCallback> &cb)
{
    if (cb == nullptr) {
        NETMGR_EXT_LOG_E("callback is nullptr");
        return NET_MDNS_ERR_ILLEGAL_ARGUMENT;
    }
    {
        std::lock_guard<std::mutex> guard(mutex_);
        auto iter =
            std::find_if(registerMap_.begin(), registerMap_.end(), [&](const auto &x) { return x.first == cb; });
        if (iter != registerMap_.end()) {
            return NET_MDNS_ERR_CALLBACK_DUPLICATED;
        }
        registerMap_.emplace_back(cb, serviceInfo.name + MDNS_DOMAIN_SPLITER_STR + serviceInfo.type);
    }
    MDnsProtocolImpl::Result result{.serviceName = serviceInfo.name,
                                    .serviceType = serviceInfo.type,
                                    .port = serviceInfo.port,
                                    .txt = serviceInfo.txtRecord};
    int32_t err = impl.Register(result);
    std::thread([this, cb, serviceInfo, err]() {
        cb->HandleRegister(serviceInfo, err);
        cb->HandleRegisterResult(serviceInfo, err);
    }).detach();
    return err;
}

int32_t MDnsManager::UnRegisterService(const sptr<IRegistrationCallback> &cb)
{
    if (cb == nullptr) {
        NETMGR_EXT_LOG_E("callback is nullptr");
        return NET_MDNS_ERR_ILLEGAL_ARGUMENT;
    }
    std::string name;
    bool found = false;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        auto iter =
            std::find_if(registerMap_.begin(), registerMap_.end(), [&](const auto &x) { return x.first == cb; });
        found = (iter != registerMap_.end());
        if (found) {
            name = iter->second;
        }
    }
    int32_t err = 0;
    if (found) {
        err = impl.UnRegister(name);
    } else {
        err = NET_MDNS_ERR_CALLBACK_NOT_FOUND;
    }
    MDnsServiceInfo info;
    impl.ExtractNameAndType(name, info.name, info.type);
    std::thread([this, cb, info, err]() { cb->HandleUnRegister(info, err); }).detach();
    return err;
}

int32_t MDnsManager::StartDiscoverService(const std::string &serviceType, const sptr<IDiscoveryCallback> &cb)
{
    if (cb == nullptr) {
        NETMGR_EXT_LOG_E("callback is nullptr");
        return NET_MDNS_ERR_ILLEGAL_ARGUMENT;
    }
    {
        std::lock_guard<std::mutex> guard(mutex_);
        if (std::find_if(discoveryMap_.begin(), discoveryMap_.end(), [&](const auto &x) { return x.first == cb; }) !=
            discoveryMap_.end()) {
            return NET_MDNS_ERR_CALLBACK_DUPLICATED;
        }
        discoveryMap_.emplace_back(cb, serviceType);
    }
    MDnsServiceInfo info;
    info.type = serviceType;
    int32_t err = impl.Discovery(serviceType);
    std::thread([this, cb, info, err]() { cb->HandleStartDiscover(info, err); }).detach();
    return err;
}

int32_t MDnsManager::StopDiscoverService(const sptr<IDiscoveryCallback> &cb)
{
    if (cb == nullptr) {
        NETMGR_EXT_LOG_E("callback is nullptr");
        return NET_MDNS_ERR_ILLEGAL_ARGUMENT;
    }
    std::string name;
    bool found = false;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        auto iter =
            std::find_if(discoveryMap_.begin(), discoveryMap_.end(), [&](const auto &x) { return x.first == cb; });
        found = (iter != discoveryMap_.end());
        if (found) {
            name = iter->second;
        }
    }

    int32_t err = 0;
    if (found) {
        err = impl.StopDiscovery(name);
    } else {
        err = NET_MDNS_ERR_CALLBACK_NOT_FOUND;
    }
    MDnsServiceInfo info;
    info.type = name;
    std::thread([this, cb, info, err]() { cb->HandleStopDiscover(info, err); }).detach();
    return err;
}

int32_t MDnsManager::ResolveService(const MDnsServiceInfo &serviceInfo, const sptr<IResolveCallback> &cb)
{
    if (cb == nullptr) {
        NETMGR_EXT_LOG_E("callback is nullptr");
        return NET_MDNS_ERR_ILLEGAL_ARGUMENT;
    }
    std::string instance = serviceInfo.name + MDNS_DOMAIN_SPLITER_STR + serviceInfo.type;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        if (std::find_if(resolveMap_.begin(), resolveMap_.end(), [&](const auto &x) { return x.first == cb; }) !=
            resolveMap_.end()) {
            return NET_MDNS_ERR_CALLBACK_DUPLICATED;
        }
        resolveMap_.emplace_back(cb, instance);
    }
    std::thread([this, cb, instance]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(DEFAULT_RESOLVE_TIMEOUT));
        std::lock_guard<std::mutex> guard(mutex_);
        auto iter = std::find_if(resolveMap_.begin(), resolveMap_.end(), [&](const auto &x) { return x.first == cb; });
        if (iter != resolveMap_.end()) {
            MDnsServiceInfo info;
            impl.StopResolveInstance(instance);
            cb->HandleResolveResult(info, NET_MDNS_ERR_TIMEOUT);
            resolveMap_.erase(iter);
        }
    }).detach();
    return impl.ResolveInstance(instance);
}

void MDnsManager::InitHandler()
{
    static auto handle = [this](const MDnsProtocolImpl::Result &result, int32_t error) {
        ReceiveResult(result, error);
    };
    impl.SetHandler(handle);
}

void MDnsManager::ReceiveResult(const MDnsProtocolImpl::Result &result, int32_t error)
{
    switch (result.type) {
        case MDnsProtocolImpl::SERVICE_STARTED:
            [[fallthrough]];
        case MDnsProtocolImpl::SERVICE_STOPED:
            return ReceiveRegister(result, error);
        case MDnsProtocolImpl::SERVICE_FOUND:
            [[fallthrough]];
        case MDnsProtocolImpl::SERVICE_LOST:
            return ReceiveDiscover(result, error);
        case MDnsProtocolImpl::INSTANCE_RESOLVED:
            return ReceiveInstanceResolve(result, error);
        case MDnsProtocolImpl::DOMAIN_RESOLVED:
            return ReceiveResolve(result, error);
        case MDnsProtocolImpl::UNKNOWN:
            [[fallthrough]];
        default:
            return;
    }
}

void MDnsManager::ReceiveRegister(const MDnsProtocolImpl::Result &result, int32_t error)
{
    sptr<IRegistrationCallback> cb(nullptr);
    {
        std::lock_guard<std::mutex> guard(mutex_);
        auto iter = std::find_if(registerMap_.begin(), registerMap_.end(), [&](const auto &x) {
            return x.second == result.serviceName + MDNS_DOMAIN_SPLITER_STR + result.serviceType;
        });
        if (iter == registerMap_.end()) {
            return;
        }
        cb = iter->first;
    }
    if (!cb) {
        return;
    }
    cb->HandleRegisterResult(ConvertResultToInfo(result), error);
}

void MDnsManager::ReceiveDiscover(const MDnsProtocolImpl::Result &result, int32_t error)
{
    std::vector<sptr<IDiscoveryCallback>> callbacks;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        NETMGR_EXT_LOG_D("discoveryMap_ size: [%{public}zu]", discoveryMap_.size());
        for (auto iter = discoveryMap_.begin(); iter != discoveryMap_.end(); ++iter) {
            iter =
                std::find_if(iter, discoveryMap_.end(), [&](const auto &x) { return x.second == result.serviceType; });
            if (iter == discoveryMap_.end()) {
                break;
            }
            callbacks.emplace_back(iter->first);
        }
    }
    MDnsServiceInfo info;
    info.name = result.serviceName;
    info.type = result.serviceType;
    for (auto &&cb : callbacks) {
        if (result.type == MDnsProtocolImpl::SERVICE_FOUND) {
            cb->HandleServiceFound(info, error);
        }
        if (result.type == MDnsProtocolImpl::SERVICE_LOST) {
            cb->HandleServiceLost(info, error);
        }
    }
}

void MDnsManager::ReceiveInstanceResolve(const MDnsProtocolImpl::Result &result, int32_t error)
{
    std::lock_guard<std::mutex> guard(mutex_);
    if (result.addr.empty() && !result.domain.empty()) {
        resolvResults_.emplace_back(result);
        impl.Resolve(resolvResults_.back().domain);
        return;
    }
    auto iter = resolveMap_.end();
    sptr<IResolveCallback> cb(nullptr);
    iter = std::find_if(resolveMap_.begin(), resolveMap_.end(), [&](const auto &x) {
        return x.second == result.serviceName + MDNS_DOMAIN_SPLITER_STR + result.serviceType;
    });
    if (iter == resolveMap_.end()) {
        return;
    }
    cb = iter->first;
    impl.StopResolveInstance(result.serviceName + MDNS_DOMAIN_SPLITER_STR + result.serviceType);
    cb->HandleResolveResult(ConvertResultToInfo(result), error);
    resolveMap_.erase(iter);
}

void MDnsManager::ReceiveResolve(const MDnsProtocolImpl::Result &result, int32_t error)
{
    auto iter = resolveMap_.end();
    auto res = resolvResults_.end();
    sptr<IResolveCallback> cb(nullptr);
    std::lock_guard<std::mutex> guard(mutex_);
    res = std::find_if(resolvResults_.begin(), resolvResults_.end(),
                       [&](const auto &x) { return x.domain == result.domain; });
    if (res == resolvResults_.end()) {
        return;
    }
    res->ipv6 = result.ipv6;
    res->addr = result.addr;
    iter = std::find_if(resolveMap_.begin(), resolveMap_.end(), [&](const auto &x) {
        return x.second == res->serviceName + MDNS_DOMAIN_SPLITER_STR + res->serviceType;
    });
    if (iter == resolveMap_.end()) {
        return;
    }
    cb = iter->first;
    impl.StopResolve(res->domain);
    impl.StopResolveInstance(result.serviceName + MDNS_DOMAIN_SPLITER_STR + result.serviceType);
    cb->HandleResolveResult(ConvertResultToInfo(*res), error);
    resolvResults_.erase(res);
    resolveMap_.erase(iter);
}

MDnsServiceInfo MDnsManager::ConvertResultToInfo(const MDnsProtocolImpl::Result &result)
{
    MDnsServiceInfo info;
    info.name = result.serviceName;
    info.type = result.serviceType;
    if (!result.addr.empty()) {
        info.family = result.ipv6 ? MDnsServiceInfo::IPV6 : MDnsServiceInfo::IPV4;
    }
    info.addr = result.addr;
    info.port = result.port;
    info.txtRecord = result.txt;
    return info;
}

void MDnsManager::GetDumpMessage(std::string &message)
{
    message.append("mDNS Info:\n");
    const auto &config = impl.GetConfig();
    message.append("\tIPv6 Support: " + std::to_string(config.ipv6Support) + "\n");
    message.append("\tAll Iface: " + std::to_string(config.configAllIface) + "\n");
    message.append("\tTop Domain: " + config.topDomain + "\n");
    message.append("\tHostname: " + config.hostname + "\n");
    message.append("\tService Count: " + std::to_string(registerMap_.size()) + "\n");
}
} // namespace NetManagerStandard
} // namespace OHOS