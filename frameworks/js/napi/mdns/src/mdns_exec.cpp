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

#include "napi_utils.h"
#include "net_manager_constants.h"
#include "netmanager_ext_log.h"

#include "constant.h"
#include "mdns_client.h"
#include "mdns_exec.h"
#include "mdns_instances.h"

namespace OHOS {
namespace NetManagerStandard {
template <class T> bool WaitTimeout(const int &ret, const SyncVariable &sv, T *context)
{
    bool bRet = false;
    if (sv.retCode_ == NETMANAGER_EXT_SUCCESS) {
        context->SetServiceInfo(sv.serviceInfo_);
        bRet = true;
    }
    context->SetErrorCode(ret);
    return bRet;
}

bool MDnsExec::ExecAddLocalService(MDnsAddLocalServiceContext *context)
{
    auto ret = DelayedSingleton<MDnsClient>::GetInstance()->RegisterService(context->GetServiceInfo(),
                                                                            context->GetObserver());
    context->SetErrorCode(ret);
    return ret == NETMANAGER_EXT_SUCCESS;
}

bool MDnsExec::ExecRemoveLocalService(MDnsRemoveLocalServiceContext *context)
{
    sptr<IRegistrationCallback> callback = context->GetObserver();
    auto ret = DelayedSingleton<MDnsClient>::GetInstance()->UnRegisterService(callback);
    context->SetErrorCode(ret);
    return ret == NETMANAGER_EXT_SUCCESS;
}

bool MDnsExec::ExecResolveLocalService(MDnsResolveLocalServiceContext *context)
{
    auto ret = DelayedSingleton<MDnsClient>::GetInstance()->ResolveService(context->GetServiceInfo(),
                                                                           context->GetObserver());
    if (!MDnsResolveObserver::resloverSync_.bResState_) {
        std::unique_lock<std::mutex> lk(MDnsResolveObserver::resloverSync_.mutex_);
        if (MDnsResolveObserver::resloverSync_.cv_.wait_for(lk, std::chrono::seconds(SYNC_TIMEOUT)) ==
            std::cv_status::timeout) {
            return false;
        }
    }

    if (MDnsResolveObserver::resloverSync_.retCode_ == NETMANAGER_EXT_SUCCESS) {
        context->SetServiceInfo(MDnsResolveObserver::resloverSync_.serviceInfo_);
    }
    ret = MDnsResolveObserver::resloverSync_.retCode_;
    MDnsResolveObserver::resloverSync_.Clear();
    context->SetErrorCode(ret);
    return ret == NETMANAGER_EXT_SUCCESS;
}

bool MDnsExec::ExecStartSearchingMDNS(MDnsStartSearchingContext *context)
{
    EventManager *manager = context->GetManager();
    if (manager == nullptr) {
        NETMGR_EXT_LOG_E("manager is nullptr");
        return false;
    }
    auto discover = static_cast<MDnsDiscoveryInstance *>(manager->GetData());
    if (discover == nullptr) {
        NETMGR_EXT_LOG_E("discover is nullptr");
        return false;
    }
    auto ret = DelayedSingleton<MDnsClient>::GetInstance()->StartDiscoverService(discover->serviceType_,
                                                                                 discover->observer_);
    context->SetErrorCode(ret);
    return ret == NETMANAGER_EXT_SUCCESS;
}

bool MDnsExec::ExecStopSearchingMDNS(MDnsStopSearchingContext *context)
{
    EventManager *manager = context->GetManager();
    if (manager == nullptr) {
        NETMGR_EXT_LOG_E("manager is nullptr");
        return false;
    }
    auto discover = static_cast<MDnsDiscoveryInstance *>(manager->GetData());
    if (discover == nullptr) {
        NETMGR_EXT_LOG_E("discover is nullptr");
        return false;
    }
    auto ret = DelayedSingleton<MDnsClient>::GetInstance()->StopDiscoverService(discover->observer_);
    context->SetErrorCode(ret);
    return ret == NETMANAGER_EXT_SUCCESS;
}

napi_value CreateAttributeObj(napi_env env, MDnsServiceInfo serviceInfo)
{
    napi_value attrObj = NapiUtils::CreateObject(env);
    TxtRecord attrMap = serviceInfo.GetAttrMap();
    auto attrArrSize = attrMap.size();
    size_t index = 0;
    auto iter = attrMap.begin();
    for (; iter != attrMap.end(); iter++, index++) {
        napi_value attrArr = NapiUtils::CreateArray(env, attrArrSize);
        NapiUtils::SetStringPropertyUtf8(env, attrArr, SERVICEINFO_ATTR_KEY, iter->first);

        auto valArrSize = iter->second.size();
        napi_value valArr = NapiUtils::CreateArray(env, valArrSize);
        auto setIter = iter->second.begin();
        size_t setIndex = 0;
        for (; setIter != iter->second.end(); setIter++, setIndex++) {
            NapiUtils::SetArrayElement(env, valArr, setIndex, NapiUtils::CreateUint32(env, *setIter));
        }
        NapiUtils::SetNamedProperty(env, attrArr, SERVICEINFO_ATTR_VALUE, valArr);
        NapiUtils::SetArrayElement(env, attrObj, index, attrArr);
    }
    return attrObj;
}

template <class T> napi_value CreateCallbackParam(T *context)
{
    napi_env env = context->GetEnv();
    MDnsServiceInfo serviceInfo = context->GetServiceInfo();
    NETMANAGER_EXT_LOGI("CreateCallbackParam [%{public}s][%{public}s][%{public}s][%{public}d][%{public}d]",
                        serviceInfo.name.c_str(), serviceInfo.addr.c_str(), serviceInfo.type.c_str(),
                        serviceInfo.port, serviceInfo.family);

    napi_value object = NapiUtils::CreateObject(env);
    NapiUtils::SetStringPropertyUtf8(env, object, SERVICEINFO_TYPE, serviceInfo.type);
    NapiUtils::SetStringPropertyUtf8(env, object, SERVICEINFO_NAME, serviceInfo.name);
    NapiUtils::SetInt32Property(env, object, SERVICEINFO_PORT, serviceInfo.port);

    napi_value eleObj = NapiUtils::CreateObject(env);
    NapiUtils::SetStringPropertyUtf8(env, eleObj, SERVICEINFO_ADDRESS, serviceInfo.addr);
    NapiUtils::SetInt32Property(env, eleObj, SERVICEINFO_PORT, serviceInfo.port);
    int32_t family = serviceInfo.family == MDnsServiceInfo::IPV6 ? 0 : 1;
    NapiUtils::SetInt32Property(env, eleObj, SERVICEINFO_FAMILY, family);

    napi_value attrArrObj = CreateAttributeObj(env, serviceInfo);

    NapiUtils::SetNamedProperty(env, object, SERVICEINFO_HOST, eleObj);
    NapiUtils::SetNamedProperty(env, object, SERVICEINFO_ATTR, attrArrObj);
    return object;
}

napi_value MDnsExec::AddLocalServiceCallback(MDnsAddLocalServiceContext *context)
{
    return CreateCallbackParam<MDnsAddLocalServiceContext>(context);
}

napi_value MDnsExec::RemoveLocalServiceCallback(MDnsRemoveLocalServiceContext *context)
{
    return CreateCallbackParam<MDnsRemoveLocalServiceContext>(context);
}

napi_value MDnsExec::ResolveLocalServiceCallback(MDnsResolveLocalServiceContext *context)
{
    return CreateCallbackParam<MDnsResolveLocalServiceContext>(context);
}

napi_value MDnsExec::StartSearchingMDNSCallback(MDnsStartSearchingContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

napi_value MDnsExec::StopSearchingMDNSCallback(MDnsStopSearchingContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}
} // namespace NetManagerStandard
} // namespace OHOS
