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

#[ani_rs::ani(path = "@ohos.net.vpn.vpn.VpnConnectState")]
#[derive(Clone)]
pub enum VpnConnectState {
    VpnConnected = 1,
    VpnDisconnected = 2,
}

#[ani_rs::ani(path = "@ohos.net.vpn.vpn.MultiVpnConnectStateInner")]
#[derive(Clone)]
pub struct MultiVpnConnectState {
    pub is_connected: bool,
    pub bundle_name: String,
    pub vpn_id: String,
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

#[ani_rs::ani(path = "@ohos.net.vpn.vpn.VpnConfigInner")]
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
    pub is_legacy: Option<bool>,
    pub is_blocking: Option<bool>,
    pub trusted_applications: Option<Vec<String>>,
    pub blocked_applications: Option<Vec<String>>,
}

#[ani_rs::ani(path = "@ohos.net.vpn.vpn.SysVpnConfigInner")]
#[derive(Clone)]
pub struct SysVpnConfig {
    pub addresses: Vec<LinkAddress>,
    pub routes: Option<Vec<RouteInfo>>,
    pub dns_addresses: Option<Vec<String>>,
    pub search_domains: Option<Vec<String>>,
    pub mtu: Option<i32>,
    pub is_i_pv4_accepted: Option<bool>,
    pub is_i_pv6_accepted: Option<bool>,
    pub is_legacy: Option<bool>,
    pub is_blocking: Option<bool>,
    pub trusted_applications: Option<Vec<String>>,
    pub blocked_applications: Option<Vec<String>>,
    pub vpn_id: Option<String>,
    pub vpn_name: Option<String>,
    pub vpn_type: Option<SysVpnType>,
    pub user_name: Option<String>,
    pub password: Option<String>,
    pub save_login: Option<bool>,
    pub user_id: Option<i32>,
    pub forwarding_routes: Option<String>,
    pub remote_addresses: Option<Vec<String>>,
    pub local_addresses: Option<Vec<LinkAddress>>,
    pub pkcs12_password: Option<String>,
    pub pkcs12_file_data: Option<Uint8Array>,
}

#[ani_rs::ani(path = "@ohos.net.vpn.vpn.OpenVpnConfigInner")]
#[derive(Clone)]
pub struct OpenVpnConfig {
    pub addresses: Vec<LinkAddress>,
    pub routes: Option<Vec<RouteInfo>>,
    pub dns_addresses: Option<Vec<String>>,
    pub search_domains: Option<Vec<String>>,
    pub mtu: Option<i32>,
    pub is_i_pv4_accepted: Option<bool>,
    pub is_i_pv6_accepted: Option<bool>,
    pub is_legacy: Option<bool>,
    pub is_blocking: Option<bool>,
    pub trusted_applications: Option<Vec<String>>,
    pub blocked_applications: Option<Vec<String>>,
    pub vpn_id: Option<String>,
    pub vpn_name: Option<String>,
    pub vpn_type: Option<SysVpnType>,
    pub user_name: Option<String>,
    pub password: Option<String>,
    pub save_login: Option<bool>,
    pub user_id: Option<i32>,
    pub forwarding_routes: Option<String>,
    pub remote_addresses: Option<Vec<String>>,
    pub local_addresses: Option<Vec<LinkAddress>>,
    pub pkcs12_password: Option<String>,
    pub pkcs12_file_data: Option<Uint8Array>,
    pub ovpn_port: Option<String>,
    pub ovpn_protocol: Option<i32>,
    pub ovpn_config: Option<String>,
    pub ovpn_auth_type: Option<i32>,
    pub askpass: Option<String>,
    pub ovpn_config_file_path: Option<String>,
    pub ovpn_ca_cert_file_path: Option<String>,
    pub ovpn_user_cert_file_path: Option<String>,
    pub ovpn_private_key_file_path: Option<String>,
}

#[ani_rs::ani(path = "@ohos.net.vpn.vpn.IpsecVpnConfigInner")]
#[derive(Clone)]
pub struct IpsecVpnConfig {
    pub addresses: Vec<LinkAddress>,
    pub routes: Option<Vec<RouteInfo>>,
    pub dns_addresses: Option<Vec<String>>,
    pub search_domains: Option<Vec<String>>,
    pub mtu: Option<i32>,
    pub is_i_pv4_accepted: Option<bool>,
    pub is_i_pv6_accepted: Option<bool>,
    pub is_legacy: Option<bool>,
    pub is_blocking: Option<bool>,
    pub trusted_applications: Option<Vec<String>>,
    pub blocked_applications: Option<Vec<String>>,
    pub vpn_id: Option<String>,
    pub vpn_name: Option<String>,
    pub vpn_type: Option<SysVpnType>,
    pub user_name: Option<String>,
    pub password: Option<String>,
    pub save_login: Option<bool>,
    pub user_id: Option<i32>,
    pub forwarding_routes: Option<String>,
    pub remote_addresses: Option<Vec<String>>,
    pub local_addresses: Option<Vec<LinkAddress>>,
    pub pkcs12_password: Option<String>,
    pub pkcs12_file_data: Option<Uint8Array>,
    pub ipsec_pre_shared_key: Option<String>,
    pub ipsec_identifier: Option<String>,
    pub swanctl_config: Option<String>,
    pub strong_swan_config: Option<String>,
    pub ipsec_ca_cert_config: Option<String>,
    pub ipsec_private_user_cert_config: Option<String>,
    pub ipsec_public_user_cert_config: Option<String>,
    pub ipsec_private_server_cert_config: Option<String>,
    pub ipsec_public_server_cert_config: Option<String>,
    pub ipsec_ca_cert_file_path: Option<String>,
    pub ipsec_private_user_cert_file_path: Option<String>,
    pub ipsec_public_user_cert_file_path: Option<String>,
    pub ipsec_private_server_cert_file_path: Option<String>,
    pub ipsec_public_server_cert_file_path: Option<String>,
}

#[ani_rs::ani(path = "@ohos.net.vpn.vpn.L2tpVpnConfigInner")]
#[derive(Clone)]
pub struct L2tpVpnConfig {
    pub addresses: Vec<LinkAddress>,
    pub routes: Option<Vec<RouteInfo>>,
    pub dns_addresses: Option<Vec<String>>,
    pub search_domains: Option<Vec<String>>,
    pub mtu: Option<i32>,
    pub is_i_pv4_accepted: Option<bool>,
    pub is_i_pv6_accepted: Option<bool>,
    pub is_legacy: Option<bool>,
    pub is_blocking: Option<bool>,
    pub trusted_applications: Option<Vec<String>>,
    pub blocked_applications: Option<Vec<String>>,
    pub vpn_id: Option<String>,
    pub vpn_name: Option<String>,
    pub vpn_type: Option<SysVpnType>,
    pub user_name: Option<String>,
    pub password: Option<String>,
    pub save_login: Option<bool>,
    pub user_id: Option<i32>,
    pub forwarding_routes: Option<String>,
    pub remote_addresses: Option<Vec<String>>,
    pub local_addresses: Option<Vec<LinkAddress>>,
    pub pkcs12_password: Option<String>,
    pub pkcs12_file_data: Option<Uint8Array>,
    pub ipsec_pre_shared_key: Option<String>,
    pub ipsec_identifier: Option<String>,
    pub strong_swan_config: Option<String>,
    pub ipsec_ca_cert_config: Option<String>,
    pub ipsec_private_user_cert_config: Option<String>,
    pub ipsec_public_user_cert_config: Option<String>,
    pub ipsec_private_server_cert_config: Option<String>,
    pub ipsec_public_server_cert_config: Option<String>,
    pub ipsec_ca_cert_file_path: Option<String>,
    pub ipsec_private_user_cert_file_path: Option<String>,
    pub ipsec_public_user_cert_file_path: Option<String>,
    pub ipsec_private_server_cert_file_path: Option<String>,
    pub ipsec_public_server_cert_file_path: Option<String>,
    pub ipsec_config: Option<String>,
    pub ipsec_secrets: Option<String>,
    pub options_l2tpd_client: Option<String>,
    pub xl2tpd_config: Option<String>,
    pub l2tp_shared_key: Option<String>,
}

#[ani_rs::ani(path = "@ohos.net.vpn.vpn.SysVpnType")]
#[derive(Clone, Copy)]
pub enum SysVpnType {
    Ikev2IpsecMschapv2 = 1,
    Ikev2IpsecPsk = 2,
    Ikev2IpsecRsa = 3,
    L2tpIpsecPsk = 4,
    L2tpIpsecRsa = 5,
    IpsecXauthPsk = 6,
    IpsecXauthRsa = 7,
    IpsecHybridRsa = 8,
    Openvpn = 9,
}
