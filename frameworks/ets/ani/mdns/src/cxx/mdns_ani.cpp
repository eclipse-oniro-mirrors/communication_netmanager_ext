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

#include <atomic>
#include <chrono>
#include <cstdint>

#include "ani_base_context.h"
#include "context.h"
#include "cxx.h"
#include "net_manager_ext_constants.h"
#include "net_manager_constants.h"
#include "errorcode_convertor.h"
#include "wrapper.rs.h"
#include "mdns_common.h"

namespace OHOS {
namespace NetManagerStandard {

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


static void SetServiceInfoAttrs(const MDnsServiceInfoAni &infoAni, MDnsServiceInfo &info)
{
    if (infoAni.attr_keys.empty()) {
        return;
    }
    TxtRecord attrMap;
    size_t offset = 0;
    for (size_t i = 0; i < infoAni.attr_keys.size(); ++i) {
        if (i >= infoAni.attr_value_lengths.size()) {
            break;
        }
        int32_t len = infoAni.attr_value_lengths[i];
        if (len <= 0 || offset + static_cast<size_t>(len) > infoAni.attr_values.size()) {
            break;
        }
        std::string key = std::string(infoAni.attr_keys[i]);
        std::vector<uint8_t> val(
            infoAni.attr_values.data() + offset,
            infoAni.attr_values.data() + offset + static_cast<size_t>(len)
        );
        attrMap[key] = std::move(val);
        offset += static_cast<size_t>(len);
    }
    info.SetAttrMap(attrMap);
}

static void GetServiceInfoAttrs(const MDnsServiceInfo &info, MDnsServiceInfoAni &infoAni)
{
    TxtRecord attrMap = const_cast<MDnsServiceInfo &>(info).GetAttrMap();
    for (const auto &[key, val] : attrMap) {
        infoAni.attr_keys.push_back(rust::String(key));
        for (uint8_t b : val) {
            infoAni.attr_values.push_back(b);
        }
        infoAni.attr_value_lengths.push_back(static_cast<int32_t>(val.size()));
    }
}

static MDnsServiceInfoAni BuildServiceInfoAni(const MDnsServiceInfo &serviceInfo)
{
    MDnsServiceInfoAni info{};
    info.name = rust::String(serviceInfo.name);
    info.type_ = rust::String(serviceInfo.type);
    info.family = serviceInfo.family;
    info.addr = rust::String(serviceInfo.addr);
    info.port = serviceInfo.port;
    GetServiceInfoAttrs(serviceInfo, info);
    return info;
}

static sptr<MDnsResolveCallbackAni> GetOrCreateResolveCallback(const std::string &key,
    bool &isNewCallback, int32_t &ret)
{
    std::lock_guard<std::mutex> lock(GetResolveMutex());
    auto it = GetResolveCallbackMap().find(key);
    if (it != GetResolveCallbackMap().end()) {
        return it->second;
    }
    auto cb =
        sptr<MDnsResolveCallbackAni>(new (std::nothrow) MDnsResolveCallbackAni());
    if (cb == nullptr) {
        ret = NET_MDNS_ERR_UNKNOWN;
        return nullptr;
    }
    GetResolveCallbackMap()[key] = cb;
    isNewCallback = true;
    return cb;
}

rust::String GetErrorCodeAndMessage(int32_t &errorCode)
{
    NetBaseErrorCodeConvertor convertor;
    rust::String result(convertor.ConvertErrorCode(errorCode));
    return result;
}

MDnsServiceInfoAni RegisterService(const rust::String &bundleName, const MDnsServiceInfoAni &serviceInfo,
    int32_t &ret)
{
    MDnsServiceInfo info;
    info.name = std::string(serviceInfo.name);
    info.type = std::string(serviceInfo.type_);
    info.family = serviceInfo.family;
    info.addr = std::string(serviceInfo.addr);
    info.port = serviceInfo.port;
    SetServiceInfoAttrs(serviceInfo, info);

    std::string bName = std::string(bundleName);
    std::string key = bName + MDNS_HOSTPORT_SPLITER_STR + info.name + MDNS_DOMAIN_SPLITER_STR + info.type;

    std::lock_guard<std::mutex> lock(GetRegisterMutex());
    sptr<MDnsRegistrationCallbackAni> cb;
    auto it = GetRegisterCallbackMap().find(key);
    if (it != GetRegisterCallbackMap().end()) {
        cb = it->second;
    } else {
        cb = new (std::nothrow) MDnsRegistrationCallbackAni();
        if (cb == nullptr) {
            ret = NETMANAGER_EXT_ERR_PARAMETER_ERROR;
            return serviceInfo;
        }
        GetRegisterCallbackMap()[key] = cb;
    }

    ret = DelayedSingleton<MDnsClient>::GetInstance()->RegisterService(info, cb);
    return serviceInfo;
}

MDnsServiceInfoAni UnRegisterService(const rust::String &bundleName, const MDnsServiceInfoAni &serviceInfo,
    int32_t &ret)
{
    MDnsServiceInfo info;
    info.name = std::string(serviceInfo.name);
    info.type = std::string(serviceInfo.type_);
    info.family = serviceInfo.family;
    info.addr = std::string(serviceInfo.addr);
    info.port = serviceInfo.port;

    std::string bName = std::string(bundleName);
    std::string key = bName + MDNS_HOSTPORT_SPLITER_STR + info.name + MDNS_DOMAIN_SPLITER_STR + info.type;

    std::lock_guard<std::mutex> lock(GetRegisterMutex());
    auto it = GetRegisterCallbackMap().find(key);
    if (it == GetRegisterCallbackMap().end()) {
        ret = NET_MDNS_ERR_ILLEGAL_ARGUMENT;
        return serviceInfo;
    }
    sptr<MDnsRegistrationCallbackAni> cb = it->second;
    GetRegisterCallbackMap().erase(it);

    ret = DelayedSingleton<MDnsClient>::GetInstance()->UnRegisterService(cb);
    return serviceInfo;
}

MDnsServiceInfoAni ResolveService(const MDnsServiceInfoAni &serviceInfo, int32_t &ret)
{
    MDnsServiceInfo info;
    info.name = std::string(serviceInfo.name);
    info.type = std::string(serviceInfo.type_);
    info.family = serviceInfo.family;
    info.addr = std::string(serviceInfo.addr);
    info.port = serviceInfo.port;

    std::string key = info.name + MDNS_DOMAIN_SPLITER_STR + info.type;
    bool isNewCallback = false;
    auto cb = GetOrCreateResolveCallback(key, isNewCallback, ret);
    if (cb == nullptr) {
        return serviceInfo;
    }

    cb->ResetResolveState();
    ret = DelayedSingleton<MDnsClient>::GetInstance()->ResolveService(info, cb);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        if (isNewCallback) {
            std::lock_guard<std::mutex> lock(GetResolveMutex());
            GetResolveCallbackMap().erase(key);
            cb = nullptr;
        }
        return serviceInfo;
    }

    {
        std::unique_lock<std::mutex> lk(cb->mutex_);
        if (!cb->cv_.wait_for(lk, std::chrono::seconds(SYNC_TIMEOUT),
                              [&cb]() { return cb->resolved_; })) {
            if (isNewCallback) {
                std::lock_guard<std::mutex> lock(GetResolveMutex());
                GetResolveCallbackMap().erase(key);
                cb = nullptr;
            }
            ret = NET_MDNS_ERR_TIMEOUT;
            return serviceInfo;
        }
    }

    ret = cb->resolvedRetCode_;
    if (ret != NETMANAGER_EXT_SUCCESS) {
        return serviceInfo;
    }

    MDnsServiceInfoAni result = BuildServiceInfoAni(cb->resolvedServiceInfo_);
    return result;
}

struct DiscoveryInstanceInfo {
    std::string serviceType;
    sptr<MDnsDiscoveryCallbackAni> callback;
};

static std::mutex &GetDiscoveryInstanceMutex()
{
    static std::mutex mtx;
    return mtx;
}

static std::map<int64_t, DiscoveryInstanceInfo> &GetDiscoveryInstanceMap()
{
    static std::map<int64_t, DiscoveryInstanceInfo> map;
    return map;
}

static std::atomic<int64_t> g_nextDiscoveryPtr{1};

int32_t StartSearchingMDNSAni(int64_t nativePtr)
{
    std::lock_guard<std::mutex> lock(GetDiscoveryInstanceMutex());
    auto it = GetDiscoveryInstanceMap().find(nativePtr);
    if (it == GetDiscoveryInstanceMap().end()) {
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    std::string serviceType = it->second.serviceType;
    sptr<MDnsDiscoveryCallbackAni> cb = it->second.callback;
    if (cb == nullptr) {
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    int32_t ret = DelayedSingleton<MDnsClient>::GetInstance()->StartDiscoverService(serviceType, cb);
    MDnsServiceInfo info;
    info.type = serviceType;
    cb->HandleStartDiscover(info, ret);
    return ret;
}

int32_t StopSearchingMDNSAni(int64_t nativePtr)
{
    std::lock_guard<std::mutex> lock(GetDiscoveryInstanceMutex());
    auto it = GetDiscoveryInstanceMap().find(nativePtr);
    if (it == GetDiscoveryInstanceMap().end()) {
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    std::string serviceType = it->second.serviceType;
    sptr<MDnsDiscoveryCallbackAni> cb = it->second.callback;
    if (cb == nullptr) {
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    int32_t ret = DelayedSingleton<MDnsClient>::GetInstance()->StopDiscoverService(cb);
    MDnsServiceInfo info;
    info.type = serviceType;
    cb->HandleStopDiscover(info, ret);
    return ret;
}

int32_t MDnsRegistrationCallbackAni::HandleRegister(
    const MDnsServiceInfo &serviceInfo, int32_t retCode)
{
    return NETMANAGER_EXT_SUCCESS;
}

int32_t MDnsRegistrationCallbackAni::HandleUnRegister(
    const MDnsServiceInfo &serviceInfo, int32_t retCode)
{
    return NETMANAGER_EXT_SUCCESS;
}

int32_t MDnsRegistrationCallbackAni::HandleRegisterResult(
    const MDnsServiceInfo &serviceInfo, int32_t retCode)
{
    return NETMANAGER_EXT_SUCCESS;
}

int32_t MDnsDiscoveryCallbackAni::HandleStartDiscover(
    const MDnsServiceInfo &serviceInfo, int32_t retCode)
{
    MDnsServiceInfoAni info = BuildServiceInfoAni(serviceInfo);
    execute_discovery_start(info, retCode, nativePtr_);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t MDnsDiscoveryCallbackAni::HandleStopDiscover(
    const MDnsServiceInfo &serviceInfo, int32_t retCode)
{
    MDnsServiceInfoAni info = BuildServiceInfoAni(serviceInfo);
    execute_discovery_stop(info, retCode, nativePtr_);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t MDnsDiscoveryCallbackAni::HandleServiceFound(
    const MDnsServiceInfo &serviceInfo, int32_t retCode)
{
    MDnsServiceInfoAni info = BuildServiceInfoAni(serviceInfo);
    execute_service_found(info, retCode, nativePtr_);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t MDnsDiscoveryCallbackAni::HandleServiceLost(
    const MDnsServiceInfo &serviceInfo, int32_t retCode)
{
    MDnsServiceInfoAni info = BuildServiceInfoAni(serviceInfo);
    execute_service_lost(info, retCode, nativePtr_);
    return NETMANAGER_EXT_SUCCESS;
}

void MDnsResolveCallbackAni::ResetResolveState()
{
    std::lock_guard<std::mutex> lock(mutex_);
    resolved_ = false;
}

int32_t MDnsResolveCallbackAni::HandleResolveResult(
    const MDnsServiceInfo &serviceInfo, int32_t retCode)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        resolvedRetCode_ = retCode;
        resolvedServiceInfo_ = serviceInfo;
        resolved_ = true;
    }
    cv_.notify_one();

    return NETMANAGER_EXT_SUCCESS;
}

bool IsStageContext(AniEnv *env, AniObject *obj)
{
    if (env == nullptr || obj == nullptr) {
        return false;
    }
    ani_boolean stageMode;
    ani_status status = AbilityRuntime::IsStageContext(reinterpret_cast<ani_env *>(env),
        *reinterpret_cast<ani_object *>(obj), stageMode);
    if (status != ANI_OK) {
        return false;
    }
    return stageMode == 1;
}

std::shared_ptr<AbilityRuntime::Context> GetStageModeContext(AniEnv **env, AniObject *obj)
{
    if (env == nullptr || *env == nullptr || obj == nullptr) {
        return nullptr;
    }
    return AbilityRuntime::GetStageModeContext(reinterpret_cast<ani_env *>(*env),
                                               *reinterpret_cast<ani_object *>(obj));
}

rust::String GetBundleName(std::shared_ptr<AbilityRuntime::Context> context)
{
    if (context == nullptr) {
        return rust::String();
    }
    return rust::String(context->GetBundleName());
}

int64_t CreateDiscoveryServiceAni(const rust::String &serviceType)
{
    std::lock_guard<std::mutex> lock(GetDiscoveryInstanceMutex());
    int64_t ptr = g_nextDiscoveryPtr.fetch_add(1);
    auto cb = sptr<MDnsDiscoveryCallbackAni>(new (std::nothrow) MDnsDiscoveryCallbackAni());
    if (cb == nullptr) {
        return 0;
    }
    cb->SetNativePtr(ptr);
    GetDiscoveryInstanceMap()[ptr] = {std::string(serviceType), cb};
    return ptr;
}

rust::String GetServiceTypeFromPtr(int64_t nativePtr)
{
    std::lock_guard<std::mutex> lock(GetDiscoveryInstanceMutex());
    auto it = GetDiscoveryInstanceMap().find(nativePtr);
    if (it == GetDiscoveryInstanceMap().end()) {
        return rust::String();
    }
    return rust::String(it->second.serviceType);
}

void DestroyDiscoveryServiceAni(int64_t nativePtr)
{
    if (nativePtr == 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(GetDiscoveryInstanceMutex());
    GetDiscoveryInstanceMap().erase(nativePtr);
}

} // namespace NetManagerStandard
} // namespace OHOS
