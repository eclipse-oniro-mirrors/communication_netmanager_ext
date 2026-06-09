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

use std::{
    ffi::CString,
    ops::Deref,
    os::raw::c_char,
    sync::{Mutex, OnceLock},
};

use ani_rs::{
    business_error::BusinessError,
    objects::{AniFnObject, GlobalRefCallback},
    AniEnv,
};

use crate::{
    bridge,
    wrapper::{convert_to_business_error, ffi, EthernetClient},
};

struct Register {
    inner: Mutex<Vec<CallbackFlavor>>,
}

impl Register {
    fn new() -> Self {
        Self {
            inner: Mutex::new(Vec::new()),
        }
    }

    pub fn get_instance() -> &'static Self {
        static INSTANCE: OnceLock<Register> = OnceLock::new();
        INSTANCE.get_or_init(Register::new)
    }

    pub fn register(&self, callback: CallbackFlavor) -> Result<(), i32> {
        let mut inner = self.inner.lock().unwrap_or_else(|e| {
            crate::ethernet_error!("Mutex poisoned in ethernet register, recovering data");
            e.into_inner()
        });
        let was_empty = inner.is_empty();

        inner.retain(|c| *c != callback);
        inner.push(callback);

        // Only call on when transitioning from empty to non-empty
        if was_empty {
            EthernetClient::on_interface_state_change()?;
        }

        Ok(())
    }

    pub fn unregister(&self, callback_ref: Option<CallbackFlavor>) -> Result<(), i32> {
        let mut inner = self.inner.lock().unwrap_or_else(|e| {
            crate::ethernet_error!("Mutex poisoned in ethernet unregister, recovering data");
            e.into_inner()
        });
        if let Some(callback) = callback_ref {
            inner.retain(|c| *c != callback)
        } else {
            inner.clear();
        }

        // Only call off when all callbacks are removed
        if inner.is_empty() {
            EthernetClient::off_interface_state_change()?;
        }

        Ok(())
    }

    pub fn on_interface_added(&self, iface: String) {
        let inner = self.inner.lock().unwrap_or_else(|e| {
            crate::ethernet_error!("Mutex poisoned in on_interface_added, recovering data");
            e.into_inner()
        });
        for listen in inner.deref() {
            if let CallbackFlavor::InterfaceStateChange(callback) = listen {
                let info = bridge::InterfaceStateInfo {
                    iface: iface.clone(),
                    active: true,
                };
                callback.execute((info,));
            }
        }
    }

    pub fn on_interface_removed(&self, iface: String) {
        let inner = self.inner.lock().unwrap_or_else(|e| {
            crate::ethernet_error!("Mutex poisoned in on_interface_removed, recovering data");
            e.into_inner()
        });
        for listen in inner.deref() {
            if let CallbackFlavor::InterfaceStateChange(callback) = listen {
                let info = bridge::InterfaceStateInfo {
                    iface: iface.clone(),
                    active: false,
                };
                callback.execute((info,));
            }
        }
    }

    pub fn on_interface_changed(&self, info: ffi::InterfaceStateInfo) {
        let inner = self.inner.lock().unwrap_or_else(|e| {
            crate::ethernet_error!("Mutex poisoned in on_interface_changed, recovering data");
            e.into_inner()
        });
        let param = bridge::InterfaceStateInfo {
            iface: info.iface,
            active: info.active,
        };
        for listen in inner.deref() {
            if let CallbackFlavor::InterfaceStateChange(callback) = listen {
                callback.execute((param.clone(),));
            }
        }
    }
}

pub fn execute_interface_added(iface: String) {
    Register::get_instance().on_interface_added(iface);
}

pub fn execute_interface_removed(iface: String) {
    Register::get_instance().on_interface_removed(iface);
}

pub fn execute_interface_changed(info: ffi::InterfaceStateInfo) {
    Register::get_instance().on_interface_changed(info);
}

#[ani_rs::native]
pub fn on_interface_state_change(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let callback = callback.into_global_callback(env).unwrap();
    let flavor = CallbackFlavor::InterfaceStateChange(callback);
    Register::get_instance()
        .register(flavor)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn off_interface_state_change(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let callback_flavor = if env.is_undefined(&callback).unwrap() {
        None
    } else {
        let callback_global = callback.into_global_callback(env).unwrap();
        Some(CallbackFlavor::InterfaceStateChange(callback_global))
    };

    Register::get_instance()
        .unregister(callback_flavor)
        .map_err(convert_to_business_error)
}

#[derive(PartialEq, Eq)]
pub enum CallbackFlavor {
    InterfaceStateChange(GlobalRefCallback<(bridge::InterfaceStateInfo,)>),
}
