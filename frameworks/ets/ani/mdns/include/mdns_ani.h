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
namespace AbilityRuntime {
class Context;
}

namespace NetManagerStandard {

struct AniEnv;
struct AniObject;
struct MDnsServiceInfoAni;

class MDnsRegistrationCallbackAni : public RegistrationCallbackStub {
public:
    int32_t HandleRegister(const MDnsServiceInfo &serviceInfo, int32_t retCode) override;
    int32_t HandleUnRegister(const MDnsServiceInfo &serviceInfo, int32_t retCode) override;
    int32_t HandleRegisterResult(const MDnsServiceInfo &serviceInfo, int32_t retCode) override;
};

class MDnsDiscoveryCallbackAni : public DiscoveryCallbackStub {
public:
    void SetNativePtr(int64_t nativePtr) { nativePtr_ = nativePtr; }
    int64_t GetNativePtr() const { return nativePtr_; }

    int32_t HandleStartDiscover(const MDnsServiceInfo &serviceInfo, int32_t retCode) override;
    int32_t HandleStopDiscover(const MDnsServiceInfo &serviceInfo, int32_t retCode) override;
    int32_t HandleServiceFound(const MDnsServiceInfo &serviceInfo, int32_t retCode) override;
    int32_t HandleServiceLost(const MDnsServiceInfo &serviceInfo, int32_t retCode) override;

private:
    int64_t nativePtr_ = 0;
};

class MDnsResolveCallbackAni : public ResolveCallbackStub {
public:
    int32_t HandleResolveResult(const MDnsServiceInfo &serviceInfo, int32_t retCode) override;
    void ResetResolveState();

    std::mutex mutex_;
    std::condition_variable cv_;
    bool resolved_ = false;
    MDnsServiceInfo resolvedServiceInfo_;
    int32_t resolvedRetCode_ = -1;
};

std::map<std::string, sptr<MDnsRegistrationCallbackAni>>& GetRegisterCallbackMap();
std::mutex& GetRegisterMutex();

std::map<std::string, sptr<MDnsResolveCallbackAni>>& GetResolveCallbackMap();
std::mutex& GetResolveMutex();

MDnsServiceInfoAni RegisterService(const rust::String &bundleName, const MDnsServiceInfoAni &serviceInfo,
    int32_t &ret);
MDnsServiceInfoAni UnRegisterService(const rust::String &bundleName, const MDnsServiceInfoAni &serviceInfo,
    int32_t &ret);
int32_t StartSearchingMDNSAni(int64_t nativePtr);
int32_t StopSearchingMDNSAni(int64_t nativePtr);
MDnsServiceInfoAni ResolveService(const MDnsServiceInfoAni &serviceInfo, int32_t &ret);

rust::String GetErrorCodeAndMessage(int32_t &errorCode);

bool IsStageContext(AniEnv *env, AniObject *obj);
std::shared_ptr<AbilityRuntime::Context> GetStageModeContext(AniEnv **env, AniObject *obj);
rust::String GetBundleName(std::shared_ptr<AbilityRuntime::Context> context);

int64_t CreateDiscoveryServiceAni(const rust::String &serviceType);
rust::String GetServiceTypeFromPtr(int64_t nativePtr);
void DestroyDiscoveryServiceAni(int64_t nativePtr);

} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_MDNS_ANI_H