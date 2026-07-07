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

#ifndef NET_MDNS_ANI_H
#define NET_MDNS_ANI_H

#include <condition_variable>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>

#include "cxx.h"
#include "singleton.h"
#include "mdns_client.h"
#include "discovery_callback_stub.h"
#include "registration_callback_stub.h"
#include "resolve_callback_stub.h"
#include "mdns_service_info.h"

namespace OHOS {
namespace NetManagerAni {

struct MDnsServiceInfoFFI {
    rust::String name;
    rust::String type_;
    int32_t family;
    rust::String addr;
    int32_t port;
    rust::Vec<rust::String> attr_keys;
    rust::Vec<uint8_t> attr_values;
    rust::Vec<int32_t> attr_value_lengths;
};

class MDnsRegistrationCallbackAni : public NetManagerStandard::RegistrationCallbackStub {
public:
    int32_t HandleRegister(const NetManagerStandard::MDnsServiceInfo &serviceInfo, int32_t retCode) override;
    int32_t HandleUnRegister(const NetManagerStandard::MDnsServiceInfo &serviceInfo, int32_t retCode) override;
    int32_t HandleRegisterResult(const NetManagerStandard::MDnsServiceInfo &serviceInfo, int32_t retCode) override;
};

class MDnsDiscoveryCallbackAni : public NetManagerStandard::DiscoveryCallbackStub {
public:
    int32_t HandleStartDiscover(const NetManagerStandard::MDnsServiceInfo &serviceInfo, int32_t retCode) override;
    int32_t HandleStopDiscover(const NetManagerStandard::MDnsServiceInfo &serviceInfo, int32_t retCode) override;
    int32_t HandleServiceFound(const NetManagerStandard::MDnsServiceInfo &serviceInfo, int32_t retCode) override;
    int32_t HandleServiceLost(const NetManagerStandard::MDnsServiceInfo &serviceInfo, int32_t retCode) override;
};

class MDnsResolveCallbackAni : public NetManagerStandard::ResolveCallbackStub {
public:
    int32_t HandleResolveResult(const NetManagerStandard::MDnsServiceInfo &serviceInfo, int32_t retCode) override;
    void ResetResolveState();

    std::mutex mutex_;
    std::condition_variable cv_;
    bool resolved_ = false;
    NetManagerStandard::MDnsServiceInfo resolvedServiceInfo_;
    int32_t resolvedRetCode_ = -1;
};

// Callback map for RegisterService/UnRegisterService pairing
// Key format: "bundleName\x01serviceName\x02serviceType" (using safe separators to avoid key conflicts)
std::map<std::string, sptr<MDnsRegistrationCallbackAni>>& GetRegisterCallbackMap();
std::mutex& GetRegisterMutex();

// Callback map for StartDiscoverService/StopDiscoverService pairing
// Key: serviceType
std::map<std::string, sptr<MDnsDiscoveryCallbackAni>>& GetDiscoveryCallbackMap();
std::mutex& GetDiscoveryMutex();

// Callback map for ResolveService, cleaned up in HandleResolveResult
// Key: "serviceName\x01serviceType" (using safe separator to avoid key conflicts)
std::map<std::string, sptr<MDnsResolveCallbackAni>>& GetResolveCallbackMap();
std::mutex& GetResolveMutex();

MDnsServiceInfoFFI RegisterService(const rust::String &bundleName, const MDnsServiceInfoFFI &serviceInfo,
    int32_t &ret);
int32_t UnRegisterService(const rust::String &bundleName, const MDnsServiceInfoFFI &serviceInfo, int32_t &ret);
int32_t StartDiscoverService(const rust::String &serviceType, int32_t &ret);
int32_t StopDiscoverService(const rust::String &serviceType, int32_t &ret);
MDnsServiceInfoFFI ResolveService(const MDnsServiceInfoFFI &serviceInfo, int32_t &ret);

int32_t MDnsObserverRegister();
int32_t MDnsObserverUnRegister();

rust::String GetErrorCodeAndMessage(int32_t &errorCode);

} // namespace NetManagerAni
} // namespace OHOS
#endif // NET_MDNS_ANI_H