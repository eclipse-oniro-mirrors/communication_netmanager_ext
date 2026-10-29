/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "ethernet_client.h"
#include "eth_eap_state_callback_stub.h"
#include "net_manager_constants.h"
#include "netmanager_ext_log.h"
#include "securec.h"

namespace OHOS {
namespace NetManagerStandard {

#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE

/** 参数类型错误的标准错误码（BusinessError 401） */
static constexpr int32_t EAP_ERRCODE_PARAMETER_ERROR = 401;

/** EthEapConnectAction 枚举值（与 d.ts / eap_module 导出保持一致） */
static constexpr int32_t ETH_EAP_ACTION_CONNECT = 0;
static constexpr int32_t ETH_EAP_ACTION_DISCONNECT = 1;

/** 错误码对应的错误消息（与 d.ts @throws 文档一致） */
static const char *GetEapErrMsg(int32_t errCode)
{
    switch (errCode) {
        case EAP_ERRCODE_PARAMETER_ERROR:
            return "Parameter error";
        case EAP_ERRCODE_PERMISSION_DENIED:
            return "Permission denied";
        case EAP_ERRCODE_INVALID_NETID:
            return "Invalid netId";
        case EAP_ERRCODE_LOGOFF_FAIL:
            return "Log off fail";
        case EAP_ERRCODE_INVALID_PROFILE:
            return "Invalid profile";
        case EAP_ERRCODE_NETMANAGER_STOP:
            return "Netmanager stop";
        case EAP_ERRCODE_INVALID_ETH_STATE:
            return "Invalid eth state";
        case EAP_ERRCODE_ENTERPRISE_OCCUPIED:
            return "Enterprise app occupied";
        case EAP_ERRCODE_CONFIG_LOCKED:
            return "Configuration locked";
        case EAP_ERRCODE_CERT_PROCESS_FAILED:
            return "Certificate process failed";
        case NETMANAGER_EXT_ERR_FILE_NOT_FOUND:
            return "Config file not found";
        default:
            return "Internal error";
    }
}

/** 构造 BusinessError 风格的错误对象（含 code/message 属性） */
static napi_value CreateBusinessError(napi_env env, int32_t errCode)
{
    napi_value code = NapiUtils::CreateInt32(env, errCode);
    napi_value message = NapiUtils::CreateStringUtf8(env, GetEapErrMsg(errCode));
    napi_value result = nullptr;
    napi_status status = napi_create_error(env, code, message, &result);
    if (status != napi_ok || result == nullptr) {
        NETMGR_EXT_LOG_E("CreateBusinessError: napi_create_error failed=%{public}d", static_cast<int32_t>(status));
        return NapiUtils::GetUndefined(env);
    }
    return result;
}

/** 抛出 BusinessError 风格异常（on/off 同步接口失败场景） */
static void ThrowBusinessError(napi_env env, int32_t errCode)
{
    napi_value err = CreateBusinessError(env, errCode);
    if (err != nullptr) {
        napi_throw(env, err);
    }
}

/** 创建并立即 resolve 的 Promise（无返回数据场景） */
static napi_value CreateResolvedPromise(napi_env env)
{
    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    if (napi_create_promise(env, &deferred, &promise) != napi_ok) {
        return NapiUtils::GetUndefined(env);
    }
    napi_resolve_deferred(env, deferred, NapiUtils::GetUndefined(env));
    return promise;
}

/** 创建并立即 reject 的 Promise（reject 值为 BusinessError，JS 侧 catch 可拿 e.code） */
static napi_value CreateRejectedPromise(napi_env env, int32_t errCode)
{
    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    if (napi_create_promise(env, &deferred, &promise) != napi_ok) {
        return NapiUtils::GetUndefined(env);
    }
    napi_reject_deferred(env, deferred, CreateBusinessError(env, errCode));
    return promise;
}

/**
 * @brief 从 JS 对象解析 EthEapProfile。
 * 密码/证书口令经平台安全 API 读入 SecureData（G.STD.07：不经 std::string 中转，
 * 避免敏感信息堆上驻留）；certEntry 支持 Uint8Array。
 */
static void ParseEthEapProfileFromJs(napi_env env, napi_value jsObj, EthEapProfile &profile)
{
    int32_t tmpVal = NapiUtils::GetInt32Property(env, jsObj, "eapMethod");
    profile.eapMethod = static_cast<EapMethod>(tmpVal);
    tmpVal = NapiUtils::GetInt32Property(env, jsObj, "phase2Method");
    profile.phase2Method = static_cast<Phase2Method>(tmpVal);
    profile.identity = NapiUtils::GetStringPropertyUtf8(env, jsObj, "identity");
    profile.anonymousIdentity = NapiUtils::GetStringPropertyUtf8(env, jsObj, "anonymousIdentity");
    profile.password = NapiUtils::GetSecureDataPropertyUtf8(env, jsObj, "password");
    profile.caCertAliases = NapiUtils::GetStringPropertyUtf8(env, jsObj, "caCertAliases");
    profile.caPath = NapiUtils::GetStringPropertyUtf8(env, jsObj, "caPath");
    profile.clientCertAliases = NapiUtils::GetStringPropertyUtf8(env, jsObj, "clientCertAliases");
    bool hasCertEntry = NapiUtils::HasNamedProperty(env, jsObj, "certEntry");
    if (hasCertEntry) {
        napi_value certArray = NapiUtils::GetNamedProperty(env, jsObj, "certEntry");
        bool isTypedArray = false;
        if (napi_is_typedarray(env, certArray, &isTypedArray) == napi_ok && isTypedArray) {
            size_t length = 0;
            size_t offset = 0;
            napi_typedarray_type type;
            napi_value buffer = nullptr;
            napi_get_typedarray_info(env, certArray, &type, &length, nullptr, &buffer, &offset);
            if (type == napi_uint8_array && buffer != nullptr) {
                size_t total = 0;
                uint8_t *data = nullptr;
                napi_get_arraybuffer_info(env, buffer, reinterpret_cast<void **>(&data), &total);
                length = std::min<size_t>(length, total - offset);
                profile.certEntry.assign(&data[offset], &data[offset] + length);
            }
        }
    }
    profile.certPassword = NapiUtils::GetSecureDataPropertyUtf8(env, jsObj, "certPassword");
    profile.altSubjectMatch = NapiUtils::GetStringPropertyUtf8(env, jsObj, "altSubjectMatch");
    profile.domainSuffixMatch = NapiUtils::GetStringPropertyUtf8(env, jsObj, "domainSuffixMatch");
    profile.realm = NapiUtils::GetStringPropertyUtf8(env, jsObj, "realm");
    profile.plmn = NapiUtils::GetStringPropertyUtf8(env, jsObj, "plmn");
    profile.eapSubId = NapiUtils::GetInt32Property(env, jsObj, "eapSubId");
}

/** 将 EthEapConfig 转 JS 对象；敏感凭据只写不回读（password/certPassword 空串、certEntry 空数组） */
static napi_value CreateEthEapConfigJs(napi_env env, const EthEapConfig &config)
{
    napi_value obj = NapiUtils::CreateObject(env);
    napi_value profileObj = NapiUtils::CreateObject(env);
    NapiUtils::SetInt32Property(env, profileObj, "eapMethod", static_cast<int32_t>(config.profile.eapMethod));
    NapiUtils::SetInt32Property(env, profileObj, "phase2Method", static_cast<int32_t>(config.profile.phase2Method));
    NapiUtils::SetStringPropertyUtf8(env, profileObj, "identity", config.profile.identity);
    NapiUtils::SetStringPropertyUtf8(env, profileObj, "anonymousIdentity", config.profile.anonymousIdentity);
    NapiUtils::SetStringPropertyUtf8(env, profileObj, "password", "");
    NapiUtils::SetStringPropertyUtf8(env, profileObj, "caCertAliases", config.profile.caCertAliases);
    NapiUtils::SetStringPropertyUtf8(env, profileObj, "caPath", config.profile.caPath);
    NapiUtils::SetStringPropertyUtf8(env, profileObj, "clientCertAliases", config.profile.clientCertAliases);
    napi_value emptyBuffer = nullptr;
    napi_status abStatus = napi_create_arraybuffer(env, 0, nullptr, &emptyBuffer);
    if (abStatus != napi_ok) {
        NETMGR_EXT_LOG_E("CreateEthEapConfigJs: napi_create_arraybuffer failed=%{public}d", abStatus);
        NapiUtils::SetNamedProperty(env, profileObj, "certEntry", NapiUtils::GetUndefined(env));
    } else {
        napi_value emptyArray = nullptr;
        napi_status taStatus = napi_create_typedarray(env, napi_uint8_array, 0, emptyBuffer, 0, &emptyArray);
        if (taStatus != napi_ok) {
            NETMGR_EXT_LOG_E("CreateEthEapConfigJs: napi_create_typedarray failed=%{public}d", taStatus);
            NapiUtils::SetNamedProperty(env, profileObj, "certEntry", NapiUtils::GetUndefined(env));
        } else {
            NapiUtils::SetNamedProperty(env, profileObj, "certEntry", emptyArray);
        }
    }
    NapiUtils::SetStringPropertyUtf8(env, profileObj, "certPassword", "");
    NapiUtils::SetStringPropertyUtf8(env, profileObj, "altSubjectMatch", config.profile.altSubjectMatch);
    NapiUtils::SetStringPropertyUtf8(env, profileObj, "domainSuffixMatch", config.profile.domainSuffixMatch);
    NapiUtils::SetStringPropertyUtf8(env, profileObj, "realm", config.profile.realm);
    NapiUtils::SetStringPropertyUtf8(env, profileObj, "plmn", config.profile.plmn);
    NapiUtils::SetInt32Property(env, profileObj, "eapSubId", config.profile.eapSubId);
    NapiUtils::SetNamedProperty(env, obj, "profile", profileObj);
    NapiUtils::SetBooleanProperty(env, obj, "enabled", config.enabled);
    NapiUtils::SetBooleanProperty(env, obj, "autoAuth", config.autoAuth);
    NapiUtils::SetBooleanProperty(env, obj, "locked", config.locked);
    return obj;
}

/**
 * @brief 将 EthEapStateInfo 转成 JS 对象 {state, retryCount, message}。
 */
static napi_value CreateEthEapStateInfoJs(napi_env env, const EthEapStateInfo &stateInfo)
{
    napi_value infoObj = NapiUtils::CreateObject(env);
    NapiUtils::SetInt32Property(env, infoObj, "state", static_cast<int32_t>(stateInfo.state));
    NapiUtils::SetInt32Property(env, infoObj, "retryCount", stateInfo.retryCount);
    NapiUtils::SetStringPropertyUtf8(env, infoObj, "message", stateInfo.message);
    return infoObj;
}

/** 状态事件跨线程投递通道（进程内单例）：JS 线程创建、进程常驻，避免与在途事件的生命周期竞争 */
static napi_threadsafe_function g_stateChangeTsf = nullptr;
/** JS 侧注册的状态回调引用列表（仅在 JS 线程访问：On/Off/CallJs） */
static std::vector<napi_ref> g_stateChangeCbRefs;

/**
 * @brief 802.1X 状态变化回调的 NAPI 实现（继承服务端 Stub）。
 * OnEthEapStateChanged 运行在 binder 线程，不能直接调 napi 接口（ecma 虚拟机仅允许
 * JS 主线程执行，否则致命崩溃）：仅打包事件经 threadsafe function 投递回 JS 线程执行。
 */
class EthEapStateNapiCallback : public EthEapStateCallbackStub {
public:
    /** 服务端通知入口（binder 线程）：打包事件投递到 JS 线程，不触碰 napi 对象 */
    int32_t OnEthEapStateChanged(const EthEapStateInfo &stateInfo) override
    {
        if (g_stateChangeTsf == nullptr) {
            NETMGR_EXT_LOG_E("OnEthEapStateChanged: threadsafe function not initialized");
            return NETMANAGER_EXT_ERR_INTERNAL;
        }
        auto *data = new (std::nothrow) EthEapStateInfo(stateInfo);
        if (data == nullptr) {
            NETMGR_EXT_LOG_E("OnEthEapStateChanged: alloc state info failed");
            return NETMANAGER_EXT_ERR_INTERNAL;
        }
        // acquire/release 成对使用，允许多个 binder 线程并发投递
        napi_acquire_threadsafe_function(g_stateChangeTsf);
        napi_status status = napi_call_threadsafe_function(g_stateChangeTsf, data, napi_tsfn_nonblocking);
        napi_release_threadsafe_function(g_stateChangeTsf, napi_tsfn_release);
        if (status != napi_ok) {
            delete data;
            NETMGR_EXT_LOG_E("OnEthEapStateChanged: post to JS thread failed=%{public}d", static_cast<int32_t>(status));
            return NETMANAGER_EXT_ERR_INTERNAL;
        }
        return NETMANAGER_EXT_SUCCESS;
    }
};

/** 保护全局状态回调对象的互斥锁 */
static std::mutex g_stateCallbackMutex;
/** 全局唯一的 802.1X 状态回调对象（跨多个 env 时先到先得，通常单 env） */
static sptr<EthEapStateNapiCallback> g_stateCallback = nullptr;

/** threadsafe function 的 JS 线程执行体：组装参数并逐个调用已注册 JS 回调 */
static void StateChangeCallJs(napi_env env, napi_value jsCallback, void *context, void *data)
{
    (void)jsCallback;
    (void)context;
    auto *stateInfo = static_cast<EthEapStateInfo *>(data);
    if (stateInfo == nullptr) {
        return;
    }
    if (env == nullptr || g_stateChangeCbRefs.empty()) {
        delete stateInfo;
        return;
    }
    napi_value infoObj = CreateEthEapStateInfoJs(env, *stateInfo);
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    for (auto &ref : g_stateChangeCbRefs) {
        napi_value cb = nullptr;
        napi_get_reference_value(env, ref, &cb);
        if (cb == nullptr) {
            continue;
        }
        napi_value result = nullptr;
        // 此时已在 JS 线程，可安全调用 JS 函数
        napi_call_function(env, undefined, cb, 1, &infoObj, &result);
    }
    delete stateInfo;
}

/** 创建跨线程投递通道（必须在 JS 线程调用）；成功或已存在返回 true */
static bool CreateStateChangeTsf(napi_env env)
{
    if (g_stateChangeTsf != nullptr) {
        return true;
    }
    napi_value resourceName = nullptr;
    napi_status status = napi_create_string_utf8(env, "ethEapStateChange", NAPI_AUTO_LENGTH, &resourceName);
    if (status != napi_ok) {
        NETMGR_EXT_LOG_E("CreateStateChangeTsf: create resource name failed=%{public}d", static_cast<int32_t>(status));
        return false;
    }
    status = napi_create_threadsafe_function(env, nullptr, nullptr, resourceName, 0, 1, nullptr, nullptr, nullptr,
                                             StateChangeCallJs, &g_stateChangeTsf);
    if (status != napi_ok) {
        g_stateChangeTsf = nullptr;
        NETMGR_EXT_LOG_E("CreateStateChangeTsf: napi_create_threadsafe_function failed=%{public}d",
                        static_cast<int32_t>(status));
        return false;
    }
    return true;
}

/** 获取（或首次创建并注册到 EthernetClient）全局状态回调对象，失败返回 nullptr */
static sptr<EthEapStateNapiCallback> GetOrCreateStateCallback(napi_env env)
{
    std::lock_guard<std::mutex> lock(g_stateCallbackMutex);
    if (g_stateCallback == nullptr) {
        if (!CreateStateChangeTsf(env)) {
            NETMGR_EXT_LOG_E("GetOrCreateStateCallback: CreateStateChangeTsf failed");
            return nullptr;
        }
        g_stateCallback = sptr<EthEapStateNapiCallback>::MakeSptr();
        if (g_stateCallback != nullptr) {
            int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->RegisterEthEapStateCallback(g_stateCallback);
            if (ret != NETMANAGER_EXT_SUCCESS) {
                // 注册失败：清空全局对象，JS 侧不能误认为订阅成功
                NETMGR_EXT_LOG_E("GetOrCreateStateCallback: RegisterEthEapStateCallback failed, ret=%{public}d", ret);
                g_stateCallback = nullptr;
                return nullptr;
            }
            NETMANAGER_EXT_LOGI("GetOrCreateStateCallback: RegisterEthEapStateCallback success");
        }
    }
    return g_stateCallback;
}

/** setEthEapConfig（同步）：解析配置保存到服务端；参数错误抛 401，业务失败抛 BusinessError */
napi_value SetEthEapConfig(napi_env env, napi_callback_info info)
{
    NETMANAGER_EXT_LOGI("SetEthEapConfig called from JS");
    size_t argc = ARG_NUM_1;
    napi_value argv[ARG_NUM_1] = {0};
    napi_value thisVar = 0;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc < ARG_NUM_1 || NapiUtils::GetValueType(env, argv[ARG_INDEX_0]) != napi_object) {
        ThrowBusinessError(env, EAP_ERRCODE_PARAMETER_ERROR);
        return NapiUtils::GetUndefined(env);
    }
    EthEapConfig config;
    napi_value profileVal = NapiUtils::GetNamedProperty(env, argv[ARG_INDEX_0], "profile");
    if (NapiUtils::GetValueType(env, profileVal) != napi_object) {
        ThrowBusinessError(env, EAP_ERRCODE_PARAMETER_ERROR);
        return NapiUtils::GetUndefined(env);
    }
    ParseEthEapProfileFromJs(env, profileVal, config.profile);
    config.enabled = NapiUtils::GetBooleanProperty(env, argv[ARG_INDEX_0], "enabled");
    config.autoAuth = NapiUtils::GetBooleanProperty(env, argv[ARG_INDEX_0], "autoAuth");
    config.locked = NapiUtils::GetBooleanProperty(env, argv[ARG_INDEX_0], "locked");
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->SetEthEapConfig(config);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        ThrowBusinessError(env, ret);
    }
    return NapiUtils::GetUndefined(env);
}

/** getEthEapConfig（Promise）：未配置 resolve 默认对象，敏感字段置空不回读 */
napi_value GetEthEapConfig(napi_env env, napi_callback_info info)
{
    NETMGR_EXT_LOG_D("GetEthEapConfig enter");
    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    if (napi_create_promise(env, &deferred, &promise) != napi_ok) {
        NETMGR_EXT_LOG_E("GetEthEapConfig: napi_create_promise failed");
        return NapiUtils::GetUndefined(env);
    }
    EthEapConfig config;
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->GetEthEapConfig(config);
    NETMGR_EXT_LOG_D("GetEthEapConfig ret=%{public}d", ret);
    if (ret == NETMANAGER_EXT_ERR_FILE_NOT_FOUND) {
        // 未配置：resolve 默认配置对象（d.ts 契约，enabled=false 可区分未配置与已配置但关闭）
        EthEapConfig defaultConfig;
        defaultConfig.enabled = false;
        defaultConfig.autoAuth = false;
        defaultConfig.locked = false;
        napi_resolve_deferred(env, deferred, CreateEthEapConfigJs(env, defaultConfig));
        return promise;
    }
    if (ret != NETMANAGER_EXT_SUCCESS) {
        napi_reject_deferred(env, deferred, CreateBusinessError(env, ret));
        return promise;
    }
    napi_resolve_deferred(env, deferred, CreateEthEapConfigJs(env, config));
    return promise;
}

/** deleteEthEapConfig（Promise）：删除服务端配置，未配置时同样 resolve（幂等） */
napi_value DeleteEthEapConfig(napi_env env, napi_callback_info info)
{
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->DeleteEthEapConfig();
    if (ret != NETMANAGER_EXT_SUCCESS) {
        return CreateRejectedPromise(env, ret);
    }
    return CreateResolvedPromise(env);
}

/** getEthEapStateInfo（同步）：返回 {state, retryCount, message}，失败抛 BusinessError */
napi_value GetEthEapStateInfo(napi_env env, napi_callback_info info)
{
    EthEapStateInfo stateInfo;
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->GetEthEapStateInfo(stateInfo);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        ThrowBusinessError(env, ret);
        return NapiUtils::GetUndefined(env);
    }
    return CreateEthEapStateInfoJs(env, stateInfo);
}

/** onStateChange（同步）：首次调用创建并注册全局回调，随后加入回调列表 */
napi_value OnStateChange(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_NUM_1;
    napi_value argv[ARG_NUM_1] = {0};
    napi_value thisVar = 0;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc < ARG_NUM_1 || NapiUtils::GetValueType(env, argv[ARG_INDEX_0]) != napi_function) {
        NETMGR_EXT_LOG_E("OnStateChange: callback is not a function");
        ThrowBusinessError(env, EAP_ERRCODE_PARAMETER_ERROR);
        return NapiUtils::GetUndefined(env);
    }
    auto stateCb = GetOrCreateStateCallback(env);
    if (stateCb == nullptr) {
        NETMGR_EXT_LOG_E("OnStateChange: failed to get state callback");
        ThrowBusinessError(env, EAP_ERRCODE_INTERNAL_ERROR);
        return NapiUtils::GetUndefined(env);
    }
    // 持久引用防止 JS 回调函数被 GC 回收
    napi_ref ref = nullptr;
    napi_create_reference(env, argv[ARG_INDEX_0], 1, &ref);
    if (ref != nullptr) {
        g_stateChangeCbRefs.push_back(ref);
    } else {
        NETMGR_EXT_LOG_E("OnStateChange: napi_create_reference failed");
    }
    return NapiUtils::GetUndefined(env);
}

/** offStateChange（同步）：带 callback 移除该回调；不带移除全部并注销底层回调 */
napi_value OffStateChange(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_NUM_1;
    napi_value argv[ARG_NUM_1] = {0};
    napi_value thisVar = 0;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc >= ARG_NUM_1 && NapiUtils::GetValueType(env, argv[ARG_INDEX_0]) != napi_function &&
        NapiUtils::GetValueType(env, argv[ARG_INDEX_0]) != napi_undefined) {
        ThrowBusinessError(env, EAP_ERRCODE_PARAMETER_ERROR);
        return NapiUtils::GetUndefined(env);
    }
    std::lock_guard<std::mutex> lock(g_stateCallbackMutex);
    if (g_stateCallback == nullptr) {
        return NapiUtils::GetUndefined(env);
    }
    if (argc >= ARG_NUM_1 && NapiUtils::GetValueType(env, argv[ARG_INDEX_0]) == napi_function) {
        // 指定回调：同一 JS 函数对象才视为相等，匹配则删除引用并移出列表
        napi_value target = argv[ARG_INDEX_0];
        for (auto it = g_stateChangeCbRefs.begin(); it != g_stateChangeCbRefs.end(); ++it) {
            napi_value cb = nullptr;
            napi_get_reference_value(env, *it, &cb);
            bool equals = false;
            if (cb != nullptr && napi_strict_equals(env, cb, target, &equals) == napi_ok && equals) {
                napi_delete_reference(env, *it);
                g_stateChangeCbRefs.erase(it);
                break;
            }
        }
    } else {
        // 未指定回调：先停服务端通知再清空 JS 引用列表；在途事件在 JS 线程发现列表为空后仅丢弃
        int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->UnregisterEthEapStateCallback(g_stateCallback);
        if (ret != NETMANAGER_EXT_SUCCESS) {
            NETMGR_EXT_LOG_E("OffStateChange: UnregisterEthEapStateCallback failed: %{public}d", ret);
            ThrowBusinessError(env, ret);
            return NapiUtils::GetUndefined(env);
        }
        for (auto &ref : g_stateChangeCbRefs) {
            napi_delete_reference(env, ref);
        }
        g_stateChangeCbRefs.clear();
        g_stateCallback = nullptr;
    }
    return NapiUtils::GetUndefined(env);
}

/** connect（Promise）：action 0=按已保存配置重连认证；1=停止自动认证并登出 */
napi_value Connect(napi_env env, napi_callback_info info)
{
    NETMGR_EXT_LOG_I("Connect called from JS");
    size_t argc = ARG_NUM_1;
    napi_value argv[ARG_NUM_1] = {0};
    napi_value thisVar = 0;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    int32_t action = -1;
    if (argc < ARG_NUM_1 || NapiUtils::GetValueType(env, argv[ARG_INDEX_0]) != napi_number ||
        napi_get_value_int32(env, argv[ARG_INDEX_0], &action) != napi_ok ||
        (action != ETH_EAP_ACTION_CONNECT && action != ETH_EAP_ACTION_DISCONNECT)) {
        return CreateRejectedPromise(env, EAP_ERRCODE_PARAMETER_ERROR);
    }
    bool connect = (action == ETH_EAP_ACTION_CONNECT);
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->ConnectEthEap(connect);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        return CreateRejectedPromise(env, ret);
    }
    return CreateResolvedPromise(env);
}

#endif // NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE

} // namespace NetManagerStandard
} // namespace OHOS
