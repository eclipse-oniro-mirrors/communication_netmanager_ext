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

// ==================== Enums ====================

/// NetFirewallRuleDirection
#[ani_rs::ani(path = "@ohos.net.netFirewall.netFirewall.NetFirewallRuleDirection")]
#[derive(Clone, Copy)]
pub enum NetFirewallRuleDirection {
    RuleIn = 1,
    RuleOut = 2,
}

/// FirewallRuleAction
#[ani_rs::ani(path = "@ohos.net.netFirewall.netFirewall.FirewallRuleAction")]
#[derive(Clone, Copy)]
pub enum FirewallRuleAction {
    RuleAllow = 0,
    RuleDeny = 1,
}

/// NetFirewallRuleType
#[ani_rs::ani(path = "@ohos.net.netFirewall.netFirewall.NetFirewallRuleType")]
#[derive(Clone, Copy)]
pub enum NetFirewallRuleType {
    RuleIp = 1,
    RuleDomain = 2,
    RuleDns = 3,
}

/// NetFirewallOrderField
#[ani_rs::ani(path = "@ohos.net.netFirewall.netFirewall.NetFirewallOrderField")]
#[derive(Clone, Copy)]
pub enum NetFirewallOrderField {
    OrderByRuleName = 1,
    OrderByRecordTime = 100,
}

/// NetFirewallOrderType
#[ani_rs::ani(path = "@ohos.net.netFirewall.netFirewall.NetFirewallOrderType")]
#[derive(Clone, Copy)]
pub enum NetFirewallOrderType {
    OrderAsc = 1,
    OrderDesc = 100,
}

// ==================== Param Interfaces ====================

#[ani_rs::ani(path = "@ohos.net.netFirewall.netFirewall.RequestParam")]
#[derive(Clone)]
pub struct RequestParam {
    pub page: i32,
    pub page_size: i32,
    pub order_field: NetFirewallOrderField,
    pub order_type: NetFirewallOrderType,
}

#[ani_rs::ani(path = "@ohos.net.netFirewall.netFirewall.NetFirewallDnsParams")]
#[derive(Clone)]
pub struct NetFirewallDnsParams {
    pub primary_dns: String,
    pub standby_dns: Option<String>,
}

#[ani_rs::ani(path = "@ohos.net.netFirewall.netFirewall.NetFirewallIpParams")]
#[derive(Clone)]
pub struct NetFirewallIpParams {
    pub type_: i32,
    pub family: Option<i32>,
    pub address: Option<String>,
    pub mask: Option<i32>,
    pub start_ip: Option<String>,
    pub end_ip: Option<String>,
}

#[ani_rs::ani(path = "@ohos.net.netFirewall.netFirewall.NetFirewallDomainParams")]
#[derive(Clone)]
pub struct NetFirewallDomainParams {
    pub is_wildcard: bool,
    pub domain: String,
}

#[ani_rs::ani(path = "@ohos.net.netFirewall.netFirewall.NetFirewallPortParams")]
#[derive(Clone)]
pub struct NetFirewallPortParams {
    pub start_port: i32,
    pub end_port: i32,
}

// ==================== Main Data Interfaces ====================

#[ani_rs::ani(path = "@ohos.net.netFirewall.netFirewall.NetFirewallRule")]
#[derive(Clone)]
pub struct NetFirewallRule {
    pub user_id: i32,
    pub name: String,
    pub direction: NetFirewallRuleDirection,
    pub action: FirewallRuleAction,
    pub type_: NetFirewallRuleType,
    pub is_enabled: bool,
    pub id: Option<i32>,
    pub description: Option<String>,
    pub app_uid: Option<i32>,
    pub local_ips: Option<Vec<NetFirewallIpParams>>,
    pub remote_ips: Option<Vec<NetFirewallIpParams>>,
    pub protocol: Option<i32>,
    pub local_ports: Option<Vec<NetFirewallPortParams>>,
    pub remote_ports: Option<Vec<NetFirewallPortParams>>,
    pub domains: Option<Vec<NetFirewallDomainParams>>,
    pub dns: Option<NetFirewallDnsParams>,
}

#[ani_rs::ani(path = "@ohos.net.netFirewall.netFirewall.NetFirewallPolicy")]
#[derive(Clone)]
pub struct NetFirewallPolicy {
    pub is_open: bool,
    pub in_action: FirewallRuleAction,
    pub out_action: FirewallRuleAction,
}

#[ani_rs::ani(path = "@ohos.net.netFirewall.netFirewall.InterceptedRecord")]
#[derive(Clone)]
pub struct InterceptedRecord {
    pub time: i64,
    pub local_ip: Option<String>,
    pub remote_ip: Option<String>,
    pub local_port: Option<i32>,
    pub remote_port: Option<i32>,
    pub protocol: Option<i32>,
    pub app_uid: Option<i32>,
    pub domain: Option<String>,
}

#[ani_rs::ani(path = "@ohos.net.netFirewall.netFirewall.FirewallRulePage")]
#[derive(Clone)]
pub struct FirewallRulePage {
    pub page: i32,
    pub page_size: i32,
    pub total_page: i32,
    pub data: Vec<NetFirewallRule>,
}

#[ani_rs::ani(path = "@ohos.net.netFirewall.netFirewall.InterceptedRecordPage")]
#[derive(Clone)]
pub struct InterceptedRecordPage {
    pub page: i32,
    pub page_size: i32,
    pub total_page: i32,
    pub data: Vec<InterceptedRecord>,
}
