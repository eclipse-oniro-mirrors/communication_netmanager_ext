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

use crate::bridge::{EthernetDeviceInfos, InterfaceConfiguration, MacAddressInfo};
use crate::wrapper::{convert_to_business_error, EthernetClient};

#[ani_rs::native]
pub fn is_ethernet_enabled() -> Result<bool, BusinessError> {
    EthernetClient::is_ethernet_enabled().map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn disable_ethernet_interface() -> Result<(), BusinessError> {
    EthernetClient::disable_ethernet_interface().map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn enable_ethernet_interface() -> Result<(), BusinessError> {
    EthernetClient::enable_ethernet_interface().map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_all_active_ifaces() -> Result<Vec<String>, BusinessError> {
    EthernetClient::get_all_active_ifaces().map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn is_iface_active(iface: String) -> Result<i32, BusinessError> {
    EthernetClient::is_iface_active(iface).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_iface_config(iface: String) -> Result<InterfaceConfiguration, BusinessError> {
    EthernetClient::get_iface_config(iface).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn set_iface_config(iface: String, config: InterfaceConfiguration) -> Result<(), BusinessError> {
    EthernetClient::set_iface_config(iface, config).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_ethernet_device_infos() -> Result<Vec<EthernetDeviceInfos>, BusinessError> {
    EthernetClient::get_device_information().map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_mac_address() -> Result<Vec<MacAddressInfo>, BusinessError> {
    EthernetClient::get_mac_address().map_err(convert_to_business_error)
}
