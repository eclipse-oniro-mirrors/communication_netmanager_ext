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

use crate::bridge;
use crate::register::{execute_eap_data, execute_eth_eap_state_changed};
use ani_rs::business_error::BusinessError;
use ani_rs::typed_array::ArrayBuffer;

pub(crate) fn convert_to_business_error(err: i32) -> BusinessError {
    BusinessError::new(err, String::new())
}

/// Securely zeroize a String's buffer in memory before clearing.
/// Overwrites all bytes with zeros then clears and shrinks, ensuring
/// plaintext does not linger in freed memory.
fn zeroize_string(s: &mut String) {
    if !s.is_empty() {
        unsafe {
            std::ptr::write_bytes(s.as_mut_ptr(), 0u8, s.len());
        }
    }
    s.clear();
    s.shrink_to_fit();
}

/// Securely zeroize a Vec's buffer in memory before clearing.
fn zeroize_vec<T>(v: &mut Vec<T>) {
    if !v.is_empty() {
        unsafe {
            std::ptr::write_bytes(v.as_mut_ptr(), 0u8, v.len() * std::mem::size_of::<T>());
        }
    }
    v.clear();
    v.shrink_to_fit();
}

pub(crate) struct NetConnClient;

impl NetConnClient {
    pub(crate) fn reg_custom_eap_handler(net_type: i32, eap_code: i32, eap_type: i32) -> Result<(), i32> {
        let ret = unsafe { ffi::RegCustomEapHandler(net_type, eap_code, eap_type) };
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub(crate) fn unreg_custom_eap_handler(net_type: i32, eap_code: i32, eap_type: i32) -> Result<(), i32> {
        let ret = unsafe { ffi::UnregCustomEapHandler(net_type, eap_code, eap_type) };
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub(crate) fn reply_custom_eap_data(
        result: bridge::CustomResult,
        msg_id: i32,
        buffer_len: i32,
        eap_buffer: ArrayBuffer,
    ) -> Result<(), i32> {
        let eap_buffer_vec: Vec<u8> = eap_buffer.to_vec();
        if buffer_len < 0 || buffer_len as usize != eap_buffer_vec.len() {
            return Err(-1);
        }
        let ret = unsafe { ffi::ReplyCustomEapData(result as i32, msg_id, buffer_len, &eap_buffer_vec) };
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub(crate) fn start_eth_eap(net_id: i32, mut profile: bridge::EthEapProfile) -> Result<(), i32> {
        // Sensitive fields (password, cert_password) are cloned into the FFI config;
        // C++ side zeros them after use with memset_s. After the FFI call, we also
        // zeroize the Rust-side originals to prevent plaintext lingering in memory.
        let config = ffi::EthEapConfig {
            eap_method: profile.eap_method as i32,
            phase2_method: profile.phase2_method as i32,
            identity: profile.identity,
            anonymous_identity: profile.anonymous_identity,
            password: profile.password.clone(),
            ca_cert_aliases: profile.ca_cert_aliases,
            ca_path: profile.ca_path,
            client_cert_aliases: profile.client_cert_aliases,
            cert_entry: profile.cert_entry.to_vec(),
            cert_password: profile.cert_password.clone(),
            alt_subject_match: profile.alt_subject_match,
            domain_suffix_match: profile.domain_suffix_match,
            realm: profile.realm,
            plmn: profile.plmn,
            eap_sub_id: profile.eap_sub_id,
        };
        let ret = unsafe { ffi::StartEthEap(net_id, config) };

        // Zeroize sensitive fields on the Rust side after FFI transfer completes
        zeroize_string(&mut profile.password);
        zeroize_string(&mut profile.cert_password);

        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub(crate) fn log_off_eth_eap(net_id: i32) -> Result<(), i32> {
        let ret = unsafe { ffi::LogOffEthEap(net_id) };
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }
}

const NETMANAGER_EXT_ERR_FILE_NOT_FOUND: i32 = 2200209;

impl NetConnClient {
    pub(crate) fn set_eth_eap_config(config: bridge::EthEapConfig) -> Result<(), i32> {
        let ret = unsafe { ffi::SetEthEapConfig(config.into()) };
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub(crate) fn get_eth_eap_config() -> Result<bridge::EthEapConfig, i32> {
        let mut config = ffi::EthEapConfigAni::new();
        let ret = unsafe { ffi::GetEthEapConfig(&mut config) };
        if ret == NETMANAGER_EXT_ERR_FILE_NOT_FOUND {
            return Ok(ffi::EthEapConfigAni::new().into());
        }
        if ret != 0 {
            return Err(ret);
        }
        Ok(config.into())
    }

    pub(crate) fn delete_eth_eap_config() -> Result<(), i32> {
        let ret = unsafe { ffi::DeleteEthEapConfig() };
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub(crate) fn get_eth_eap_state_info() -> Result<bridge::EthEapStateInfo, i32> {
        let mut state = ffi::EthEapStateInfoAni::new();
        let ret = unsafe { ffi::GetEthEapStateInfo(&mut state) };
        if ret != 0 {
            return Err(ret);
        }
        Ok(state.into())
    }

    pub(crate) fn connect_eth_eap(action: bridge::EthEapConnectAction) -> Result<(), i32> {
        let connect = matches!(action, bridge::EthEapConnectAction::ACTION_CONNECT);
        let ret = unsafe { ffi::ConnectEthEap(connect) };
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub(crate) fn register_eth_eap_state_callback() -> Result<(), i32> {
        let ret = unsafe { ffi::RegisterEthEapStateCallback() };
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub(crate) fn unregister_eth_eap_state_callback() -> Result<(), i32> {
        let ret = unsafe { ffi::UnregisterEthEapStateCallback() };
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }
}

fn eap_method_from_i32(value: i32) -> bridge::EapMethod {
    match value {
        1 => bridge::EapMethod::EAP_PEAP,
        2 => bridge::EapMethod::EAP_TLS,
        3 => bridge::EapMethod::EAP_TTLS,
        4 => bridge::EapMethod::EAP_PWD,
        5 => bridge::EapMethod::EAP_SIM,
        6 => bridge::EapMethod::EAP_AKA,
        7 => bridge::EapMethod::EAP_AKA_PRIME,
        8 => bridge::EapMethod::EAP_UNAUTH_TLS,
        _ => bridge::EapMethod::EAP_NONE,
    }
}

fn phase2_method_from_i32(value: i32) -> bridge::Phase2Method {
    match value {
        1 => bridge::Phase2Method::PHASE2_PAP,
        2 => bridge::Phase2Method::PHASE2_MSCHAP,
        3 => bridge::Phase2Method::PHASE2_MSCHAPV2,
        4 => bridge::Phase2Method::PHASE2_GTC,
        5 => bridge::Phase2Method::PHASE2_SIM,
        6 => bridge::Phase2Method::PHASE2_AKA,
        7 => bridge::Phase2Method::PHASE2_AKA_PRIME,
        _ => bridge::Phase2Method::PHASE2_NONE,
    }
}

fn eth_eap_state_from_i32(value: i32) -> bridge::EthEapState {
    match value {
        1 => bridge::EthEapState::STATE_AUTHENTICATING,
        2 => bridge::EthEapState::STATE_AUTHENTICATED,
        3 => bridge::EthEapState::STATE_FAILED,
        4 => bridge::EthEapState::STATE_RETRYING,
        _ => bridge::EthEapState::STATE_IDLE,
    }
}

impl ffi::EthEapConfig {
    pub fn new() -> Self {
        Self {
            eap_method: 0,
            phase2_method: 0,
            identity: String::new(),
            anonymous_identity: String::new(),
            password: String::new(),
            ca_cert_aliases: String::new(),
            ca_path: String::new(),
            client_cert_aliases: String::new(),
            cert_entry: Vec::new(),
            cert_password: String::new(),
            alt_subject_match: String::new(),
            domain_suffix_match: String::new(),
            realm: String::new(),
            plmn: String::new(),
            eap_sub_id: 0,
        }
    }
}

impl ffi::EthEapConfigAni {
    pub fn new() -> Self {
        Self {
            profile: ffi::EthEapConfig::new(),
            enabled: false,
            auto_auth: false,
            locked: false,
        }
    }
}

impl ffi::EthEapStateInfoAni {
    pub fn new() -> Self {
        Self {
            state: 0,
            retry_count: 0,
            message: String::new(),
        }
    }
}

impl From<bridge::EthEapConfig> for ffi::EthEapConfigAni {
    fn from(value: bridge::EthEapConfig) -> Self {
        let mut profile = value.profile;
        let config = Self {
            profile: ffi::EthEapConfig {
                eap_method: profile.eap_method as i32,
                phase2_method: profile.phase2_method as i32,
                identity: profile.identity,
                anonymous_identity: profile.anonymous_identity,
                password: profile.password.clone(),
                ca_cert_aliases: profile.ca_cert_aliases,
                ca_path: profile.ca_path,
                client_cert_aliases: profile.client_cert_aliases,
                cert_entry: profile.cert_entry.to_vec(),
                cert_password: profile.cert_password.clone(),
                alt_subject_match: profile.alt_subject_match,
                domain_suffix_match: profile.domain_suffix_match,
                realm: profile.realm,
                plmn: profile.plmn,
                eap_sub_id: profile.eap_sub_id,
            },
            enabled: value.enabled,
            auto_auth: value.auto_auth,
            locked: value.locked,
        };
        zeroize_string(&mut profile.password);
        zeroize_string(&mut profile.cert_password);
        config
    }
}

impl From<ffi::EthEapConfigAni> for bridge::EthEapConfig {
    fn from(value: ffi::EthEapConfigAni) -> Self {
        let profile = value.profile;
        Self {
            profile: bridge::EthEapProfile {
                eap_method: eap_method_from_i32(profile.eap_method),
                phase2_method: phase2_method_from_i32(profile.phase2_method),
                identity: profile.identity,
                anonymous_identity: profile.anonymous_identity,
                password: String::new(),
                ca_cert_aliases: profile.ca_cert_aliases,
                ca_path: profile.ca_path,
                client_cert_aliases: profile.client_cert_aliases,
                cert_entry: ArrayBuffer::new_with_vec(Vec::new()),
                cert_password: String::new(),
                alt_subject_match: profile.alt_subject_match,
                domain_suffix_match: profile.domain_suffix_match,
                realm: profile.realm,
                plmn: profile.plmn,
                eap_sub_id: profile.eap_sub_id,
            },
            enabled: value.enabled,
            auto_auth: value.auto_auth,
            locked: value.locked,
        }
    }
}

impl From<ffi::EthEapStateInfoAni> for bridge::EthEapStateInfo {
    fn from(value: ffi::EthEapStateInfoAni) -> Self {
        Self {
            state: eth_eap_state_from_i32(value.state),
            retry_count: value.retry_count,
            message: value.message,
        }
    }
}

#[cxx::bridge(namespace = "OHOS::NetManagerAni")]
pub(crate) mod ffi {
    struct EthEapConfig {
        eap_method: i32,
        phase2_method: i32,
        identity: String,
        anonymous_identity: String,
        password: String,
        ca_cert_aliases: String,
        ca_path: String,
        client_cert_aliases: String,
        cert_entry: Vec<u8>,
        cert_password: String,
        alt_subject_match: String,
        domain_suffix_match: String,
        realm: String,
        plmn: String,
        eap_sub_id: i32,
    }

    pub struct EapAniData {
        pub msg_id: i32,
        pub buffer_len: i32,
        pub eap_buffer: Vec<u8>,
    }

    struct EthEapConfigAni {
        profile: EthEapConfig,
        enabled: bool,
        auto_auth: bool,
        locked: bool,
    }

    pub struct EthEapStateInfoAni {
        pub state: i32,
        pub retry_count: i32,
        pub message: String,
    }

    extern "Rust" {
        fn execute_eap_data(data: EapAniData);
        fn execute_eth_eap_state_changed(state_info: EthEapStateInfoAni);
    }

    unsafe extern "C++" {
        include!("eap_ani.h");

        fn RegCustomEapHandler(net_type: i32, eap_code: i32, eap_type: i32) -> i32;
        fn UnregCustomEapHandler(net_type: i32, eap_code: i32, eap_type: i32) -> i32;
        fn ReplyCustomEapData(result: i32, msg_id: i32, buffer_len: i32, eap_buffer: &Vec<u8>) -> i32;
        fn StartEthEap(net_id: i32, config: EthEapConfig) -> i32;
        fn LogOffEthEap(net_id: i32) -> i32;
        fn SetEthEapConfig(config: EthEapConfigAni) -> i32;
        fn GetEthEapConfig(config: &mut EthEapConfigAni) -> i32;
        fn DeleteEthEapConfig() -> i32;
        fn GetEthEapStateInfo(state_info: &mut EthEapStateInfoAni) -> i32;
        fn ConnectEthEap(connect: bool) -> i32;
        fn RegisterEthEapStateCallback() -> i32;
        fn UnregisterEthEapStateCallback() -> i32;
    }
}
