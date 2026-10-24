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

#ifndef NETMANAGER_EXT_TEST_NAPI_MOCK_H
#define NETMANAGER_EXT_TEST_NAPI_MOCK_H

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <napi/native_api.h>

/**
 * 本文件为 eap_napi_config 单元测试提供最小可用的 napi 环境模拟：
 * - 完成 third_party/node 头文件中的不透明结构（仅本测试目标内有效）；
 * - napi_mock.cpp 以强定义覆盖链接期所有被测代码用到的 napi_* 符号；
 * - NapiUtils（netmanager_base 可执行目标内同一定义胜出）按真实实现语义模拟。
 * 说明：真实 JS 虚拟机无法在 native 单测中运行，此模拟使 eap_napi_config.cpp
 * 的解析/序列化/异常/Promise/线程安全函数投递逻辑可被确定性驱动。
 */

/* ---------------- 不透明结构补全 ---------------- */
struct napi_env__ {
    /** 当前挂起的异常（napi_throw 写入） */
    napi_value__ *pendingException = nullptr;
    /** napi_call_function 调用记录（func + argv） */
    struct CallRecord {
        napi_value__ *func = nullptr;
        std::vector<napi_value__ *> argv;
    };
    std::vector<CallRecord> callRecords;
    /** Promise 兑现记录 */
    struct DeferredRecord {
        napi_deferred__ *deferred = nullptr;
        napi_value__ *value = nullptr;
    };
    std::vector<DeferredRecord> resolvedRecords;
    std::vector<DeferredRecord> rejectedRecords;
    /** 最近一次 napi_create_threadsafe_function 产物 */
    napi_threadsafe_function__ *lastTsfn = nullptr;
};

struct napi_value__ {
    napi_valuetype type = napi_undefined;
    int32_t intVal = 0;
    bool boolVal = false;
    std::string strVal;
    /** arraybuffer / typedarray 底层数据 */
    std::vector<uint8_t> binVal;
    bool isTypedArray = false;
    napi_typedarray_type taType = napi_uint8_array;
    size_t taLength = 0;
    size_t taOffset = 0;
    napi_value__ *taBuffer = nullptr;
    /** object 属性表 */
    std::map<std::string, napi_value__ *> props;
};

struct napi_callback_info__ {
    std::vector<napi_value__ *> argv;
    napi_value__ *thisVar = nullptr;
};

struct napi_ref__ {
    napi_value__ *value = nullptr;
    uint32_t refcount = 1;
};

struct napi_deferred__ {
    napi_env__ *env = nullptr;
    napi_value__ *promise = nullptr;
    bool settled = false;
    bool rejected = false;
};

struct napi_threadsafe_function__ {
    napi_env__ *env = nullptr;
    napi_value__ *jsFunc = nullptr;
    napi_threadsafe_function_call_js callJsCb = nullptr;
    void *context = nullptr;
    uint32_t refcount = 1;
    bool released = false;
};

/* ---------------- 测试驱动辅助 ---------------- */
namespace OHOS {
namespace NetManagerStandard {
namespace NapiMock {

/** 进程级共享 env：与被测全局变量（g_stateChangeTsf 等）生命周期一致 */
napi_env SharedEnv();

/** 清空异常/调用/Promise 记录（不释放已创建的值） */
void ResetEnvRecords(napi_env env);

/* 值构造 */
napi_value MakeUndefined(napi_env env);
napi_value MakeObject(napi_env env);
napi_value MakeNumber(napi_env env, int32_t value);
napi_value MakeBoolean(napi_env env, bool value);
napi_value MakeString(napi_env env, const std::string &value);
napi_value MakeFunction(napi_env env);
napi_value MakeUint8Array(napi_env env, const std::vector<uint8_t> &data);
napi_value MakeInt16Array(napi_env env, size_t length);
void SetProp(napi_env env, napi_value object, const std::string &name, napi_value value);
napi_value GetProp(napi_env env, napi_value object, const std::string &name);

/* callback_info 构造 */
napi_callback_info MakeCbInfo(napi_env env, std::initializer_list<napi_value> args);

/* 断言辅助 */
bool HasPendingException(napi_env env);
/** 读取挂起异常的 code 属性（CreateBusinessError 写入 int32），无异常返回 -1 */
int32_t PendingExceptionCode(napi_env env);
/** Promise 兑现辅助：返回首个拒绝值并标记存在 */
size_t ResolvedCount(napi_env env);
size_t RejectedCount(napi_env env);
napi_value__ *LastRejectedValue(napi_env env);
napi_value__ *LastResolvedValue(napi_env env);
size_t JsCallCount(napi_env env);
napi_env__::CallRecord *LastJsCall(napi_env env);

} // namespace NapiMock
} // namespace NetManagerStandard
} // namespace OHOS

#endif // NETMANAGER_EXT_TEST_NAPI_MOCK_H
