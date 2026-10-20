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
    sync::{OnceLock, RwLock},
};

use ani_rs::{
    business_error::BusinessError,
    objects::{AniFnObject, GlobalRefCallback},
    typed_array::Uint8Array,
    AniEnv,
};

use crate::{
    bridge,
    wrapper::{
        convert_to_business_error,
        ffi::{EapCodeTypeAni, EapDataAni},
        NetConnClient,
    },
};

enum RegTriggerMode {
    SaLaunch = 0,
    UserRegister = 1,
    Unregister = 2,
}

const REGISTERINFO_MAX_NUM: usize = 16;
const EAP_ERRCODE_INTERNAL_ERROR: i32 = 33200099;

struct Register {
    inner: RwLock<HashMap<i32, HashMap<(i32, i32), Vec<GlobalRefCallback<(bridge::EapData,)>>>>>,
}

impl Register {
    fn new() -> Self {
        NetConnClient::init_sa_listener();
        Self {
            inner: RwLock::new(HashMap::new()),
        }
    }

    pub fn get_instance() -> &'static Self {
        static INSTANCE: OnceLock<Register> = OnceLock::new();
        INSTANCE.get_or_init(Register::new)
    }

    pub fn register(
        &self,
        net_type: i32,
        eap_code: i32,
        eap_type: i32,
        callback: GlobalRefCallback<(bridge::EapData,)>,
    ) -> Result<(), i32> {
        NetConnClient::validate_eap_params(net_type, eap_code, eap_type)?;

        let key = (eap_code, eap_type);
        let reg_items = {
            let mut inner = self.inner.write().unwrap_or_else(|e| {
                crate::eap_error!("RwLock poisoned in eap register, recovering data");
                e.into_inner()
            });

            let net_type_map = inner.entry(net_type).or_insert_with(HashMap::new);
            if let Some(callback_vec) = net_type_map.get(&key) {
                if callback_vec.iter().any(|c| *c == callback) {
                    crate::eap_error!("eapCode {}, eapType {} callback is registered!", eap_code, eap_type);
                    return Ok(());
                }
            } else if net_type_map.len() > REGISTERINFO_MAX_NUM {
                crate::eap_error!("RegisterInfo Exceeding the maximum value!");
                return Err(EAP_ERRCODE_INTERNAL_ERROR);
            }
            net_type_map.entry(key).or_default().push(callback);

            let mut reg_items = Vec::with_capacity(net_type_map.len());
            for &(code, typ) in net_type_map.keys() {
                reg_items.push(EapCodeTypeAni { eap_code: code, eap_type: typ });
            }
            reg_items
        };

        NetConnClient::reg_custom_eap_handler(
            RegTriggerMode::UserRegister as i32, net_type, &reg_items)
    }

    pub fn unregister(
        &self,
        net_type: i32,
        eap_code: i32,
        eap_type: i32,
        callback: &GlobalRefCallback<(bridge::EapData,)>,
    ) -> Result<(), i32> {
        NetConnClient::validate_eap_params(net_type, eap_code, eap_type)?;

        let key = (eap_code, eap_type);
        let reg_items = {
            let mut inner = self.inner.write().unwrap_or_else(|e| {
                crate::eap_error!("RwLock poisoned in eap unregister, recovering data");
                e.into_inner()
            });

            let net_type_map = match inner.get_mut(&net_type) {
                Some(map) => map,
                None => {
                    crate::eap_error!("not find netType {} handler", net_type);
                    return Err(EAP_ERRCODE_INTERNAL_ERROR);
                }
            };
            let callback_vec = match net_type_map.get_mut(&key) {
                Some(vec) => vec,
                None => {
                    crate::eap_error!("not find (eapCode {}, eapType {}) handler", eap_code, eap_type);
                    return Err(EAP_ERRCODE_INTERNAL_ERROR);
                }
            };

            let before_len = callback_vec.len();
            callback_vec.retain(|c| *c != *callback);
            if callback_vec.len() == before_len {
                return Ok(());
            }

            if callback_vec.is_empty() {
                net_type_map.remove(&key);
                let mut reg_items = Vec::with_capacity(net_type_map.len());
                for &(code, typ) in net_type_map.keys() {
                    reg_items.push(EapCodeTypeAni { eap_code: code, eap_type: typ });
                }
                Some(reg_items)
            } else {
                None
            }
        };

        if let Some(reg_items) = reg_items {
            NetConnClient::reg_custom_eap_handler(
                RegTriggerMode::Unregister as i32, net_type, &reg_items)?;
        }
        Ok(())
    }

    pub fn on_eap_data(&self, net_type: i32, eap_code: i32, eap_type: i32, data: bridge::EapData) {
        let key = (eap_code, eap_type);
        let inner = self.inner.read().unwrap_or_else(|e| {
            crate::eap_error!("RwLock poisoned in on_eap_data, recovering data");
            e.into_inner()
        });

        if let Some(callback_vec) = inner.get(&net_type).and_then(|map| map.get(&key)) {
            for callback in callback_vec {
                callback.execute((data.clone(),));
            }
        }
    }

    pub fn reg_on_sa_launch(&self, net_type: i32) -> Result<(), i32> {
        let inner = self.inner.read().unwrap_or_else(|e| {
            crate::eap_error!("RwLock poisoned in reg_on_sa_launch, recovering data");
            e.into_inner()
        });

        let net_type_map = match inner.get(&net_type) {
            Some(map) => map,
            None => return Ok(()),
        };

        let mut reg_items = Vec::with_capacity(net_type_map.len());
        for &(code, typ) in net_type_map.keys() {
            reg_items.push(EapCodeTypeAni { eap_code: code, eap_type: typ });
        }
        drop(inner);

        NetConnClient::reg_custom_eap_handler(
            RegTriggerMode::SaLaunch as i32, net_type, &reg_items)
    }
}

pub fn execute_eap_data(data: EapDataAni) {
    let bridge_data = bridge::EapData {
        msg_id: data.msg_id,
        eap_buffer: Uint8Array::new_with_vec(data.eap_buffer),
        buffer_len: data.buffer_len,
    };
    Register::get_instance().on_eap_data(data.net_type, data.eap_code, data.eap_type, bridge_data);
}

pub fn on_sa_status_change(net_type: i32) {
    let _ = Register::get_instance().reg_on_sa_launch(net_type);
}

#[ani_rs::native]
pub(crate) fn reg_custom_eap_handler(
    env: &AniEnv,
    net_type: i32,
    eap_code: i32,
    eap_type: i32,
    callback: AniFnObject,
) -> Result<(), BusinessError> {
    if !NetConnClient::is_eap_feature_enabled() {
        return Ok(());
    }
    let callback = callback
        .into_global_callback(env)
        .map_err(BusinessError::from)?;
    Register::get_instance()
        .register(net_type, eap_code, eap_type, callback)
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
    if !NetConnClient::is_eap_feature_enabled() {
        return Ok(());
    }
    let callback = callback
        .into_global_callback(env)
        .map_err(BusinessError::from)?;
    Register::get_instance()
        .unregister(net_type, eap_code, eap_type, &callback)
        .map_err(convert_to_business_error)
}
