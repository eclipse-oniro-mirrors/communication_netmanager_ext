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
    vpnext::get_native_ptr,
    wrapper::convert_to_business_error,
};

struct Register {
    inner: Mutex<HashMap<i64, Vec<GlobalRefCallback<(bool,)>>>>,
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

    pub fn register(&self, native_ptr: i64, callback: GlobalRefCallback<(bool,)>) -> Result<(), i32> {
        let mut inner = self.inner.lock().unwrap();
        inner.entry(native_ptr).or_default().retain(|c| *c != callback);
        inner.entry(native_ptr).or_default().push(callback);
        Ok(())
    }

    pub fn unregister(
        &self,
        native_ptr: i64,
        callback: Option<GlobalRefCallback<(bool,)>>,
    ) -> Result<(), i32> {
        let mut inner = self.inner.lock().unwrap();
        if let Some(listeners) = inner.get_mut(&native_ptr) {
            if let Some(callback) = callback {
                listeners.retain(|c| *c != callback);
            } else {
                listeners.clear();
            }
            if listeners.is_empty() {
                inner.remove(&native_ptr);
            }
        }
        Ok(())
    }

    pub fn on_authorization_result(&self, authorized: bool) {
        let inner = self.inner.lock().unwrap();
        for listeners in inner.values() {
            for callback in listeners.deref() {
                callback.execute((authorized,));
            }
        }
    }
}

pub fn unregister_all(native_ptr: i64) {
    let _ = Register::get_instance().unregister(native_ptr, None);
}

pub fn execute_vpn_ext_authorization_result(authorized: bool) {
    Register::get_instance().on_authorization_result(authorized);
}

#[ani_rs::native]
pub fn on_authorization_result(env: &AniEnv, this: AniObject, callback: AniFnObject) -> Result<(), BusinessError> {
    let native_ptr = get_native_ptr(env, &this);
    let callback = callback.into_global_callback(env).unwrap();
    Register::get_instance()
        .register(native_ptr, callback)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn off_authorization_result(env: &AniEnv, this: AniObject, callback: AniFnObject) -> Result<(), BusinessError> {
    let native_ptr = get_native_ptr(env, &this);
    let callback_flavor = if env.is_undefined(&callback).unwrap() {
        None
    } else {
        let callback_global = callback.into_global_callback(env).unwrap();
        Some(callback_global)
    };

    Register::get_instance()
        .unregister(native_ptr, callback_flavor)
        .map_err(convert_to_business_error)
}
