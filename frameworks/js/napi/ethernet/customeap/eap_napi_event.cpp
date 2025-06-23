/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
 
#include "eap_napi_event.h"
#include "napi_constant.h"
 
namespace OHOS {
namespace NetManagerStandard {
 
namespace {
#ifdef NET_EXTENSIBLE_AUTHENTICATION
static bool CheckParamsType(napi_env env, napi_value *params, size_t paramsCount)
{
    if (static_cast<int>(paramsCount) == PARAM_TRIPLE_OPTIONS_AND_CALLBACK) {
        return NapiUtils::GetValueType(env, params[ARG_INDEX_0]) == napi_number &&
               NapiUtils::GetValueType(env, params[ARG_INDEX_1]) == napi_number &&
               NapiUtils::GetValueType(env, params[ARG_INDEX_2]) == napi_number &&
               NapiUtils::GetValueType(env, params[ARG_INDEX_3]) == napi_function;
    }
    if (static_cast<int>(paramsCount) == PARAM_DOUBLE_OPTIONS) {
        return NapiUtils::GetValueType(env, params[ARG_INDEX_0]) == napi_number &&
               NapiUtils::GetValueType(env, params[ARG_INDEX_1]) == napi_object;
    }
    return false;
}
#endif
} // namespace
 
napi_value RegCustomEapHandler(napi_env env, napi_callback_info info)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    size_t requireArgc = 4;
    size_t argc = 4;
    napi_value argv[4] = {0};
    napi_value thisVar = 0;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (requireArgc > argc) {
        NETMANAGER_EXT_LOGE("requireArgc:%{public}zu, argc:%{public}zu", requireArgc, argc);
        return NapiUtils::GetUndefined(env);
    }
 
    if (!CheckParamsType(env, argv, argc)) {
        NETMANAGER_EXT_LOGE("params type error");
        return NapiUtils::GetUndefined(env);
    }
 
    int32_t netType = NapiUtils::GetInt32FromValue(env, argv[ARG_INDEX_0]);
    if (netType < static_cast<int>(NetType::WLAN0) || netType >= static_cast<int>(NetType::INVALID)) {
        NETMANAGER_EXT_LOGE("valid netType %{public}d", static_cast<int>(netType));
        return NapiUtils::GetUndefined(env);
    }
 
    int32_t eapCode = NapiUtils::GetInt32FromValue(env, argv[ARG_INDEX_1]);
    if (eapCode < EAP_CODE_MIN || eapCode > EAP_CODE_MAX) {
        NETMANAGER_EXT_LOGE("valid eapCode %{public}d", eapCode);
        return NapiUtils::GetUndefined(env);
    }
 
    int32_t eapType = NapiUtils::GetInt32FromValue(env, argv[ARG_INDEX_2]);
    if (eapType < EAP_TYPE_MIN || eapType > EAP_TYPE_MAX) {
        NETMANAGER_EXT_LOGE("valid eapType %{public}d", eapType);
        return NapiUtils::GetUndefined(env);
    }
 
    EapEventMgr::GetInstance().RegCustomEapHandler(env, static_cast<NetType>(netType),
        eapCode, eapType, argv[ARG_INDEX_3]);
#endif
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}
 
napi_value UnRegCustomEapHandler(napi_env env, napi_callback_info info)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    size_t requireArgc = 4;
    size_t argc = 4;
    napi_value argv[4] = {0};
    napi_value thisVar = 0;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (requireArgc > argc) {
        NETMANAGER_EXT_LOGE("requireArgc:%{public}zu, argc:%{public}zu", requireArgc, argc);
        return NapiUtils::GetUndefined(env);
    }
 
    if (!CheckParamsType(env, argv, argc)) {
        NETMANAGER_EXT_LOGE("params type error");
        return NapiUtils::GetUndefined(env);
    }
 
    int32_t netType = NapiUtils::GetInt32FromValue(env, argv[ARG_INDEX_0]);
    if (netType < static_cast<int>(NetType::WLAN0) || netType >= static_cast<int>(NetType::INVALID)) {
        NETMANAGER_EXT_LOGE("valid netType %{public}d", static_cast<int>(netType));
        return NapiUtils::GetUndefined(env);
    }
 
    int32_t eapCode = NapiUtils::GetInt32FromValue(env, argv[ARG_INDEX_1]);
    if (eapCode < EAP_CODE_MIN || eapCode > EAP_CODE_MAX) {
        NETMANAGER_EXT_LOGE("valid eapCode %{public}d", eapCode);
        return NapiUtils::GetUndefined(env);
    }
 
    int32_t eapType = NapiUtils::GetInt32FromValue(env, argv[ARG_INDEX_2]);
    if (eapType < EAP_TYPE_MIN || eapType > EAP_TYPE_MAX) {
        NETMANAGER_EXT_LOGE("valid eapType %{public}d", eapType);
        return NapiUtils::GetUndefined(env);
    }
 
    EapEventMgr::GetInstance().UnRegCustomEapHandler(env, static_cast<NetType>(netType),
        eapCode, eapType, argv[ARG_INDEX_3]);
#endif
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}
 
napi_value ReplyCustomEapData(napi_env env, napi_callback_info info)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    size_t requireArgc = 2;
    size_t argc = 2;
    napi_value argv[2] = {0};
    napi_value thisVar = 0;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (requireArgc > argc) {
        NETMANAGER_EXT_LOGE("requireArgc:%{public}zu, argc:%{public}zu", requireArgc, argc);
        return NapiUtils::GetUndefined(env);
    }
 
    if (!CheckParamsType(env, argv, argc)) {
        NETMANAGER_EXT_LOGE("params type error");
        return NapiUtils::GetUndefined(env);
    }
 
    CustomResult customResult = static_cast<CustomResult>(NapiUtils::GetInt32FromValue(env, argv[ARG_INDEX_0]));
    sptr<EapData> eapData = new (std::nothrow) EapData();
    if (eapData == nullptr) {
        NETMANAGER_EXT_LOGE("%{public}s, eapData is nullptr", __func__);
        return NapiUtils::GetUndefined(env);
    }
    eapData->msgId = NapiUtils::GetInt32Property(env, argv[ARG_INDEX_1], "msgId");
    eapData->bufferLen = NapiUtils::GetInt32Property(env, argv[ARG_INDEX_1], "bufferLen");
    NapiUtils::GetVectorUint8Property(env, argv[ARG_INDEX_1], "eapBuffer", eapData->eapBuffer);
 
    NETMANAGER_EXT_LOGI("%{public}s, msgId:%{public}d, bufferLen:%{public}d, result:%{public}d, buffsize:%{public}zu",
        __func__, eapData->msgId, eapData->bufferLen, static_cast<int>(customResult), eapData->eapBuffer.size());
    EapEventMgr::GetInstance().ReplyCustomEapData(customResult, eapData);
#endif
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}
 
} // namespace NetManagerStandard
} // namespace OHOS