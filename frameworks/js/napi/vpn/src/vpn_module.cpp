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

#include <cstdint>

#include "errorcode_convertor.h"
#include "module_template.h"
#include "napi_utils.h"
#include "net_manager_constants.h"
#include "netmanager_ext_log.h"
#include "networkvpn_client.h"
#include "vpn_connection.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t ARG_NUM_0 = 0;
constexpr int32_t PARAM_ONE = 1;

static void *MakeData(napi_env env, size_t argc, napi_value *argv, EventManager *manager)
{
    if ((argc != PARAM_ONE) || (NapiUtils::GetValueType(env, argv[ARG_NUM_0]) != napi_object)) {
        NETMANAGER_EXT_LOGE("funciton prameter error");
        napi_throw_error(env, std::to_string(NETMANAGER_EXT_ERR_PARAMETER_ERROR).c_str(), "Parameter error");
        return nullptr;
    }

    int32_t ret = NetworkVpnClient::GetInstance().CreateVpnConnection();
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMANAGER_EXT_LOGE("execute CreateVpnConnection failed: %{public}d", ret);
        std::string errorMsg = NetBaseErrorCodeConvertor().ConvertErrorCode(ret);
        napi_throw_error(env, std::to_string(ret).c_str(), errorMsg.c_str());
        return nullptr;
    }
    return reinterpret_cast<void *>(&NetworkVpnClient::GetInstance());
}

static napi_value CreateVpnConnection(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::NewInstance(env, info, VPN_CONNECTION, MakeData, [](napi_env, void *data, void *) {
        NETMANAGER_EXT_LOGI("finalize VpnConnection");
    });
}

napi_value RegisterVpnModule(napi_env env, napi_value exports)
{
    NapiUtils::DefineProperties(env, exports,
                                {
                                    DECLARE_NAPI_FUNCTION(CREATE_VPN_CONNECTION, CreateVpnConnection),
                                });
    ModuleTemplate::DefineClass(env, exports,
                                {
                                    DECLARE_NAPI_FUNCTION(SET_UP, VpnConnection::SetUp),
                                    DECLARE_NAPI_FUNCTION(PROTECT, VpnConnection::Protect),
                                    DECLARE_NAPI_FUNCTION(DESTROY, VpnConnection::Destroy),
                                },
                                VPN_CONNECTION);
    return exports;
}

static napi_module g_vpnModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = RegisterVpnModule,
    .nm_modname = VPN_MODULE_NAME,
    .nm_priv = nullptr,
    .reserved = {nullptr},
};

extern "C" __attribute__((constructor)) void VpnNapiRegister()
{
    napi_module_register(&g_vpnModule);
}
} // namespace NetManagerStandard
} // namespace OHOS