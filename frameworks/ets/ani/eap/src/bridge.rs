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

use ani_rs::typed_array::Uint8Array;

#[ani_rs::ani(path = "@ohos.net.eap.eap.EapMethod")]
#[derive(Debug, Clone, Copy)]
pub enum EapMethod {
    EapNone = 0,
    EapPeap = 1,
    EapTls = 2,
    EapTtls = 3,
    EapPwd = 4,
    EapSim = 5,
    EapAka = 6,
    EapAkaPrime = 7,
    EapUnauthTls = 8,
}

#[ani_rs::ani(path = "@ohos.net.eap.eap.Phase2Method")]
#[derive(Debug, Clone, Copy)]
pub enum Phase2Method {
    Phase2None = 0,
    Phase2Pap = 1,
    Phase2Mschap = 2,
    Phase2Mschapv2 = 3,
    Phase2Gtc = 4,
    Phase2Sim = 5,
    Phase2Aka = 6,
    Phase2AkaPrime = 7,
}

#[ani_rs::ani(path = "@ohos.net.eap.eap.CustomResult")]
#[derive(Debug, Clone, Copy)]
pub enum CustomResult {
    ResultFail = 0,
    ResultNext = 1,
    ResultFinish = 2,
}

#[ani_rs::ani(path = "@ohos.net.eap.eap.EapDataInner")]
#[derive(Clone)]
pub struct EapData {
    pub msg_id: i32,
    pub eap_buffer: Uint8Array,
    pub buffer_len: i32,
}

#[ani_rs::ani(path = "@ohos.net.eap.eap.EthEapState")]
#[derive(Debug, Clone, Copy)]
pub enum EthEapState {
    STATE_IDLE = 0,
    STATE_AUTHENTICATING = 1,
    STATE_AUTHENTICATED = 2,
    STATE_FAILED = 3,
    STATE_RETRYING = 4,
}

#[ani_rs::ani(path = "@ohos.net.eap.eap.EthEapConnectAction")]
#[derive(Debug, Clone, Copy)]
pub enum EthEapConnectAction {
    ACTION_CONNECT = 0,
    ACTION_DISCONNECT = 1,
}

#[ani_rs::ani(path = "@ohos.net.eap.eap.EthEapStateInfoInner")]
#[derive(Clone)]
pub struct EthEapStateInfo {
    pub state: EthEapState,
    pub retry_count: i32,
    pub message: String,
}

#[ani_rs::ani(path = "@ohos.net.eap.eap.EthEapConfigInner")]
#[derive(Clone)]
pub struct EthEapConfig {
    pub profile: EthEapProfile,
    pub enabled: bool,
    pub auto_auth: bool,
    pub locked: bool,
}

#[ani_rs::ani(path = "@ohos.net.eap.eap.EthEapProfileInner")]
#[derive(Clone)]
pub struct EthEapProfile {
    pub eap_method: EapMethod,
    pub phase2_method: Phase2Method,
    pub identity: String,
    pub anonymous_identity: String,
    pub password: String,
    pub ca_cert_aliases: String,
    pub ca_path: String,
    pub client_cert_aliases: String,
    pub cert_entry: Uint8Array,
    pub cert_password: String,
    pub alt_subject_match: String,
    pub domain_suffix_match: String,
    pub realm: String,
    pub plmn: String,
    pub eap_sub_id: i32,
}
