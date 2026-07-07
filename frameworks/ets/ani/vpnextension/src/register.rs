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

use crate::wrapper::{convert_to_business_error, ffi};
use crate::vpnext_error;

struct Register {
    inner: Mutex<Vec<GlobalRefCallback<(bool,)>>>,
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

    pub fn register(&self, callback: GlobalRefCallback<(bool,)>) -> Result<(), i32> {
        let mut inner = self.inner.lock().map_err(|_| {
            vpnext_error!("Mutex lock poisoned in register");
            1
        })?;
        if inner.is_empty() {
            let ret = ffi::VpnExtObserverRegister();
            if ret != 0 {
                return Err(ret);
            }
        }
        inner.push(callback);
        Ok(())
    }

    pub fn unregister(&self, callback: Option<GlobalRefCallback<(bool,)>>) -> Result<(), i32> {
        let mut inner = self.inner.lock().map_err(|_| {
            vpnext_error!("Mutex lock poisoned in unregister");
            1
        })?;
        if let Some(callback) = callback {
            inner.retain(|c| *c != callback)
        } else {
            inner.clear()
        }

        if inner.is_empty() {
            let ret = ffi::VpnExtObserverUnRegister();
            if ret != 0 {
                return Err(ret);
            }
        }

        Ok(())
    }

    pub fn on_authorization_result(&self, authorized: bool) {
        let callbacks = match self.inner.lock() {
            Ok(guard) => {
                let cbs: Vec<_> = guard.deref().iter().cloned().collect();
                cbs
            }
            Err(_) => {
                vpnext_error!("Mutex lock poisoned in on_authorization_result, skip callback");
                return;
            }
        };
        for callback in &callbacks {
            callback.execute((authorized,));
        }
    }
}

pub fn execute_vpn_ext_authorization_result(authorized: bool) {
    Register::get_instance().on_authorization_result(authorized);
}

#[ani_rs::native]
pub fn regist_on_auth_result(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let callback = callback.into_global_callback(env).map_err(|e| {
        vpnext_error!("into_global_callback failed: {:?}", e);
        convert_to_business_error(0)
    })?;
    Register::get_instance()
        .register(callback)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn unregist_on_auth_result(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let callback_flavor = if env.is_undefined(&callback).map_err(|e| {
        vpnext_error!("is_undefined check failed: {:?}", e);
        convert_to_business_error(0)
    })? {
        None
    } else {
        let callback_global = callback.into_global_callback(env).map_err(|e| {
            vpnext_error!("into_global_callback failed: {:?}", e);
            convert_to_business_error(0)
        })?;
        Some(callback_global)
    };

    Register::get_instance()
        .unregister(callback_flavor)
        .map_err(convert_to_business_error)
}
