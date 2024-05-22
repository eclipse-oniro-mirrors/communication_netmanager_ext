/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef VPN_CONNECTION_H
#define VPN_CONNECTION_H

#include <napi/native_api.h>

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr const char *VPN_MODULE_NAME = "net.vpn";
constexpr const char *CREATE_VPN_CONNECTION = "createVpnConnection";
constexpr const char *VPN_CONNECTION = "VpnConnection";
constexpr const char *PREPARE = "prepare";
constexpr const char *SET_UP = "setUp";
constexpr const char *PROTECT = "protect";
constexpr const char *DESTROY = "destroy";
constexpr const char *ON = "on";
constexpr const char *OFF = "off";
constexpr const char *SAVE_SYSTEM_VPN = "saveSystemVpn";
constexpr const char *DELETE_SYSTEM_VPN = "deleteSystemVpn";
constexpr const char *GET_SYSTEM_VPN_LIST = "getSystemVpnList";
constexpr const char *GET_SYSTEM_VPN = "getSystemVpn";
constexpr const char *GET_CONNECTED_SYSTEM_VPN = "getConnectedSystemVpn";
} // namespace

namespace VpnConnection {
napi_value Prepare(napi_env env, napi_callback_info info);
napi_value SetUp(napi_env env, napi_callback_info info);
napi_value Protect(napi_env env, napi_callback_info info);
napi_value Destroy(napi_env env, napi_callback_info info);
napi_value On(napi_env env, napi_callback_info info);
napi_value Off(napi_env env, napi_callback_info info);
napi_value SaveSystemVpn(napi_env env, napi_callback_info info);
napi_value DeleteSystemVpn(napi_env env, napi_callback_info info);
napi_value GetSystemVpnList(napi_env env, napi_callback_info info);
napi_value GetSystemVpn(napi_env env, napi_callback_info info);
napi_value GetConnectedSystemVpn(napi_env env, napi_callback_info info);
} // namespace VpnConnection
} // namespace NetManagerStandard
} // namespace OHOS
#endif // VPN_CONNECTION_H