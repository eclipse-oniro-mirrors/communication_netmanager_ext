// Copyright (C) 2026 Huawei Device Co., Ltd.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

use crate::bridge;

// ==================== Validation Constants ====================
// Error codes must match net_manager_constants.h
const FIREWALL_ERR_PARAMETER_ERROR: i32 = 401;
const FIREWALL_ERR_INVALID_PARAMETER: i32 = 2100001;

const MAX_RULE_NAME_LEN: usize = 128;
const MAX_RULE_DESCRIPTION_LEN: usize = 256;
const MAX_RULE_PORT: i32 = 65535;
const MAX_EXACT_DOMAIN_NAME_LEN: usize = 253;
const MAX_FUZZY_DOMAIN_NAME_LEN: usize = 63;

// IP constants (matching netfirewall_parcel.h)
const FAMILY_IPV4: i32 = 1;
const FAMILY_IPV6: i32 = 2;
const SINGLE_IP: i32 = 1;
const MULTIPLE_IP: i32 = 2;
const IPV4_MASK_MAX: i32 = 32;
const IPV6_MASK_MAX: i32 = 128;

// Protocol constants (matching net_manager_constants.h NetworkProtocol)
const PROTO_ICMP: i32 = 1;
const PROTO_TCP: i32 = 6;
const PROTO_UDP: i32 = 17;
const PROTO_ICMPV6: i32 = 58;
const PROTO_SAT_EXPAK: i32 = 64;

/// Validate protocol value is one of the allowed set.
fn validate_protocol(protocol: i32) -> bool {
    matches!(protocol, PROTO_ICMP | PROTO_TCP | PROTO_UDP | PROTO_ICMPV6 | PROTO_SAT_EXPAK)
}

/// Validate userId: must be >= 0 (matching NAPI CheckRuleNumberParam for NET_FIREWALL_USER_ID).
fn validate_user_id(user_id: i32) -> Result<(), i32> {
    if user_id < 0 {
        return Err(FIREWALL_ERR_INVALID_PARAMETER);
    }
    Ok(())
}

/// Validate ruleId: 0 <= ruleId <= INT32_MAX (matching NAPI CheckUpdateFirewallRule).
fn validate_rule_id(rule_id: i32) -> Result<(), i32> {
    if rule_id < 0 {
        return Err(FIREWALL_ERR_INVALID_PARAMETER);
    }
    Ok(())
}

/// Validate IP address format: must be a valid IPv4 dotted-decimal or IPv6 colon-hex string.
fn is_valid_ip_address(ip: &str) -> bool {
    if ip.is_empty() {
        return false;
    }
    let is_ipv4 = ip
        .chars()
        .all(|c| c.is_ascii_digit() || c == '.');
    if is_ipv4 {
        let parts: Vec<&str> = ip.split('.').collect();
        if parts.len() != 4 {
            return false;
        }
        return parts.iter().all(|p| {
            if p.is_empty() || p.len() > 3 {
                return false;
            }
            // Must not have leading zeros (except for "0" itself)
            if p.len() > 1 && p.starts_with('0') {
                return false;
            }
            match p.parse::<u32>() {
                Ok(n) => n <= 255,
                Err(_) => false,
            }
        });
    }
    // IPv6: allow hex digits, colons; handle compressed form (::)
    let is_ipv6 = ip
        .chars()
        .all(|c| c.is_ascii_hexdigit() || c == ':');
    if is_ipv6 {
        if ip.contains("::") {
            // Compressed form: :: can appear at most once, representing 1-8 zero groups.
            // After removing empty parts, 0-7 non-empty groups are valid
            // (0 for "::" itself, 1 for "::1", up to 7 for "2001:db8:1:2:3:4:5::").
            if ip.matches("::").count() > 1 {
                return false;
            }
            let parts: Vec<&str> = ip.split(':').filter(|s| !s.is_empty()).collect();
            if parts.len() > 7 {
                return false;
            }
            return parts.iter().all(|p| p.len() <= 4);
        } else {
            // Uncompressed form: exactly 8 groups required
            let parts: Vec<&str> = ip.split(':').collect();
            if parts.len() != 8 {
                return false;
            }
            return parts.iter().all(|p| !p.is_empty() && p.len() <= 4);
        }
    }
    false
}

/// Validate domain against strict pattern (matching NAPI DOMAIN_PATTERN / WILDCARD_DOMAIN_PATTERN).
/// Returns true if the domain passes validation.
fn is_valid_domain(domain: &str, is_wildcard: bool) -> bool {
    if domain.is_empty() {
        return false;
    }
    let max_len = if is_wildcard { MAX_FUZZY_DOMAIN_NAME_LEN } else { MAX_EXACT_DOMAIN_NAME_LEN };
    if domain.len() > max_len {
        return false;
    }
    // Must not contain null bytes
    if domain.contains('\0') {
        return false;
    }
    // Split into labels
    let labels: Vec<&str> = domain.split('.').collect();
    if labels.len() < 2 {
        return false; // must have at least "label.tld"
    }
    // Check for empty labels or trailing dot
    if labels.iter().any(|l| l.is_empty()) {
        return false;
    }
    for (i, label) in labels.iter().enumerate() {
        // Trailing dot allowed on the last label (empty after final dot) is already filtered above
        if label.len() > 63 {
            return false;
        }
        // Wildcard: first label can be "*"
        if is_wildcard && i == 0 && *label == "*" {
            continue;
        }
        // Each label must be: [a-zA-Z0-9][-a-zA-Z0-9]{0,62}
        let mut chars = label.chars();
        let first = match chars.next() {
            Some(c) => c,
            None => return false,
        };
        if !first.is_ascii_alphanumeric() {
            return false;
        }
        if !chars.all(|c| c.is_ascii_alphanumeric() || c == '-') {
            return false;
        }
        // Cannot end with hyphen (check last char)
        if label.ends_with('-') {
            return false;
        }
    }
    true
}

/// Validate DNS IP address: must be a valid IP (matching NAPI CheckFirewallDns → CheckIpAddress).
fn validate_dns_ip(ip: &str) -> bool {
    is_valid_ip_address(ip)
}

/// Validate DNS params (matching NAPI CheckFirewallDns).
/// primaryDns is required and must be a valid IP; standbyDns if provided must also be valid.
fn validate_dns_params(dns: &bridge::NetFirewallDnsParams) -> Result<(), i32> {
    if dns.primary_dns.is_empty() || !validate_dns_ip(&dns.primary_dns) {
        return Err(FIREWALL_ERR_INVALID_PARAMETER);
    }
    if let Some(ref standby) = dns.standby_dns {
        if !standby.is_empty() && !validate_dns_ip(standby) {
            return Err(FIREWALL_ERR_INVALID_PARAMETER);
        }
    }
    Ok(())
}

/// Validate a NetFirewallIpParams entry (matching NAPI CheckIpList → CheckSingeIp / CheckMultipleIp).
fn validate_ip_params(ip: &bridge::NetFirewallIpParams) -> Result<(), i32> {
    let family = ip.family.unwrap_or(FAMILY_IPV4);
    if family != FAMILY_IPV4 && family != FAMILY_IPV6 {
        return Err(FIREWALL_ERR_INVALID_PARAMETER);
    }
    let ip_type = ip.type_;
    if ip_type != SINGLE_IP && ip_type != MULTIPLE_IP {
        return Err(FIREWALL_ERR_INVALID_PARAMETER);
    }
    let mask_max = if family == FAMILY_IPV4 { IPV4_MASK_MAX } else { IPV6_MASK_MAX };
    if ip_type == SINGLE_IP {
        // Address must be present and valid
        let addr = ip.address.as_deref().unwrap_or("");
        if addr.is_empty() || !is_valid_ip_address(addr) {
            return Err(FIREWALL_ERR_INVALID_PARAMETER);
        }
        // Mask validation: mask can be 0 (e.g. 0.0.0.0/0), must be >= 0
        if let Some(mask) = ip.mask {
            if mask < 0 || mask > mask_max {
                return Err(FIREWALL_ERR_INVALID_PARAMETER);
            }
        }
    } else {
        // MULTIPLE_IP: start_ip and end_ip must be valid
        let start = ip.start_ip.as_deref().unwrap_or("");
        let end = ip.end_ip.as_deref().unwrap_or("");
        if start.is_empty() || end.is_empty()
            || !is_valid_ip_address(start)
            || !is_valid_ip_address(end)
        {
            return Err(FIREWALL_ERR_INVALID_PARAMETER);
        }
    }
    Ok(())
}

/// Validate port params (matching NAPI CheckPortList):
/// startPort: 0 <= startPort <= 65535
/// endPort: startPort <= endPort <= 65535
fn validate_port_params(port: &bridge::NetFirewallPortParams) -> Result<(), i32> {
    if port.start_port < 0 || port.start_port > MAX_RULE_PORT {
        return Err(FIREWALL_ERR_INVALID_PARAMETER);
    }
    if port.end_port < 0 || port.end_port > MAX_RULE_PORT || port.end_port < port.start_port {
        return Err(FIREWALL_ERR_INVALID_PARAMETER);
    }
    Ok(())
}

/// Validate domain params (matching NAPI CheckDomainList).
fn validate_domain_params(domain: &bridge::NetFirewallDomainParams) -> Result<(), i32> {
    if domain.domain.is_empty() || domain.domain.contains('\0') {
        return Err(FIREWALL_ERR_INVALID_PARAMETER);
    }
    if !is_valid_domain(&domain.domain, domain.is_wildcard) {
        return Err(FIREWALL_ERR_INVALID_PARAMETER);
    }
    Ok(())
}

/// Validate a NetFirewallRule (matching NAPI CheckFirewallRule).
/// Covers all input fields: name, description, direction, action, type_,
/// user_id, app_uid, protocol, local/remote IPs, local/remote ports, domains,
/// and DNS (primary_dns format, standby_dns format).
fn validate_firewall_rule(rule: &bridge::NetFirewallRule) -> Result<(), i32> {
    // Rule name: required (non-empty), max length 128
    if rule.name.is_empty() || rule.name.len() > MAX_RULE_NAME_LEN {
        return Err(FIREWALL_ERR_INVALID_PARAMETER);
    }
    // Rule description: optional, max length 256 (already handled by String, just check length)
    if let Some(ref desc) = rule.description {
        if desc.len() > MAX_RULE_DESCRIPTION_LEN {
            return Err(FIREWALL_ERR_INVALID_PARAMETER);
        }
    }
    // Direction: must be RuleIn(1) or RuleOut(2)
    let dir_val = rule.direction as i32;
    if dir_val != 1 && dir_val != 2 {
        return Err(FIREWALL_ERR_INVALID_PARAMETER);
    }
    // Action: must be RuleAllow(0) or RuleDeny(1)
    let action_val = rule.action as i32;
    if action_val != 0 && action_val != 1 {
        return Err(FIREWALL_ERR_INVALID_PARAMETER);
    }
    // Rule type: must be RuleIp(1), RuleDomain(2), or RuleDns(3)
    let rule_type = rule.type_ as i32;
    if rule_type < 1 || rule_type > 3 {
        return Err(FIREWALL_ERR_INVALID_PARAMETER);
    }
    // userId: must be >= 0
    if rule.user_id < 0 {
        return Err(FIREWALL_ERR_INVALID_PARAMETER);
    }
    // appUid: must be 0..=INT32_MAX
    if let Some(app_uid) = rule.app_uid {
        if app_uid < 0 {
            return Err(FIREWALL_ERR_INVALID_PARAMETER);
        }
    }
    // Protocol: mandatory for RULE_IP (matching NAPI CheckFirewallRule)
    if rule_type == 1 {
        if rule.protocol.is_none() {
            return Err(FIREWALL_ERR_INVALID_PARAMETER);
        }
        if !validate_protocol(rule.protocol.unwrap()) {
            return Err(FIREWALL_ERR_INVALID_PARAMETER);
        }
    } else {
        // For RULE_DOMAIN / RULE_DNS: protocol is optional, but if present must be valid
        if let Some(protocol) = rule.protocol {
            if !validate_protocol(protocol) {
                return Err(FIREWALL_ERR_INVALID_PARAMETER);
            }
        }
    }
    // Validate IP list entries
    if let Some(ref ips) = rule.local_ips {
        for ip in ips {
            validate_ip_params(ip)?;
        }
    }
    if let Some(ref ips) = rule.remote_ips {
        for ip in ips {
            validate_ip_params(ip)?;
        }
    }
    // Validate port list entries
    if let Some(ref ports) = rule.local_ports {
        for port in ports {
            validate_port_params(port)?;
        }
    }
    if let Some(ref ports) = rule.remote_ports {
        for port in ports {
            validate_port_params(port)?;
        }
    }
    // Validate domain list entries
    if let Some(ref domains) = rule.domains {
        for domain in domains {
            validate_domain_params(domain)?;
        }
    }
    // Validate DNS
    if let Some(ref dns) = rule.dns {
        validate_dns_params(dns)?;
    }
    Ok(())
}

// ==================== Client Implementation ====================

pub struct NetFirewallClient;

impl NetFirewallClient {
    pub fn set_net_firewall_policy(user_id: i32, policy: bridge::NetFirewallPolicy) -> Result<(), i32> {
        validate_user_id(user_id)?;
        let ffi_policy = ffi::NetFirewallPolicyFFI::from(policy);
        let ret = ffi::FFISetNetFirewallPolicy(user_id, &ffi_policy);
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn get_net_firewall_policy(user_id: i32) -> Result<bridge::NetFirewallPolicy, i32> {
        validate_user_id(user_id)?;
        let mut ffi_policy = ffi::NetFirewallPolicyFFI::default_value();
        let ret = ffi::FFIGetNetFirewallPolicy(user_id, &mut ffi_policy);
        if ret != 0 {
            return Err(ret);
        }
        let result = bridge::NetFirewallPolicy::from(ffi_policy);
        Ok(result)
    }

    pub fn add_net_firewall_rule(rule: bridge::NetFirewallRule) -> Result<i32, i32> {
        validate_firewall_rule(&rule)?;
        let ffi_rule = ffi::NetFirewallRuleFFI::from(rule);
        let mut result = 0i32;
        let ret = ffi::FFIAddNetFirewallRule(&ffi_rule, &mut result);
        if ret != 0 {
            return Err(ret);
        }
        Ok(result)
    }

    pub fn update_net_firewall_rule(rule: bridge::NetFirewallRule) -> Result<(), i32> {
        // For update, rule.id is required and must be valid
        let id = rule.id.ok_or(FIREWALL_ERR_INVALID_PARAMETER)?;
        validate_rule_id(id)?;
        validate_firewall_rule(&rule)?;
        let ffi_rule = ffi::NetFirewallRuleFFI::from(rule);
        let ret = ffi::FFIUpdateNetFirewallRule(&ffi_rule);
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn remove_net_firewall_rule(user_id: i32, rule_id: i32) -> Result<(), i32> {
        validate_user_id(user_id)?;
        validate_rule_id(rule_id)?;
        let ret = ffi::FFIRemoveNetFirewallRule(user_id, rule_id);
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn get_net_firewall_rules(
        user_id: i32,
        request_param: bridge::RequestParam,
    ) -> Result<bridge::FirewallRulePage, i32> {
        validate_user_id(user_id)?;
        let ffi_param = ffi::RequestParamFFI::from(request_param);
        let mut ffi_page = ffi::FirewallRulePageFFI::default_value();
        let ret = ffi::FFIGetNetFirewallRules(user_id, &ffi_param, &mut ffi_page);
        if ret != 0 {
            return Err(ret);
        }
        let result = bridge::FirewallRulePage::from(ffi_page);
        Ok(result)
    }

    pub fn get_net_firewall_rule(user_id: i32, rule_id: i32) -> Result<bridge::NetFirewallRule, i32> {
        validate_user_id(user_id)?;
        validate_rule_id(rule_id)?;
        let mut ffi_rule = ffi::NetFirewallRuleResultFFI::default_value();
        let ret = ffi::FFIGetNetFirewallRule(user_id, rule_id, &mut ffi_rule);
        if ret != 0 {
            return Err(ret);
        }
        let result = bridge::NetFirewallRule::from(ffi_rule);
        Ok(result)
    }

    pub fn get_intercepted_records(
        user_id: i32,
        request_param: bridge::RequestParam,
    ) -> Result<bridge::InterceptedRecordPage, i32> {
        validate_user_id(user_id)?;
        let ffi_param = ffi::RequestParamFFI::from(request_param);
        let mut ffi_page = ffi::InterceptRecordPageFFI::default_value();
        let ret = ffi::FFIGetInterceptedRecords(user_id, &ffi_param, &mut ffi_page);
        if ret != 0 {
            return Err(ret);
        }
        let result = bridge::InterceptedRecordPage::from(ffi_page);
        Ok(result)
    }

}

// ---- Default values for FFI structs ----

impl ffi::NetFirewallPolicyFFI {
    fn default_value() -> Self {
        ffi::NetFirewallPolicyFFI {
            is_open: false,
            in_action: 0,
            out_action: 0,
        }
    }
}

impl ffi::FirewallRulePageFFI {
    fn default_value() -> Self {
        ffi::FirewallRulePageFFI {
            page: 0,
            page_size: 0,
            total_page: 0,
            data: Vec::new(),
        }
    }
}

impl ffi::NetFirewallRuleResultFFI {
    fn default_value() -> Self {
        ffi::NetFirewallRuleResultFFI {
            rule_id: 0,
            rule_name: String::new(),
            rule_description: String::new(),
            rule_direction: 0,
            rule_action: 0,
            rule_type: 0,
            is_enabled: false,
            app_uid: 0,
            protocol: 0,
            user_id: 0,
            local_ips: Vec::new(),
            remote_ips: Vec::new(),
            local_ports: Vec::new(),
            remote_ports: Vec::new(),
            domains: Vec::new(),
            dns: ffi::NetFirewallDnsParamFFI {
                has_dns: false,
                primary_dns: String::new(),
                standby_dns: String::new(),
            },
        }
    }
}

impl ffi::InterceptRecordPageFFI {
    fn default_value() -> Self {
        ffi::InterceptRecordPageFFI {
            page: 0,
            page_size: 0,
            total_page: 0,
            data: Vec::new(),
        }
    }
}

// ---- bridge -> ffi conversions ----

impl From<bridge::NetFirewallPolicy> for ffi::NetFirewallPolicyFFI {
    fn from(policy: bridge::NetFirewallPolicy) -> Self {
        ffi::NetFirewallPolicyFFI {
            is_open: policy.is_open,
            in_action: policy.in_action as i32,
            out_action: policy.out_action as i32,
        }
    }
}

impl From<bridge::RequestParam> for ffi::RequestParamFFI {
    fn from(param: bridge::RequestParam) -> Self {
        ffi::RequestParamFFI {
            page: param.page,
            page_size: param.page_size,
            order_field: param.order_field as i32,
            order_type: param.order_type as i32,
        }
    }
}

// ---- ffi -> bridge conversions ----

impl From<ffi::NetFirewallPolicyFFI> for bridge::NetFirewallPolicy {
    fn from(ffi_policy: ffi::NetFirewallPolicyFFI) -> Self {
        bridge::NetFirewallPolicy {
            is_open: ffi_policy.is_open,
            in_action: match ffi_policy.in_action {
                0 => bridge::FirewallRuleAction::RuleAllow,
                _ => bridge::FirewallRuleAction::RuleDeny,
            },
            out_action: match ffi_policy.out_action {
                0 => bridge::FirewallRuleAction::RuleAllow,
                _ => bridge::FirewallRuleAction::RuleDeny,
            },
        }
    }
}

impl From<ffi::FirewallRulePageFFI> for bridge::FirewallRulePage {
    fn from(ffi_page: ffi::FirewallRulePageFFI) -> Self {
        bridge::FirewallRulePage {
            page: ffi_page.page,
            page_size: ffi_page.page_size,
            total_page: ffi_page.total_page,
            data: ffi_page.data.into_iter().map(bridge::NetFirewallRule::from).collect(),
        }
    }
}

impl From<ffi::InterceptRecordPageFFI> for bridge::InterceptedRecordPage {
    fn from(ffi_page: ffi::InterceptRecordPageFFI) -> Self {
        bridge::InterceptedRecordPage {
            page: ffi_page.page,
            page_size: ffi_page.page_size,
            total_page: ffi_page.total_page,
            data: ffi_page.data.into_iter().map(bridge::InterceptedRecord::from).collect(),
        }
    }
}

// ---- bridge NetFirewallRule <-> ffi NetFirewallRuleFFI (input) ----

impl From<bridge::NetFirewallRule> for ffi::NetFirewallRuleFFI {
    fn from(rule: bridge::NetFirewallRule) -> Self {
        // DNS validation is performed upstream in validate_firewall_rule (called before
        // this conversion in both add_net_firewall_rule and update_net_firewall_rule).
        // The From trait cannot return Result, so the conversion layer defensively
        // falls back to empty string for invalid input that passed validation.
        let dns = rule.dns.map_or(
            ffi::NetFirewallDnsParamFFI {
                has_dns: false,
                primary_dns: String::new(),
                standby_dns: String::new(),
            },
            |d| {
                // Validate DNS IP format (matching NAPI CheckFirewallDns -> CheckIpAddress).
                // Already validated upstream in validate_firewall_rule.
                let primary = if d.primary_dns.is_empty()
                    || d.primary_dns.contains('\0')
                    || !validate_dns_ip(&d.primary_dns)
                {
                    String::new()
                } else {
                    d.primary_dns
                };
                let standby = d.standby_dns.map_or(String::new(), |s| {
                    if s.is_empty() || s.contains('\0') || !validate_dns_ip(&s) {
                        String::new()
                    } else {
                        s
                    }
                });
                ffi::NetFirewallDnsParamFFI {
                    has_dns: !primary.is_empty(),
                    primary_dns: primary,
                    standby_dns: standby,
                }
            },
        );

        ffi::NetFirewallRuleFFI {
            // rule.id is optional for add (0 = new rule, C++ assigns real ID) but
            // guaranteed Some for update by update_net_firewall_rule validation.
            rule_id: rule.id.unwrap_or(0),
            rule_name: rule.name,
            rule_description: rule.description.unwrap_or_default(),
            rule_direction: rule.direction as i32,
            rule_action: rule.action as i32,
            rule_type: rule.type_ as i32,
            is_enabled: rule.is_enabled,
            app_uid: rule.app_uid.unwrap_or(0),
            protocol: {
                let p = rule.protocol.unwrap_or(0);
                if p < 0 || p > 255 {
                    0
                } else {
                    p as u8
                }
            },
            user_id: rule.user_id,
            local_ips: rule
                .local_ips
                .unwrap_or_default()
                .into_iter()
                .map(ffi::NetFirewallIpParamFFI::from)
                .collect(),
            remote_ips: rule
                .remote_ips
                .unwrap_or_default()
                .into_iter()
                .map(ffi::NetFirewallIpParamFFI::from)
                .collect(),
            local_ports: rule
                .local_ports
                .unwrap_or_default()
                .into_iter()
                .map(ffi::NetFirewallPortParamFFI::from)
                .collect(),
            remote_ports: rule
                .remote_ports
                .unwrap_or_default()
                .into_iter()
                .map(ffi::NetFirewallPortParamFFI::from)
                .collect(),
            domains: rule
                .domains
                .unwrap_or_default()
                .into_iter()
                .map(ffi::NetFirewallDomainParamFFI::from)
                .collect(),
            dns,
        }
    }
}

// ---- ffi NetFirewallRuleResultFFI (output) -> bridge NetFirewallRule ----

impl From<ffi::NetFirewallRuleResultFFI> for bridge::NetFirewallRule {
    fn from(rule: ffi::NetFirewallRuleResultFFI) -> Self {
        bridge::NetFirewallRule {
            user_id: rule.user_id,
            name: rule.rule_name,
            direction: match rule.rule_direction {
                1 => bridge::NetFirewallRuleDirection::RuleIn,
                2 => bridge::NetFirewallRuleDirection::RuleOut,
                // Unexpected value: default to RuleOut as a safe fallback
                _ => bridge::NetFirewallRuleDirection::RuleOut,
            },
            action: match rule.rule_action {
                0 => bridge::FirewallRuleAction::RuleAllow,
                _ => bridge::FirewallRuleAction::RuleDeny,
            },
            type_: match rule.rule_type {
                1 => bridge::NetFirewallRuleType::RuleIp,
                2 => bridge::NetFirewallRuleType::RuleDomain,
                3 => bridge::NetFirewallRuleType::RuleDns,
                // Unknown rule_type from FFI: default to RuleDns as safest fallback
                _ => bridge::NetFirewallRuleType::RuleDns,
            },
            is_enabled: rule.is_enabled,
            id: Some(rule.rule_id),
            description: Some(rule.rule_description),
            app_uid: Some(rule.app_uid),
            local_ips: Some(
                rule.local_ips
                    .into_iter()
                    .map(bridge::NetFirewallIpParams::from)
                    .collect(),
            ),
            remote_ips: Some(
                rule.remote_ips
                    .into_iter()
                    .map(bridge::NetFirewallIpParams::from)
                    .collect(),
            ),
            protocol: Some(rule.protocol as i32),
            local_ports: Some(
                rule.local_ports
                    .into_iter()
                    .map(bridge::NetFirewallPortParams::from)
                    .collect(),
            ),
            remote_ports: Some(
                rule.remote_ports
                    .into_iter()
                    .map(bridge::NetFirewallPortParams::from)
                    .collect(),
            ),
            domains: Some(
                rule.domains
                    .into_iter()
                    .map(bridge::NetFirewallDomainParams::from)
                    .collect(),
            ),
            dns: if rule.dns.has_dns {
                Some(bridge::NetFirewallDnsParams {
                    primary_dns: rule.dns.primary_dns,
                    standby_dns: if rule.dns.standby_dns.is_empty() {
                        None
                    } else {
                        Some(rule.dns.standby_dns)
                    },
                })
            } else {
                None
            },
        }
    }
}

// ---- InterceptedRecord conversions ----

impl From<ffi::InterceptedRecordFFI> for bridge::InterceptedRecord {
    fn from(record: ffi::InterceptedRecordFFI) -> Self {
        bridge::InterceptedRecord {
            time: record.time,
            local_ip: Some(record.local_ip),
            remote_ip: Some(record.remote_ip),
            local_port: Some(record.local_port),
            remote_port: Some(record.remote_port),
            protocol: Some(record.protocol),
            app_uid: Some(record.app_uid),
            domain: Some(record.domain),
        }
    }
}

// ---- Sub-param conversions (bridge -> ffi) ----

impl From<bridge::NetFirewallIpParams> for ffi::NetFirewallIpParamFFI {
    fn from(ip: bridge::NetFirewallIpParams) -> Self {
        let family = ip.family.unwrap_or(FAMILY_IPV4);
        // Validation in validate_ip_params should prevent out-of-range family, but the
        // From trait cannot return Result. Fall back to a safe default instead of panicking.
        let family_u8 = if (0..=255).contains(&family) {
            family as u8
        } else {
            FAMILY_IPV4 as u8
        };
        // Matching NAPI net_firewall_rule_parse.cpp: if mask==0 for single IP,
        // default to max mask (32 for IPv4, 128 for IPv6).
        let default_mask = if family_u8 == FAMILY_IPV4 as u8 { IPV4_MASK_MAX } else { IPV6_MASK_MAX } as u8;
        let mask = ip.mask.unwrap_or(0) as u8;
        let mask = if mask == 0 { default_mask } else { mask };
        ffi::NetFirewallIpParamFFI {
            family: family_u8,
            ip_type: ip.type_ as u8,
            mask,
            start_ip: ip.start_ip.or_else(|| ip.address.clone()).unwrap_or_default(),
            end_ip: ip.end_ip.unwrap_or_default(),
        }
    }
}

impl From<ffi::NetFirewallIpParamFFI> for bridge::NetFirewallIpParams {
    fn from(ip: ffi::NetFirewallIpParamFFI) -> Self {
        bridge::NetFirewallIpParams {
            type_: ip.ip_type as i32,
            family: Some(ip.family as i32),
            address: Some(ip.start_ip.clone()),
            mask: Some(ip.mask as i32),
            start_ip: Some(ip.start_ip),
            end_ip: Some(ip.end_ip),
        }
    }
}

impl From<bridge::NetFirewallPortParams> for ffi::NetFirewallPortParamFFI {
    fn from(port: bridge::NetFirewallPortParams) -> Self {
        // Port range validation (matching NAPI CheckPortList):
        // startPort: 0 <= startPort <= 65535; endPort: startPort <= endPort <= 65535;
        // Already validated upstream in validate_firewall_rule.
        let start = if port.start_port >= 0 && port.start_port <= 65535 {
            port.start_port as u16
        } else {
            0 // Invalid input recovery: validation already caught this
        };
        let end = if port.end_port >= port.start_port && port.end_port <= 65535 {
            port.end_port as u16
        } else {
            0
        };
        ffi::NetFirewallPortParamFFI {
            start_port: start,
            end_port: end,
        }
    }
}

impl From<ffi::NetFirewallPortParamFFI> for bridge::NetFirewallPortParams {
    fn from(port: ffi::NetFirewallPortParamFFI) -> Self {
        bridge::NetFirewallPortParams {
            start_port: port.start_port as i32,
            end_port: port.end_port as i32,
        }
    }
}

impl From<bridge::NetFirewallDomainParams> for ffi::NetFirewallDomainParamFFI {
    fn from(domain: bridge::NetFirewallDomainParams) -> Self {
        // Matching NAPI CheckDomainList: strict regex + length-by-type + wildcard support.
        // Already validated upstream in validate_firewall_rule.
        let validated_domain = if is_valid_domain(&domain.domain, domain.is_wildcard) {
            domain.domain
        } else {
            String::new()
        };
        ffi::NetFirewallDomainParamFFI {
            is_wildcard: domain.is_wildcard,
            domain: validated_domain,
        }
    }
}

impl From<ffi::NetFirewallDomainParamFFI> for bridge::NetFirewallDomainParams {
    fn from(domain: ffi::NetFirewallDomainParamFFI) -> Self {
        bridge::NetFirewallDomainParams {
            is_wildcard: domain.is_wildcard,
            domain: domain.domain,
        }
    }
}

// ---- cxx::bridge module ----

#[cxx::bridge(namespace = "OHOS::NetManagerStandard")]
pub(crate) mod ffi {
    // Shared struct types (must match netFirewall_ani.h EXACTLY)

    #[namespace = "OHOS::NetManagerStandard"]
    struct NetFirewallIpParamFFI {
        family: u8,
        ip_type: u8,
        mask: u8,
        start_ip: String,
        end_ip: String,
    }

    #[namespace = "OHOS::NetManagerStandard"]
    struct NetFirewallPortParamFFI {
        start_port: u16,
        end_port: u16,
    }

    #[namespace = "OHOS::NetManagerStandard"]
    struct NetFirewallDomainParamFFI {
        is_wildcard: bool,
        domain: String,
    }

    #[namespace = "OHOS::NetManagerStandard"]
    struct NetFirewallDnsParamFFI {
        has_dns: bool,
        primary_dns: String,
        standby_dns: String,
    }

    // NetFirewallRuleFFI — input-only (add/update rule). Field-parity with
    // NetFirewallRuleResultFFI is intentional: separate types for distinct API directions.
    #[namespace = "OHOS::NetManagerStandard"]
    struct NetFirewallRuleFFI {
        rule_id: i32,
        rule_name: String,
        rule_description: String,
        rule_direction: i32,
        rule_action: i32,
        rule_type: i32,
        is_enabled: bool,
        app_uid: i32,
        protocol: u8,
        user_id: i32,
        local_ips: Vec<NetFirewallIpParamFFI>,
        remote_ips: Vec<NetFirewallIpParamFFI>,
        local_ports: Vec<NetFirewallPortParamFFI>,
        remote_ports: Vec<NetFirewallPortParamFFI>,
        domains: Vec<NetFirewallDomainParamFFI>,
        dns: NetFirewallDnsParamFFI,
    }

    #[namespace = "OHOS::NetManagerStandard"]
    struct NetFirewallPolicyFFI {
        is_open: bool,
        in_action: i32,
        out_action: i32,
    }

    #[namespace = "OHOS::NetManagerStandard"]
    struct RequestParamFFI {
        page: i32,
        page_size: i32,
        order_field: i32,
        order_type: i32,
    }

    // NetFirewallRuleResultFFI — output-only (get/list rule). Field-parity with
    // NetFirewallRuleFFI is intentional: separate types for distinct API directions.
    #[namespace = "OHOS::NetManagerStandard"]
    struct NetFirewallRuleResultFFI {
        rule_id: i32,
        rule_name: String,
        rule_description: String,
        rule_direction: i32,
        rule_action: i32,
        rule_type: i32,
        is_enabled: bool,
        app_uid: i32,
        protocol: u8,
        user_id: i32,
        local_ips: Vec<NetFirewallIpParamFFI>,
        remote_ips: Vec<NetFirewallIpParamFFI>,
        local_ports: Vec<NetFirewallPortParamFFI>,
        remote_ports: Vec<NetFirewallPortParamFFI>,
        domains: Vec<NetFirewallDomainParamFFI>,
        dns: NetFirewallDnsParamFFI,
    }

    #[namespace = "OHOS::NetManagerStandard"]
    struct FirewallRulePageFFI {
        page: i32,
        page_size: i32,
        total_page: i32,
        data: Vec<NetFirewallRuleResultFFI>,
    }

    #[namespace = "OHOS::NetManagerStandard"]
    struct InterceptedRecordFFI {
        time: i64,
        local_ip: String,
        remote_ip: String,
        local_port: i32,
        remote_port: i32,
        protocol: i32,
        app_uid: i32,
        domain: String,
    }

    #[namespace = "OHOS::NetManagerStandard"]
    struct InterceptRecordPageFFI {
        page: i32,
        page_size: i32,
        total_page: i32,
        data: Vec<InterceptedRecordFFI>,
    }

    // C++ functions implemented in netFirewall_ani.cpp
    unsafe extern "C++" {
        include!("netFirewall_ani.h");

        #[namespace = "OHOS::NetManagerStandard"]
        type NetFirewallIpParamFFI;

        #[namespace = "OHOS::NetManagerStandard"]
        type NetFirewallPortParamFFI;

        #[namespace = "OHOS::NetManagerStandard"]
        type NetFirewallDomainParamFFI;

        #[namespace = "OHOS::NetManagerStandard"]
        type NetFirewallDnsParamFFI;

        #[namespace = "OHOS::NetManagerStandard"]
        type NetFirewallRuleFFI;

        #[namespace = "OHOS::NetManagerStandard"]
        type NetFirewallPolicyFFI;

        #[namespace = "OHOS::NetManagerStandard"]
        type RequestParamFFI;

        #[namespace = "OHOS::NetManagerStandard"]
        type NetFirewallRuleResultFFI;

        #[namespace = "OHOS::NetManagerStandard"]
        type FirewallRulePageFFI;

        #[namespace = "OHOS::NetManagerStandard"]
        type InterceptedRecordFFI;

        #[namespace = "OHOS::NetManagerStandard"]
        type InterceptRecordPageFFI;

        fn FFISetNetFirewallPolicy(user_id: i32, policy: &NetFirewallPolicyFFI) -> i32;
        fn FFIGetNetFirewallPolicy(user_id: i32, policy: &mut NetFirewallPolicyFFI) -> i32;
        fn FFIAddNetFirewallRule(rule: &NetFirewallRuleFFI, result: &mut i32) -> i32;
        fn FFIUpdateNetFirewallRule(rule: &NetFirewallRuleFFI) -> i32;
        fn FFIRemoveNetFirewallRule(user_id: i32, rule_id: i32) -> i32;
        fn FFIGetNetFirewallRules(user_id: i32, request_param: &RequestParamFFI, page_info: &mut FirewallRulePageFFI) -> i32;
        fn FFIGetNetFirewallRule(user_id: i32, rule_id: i32, rule: &mut NetFirewallRuleResultFFI) -> i32;
        fn FFIGetInterceptedRecords(user_id: i32, request_param: &RequestParamFFI, page_info: &mut InterceptRecordPageFFI) -> i32;
    }
}

