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

use ani_rs::business_error::BusinessError;
use crate::bridge;
use crate::wrapper::{convert_to_business_error, NetConnClient};

#[ani_rs::native]
pub(crate) fn reply_custom_eap_data(
    result: bridge::CustomResult,
    data: bridge::EapData,
) -> Result<(), BusinessError> {
    if data.buffer_len < 0 || data.buffer_len as usize != data.eap_buffer.len() {
        return Err(BusinessError::new(-1, "buffer length mismatch".to_string()));
    }
    NetConnClient::reply_custom_eap_data(result, data.msg_id, data.buffer_len, data.eap_buffer)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn start_eth_eap(net_id: i32, profile: bridge::EthEapProfile) -> Result<(), BusinessError> {
    NetConnClient::start_eth_eap(net_id, profile).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn log_off_eth_eap(net_id: i32) -> Result<(), BusinessError> {
    NetConnClient::log_off_eth_eap(net_id).map_err(convert_to_business_error)
}
