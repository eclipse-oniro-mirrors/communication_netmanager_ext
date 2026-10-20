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
        "onConnectSync": register::on_connect,
        "onConnectMultiSync": register::on_connect_multi,
        "offConnectSync": register::off_connect,
        "offConnectMultiSync": register::off_connect_multi,
        "addSysVpnConfigSync": vpn::add_sys_vpn_config,
        "deleteSysVpnConfigSync": vpn::delete_sys_vpn_config,
        "getSysVpnConfigListSync": vpn::get_sys_vpn_config_list,
        "getSysVpnConfigSync": vpn::get_sys_vpn_config,
        "getConnectedSysVpnConfigSync": vpn::get_connected_sys_vpn_config,
        "getConnectedVpnAppInfoSync": vpn::get_connected_vpn_app_info,
    ]
    class "@ohos.net.vpn.vpn.VpnConnectionInner"
    [
        "setUpSync": vpn::set_up,
        "protectSync": vpn::protect,
        "destroySync": vpn::destroy,
    ]
    class "@ohos.net.vpn.vpn.Cleaner"
    [
        "clean": vpn::destroy_vpn_connection,
    ]
}

const LOG_LABEL: hilog_rust::HiLogLabel = hilog_rust::HiLogLabel {
    log_type: hilog_rust::LogType::LogCore,
    domain: 0xD0015B0,
    tag: "NETMANAGER_EXT",
};

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
