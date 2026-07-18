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
mod register;
mod vpnext;
mod wrapper;

ani_rs::ani_constructor! {
    namespace "@ohos.net.vpnExtension.vpnExtension"
    [
        "startVpnExtensionAbilitySync": vpnext::start_vpn_extension_ability,
        "stopVpnExtensionAbilitySync": vpnext::stop_vpn_extension_ability,
        "setAlwaysOnVpnEnabledSync": vpnext::set_always_on_vpn_enabled,
        "isAlwaysOnVpnEnabledSync": vpnext::is_always_on_vpn_enabled,
        "updateVpnAuthorizedStateSync": vpnext::update_vpn_authorized_state,
        "createVpnConnectionSync": vpnext::create_vpn_connection,
        "createVpnObserverSync": vpnext::create_vpn_observer,
    ]
    class "@ohos.net.vpnExtension.vpnExtension.VpnObserverInner"
    [
        "onAuthorizationResultSync": register::on_authorization_result,
        "offAuthorizationResultSync": register::off_authorization_result,
    ]
    class "@ohos.net.vpnExtension.vpnExtension.VpnConnectionInner"
    [
        "createSync": vpnext::create,
        "generateVpnIdSync": vpnext::generate_vpn_id,
        "protectSync": vpnext::protect,
        "destroySync": vpnext::destroy,
        "protectProcessNetSync": vpnext::protect_process_net,
    ]
    class "@ohos.net.vpnExtension.vpnExtension.VpnObserverCleaner"
    [
        "native_clean" : vpnext::destroy_vpn_observer,
    ]
    class "@ohos.net.vpnExtension.vpnExtension.VpnConnectionCleaner"
    [
        "native_clean" : vpnext::destroy_vpn_connection,
    ]
}

const LOG_LABEL: hilog_rust::HiLogLabel = hilog_rust::HiLogLabel {
    log_type: hilog_rust::LogType::LogCore,
    domain: 0xD0015B0,
    tag: "NETMANAGER_EXT",
};

#[used]
#[link_section = ".init_array"]
static G_VPNEXT_PANIC_HOOK: extern "C" fn() = {
    #[link_section = ".text.startup"]
    extern "C" fn init() {
        std::panic::set_hook(Box::new(|info| {
            if let Some(location) = info.location() {
                vpnext_error!("Panic occurred at {}:{}", location.file(), location.line());
            } else {
                vpnext_error!("Panic occurred: unknown location");
            }
        }));
    }
    init
};
