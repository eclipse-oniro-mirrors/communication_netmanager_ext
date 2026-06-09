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
use crate::register::execute_eap_data;
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

    extern "Rust" {
        fn execute_eap_data(data: EapAniData);
    }

    unsafe extern "C++" {
        include!("eap_ani.h");

        fn RegCustomEapHandler(net_type: i32, eap_code: i32, eap_type: i32) -> i32;
        fn UnregCustomEapHandler(net_type: i32, eap_code: i32, eap_type: i32) -> i32;
        fn ReplyCustomEapData(result: i32, msg_id: i32, buffer_len: i32, eap_buffer: &Vec<u8>) -> i32;
        fn StartEthEap(net_id: i32, config: EthEapConfig) -> i32;
        fn LogOffEthEap(net_id: i32) -> i32;
    }
}
