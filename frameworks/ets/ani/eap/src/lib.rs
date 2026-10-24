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

pub mod log;
mod bridge;
mod eap;
mod register;
pub mod wrapper;

ani_rs::ani_constructor! {
    namespace "@ohos.net.eap.eap"
    [
        "regCustomEapHandlerSync": register::reg_custom_eap_handler,
        "unregCustomEapHandlerSync": register::unreg_custom_eap_handler,
        "replyCustomEapDataSync": eap::reply_custom_eap_data,
        "startEthEapSync": eap::start_eth_eap,
        "logOffEthEapSync": eap::log_off_eth_eap,
        "setEthEapConfigSync": eap::set_eth_eap_config,
        "getEthEapConfigSync": eap::get_eth_eap_config,
        "deleteEthEapConfigSync": eap::delete_eth_eap_config,
        "getEthEapStateInfoSync": eap::get_eth_eap_state_info,
        "onStateChangeSync": register::on_state_change,
        "offStateChangeSync": register::off_state_change,
        "connectSync": eap::connect_eth_eap,
    ]
}

const LOG_LABEL: hilog_rust::HiLogLabel = hilog_rust::HiLogLabel {
    log_type: hilog_rust::LogType::LogCore,
    domain: 0xD0015B0,
    tag: "NETMANAGER_EXT",
};

#[used]
#[link_section = ".init_array"]
static G_EAP_PANIC_HOOK: extern "C" fn() = {
    #[link_section = ".text.startup"]
    extern "C" fn init() {
        std::panic::set_hook(Box::new(|info| {
            eap_error!("Panic occurred: {:?}", info);
        }));
    }
    init
};