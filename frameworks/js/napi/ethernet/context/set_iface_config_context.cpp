/*
 * Copyright (C) 2022-2023 Huawei Device Co., Ltd.
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

#include "set_iface_config_context.h"

#include <new>

#include "constant.h"
#include "napi_utils.h"
#include "net_manager_constants.h"
#include "netmanager_base_log.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr const char *OBJECT_HTTP_RPPXY = "httpProxy";
constexpr const char *HTTP_RPPXY_HOST = "host";
constexpr const char *HTTP_RPPXY_PORT = "port";
constexpr const char *HTTP_RPPXY_EXCLUSION_LIST = "exclusionList";
bool CheckParamsType(napi_env env, napi_value *params, size_t paramsCount)
{
    if (paramsCount == PARAM_DOUBLE_OPTIONS || paramsCount == PARAM_DOUBLE_OPTIONS_AND_CALLBACK) {
        if (NapiUtils::GetValueType(env, params[ARG_NUM_0]) != napi_string ||
            NapiUtils::GetValueType(env, params[ARG_NUM_1]) != napi_object) {
            return false;
        }
    } else {
        // if paramsCount is not 2 or 3, means count error.
        return false;
    }
    return true;
}
} // namespace

SetIfaceConfigContext::SetIfaceConfigContext(napi_env env, EventManager *manager) : BaseContext(env, manager) {}

void SetIfaceConfigContext::ParseParams(napi_value *params, size_t paramsCount)
{
    if (!CheckParamsType(GetEnv(), params, paramsCount)) {
        SetNeedThrowException(true);
        SetErrorCode(NETMANAGER_EXT_ERR_PARAMETER_ERROR);
        return;
    }
    iface_ = NapiUtils::GetStringFromValueUtf8(GetEnv(), params[ARG_NUM_0]);
    config_ = new (std::nothrow) InterfaceConfiguration();
    if (config_ == nullptr) {
        SetParseOK(false);
        return;
    }
    config_->mode_ = static_cast<IPSetMode>(NapiUtils::GetInt32Property(GetEnv(), params[ARG_NUM_1], "mode"));
    std::string ipAddresses = NapiUtils::GetStringPropertyUtf8(GetEnv(), params[ARG_NUM_1], "ipAddr");
    StaticConfiguration::ExtractNetAddrBySeparator(ipAddresses, config_->ipStatic_.ipAddrList_);

    std::string routeAddresses = NapiUtils::GetStringPropertyUtf8(GetEnv(), params[ARG_NUM_1], "route");
    StaticConfiguration::ExtractNetAddrBySeparator(routeAddresses, config_->ipStatic_.routeList_);

    std::string gatewayAddresses = NapiUtils::GetStringPropertyUtf8(GetEnv(), params[ARG_NUM_1], "gateway");
    StaticConfiguration::ExtractNetAddrBySeparator(gatewayAddresses, config_->ipStatic_.gatewayList_);

    std::string maskAddresses = NapiUtils::GetStringPropertyUtf8(GetEnv(), params[ARG_NUM_1], "netMask");
    StaticConfiguration::ExtractNetAddrBySeparator(maskAddresses, config_->ipStatic_.netMaskList_);

    std::string dnsServers = NapiUtils::GetStringPropertyUtf8(GetEnv(), params[ARG_NUM_1], "dnsServers");
    StaticConfiguration::ExtractNetAddrBySeparator(dnsServers, config_->ipStatic_.dnsServers_);

    config_->ipStatic_.domain_ = NapiUtils::GetStringPropertyUtf8(GetEnv(), params[ARG_NUM_1], "domain");

    ParseHttpProxy(params);
    SetParseOK((paramsCount == PARAM_DOUBLE_OPTIONS_AND_CALLBACK) ? (SetCallback(params[ARG_NUM_2]) == napi_ok) : true);
}

void SetIfaceConfigContext::ParseHttpProxy(napi_value *params)
{
    if (!NapiUtils::HasNamedProperty(GetEnv(), params[ARG_NUM_1], OBJECT_HTTP_RPPXY)) {
        NETMANAGER_BASE_LOGE("Do not use HttpProxy.");
        return;
    }
    napi_value value = NapiUtils::GetNamedProperty(GetEnv(), params[ARG_NUM_1], OBJECT_HTTP_RPPXY);
    if (NapiUtils::GetValueType(GetEnv(), value) != napi_object) {
        NETMANAGER_BASE_LOGE("httpProxy is not a object");
        return;
    }

    std::string host = NapiUtils::GetStringPropertyUtf8(GetEnv(), value, HTTP_RPPXY_HOST);
    uint16_t port =
        host.empty() ? 0 : static_cast<uint16_t>(NapiUtils::GetUint32Property(GetEnv(), value, HTTP_RPPXY_PORT));
    std::list<std::string> exclusionList;
    if (!host.empty()) {
        napi_value exclusionsValue = NapiUtils::GetNamedProperty(GetEnv(), value, HTTP_RPPXY_EXCLUSION_LIST);
        uint32_t size = NapiUtils::GetArrayLength(GetEnv(), exclusionsValue);
        for (uint32_t i = 0; i < size; ++i) {
            napi_value element = NapiUtils::GetArrayElement(GetEnv(), exclusionsValue, i);
            exclusionList.push_back(NapiUtils::GetStringFromValueUtf8(GetEnv(), element));
        }
    }

    config_->httpProxy_.SetHost(std::move(host));
    config_->httpProxy_.SetPort(port);
    config_->httpProxy_.SetExclusionList(exclusionList);
}
} // namespace NetManagerStandard
} // namespace OHOS
