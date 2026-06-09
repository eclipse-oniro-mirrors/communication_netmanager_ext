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
mod eap;
pub mod log;
mod register;
pub mod wrapper;

use std::ffi::{c_char, CString};
use std::panic;
use std::sync::OnceLock;

static PANIC_HOOK_INSTALLED: OnceLock<()> = OnceLock::new();

#[no_mangle]
extern "C" fn EapAniRegister() {
    PANIC_HOOK_INSTALLED.get_or_init(|| {
        panic::set_hook(Box::new(|info| {
            let location = info.location().map(|l| format!("{}:{}:{}", l.file(), l.line(), l.column()))
                .unwrap_or_else(|| "unknown".to_string());
            let message = info.payload().downcast_ref::<&str>().map(|s| s.to_string())
                .or_else(|| info.payload().downcast_ref::<String>().cloned())
                .unwrap_or_else(|| "unknown".to_string());
            eap_error!("Panic occurred at {}: {}", location, message);
        }));
    });
}

ani_rs::ani_constructor! {
    namespace "@ohos.net.eap.eap"
    [
        "regCustomEapHandlerSync": register::reg_custom_eap_handler,
        "unregCustomEapHandlerSync": register::unreg_custom_eap_handler,
        "replyCustomEapDataSync": eap::reply_custom_eap_data,
        "startEthEapSync": eap::start_eth_eap,
        "logOffEthEapSync": eap::log_off_eth_eap,
    ]
}
