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
    wrapper::{convert_to_business_error, ffi},
};
use crate::vpn_error;

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
        let mut inner = self.inner.lock().map_err(|_| {
            vpn_error!("Mutex lock poisoned in register");
            1
        })?;
        let ret = ffi::VpnObserverRegister();
        if ret != 0 {
            return Err(ret);
        }
        inner.push(callback);
        Ok(())
    }

    pub fn unregister(
        &self,
        callback_ref: Option<CallbackFlavor>,
        event: VpnEventType,
    ) -> Result<(), i32> {
        let mut inner = self.inner.lock().map_err(|_| {
            vpn_error!("Mutex lock poisoned in unregister");
            1
        })?;
        if let Some(callback) = callback_ref {
            inner.retain(|c| *c != callback)
        } else {
            inner.retain(|c| match event {
                VpnEventType::EventConnect => {
                    if let CallbackFlavor::Connect(_) = c {
                        false
                    } else {
                        true
                    }
                }
                VpnEventType::EventConnectMulti => {
                    if let CallbackFlavor::ConnectMulti(_) = c {
                        false
                    } else {
                        true
                    }
                }
            })
        }

        if inner.is_empty() {
            let ret = ffi::VpnObserverUnRegister();
            if ret != 0 {
                return Err(ret);
            }
        }

        Ok(())
    }

    pub fn on_vpn_state_changed(&self, info: ffi::VpnStateInfo) {
        let inner = match self.inner.lock() {
            Ok(guard) => guard,
            Err(_) => {
                vpn_error!("Mutex lock poisoned in on_vpn_state_changed, skip callback");
                return;
            }
        };
        for listen in inner.deref() {
            if let CallbackFlavor::Connect(callback) = listen {
                let state = bridge::VpnConnectState::from_ffi(&info);
                callback.execute((state,));
            }
        }
    }

    pub fn on_multi_vpn_state_changed(&self, info: ffi::VpnStateInfo) {
        let inner = match self.inner.lock() {
            Ok(guard) => guard,
            Err(_) => {
                vpn_error!("Mutex lock poisoned in on_multi_vpn_state_changed, skip callback");
                return;
            }
        };
        let state = bridge::MultiVpnConnectState {
            is_connected: info.is_connected,
            bundle_name: info.bundle_name.clone(),
            vpn_id: info.vpn_id.clone(),
        };
        for listen in inner.deref() {
            if let CallbackFlavor::ConnectMulti(callback) = listen {
                callback.execute((state.clone(),));
            }
        }
    }
}

pub fn execute_vpn_state_changed(info: ffi::VpnStateInfo) {
    Register::get_instance().on_vpn_state_changed(info);
}

pub fn execute_multi_vpn_state_changed(info: ffi::VpnStateInfo) {
    Register::get_instance().on_multi_vpn_state_changed(info);
}

#[ani_rs::native]
pub fn on_connect(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let callback = callback.into_global_callback(env).map_err(|e| {
        vpn_error!("into_global_callback failed: {:?}", e);
        convert_to_business_error(0)
    })?;
    let flavor = CallbackFlavor::Connect(callback);
    Register::get_instance()
        .register(flavor)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn on_connect_multi(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let callback = callback.into_global_callback(env).map_err(|e| {
        vpn_error!("into_global_callback failed: {:?}", e);
        convert_to_business_error(0)
    })?;
    let flavor = CallbackFlavor::ConnectMulti(callback);
    Register::get_instance()
        .register(flavor)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn off_connect(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let callback_flavor = if env.is_undefined(&callback).map_err(|e| {
        vpn_error!("is_undefined check failed: {:?}", e);
        convert_to_business_error(0)
    })? {
        None
    } else {
        let callback_global = callback.into_global_callback(env).map_err(|e| {
            vpn_error!("into_global_callback failed: {:?}", e);
            convert_to_business_error(0)
        })?;
        Some(CallbackFlavor::Connect(callback_global))
    };

    Register::get_instance()
        .unregister(callback_flavor, VpnEventType::EventConnect)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn off_connect_multi(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let callback_flavor = if env.is_undefined(&callback).map_err(|e| {
        vpn_error!("is_undefined check failed: {:?}", e);
        convert_to_business_error(0)
    })? {
        None
    } else {
        let callback_global = callback.into_global_callback(env).map_err(|e| {
            vpn_error!("into_global_callback failed: {:?}", e);
            convert_to_business_error(0)
        })?;
        Some(CallbackFlavor::ConnectMulti(callback_global))
    };

    Register::get_instance()
        .unregister(callback_flavor, VpnEventType::EventConnectMulti)
        .map_err(convert_to_business_error)
}

#[derive(PartialEq, Eq)]
pub enum CallbackFlavor {
    Connect(GlobalRefCallback<(bridge::VpnConnectState,)>),
    ConnectMulti(GlobalRefCallback<(bridge::MultiVpnConnectState,)>),
}

pub enum VpnEventType {
    EventConnect,
    EventConnectMulti,
}
