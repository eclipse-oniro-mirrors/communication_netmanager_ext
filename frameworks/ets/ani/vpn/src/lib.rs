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
mod vpn;
mod wrapper;

ani_rs::ani_constructor! {
    namespace "@ohos.net.vpn.vpn"
    [
        "createVpnConnectionSync": vpn::create_vpn_connection,
        "setUpSync": vpn::set_up,
        "protectSync": vpn::protect,
        "destroySync": vpn::destroy,
        "addSysVpnConfigSync": vpn::add_sys_vpn_config,
        "deleteSysVpnConfigSync": vpn::delete_sys_vpn_config,
        "getSysVpnConfigListSync": vpn::get_sys_vpn_config_list,
        "getSysVpnConfigSync": vpn::get_sys_vpn_config,
        "getConnectedSysVpnConfigSync": vpn::get_connected_sys_vpn_config,
        "getConnectedVpnAppInfoSync": vpn::get_connected_vpn_app_info,
        "onConnect": register::on_connect,
        "onConnectMulti": register::on_connect_multi,
        "offConnect": register::off_connect,
        "offConnectMulti": register::off_connect_multi,
    ]
}

#[used]
#[link_section = ".init_array"]
static G_VPN_PANIC_HOOK: extern "C" fn() = {
    #[link_section = ".text.startup"]
    extern "C" fn init() {
        std::panic::set_hook(Box::new(|info| {
            if let Some(location) = info.location() {
                vpn_error!("Panic occurred at {}:{}", location.file(), location.line());
            } else {
                vpn_error!("Panic occurred: unknown location");
            }
        }));
    }
    init
};
