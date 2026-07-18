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
    collections::HashMap,
    ops::Deref,
    sync::{Mutex, OnceLock},
};

use ani_rs::{
    business_error::BusinessError,
    objects::{AniFnObject, AniObject, GlobalRefCallback},
    AniEnv,
};

use crate::{
    bridge,
    wrapper::{convert_to_business_error, ffi, get_native_ptr},
};

struct Register {
    inner: Mutex<HashMap<i64, Vec<CallbackFlavor>>>,
}

impl Register {
    fn new() -> Self {
        Self {
            inner: Mutex::new(HashMap::new()),
        }
    }

    pub fn get_instance() -> &'static Self {
        static INSTANCE: OnceLock<Register> = OnceLock::new();
        INSTANCE.get_or_init(Register::new)
    }

    pub fn register(&self, native_ptr: i64, callback: CallbackFlavor) -> Result<(), i32> {
        let mut inner = self.inner.lock().unwrap();
        inner.entry(native_ptr).or_default().retain(|c| *c != callback);
        inner.entry(native_ptr).or_default().push(callback);
        Ok(())
    }

    pub fn unregister(
        &self,
        native_ptr: i64,
        callback_ref: Option<CallbackFlavor>,
        event: MdnsEventType,
    ) -> Result<(), i32> {
        let mut inner = self.inner.lock().unwrap();
        let entry = inner.get_mut(&native_ptr);
        if let Some(callbacks) = entry {
            if let Some(callback) = callback_ref {
                callbacks.retain(|c| *c != callback);
            } else {
                callbacks.retain(|c| !event.matches(c));
            }
            if callbacks.is_empty() {
                inner.remove(&native_ptr);
            }
        }
        Ok(())
    }

    pub fn on_discovery_start(&self, info: ffi::MDnsServiceInfoAni, ret_code: i32, native_ptr: i64) {
        let inner = self.inner.lock().unwrap();
        let service_info = bridge::LocalServiceInfo::from(info);
        let event_info = bridge::DiscoveryEventInfo {
            service_info,
            error_code: if ret_code != 0 { Some(ret_code) } else { None },
        };
        if let Some(callbacks) = inner.get(&native_ptr) {
            for listen in callbacks.deref() {
                if let CallbackFlavor::DiscoveryStart(callback) = listen {
                    callback.execute((event_info.clone(),));
                }
            }
        }
    }

    pub fn on_discovery_stop(&self, info: ffi::MDnsServiceInfoAni, ret_code: i32, native_ptr: i64) {
        let inner = self.inner.lock().unwrap();
        let service_info = bridge::LocalServiceInfo::from(info);
        let event_info = bridge::DiscoveryEventInfo {
            service_info,
            error_code: if ret_code != 0 { Some(ret_code) } else { None },
        };
        if let Some(callbacks) = inner.get(&native_ptr) {
            for listen in callbacks.deref() {
                if let CallbackFlavor::DiscoveryStop(callback) = listen {
                    callback.execute((event_info.clone(),));
                }
            }
        }
    }

    pub fn on_service_found(&self, info: ffi::MDnsServiceInfoAni, ret_code: i32, native_ptr: i64) {
        let inner = self.inner.lock().unwrap();
        let _ = ret_code;
        let param = bridge::LocalServiceInfo::from(info);
        if let Some(callbacks) = inner.get(&native_ptr) {
            for listen in callbacks.deref() {
                if let CallbackFlavor::ServiceFound(callback) = listen {
                    callback.execute((param.clone(),));
                }
            }
        }
    }

    pub fn on_service_lost(&self, info: ffi::MDnsServiceInfoAni, ret_code: i32, native_ptr: i64) {
        let inner = self.inner.lock().unwrap();
        let _ = ret_code;
        let param = bridge::LocalServiceInfo::from(info);
        if let Some(callbacks) = inner.get(&native_ptr) {
            for listen in callbacks.deref() {
                if let CallbackFlavor::ServiceLost(callback) = listen {
                    callback.execute((param.clone(),));
                }
            }
        }
    }
}

pub fn execute_discovery_start(info: ffi::MDnsServiceInfoAni, ret_code: i32, native_ptr: i64) {
    Register::get_instance().on_discovery_start(info, ret_code, native_ptr);
}

pub fn execute_discovery_stop(info: ffi::MDnsServiceInfoAni, ret_code: i32, native_ptr: i64) {
    Register::get_instance().on_discovery_stop(info, ret_code, native_ptr);
}

pub fn execute_service_found(info: ffi::MDnsServiceInfoAni, ret_code: i32, native_ptr: i64) {
    Register::get_instance().on_service_found(info, ret_code, native_ptr);
}

pub fn execute_service_lost(info: ffi::MDnsServiceInfoAni, ret_code: i32, native_ptr: i64) {
    Register::get_instance().on_service_lost(info, ret_code, native_ptr);
}

#[ani_rs::native]
pub fn on_discovery_start(env: &AniEnv, this: AniObject, callback: AniFnObject) -> Result<(), BusinessError> {
    let native_ptr = get_native_ptr(env, &this);
    let callback = callback.into_global_callback(env).unwrap();
    let flavor = CallbackFlavor::DiscoveryStart(callback);
    Register::get_instance()
        .register(native_ptr, flavor)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn on_discovery_stop(env: &AniEnv, this: AniObject, callback: AniFnObject) -> Result<(), BusinessError> {
    let native_ptr = get_native_ptr(env, &this);
    let callback = callback.into_global_callback(env).unwrap();
    let flavor = CallbackFlavor::DiscoveryStop(callback);
    Register::get_instance()
        .register(native_ptr, flavor)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn on_service_found(env: &AniEnv, this: AniObject, callback: AniFnObject) -> Result<(), BusinessError> {
    let native_ptr = get_native_ptr(env, &this);
    let callback = callback.into_global_callback(env).unwrap();
    let flavor = CallbackFlavor::ServiceFound(callback);
    Register::get_instance()
        .register(native_ptr, flavor)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn on_service_lost(env: &AniEnv, this: AniObject, callback: AniFnObject) -> Result<(), BusinessError> {
    let native_ptr = get_native_ptr(env, &this);
    let callback = callback.into_global_callback(env).unwrap();
    let flavor = CallbackFlavor::ServiceLost(callback);
    Register::get_instance()
        .register(native_ptr, flavor)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn off_discovery_start(env: &AniEnv, this: AniObject, callback: AniFnObject) -> Result<(), BusinessError> {
    let native_ptr = get_native_ptr(env, &this);
    let callback_flavor = if env.is_undefined(&callback).unwrap() {
        None
    } else {
        let callback_global = callback.into_global_callback(env).unwrap();
        Some(CallbackFlavor::DiscoveryStart(callback_global))
    };

    Register::get_instance()
        .unregister(native_ptr, callback_flavor, MdnsEventType::EventDiscoveryStart)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn off_discovery_stop(env: &AniEnv, this: AniObject, callback: AniFnObject) -> Result<(), BusinessError> {
    let native_ptr = get_native_ptr(env, &this);
    let callback_flavor = if env.is_undefined(&callback).unwrap() {
        None
    } else {
        let callback_global = callback.into_global_callback(env).unwrap();
        Some(CallbackFlavor::DiscoveryStop(callback_global))
    };

    Register::get_instance()
        .unregister(native_ptr, callback_flavor, MdnsEventType::EventDiscoveryStop)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn off_service_found(env: &AniEnv, this: AniObject, callback: AniFnObject) -> Result<(), BusinessError> {
    let native_ptr = get_native_ptr(env, &this);
    let callback_flavor = if env.is_undefined(&callback).unwrap() {
        None
    } else {
        let callback_global = callback.into_global_callback(env).unwrap();
        Some(CallbackFlavor::ServiceFound(callback_global))
    };

    Register::get_instance()
        .unregister(native_ptr, callback_flavor, MdnsEventType::EventServiceFound)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn off_service_lost(env: &AniEnv, this: AniObject, callback: AniFnObject) -> Result<(), BusinessError> {
    let native_ptr = get_native_ptr(env, &this);
    let callback_flavor = if env.is_undefined(&callback).unwrap() {
        None
    } else {
        let callback_global = callback.into_global_callback(env).unwrap();
        Some(CallbackFlavor::ServiceLost(callback_global))
    };

    Register::get_instance()
        .unregister(native_ptr, callback_flavor, MdnsEventType::EventServiceLost)
        .map_err(convert_to_business_error)
}

#[derive(PartialEq, Eq)]
pub enum CallbackFlavor {
    DiscoveryStart(GlobalRefCallback<(bridge::DiscoveryEventInfo,)>),
    DiscoveryStop(GlobalRefCallback<(bridge::DiscoveryEventInfo,)>),
    ServiceFound(GlobalRefCallback<(bridge::LocalServiceInfo,)>),
    ServiceLost(GlobalRefCallback<(bridge::LocalServiceInfo,)>),
}

pub enum MdnsEventType {
    EventDiscoveryStart,
    EventDiscoveryStop,
    EventServiceFound,
    EventServiceLost,
}

impl MdnsEventType {
    fn matches(&self, flavor: &CallbackFlavor) -> bool {
        match self {
            MdnsEventType::EventDiscoveryStart => {
                matches!(flavor, CallbackFlavor::DiscoveryStart(_))
            }
            MdnsEventType::EventDiscoveryStop => {
                matches!(flavor, CallbackFlavor::DiscoveryStop(_))
            }
            MdnsEventType::EventServiceFound => {
                matches!(flavor, CallbackFlavor::ServiceFound(_))
            }
            MdnsEventType::EventServiceLost => {
                matches!(flavor, CallbackFlavor::ServiceLost(_))
            }
        }
    }
}
