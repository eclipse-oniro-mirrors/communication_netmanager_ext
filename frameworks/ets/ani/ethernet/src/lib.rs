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

#[macro_use]
pub mod log;
mod bridge;
mod ethernet;
mod register;
pub mod wrapper;

ani_rs::ani_constructor! {
    namespace "@ohos.net.ethernet.ethernet"
    [
        "isEthernetEnabledNative": ethernet::is_ethernet_enabled,
        "disableEthernetInterfaceNative": ethernet::disable_ethernet_interface,
        "enableEthernetInterfaceNative": ethernet::enable_ethernet_interface,
        "getAllActiveIfacesNative": ethernet::get_all_active_ifaces,
        "isIfaceActiveNative": ethernet::is_iface_active,
        "getIfaceConfigNative": ethernet::get_iface_config,
        "setIfaceConfigNative": ethernet::set_iface_config,
        "getEthernetDeviceInfosNative": ethernet::get_ethernet_device_infos,
        "getMacAddressNative": ethernet::get_mac_address,
        "onInterfaceStateChangeNative": register::on_interface_state_change,
        "offInterfaceStateChangeNative": register::off_interface_state_change,
    ]
}

const LOG_LABEL: hilog_rust::HiLogLabel = hilog_rust::HiLogLabel {
    log_type: hilog_rust::LogType::LogCore,
    domain: 0xD0015B0,
    tag: "NETMANAGER_EXT",
};

#[used]
#[link_section = ".init_array"]
static G_ETHERNET_PANIC_HOOK: extern "C" fn() = {
    #[link_section = ".text.startup"]
    extern "C" fn init() {
        std::panic::set_hook(Box::new(|info| {
            ethernet_error!("Panic occurred: {:?}", info);
        }));
    }
    init
};