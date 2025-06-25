/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
 
#ifndef NET_EXT_NAPI_GET_DEVICE_INFOMATION_CONTEXT_H
#define NET_EXT_NAPI_GET_DEVICE_INFOMATION_CONTEXT_H
 
#include <string>
#include <napi/native_api.h>
 
#include "base_context.h"
#include "ethernet_device_info.h"
 
namespace OHOS {
namespace NetManagerStandard {
class GetDeviceInformationContext : public BaseContext {
public:
    GetDeviceInformationContext() = delete;
    explicit GetDeviceInformationContext(napi_env env, std::shared_ptr<EventManager>& manager);
 
    void ParseParams(napi_value *params, size_t paramsCount);
 
public:
    std::vector<EthernetDeviceInfo> deviceInfo_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_EXT_NAPI_GET_DEVICE_INFOMATION_CONTEXT_H