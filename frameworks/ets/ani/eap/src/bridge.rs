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

use ani_rs::typed_array::ArrayBuffer;

#[ani_rs::ani(path = "@ohos.net.eap.eap.EapMethod")]
#[derive(Debug, Clone, Copy)]
pub enum EapMethod {
    EAP_NONE = 0,
    EAP_PEAP = 1,
    EAP_TLS = 2,
    EAP_TTLS = 3,
    EAP_PWD = 4,
    EAP_SIM = 5,
    EAP_AKA = 6,
    EAP_AKA_PRIME = 7,
    EAP_UNAUTH_TLS = 8,
}

#[ani_rs::ani(path = "@ohos.net.eap.eap.Phase2Method")]
#[derive(Debug, Clone, Copy)]
pub enum Phase2Method {
    PHASE2_NONE = 0,
    PHASE2_PAP = 1,
    PHASE2_MSCHAP = 2,
    PHASE2_MSCHAPV2 = 3,
    PHASE2_GTC = 4,
    PHASE2_SIM = 5,
    PHASE2_AKA = 6,
    PHASE2_AKA_PRIME = 7,
}

#[ani_rs::ani(path = "@ohos.net.eap.eap.CustomResult")]
#[derive(Debug, Clone, Copy)]
pub enum CustomResult {
    RESULT_FAIL = 0,
    RESULT_NEXT = 1,
    RESULT_FINISH = 2,
}

#[ani_rs::ani(path = "@ohos.net.eap.eap.EapDataInner")]
#[derive(Clone)]
pub struct EapData {
    pub msg_id: i32,
    pub eap_buffer: ArrayBuffer,
    pub buffer_len: i32,
}

#[ani_rs::ani(path = "@ohos.net.eap.eap.EthEapProfileInner")]
#[derive(Clone)]
pub struct EthEapProfile {
    pub eap_method: EapMethod,
    pub phase2_method: Phase2Method,
    pub identity: String,
    pub anonymous_identity: String,
    /// Sensitive: zeroized after FFI call in wrapper, do not log or expose
    pub password: String,
    pub ca_cert_aliases: String,
    pub ca_path: String,
    pub client_cert_aliases: String,
    pub cert_entry: ArrayBuffer,
    /// Sensitive: zeroized after FFI call in wrapper, do not log or expose
    pub cert_password: String,
    pub alt_subject_match: String,
    pub domain_suffix_match: String,
    pub realm: String,
    pub plmn: String,
    pub eap_sub_id: i32,
}
