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
use crate::bridge::{EthernetDeviceInfos, HttpProxy, InterfaceConfiguration, MacAddressInfo};
use ffi::{HttpProxyAni, InterfaceConfigurationAni, EthernetDeviceInfosAni, MacAddressInfoAni};
use crate::register::{execute_interface_added, execute_interface_removed, execute_interface_changed};

pub struct EthernetClient;

impl EthernetClient {
    pub fn is_ethernet_enabled() -> Result<bool, i32> {
        let mut enabled = false;
        let ret = unsafe { ffi::IsEthernetEnabled(&mut enabled) };
        if ret != 0 {
            return Err(ret);
        }
        Ok(enabled)
    }

    pub fn disable_ethernet_interface() -> Result<(), i32> {
        let ret = unsafe { ffi::DisableEthernetInterface() };
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn enable_ethernet_interface() -> Result<(), i32> {
        let ret = unsafe { ffi::EnableEthernetInterface() };
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn get_all_active_ifaces() -> Result<Vec<String>, i32> {
        let mut ifaces = Vec::new();
        let ret = unsafe { ffi::g_getAllActiveIfaces(&mut ifaces) };
        if ret != 0 {
            return Err(ret);
        }
        Ok(ifaces)
    }

    pub fn is_iface_active(iface: String) -> Result<i32, i32> {
        let mut active_status = 0;
        let ret = unsafe { ffi::IsIfaceActive(&iface, &mut active_status) };
        if ret != 0 {
            return Err(ret);
        }
        Ok(active_status)
    }

    pub fn get_iface_config(iface: String) -> Result<InterfaceConfiguration, i32> {
        let mut iface_config = InterfaceConfigurationAni::new();
        let ret = unsafe { ffi::GetIfaceConfig(&iface, &mut iface_config) };
        if ret != 0 {
            return Err(ret);
        }
        Ok(iface_config.into())
    }

    pub fn set_iface_config(iface: String, config: InterfaceConfiguration) -> Result<(), i32> {
        let iface_config = InterfaceConfigurationAni::from(config);
        let ret = unsafe { ffi::SetIfaceConfig(&iface, &iface_config) };
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn get_device_information() -> Result<Vec<EthernetDeviceInfos>, i32> {
        let mut device_infos = Vec::new();
        let ret = unsafe { ffi::GetDeviceInformation(&mut device_infos) };
        if ret != 0 {
            return Err(ret);
        }
        Ok(device_infos.into_iter().map(Into::into).collect())
    }

    pub fn get_mac_address() -> Result<Vec<MacAddressInfo>, i32> {
        let mut mac_addrs = Vec::new();
        let ret = unsafe { ffi::GetMacAddress(&mut mac_addrs) };
        if ret != 0 {
            return Err(ret);
        }
        Ok(mac_addrs.into_iter().map(Into::into).collect())
    }

    pub fn on_interface_state_change() -> Result<(), i32> {
        let ret = unsafe { ffi::RegisterInterfaceStateCallback() };
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn off_interface_state_change() -> Result<(), i32> {
        let ret = unsafe { ffi::UnregisterInterfaceStateCallback() };
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }
}

pub fn convert_to_business_error(mut code: i32) -> BusinessError {
    let error_msg = ffi::GetErrorCodeAndMessage(&mut code);
    BusinessError::new(code, error_msg)
}

impl HttpProxyAni {
    fn new() -> Self {
        Self {
            host: String::new(),
            port: 0,
            exclusion_list: Vec::new(),
            username: String::new(),
            password: String::new(),
        }
    }
}

impl From<ffi::HttpProxyAni> for HttpProxy {
    fn from(value: ffi::HttpProxyAni) -> Self {
        Self {
            host: value.host,
            port: value.port,
            exclusion_list: value.exclusion_list,
            username: if value.username.is_empty() { None } else { Some(value.username) },
            password: if value.password.is_empty() { None } else { Some(value.password) },
        }
    }
}

impl From<HttpProxy> for ffi::HttpProxyAni {
    fn from(value: HttpProxy) -> Self {
        Self {
            host: value.host,
            port: value.port,
            exclusion_list: value.exclusion_list,
            username: value.username.unwrap_or_default(),
            password: value.password.unwrap_or_default(),
        }
    }
}

impl InterfaceConfigurationAni {
    pub fn new() -> Self {
        Self {
            mode: 0,
            ip_addr: String::new(),
            route: String::new(),
            gateway: String::new(),
            net_mask: String::new(),
            dns_servers: String::new(),
            http_proxy: HttpProxyAni::new(),
        }
    }
}

impl From<InterfaceConfiguration> for InterfaceConfigurationAni {
    fn from(value: InterfaceConfiguration) -> Self {
        Self {
            mode: value.mode.into(),
            ip_addr: value.ip_addr,
            route: value.route,
            gateway: value.gateway,
            net_mask: value.net_mask,
            dns_servers: value.dns_servers,
            http_proxy: match value.http_proxy {
                Some(proxy) => proxy.into(),
                None => HttpProxyAni::new(),
            },
        }
    }
}

impl From<InterfaceConfigurationAni> for InterfaceConfiguration {
    fn from(value: ffi::InterfaceConfigurationAni) -> Self {
        Self {
            mode: value.mode.into(),
            ip_addr: value.ip_addr,
            route: value.route,
            gateway: value.gateway,
            net_mask: value.net_mask,
            dns_servers: value.dns_servers,
            http_proxy: if value.http_proxy.host.is_empty() { None } else { Some(value.http_proxy.into()) },
        }
    }
}

impl EthernetDeviceInfosAni {
    pub fn new() -> Self {
        Self {
            iface_name: String::new(),
            device_name: String::new(),
            connection_mode: 0,
            supplier_name: String::new(),
            supplier_id: String::new(),
            product_name: String::new(),
            maximum_rate: String::new(),
        }
    }
}

impl From<EthernetDeviceInfosAni> for EthernetDeviceInfos {
    fn from(value: EthernetDeviceInfosAni) -> Self {
        Self {
            iface_name: value.iface_name,
            device_name: value.device_name,
            connection_mode: value.connection_mode.into(),
            supplier_name: value.supplier_name,
            supplier_id: value.supplier_id,
            product_name: value.product_name,
            maximum_rate: value.maximum_rate,
        }
    }
}

impl MacAddressInfoAni {
    pub fn new() -> Self {
        Self {
            iface: String::new(),
            mac_address: String::new(),
        }
    }
}

impl From<MacAddressInfoAni> for MacAddressInfo {
    fn from(value: MacAddressInfoAni) -> Self {
        Self {
            iface: value.iface,
            mac_address: value.mac_address,
        }
    }
}

#[cxx::bridge(namespace = "OHOS::NetManagerAni")]
pub(crate) mod ffi {
    pub struct InterfaceStateInfoAni {
        pub iface: String,
        pub active: bool,
    }

    struct HttpProxyAni {
        pub host: String,
        pub port: i32,
        pub exclusion_list: Vec<String>,
        pub username: String,
        pub password: String,
    }

    struct InterfaceConfigurationAni {
        pub mode: i32,
        pub ip_addr: String,
        pub route: String,
        pub gateway: String,
        pub net_mask: String,
        pub dns_servers: String,
        pub http_proxy: HttpProxyAni,
    }

    struct EthernetDeviceInfosAni {
        pub iface_name: String,
        pub device_name: String,
        pub connection_mode: i32,
        pub supplier_name: String,
        pub supplier_id: String,
        pub product_name: String,
        pub maximum_rate: String,
    }

    struct MacAddressInfoAni {
        pub iface: String,
        pub mac_address: String,
    }

    extern "Rust" {
        fn execute_interface_added(iface: String);
        fn execute_interface_removed(iface: String);
        fn execute_interface_changed(info: InterfaceStateInfoAni);
    }

    unsafe extern "C++" {
        include!("ethernet_ani.h");

        fn IsEthernetEnabled(enabled: &mut bool) -> i32;
        fn DisableEthernetInterface() -> i32;
        fn EnableEthernetInterface() -> i32;
        fn g_getAllActiveIfaces(active_ifaces: &mut Vec<String>) -> i32;
        fn IsIfaceActive(iface: &String, active_status: &mut i32) -> i32;
        fn GetIfaceConfig(iface: &String, iface_config: &mut InterfaceConfigurationAni) -> i32;
        fn SetIfaceConfig(iface: &String, iface_config: &InterfaceConfigurationAni) -> i32;
        fn GetDeviceInformation(device_infos: &mut Vec<EthernetDeviceInfosAni>) -> i32;
        fn GetMacAddress(mac_addrs: &mut Vec<MacAddressInfoAni>) -> i32;
        fn RegisterInterfaceStateCallback() -> i32;
        fn UnregisterInterfaceStateCallback() -> i32;
        fn GetErrorCodeAndMessage(error_code: &mut i32) -> String;
    }
}
