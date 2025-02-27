/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <napi/native_api.h>
#include <napi/native_common.h>

#include "add_netfirewall_rule_context.h"
#include "delete_netfirewall_rule_context.h"
#include "get_all_intercept_records_context.h"
#include "get_all_netfirewall_rules_context.h"
#include "get_netfirewall_rule_context.h"
#include "get_netfirewall_policy_context.h"
#include "module_template.h"
#include "napi_utils.h"
#include "net_firewall_async_work.h"
#include "set_netfirewall_policy_context.h"
#include "update_netfirewall_rule_context.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
static constexpr const char *FUNCTION_SET_NET_FIREWALL_POLICY = "setNetFirewallPolicy";
static constexpr const char *FUNCTION_GET_NET_FIREWALL_POLICY = "getNetFirewallPolicy";
static constexpr const char *FUNCTION_ADD_NET_FIREWALL_RULE = "addNetFirewallRule";
static constexpr const char *FUNCTION_UPDATE_NET_FIREWALL_RULE = "updateNetFirewallRule";
static constexpr const char *FUNCTION_DELETE_NET_FIREWALL_RULE = "removeNetFirewallRule";
static constexpr const char *FUNCTION_GET_NET_FIREWALL_RULES = "getNetFirewallRules";
static constexpr const char *FUNCTION_GET_NET_FIREWALL_RULE = "getNetFirewallRule";
static constexpr const char *FUNCTION_GET_INTERCEPT_RECORDS = "getInterceptRecords";
static constexpr const char *ENUM_NETFIREWALLRULEDIRECTION = "NetFirewallRuleDirection";
static constexpr const char *NETFIREWALLRULEDIRECTION_RULE_IN = "RULE_IN";
static constexpr const char *NETFIREWALLRULEDIRECTION_RULE_OUT = "RULE_OUT";
static constexpr const char *ENUM_FIREWALLRULEACTION = "FirewallRuleAction";
static constexpr const char *FIREWALLRULEACTION_RULE_ALLOW = "RULE_ALLOW";
static constexpr const char *FIREWALLRULEACTION_RULE_DENY = "RULE_DENY";
static constexpr const char *ENUM_NETFIREWALLRULETYPE = "NetFirewallRuleType";
static constexpr const char *NETFIREWALLRULETYPE_RULE_IP = "RULE_IP";
static constexpr const char *NETFIREWALLRULETYPE_RULE_DOMAIN = "RULE_DOMAIN";
static constexpr const char *NETFIREWALLRULETYPE_RULE_DNS = "RULE_DNS";
static constexpr const char *ENUM_NETFIREWALLORDERFIELD = "NetFirewallOrderField";
static constexpr const char *NETFIREWALLORDERFIELD_ORDER_BY_RULE_NAME = "ORDER_BY_RULE_NAME";
static constexpr const char *NETFIREWALLORDERFIELD_ORDER_BY_RECORD_TIME = "ORDER_BY_RECORD_TIME";
static constexpr const char *ENUM_NETFIREWALLORDERTYPE = "NetFirewallOrderType";
static constexpr const char *NETFIREWALLORDERTYPE_ORDER_ASC = "ORDER_ASC";
static constexpr const char *NETFIREWALLORDERTYPE_ORDER_DESC = "ORDER_DESC";

napi_value SetNetFirewallPolicy(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<SetNetFirewallPolicyContext>(env, info, FUNCTION_SET_NET_FIREWALL_POLICY, nullptr,
        NetFirewallAsyncWork::ExecSetNetFirewallPolicy, NetFirewallAsyncWork::SetNetFirewallPolicyCallback);
}

napi_value GetNetFirewallPolicy(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetNetFirewallPolicyContext>(env, info, FUNCTION_GET_NET_FIREWALL_POLICY, nullptr,
        NetFirewallAsyncWork::ExecGetNetFirewallPolicy, NetFirewallAsyncWork::GetNetFirewallPolicyCallback);
}

napi_value AddNetFirewallRule(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<AddNetFirewallRuleContext>(env, info, FUNCTION_ADD_NET_FIREWALL_RULE, nullptr,
        NetFirewallAsyncWork::ExecAddNetFirewallRule, NetFirewallAsyncWork::AddNetFirewallRuleCallback);
}

napi_value UpdateNetFirewallRule(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<UpdateNetFirewallRuleContext>(env, info, FUNCTION_UPDATE_NET_FIREWALL_RULE,
        nullptr, NetFirewallAsyncWork::ExecUpdateNetFirewallRule, NetFirewallAsyncWork::UpdateNetFirewallRuleCallback);
}

napi_value DeleteNetFirewallRule(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<DeleteNetFirewallRuleContext>(env, info, FUNCTION_DELETE_NET_FIREWALL_RULE,
        nullptr, NetFirewallAsyncWork::ExecDeleteNetFirewallRule, NetFirewallAsyncWork::DeleteNetFirewallRuleCallback);
}

napi_value GetNetFirewallRules(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetNetFirewallRulesContext>(env, info, FUNCTION_GET_NET_FIREWALL_RULES,
        nullptr, NetFirewallAsyncWork::ExecGetNetFirewallRules,
        NetFirewallAsyncWork::GetNetFirewallRulesCallback);
}

napi_value GetNetFirewallRule(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetNetFirewallRuleContext>(env, info, FUNCTION_GET_NET_FIREWALL_RULE, nullptr,
        NetFirewallAsyncWork::ExecGetNetFirewallRule, NetFirewallAsyncWork::GetNetFirewallRuleCallback);
}

napi_value GetInterceptRecords(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetInterceptRecordsContext>(env, info, FUNCTION_GET_INTERCEPT_RECORDS,
        nullptr, NetFirewallAsyncWork::ExecGetInterceptRecords,
        NetFirewallAsyncWork::GetInterceptRecordCallbacks);
}
} // namespace

void DeclareNetFirewallInterface(napi_env env, napi_value exports)
{
    NapiUtils::DefineProperties(env, exports,
        {
        DECLARE_NAPI_FUNCTION(FUNCTION_SET_NET_FIREWALL_POLICY, SetNetFirewallPolicy),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_NET_FIREWALL_POLICY, GetNetFirewallPolicy),
        DECLARE_NAPI_FUNCTION(FUNCTION_ADD_NET_FIREWALL_RULE, AddNetFirewallRule),
        DECLARE_NAPI_FUNCTION(FUNCTION_UPDATE_NET_FIREWALL_RULE, UpdateNetFirewallRule),
        DECLARE_NAPI_FUNCTION(FUNCTION_DELETE_NET_FIREWALL_RULE, DeleteNetFirewallRule),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_NET_FIREWALL_RULES, GetNetFirewallRules),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_NET_FIREWALL_RULE, GetNetFirewallRule),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_INTERCEPT_RECORDS, GetInterceptRecords),
        });
    return exports;
}

void InitNetFirewallRuleDirection(napi_env env, napi_value exports)
{
    NapiUtils::DefineProperties(env, exports, {
        DECLARE_NAPI_STATIC_PROPERTY(NETFIREWALLRULEDIRECTION_RULE_IN,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(NetFirewallRuleDirection::RULE_IN))),
        DECLARE_NAPI_STATIC_PROPERTY(NETFIREWALLRULEDIRECTION_RULE_OUT,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(NetFirewallRuleDirection::RULE_OUT))),
    });

    std::initializer_list<napi_property_descriptor> properties = {
        DECLARE_NAPI_STATIC_PROPERTY(NETFIREWALLRULEDIRECTION_RULE_IN,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(NetFirewallRuleDirection::RULE_IN))),
        DECLARE_NAPI_STATIC_PROPERTY(NETFIREWALLRULEDIRECTION_RULE_OUT,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(NetFirewallRuleDirection::RULE_OUT))),
    };

    napi_value netFirewallRuleDirection = NapiUtils::CreateObject(env);
    NapiUtils::DefineProperties(env, netFirewallRuleDirection, properties);
    NapiUtils::SetNamedProperty(env, exports, ENUM_NETFIREWALLRULEDIRECTION, netFirewallRuleDirection);
}

void InitFirewallRuleAction(napi_env env, napi_value exports)
{
    NapiUtils::DefineProperties(env, exports, {
        DECLARE_NAPI_STATIC_PROPERTY(FIREWALLRULEACTION_RULE_ALLOW,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(FirewallRuleAction::RULE_ALLOW))),
        DECLARE_NAPI_STATIC_PROPERTY(FIREWALLRULEACTION_RULE_DENY,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(FirewallRuleAction::RULE_DENY))),
    });

    std::initializer_list<napi_property_descriptor> properties = {
        DECLARE_NAPI_STATIC_PROPERTY(FIREWALLRULEACTION_RULE_ALLOW,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(FirewallRuleAction::RULE_ALLOW))),
        DECLARE_NAPI_STATIC_PROPERTY(FIREWALLRULEACTION_RULE_DENY,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(FirewallRuleAction::RULE_DENY))),
    };

    napi_value firewallRuleAction = NapiUtils::CreateObject(env);
    NapiUtils::DefineProperties(env, firewallRuleAction, properties);
    NapiUtils::SetNamedProperty(env, exports, ENUM_FIREWALLRULEACTION, firewallRuleAction);
}

void InitNetFirewallRuleType(napi_env env, napi_value exports)
{
    NapiUtils::DefineProperties(env, exports, {
        DECLARE_NAPI_STATIC_PROPERTY(NETFIREWALLRULETYPE_RULE_IP,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(NetFirewallRuleType::RULE_IP))),
        DECLARE_NAPI_STATIC_PROPERTY(NETFIREWALLRULETYPE_RULE_DOMAIN,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(NetFirewallRuleType::RULE_DOMAIN))),
        DECLARE_NAPI_STATIC_PROPERTY(NETFIREWALLRULETYPE_RULE_DNS,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(NetFirewallRuleType::RULE_DNS))),
    });

    std::initializer_list<napi_property_descriptor> properties = {
        DECLARE_NAPI_STATIC_PROPERTY(NETFIREWALLRULETYPE_RULE_IP,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(NetFirewallRuleType::RULE_IP))),
        DECLARE_NAPI_STATIC_PROPERTY(NETFIREWALLRULETYPE_RULE_DOMAIN,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(NetFirewallRuleType::RULE_DOMAIN))),
        DECLARE_NAPI_STATIC_PROPERTY(NETFIREWALLRULETYPE_RULE_DNS,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(NetFirewallRuleType::RULE_DNS))),
    };

    napi_value netFirewallRuleType = NapiUtils::CreateObject(env);
    NapiUtils::DefineProperties(env, netFirewallRuleType, properties);
    NapiUtils::SetNamedProperty(env, exports, ENUM_NETFIREWALLRULETYPE, netFirewallRuleType);
}

void InitNetFirewallOrderField(napi_env env, napi_value exports)
{
    NapiUtils::DefineProperties(env, exports, {
        DECLARE_NAPI_STATIC_PROPERTY(NETFIREWALLORDERFIELD_ORDER_BY_RULE_NAME,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(NetFirewallOrderField::ORDER_BY_RULE_NAME))),
        DECLARE_NAPI_STATIC_PROPERTY(NETFIREWALLORDERFIELD_ORDER_BY_RECORD_TIME,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(NetFirewallOrderField::ORDER_BY_RECORD_TIME))),
    });

    std::initializer_list<napi_property_descriptor> properties = {
        DECLARE_NAPI_STATIC_PROPERTY(NETFIREWALLORDERFIELD_ORDER_BY_RULE_NAME,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(NetFirewallOrderField::ORDER_BY_RULE_NAME))),
        DECLARE_NAPI_STATIC_PROPERTY(NETFIREWALLORDERFIELD_ORDER_BY_RECORD_TIME,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(NetFirewallOrderField::ORDER_BY_RECORD_TIME))),
    };

    napi_value netFirewallOrderField = NapiUtils::CreateObject(env);
    NapiUtils::DefineProperties(env, netFirewallOrderField, properties);
    NapiUtils::SetNamedProperty(env, exports, ENUM_NETFIREWALLORDERFIELD, netFirewallOrderField);
}

void InitNetFirewallOrderType(napi_env env, napi_value exports)
{
    NapiUtils::DefineProperties(env, exports, {
        DECLARE_NAPI_STATIC_PROPERTY(NETFIREWALLORDERTYPE_ORDER_ASC,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(NetFirewallOrderType::ORDER_ASC))),
        DECLARE_NAPI_STATIC_PROPERTY(NETFIREWALLORDERTYPE_ORDER_DESC,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(NetFirewallOrderType::ORDER_DESC))),
    });
    
    std::initializer_list<napi_property_descriptor> properties = {
        DECLARE_NAPI_STATIC_PROPERTY(NETFIREWALLORDERTYPE_ORDER_ASC,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(NetFirewallOrderType::ORDER_ASC))),
        DECLARE_NAPI_STATIC_PROPERTY(NETFIREWALLORDERTYPE_ORDER_DESC,
            NapiUtils::CreateUint32(env, static_cast<uint32_t>(NetFirewallOrderType::ORDER_DESC))),
    };

    napi_value netFirewallOrderType = NapiUtils::CreateObject(env);
    NapiUtils::DefineProperties(env, netFirewallOrderType, properties);
    NapiUtils::SetNamedProperty(env, exports, ENUM_NETFIREWALLORDERTYPE, netFirewallOrderType);
}

void DeclareNetFirewallProperties(napi_env env, napi_value exports)
{
    InitNetFirewallRuleDirection(env, exports);
    InitFirewallRuleAction(env, exports);
    InitNetFirewallRuleType(env, exports);
    InitNetFirewallOrderField(env, exports);
    InitNetFirewallOrderType(env, exports);
}

napi_value DeclareNetFirewallModule(napi_env env, napi_value exports)
{
    DeclareNetFirewallInterface(env, exports);
    DeclareNetFirewallProperties(env, exports);
    return exports;
}

static napi_module g_netfirewallModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = DeclareNetFirewallModule,
    .nm_modname = "net.netfirewall",
    .nm_priv = (0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterNetFirewallModule(void)
{
    napi_module_register(&g_netfirewallModule);
}
} // namespace NetManagerStandard
} // namespace OHOS
