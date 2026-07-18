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

pub struct NetFirewallClient;

impl NetFirewallClient {
    pub fn set_net_firewall_policy(user_id: i32, policy: bridge::NetFirewallPolicy) -> Result<(), i32> {
        let ffi_policy = ffi::NetFirewallPolicyAni::from(policy);
        let ret = ffi::SetNetFirewallPolicy(user_id, &ffi_policy);
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn get_net_firewall_policy(user_id: i32) -> Result<bridge::NetFirewallPolicy, i32> {
        let mut ffi_policy = ffi::NetFirewallPolicyAni::new();
        let ret = ffi::GetNetFirewallPolicy(user_id, &mut ffi_policy);
        if ret != 0 {
            return Err(ret);
        }
        let result = bridge::NetFirewallPolicy::from(ffi_policy);
        Ok(result)
    }

    pub fn add_net_firewall_rule(rule: bridge::NetFirewallRule) -> Result<i32, i32> {
        let ffi_rule = ffi::NetFirewallRuleAni::from(rule);
        let mut result = 0i32;
        let ret = ffi::AddNetFirewallRule(&ffi_rule, &mut result);
        if ret != 0 {
            return Err(ret);
        }
        Ok(result)
    }

    pub fn update_net_firewall_rule(rule: bridge::NetFirewallRule) -> Result<(), i32> {
        let ffi_rule = ffi::NetFirewallRuleAni::from(rule);
        let ret = ffi::UpdateNetFirewallRule(&ffi_rule);
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn remove_net_firewall_rule(user_id: i32, rule_id: i32) -> Result<(), i32> {
        let ret = ffi::RemoveNetFirewallRule(user_id, rule_id);
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn get_net_firewall_rules(
        user_id: i32,
        request_param: bridge::RequestParam,
    ) -> Result<bridge::FirewallRulePage, i32> {
        let ffi_param = ffi::RequestParamAni::from(request_param);
        let mut ffi_page = ffi::FirewallRulePageAni::new();
        let ret = ffi::GetNetFirewallRules(user_id, &ffi_param, &mut ffi_page);
        if ret != 0 {
            return Err(ret);
        }
        let result = bridge::FirewallRulePage::from(ffi_page);
        Ok(result)
    }

    pub fn get_net_firewall_rule(user_id: i32, rule_id: i32) -> Result<bridge::NetFirewallRule, i32> {
        let mut ffi_rule = ffi::NetFirewallRuleAni::new();
        let ret = ffi::GetNetFirewallRule(user_id, rule_id, &mut ffi_rule);
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
        let ffi_param = ffi::RequestParamAni::from(request_param);
        let mut ffi_page = ffi::InterceptedRecordPageAni::new();
        let ret = ffi::GetInterceptedRecords(user_id, &ffi_param, &mut ffi_page);
        if ret != 0 {
            return Err(ret);
        }
        let result = bridge::InterceptedRecordPage::from(ffi_page);
        Ok(result)
    }
}

impl ffi::NetFirewallPolicyAni {
    fn new() -> Self {
        ffi::NetFirewallPolicyAni {
            is_open: false,
            in_action: 0,
            out_action: 0,
        }
    }
}

impl ffi::FirewallRulePageAni {
    fn new() -> Self {
        ffi::FirewallRulePageAni {
            page: 0,
            page_size: 0,
            total_page: 0,
            data: Vec::new(),
        }
    }
}

impl ffi::NetFirewallRuleAni {
    fn new() -> Self {
        ffi::NetFirewallRuleAni {
            user_id: 0,
            name: String::new(),
            direction: 0,
            action: 0,
            type_: 0,
            is_enabled: false,
            id: 0,
            description: String::new(),
            app_uid: 0,
            local_ips: Vec::new(),
            remote_ips: Vec::new(),
            protocol: 0,
            local_ports: Vec::new(),
            remote_ports: Vec::new(),
            domains: Vec::new(),
            dns: ffi::NetFirewallDnsParamAni {
                primary_dns: String::new(),
                standby_dns: String::new(),
            },
            iface: String::new(),
        }
    }
}

impl ffi::InterceptedRecordPageAni {
    fn new() -> Self {
        ffi::InterceptedRecordPageAni {
            page: 0,
            page_size: 0,
            total_page: 0,
            data: Vec::new(),
        }
    }
}

impl From<bridge::NetFirewallPolicy> for ffi::NetFirewallPolicyAni {
    fn from(policy: bridge::NetFirewallPolicy) -> Self {
        ffi::NetFirewallPolicyAni {
            is_open: policy.is_open,
            in_action: policy.in_action as i32,
            out_action: policy.out_action as i32,
        }
    }
}

impl From<bridge::RequestParam> for ffi::RequestParamAni {
    fn from(param: bridge::RequestParam) -> Self {
        ffi::RequestParamAni {
            page: param.page,
            page_size: param.page_size,
            order_field: param.order_field as i32,
            order_type: param.order_type as i32,
        }
    }
}

impl From<ffi::NetFirewallPolicyAni> for bridge::NetFirewallPolicy {
    fn from(ffi_policy: ffi::NetFirewallPolicyAni) -> Self {
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

impl From<ffi::FirewallRulePageAni> for bridge::FirewallRulePage {
    fn from(ffi_page: ffi::FirewallRulePageAni) -> Self {
        bridge::FirewallRulePage {
            page: ffi_page.page,
            page_size: ffi_page.page_size,
            total_page: ffi_page.total_page,
            data: ffi_page.data.into_iter().map(bridge::NetFirewallRule::from).collect(),
        }
    }
}

impl From<ffi::InterceptedRecordPageAni> for bridge::InterceptedRecordPage {
    fn from(ffi_page: ffi::InterceptedRecordPageAni) -> Self {
        bridge::InterceptedRecordPage {
            page: ffi_page.page,
            page_size: ffi_page.page_size,
            total_page: ffi_page.total_page,
            data: ffi_page.data.into_iter().map(bridge::InterceptedRecord::from).collect(),
        }
    }
}

impl From<bridge::NetFirewallRule> for ffi::NetFirewallRuleAni {
    fn from(rule: bridge::NetFirewallRule) -> Self {
        let dns = rule.dns.map_or(
            ffi::NetFirewallDnsParamAni {
                primary_dns: String::new(),
                standby_dns: String::new(),
            },
            |d| ffi::NetFirewallDnsParamAni {
                primary_dns: d.primary_dns,
                standby_dns: d.standby_dns.unwrap_or_default(),
            },
        );

        ffi::NetFirewallRuleAni {
            user_id: rule.user_id,
            name: rule.name,
            direction: rule.direction as i32,
            action: rule.action as i32,
            type_: rule.type_ as i32,
            is_enabled: rule.is_enabled,
            id: rule.id.unwrap_or(0),
            description: rule.description.unwrap_or_default(),
            app_uid: rule.app_uid.unwrap_or(0),
            local_ips: rule
                .local_ips
                .unwrap_or_default()
                .into_iter()
                .map(ffi::NetFirewallIpParamsAni::from)
                .collect(),
            remote_ips: rule
                .remote_ips
                .unwrap_or_default()
                .into_iter()
                .map(ffi::NetFirewallIpParamsAni::from)
                .collect(),
            protocol: rule.protocol.unwrap_or(0),
            local_ports: rule
                .local_ports
                .unwrap_or_default()
                .into_iter()
                .map(ffi::NetFirewallPortParamAni::from)
                .collect(),
            remote_ports: rule
                .remote_ports
                .unwrap_or_default()
                .into_iter()
                .map(ffi::NetFirewallPortParamAni::from)
                .collect(),
            domains: rule
                .domains
                .unwrap_or_default()
                .into_iter()
                .map(ffi::NetFirewallDomainParamAni::from)
                .collect(),
            dns,
            iface: rule.iface.unwrap_or_default(),
        }
    }
}

impl From<ffi::NetFirewallRuleAni> for bridge::NetFirewallRule {
    fn from(rule: ffi::NetFirewallRuleAni) -> Self {
        bridge::NetFirewallRule {
            user_id: rule.user_id,
            name: rule.name,
            direction: match rule.direction {
                1 => bridge::NetFirewallRuleDirection::RuleIn,
                2 => bridge::NetFirewallRuleDirection::RuleOut,
                _ => bridge::NetFirewallRuleDirection::RuleOut,
            },
            action: match rule.action {
                0 => bridge::FirewallRuleAction::RuleAllow,
                _ => bridge::FirewallRuleAction::RuleDeny,
            },
            type_: match rule.type_ {
                1 => bridge::NetFirewallRuleType::RuleIp,
                2 => bridge::NetFirewallRuleType::RuleDomain,
                3 => bridge::NetFirewallRuleType::RuleDns,
                _ => bridge::NetFirewallRuleType::RuleDns,
            },
            is_enabled: rule.is_enabled,
            id: Some(rule.id),
            description: Some(rule.description),
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
            protocol: Some(rule.protocol),
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
            dns: if !rule.dns.primary_dns.is_empty() || !rule.dns.standby_dns.is_empty() {
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
            iface: if rule.iface.is_empty() { None } else { Some(rule.iface) },
        }
    }
}

impl From<ffi::InterceptedRecordAni> for bridge::InterceptedRecord {
    fn from(record: ffi::InterceptedRecordAni) -> Self {
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

impl From<bridge::NetFirewallIpParams> for ffi::NetFirewallIpParamsAni {
    fn from(ip: bridge::NetFirewallIpParams) -> Self {
        ffi::NetFirewallIpParamsAni {
            ip_type: ip.type_,
            family: ip.family.unwrap_or(0),
            address: ip.address.unwrap_or_default(),
            mask: ip.mask.unwrap_or(0),
            start_ip: ip.start_ip.unwrap_or_default(),
            end_ip: ip.end_ip.unwrap_or_default(),
        }
    }
}

impl From<ffi::NetFirewallIpParamsAni> for bridge::NetFirewallIpParams {
    fn from(ip: ffi::NetFirewallIpParamsAni) -> Self {
        bridge::NetFirewallIpParams {
            type_: ip.ip_type,
            family: Some(ip.family),
            address: Some(ip.address.clone()),
            mask: Some(ip.mask),
            start_ip: Some(ip.start_ip),
            end_ip: Some(ip.end_ip),
        }
    }
}

impl From<bridge::NetFirewallPortParams> for ffi::NetFirewallPortParamAni {
    fn from(port: bridge::NetFirewallPortParams) -> Self {
        ffi::NetFirewallPortParamAni {
            start_port: port.start_port,
            end_port: port.end_port,
        }
    }
}

impl From<ffi::NetFirewallPortParamAni> for bridge::NetFirewallPortParams {
    fn from(port: ffi::NetFirewallPortParamAni) -> Self {
        bridge::NetFirewallPortParams {
            start_port: port.start_port,
            end_port: port.end_port,
        }
    }
}

impl From<bridge::NetFirewallDomainParams> for ffi::NetFirewallDomainParamAni {
    fn from(domain: bridge::NetFirewallDomainParams) -> Self {
        ffi::NetFirewallDomainParamAni {
            is_wildcard: domain.is_wildcard,
            domain: domain.domain,
        }
    }
}

impl From<ffi::NetFirewallDomainParamAni> for bridge::NetFirewallDomainParams {
    fn from(domain: ffi::NetFirewallDomainParamAni) -> Self {
        bridge::NetFirewallDomainParams {
            is_wildcard: domain.is_wildcard,
            domain: domain.domain,
        }
    }
}

#[cxx::bridge(namespace = "OHOS::NetManagerAni")]
pub(crate) mod ffi {
    struct NetFirewallIpParamsAni {
        ip_type: i32,
        family: i32,
        address: String,
        mask: i32,
        start_ip: String,
        end_ip: String,
    }

    struct NetFirewallPortParamAni {
        start_port: i32,
        end_port: i32,
    }

    struct NetFirewallDomainParamAni {
        is_wildcard: bool,
        domain: String,
    }

    struct NetFirewallDnsParamAni {
        primary_dns: String,
        standby_dns: String,
    }

    struct NetFirewallRuleAni {
        user_id: i32,
        name: String,
        direction: i32,
        action: i32,
        type_: i32,
        is_enabled: bool,
        id: i32,
        description: String,
        app_uid: i32,
        local_ips: Vec<NetFirewallIpParamsAni>,
        remote_ips: Vec<NetFirewallIpParamsAni>,
        protocol: i32,
        local_ports: Vec<NetFirewallPortParamAni>,
        remote_ports: Vec<NetFirewallPortParamAni>,
        domains: Vec<NetFirewallDomainParamAni>,
        dns: NetFirewallDnsParamAni,
        iface: String,
    }

    struct NetFirewallPolicyAni {
        is_open: bool,
        in_action: i32,
        out_action: i32,
    }

    struct RequestParamAni {
        page: i32,
        page_size: i32,
        order_field: i32,
        order_type: i32,
    }

    struct FirewallRulePageAni {
        page: i32,
        page_size: i32,
        total_page: i32,
        data: Vec<NetFirewallRuleAni>,
    }

    struct InterceptedRecordAni {
        time: u64,
        local_ip: String,
        remote_ip: String,
        local_port: i32,
        remote_port: i32,
        protocol: i32,
        app_uid: i32,
        domain: String,
    }

    struct InterceptedRecordPageAni {
        page: i32,
        page_size: i32,
        total_page: i32,
        data: Vec<InterceptedRecordAni>,
    }

    unsafe extern "C++" {
        include!("net_firewall_ani.h");

        fn SetNetFirewallPolicy(user_id: i32, policy: &NetFirewallPolicyAni) -> i32;
        fn GetNetFirewallPolicy(user_id: i32, policy: &mut NetFirewallPolicyAni) -> i32;
        fn AddNetFirewallRule(rule: &NetFirewallRuleAni, result: &mut i32) -> i32;
        fn UpdateNetFirewallRule(rule: &NetFirewallRuleAni) -> i32;
        fn RemoveNetFirewallRule(user_id: i32, rule_id: i32) -> i32;
        fn GetNetFirewallRules(user_id: i32, request_param: &RequestParamAni,
            page_info: &mut FirewallRulePageAni) -> i32;
        fn GetNetFirewallRule(user_id: i32, rule_id: i32, rule: &mut NetFirewallRuleAni) -> i32;
        fn GetInterceptedRecords(user_id: i32, request_param: &RequestParamAni,
            page_info: &mut InterceptedRecordPageAni) -> i32;
    }
}
