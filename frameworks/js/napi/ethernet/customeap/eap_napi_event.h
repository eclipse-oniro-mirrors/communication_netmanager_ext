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
#ifndef EAP_NAPI_EVENT_H
#define EAP_NAPI_EVENT_H

#include <napi/native_api.h>
#include "eap_data.h"
#include "eap_event_mgr.h"
#include "eth_eap_profile.h"
#include "eth_eap_config.h"

namespace OHOS {
namespace NetManagerStandard {

napi_value RegCustomEapHandler(napi_env env, napi_callback_info cbinfo);
napi_value UnRegCustomEapHandler(napi_env env, napi_callback_info cbinfo);
napi_value ReplyCustomEapData(napi_env env, napi_callback_info cbinfo);
napi_value StartEthEap(napi_env env, napi_callback_info info);
napi_value LogOffEthEap(napi_env env, napi_callback_info info);

#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
napi_value SetEthEapConfig(napi_env env, napi_callback_info info);
napi_value GetEthEapConfig(napi_env env, napi_callback_info info);
napi_value DeleteEthEapConfig(napi_env env, napi_callback_info info);
napi_value GetEthEapStateInfo(napi_env env, napi_callback_info info);
napi_value OnStateChange(napi_env env, napi_callback_info info);
napi_value OffStateChange(napi_env env, napi_callback_info info);

/** 认证后断连或再次连接（JS 侧 connect，Promise）：action 0=按已保存配置重连认证；1=停止自动认证并登出 */
napi_value Connect(napi_env env, napi_callback_info info);
#endif

} // namespace NetManagerStandard
} // namespace OHOS

#endif // EAP_NAPI_EVENT_H