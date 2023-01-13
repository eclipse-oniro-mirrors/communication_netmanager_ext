/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#include "napi_ethernet.h"

#include <array>
#include <new>

#include <napi/native_common.h>

#include "ethernet_client.h"
#include "i_ethernet_service.h"
#include "iservice_registry.h"
#include "napi_common.h"
#include "netmgr_ext_log_wrapper.h"
#include "system_ability_definition.h"
#include "netmanager_base_common_utils.h"

// underside macro can delete context, if napi_status is not napi_ok.
#define NAPI_CALL_BASE_ENHANCE(env, theCall, retVal, context) \
    do {                                                      \
        if ((theCall) != napi_ok) {                           \
            delete context;                                   \
            context = nullptr;                                \
            GET_AND_THROW_LAST_ERROR((env));                  \
            return retVal;                                    \
        }                                                     \
    } while (0)                                               \

#define NAPI_CALL_ENHANCE(env, theCall, context) NAPI_CALL_BASE_ENHANCE(env, theCall, nullptr, context)

#define NAPI_CALL_RETURN_VOID_ENHANCE(env, theCall, context) \
    NAPI_CALL_BASE_ENHANCE(env, theCall, NAPI_RETVAL_NOTHING, context)

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr const char *DNS_SEPARATOR = ",";
constexpr int32_t DNS_MAX_SIZE = 10;
}
void NapiEthernet::ExecSetIfaceConfig(napi_env env, void *data)
{
    if (data == nullptr) {
        NETMGR_EXT_LOG_E("data is nullptr");
        return;
    }
    EthernetAsyncContext *context = static_cast<EthernetAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_EXT_LOG_E("context is nullptr");
        return;
    }
    if (context->ipMode == 0 && (context->ipAddr.empty() || context->netMask.empty())) {
        NETMGR_EXT_LOG_E("static ip or mask have empty");
        return;
    }
    INetAddr addr0;
    sptr<InterfaceConfiguration> config = new (std::nothrow) InterfaceConfiguration();
    if (config == nullptr) {
        NETMGR_EXT_LOG_E("config is nullptr");
        return;
    }
    config->mode_ = static_cast<IPSetMode>(context->ipMode);
    config->ipStatic_.ipAddr_.address_ = context->ipAddr;
    config->ipStatic_.route_.address_ = context->route;
    config->ipStatic_.gateway_.address_ = context->gateway;
    config->ipStatic_.netMask_.address_ = context->netMask;
    for (const auto &dns : CommonUtils::Split(context->dnsServers, DNS_SEPARATOR)) {
        INetAddr addr;
        addr.address_ = dns;
        config->ipStatic_.dnsServers_.push_back(addr);
        if (config->ipStatic_.dnsServers_.size() == DNS_MAX_SIZE) {
            break;
        }
    }
    config->ipStatic_.domain_ = context->domain;
    context->result = DelayedSingleton<EthernetClient>::GetInstance()->SetIfaceConfig(context->iface, config);
    NETMGR_EXT_LOG_D("ExecSetIfaceConfig result =[%{public}d]", context->result);
}

void NapiEthernet::CompleteSetIfaceConfig(napi_env env, napi_status status, void *data)
{
    if (data == nullptr) {
        NETMGR_EXT_LOG_E("data is nullptr");
        return;
    }
    EthernetAsyncContext *context = static_cast<EthernetAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_EXT_LOG_E("context is nullptr");
        return;
    }
    napi_value info = nullptr;
    if (context->result != ERR_NONE) {
        napi_create_int32(env, context->result, &info);
    } else {
        info = NapiCommon::CreateUndefined(env);
    }
    if (context->callbackRef == nullptr) {
        if (context->result != ERR_NONE) {
            NAPI_CALL_RETURN_VOID_ENHANCE(env, napi_reject_deferred(env, context->deferred, info), context);
        } else {
            NAPI_CALL_RETURN_VOID_ENHANCE(env, napi_resolve_deferred(env, context->deferred, info), context);
        }
    } else {
        napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        if (context->result != ERR_NONE) {
            callbackValues[CALLBACK_ARGV_INDEX_0] = info;
        } else {
            callbackValues[CALLBACK_ARGV_INDEX_1] = info;
        }
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

void NapiEthernet::ExecGetIfaceConfig(napi_env env, void *data)
{
    if (data == nullptr) {
        NETMGR_EXT_LOG_E("data is nullptr");
        return;
    }
    EthernetAsyncContext *context = static_cast<EthernetAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_EXT_LOG_E("context is nullptr");
        return;
    }
    sptr<InterfaceConfiguration> config =
        DelayedSingleton<EthernetClient>::GetInstance()->GetIfaceConfig(context->iface);
    if (config != nullptr) {
        context->result = 1;
        context->ipMode = config->mode_;
        context->ipAddr = config->ipStatic_.ipAddr_.address_;
        context->route = config->ipStatic_.route_.address_;
        context->gateway = config->ipStatic_.gateway_.address_;
        context->netMask = config->ipStatic_.netMask_.address_;
        context->domain = config->ipStatic_.domain_;
        for (uint32_t i = 0; i < config->ipStatic_.dnsServers_.size(); i++) {
            context->dnsServers = context->dnsServers + config->ipStatic_.dnsServers_[i].address_;
            if (config->ipStatic_.dnsServers_.size() - i > 1) {
                context->dnsServers = context->dnsServers + DNS_SEPARATOR;
            }
        }
    } else {
        context->result = -1;
    }
}

void NapiEthernet::CompleteGetIfaceConfig(napi_env env, napi_status status, void *data)
{
    if (data == nullptr) {
        NETMGR_EXT_LOG_E("data is nullptr");
        return;
    }
    EthernetAsyncContext *context = static_cast<EthernetAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_EXT_LOG_E("context is nullptr");
        return;
    }
    napi_value info = nullptr;
    napi_value infoFail = nullptr;
    napi_create_object(env, &info);
    napi_create_int32(env, context->result, &infoFail);
    NapiCommon::SetPropertyInt32(env, info, "mode", context->ipMode);
    NapiCommon::SetPropertyString(env, info, "ipAddr", context->ipAddr);
    NapiCommon::SetPropertyString(env, info, "route", context->route);
    NapiCommon::SetPropertyString(env, info, "gateway", context->gateway);
    NapiCommon::SetPropertyString(env, info, "netMask", context->netMask);
    NapiCommon::SetPropertyString(env, info, "dnsServers", context->dnsServers);
    NapiCommon::SetPropertyString(env, info, "domain", context->domain);
    if (context->callbackRef == nullptr) {
        if (context->result == -1) {
            NAPI_CALL_RETURN_VOID_ENHANCE(env, napi_reject_deferred(env, context->deferred, infoFail), context);
        } else {
            NAPI_CALL_RETURN_VOID_ENHANCE(env, napi_resolve_deferred(env, context->deferred, info), context);
        }
    } else {
        napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        if (context->result == -1) {
            callbackValues[ARGV_INDEX_0] = infoFail;
        } else {
            callbackValues[ARGV_INDEX_1] = info;
        }
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

void NapiEthernet::ExecIsIfaceActive(napi_env env, void *data)
{
    if (data == nullptr) {
        NETMGR_EXT_LOG_E("data is nullptr");
        return;
    }
    EthernetAsyncContext *context = static_cast<EthernetAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_EXT_LOG_E("context is nullptr");
        return;
    }
    if (context->isIface) {
        context->ifActivate = DelayedSingleton<EthernetClient>::GetInstance()->IsIfaceActive(context->iface);
    } else {
        context->ifActivate = DelayedSingleton<EthernetClient>::GetInstance()->GetAllActiveIfaces().size() > 0;
    }
    NETMGR_EXT_LOG_D("ifActivate == [%{public}d]", context->ifActivate);
}

void NapiEthernet::CompleteIsIfaceActive(napi_env env, napi_status status, void *data)
{
    if (data == nullptr) {
        NETMGR_EXT_LOG_E("data is nullptr");
        return;
    }
    EthernetAsyncContext *context = static_cast<EthernetAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_EXT_LOG_E("context is nullptr");
        return;
    }
    napi_value info = nullptr;
    napi_create_int32(env, context->ifActivate, &info);
    if (context->callbackRef == nullptr) {
        NAPI_CALL_RETURN_VOID_ENHANCE(env, napi_resolve_deferred(env, context->deferred, info), context);
    } else {
        napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        callbackValues[ARGV_INDEX_1] = info;
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

void NapiEthernet::ExecGetAllActiveIfaces(napi_env env, void *data)
{
    if (data == nullptr) {
        NETMGR_EXT_LOG_E("data is nullptr");
        return;
    }
    EthernetAsyncContext *context = static_cast<EthernetAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_EXT_LOG_E("context is nullptr");
        return;
    }
    context->ethernetNameList = DelayedSingleton<EthernetClient>::GetInstance()->GetAllActiveIfaces();
}

void NapiEthernet::CompleteGetAllActiveIfaces(napi_env env, napi_status status, void *data)
{
    if (data == nullptr) {
        NETMGR_EXT_LOG_E("data is nullptr");
        return;
    }
    EthernetAsyncContext *context = static_cast<EthernetAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_EXT_LOG_E("context is nullptr");
        return;
    }

    // creat function return
    napi_value infoAttay = nullptr;
    napi_value info = nullptr;
    if (context->ethernetNameList.size() > 0) {
        napi_create_array_with_length(env, context->ethernetNameList.size(), &infoAttay);
        for (size_t index = 0; index < context->ethernetNameList.size(); index++) {
            napi_create_string_utf8(env, context->ethernetNameList[index].c_str(), NAPI_AUTO_LENGTH, &info);
            napi_set_element(env, infoAttay, index, info);
        }
    } else {
        napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &infoAttay);
    }
    if (context->callbackRef == nullptr) {
        NAPI_CALL_RETURN_VOID_ENHANCE(env, napi_resolve_deferred(env, context->deferred, infoAttay), context);
    } else {
        napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        callbackValues[CALLBACK_ARGV_INDEX_1] = infoAttay;
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

napi_value NapiEthernet::SetIfaceConfig(napi_env env, napi_callback_info info)
{
    size_t argc = ARGV_NUM_3;
    napi_value argv[] = {nullptr, nullptr, nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    EthernetAsyncContext *context = new EthernetAsyncContext();

    NAPI_CALL_ENHANCE(env, napi_get_value_string_utf8(env, argv[ARGV_INDEX_0], context->iface, ETHERNET_NAME_MAX_BYTE,
                                              &(context->ethernetNameRealBytes)), context);

    NapiCommon::GetPropertyInt32(env, argv[ARGV_INDEX_1], "mode", context->ipMode);
    if (context->ipMode == IPSetMode::STATIC) {
        NapiCommon::GetPropertyString(env, argv[ARGV_INDEX_1], "ipAddr", context->ipAddr);
        NapiCommon::GetPropertyString(env, argv[ARGV_INDEX_1], "route", context->route);
        NapiCommon::GetPropertyString(env, argv[ARGV_INDEX_1], "gateway", context->gateway);
        NapiCommon::GetPropertyString(env, argv[ARGV_INDEX_1], "netMask", context->netMask);
        NapiCommon::GetPropertyString(env, argv[ARGV_INDEX_1], "dnsServers", context->dnsServers);
        NapiCommon::GetPropertyString(env, argv[ARGV_INDEX_1], "domain", context->domain);
    }
    napi_value result = nullptr;
    if (argc == ARGV_NUM_2) {
        if (context->callbackRef == nullptr) {
            NAPI_CALL_ENHANCE(env, napi_create_promise(env, &context->deferred, &result), context);
        } else {
            NAPI_CALL_ENHANCE(env, napi_get_undefined(env, &result), context);
        }
    } else if (argc == ARGV_NUM_3) {
        NAPI_CALL_ENHANCE(env, napi_create_reference(env, argv[ARGV_INDEX_2], CALLBACK_REF_CNT,
            &context->callbackRef), context);
    } else {
        NETMGR_EXT_LOG_E("SetIfaceConfig  exception");
    }

    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL_ENHANCE(env, napi_get_undefined(env, &resource), context);
    NAPI_CALL_ENHANCE(env, napi_create_string_utf8(env, "SetIfaceConfig", NAPI_AUTO_LENGTH, &resourceName), context);
    NAPI_CALL_ENHANCE(env, napi_create_async_work(env, resource, resourceName, ExecSetIfaceConfig,
        CompleteSetIfaceConfig, (void *)context, &context->work), context);
    NAPI_CALL_ENHANCE(env, napi_queue_async_work(env, context->work), context);
    return result;
}

napi_value NapiEthernet::GetIfaceConfig(napi_env env, napi_callback_info info)
{
    size_t argc = ARGV_NUM_2;
    napi_value argv[] = {nullptr, nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    EthernetAsyncContext *context = new EthernetAsyncContext();

    NAPI_CALL_ENHANCE(env, napi_get_value_string_utf8(env, argv[ARGV_INDEX_0], context->iface, ETHERNET_NAME_MAX_BYTE,
                                              &(context->ethernetNameRealBytes)), context);

    napi_value result = nullptr;
    if (argc == ARGV_NUM_1) {
        if (context->callbackRef == nullptr) {
            NAPI_CALL_ENHANCE(env, napi_create_promise(env, &context->deferred, &result), context);
        } else {
            NAPI_CALL_ENHANCE(env, napi_get_undefined(env, &result), context);
        }
    } else if (argc == ARGV_NUM_2) {
        NAPI_CALL_ENHANCE(env, napi_create_reference(env, argv[ARGV_INDEX_1], CALLBACK_REF_CNT,
            &context->callbackRef), context);
    } else {
        NETMGR_EXT_LOG_E("GetIfaceConfig  exception");
    }

    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL_ENHANCE(env, napi_get_undefined(env, &resource), context);
    NAPI_CALL_ENHANCE(env, napi_create_string_utf8(env, "GetIfaceConfig", NAPI_AUTO_LENGTH, &resourceName), context);
    NAPI_CALL_ENHANCE(env, napi_create_async_work(env, resource, resourceName, ExecGetIfaceConfig,
        CompleteGetIfaceConfig, (void *)context, &context->work), context);
    NAPI_CALL_ENHANCE(env, napi_queue_async_work(env, context->work), context);
    return result;
}

napi_value NapiEthernet::IsIfaceActive(napi_env env, napi_callback_info info)
{
    size_t argc = ARGV_NUM_2;
    napi_value argv[] = {nullptr, nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    EthernetAsyncContext *context = new EthernetAsyncContext();

    if (argc != ARGV_NUM_0 && NapiCommon::MatchValueType(env, argv[ARGV_INDEX_0], napi_string)) {
        NAPI_CALL_ENHANCE(env, napi_get_value_string_utf8(env, argv[ARGV_INDEX_0], context->iface,
            ETHERNET_NAME_MAX_BYTE, &(context->ethernetNameRealBytes)), context);
        NETMGR_EXT_LOG_E("IsIfaceActive [%{public}s]", context->iface);
    }
    napi_value result = nullptr;
    if (argc == ARGV_NUM_0) {
        context->isIface = false;
        NAPI_CALL_ENHANCE(env, napi_create_promise(env, &context->deferred, &result), context);
    } else if (argc == ARGV_NUM_1) {
        if (NapiCommon::MatchValueType(env, argv[ARGV_INDEX_0], napi_function)) {
            context->isIface = false;
            NAPI_CALL_ENHANCE(env, napi_create_reference(env, argv[ARGV_INDEX_0], CALLBACK_REF_CNT,
                &context->callbackRef), context);
        } else {
            context->isIface = true;
            if (context->callbackRef == nullptr) {
                NAPI_CALL_ENHANCE(env, napi_create_promise(env, &context->deferred, &result), context);
            } else {
                NAPI_CALL_ENHANCE(env, napi_get_undefined(env, &result), context);
            }
        }
    } else if (argc == ARGV_NUM_2) {
        context->isIface = true;
        NAPI_CALL_ENHANCE(env, napi_create_reference(env, argv[ARGV_INDEX_1], CALLBACK_REF_CNT,
            &context->callbackRef), context);
    } else {
        NETMGR_EXT_LOG_E("IsIfaceActive  exception");
    }

    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL_ENHANCE(env, napi_get_undefined(env, &resource), context);
    NAPI_CALL_ENHANCE(env, napi_create_string_utf8(env, "IsIfaceActive", NAPI_AUTO_LENGTH, &resourceName), context);
    NAPI_CALL_ENHANCE(env, napi_create_async_work(env, resource, resourceName, ExecIsIfaceActive, CompleteIsIfaceActive,
                                          (void *)context, &context->work), context);
    NAPI_CALL_ENHANCE(env, napi_queue_async_work(env, context->work), context);
    return result;
}

napi_value NapiEthernet::GetAllActiveIfaces(napi_env env, napi_callback_info info)
{
    size_t argc = ARGV_NUM_1;
    napi_value argv[] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    EthernetAsyncContext *context = new EthernetAsyncContext();
    napi_value result = nullptr;
    if (argc == ARGV_NUM_0) {
        if (context->callbackRef == nullptr) {
            NAPI_CALL_ENHANCE(env, napi_create_promise(env, &context->deferred, &result), context);
        } else {
            NAPI_CALL_ENHANCE(env, napi_get_undefined(env, &result), context);
        }
    } else if (argc == ARGV_NUM_1) {
        NAPI_CALL_ENHANCE(env, napi_create_reference(env, argv[ARGV_INDEX_0], CALLBACK_REF_CNT,
            &context->callbackRef), context);
    } else {
        NETMGR_EXT_LOG_E("GetAllActiveIfaces  exception.");
    }

    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL_ENHANCE(env, napi_get_undefined(env, &resource), context);
    NAPI_CALL_ENHANCE(env, napi_create_string_utf8(env, "GetAllActiveIfaces", NAPI_AUTO_LENGTH, &resourceName),
        context);
    NAPI_CALL_ENHANCE(env, napi_create_async_work(env, resource, resourceName, ExecGetAllActiveIfaces,
                                          CompleteGetAllActiveIfaces, (void *)context, &context->work), context);
    NAPI_CALL_ENHANCE(env, napi_queue_async_work(env, context->work), context);
    return result;
}

napi_value NapiEthernet::DeclareEthernetData(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("STATIC",
                                     NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(IPSetMode::STATIC))),
        DECLARE_NAPI_STATIC_PROPERTY("DHCP", NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(IPSetMode::DHCP))),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

napi_value NapiEthernet::DeclareEthernetInterface(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("setIfaceConfig", SetIfaceConfig),
        DECLARE_NAPI_FUNCTION("getIfaceConfig", GetIfaceConfig),
        DECLARE_NAPI_FUNCTION("isIfaceActive", IsIfaceActive),
        DECLARE_NAPI_FUNCTION("getAllActiveIfaces", GetAllActiveIfaces),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

napi_value NapiEthernet::RegisterEthernetInterface(napi_env env, napi_value exports)
{
    DeclareEthernetInterface(env, exports);
    DeclareEthernetData(env, exports);
    return nullptr;
}

static napi_module _ethernetModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = NapiEthernet::RegisterEthernetInterface,
    .nm_modname = "net.ethernet",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterEthernetModule(void)
{
    napi_module_register(&_ethernetModule);
}
} // namespace NetManagerStandard
} // namespace OHOS
