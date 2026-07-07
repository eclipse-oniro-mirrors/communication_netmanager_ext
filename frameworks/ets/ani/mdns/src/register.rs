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
    wrapper::{convert_to_business_error, ffi, MDnsClient},
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

        if inner.is_empty() {
            MDnsClient::register_mdns_observer()?;
        }

        inner.push(callback);
        Ok(())
    }

    pub fn unregister(
        &self,
        callback_ref: Option<CallbackFlavor>,
        event: MdnsEventType,
    ) -> Result<(), i32> {
        let mut inner = self.inner.lock().unwrap();
        if let Some(callback) = callback_ref {
            inner.retain(|c| *c != callback)
        } else {
            inner.retain(|c| match event {
                MdnsEventType::EventDiscoveryStart => {
                    matches!(c, CallbackFlavor::DiscoveryStart(_))
                }
                MdnsEventType::EventDiscoveryStop => {
                    matches!(c, CallbackFlavor::DiscoveryStop(_))
                }
                MdnsEventType::EventServiceFound => {
                    matches!(c, CallbackFlavor::ServiceFound(_))
                }
                MdnsEventType::EventServiceLost => {
                    matches!(c, CallbackFlavor::ServiceLost(_))
                }
                MdnsEventType::EventRegisterResult => {
                    matches!(c, CallbackFlavor::RegisterResult(_))
                }
                MdnsEventType::EventResolveResult => {
                    matches!(c, CallbackFlavor::ResolveResult(_))
                }
            })
        }

        if inner.is_empty() {
            MDnsClient::unregister_mdns_observer()?;
        }

        Ok(())
    }

    pub fn on_discovery_start(&self, info: ffi::MDnsServiceInfoFFI, ret_code: i32) {
        let inner = self.inner.lock().unwrap();
        let service_info = bridge::LocalServiceInfo::from(info);
        let event_info = bridge::DiscoveryEventInfo {
            service_info,
            error_code: if ret_code != 0 { Some(ret_code) } else { None },
        };
        for listen in inner.deref() {
            if let CallbackFlavor::DiscoveryStart(callback) = listen {
                callback.execute((event_info.clone(),));
            }
        }
    }

    pub fn on_discovery_stop(&self, info: ffi::MDnsServiceInfoFFI, ret_code: i32) {
        let inner = self.inner.lock().unwrap();
        let service_info = bridge::LocalServiceInfo::from(info);
        let event_info = bridge::DiscoveryEventInfo {
            service_info,
            error_code: if ret_code != 0 { Some(ret_code) } else { None },
        };
        for listen in inner.deref() {
            if let CallbackFlavor::DiscoveryStop(callback) = listen {
                callback.execute((event_info.clone(),));
            }
        }
    }

    pub fn on_service_found(&self, info: ffi::MDnsServiceInfoFFI, ret_code: i32) {
        let inner = self.inner.lock().unwrap();
        let param = bridge::LocalServiceInfo::from(info);
        for listen in inner.deref() {
            if let CallbackFlavor::ServiceFound(callback) = listen {
                callback.execute((param.clone(),));
            }
        }
    }

    pub fn on_service_lost(&self, info: ffi::MDnsServiceInfoFFI, ret_code: i32) {
        let inner = self.inner.lock().unwrap();
        let param = bridge::LocalServiceInfo::from(info);
        for listen in inner.deref() {
            if let CallbackFlavor::ServiceLost(callback) = listen {
                callback.execute((param.clone(),));
            }
        }
    }

    pub fn on_register_result(&self, info: ffi::MDnsServiceInfoFFI, ret_code: i32) {
        let inner = self.inner.lock().unwrap();
        let event_info = bridge::DiscoveryEventInfo {
            service_info: bridge::LocalServiceInfo::from(info),
            error_code: if ret_code != 0 { Some(ret_code) } else { None },
        };
        for listen in inner.deref() {
            if let CallbackFlavor::RegisterResult(callback) = listen {
                callback.execute((event_info.clone(),));
            }
        }
    }

    pub fn on_resolve_result(&self, info: ffi::MDnsServiceInfoFFI, ret_code: i32) {
        let inner = self.inner.lock().unwrap();
        let event_info = bridge::DiscoveryEventInfo {
            service_info: bridge::LocalServiceInfo::from(info),
            error_code: if ret_code != 0 { Some(ret_code) } else { None },
        };
        for listen in inner.deref() {
            if let CallbackFlavor::ResolveResult(callback) = listen {
                callback.execute((event_info.clone(),));
            }
        }
    }
}

// ---- FFI callbacks from C++ ----

pub fn execute_discovery_start(info: ffi::MDnsServiceInfoFFI, ret_code: i32) {
    Register::get_instance().on_discovery_start(info, ret_code);
}

pub fn execute_discovery_stop(info: ffi::MDnsServiceInfoFFI, ret_code: i32) {
    Register::get_instance().on_discovery_stop(info, ret_code);
}

pub fn execute_service_found(info: ffi::MDnsServiceInfoFFI, ret_code: i32) {
    Register::get_instance().on_service_found(info, ret_code);
}

pub fn execute_service_lost(info: ffi::MDnsServiceInfoFFI, ret_code: i32) {
    Register::get_instance().on_service_lost(info, ret_code);
}

pub fn execute_register_result(info: ffi::MDnsServiceInfoFFI, ret_code: i32) {
    Register::get_instance().on_register_result(info, ret_code);
}

pub fn execute_resolve_result(info: ffi::MDnsServiceInfoFFI, ret_code: i32) {
    Register::get_instance().on_resolve_result(info, ret_code);
}

// ---- ANI native functions ----

#[ani_rs::native]
pub fn on_discovery_start(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let callback = callback.into_global_callback(env).unwrap();
    let flavor = CallbackFlavor::DiscoveryStart(callback);
    Register::get_instance()
        .register(flavor)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn on_discovery_stop(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let callback = callback.into_global_callback(env).unwrap();
    let flavor = CallbackFlavor::DiscoveryStop(callback);
    Register::get_instance()
        .register(flavor)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn on_service_found(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let callback = callback.into_global_callback(env).unwrap();
    let flavor = CallbackFlavor::ServiceFound(callback);
    Register::get_instance()
        .register(flavor)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn on_service_lost(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let callback = callback.into_global_callback(env).unwrap();
    let flavor = CallbackFlavor::ServiceLost(callback);
    Register::get_instance()
        .register(flavor)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn off_discovery_start(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let callback_flavor = if env.is_undefined(&callback).unwrap() {
        None
    } else {
        let callback_global = callback.into_global_callback(env).unwrap();
        Some(CallbackFlavor::DiscoveryStart(callback_global))
    };

    Register::get_instance()
        .unregister(callback_flavor, MdnsEventType::EventDiscoveryStart)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn off_discovery_stop(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let callback_flavor = if env.is_undefined(&callback).unwrap() {
        None
    } else {
        let callback_global = callback.into_global_callback(env).unwrap();
        Some(CallbackFlavor::DiscoveryStop(callback_global))
    };

    Register::get_instance()
        .unregister(callback_flavor, MdnsEventType::EventDiscoveryStop)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn off_service_found(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let callback_flavor = if env.is_undefined(&callback).unwrap() {
        None
    } else {
        let callback_global = callback.into_global_callback(env).unwrap();
        Some(CallbackFlavor::ServiceFound(callback_global))
    };

    Register::get_instance()
        .unregister(callback_flavor, MdnsEventType::EventServiceFound)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn off_service_lost(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let callback_flavor = if env.is_undefined(&callback).unwrap() {
        None
    } else {
        let callback_global = callback.into_global_callback(env).unwrap();
        Some(CallbackFlavor::ServiceLost(callback_global))
    };

    Register::get_instance()
        .unregister(callback_flavor, MdnsEventType::EventServiceLost)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn on_register_result(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let callback = callback.into_global_callback(env).unwrap();
    let flavor = CallbackFlavor::RegisterResult(callback);
    Register::get_instance()
        .register(flavor)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn on_resolve_result(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let callback = callback.into_global_callback(env).unwrap();
    let flavor = CallbackFlavor::ResolveResult(callback);
    Register::get_instance()
        .register(flavor)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn off_register_result(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let callback_flavor = if env.is_undefined(&callback).unwrap() {
        None
    } else {
        let callback_global = callback.into_global_callback(env).unwrap();
        Some(CallbackFlavor::RegisterResult(callback_global))
    };

    Register::get_instance()
        .unregister(callback_flavor, MdnsEventType::EventRegisterResult)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn off_resolve_result(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let callback_flavor = if env.is_undefined(&callback).unwrap() {
        None
    } else {
        let callback_global = callback.into_global_callback(env).unwrap();
        Some(CallbackFlavor::ResolveResult(callback_global))
    };

    Register::get_instance()
        .unregister(callback_flavor, MdnsEventType::EventResolveResult)
        .map_err(convert_to_business_error)
}

#[derive(PartialEq, Eq)]
pub enum CallbackFlavor {
    DiscoveryStart(GlobalRefCallback<(bridge::DiscoveryEventInfo,)>),
    DiscoveryStop(GlobalRefCallback<(bridge::DiscoveryEventInfo,)>),
    ServiceFound(GlobalRefCallback<(bridge::LocalServiceInfo,)>),
    ServiceLost(GlobalRefCallback<(bridge::LocalServiceInfo,)>),
    RegisterResult(GlobalRefCallback<(bridge::DiscoveryEventInfo,)>),
    ResolveResult(GlobalRefCallback<(bridge::DiscoveryEventInfo,)>),
}

pub enum MdnsEventType {
    EventDiscoveryStart,
    EventDiscoveryStop,
    EventServiceFound,
    EventServiceLost,
    EventRegisterResult,
    EventResolveResult,
}
