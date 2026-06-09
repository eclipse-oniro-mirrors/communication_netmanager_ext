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

use ani_rs;

#[ani_rs::ani(path = "@ohos.net.ethernet.ethernet.HttpProxyInner")]
#[derive(Clone, Default)]
pub struct HttpProxy {
    pub host: String,
    pub port: i32,
    pub exclusion_list: Vec<String>,
    pub username: Option<String>,
    // SECURITY: This field stores the password in plaintext for FFI bridge compatibility.
    // It MUST NEVER be logged, serialized to JSON, or transmitted over the network.
    // Use `sanitize()` when debugging or displaying this struct.
    pub password: Option<String>,
}

impl HttpProxy {
    /// Returns a sanitized copy with the password field replaced by "***".
    /// Use this for any diagnostic output, logging, or display purposes.
    pub fn sanitize(&self) -> HttpProxy {
        HttpProxy {
            host: self.host.clone(),
            port: self.port,
            exclusion_list: self.exclusion_list.clone(),
            username: self.username.clone(),
            password: self.password.as_ref().map(|_| "***".to_string()),
        }
    }
}

#[ani_rs::ani(path = "@ohos.net.ethernet.ethernet.MacAddressInfo")]
#[derive(Clone)]
pub struct MacAddressInfo {
    pub iface: String,
    pub mac_address: String,
}

#[ani_rs::ani(path = "@ohos.net.ethernet.ethernet.InterfaceStateInfo")]
#[derive(Clone)]
pub struct InterfaceStateInfo {
    pub iface: String,
    pub active: bool,
}

#[ani_rs::ani(path = "@ohos.net.ethernet.ethernet.InterfaceConfiguration")]
#[derive(Clone)]
pub struct InterfaceConfiguration {
    pub mode: i32,
    pub ip_addr: String,
    pub route: String,
    pub gateway: String,
    pub net_mask: String,
    pub dns_servers: String,
    pub http_proxy: Option<HttpProxy>,
}

#[ani_rs::ani(path = "@ohos.net.ethernet.ethernet.EthernetDeviceInfos")]
#[derive(Clone)]
pub struct EthernetDeviceInfos {
    pub iface_name: String,
    pub device_name: String,
    pub connection_mode: i32,
    pub supplier_name: String,
    pub supplier_id: String,
    pub product_name: String,
    pub maximum_rate: String,
}