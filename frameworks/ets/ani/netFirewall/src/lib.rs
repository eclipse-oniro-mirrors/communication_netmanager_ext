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
mod net_firewall;
mod wrapper;

ani_rs::ani_constructor! {
    namespace "@ohos.net.netFirewall.netFirewall"
    [
        "setNetFirewallPolicySync": net_firewall::set_net_firewall_policy,
        "getNetFirewallPolicySync": net_firewall::get_net_firewall_policy,
        "addNetFirewallRuleSync": net_firewall::add_net_firewall_rule,
        "updateNetFirewallRuleSync": net_firewall::update_net_firewall_rule,
        "removeNetFirewallRuleSync": net_firewall::remove_net_firewall_rule,
        "getNetFirewallRulesSync": net_firewall::get_net_firewall_rules,
        "getNetFirewallRuleSync": net_firewall::get_net_firewall_rule,
        "getInterceptedRecordsSync": net_firewall::get_intercepted_records,
    ]
}

const LOG_LABEL: hilog_rust::HiLogLabel = hilog_rust::HiLogLabel {
    log_type: hilog_rust::LogType::LogCore,
    domain: 0xD0015B0,
    tag: "NETMANAGER_EXT",
};

#[used]
#[link_section = ".init_array"]
static G_NETFIREWALL_PANIC_HOOK: extern "C" fn() = {
    #[link_section = ".text.startup"]
    extern "C" fn init() {
        std::panic::set_hook(Box::new(|info| {
            netfw_error!("Panic occurred: {:?}", info);
        }));
    }
    init
};
