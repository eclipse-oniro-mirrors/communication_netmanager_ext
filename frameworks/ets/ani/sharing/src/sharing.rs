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

use ani_rs::business_error::BusinessError;

use crate::{bridge, wrapper::SharingClient};

#[ani_rs::native]
pub fn is_sharing_supported() -> Result<bool, BusinessError> {
    SharingClient::is_sharing_supported().map_err(|err| {
        BusinessError::new(err as i32, "Failed to check sharing support".to_string())
    })
}

#[ani_rs::native]
pub fn is_sharing() -> Result<bool, BusinessError> {
    SharingClient::is_sharing()
        .map_err(|err| BusinessError::new(err as i32, "Failed to check sharing status".to_string()))
}

#[ani_rs::native]
pub fn start_sharing(share_type: bridge::SharingIfaceType) -> Result<i32, BusinessError> {
    SharingClient::start_sharing(share_type)
        .map_err(|err| BusinessError::new(err as i32, "Failed to start sharing".to_string()))
}

#[ani_rs::native]
pub fn stop_sharing(share_type: bridge::SharingIfaceType) -> Result<i32, BusinessError> {
    SharingClient::stop_sharing(share_type)
        .map_err(|err| BusinessError::new(err as i32, "Failed to stop sharing".to_string()))
}

#[ani_rs::native]
pub fn get_stats_rx_bytes() -> Result<i32, BusinessError> {
    SharingClient::get_stats_rx_bytes()
        .map_err(|err| BusinessError::new(err as i32, "Failed to get RX bytes".to_string()))
}

#[ani_rs::native]
pub fn get_stats_tx_bytes() -> Result<i32, BusinessError> {
    SharingClient::get_stats_tx_bytes()
        .map_err(|err| BusinessError::new(err as i32, "Failed to get TX bytes".to_string()))
}

#[ani_rs::native]
pub fn get_stats_total_bytes() -> Result<i32, BusinessError> {
    SharingClient::get_stats_total_bytes()
        .map_err(|err| BusinessError::new(err as i32, "Failed to get total bytes".to_string()))
}

#[ani_rs::native]
pub fn get_sharing_ifaces(state: bridge::SharingIfaceState) -> Result<Vec<String>, BusinessError> {
    SharingClient::get_sharing_ifaces(state).map_err(|err| {
        BusinessError::new(err as i32, "Failed to get sharing interface".to_string())
    })
}

#[ani_rs::native]
pub fn get_sharing_state(
    share_type: bridge::SharingIfaceType,
) -> Result<bridge::SharingIfaceState, BusinessError> {
    SharingClient::get_sharing_state(share_type)
        .map_err(|err| BusinessError::new(err as i32, "Failed to get sharing state".to_string()))
}

#[ani_rs::native]
pub fn get_sharable_regexs(
    share_type: bridge::SharingIfaceType,
) -> Result<Vec<String>, BusinessError> {
    SharingClient::get_sharable_regexs(share_type)
        .map_err(|err| BusinessError::new(err as i32, "Failed to get sharable regexs".to_string()))
}
