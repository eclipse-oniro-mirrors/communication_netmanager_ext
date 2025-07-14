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
 
#ifndef COMMUNICATION_NET_MANAGER_BASE_EAP_MODULE_H
#define COMMUNICATION_NET_MANAGER_BASE_EAP_MODULE_H
 
#include <napi/native_api.h>
#include "eap_napi_event.h"
 
namespace OHOS {
namespace NetManagerStandard {
class EapModule final {
public:
    static constexpr const char *interfaceNetEapCustomResult = "CustomResult";
    static constexpr const char *functionRegCustomEapHandler = "regCustomEapHandler";
    static constexpr const char *functionUnregCustomEapHandler = "unregCustomEapHandler";
    static constexpr const char *functionReplyCustomEapData = "replyCustomEapData";
 
    static napi_value InitEapModule(napi_env env, napi_value exports);
private:
    static void InitProperties(napi_env env, napi_value exports);
    static void DeclareEapMethod(napi_env env, napi_value exports);
    static void DeclarePhase2Method(napi_env env, napi_value exports);
};
} // namespace OHOS::NetManagerStandard
} // OHOS
 
#endif /* COMMUNICATION_NET_MANAGER_BASE_EAP_MODULE_H */