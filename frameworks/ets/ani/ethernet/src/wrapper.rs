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
use crate::register::{execute_interface_added, execute_interface_removed, execute_interface_changed};

/// Standard error code for internal operation failures.
/// Avoids hard-coded -1 which is inconsistent with FFI error codes.
const ERR_INTERNAL: i32 = 2100003;
/// Standard error code for invalid parameter / parsing failures.
const ERR_INVALID_PARAMETER: i32 = 2100001;

/// Unescape a field produced by C++ EscapeField (which escapes `,` and `;`
/// as `\,` and `\;`).
fn unescape_field(s: &str) -> String {
    let mut result = String::with_capacity(s.len());
    let chars: Vec<char> = s.chars().collect();
    let mut i = 0;
    while i < chars.len() {
        if chars[i] == '\\' && i + 1 < chars.len() && (chars[i + 1] == ',' || chars[i + 1] == ';') {
            result.push(chars[i + 1]);
            i += 2;
        } else {
            result.push(chars[i]);
            i += 1;
        }
    }
    result
}

/// Escape commas and semicolons in a field for the C++ delimiter-based format.
fn escape_field(s: &str) -> String {
    let mut result = String::with_capacity(s.len());
    for c in s.chars() {
        if c == ',' || c == ';' {
            result.push('\\');
        }
        result.push(c);
    }
    result
}

/// Parse a proxy string of the form "host:port:excl1;excl2", correctly handling
/// IPv6 host addresses enclosed in brackets (e.g. "[::1]:80:excl").
/// Returns HttpProxy or None if the string is empty or invalid.
fn parse_proxy_string(s: String) -> Option<HttpProxy> {
    if s.is_empty() {
        return None;
    }
    // Try IPv6 format first: host starts with '['
    if s.starts_with('[') {
        let closing_bracket = s.find(']');
        if let Some(cb) = closing_bracket {
            let host = s[..cb + 1].to_string(); // includes brackets: [::1]
            let after_bracket = &s[cb + 1..];
            // After ']' there should be ":port:excl" or ":port"
            if after_bracket.starts_with(':') {
                let remaining = &after_bracket[1..];
                let remaining_parts: Vec<&str> = remaining.split(':').collect();
                let port = remaining_parts.first()
                    .and_then(|p| p.parse::<i32>().ok())
                    .unwrap_or(0);
                let exclusion_list = remaining_parts.get(1).map(|excl| {
                    excl.split(';').map(|s| unescape_field(s)).filter(|s| !s.is_empty()).collect()
                }).unwrap_or_default();
                if host.is_empty() {
                    return None;
                }
                return Some(HttpProxy {
                    host,
                    port,
                    exclusion_list,
                    username: None,
                    password: None,
                });
            }
        }
        // Malformed IPv6 proxy string (no closing bracket or no colon after bracket)
        return None;
    }
    // IPv4 / hostname format: simple colon-split
    let proxy_parts: Vec<&str> = s.split(':').collect();
    let host = proxy_parts.first().map(|h| h.to_string()).unwrap_or_default();
    if host.is_empty() {
        return None;
    }
    let port = proxy_parts.get(1).and_then(|p| p.parse::<i32>().ok()).unwrap_or(0);
    let exclusion_list = proxy_parts.get(2).map(|excl| {
        excl.split(';').map(|s| unescape_field(s)).filter(|s| !s.is_empty()).collect()
    }).unwrap_or_default();
    Some(HttpProxy {
        host,
        port,
        exclusion_list,
        username: None,
        password: None,
    })
}

/// Format a proxy host for serialization: wrap IPv6 addresses in brackets
/// so that the colon-split format "host:port:excl" remains parseable.
fn format_proxy_host(host: &str) -> String {
    // If the host contains ':' (IPv6 address) but is NOT already bracketed,
    // wrap it in brackets. IPv6 addresses like "::1" or "fe80::1" contain colons
    // and must be enclosed in [brackets] per RFC 2732.
    if host.contains(':') && !host.starts_with('[') {
        format!("[{}]", host)
    } else {
        host.to_string()
    }
}

pub struct EthernetClient;

impl EthernetClient {
    pub fn is_ethernet_enabled() -> Result<bool, i32> {
        let mut ret = 0;
        let enabled = unsafe { ffi::IsEthernetEnabled(&mut ret) };
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
        let mut ret = 0;
        let result = unsafe { ffi::GetIfaceConfig(&iface, &mut ret) };
        if ret != 0 {
            return Err(ret);
        }
        let parts: Vec<&str> = result.split(',').collect();
        const MIN_CONFIG_FIELDS: usize = 6; // mode,ip,route,gateway,netMask,dnsServers
        if parts.len() < MIN_CONFIG_FIELDS {
            return Err(ERR_INVALID_PARAMETER);
        }
        let mode = parts.get(0)
            .and_then(|s| s.parse::<i32>().ok())
            .ok_or(ERR_INVALID_PARAMETER)?;
        let ip_addr = parts.get(1).map(|s| unescape_field(s)).unwrap_or_default();
        let route = parts.get(2).map(|s| unescape_field(s)).unwrap_or_default();
        let gateway = parts.get(3).map(|s| unescape_field(s)).unwrap_or_default();
        let net_mask = parts.get(4).map(|s| unescape_field(s)).unwrap_or_default();
        let dns_servers = parts.get(5).map(|s| unescape_field(s)).unwrap_or_default();

        let http_proxy = parts.get(6).and_then(|s| {
            let s = unescape_field(s);
            parse_proxy_string(s)
        });

        Ok(InterfaceConfiguration {
            mode,
            ip_addr,
            route,
            gateway,
            net_mask,
            dns_servers,
            http_proxy,
        })
    }

    pub fn set_iface_config(iface: String, config: InterfaceConfiguration) -> Result<(), i32> {
        let proxy_str = match &config.http_proxy {
            Some(proxy) => {
                let excl = proxy.exclusion_list.iter()
                    .map(|s| escape_field(s))
                    .collect::<Vec<String>>()
                    .join(";");
                // Use format_proxy_host to ensure IPv6 addresses are wrapped in brackets
                // so the serialized "host:port:excl" format remains correctly parseable.
                format!("{}:{}:{}", format_proxy_host(&proxy.host), proxy.port, excl)
            }
            None => String::new(),
        };
        let config_str = format!(
            "{},{},{},{},{},{},{}",
            config.mode,
            escape_field(&config.ip_addr),
            escape_field(&config.route),
            escape_field(&config.gateway),
            escape_field(&config.net_mask),
            escape_field(&config.dns_servers),
            escape_field(&proxy_str)
        );
        let ret = unsafe { ffi::SetIfaceConfig(&iface, &config_str) };
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn get_device_information() -> Result<Vec<EthernetDeviceInfos>, i32> {
        let mut ret = 0;
        let result = unsafe { ffi::GetDeviceInformation(&mut ret) };
        if ret != 0 {
            return Err(ret);
        }
        if result.is_empty() {
            return Ok(Vec::new());
        }
        let mut device_list = Vec::new();
        for item in result.split(';') {
            let parts: Vec<&str> = item.split(',').collect();
            const MIN_DEVICE_FIELDS: usize = 7;
            if parts.len() < MIN_DEVICE_FIELDS {
                continue;
            }
            let info = EthernetDeviceInfos {
                iface_name: parts.first().map(|s| unescape_field(s)).unwrap_or_default(),
                device_name: parts.get(1).map(|s| unescape_field(s)).unwrap_or_default(),
                connection_mode: parts.get(2).and_then(|s| s.parse::<i32>().ok()).unwrap_or(0),
                supplier_name: parts.get(3).map(|s| unescape_field(s)).unwrap_or_default(),
                supplier_id: parts.get(4).map(|s| unescape_field(s)).unwrap_or_default(),
                product_name: parts.get(5).map(|s| unescape_field(s)).unwrap_or_default(),
                maximum_rate: parts.get(6).map(|s| unescape_field(s)).unwrap_or_default(),
            };
            device_list.push(info);
        }
        Ok(device_list)
    }

    pub fn get_mac_address() -> Result<Vec<MacAddressInfo>, i32> {
        let mut ret = 0;
        let result = unsafe { ffi::GetMacAddress(&mut ret) };
        if ret != 0 {
            return Err(ret);
        }
        if result.is_empty() {
            return Ok(Vec::new());
        }
        let mut mac_list = Vec::new();
        for item in result.split(';') {
            let parts: Vec<&str> = item.split(',').collect();
            const MIN_MAC_FIELDS: usize = 2;
            if parts.len() < MIN_MAC_FIELDS {
                continue;
            }
            let info = MacAddressInfo {
                iface: parts.first().map(|s| unescape_field(s)).unwrap_or_default(),
                mac_address: parts.get(1).map(|s| unescape_field(s)).unwrap_or_default(),
            };
            mac_list.push(info);
        }
        Ok(mac_list)
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

#[cxx::bridge(namespace = "OHOS::NetManagerAni")]
pub(crate) mod ffi {
    pub struct InterfaceStateInfo {
        pub iface: String,
        pub active: bool,
    }

    extern "Rust" {
        fn execute_interface_added(iface: String);
        fn execute_interface_removed(iface: String);
        fn execute_interface_changed(info: InterfaceStateInfo);
    }

    unsafe extern "C++" {
        include!("ethernet_ani.h");

        fn IsEthernetEnabled(ret: &mut i32) -> bool;
        fn DisableEthernetInterface() -> i32;
        fn EnableEthernetInterface() -> i32;
        fn g_getAllActiveIfaces(active_ifaces: &mut Vec<String>) -> i32;
        fn IsIfaceActive(iface: &String, active_status: &mut i32) -> i32;
        fn GetIfaceConfig(iface: &String, ret: &mut i32) -> String;
        fn SetIfaceConfig(iface: &String, config_str: &String) -> i32;
        fn GetDeviceInformation(ret: &mut i32) -> String;
        fn GetMacAddress(ret: &mut i32) -> String;
        fn RegisterInterfaceStateCallback() -> i32;
        fn UnregisterInterfaceStateCallback() -> i32;
        fn GetErrorCodeAndMessage(error_code: &mut i32) -> String;
    }
}
