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

#[ani_rs::ani(path = "@ohos.net.vpnExtension.vpnExtension.VpnConnectState")]
#[derive(Clone)]
pub enum VpnConnectState {
    Connected = 0,
    Disconnected = 1,
}

#[ani_rs::ani(path = "@ohos.net.connection.connection.NetAddressInner")]
#[derive(Clone)]
pub struct NetAddress {
    pub address: String,
    pub family: Option<i32>,
    pub port: Option<i32>,
}

#[ani_rs::ani(path = "@ohos.net.connection.connection.LinkAddressInner")]
#[derive(Clone)]
pub struct LinkAddress {
    pub address: NetAddress,
    pub prefix_length: i32,
}

#[ani_rs::ani(path = "@ohos.net.connection.connection.RouteInfoInner")]
#[derive(Clone)]
pub struct RouteInfo {
    pub iface: String,
    pub destination: LinkAddress,
    pub gateway: NetAddress,
    pub has_gateway: bool,
    pub is_default_route: bool,
}

#[ani_rs::ani(path = "@ohos.net.vpnExtension.vpnExtension.VpnConfigInner")]
#[derive(Clone)]
pub struct VpnConfig {
    pub vpn_id: Option<String>,
    pub addresses: Vec<LinkAddress>,
    pub routes: Option<Vec<RouteInfo>>,
    pub dns_addresses: Option<Vec<String>>,
    pub search_domains: Option<Vec<String>>,
    pub mtu: Option<i32>,
    pub is_i_pv4_accepted: Option<bool>,
    pub is_i_pv6_accepted: Option<bool>,
    pub is_internal: Option<bool>,
    pub is_blocking: Option<bool>,
    pub trusted_applications: Option<Vec<String>>,
    pub blocked_applications: Option<Vec<String>>,
}
