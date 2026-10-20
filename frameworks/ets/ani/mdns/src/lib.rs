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
mod mdns;
mod register;
mod wrapper;
mod context;

ani_rs::ani_constructor! {
    namespace "@ohos.net.mdns.mdns"
    [
        "addLocalServiceSync": mdns::add_local_service,
        "removeLocalServiceSync": mdns::remove_local_service,
        "createDiscoveryServiceSync": mdns::create_discovery_service,
        "resolveLocalServiceSync": mdns::resolve_local_service,

    ]
    class "@ohos.net.mdns.mdns.DiscoveryServiceInner"
    [
        "onDiscoveryStartSync": register::on_discovery_start,
        "offDiscoveryStartSync": register::off_discovery_start,
        "onDiscoveryStopSync": register::on_discovery_stop,
        "offDiscoveryStopSync": register::off_discovery_stop,
        "onServiceFoundSync": register::on_service_found,
        "offServiceFoundSync": register::off_service_found,
        "onServiceLostSync": register::on_service_lost,
        "offServiceLostSync": register::off_service_lost,
        "startSearchingMDNSSync": mdns::start_searching_mdns,
        "stopSearchingMDNSSync": mdns::stop_searching_mdns,
    ]
    class "@ohos.net.mdns.mdns.Cleaner"
    [
        "clean" : mdns::destroy_discovery_service,
    ]
}

const LOG_LABEL: hilog_rust::HiLogLabel = hilog_rust::HiLogLabel {
    log_type: hilog_rust::LogType::LogCore,
    domain: 0xD0015B0,
    tag: "NETMANAGER_EXT",
};

#[used]
#[link_section = ".init_array"]
static G_MDNS_PANIC_HOOK: extern "C" fn() = {
    #[link_section = ".text.startup"]
    extern "C" fn init() {
        std::panic::set_hook(Box::new(|info| {
            mdns_error!("Panic occurred: {:?}", info);
        }));
    }
    init
};