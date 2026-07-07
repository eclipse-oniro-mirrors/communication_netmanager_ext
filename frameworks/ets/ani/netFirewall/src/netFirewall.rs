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

use ani_rs::business_error::BusinessError;

use crate::bridge;
use crate::wrapper::NetFirewallClient;

fn convert_to_business_error(code: i32) -> BusinessError {
    BusinessError::new(code, format!("operation failed, error code: {}", code))
}

#[ani_rs::native]
pub fn set_net_firewall_policy(
    user_id: i32,
    policy: bridge::NetFirewallPolicy,
) -> Result<(), BusinessError> {
    NetFirewallClient::set_net_firewall_policy(user_id, policy)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_net_firewall_policy(
    user_id: i32,
) -> Result<bridge::NetFirewallPolicy, BusinessError> {
    NetFirewallClient::get_net_firewall_policy(user_id)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn add_net_firewall_rule(
    rule: bridge::NetFirewallRule,
) -> Result<i32, BusinessError> {
    NetFirewallClient::add_net_firewall_rule(rule)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn update_net_firewall_rule(
    rule: bridge::NetFirewallRule,
) -> Result<(), BusinessError> {
    NetFirewallClient::update_net_firewall_rule(rule)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn remove_net_firewall_rule(
    user_id: i32,
    rule_id: i32,
) -> Result<(), BusinessError> {
    NetFirewallClient::remove_net_firewall_rule(user_id, rule_id)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_net_firewall_rules(
    user_id: i32,
    request_param: bridge::RequestParam,
) -> Result<bridge::FirewallRulePage, BusinessError> {
    NetFirewallClient::get_net_firewall_rules(user_id, request_param)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_net_firewall_rule(
    user_id: i32,
    rule_id: i32,
) -> Result<bridge::NetFirewallRule, BusinessError> {
    NetFirewallClient::get_net_firewall_rule(user_id, rule_id)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_intercepted_records(
    user_id: i32,
    request_param: bridge::RequestParam,
) -> Result<bridge::InterceptedRecordPage, BusinessError> {
    NetFirewallClient::get_intercepted_records(user_id, request_param)
        .map_err(convert_to_business_error)
}
