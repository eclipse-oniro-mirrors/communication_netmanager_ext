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

#include "napi_ethernet.h"
#include <memory>
#include "system_ability_definition.h"
#include "iservice_registry.h"
#include "netmgr_ext_log_wrapper.h"
#include "i_ethernet_service.h"
#include "ethernet_client.h"
#include "napi_common.h"

namespace OHOS {
namespace NetManagerStandard {
NapiEthernet::NapiEthernet() {}

void NapiEthernet::ExecSetIfaceConfig(napi_env env, void *data)
{
    NETMGR_EXT_LOG_D("ExecSetIfaceConfig");
    EthernetAsyncContext *context = (EthernetAsyncContext *)data;
    if (context == nullptr) {
        NETMGR_EXT_LOG_E("context == nullptr");
        return;
    }
    INetAddr addr0;
    sptr<InterfaceConfiguration> config = std::make_unique<InterfaceConfiguration>().release();
    if (config == nullptr) {
        NETMGR_EXT_LOG_E("config == nullptr");
        return;
    }
    addr0.address_ = context->dnsServers;
    config->mode_ = static_cast<IPSetMode>(context->ipMode);
    config->ipStatic_.ipAddr_.address_ = context->ipAddr;
    config->ipStatic_.route_.address_ = context->route;
    config->ipStatic_.gateway_.address_ = context->gateway;
    config->ipStatic_.netMask_.address_ = context->netMask;
    config->ipStatic_.dnsServers_.push_back(addr0);
    config->ipStatic_.domain_ = context->domain;
    context->result = DelayedSingleton<EthernetClient>::GetInstance()->SetIfaceConfig(
        context->iface, config);
    NETMGR_EXT_LOG_D("ExecSetIfaceConfig result =[%{public}d]", context->result);
}

void NapiEthernet::CompleteSetIfaceConfig(napi_env env, napi_status status, void *data)
{
    NETMGR_EXT_LOG_D("CompleteSetIfaceConfig");
    EthernetAsyncContext* context = static_cast<EthernetAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_EXT_LOG_E("context == nullptr");
        return;
    }
    napi_value info = nullptr;
    if (context->result != ERR_NONE) {
        napi_create_int32(env, context->result, &info);
    } else {
        info = NapiCommon::CreateUndefined(env);
    }
    if (context->callbackRef == nullptr) {
        // promiss return
        if (context->result != ERR_NONE) {
            NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, context->deferred, info));
        } else {
            NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, info));
        }
    } else {
        // call back return
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
    NETMGR_EXT_LOG_I("ExecGetIfaceConfig");
    EthernetAsyncContext *context = (EthernetAsyncContext *)data;
    if (context == nullptr) {
        NETMGR_EXT_LOG_E("context == nullptr");
        return;
    }
    sptr<InterfaceConfiguration> config =
        DelayedSingleton<EthernetClient>::GetInstance()->GetIfaceConfig(context->iface);
    if (config != nullptr) {
        context->result = 1;
        std::string tap;
        context->ipMode = config->mode_;
        NETMGR_EXT_LOG_I("config->mode_ = [%{public}d]", config->mode_);
        context->ipAddr = config->ipStatic_.ipAddr_.address_;
        NETMGR_EXT_LOG_I("config->ipAddr_ = [%{public}s]", config->ipStatic_.ipAddr_.address_.c_str());
        context->route = config->ipStatic_.route_.address_;
        NETMGR_EXT_LOG_I("config->route_ = [%{public}s]", config->ipStatic_.route_.address_.c_str());
        context->gateway = config->ipStatic_.gateway_.address_;
        NETMGR_EXT_LOG_I("config->gateway_ = [%{public}s]", config->ipStatic_.gateway_.address_.c_str());
        context->netMask = config->ipStatic_.netMask_.address_;
        NETMGR_EXT_LOG_I("config->netMask_ = [%{public}s]", config->ipStatic_.netMask_.address_.c_str());
        context->domain = config->ipStatic_.domain_;
        NETMGR_EXT_LOG_I("config->domain = [%{public}s]", config->ipStatic_.domain_.c_str());
        for (auto it = config->ipStatic_.dnsServers_.begin(); it != config->ipStatic_.dnsServers_.end(); ++it) {
            if (context->dnsServers.empty()) {
                context->dnsServers = it->address_;
                NETMGR_EXT_LOG_I("config->dnsServers_ = [%{public}s]", it->ToString(tap).c_str());
            }
        }
    } else {
        context->result = -1;
    }
}

void NapiEthernet::CompleteGetIfaceConfig(napi_env env, napi_status status, void *data)
{
    NETMGR_EXT_LOG_I("CompleteGetIfaceConfig");
    EthernetAsyncContext* context = static_cast<EthernetAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_EXT_LOG_E("context == nullptr");
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
            NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, context->deferred, infoFail));
        } else {
            NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, info));
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
    NETMGR_EXT_LOG_I("ExecIsIfaceActive");
    EthernetAsyncContext *context = (EthernetAsyncContext *)data;
    if (context == nullptr) {
        NETMGR_EXT_LOG_E("context == nullptr");
        return;
    }
    context->ifActivate = DelayedSingleton<EthernetClient>::GetInstance()->IsIfaceActive(context->iface);
    NETMGR_EXT_LOG_I("ifActivate == [%{public}d]", context->ifActivate);
}

void NapiEthernet::CompleteIsIfaceActive(napi_env env, napi_status status, void *data)
{
    NETMGR_EXT_LOG_I("CompleteIsIfaceActive");
    EthernetAsyncContext* context = static_cast<EthernetAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_EXT_LOG_E("context == nullptr");
        return;
    }
    napi_value info = nullptr;
    napi_create_int32(env, context->ifActivate, &info);
    if (context->callbackRef == nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, info));
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
    NETMGR_EXT_LOG_I("ExecGetAllActiveIfaces");
    EthernetAsyncContext *context = (EthernetAsyncContext *)data;
    if (context == nullptr) {
        NETMGR_EXT_LOG_E("context == nullptr");
        return;
    }
    context->ethernetNameList = DelayedSingleton<EthernetClient>::GetInstance()->GetAllActiveIfaces();
}

void NapiEthernet::CompleteGetAllActiveIfaces(napi_env env, napi_status status, void *data)
{
    NETMGR_EXT_LOG_I("CompleteGetAllActiveIfaces");
    EthernetAsyncContext* context = static_cast<EthernetAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_EXT_LOG_E("context == nullptr");
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
        NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, infoAttay));
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
    NETMGR_EXT_LOG_I("SetIfaceConfig");
    size_t argc = ARGV_NUM_3;
    napi_value argv[] = {nullptr, nullptr, nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NETMGR_EXT_LOG_I("SetIfaceConfig agvc = [%{public}zu]", argc);
    EthernetAsyncContext* context = std::make_unique<EthernetAsyncContext>().release();
    // Parse Js argv
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[ARGV_INDEX_0],
        context->iface, ETHERNET_NAME_MAX_BYTE, &(context->ethernetNameRealBytes)));
    NETMGR_EXT_LOG_I("SetIfaceConfig iface=[%{public}s]", context->iface);
    // Parse Js object [ip]
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
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result));
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_3) {
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_2], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_EXT_LOG_E("SetIfaceConfig  exception");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "SetIfaceConfig", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecSetIfaceConfig,
        CompleteSetIfaceConfig,
        (void *)context,
        &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiEthernet::GetIfaceConfig(napi_env env, napi_callback_info info)
{
    NETMGR_EXT_LOG_I("GetIfaceConfig");
    size_t argc = ARGV_NUM_2;
    napi_value argv[] = {nullptr, nullptr} ;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NETMGR_EXT_LOG_I("GetIfaceConfig agvc = [%{public}zu]", argc);
    EthernetAsyncContext* context = std::make_unique<EthernetAsyncContext>().release();
    // Parse Js argv
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[ARGV_INDEX_0],
        context->iface, ETHERNET_NAME_MAX_BYTE, &(context->ethernetNameRealBytes)));
    NETMGR_EXT_LOG_I("GetIfaceConfig [%{public}s]", context->iface);
    napi_value result = nullptr;
    if (argc == ARGV_NUM_1) {
        if (context->callbackRef == nullptr) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result));
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_2) {
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_1], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_EXT_LOG_E("GetIfaceConfig  exception");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "GetIfaceConfig", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecGetIfaceConfig,
        CompleteGetIfaceConfig,
        (void *)context,
        &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiEthernet::IsIfaceActive(napi_env env, napi_callback_info info)
{
    NETMGR_EXT_LOG_I("IsIfaceActive");
    size_t argc = ARGV_NUM_2;
    napi_value argv[] = {nullptr, nullptr} ;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NETMGR_EXT_LOG_I("IsIfaceActive agvc = [%{public}zu]", argc);
    EthernetAsyncContext* context = std::make_unique<EthernetAsyncContext>().release();
    // Parse Js argv
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[ARGV_INDEX_0],
        context->iface, ETHERNET_NAME_MAX_BYTE, &(context->ethernetNameRealBytes)));
    NETMGR_EXT_LOG_I("IsIfaceActive [%{public}s]", context->iface);
    napi_value result = nullptr;
    if (argc == ARGV_NUM_1) {
        if (context->callbackRef == nullptr) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result));
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_2) {
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_1], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_EXT_LOG_E("IsIfaceActive  exception");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "IsIfaceActive", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecIsIfaceActive,
        CompleteIsIfaceActive,
        (void *)context,
        &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiEthernet::GetAllActiveIfaces(napi_env env, napi_callback_info info)
{
    size_t argc = ARGV_NUM_1;
    napi_value argv[] = {nullptr} ;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NETMGR_EXT_LOG_I("GetAllActiveIfaces agvc = [%{public}zu]", argc);
    EthernetAsyncContext* context = std::make_unique<EthernetAsyncContext>().release();
    napi_value result = nullptr;
    if (argc == ARGV_NUM_0) {
        if (context->callbackRef == nullptr) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result));
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_1) {
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_0], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_EXT_LOG_E("GetAllActiveIfaces  exception.");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "GetAllActiveIfaces", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecGetAllActiveIfaces,
        CompleteGetAllActiveIfaces,
        (void *)context,
        &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiEthernet::DeclareEthernetData(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("STATIC",
            NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(IPSetMode::STATIC))),
        DECLARE_NAPI_STATIC_PROPERTY("DHCP",
            NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(IPSetMode::DHCP))),
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
