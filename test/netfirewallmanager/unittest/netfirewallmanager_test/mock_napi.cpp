/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "napi/native_api.h"
#include <cstdint>
#include <cstring>
#include <string>

static thread_local bool g_mockHasInterface = true;
static thread_local bool g_mockHasStringProperties = true;
static thread_local napi_valuetype g_mockValueType = napi_undefined;
static thread_local std::string g_mockStringResult;

void SetMockHasInterface(bool hasInterface)
{
    g_mockHasInterface = hasInterface;
}

void SetMockHasStringProperties(bool hasProperties)
{
    g_mockHasStringProperties = hasProperties;
}

void SetMockValueType(napi_valuetype type)
{
    g_mockValueType = type;
}

void SetMockStringResult(const std::string &str)
{
    g_mockStringResult = str;
}

extern "C" {
napi_status napi_has_named_property(napi_env env, napi_value object,
    const char* utf8name, bool* result)
{
    if (utf8name == nullptr) {
        return napi_invalid_arg;
    }
    if (result) {
        if (std::strcmp(utf8name, "interface") == 0) {
            *result = g_mockHasInterface;
        } else {
            *result = g_mockHasStringProperties;
        }
    }
    return napi_ok;
}

napi_status napi_get_value_string_utf8(napi_env env, napi_value value,
    char* buf, size_t bufsize, size_t* result)
{
    size_t len = g_mockStringResult.size();
    if (result) {
        *result = len;
    }
    if (buf && bufsize > 0) {
        size_t copyLen = (len < bufsize - 1) ? len : (bufsize - 1);
        std::memcpy_s(buf, bufsize, g_mockStringResult.c_str(), copyLen);
        buf[copyLen] = '\0';
    }
    return napi_ok;
}

napi_status napi_typeof(napi_env env, napi_value value, napi_valuetype* result)
{
    if (result) {
        if (value == nullptr) {
            *result = napi_undefined;
        } else {
            *result = g_mockValueType;
        }
    }
    return napi_ok;
}

napi_status napi_get_value_bool(napi_env env, napi_value value, bool* result)
{
    if (result) {
        *result = static_cast<bool>(reinterpret_cast<uintptr_t>(value));
    }
    return napi_ok;
}

napi_status napi_is_array(napi_env env, napi_value value, bool* result)
{
    if (result) {
        *result = false;
    }
    return napi_ok;
}

napi_status napi_get_array_length(napi_env env, napi_value value, uint32_t* result)
{
    if (result) {
        *result = 0;
    }
    return napi_ok;
}

napi_status napi_get_element(napi_env env, napi_value object, uint32_t index, napi_value* result)
{
    if (result) {
        *result = nullptr;
    }
    return napi_ok;
}

napi_status napi_create_object(napi_env env, napi_value* result)
{
    if (result) {
        *result = nullptr;
    }
    return napi_ok;
}

napi_status napi_create_array(napi_env env, napi_value* result)
{
    if (result) {
        *result = nullptr;
    }
    return napi_ok;
}

napi_status napi_create_string_utf8(napi_env env, const char* str, size_t length, napi_value* result)
{
    if (result) {
        *result = nullptr;
    }
    return napi_ok;
}

napi_status napi_set_named_property(napi_env env, napi_value object,
    const char* utf8name, napi_value value)
{
    return napi_ok;
}

napi_status napi_create_uint32(napi_env env, uint32_t value, napi_value* result)
{
    if (result) {
        *result = reinterpret_cast<napi_value>(static_cast<uintptr_t>(value));
    }
    return napi_ok;
}

napi_status napi_get_undefined(napi_env env, napi_value* result)
{
    if (result) {
        *result = nullptr;
    }
    return napi_ok;
}

napi_status napi_get_boolean(napi_env env, bool value, napi_value* result)
{
    if (result) {
        *result = reinterpret_cast<napi_value>(static_cast<uintptr_t>(value));
    }
    return napi_ok;
}

napi_status napi_set_element(napi_env env, napi_value object, uint32_t index, napi_value value)
{
    return napi_ok;
}

napi_status napi_create_arraybuffer(napi_env env, size_t byte_length, void** data, napi_value* result)
{
    if (data) {
        *data = nullptr;
    }
    if (result) {
        *result = nullptr;
    }
    return napi_ok;
}

napi_status napi_is_arraybuffer(napi_env env, napi_value value, bool* result)
{
    if (result) {
        *result = false;
    }
    return napi_ok;
}

napi_status napi_get_arraybuffer_info(napi_env env, napi_value arraybuffer, void** data, size_t* byte_length)
{
    if (data) {
        *data = nullptr;
    }
    if (byte_length) {
        *byte_length = 0;
    }
    return napi_ok;
}
}
