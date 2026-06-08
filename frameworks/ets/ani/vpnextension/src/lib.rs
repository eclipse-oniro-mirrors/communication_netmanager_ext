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
        "createSync": vpnext::create,
        "protectSync": vpnext::protect,
        "destroySync": vpnext::destroy,
        "destroyVpnSync": vpnext::destroy_by_vpn_id,
        "protectProcessNetSync": vpnext::protect_process_net,
        "generateVpnIdSync": vpnext::generate_vpn_id,
        "registOnAuthResult": register::regist_on_auth_result,
        "unregistOnAuthResult": register::unregist_on_auth_result,
    ]
}

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
