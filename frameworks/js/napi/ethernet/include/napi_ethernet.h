/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef NAPI_ETHERNET_H
#define NAPI_ETHERNET_H

#include <string>
#include <vector>
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace NetManagerStandard {
// ethernet name max bytes
constexpr int32_t ETHERNET_NAME_MAX_BYTE = 32;
// ip address max bytes
constexpr int32_t IP_ADDR_MAX_BYTE = 128;
// ip address type [static]
const std::string STATIC_IP = "static";
// ip address type [dynamic]
const std::string DYNAMIC_IP = "dynamic";

struct EthernetAsyncContext {
    napi_async_work work = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callbackRef = nullptr;
    // Data context
    char ethernetName[ETHERNET_NAME_MAX_BYTE] = {0};
    size_t ethernetNameRealBytes = 0;
    int32_t ifActivate = 0;
    // get all ethernetName
    std::vector<std::string> ethernetNameList;
    // [0] static, [1] dhcp
    int32_t ipMode = 0;
    // ip address info
    std::string ipAddrInfo;
    // route address info
    std::string routeAddrInfo;
    // gate address info
    std::string gateAddrInfo;
    // mask address info
    std::string maskAddrInfo;
    // dns0 address info
    std::string dns0AddrInfo;
    // dns1 address info
    std::string dns1AddrInfo;
    int32_t result = 0;
};

class NapiEthernet {
public:
    NapiEthernet();
    ~NapiEthernet() = default;
    static napi_value RegisterEthernetInterface(napi_env env, napi_value exports);
    static napi_value DeclareEthernetInterface(napi_env env, napi_value exports);
    static napi_value DeclareEthernetData(napi_env env, napi_value exports);

    static void ExecSetIfaceConfig(napi_env env, void *data);
    static void CompleteSetIfaceConfig(napi_env env, napi_status status, void *data);
    static void ExecGetIfaceConfig(napi_env env, void *data);
    static void CompleteGetIfaceConfig(napi_env env, napi_status status, void *data);
    static void ExecIsIfaceActive(napi_env env, void *data);
    static void CompleteIsIfaceActive(napi_env env, napi_status status, void *data);
    static void ExecGetAllActiveIfaces(napi_env env, void *data);
    static void CompleteGetAllActiveIfaces(napi_env env, napi_status status, void *data);
    // define napi interface for JS
    static napi_value SetIfaceConfig(napi_env env, napi_callback_info info);
    static napi_value GetIfaceConfig(napi_env env, napi_callback_info info);
    static napi_value IsIfaceActive(napi_env env, napi_callback_info info);
    static napi_value GetAllActiveIfaces(napi_env env, napi_callback_info info);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NAPI_ETHERNET_H
