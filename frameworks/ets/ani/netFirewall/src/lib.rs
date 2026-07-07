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

mod bridge;
mod log;
mod netFirewall;
mod wrapper;

pub(crate) use log::LOG_LABEL;

ani_rs::ani_constructor! {
    namespace "@ohos.net.netFirewall.netFirewall"
    [
        "setNetFirewallPolicySync": netFirewall::set_net_firewall_policy,
        "getNetFirewallPolicySync": netFirewall::get_net_firewall_policy,
        "addNetFirewallRuleSync": netFirewall::add_net_firewall_rule,
        "updateNetFirewallRuleSync": netFirewall::update_net_firewall_rule,
        "removeNetFirewallRuleSync": netFirewall::remove_net_firewall_rule,
        "getNetFirewallRulesSync": netFirewall::get_net_firewall_rules,
        "getNetFirewallRuleSync": netFirewall::get_net_firewall_rule,
        "getInterceptedRecordsSync": netFirewall::get_intercepted_records,
    ]
}
