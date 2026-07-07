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

#include "mdns_ani.h"

#include <chrono>
#include <cstdint>

#include "cxx.h"
#include "net_manager_ext_constants.h"
#include "net_manager_constants.h"
#include "errorcode_convertor.h"
#include "wrapper.rs.h"

namespace OHOS {
namespace NetManagerAni {

// Accessor functions for callback maps and mutexes.
// Using local static variables (Meyers' singleton) ensures deterministic initialization
// order and thread-safe lazy initialization (guaranteed by C++11).
std::map<std::string, sptr<MDnsRegistrationCallbackAni>>& GetRegisterCallbackMap()
{
    static std::map<std::string, sptr<MDnsRegistrationCallbackAni>> map;
    return map;
}
std::mutex& GetRegisterMutex()
{
    static std::mutex mtx;
    return mtx;
}

std::map<std::string, sptr<MDnsDiscoveryCallbackAni>>& GetDiscoveryCallbackMap()
{
    static std::map<std::string, sptr<MDnsDiscoveryCallbackAni>> map;
    return map;
}
std::mutex& GetDiscoveryMutex()
{
    static std::mutex mtx;
    return mtx;
}

std::map<std::string, sptr<MDnsResolveCallbackAni>>& GetResolveCallbackMap()
{
    static std::map<std::string, sptr<MDnsResolveCallbackAni>> map;
    return map;
}
std::mutex& GetResolveMutex()
{
    static std::mutex mtx;
    return mtx;
}

static sptr<MDnsDiscoveryCallbackAni> g_mdnsDiscCallback = nullptr;
static bool g_isMDnsObserverRegistered = false;
static std::mutex g_observerMutex;

// ---- Attribute conversion helpers ----

static void SetServiceInfoAttrs(const MDnsServiceInfoFFI &ffiInfo, NetManagerStandard::MDnsServiceInfo &info)
{
    if (ffiInfo.attr_keys.empty()) {
        return;
    }
    NetManagerStandard::TxtRecord attrMap;
    size_t offset = 0;
    for (size_t i = 0; i < ffiInfo.attr_keys.size(); ++i) {
        if (i >= ffiInfo.attr_value_lengths.size()) {
            break;
        }
        int32_t len = ffiInfo.attr_value_lengths[i];
        if (len <= 0 || offset + static_cast<size_t>(len) > ffiInfo.attr_values.size()) {
            break;
        }
        std::string key = std::string(ffiInfo.attr_keys[i]);
        std::vector<uint8_t> val(
            ffiInfo.attr_values.data() + offset,
            ffiInfo.attr_values.data() + offset + static_cast<size_t>(len)
        );
        attrMap[key] = std::move(val);
        offset += static_cast<size_t>(len);
    }
    info.SetAttrMap(attrMap);
}

static void GetServiceInfoAttrs(const NetManagerStandard::MDnsServiceInfo &info, MDnsServiceInfoFFI &ffiInfo)
{
    // GetAttrMap() is not const, but we only read the map
    NetManagerStandard::TxtRecord attrMap = const_cast<NetManagerStandard::MDnsServiceInfo &>(info).GetAttrMap();
    for (const auto &[key, val] : attrMap) {
        ffiInfo.attr_keys.push_back(rust::String(key));
        for (uint8_t b : val) {
            ffiInfo.attr_values.push_back(b);
        }
        ffiInfo.attr_value_lengths.push_back(static_cast<int32_t>(val.size()));
    }
}

// ---- Build FFI struct from MDnsServiceInfo (used by callbacks) ----
static MDnsServiceInfoFFI BuildServiceInfoFFI(const NetManagerStandard::MDnsServiceInfo &serviceInfo)
{
    MDnsServiceInfoFFI info{};
    info.name = rust::String(serviceInfo.name);
    info.type_ = rust::String(serviceInfo.type);
    info.family = serviceInfo.family;
    info.addr = rust::String(serviceInfo.addr);
    info.port = serviceInfo.port;
    GetServiceInfoAttrs(serviceInfo, info);
    return info;
}

// ---- Error code / message ----

rust::String GetErrorCodeAndMessage(int32_t &errorCode)
{
    NetManagerStandard::NetBaseErrorCodeConvertor convertor;
    rust::String result(convertor.ConvertErrorCode(errorCode));
    return result;
}

// ---- FFI Functions ----

MDnsServiceInfoFFI RegisterService(const rust::String &bundleName, const MDnsServiceInfoFFI &serviceInfo,
    int32_t &ret)
{
    auto client = DelayedSingleton<NetManagerStandard::MDnsClient>::GetInstance();
    NetManagerStandard::MDnsServiceInfo info;
    info.name = std::string(serviceInfo.name);
    info.type = std::string(serviceInfo.type_);
    info.family = serviceInfo.family;
    info.addr = std::string(serviceInfo.addr);
    info.port = serviceInfo.port;
    SetServiceInfoAttrs(serviceInfo, info);

    std::string bName = std::string(bundleName);
    // Validate inputs do not contain separator characters
    if (bName.find('\x01') != std::string::npos || bName.find('\x02') != std::string::npos ||
        info.name.find('\x01') != std::string::npos || info.name.find('\x02') != std::string::npos ||
        info.type.find('\x01') != std::string::npos || info.type.find('\x02') != std::string::npos) {
        ret = NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
        return serviceInfo;
    }

    std::string key = bName + "\x01" + info.name + "\x02" + info.type;

    sptr<MDnsRegistrationCallbackAni> cb;
    bool isNewCallback = false;
    {
        std::lock_guard<std::mutex> lock(GetRegisterMutex());
        auto it = GetRegisterCallbackMap().find(key);
        if (it != GetRegisterCallbackMap().end()) {
            cb = it->second;
        } else {
            cb = new (std::nothrow) MDnsRegistrationCallbackAni();
            if (cb == nullptr) {
                ret = NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
                return serviceInfo;
            }
            GetRegisterCallbackMap()[key] = cb;
            isNewCallback = true;
        }
    }

    ret = client->RegisterService(info, cb);
    if (ret != NetManagerStandard::NETMANAGER_EXT_SUCCESS && isNewCallback) {
        std::lock_guard<std::mutex> lock(GetRegisterMutex());
        GetRegisterCallbackMap().erase(key);
        // Release the local sptr reference so the object is properly freed
        cb = nullptr;
    }
    return serviceInfo;
}

int32_t UnRegisterService(const rust::String &bundleName, const MDnsServiceInfoFFI &serviceInfo, int32_t &ret)
{
    auto client = DelayedSingleton<NetManagerStandard::MDnsClient>::GetInstance();
    NetManagerStandard::MDnsServiceInfo info;
    info.name = std::string(serviceInfo.name);
    info.type = std::string(serviceInfo.type_);
    info.family = serviceInfo.family;
    info.addr = std::string(serviceInfo.addr);
    info.port = serviceInfo.port;

    std::string bName = std::string(bundleName);
    // Validate inputs do not contain separator characters
    if (bName.find('\x01') != std::string::npos || bName.find('\x02') != std::string::npos ||
        info.name.find('\x01') != std::string::npos || info.name.find('\x02') != std::string::npos ||
        info.type.find('\x01') != std::string::npos || info.type.find('\x02') != std::string::npos) {
        ret = NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
        return ret;
    }

    std::string key = bName + "\x01" + info.name + "\x02" + info.type;

    sptr<NetManagerStandard::IRegistrationCallback> cb;
    {
        std::lock_guard<std::mutex> lock(GetRegisterMutex());
        auto it = GetRegisterCallbackMap().find(key);
        if (it != GetRegisterCallbackMap().end()) {
            cb = it->second;
        }
    }

    if (cb == nullptr) {
        ret = NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
        return ret;
    }

    ret = client->UnRegisterService(cb);
    std::lock_guard<std::mutex> lock(GetRegisterMutex());
    GetRegisterCallbackMap().erase(key);
    return ret;
}

int32_t StartDiscoverService(const rust::String &serviceType, int32_t &ret)
{
    auto client = DelayedSingleton<NetManagerStandard::MDnsClient>::GetInstance();

    sptr<MDnsDiscoveryCallbackAni> cb;
    bool isNewCallback = false;
    {
        std::lock_guard<std::mutex> lock(GetDiscoveryMutex());
        auto it = GetDiscoveryCallbackMap().find(std::string(serviceType));
        if (it != GetDiscoveryCallbackMap().end()) {
            cb = it->second;
        } else {
            cb = new (std::nothrow) MDnsDiscoveryCallbackAni();
            if (cb == nullptr) {
                ret = NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
                return ret;
            }
            GetDiscoveryCallbackMap()[std::string(serviceType)] = cb;
            isNewCallback = true;
        }
    }

    ret = client->StartDiscoverService(std::string(serviceType), cb);
    if (ret != NetManagerStandard::NETMANAGER_EXT_SUCCESS && isNewCallback) {
        std::lock_guard<std::mutex> lock(GetDiscoveryMutex());
        GetDiscoveryCallbackMap().erase(std::string(serviceType));
        // Release the local sptr reference so the object is properly freed
        cb = nullptr;
    }
    return ret;
}

int32_t StopDiscoverService(const rust::String &serviceType, int32_t &ret)
{
    auto client = DelayedSingleton<NetManagerStandard::MDnsClient>::GetInstance();

    sptr<NetManagerStandard::IDiscoveryCallback> cb;
    {
        std::lock_guard<std::mutex> lock(GetDiscoveryMutex());
        auto it = GetDiscoveryCallbackMap().find(std::string(serviceType));
        if (it != GetDiscoveryCallbackMap().end()) {
            cb = it->second;
        }
    }

    if (cb == nullptr) {
        ret = NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
        return ret;
    }

    ret = client->StopDiscoverService(cb);
    std::lock_guard<std::mutex> lock(GetDiscoveryMutex());
    GetDiscoveryCallbackMap().erase(std::string(serviceType));
    return ret;
}

static sptr<MDnsResolveCallbackAni> GetOrCreateResolveCallback(const std::string &key,
    bool &isNewCallback, int32_t &ret)
{
    std::lock_guard<std::mutex> lock(GetResolveMutex());
    auto it = GetResolveCallbackMap().find(key);
    if (it != GetResolveCallbackMap().end()) {
        return it->second;
    }
    auto cb = sptr<MDnsResolveCallbackAni>(new (std::nothrow) MDnsResolveCallbackAni());
    if (cb == nullptr) {
        ret = NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
        return nullptr;
    }
    GetResolveCallbackMap()[key] = cb;
    isNewCallback = true;
    return cb;
}

MDnsServiceInfoFFI ResolveService(const MDnsServiceInfoFFI &serviceInfo, int32_t &ret)
{
    auto client = DelayedSingleton<NetManagerStandard::MDnsClient>::GetInstance();
    NetManagerStandard::MDnsServiceInfo info;
    info.name = std::string(serviceInfo.name);
    info.type = std::string(serviceInfo.type_);
    info.family = serviceInfo.family;
    info.addr = std::string(serviceInfo.addr);
    info.port = serviceInfo.port;

    // Validate inputs do not contain separator characters
    if (info.name.find('\x01') != std::string::npos || info.name.find('\x02') != std::string::npos ||
        info.type.find('\x01') != std::string::npos || info.type.find('\x02') != std::string::npos) {
        ret = NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
        return serviceInfo;
    }

    std::string key = info.name + "\x01" + info.type;
    bool isNewCallback = false;
    auto cb = GetOrCreateResolveCallback(key, isNewCallback, ret);
    if (cb == nullptr) {
        return serviceInfo;
    }

    cb->ResetResolveState();
    ret = client->ResolveService(info, cb);
    if (ret != NetManagerStandard::NETMANAGER_EXT_SUCCESS) {
        if (isNewCallback) {
            std::lock_guard<std::mutex> lock(GetResolveMutex());
            GetResolveCallbackMap().erase(key);
            cb = nullptr;
        }
        return serviceInfo;
    }

    // Wait for async result from HandleResolveResult
    {
        std::unique_lock<std::mutex> lk(cb->mutex_);
        if (!cb->cv_.wait_for(lk, std::chrono::seconds(NetManagerStandard::SYNC_TIMEOUT),
                              [&cb]() { return cb->resolved_; })) {
            if (isNewCallback) {
                std::lock_guard<std::mutex> lock(GetResolveMutex());
                GetResolveCallbackMap().erase(key);
                cb = nullptr;
            }
            ret = NetManagerStandard::NET_MDNS_ERR_TIMEOUT;
            return serviceInfo;
        }
    }

    ret = cb->resolvedRetCode_;
    if (ret != NetManagerStandard::NETMANAGER_EXT_SUCCESS) {
        return serviceInfo;
    }

    MDnsServiceInfoFFI result = BuildServiceInfoFFI(cb->resolvedServiceInfo_);
    return result;
}

// ---- Registration Callback ----

int32_t MDnsRegistrationCallbackAni::HandleRegister(
    const NetManagerStandard::MDnsServiceInfo &serviceInfo, int32_t retCode)
{
    execute_register_result(BuildServiceInfoFFI(serviceInfo), retCode);
    return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
}

int32_t MDnsRegistrationCallbackAni::HandleUnRegister(
    const NetManagerStandard::MDnsServiceInfo &serviceInfo, int32_t retCode)
{
    return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
}

int32_t MDnsRegistrationCallbackAni::HandleRegisterResult(
    const NetManagerStandard::MDnsServiceInfo &serviceInfo, int32_t retCode)
{
    execute_register_result(BuildServiceInfoFFI(serviceInfo), retCode);
    return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
}

// ---- Discovery Callback ----

int32_t MDnsDiscoveryCallbackAni::HandleStartDiscover(
    const NetManagerStandard::MDnsServiceInfo &serviceInfo, int32_t retCode)
{
    execute_discovery_start(BuildServiceInfoFFI(serviceInfo), retCode);
    return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
}

int32_t MDnsDiscoveryCallbackAni::HandleStopDiscover(
    const NetManagerStandard::MDnsServiceInfo &serviceInfo, int32_t retCode)
{
    execute_discovery_stop(BuildServiceInfoFFI(serviceInfo), retCode);
    return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
}

int32_t MDnsDiscoveryCallbackAni::HandleServiceFound(
    const NetManagerStandard::MDnsServiceInfo &serviceInfo, int32_t retCode)
{
    execute_service_found(BuildServiceInfoFFI(serviceInfo), retCode);
    return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
}

int32_t MDnsDiscoveryCallbackAni::HandleServiceLost(
    const NetManagerStandard::MDnsServiceInfo &serviceInfo, int32_t retCode)
{
    execute_service_lost(BuildServiceInfoFFI(serviceInfo), retCode);
    return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
}

// ---- Resolve Callback ----

void MDnsResolveCallbackAni::ResetResolveState()
{
    std::lock_guard<std::mutex> lock(mutex_);
    resolved_ = false;
}

int32_t MDnsResolveCallbackAni::HandleResolveResult(
    const NetManagerStandard::MDnsServiceInfo &serviceInfo, int32_t retCode)
{
    // Notify the synchronous waiter in ResolveService
    {
        std::lock_guard<std::mutex> lock(mutex_);
        resolvedRetCode_ = retCode;
        resolvedServiceInfo_ = serviceInfo;
        resolved_ = true;
    }
    cv_.notify_one();

    // Also fire the Rust callback for any registered listeners
    execute_resolve_result(BuildServiceInfoFFI(serviceInfo), retCode);

    // Clean up callback map
    std::string key = serviceInfo.name + "\x01" + serviceInfo.type;
    {
        std::lock_guard<std::mutex> lock(GetResolveMutex());
        GetResolveCallbackMap().erase(key);
    }

    return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
}

// ---- Observer Register/Unregister ----

int32_t MDnsObserverRegister()
{
    std::lock_guard<std::mutex> lock(g_observerMutex);
    if (g_isMDnsObserverRegistered) {
        return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
    }

    // Release existing callback before creating a new one to prevent memory leaks
    if (g_mdnsDiscCallback != nullptr) {
        g_mdnsDiscCallback = nullptr;
    }
    g_mdnsDiscCallback =
        sptr<MDnsDiscoveryCallbackAni>(new (std::nothrow) MDnsDiscoveryCallbackAni());
    if (g_mdnsDiscCallback == nullptr) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }

    g_isMDnsObserverRegistered = true;
    return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
}

int32_t MDnsObserverUnRegister()
{
    std::lock_guard<std::mutex> lock(g_observerMutex);
    g_isMDnsObserverRegistered = false;
    g_mdnsDiscCallback = nullptr;
    return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
}

} // namespace NetManagerAni
} // namespace OHOS
