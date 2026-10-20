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
use crate::register::{execute_eap_data, on_sa_status_change};
use ani_rs::business_error::BusinessError;

pub(crate) fn convert_to_business_error(err: i32) -> BusinessError {
    BusinessError::new(err, String::new())
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

    extern "Rust" {
        fn execute_eap_data(data: EapDataAni);
        fn on_sa_status_change(net_type: i32);
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
    }
}
