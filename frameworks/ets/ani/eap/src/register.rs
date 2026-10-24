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
    wrapper::{
        convert_to_business_error,
        ffi::{EapAniData, EthEapStateInfoAni},
        NetConnClient,
    },
};

/// Composite entry that pairs registration parameters (net_type, eap_code,
/// eap_type) with the callback, ensuring comparison and removal are based on
/// the full registration info rather than just the callback reference.
struct EapCallbackEntry {
    net_type: i32,
    eap_code: i32,
    eap_type: i32,
    callback: CallbackFlavor,
}

impl PartialEq for EapCallbackEntry {
    fn eq(&self, other: &Self) -> bool {
        self.net_type == other.net_type
            && self.eap_code == other.eap_code
            && self.eap_type == other.eap_type
            && self.callback == other.callback
    }
}

impl Eq for EapCallbackEntry {}

struct Register {
    inner: Mutex<Vec<EapCallbackEntry>>,
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

    /// Register a callback for the given (net_type, eap_code, eap_type).
    /// Design: each (net_type, eap_code, eap_type) combination supports only one callback;
    /// re-registering with the same parameters replaces the previous callback.
    pub fn register(&self, net_type: i32, eap_code: i32, eap_type: i32, callback: CallbackFlavor) -> Result<(), i32> {
        // Recover from poison: panicking would crash the service, and re-initializing
        // would lose all registered callbacks. Logging + into_inner() preserves data
        // while making the issue observable for debugging.
        let mut inner = self.inner.lock().unwrap_or_else(|e| {
            crate::eap_error!("Mutex poisoned in eap register, recovering data");
            e.into_inner()
        });
        NetConnClient::reg_custom_eap_handler(net_type, eap_code, eap_type)?;

        // Remove existing entry with same (net_type, eap_code, eap_type) — single-callback-per-key design
        inner.retain(|e| e.net_type != net_type || e.eap_code != eap_code || e.eap_type != eap_type);
        inner.push(EapCallbackEntry { net_type, eap_code, eap_type, callback });
        Ok(())
    }

    pub fn unregister(&self, net_type: i32, eap_code: i32, eap_type: i32, callback_ref: Option<CallbackFlavor>) -> Result<(), i32> {
        // Recover from poison: panicking would crash the service, and re-initializing
        // would lose all registered callbacks. Logging + into_inner() preserves data
        // while making the issue observable for debugging.
        let mut inner = self.inner.lock().unwrap_or_else(|e| {
            crate::eap_error!("Mutex poisoned in eap unregister, recovering data");
            e.into_inner()
        });
        // When callback_ref is provided, remove only entries that match both the
        // (net_type, eap_code, eap_type) key AND the specific callback reference,
        // preventing accidental removal of a correctly-registered callback when
        // the caller supplies a mismatched callback_ref.
        if let Some(ref callback) = callback_ref {
            inner.retain(|e| e.net_type != net_type || e.eap_code != eap_code
                || e.eap_type != eap_type || e.callback != *callback);
        } else {
            // Without a callback_ref, remove all entries matching the key only
            inner.retain(|e| e.net_type != net_type || e.eap_code != eap_code || e.eap_type != eap_type);
        }

        if inner.is_empty() {
            NetConnClient::unreg_custom_eap_handler(net_type, eap_code, eap_type)?;
        }

        Ok(())
    }
}

struct StateCallbackRegister {
    inner: Mutex<Vec<GlobalRefCallback<(bridge::EthEapStateInfo,)>>>,
}

impl StateCallbackRegister {
    fn get_instance() -> &'static Self {
        static INSTANCE: OnceLock<StateCallbackRegister> = OnceLock::new();
        INSTANCE.get_or_init(|| StateCallbackRegister {
            inner: Mutex::new(Vec::new()),
        })
    }

    fn register(&self, callback: GlobalRefCallback<(bridge::EthEapStateInfo,)>) -> Result<(), i32> {
        let mut inner = self.inner.lock().unwrap_or_else(|e| {
            crate::eap_error!("Mutex poisoned in state callback register, recovering data");
            e.into_inner()
        });
        if inner.iter().any(|current| current == &callback) {
            return Ok(());
        }
        if inner.is_empty() {
            // Subscribe before publishing the callback; a failed IPC must not leave a local entry.
            NetConnClient::register_eth_eap_state_callback()?;
        }
        inner.push(callback);
        Ok(())
    }

    fn unregister(&self, callback: Option<GlobalRefCallback<(bridge::EthEapStateInfo,)>>) -> Result<(), i32> {
        let mut inner = self.inner.lock().unwrap_or_else(|e| {
            crate::eap_error!("Mutex poisoned in state callback unregister, recovering data");
            e.into_inner()
        });
        let keep = |current: &GlobalRefCallback<(bridge::EthEapStateInfo,)>| match callback.as_ref() {
            Some(requested) => current != requested,
            None => false,
        };
        let remove_last = !inner.is_empty() && !inner.iter().any(&keep);
        if remove_last {
            // Preserve local subscriptions if the backend refuses to unregister.
            NetConnClient::unregister_eth_eap_state_callback()?;
        }
        inner.retain(keep);
        Ok(())
    }

    fn dispatch(&self, state_info: bridge::EthEapStateInfo) {
        // Execute from a snapshot after dropping the registry lock, allowing callbacks to call off().
        let callbacks = {
            let inner = self.inner.lock().unwrap_or_else(|e| {
                crate::eap_error!("Mutex poisoned in state callback dispatch, recovering data");
                e.into_inner()
            });
            inner.iter().cloned().collect::<Vec<_>>()
        };
        for callback in callbacks {
            callback.execute((state_info.clone(),));
        }
    }
}

impl Register {
    pub fn on_eap_data(&self, data: bridge::EapData) {
        // Recover from poison: panicking would crash the service, and re-initializing
        // would lose all registered callbacks. Logging + into_inner() preserves data
        // while making the issue observable for debugging.
        let inner = self.inner.lock().unwrap_or_else(|e| {
            crate::eap_error!("Mutex poisoned in on_eap_data, recovering data");
            e.into_inner()
        });
        for entry in inner.deref() {
            if let CallbackFlavor::CustomEapHandler(callback) = &entry.callback {
                callback.execute((data.clone(),));
            }
        }
    }
}

pub fn execute_eap_data(data: EapAniData) {
    let bridge_data = bridge::EapData {
        msg_id: data.msg_id,
        eap_buffer: ani_rs::typed_array::ArrayBuffer::new_with_vec(data.eap_buffer),
        buffer_len: data.buffer_len,
    };
    Register::get_instance().on_eap_data(bridge_data);
}

#[ani_rs::native]
pub(crate) fn reg_custom_eap_handler(
    env: &AniEnv,
    net_type: i32,
    eap_code: i32,
    eap_type: i32,
    callback: AniFnObject,
) -> Result<(), BusinessError> {
    let callback = callback.into_global_callback(env).map_err(|e| BusinessError::from(e))?;
    let flavor = CallbackFlavor::CustomEapHandler(callback);
    Register::get_instance()
        .register(net_type, eap_code, eap_type, flavor)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn unreg_custom_eap_handler(
    env: &AniEnv,
    net_type: i32,
    eap_code: i32,
    eap_type: i32,
    callback: AniFnObject,
) -> Result<(), BusinessError> {
    let callback_flavor = if env.is_undefined(&callback).map_err(|e| BusinessError::from(e))? {
        None
    } else {
        let callback_global = callback.into_global_callback(env).map_err(|e| BusinessError::from(e))?;
        Some(CallbackFlavor::CustomEapHandler(callback_global))
    };

    Register::get_instance()
        .unregister(net_type, eap_code, eap_type, callback_flavor)
        .map_err(convert_to_business_error)
}

pub fn execute_eth_eap_state_changed(state_info: EthEapStateInfoAni) {
    StateCallbackRegister::get_instance().dispatch(state_info.into());
}

#[ani_rs::native]
pub(crate) fn on_state_change(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let callback = callback.into_global_callback(env).map_err(|e| BusinessError::from(e))?;
    StateCallbackRegister::get_instance()
        .register(callback)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn off_state_change(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let callback = if env.is_undefined(&callback).map_err(|e| BusinessError::from(e))? {
        None
    } else {
        Some(callback.into_global_callback(env).map_err(|e| BusinessError::from(e))?)
    };
    StateCallbackRegister::get_instance()
        .unregister(callback)
        .map_err(convert_to_business_error)
}

#[derive(PartialEq, Eq)]
pub enum CallbackFlavor {
    CustomEapHandler(GlobalRefCallback<(bridge::EapData,)>),
}
