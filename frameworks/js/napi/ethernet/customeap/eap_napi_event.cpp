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

static bool CheckStartEthEapParams(napi_env env, napi_value *params, size_t paramsCount)
{
    if (paramsCount == PARAM_DOUBLE_OPTIONS) {
        return NapiUtils::GetValueType(env, params[ARG_INDEX_0]) == napi_number &&
            NapiUtils::GetValueType(env, params[ARG_INDEX_1]) == napi_object;
    }
    return false;
}
 
static bool GetU8VectorFromJsOptionItem(const napi_env env, const napi_value config,
    const std::string &key, std::vector<uint8_t> &value)
{
    bool hasProperty = NapiUtils::HasNamedProperty(env, config, key);
    if (!hasProperty) {
        NETMGR_EXT_LOG_E("JsObjectToU8Vector no property: %{public}s", key.c_str());
        return false;
    }
    napi_value array = NapiUtils::GetNamedProperty(env, config, key);
    bool isTypedArray = false;
    if (napi_is_typedarray(env, array, &isTypedArray) != napi_ok || !isTypedArray) {
        NETMGR_EXT_LOG_E("JsObjectToU8Vector not typedarray: %{public}s", key.c_str());
        return false;
    }
    size_t length = 0;
    size_t offset = 0;
    napi_typedarray_type type;
    napi_value buffer = nullptr;
    NAPI_CALL_BASE(env, napi_get_typedarray_info(env, array, &type, &length, nullptr, &buffer, &offset), {});
    if (type != napi_uint8_array || buffer == nullptr) {
        NETMGR_EXT_LOG_E("JsObjectToU8Vector buffer null: %{public}s", key.c_str());
        return false;
    }
    size_t total = 0;
    uint8_t *data = nullptr;
    NAPI_CALL_BASE(env, napi_get_arraybuffer_info(env, buffer, reinterpret_cast<void **>(&data), &total), {});
    length = std::min<size_t>(length, total - offset);
    value.resize(length);
    memcpy_s(value.data(), value.size(), &data[offset], length);
    return true;
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
 
    uint32_t eapCode = NapiUtils::GetUint32FromValue(env, argv[ARG_INDEX_1]);
    if (eapCode < EAP_CODE_MIN || eapCode > EAP_CODE_MAX) {
        NETMANAGER_EXT_LOGE("valid eapCode %{public}d", eapCode);
        return NapiUtils::GetUndefined(env);
    }
 
    uint32_t eapType = NapiUtils::GetUint32FromValue(env, argv[ARG_INDEX_2]);
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

napi_value StartEthEap(napi_env env, napi_callback_info info)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    size_t argc = 2;
    napi_value argv[2] = {0};
    napi_value thisVar = 0;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (!CheckStartEthEapParams(env, argv, argc)) {
        return NapiUtils::GetUndefined(env);
    }
    int32_t netId = NapiUtils::GetInt32FromValue(env, argv[ARG_INDEX_0]);
    EthEapProfile profile;
    profile.eapMethod = static_cast<EapMethod>(NapiUtils::GetInt32Property(env, argv[ARG_INDEX_1],
        "eapMethod"));
    profile.phase2Method = static_cast<Phase2Method>(NapiUtils::GetInt32Property(env, argv[ARG_INDEX_1],
        "phase2Method"));
    profile.identity = NapiUtils::GetStringPropertyUtf8(env, argv[ARG_INDEX_1], "identity");
    profile.anonymousIdentity = NapiUtils::GetStringPropertyUtf8(env, argv[ARG_INDEX_1], "anonymousIdentity");
    profile.password = NapiUtils::GetStringPropertyUtf8(env, argv[ARG_INDEX_1], "password");
    profile.caCertAliases = NapiUtils::GetStringPropertyUtf8(env, argv[ARG_INDEX_1], "caCertAliases");
    profile.caPath = NapiUtils::GetStringPropertyUtf8(env, argv[ARG_INDEX_1], "caPath");
    profile.clientCertAliases = NapiUtils::GetStringPropertyUtf8(env, argv[ARG_INDEX_1], "clientCertAliases");
    GetU8VectorFromJsOptionItem(env, argv[ARG_INDEX_1], "certEntry", profile.certEntry);
    profile.certPassword = NapiUtils::GetStringPropertyUtf8(env, argv[ARG_INDEX_1], "certPassword");
    profile.altSubjectMatch = NapiUtils::GetStringPropertyUtf8(env, argv[ARG_INDEX_1], "altSubjectMatch");
    profile.domainSuffixMatch = NapiUtils::GetStringPropertyUtf8(env, argv[ARG_INDEX_1], "domainSuffixMatch");
    profile.realm = NapiUtils::GetStringPropertyUtf8(env, argv[ARG_INDEX_1], "realm");
    profile.plmn = NapiUtils::GetStringPropertyUtf8(env, argv[ARG_INDEX_1], "plmn");
    profile.eapSubId = NapiUtils::GetInt32Property(env, argv[ARG_INDEX_1], "eapSubId");
    DelayedSingleton<EthernetClient>::GetInstance()->StartEthEap(netId, profile);
#endif
    return NapiUtils::GetUndefined(env);
}
 
napi_value LogOffEthEap(napi_env env, napi_callback_info info)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    size_t argc = 2;
    napi_value argv[2] = {0};
    napi_value thisVar = 0;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if ((argc != PARAM_JUST_OPTIONS) || (NapiUtils::GetValueType(env, argv[ARG_INDEX_0]) != napi_number)) {
        return NapiUtils::GetUndefined(env);
    }
    int32_t netId = NapiUtils::GetInt32FromValue(env, argv[ARG_INDEX_0]);
    DelayedSingleton<EthernetClient>::GetInstance()->LogOffEthEap(netId);
#endif
    return NapiUtils::GetUndefined(env);
}
 
} // namespace NetManagerStandard
} // namespace OHOS