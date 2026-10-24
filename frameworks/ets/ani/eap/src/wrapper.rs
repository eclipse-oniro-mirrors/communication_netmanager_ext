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
use crate::register::{execute_eap_data, execute_eth_eap_state_changed, on_sa_status_change};
use ani_rs::business_error::BusinessError;
use ani_rs::typed_array::Uint8Array;

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

pub(crate) struct NetConnClient;

impl NetConnClient {
    pub(crate) fn is_eap_feature_enabled() -> bool {
        unsafe { ffi::IsEapExtensibleAuthenticationEnabled() }
    }

    pub(crate) fn validate_eap_params(net_type: i32, eap_code: i32, eap_type: i32) -> Result<(), i32> {
        let ret = unsafe { ffi::ValidateEapParams(net_type, eap_code, eap_type) };
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub(crate) fn reg_custom_eap_handler(
        trigger_mode: i32,
        net_type: i32,
        reg_items: &Vec<ffi::EapCodeTypeAni>,
    ) -> Result<(), i32> {
        let ret = unsafe { ffi::RegCustomEapHandler(trigger_mode, net_type, reg_items) };
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub(crate) fn reply_custom_eap_data(
        result: bridge::CustomResult,
        msg_id: i32,
        buffer_len: i32,
        eap_buffer: Vec<u8>,
    ) -> Result<(), i32> {
        let ret = unsafe { ffi::ReplyCustomEapData(result as i32, msg_id, buffer_len, &eap_buffer) };
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub(crate) fn start_eth_eap(net_id: i32, profile: bridge::EthEapProfile) -> Result<(), i32> {
        let profile_ani = ffi::EthEapProfileAni::from(profile);
        let ret = unsafe { ffi::StartEthEap(net_id, profile_ani) };
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub(crate) fn init_sa_listener() {
        unsafe { ffi::InitEapAniSaListener() };
    }

    pub(crate) fn log_off_eth_eap(net_id: i32) -> Result<(), i32> {
        let ret = unsafe { ffi::LogOffEthEap(net_id) };
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }
}

impl From<bridge::EthEapProfile> for ffi::EthEapProfileAni {
    fn from(mut value: bridge::EthEapProfile) -> Self {
        Self {
            eap_method: value.eap_method as i32,
            phase2_method: value.phase2_method as i32,
            identity: value.identity,
            anonymous_identity: value.anonymous_identity,
            password: value.password.clone(),
            ca_cert_aliases: value.ca_cert_aliases,
            ca_path: value.ca_path,
            client_cert_aliases: value.client_cert_aliases,
            cert_entry: value.cert_entry.to_vec(),
            cert_password: value.cert_password.clone(),
            alt_subject_match: value.alt_subject_match,
            domain_suffix_match: value.domain_suffix_match,
            realm: value.realm,
            plmn: value.plmn,
            eap_sub_id: value.eap_sub_id,
        }
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
        1 => bridge::EapMethod::EapPeap,
        2 => bridge::EapMethod::EapTls,
        3 => bridge::EapMethod::EapTtls,
        4 => bridge::EapMethod::EapPwd,
        5 => bridge::EapMethod::EapSim,
        6 => bridge::EapMethod::EapAka,
        7 => bridge::EapMethod::EapAkaPrime,
        8 => bridge::EapMethod::EapUnauthTls,
        _ => bridge::EapMethod::EapNone,
    }
}

fn phase2_method_from_i32(value: i32) -> bridge::Phase2Method {
    match value {
        1 => bridge::Phase2Method::Phase2Pap,
        2 => bridge::Phase2Method::Phase2Mschap,
        3 => bridge::Phase2Method::Phase2Mschapv2,
        4 => bridge::Phase2Method::Phase2Gtc,
        5 => bridge::Phase2Method::Phase2Sim,
        6 => bridge::Phase2Method::Phase2Aka,
        7 => bridge::Phase2Method::Phase2AkaPrime,
        _ => bridge::Phase2Method::Phase2None,
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

impl ffi::EthEapProfileAni {
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
            profile: ffi::EthEapProfileAni::new(),
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
            profile: ffi::EthEapProfileAni {
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
                cert_entry: Uint8Array::new_with_vec(Vec::new()),
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
    struct EthEapProfileAni {
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

    pub struct EapDataAni {
        pub net_type: i32,
        pub eap_code: i32,
        pub eap_type: i32,
        pub msg_id: i32,
        pub buffer_len: i32,
        pub eap_buffer: Vec<u8>,
    }

    pub struct EapCodeTypeAni {
        pub eap_code: i32,
        pub eap_type: i32,
    }

    struct EthEapConfigAni {
        profile: EthEapProfileAni,
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
        fn execute_eap_data(data: EapDataAni);
        fn on_sa_status_change(net_type: i32);
        fn execute_eth_eap_state_changed(state_info: EthEapStateInfoAni);
    }

    unsafe extern "C++" {
        include!("eap_ani.h");

        fn IsEapExtensibleAuthenticationEnabled() -> bool;
        fn ValidateEapParams(net_type: i32, eap_code: i32, eap_type: i32) -> i32;
        fn RegCustomEapHandler(
            trigger_mode: i32,
            net_type: i32,
            reg_items: &Vec<EapCodeTypeAni>,
        ) -> i32;
        fn ReplyCustomEapData(result: i32, msg_id: i32, buffer_len: i32, eap_buffer: &Vec<u8>) -> i32;
        fn StartEthEap(net_id: i32, config: EthEapProfileAni) -> i32;
        fn LogOffEthEap(net_id: i32) -> i32;
        fn InitEapAniSaListener();
        fn SetEthEapConfig(config: EthEapConfigAni) -> i32;
        fn GetEthEapConfig(config: &mut EthEapConfigAni) -> i32;
        fn DeleteEthEapConfig() -> i32;
        fn GetEthEapStateInfo(state_info: &mut EthEapStateInfoAni) -> i32;
        fn ConnectEthEap(connect: bool) -> i32;
        fn RegisterEthEapStateCallback() -> i32;
        fn UnregisterEthEapStateCallback() -> i32;
    }
}
