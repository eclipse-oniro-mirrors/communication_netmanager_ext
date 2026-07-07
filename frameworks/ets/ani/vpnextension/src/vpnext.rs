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

use crate::{
    bridge,
    wrapper::{convert_to_business_error, VpnExtClient},
};

#[ani_rs::native(name = "startVpnExtensionAbilitySync")]
pub fn start_vpn_extension_ability(bundle_name: String, ability_name: String) -> Result<i32, BusinessError> {
    VpnExtClient::start_vpn_extension_ability(bundle_name, ability_name)
        .map_err(convert_to_business_error)
}

#[ani_rs::native(name = "stopVpnExtensionAbilitySync")]
pub fn stop_vpn_extension_ability(bundle_name: String, ability_name: String) -> Result<i32, BusinessError> {
    VpnExtClient::stop_vpn_extension_ability(bundle_name, ability_name)
        .map_err(convert_to_business_error)
}

#[ani_rs::native(name = "setAlwaysOnVpnEnabledSync")]
pub fn set_always_on_vpn_enabled(enable: bool, bundle_name: String) -> Result<i32, BusinessError> {
    VpnExtClient::set_always_on_vpn_enabled(enable, bundle_name)
        .map_err(convert_to_business_error)
}

#[ani_rs::native(name = "isAlwaysOnVpnEnabledSync")]
pub fn is_always_on_vpn_enabled(bundle_name: String) -> Result<bool, BusinessError> {
    VpnExtClient::is_always_on_vpn_enabled(bundle_name)
        .map_err(convert_to_business_error)
}

#[ani_rs::native(name = "updateVpnAuthorizedStateSync")]
pub fn update_vpn_authorized_state(bundle_name: String) -> Result<bool, BusinessError> {
    VpnExtClient::update_vpn_authorized_state(bundle_name)
        .map_err(convert_to_business_error)
}

#[ani_rs::native(name = "createVpnConnectionSync")]
pub fn create_vpn_connection() -> Result<i32, BusinessError> {
    VpnExtClient::create_vpn_connection().map_err(convert_to_business_error)
}

#[ani_rs::native(name = "createSync")]
pub fn create(config: bridge::VpnConfig) -> Result<i32, BusinessError> {
    VpnExtClient::create(config).map_err(convert_to_business_error)
}

#[ani_rs::native(name = "protectSync")]
pub fn protect(socket_fd: i32) -> Result<i32, BusinessError> {
    VpnExtClient::protect(socket_fd).map_err(convert_to_business_error)
}

#[ani_rs::native(name = "destroySync")]
pub fn destroy() -> Result<i32, BusinessError> {
    VpnExtClient::destroy().map_err(convert_to_business_error)
}

#[ani_rs::native(name = "destroyVpnSync")]
pub fn destroy_by_vpn_id(vpn_id: String) -> Result<i32, BusinessError> {
    VpnExtClient::destroy_by_vpn_id(vpn_id).map_err(convert_to_business_error)
}

#[ani_rs::native(name = "protectProcessNetSync")]
pub fn protect_process_net() -> Result<i32, BusinessError> {
    VpnExtClient::protect_process_net().map_err(convert_to_business_error)
}

#[ani_rs::native(name = "generateVpnIdSync")]
pub fn generate_vpn_id() -> Result<String, BusinessError> {
    VpnExtClient::generate_vpn_id().map_err(convert_to_business_error)
}
