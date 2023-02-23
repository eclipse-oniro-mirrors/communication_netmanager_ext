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
#include "netmgr_ext_log_wrapper.h"

#include "constant.h"
#include "mdns_addlocalservice_context.h"

namespace OHOS::NetManagerStandard {
std::map<std::string, sptr<IRegistrationCallback>> MDnsAddLocalServiceContext::registerCallbackMap_;
std::mutex g_mDNSRegisterMutex;

MDnsAddLocalServiceContext::MDnsAddLocalServiceContext(napi_env env, EventManager *manager)
    : MDnsBaseContext(env, manager)
{
}

void MDnsAddLocalServiceContext::ParseParams(napi_value *params, size_t paramsCount)
{
    if (!CheckParamsType(params, paramsCount)) {
        SetNeedThrowException(true);
        SetErrorCode(NET_MDNS_ERR_ILLEGAL_ARGUMENT);
        return;
    }

    std::string bundleName = NapiUtils::GetStringFromValueUtf8(GetEnv(), params[ARG_NUM_0]);
    ParseServiceInfo(GetEnv(), params[ARG_NUM_1]);
    std::string key = bundleName + serviceInfo_.name + serviceInfo_.type;

    std::lock_guard<std::mutex> lock(g_mDNSRegisterMutex);
    auto observer = registerCallbackMap_[key];
    if (observer == nullptr) {
        regObserver_ = new (std::nothrow) MDnsRegistrationObserver();
        if (regObserver_ == nullptr) {
            NETMGR_EXT_LOG_E("new MDnsRegistrationObserver failed");
            SetParseOK(false);
        }
        registerCallbackMap_[key] = regObserver_;
    } else {
        regObserver_ = observer;
    }

    if (paramsCount == PARAM_OPTIONS_AND_CALLBACK) {
        SetParseOK(SetCallback(params[ARG_NUM_2]) == napi_ok);
        return;
    }

    SetParseOK(true);
}

const MDnsServiceInfo &MDnsAddLocalServiceContext::GetServiceInfo() const
{
    return serviceInfo_;
}

void MDnsAddLocalServiceContext::SetServiceInfo(const MDnsServiceInfo &info)
{
    serviceInfo_ = info;
}

sptr<IRegistrationCallback> MDnsAddLocalServiceContext::GetObserver() const
{
    return regObserver_;
}

bool MDnsAddLocalServiceContext::CheckParamsType(napi_value *params, size_t paramsCount)
{
    bool bRet = false;
    if (paramsCount == PARAM_JUST_OPTIONS) {
        bRet = NapiUtils::GetValueType(GetEnv(), params[ARG_NUM_0]) == napi_string &&
               NapiUtils::GetValueType(GetEnv(), params[ARG_NUM_1]) == napi_object;
    } else if (paramsCount == PARAM_OPTIONS_AND_CALLBACK) {
        bRet = NapiUtils::GetValueType(GetEnv(), params[ARG_NUM_0]) == napi_string &&
               NapiUtils::GetValueType(GetEnv(), params[ARG_NUM_1]) == napi_object &&
               NapiUtils::GetValueType(GetEnv(), params[ARG_NUM_2]) == napi_function;
    }
    return bRet;
}
} // namespace OHOS::NetManagerStandard
