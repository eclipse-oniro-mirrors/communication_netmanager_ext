/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef NET_FIREWALL_ANI_H
#define NET_FIREWALL_ANI_H

#include <cstdint>
#include <memory>

#include "cxx.h"
#include "netfirewall_parcel.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {

// ==================== FFI Structs (must match cxx::bridge in wrapper.rs) ====================
// Note: Fields like user_id, app_uid, page, page_size use int32_t to match the ETS 'int' type.
// Negative-value guards are enforced in FFI functions as runtime validation.

struct NetFirewallIpParamFFI {
    uint8_t family;
    uint8_t ip_type;
    uint8_t mask;
    rust::String start_ip;
    rust::String end_ip;
};

struct NetFirewallPortParamFFI {
    uint16_t start_port;
    uint16_t end_port;
};

struct NetFirewallDomainParamFFI {
    bool is_wildcard;
    rust::String domain;
};

struct NetFirewallDnsParamFFI {
    bool has_dns;
    rust::String primary_dns;
    rust::String standby_dns;
};

// NetFirewallRuleFFI — input-only struct (add/update rules).
// Fields currently mirror NetFirewallRuleResultFFI but the two are kept as separate
// types because they represent distinct API directions and may diverge in the future.
struct NetFirewallRuleFFI {
    int32_t rule_id;
    rust::String rule_name;
    rust::String rule_description;
    int32_t rule_direction;
    int32_t rule_action;
    int32_t rule_type;
    bool is_enabled;
    int32_t app_uid;
    uint8_t protocol;
    int32_t user_id;
    rust::Vec<NetFirewallIpParamFFI> local_ips;
    rust::Vec<NetFirewallIpParamFFI> remote_ips;
    rust::Vec<NetFirewallPortParamFFI> local_ports;
    rust::Vec<NetFirewallPortParamFFI> remote_ports;
    rust::Vec<NetFirewallDomainParamFFI> domains;
    NetFirewallDnsParamFFI dns;
};

struct NetFirewallPolicyFFI {
    bool is_open;
    int32_t in_action;
    int32_t out_action;
};

struct RequestParamFFI {
    int32_t page;
    int32_t page_size;
    int32_t order_field;
    int32_t order_type;
};

// NetFirewallRuleResultFFI — output-only struct (get/list rules).
// Fields currently mirror NetFirewallRuleFFI but the two are kept as separate
// types because they represent distinct API directions and may diverge in the future.
struct NetFirewallRuleResultFFI {
    int32_t rule_id;
    rust::String rule_name;
    rust::String rule_description;
    int32_t rule_direction;
    int32_t rule_action;
    int32_t rule_type;
    bool is_enabled;
    int32_t app_uid;
    uint8_t protocol;
    int32_t user_id;
    rust::Vec<NetFirewallIpParamFFI> local_ips;
    rust::Vec<NetFirewallIpParamFFI> remote_ips;
    rust::Vec<NetFirewallPortParamFFI> local_ports;
    rust::Vec<NetFirewallPortParamFFI> remote_ports;
    rust::Vec<NetFirewallDomainParamFFI> domains;
    NetFirewallDnsParamFFI dns;
};

struct FirewallRulePageFFI {
    int32_t page;
    int32_t page_size;
    int32_t total_page;
    rust::Vec<NetFirewallRuleResultFFI> data;
};

struct InterceptedRecordFFI {
    int64_t time;
    rust::String local_ip;
    rust::String remote_ip;
    int32_t local_port;
    int32_t remote_port;
    int32_t protocol;
    int32_t app_uid;
    rust::String domain;
};

struct InterceptRecordPageFFI {
    int32_t page;
    int32_t page_size;
    int32_t total_page;
    rust::Vec<InterceptedRecordFFI> data;
};

// ==================== FFI Functions ====================
// Safety: These functions are called EXCLUSIVELY from Rust via cxx::bridge. The cxx code generator
// maps Rust &T / &mut T to C++ const T& / T& respectively, and Rust's borrow checker guarantees
// all references point to valid, initialized memory. Therefore null-pointer checks on reference
// parameters are unnecessary — a null reference is impossible in this architecture.

int32_t FFISetNetFirewallPolicy(int32_t user_id, const NetFirewallPolicyFFI &policy);
int32_t FFIGetNetFirewallPolicy(int32_t user_id, NetFirewallPolicyFFI &policy);
int32_t FFIAddNetFirewallRule(const NetFirewallRuleFFI &rule, int32_t &result);
int32_t FFIUpdateNetFirewallRule(const NetFirewallRuleFFI &rule);
int32_t FFIRemoveNetFirewallRule(int32_t user_id, int32_t rule_id);
int32_t FFIGetNetFirewallRules(int32_t user_id, const RequestParamFFI &request_param, FirewallRulePageFFI &page_info);
int32_t FFIGetNetFirewallRule(int32_t user_id, int32_t rule_id, NetFirewallRuleResultFFI &rule);
int32_t FFIGetInterceptedRecords(int32_t user_id,
    const RequestParamFFI &request_param, InterceptRecordPageFFI &page_info);

} // namespace NetManagerStandard
} // namespace OHOS

#endif // NET_FIREWALL_ANI_H
