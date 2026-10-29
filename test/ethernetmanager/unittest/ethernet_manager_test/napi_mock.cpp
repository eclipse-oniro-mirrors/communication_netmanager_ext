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

#include "napi_mock.h"

#include <cstring>

#include "napi_utils.h"

/* ==================== napi_* 强定义（覆盖链接期符号） ==================== */
extern "C" {

static napi_value__ *MockNewValue(napi_valuetype type)
{
    auto *v = new (std::nothrow) napi_value__();
    if (v != nullptr) {
        v->type = type;
    }
    return v;
}

napi_status napi_get_undefined(napi_env env, napi_value *result)
{
    (void)env;
    if (result == nullptr) {
        return napi_invalid_arg;
    }
    *result = MockNewValue(napi_undefined);
    return napi_ok;
}

napi_status napi_create_object(napi_env env, napi_value *result)
{
    (void)env;
    if (result == nullptr) {
        return napi_invalid_arg;
    }
    *result = MockNewValue(napi_object);
    return napi_ok;
}

napi_status napi_create_string_utf8(napi_env env, const char *str, size_t length, napi_value *result)
{
    (void)env;
    if (result == nullptr || str == nullptr) {
        return napi_invalid_arg;
    }
    auto *v = MockNewValue(napi_string);
    if (v == nullptr) {
        return napi_generic_failure;
    }
    v->strVal = (length == NAPI_AUTO_LENGTH) ? std::string(str) : std::string(str, length);
    *result = v;
    return napi_ok;
}

napi_status napi_create_int32(napi_env env, int32_t value, napi_value *result)
{
    (void)env;
    if (result == nullptr) {
        return napi_invalid_arg;
    }
    auto *v = MockNewValue(napi_number);
    if (v == nullptr) {
        return napi_generic_failure;
    }
    v->intVal = value;
    *result = v;
    return napi_ok;
}

napi_status napi_typeof(napi_env env, napi_value value, napi_valuetype *result)
{
    (void)env;
    if (result == nullptr) {
        return napi_invalid_arg;
    }
    *result = (value == nullptr) ? napi_undefined : value->type;
    return napi_ok;
}

napi_status napi_get_value_int32(napi_env env, napi_value value, int32_t *result)
{
    (void)env;
    if (result == nullptr) {
        return napi_invalid_arg;
    }
    if (value == nullptr || value->type != napi_number) {
        return napi_number_expected;
    }
    *result = value->intVal;
    return napi_ok;
}

napi_status napi_get_cb_info(napi_env env, napi_callback_info cbinfo, size_t *argc, napi_value *argv,
                             napi_value *thisArg, void **data)
{
    (void)env;
    (void)data;
    if (cbinfo == nullptr || argc == nullptr) {
        return napi_invalid_arg;
    }
    size_t want = (*argc == 0) ? 0 : *argc;
    size_t avail = cbinfo->argv.size();
    size_t copy = (want < avail) ? want : avail;
    for (size_t i = 0; i < copy; ++i) {
        argv[i] = cbinfo->argv[i];
    }
    *argc = avail;
    if (thisArg != nullptr) {
        *thisArg = cbinfo->thisVar;
    }
    return napi_ok;
}

napi_status napi_create_error(napi_env env, napi_value code, napi_value msg, napi_value *result)
{
    (void)env;
    if (result == nullptr) {
        return napi_invalid_arg;
    }
    auto *v = MockNewValue(napi_object);
    if (v == nullptr) {
        return napi_generic_failure;
    }
    if (code != nullptr) {
        v->props["code"] = code;
    }
    if (msg != nullptr) {
        v->props["message"] = msg;
    }
    *result = v;
    return napi_ok;
}

napi_status napi_throw(napi_env env, napi_value error)
{
    if (env == nullptr) {
        return napi_invalid_arg;
    }
    env->pendingException = error;
    return napi_ok;
}

napi_status napi_create_promise(napi_env env, napi_deferred *deferred, napi_value *promise)
{
    if (env == nullptr || deferred == nullptr || promise == nullptr) {
        return napi_invalid_arg;
    }
    auto *p = MockNewValue(napi_object);
    auto *d = new (std::nothrow) napi_deferred__();
    if (p == nullptr || d == nullptr) {
        return napi_generic_failure;
    }
    d->env = env;
    d->promise = p;
    *deferred = d;
    *promise = p;
    return napi_ok;
}

napi_status napi_resolve_deferred(napi_env env, napi_deferred deferred, napi_value resolution)
{
    if (env == nullptr || deferred == nullptr) {
        return napi_invalid_arg;
    }
    deferred->settled = true;
    deferred->rejected = false;
    env->resolvedRecords.push_back({deferred, resolution});
    return napi_ok;
}

napi_status napi_reject_deferred(napi_env env, napi_deferred deferred, napi_value rejection)
{
    if (env == nullptr || deferred == nullptr) {
        return napi_invalid_arg;
    }
    deferred->settled = true;
    deferred->rejected = true;
    env->rejectedRecords.push_back({deferred, rejection});
    return napi_ok;
}

napi_status napi_create_reference(napi_env env, napi_value value, uint32_t refcount, napi_ref *result)
{
    (void)env;
    if (result == nullptr) {
        return napi_invalid_arg;
    }
    auto *r = new (std::nothrow) napi_ref__();
    if (r == nullptr) {
        return napi_generic_failure;
    }
    r->value = value;
    r->refcount = refcount;
    *result = r;
    return napi_ok;
}

napi_status napi_get_reference_value(napi_env env, napi_ref ref, napi_value *result)
{
    (void)env;
    if (result == nullptr || ref == nullptr) {
        return napi_invalid_arg;
    }
    *result = ref->value;
    return napi_ok;
}

napi_status napi_delete_reference(napi_env env, napi_ref ref)
{
    (void)env;
    if (ref == nullptr) {
        return napi_invalid_arg;
    }
    delete ref;
    return napi_ok;
}

napi_status napi_strict_equals(napi_env env, napi_value lhs, napi_value rhs, bool *result)
{
    (void)env;
    if (result == nullptr) {
        return napi_invalid_arg;
    }
    *result = (lhs == rhs);
    return napi_ok;
}

napi_status napi_call_function(napi_env env, napi_value recv, napi_value func, size_t argc, const napi_value *argv,
                               napi_value *result)
{
    (void)recv;
    if (env == nullptr) {
        return napi_invalid_arg;
    }
    if (func == nullptr || func->type != napi_function) {
        return napi_function_expected;
    }
    napi_env__::CallRecord record;
    record.func = func;
    for (size_t i = 0; i < argc; ++i) {
        record.argv.push_back(argv[i]);
    }
    env->callRecords.push_back(record);
    if (result != nullptr) {
        *result = MockNewValue(napi_undefined);
    }
    return napi_ok;
}

napi_status napi_is_typedarray(napi_env env, napi_value value, bool *result)
{
    (void)env;
    if (result == nullptr) {
        return napi_invalid_arg;
    }
    *result = (value != nullptr) && value->isTypedArray;
    return napi_ok;
}

napi_status napi_get_typedarray_info(napi_env env, napi_value value, napi_typedarray_type *type, size_t *length,
                                     void **data, napi_value *arraybuffer, size_t *byte_offset)
{
    (void)env;
    if (value == nullptr || !value->isTypedArray) {
        return napi_invalid_arg;
    }
    if (type != nullptr) {
        *type = value->taType;
    }
    if (length != nullptr) {
        *length = value->taLength;
    }
    if (byte_offset != nullptr) {
        *byte_offset = value->taOffset;
    }
    if (arraybuffer != nullptr) {
        *arraybuffer = value->taBuffer;
    }
    if (data != nullptr && value->taBuffer != nullptr) {
        *data = value->taBuffer->binVal.data() + value->taOffset;
    }
    return napi_ok;
}

napi_status napi_get_arraybuffer_info(napi_env env, napi_value arraybuffer, void **data, size_t *byte_length)
{
    (void)env;
    if (arraybuffer == nullptr) {
        return napi_invalid_arg;
    }
    if (data != nullptr) {
        *data = arraybuffer->binVal.data();
    }
    if (byte_length != nullptr) {
        *byte_length = arraybuffer->binVal.size();
    }
    return napi_ok;
}

napi_status napi_create_arraybuffer(napi_env env, size_t byte_length, void **data, napi_value *result)
{
    (void)env;
    if (result == nullptr) {
        return napi_invalid_arg;
    }
    auto *v = MockNewValue(napi_object);
    if (v == nullptr) {
        return napi_generic_failure;
    }
    v->binVal.assign(byte_length, 0);
    if (data != nullptr) {
        *data = v->binVal.data();
    }
    *result = v;
    return napi_ok;
}

napi_status napi_create_typedarray(napi_env env, napi_typedarray_type type, size_t length, napi_value arraybuffer,
                                   size_t byte_offset, napi_value *result)
{
    (void)env;
    if (result == nullptr || arraybuffer == nullptr) {
        return napi_invalid_arg;
    }
    auto *v = MockNewValue(napi_object);
    if (v == nullptr) {
        return napi_generic_failure;
    }
    v->isTypedArray = true;
    v->taType = type;
    v->taLength = length;
    v->taOffset = byte_offset;
    v->taBuffer = arraybuffer;
    *result = v;
    return napi_ok;
}

napi_status napi_create_threadsafe_function(napi_env env, napi_value func, napi_value async_resource,
                                            napi_value async_resource_name, size_t max_queue_size,
                                            size_t initial_thread_count, void *thread_finalize_data,
                                            napi_finalize thread_finalize_cb, void *context,
                                            napi_threadsafe_function_call_js call_js_cb,
                                            napi_threadsafe_function *result)
{
    (void)async_resource;
    (void)async_resource_name;
    (void)max_queue_size;
    (void)thread_finalize_data;
    (void)thread_finalize_cb;
    if (env == nullptr || result == nullptr) {
        return napi_invalid_arg;
    }
    if (initial_thread_count == 0) {
        return napi_invalid_arg;
    }
    auto *tsfn = new (std::nothrow) napi_threadsafe_function__();
    if (tsfn == nullptr) {
        return napi_generic_failure;
    }
    tsfn->env = env;
    tsfn->jsFunc = func;
    tsfn->callJsCb = call_js_cb;
    tsfn->context = context;
    tsfn->refcount = static_cast<uint32_t>(initial_thread_count);
    tsfn->released = false;
    env->lastTsfn = tsfn;
    *result = tsfn;
    return napi_ok;
}

napi_status napi_call_threadsafe_function(napi_threadsafe_function func, void *data,
                                          napi_threadsafe_function_call_mode mode)
{
    (void)mode;
    if (func == nullptr) {
        return napi_invalid_arg;
    }
    if (func->released) {
        return napi_closing;
    }
    // 模拟投递到 JS 线程：同步执行 call_js_cb，保证测试确定性
    if (func->callJsCb != nullptr) {
        func->callJsCb(func->env, func->jsFunc, func->context, data);
    }
    return napi_ok;
}

napi_status napi_acquire_threadsafe_function(napi_threadsafe_function func)
{
    if (func == nullptr) {
        return napi_invalid_arg;
    }
    ++func->refcount;
    return napi_ok;
}

napi_status napi_release_threadsafe_function(napi_threadsafe_function func, napi_threadsafe_function_release_mode mode)
{
    if (func == nullptr) {
        return napi_invalid_arg;
    }
    if (mode == napi_tsfn_abort) {
        func->released = true;
        return napi_ok;
    }
    if (func->refcount > 0) {
        --func->refcount;
    }
    if (func->refcount == 0) {
        func->released = true;
    }
    return napi_ok;
}

} // extern "C"

/* ==================== NapiUtils 模拟（镜像真实实现语义） ==================== */
namespace OHOS {
namespace NetManagerStandard {
namespace NapiUtils {

napi_valuetype GetValueType(napi_env env, napi_value value)
{
    (void)env;
    return (value == nullptr) ? napi_undefined : value->type;
}

bool HasNamedProperty(napi_env env, napi_value object, const std::string &propertyName)
{
    (void)env;
    return object != nullptr && object->props.find(propertyName) != object->props.end();
}

napi_value GetNamedProperty(napi_env env, napi_value object, const std::string &propertyName)
{
    (void)env;
    if (object == nullptr) {
        return nullptr;
    }
    auto it = object->props.find(propertyName);
    return (it == object->props.end()) ? nullptr : it->second;
}

void SetNamedProperty(napi_env env, napi_value object, const std::string &name, napi_value value)
{
    (void)env;
    if (object != nullptr) {
        object->props[name] = value;
    }
}

napi_value CreateInt32(napi_env env, int32_t code)
{
    (void)env;
    auto *v = MockNewValue(napi_number);
    if (v != nullptr) {
        v->intVal = code;
    }
    return v;
}

int32_t GetInt32FromValue(napi_env env, napi_value value)
{
    (void)env;
    if (value == nullptr || value->type != napi_number) {
        return -1; // 镜像真实实现：NAPI_CALL_BASE 失败返回 -1
    }
    return value->intVal;
}

int32_t GetInt32Property(napi_env env, napi_value object, const std::string &propertyName)
{
    if (!HasNamedProperty(env, object, propertyName)) {
        return 0; // 镜像真实实现：缺属性返回 0
    }
    return GetInt32FromValue(env, GetNamedProperty(env, object, propertyName));
}

void SetInt32Property(napi_env env, napi_value object, const std::string &name, int32_t value)
{
    (void)env;
    if (object != nullptr) {
        auto *v = MockNewValue(napi_number);
        v->intVal = value;
        object->props[name] = v;
    }
}

napi_value CreateStringUtf8(napi_env env, const std::string &str)
{
    (void)env;
    auto *v = MockNewValue(napi_string);
    if (v != nullptr) {
        v->strVal = str;
    }
    return v;
}

std::string GetStringFromValueUtf8(napi_env env, napi_value value)
{
    (void)env;
    return (value == nullptr || value->type != napi_string) ? "" : value->strVal;
}

SecureData GetSecureDataFromValueUtf8(napi_env env, napi_value value)
{
    (void)env;
    if (value == nullptr || value->type != napi_string) {
        return SecureData();
    }
    SecureData data;
    static_cast<std::string &>(data) = value->strVal;
    return data;
}

std::string GetStringPropertyUtf8(napi_env env, napi_value object, const std::string &propertyName)
{
    if (!HasNamedProperty(env, object, propertyName)) {
        return "";
    }
    return GetStringFromValueUtf8(env, GetNamedProperty(env, object, propertyName));
}

SecureData GetSecureDataPropertyUtf8(napi_env env, napi_value object, const std::string &propertyName)
{
    return GetSecureDataFromValueUtf8(env, GetNamedProperty(env, object, propertyName));
}

void SetStringPropertyUtf8(napi_env env, napi_value object, const std::string &name, const std::string &value)
{
    (void)env;
    if (object != nullptr) {
        auto *v = MockNewValue(napi_string);
        if (v != nullptr) {
            v->strVal = value;
        }
        object->props[name] = v;
    }
}

napi_value CreateObject(napi_env env)
{
    (void)env;
    return MockNewValue(napi_object);
}

napi_value GetUndefined(napi_env env)
{
    (void)env;
    return MockNewValue(napi_undefined);
}

bool GetBooleanProperty(napi_env env, napi_value object, const std::string &propertyName)
{
    (void)env;
    if (!HasNamedProperty(env, object, propertyName)) {
        return false;
    }
    napi_value value = GetNamedProperty(env, object, propertyName);
    return (value != nullptr && value->type == napi_boolean) ? value->boolVal : false;
}

void SetBooleanProperty(napi_env env, napi_value object, const std::string &name, bool value)
{
    (void)env;
    if (object != nullptr) {
        auto *v = MockNewValue(napi_boolean);
        if (v != nullptr) {
            v->boolVal = value;
        }
        object->props[name] = v;
    }
}

} // namespace NapiUtils

/* ==================== 测试驱动辅助实现 ==================== */
namespace NapiMock {

napi_env SharedEnv()
{
    static napi_env__ sharedEnv;
    return &sharedEnv;
}

void ResetEnvRecords(napi_env env)
{
    if (env == nullptr) {
        return;
    }
    env->pendingException = nullptr;
    env->callRecords.clear();
    env->resolvedRecords.clear();
    env->rejectedRecords.clear();
}

napi_value MakeUndefined(napi_env env)
{
    (void)env;
    return MockNewValue(napi_undefined);
}

napi_value MakeObject(napi_env env)
{
    (void)env;
    return MockNewValue(napi_object);
}

napi_value MakeNumber(napi_env env, int32_t value)
{
    (void)env;
    auto *v = MockNewValue(napi_number);
    if (v != nullptr) {
        v->intVal = value;
    }
    return v;
}

napi_value MakeBoolean(napi_env env, bool value)
{
    (void)env;
    auto *v = MockNewValue(napi_boolean);
    if (v != nullptr) {
        v->boolVal = value;
    }
    return v;
}

napi_value MakeString(napi_env env, const std::string &value)
{
    (void)env;
    auto *v = MockNewValue(napi_string);
    if (v != nullptr) {
        v->strVal = value;
    }
    return v;
}

napi_value MakeFunction(napi_env env)
{
    (void)env;
    return MockNewValue(napi_function);
}

napi_value MakeUint8Array(napi_env env, const std::vector<uint8_t> &data)
{
    (void)env;
    auto *buffer = MockNewValue(napi_object);
    auto *ta = MockNewValue(napi_object);
    if (buffer == nullptr || ta == nullptr) {
        return nullptr;
    }
    buffer->binVal = data;
    ta->isTypedArray = true;
    ta->taType = napi_uint8_array;
    ta->taLength = data.size();
    ta->taOffset = 0;
    ta->taBuffer = buffer;
    return ta;
}

napi_value MakeInt16Array(napi_env env, size_t length)
{
    napi_value buffer = nullptr;
    if (napi_create_arraybuffer(env, length * sizeof(int16_t), nullptr, &buffer) != napi_ok) {
        return nullptr;
    }
    napi_value array = nullptr;
    if (napi_create_typedarray(env, napi_int16_array, length, buffer, 0, &array) != napi_ok) {
        return nullptr;
    }
    return array;
}

void SetProp(napi_env env, napi_value object, const std::string &name, napi_value value)
{
    (void)env;
    if (object != nullptr) {
        object->props[name] = value;
    }
}

napi_value GetProp(napi_env env, napi_value object, const std::string &name)
{
    (void)env;
    if (object == nullptr) {
        return nullptr;
    }
    auto it = object->props.find(name);
    return (it == object->props.end()) ? nullptr : it->second;
}

napi_callback_info MakeCbInfo(napi_env env, std::initializer_list<napi_value> args)
{
    (void)env;
    auto *info = new (std::nothrow) napi_callback_info__();
    if (info == nullptr) {
        return nullptr;
    }
    for (auto arg : args) {
        info->argv.push_back(arg);
    }
    return info;
}

bool HasPendingException(napi_env env)
{
    return env != nullptr && env->pendingException != nullptr;
}

int32_t PendingExceptionCode(napi_env env)
{
    if (env == nullptr || env->pendingException == nullptr) {
        return -1;
    }
    auto it = env->pendingException->props.find("code");
    if (it == env->pendingException->props.end() || it->second == nullptr) {
        return -1;
    }
    return it->second->intVal;
}

size_t ResolvedCount(napi_env env)
{
    return (env == nullptr) ? 0 : env->resolvedRecords.size();
}

size_t RejectedCount(napi_env env)
{
    return (env == nullptr) ? 0 : env->rejectedRecords.size();
}

napi_value__ *LastRejectedValue(napi_env env)
{
    if (env == nullptr || env->rejectedRecords.empty()) {
        return nullptr;
    }
    return env->rejectedRecords.back().value;
}

napi_value__ *LastResolvedValue(napi_env env)
{
    if (env == nullptr || env->resolvedRecords.empty()) {
        return nullptr;
    }
    return env->resolvedRecords.back().value;
}

size_t JsCallCount(napi_env env)
{
    return (env == nullptr) ? 0 : env->callRecords.size();
}

napi_env__::CallRecord *LastJsCall(napi_env env)
{
    if (env == nullptr || env->callRecords.empty()) {
        return nullptr;
    }
    return &env->callRecords.back();
}

} // namespace NapiMock
} // namespace NetManagerStandard
} // namespace OHOS
