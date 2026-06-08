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

use std::os::unix::io::RawFd;

use ani_rs::business_error::BusinessError;

use crate::{
    bridge,
    wrapper::{convert_to_business_error, VpnClient},
};

#[ani_rs::native]
pub fn create_vpn_connection() -> Result<i32, BusinessError> {
    VpnClient::create_vpn_connection().map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn set_up(config: bridge::VpnConfig) -> Result<i32, BusinessError> {
    VpnClient::set_up(config).map(|fd: RawFd| fd as i32).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn protect(socket_fd: i32) -> Result<i32, BusinessError> {
    VpnClient::protect(socket_fd).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn destroy() -> Result<i32, BusinessError> {
    VpnClient::destroy().map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn add_sys_vpn_config(config: bridge::SysVpnConfig) -> Result<i32, BusinessError> {
    VpnClient::add_sys_vpn_config(config).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn delete_sys_vpn_config(vpn_id: String) -> Result<i32, BusinessError> {
    if vpn_id.is_empty() {
        return Err(convert_to_business_error(401));
    }
    VpnClient::delete_sys_vpn_config(vpn_id).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_sys_vpn_config_list() -> Result<Vec<bridge::SysVpnConfig>, BusinessError> {
    VpnClient::get_sys_vpn_config_list().map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_sys_vpn_config(vpn_id: String) -> Result<bridge::SysVpnConfig, BusinessError> {
    if vpn_id.is_empty() {
        return Err(convert_to_business_error(401));
    }
    VpnClient::get_sys_vpn_config(vpn_id).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_connected_sys_vpn_config() -> Result<bridge::SysVpnConfig, BusinessError> {
    VpnClient::get_connected_sys_vpn_config().map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_connected_vpn_app_info() -> Result<Vec<String>, BusinessError> {
    VpnClient::get_connected_vpn_app_info().map_err(convert_to_business_error)
}
