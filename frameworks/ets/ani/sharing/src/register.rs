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

use std::sync::{Arc, Mutex, OnceLock};

use ani_rs::{
    business_error::BusinessError,
    callback::{Callback, GlobalCallback},
    objects::AniFnObject,
    AniEnv,
};

use crate::{
    bridge,
    wrapper::{ffi, SharingCallbackUnregister, SharingClient},
};

struct Registar {
    inner: Mutex<Vec<(Arc<CallbackFlavor>, SharingCallbackUnregister)>>,
}

impl Registar {
    fn new() -> Self {
        Self {
            inner: Mutex::new(Vec::new()),
        }
    }

    pub fn get_instance() -> &'static Self {
        static INSTANCE: OnceLock<Registar> = OnceLock::new();
        INSTANCE.get_or_init(Registar::new)
    }

    pub fn register(&self, callback: CallbackFlavor) -> Result<(), i32> {
        let mut inner = self.inner.lock().unwrap();
        for w in inner.iter() {
            if *w.0 == callback {
                return Err(-1);
            }
        }
        let callback = Arc::new(callback);
        let unregister =
            SharingClient::register_sharing_callback(SharingCallback::new(callback.clone()))?;
        inner.push((callback, unregister));
        Ok(())
    }

    pub fn unregister_callback(&self, callback: &CallbackFlavor) -> Result<(), i32> {
        let mut ret = 0;
        self.inner.lock().unwrap().retain_mut(|cb| {
            if *cb.0 == *callback {
                if let Err(e) = cb.1.unregister() {
                    ret = e;
                };
                return false;
            }
            true
        });
        Ok(())
    }

    pub fn unregister_sharing_state_change(
        &self,
        callback: Option<&CallbackFlavor>,
    ) -> Result<(), i32> {
        if let Some(callback) = callback {
            self.unregister_callback(callback)
        } else {
            let mut ret = 0;
            self.inner.lock().unwrap().retain_mut(|cb| {
                if let CallbackFlavor::SharingStateChange(_) = &*cb.0 {
                    if let Err(e) = cb.1.unregister() {
                        ret = e;
                        true
                    } else {
                        false
                    }
                } else {
                    true
                }
            });
            if ret != 0 {
                return Err(ret);
            }
            Ok(())
        }
    }

    pub fn unregister_interface_sharing_state_change(
        &self,
        callback: Option<&CallbackFlavor>,
    ) -> Result<(), i32> {
        if let Some(callback) = callback {
            self.unregister_callback(callback)
        } else {
            let mut ret = 0;
            self.inner.lock().unwrap().retain_mut(|cb| {
                if let CallbackFlavor::InterfaceSharingStateChange(_) = &*cb.0 {
                    if let Err(e) = cb.1.unregister() {
                        ret = e;
                        true
                    } else {
                        false
                    }
                } else {
                    true
                }
            });
            if ret != 0 {
                return Err(ret);
            }
            Ok(())
        }
    }

    pub fn unregister_sharing_upstream_change(
        &self,
        callback: Option<&CallbackFlavor>,
    ) -> Result<(), i32> {
        if let Some(callback) = callback {
            self.unregister_callback(callback)
        } else {
            let mut ret = 0;
            self.inner.lock().unwrap().retain_mut(|cb| {
                if let CallbackFlavor::SharingUpstreamChange(_) = &*cb.0 {
                    if let Err(e) = cb.1.unregister() {
                        ret = e;
                        true
                    } else {
                        false
                    }
                } else {
                    true
                }
            });
            if ret != 0 {
                return Err(ret);
            }
            Ok(())
        }
    }
}

#[ani_rs::native]
pub fn on_sharing_state_change(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let callback = Callback::new(callback).into_global(env).unwrap();
    let flavor = CallbackFlavor::SharingStateChange(callback);
    Registar::get_instance().register(flavor).map_err(|err| {
        BusinessError::new(
            err,
            "Failed to register sharing state change callback".to_string(),
        )
    })
}

#[ani_rs::native]
pub fn on_interface_sharing_state_change(
    env: &AniEnv,
    callback: AniFnObject,
) -> Result<(), BusinessError> {
    let callback = Callback::new(callback).into_global(env).unwrap();
    let flavor = CallbackFlavor::InterfaceSharingStateChange(callback);
    Registar::get_instance().register(flavor).map_err(|err| {
        BusinessError::new(
            err,
            "Failed to register interface sharing state change callback".to_string(),
        )
    })
}

#[ani_rs::native]
pub fn on_sharing_upstream_change(
    env: &AniEnv,
    callback: AniFnObject,
) -> Result<(), BusinessError> {
    let callback = Callback::new(callback).into_global(env).unwrap();
    let flavor = CallbackFlavor::SharingUpstreamChange(callback);
    Registar::get_instance().register(flavor).map_err(|err| {
        BusinessError::new(
            err,
            "Failed to register sharing upstream change callback".to_string(),
        )
    })
}

#[ani_rs::native]
pub fn off_sharing_state_change(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    if env.is_undefined(&callback.clone().into()).unwrap() {
        return Registar::get_instance()
            .unregister_sharing_state_change(None)
            .map_err(|err| {
                BusinessError::new(
                    err,
                    "Failed to unregister sharing state change callback".to_string(),
                )
            });
    }
    let callback = Callback::new(callback).into_global(env).unwrap();
    let flavor = CallbackFlavor::SharingUpstreamChange(callback);
    Registar::get_instance()
        .unregister_sharing_state_change(Some(&flavor))
        .map_err(|err| {
            BusinessError::new(
                err,
                "Failed to unregister sharing state change callback".to_string(),
            )
        })
}

#[ani_rs::native]
pub fn off_interface_sharing_state_change(
    env: &AniEnv,
    callback: AniFnObject,
) -> Result<(), BusinessError> {
    if env.is_undefined(&callback.clone().into()).unwrap() {
        return Registar::get_instance()
            .unregister_interface_sharing_state_change(None)
            .map_err(|err| {
                BusinessError::new(
                    err,
                    "Failed to unregister interface sharing state change callback".to_string(),
                )
            });
    }
    let callback = Callback::new(callback).into_global(env).unwrap();
    let flavor = CallbackFlavor::InterfaceSharingStateChange(callback);
    Registar::get_instance()
        .unregister_interface_sharing_state_change(Some(&flavor))
        .map_err(|err| {
            BusinessError::new(
                err,
                "Failed to unregister interface sharing state change callback".to_string(),
            )
        })
}

#[ani_rs::native]
pub fn off_sharing_upstream_change(
    env: &AniEnv,
    callback: AniFnObject,
) -> Result<(), BusinessError> {
    if env.is_undefined(&callback.clone().into()).unwrap() {
        return Registar::get_instance()
            .unregister_sharing_upstream_change(None)
            .map_err(|err| {
                BusinessError::new(
                    err,
                    "Failed to unregister sharing upstream change callback".to_string(),
                )
            });
    }
    let callback = Callback::new(callback).into_global(env).unwrap();
    let flavor = CallbackFlavor::SharingUpstreamChange(callback);
    Registar::get_instance()
        .unregister_sharing_upstream_change(Some(&flavor))
        .map_err(|err| {
            BusinessError::new(
                err,
                "Failed to unregister sharing upstream change callback".to_string(),
            )
        })
}

#[derive(PartialEq, Eq)]
pub enum CallbackFlavor {
    SharingStateChange(GlobalCallback<(bool,)>),
    InterfaceSharingStateChange(GlobalCallback<(bridge::InterfaceSharingStateInfo,)>),
    SharingUpstreamChange(GlobalCallback<(bridge::NetHandle,)>),
}

#[derive(PartialEq, Eq)]
pub struct SharingCallback {
    inner: Arc<CallbackFlavor>,
}

impl SharingCallback {
    fn new(flavor: Arc<CallbackFlavor>) -> Self {
        Self { inner: flavor }
    }

    pub fn on_sharing_state_change(&self, is_running: bool) {
        if let CallbackFlavor::SharingStateChange(callback) = &*self.inner {
            callback.execute_collective((is_running,));
        }
    }

    pub fn on_interface_sharing_state_change(&self, info: ffi::InterfaceSharingStateInfo) {
        if let CallbackFlavor::InterfaceSharingStateChange(callback) = &*self.inner {
            callback.execute_collective((info.into(),));
        }
    }

    pub fn on_sharing_upstream_change(&self, net_handle: ffi::NetHandle) {
        if let CallbackFlavor::SharingUpstreamChange(callback) = &*self.inner {
            callback.execute_collective((net_handle.into(),));
        }
    }
}
