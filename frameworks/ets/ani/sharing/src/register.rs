// Copyright (C) 2025 Huawei Device Co., Ltd.
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
    mem,
    ops::Deref,
    sync::{Mutex, OnceLock},
};

use ani_rs::{
    business_error::BusinessError,
    objects::{AniFnObject, GlobalRefCallback},
    AniEnv,
};

use crate::{
    bridge,
    wrapper::{convert_to_business_error, ffi, SharingClient},
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
        let mut inner = self.inner.lock().unwrap();
        SharingClient::register_sharing_observer()?;

        inner.retain(|c| {
            if std::mem::discriminant(&callback) == std::mem::discriminant(c) {
                false
            } else {
                true
            }
        });
        inner.push(callback);
        Ok(())
    }

    pub fn unregister(
        &self,
        callback_ref: Option<CallbackFlavor>,
        event: SharingEventType,
    ) -> Result<(), i32> {
        let mut inner = self.inner.lock().unwrap();
        if let Some(callback) = callback_ref {
            inner.retain(|c| *c != callback)
        } else {
            inner.retain(|c| match event {
                SharingEventType::EventSharingStateChange => {
                    if let CallbackFlavor::SharingStateChange(_) = c {
                        false
                    } else {
                        true
                    }
                }
                SharingEventType::EventInterfaceSharingStateChange => {
                    if let CallbackFlavor::InterfaceSharingStateChange(_) = c {
                        false
                    } else {
                        true
                    }
                }
                SharingEventType::EventSharingUpstreamChange => {
                    if let CallbackFlavor::SharingUpstreamChange(_) = c {
                        false
                    } else {
                        true
                    }
                }
            })
        }

        if (inner.is_empty()) {
            SharingClient::unregister_sharing_observer()?;
        }

        Ok(())
    }

    pub fn on_sharing_state_change(&self, is_running: bool) {
        let inner = self.inner.lock().unwrap();
        for listen in inner.deref() {
            if let CallbackFlavor::SharingStateChange(callback) = listen {
                callback.execute((is_running,));
            }
        }
    }

    pub fn on_interface_sharing_state_change(&self, info: ffi::InterfaceSharingStateInfo) {
        let inner = self.inner.lock().unwrap();
        let param = bridge::InterfaceSharingStateInfo::from(info);
        for listen in inner.deref() {
            if let CallbackFlavor::InterfaceSharingStateChange(callback) = listen {
                callback.execute((param.clone(),));
            }
        }
    }

    pub fn on_sharing_upstream_change(&self, net_handle: ffi::NetHandle) {
        let inner = self.inner.lock().unwrap();
        let param = bridge::NetHandle::from(net_handle);
        for listen in inner.deref() {
            if let CallbackFlavor::SharingUpstreamChange(callback) = listen {
                callback.execute((param.clone(),));
            }
        }
    }
}

pub fn execute_sharing_state_changed(is_running: bool) {
    Register::get_instance().on_sharing_state_change(is_running);
}

pub fn execute_interface_sharing_state_change(info: ffi::InterfaceSharingStateInfo) {
    Register::get_instance().on_interface_sharing_state_change(info);
}

pub fn execute_sharing_upstream_change(net_handle: ffi::NetHandle) {
    Register::get_instance().on_sharing_upstream_change(net_handle);
}

#[ani_rs::native]
pub fn on_sharing_state_change(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let callback = callback.into_global_callback(env).unwrap();
    let flavor = CallbackFlavor::SharingStateChange(callback);
    Register::get_instance()
        .register(flavor)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn on_interface_sharing_state_change(
    env: &AniEnv,
    callback: AniFnObject,
) -> Result<(), BusinessError> {
    let callback = callback.into_global_callback(env).unwrap();
    let flavor = CallbackFlavor::InterfaceSharingStateChange(callback);
    Register::get_instance()
        .register(flavor)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn on_sharing_upstream_change(
    env: &AniEnv,
    callback: AniFnObject,
) -> Result<(), BusinessError> {
    let callback = callback.into_global_callback(env).unwrap();
    let flavor = CallbackFlavor::SharingUpstreamChange(callback);
    Register::get_instance()
        .register(flavor)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn off_sharing_state_change(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let callback_flavor = if env.is_undefined(&callback).unwrap() {
        None
    } else {
        let callback_global = callback.into_global_callback(env).unwrap();
        Some(CallbackFlavor::SharingStateChange(callback_global))
    };

    Register::get_instance()
        .unregister(callback_flavor, SharingEventType::EventSharingStateChange)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn off_interface_sharing_state_change(
    env: &AniEnv,
    callback: AniFnObject,
) -> Result<(), BusinessError> {
    let callback_flavor = if env.is_undefined(&callback).unwrap() {
        None
    } else {
        let callback_global = callback.into_global_callback(env).unwrap();
        Some(CallbackFlavor::InterfaceSharingStateChange(callback_global))
    };

    Register::get_instance()
        .unregister(
            callback_flavor,
            SharingEventType::EventInterfaceSharingStateChange,
        )
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn off_sharing_upstream_change(
    env: &AniEnv,
    callback: AniFnObject,
) -> Result<(), BusinessError> {
    let callback_flavor = if env.is_undefined(&callback).unwrap() {
        None
    } else {
        let callback_global = callback.into_global_callback(env).unwrap();
        Some(CallbackFlavor::SharingUpstreamChange(callback_global))
    };

    Register::get_instance()
        .unregister(
            callback_flavor,
            SharingEventType::EventSharingUpstreamChange,
        )
        .map_err(convert_to_business_error)
}

#[derive(PartialEq, Eq)]
pub enum CallbackFlavor {
    SharingStateChange(GlobalRefCallback<(bool,)>),
    InterfaceSharingStateChange(GlobalRefCallback<(bridge::InterfaceSharingStateInfo,)>),
    SharingUpstreamChange(GlobalRefCallback<(bridge::NetHandle,)>),
}

pub enum SharingEventType {
    EventSharingStateChange,
    EventInterfaceSharingStateChange,
    EventSharingUpstreamChange,
}
