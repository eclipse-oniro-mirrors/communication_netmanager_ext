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

// HiLog-based logging macros for mdns ANI module.
// All arguments are wrapped with @private() to instruct the HiLog system
// to mask parameter values in production builds, preventing accidental
// leakage of sensitive data (e.g., service names, addresses) in logs.

#[macro_export]
macro_rules! mdns_debug {
    ($fmt: literal $(, $args:expr)* $(,)?) => {{
        use hilog_rust::hilog;
        use std::ffi::{c_char, CString};
        use crate::LOG_LABEL;
        hilog_rust::debug!(LOG_LABEL, $fmt $(, @private($args))*);
    }}
}

#[macro_export]
macro_rules! mdns_info {
    ($fmt: literal $(, $args:expr)* $(,)?) => {{
        use hilog_rust::hilog;
        use std::ffi::{c_char, CString};
        use crate::LOG_LABEL;

        hilog_rust::info!(LOG_LABEL, $fmt $(, @private($args))*);
    }}
}

#[macro_export]
macro_rules! mdns_error {
    ($fmt: literal $(, $args:expr)* $(,)?) => {{
        use hilog_rust::hilog;
        use std::ffi::{c_char, CString};
        use crate::LOG_LABEL;

        hilog_rust::error!(LOG_LABEL, $fmt $(, @private($args))*);
    }}
}
