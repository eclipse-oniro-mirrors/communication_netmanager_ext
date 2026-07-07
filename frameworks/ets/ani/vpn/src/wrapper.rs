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

use crate::bridge;
use crate::register::*;
use ani_rs::business_error::BusinessError;

pub struct VpnClient;

impl VpnClient {
    pub fn create_vpn_connection() -> Result<i32, i32> {
        let mut ret = 0;
        ffi::CreateVpnConnection(&mut ret);
        if ret != 0 {
            return Err(ret);
        }
        Ok(ret)
    }

    pub fn set_up(config: bridge::VpnConfig) -> Result<RawFd, i32> {
        let mut fd: RawFd = -1;
        let config_data: ffi::VpnConfigData = (&config).into();
        let ret = ffi::SetUp(&config_data, &mut fd);
        if ret != 0 {
            return Err(ret);
        }
        // Ownership of the fd is transferred to the ArkTS caller; caller must close it.
        Ok(fd)
    }

    pub fn protect(socket_fd: i32) -> Result<i32, i32> {
        let ret = ffi::Protect(socket_fd);
        if ret != 0 {
            return Err(ret);
        }
        Ok(ret)
    }

    pub fn destroy() -> Result<i32, i32> {
        let ret = ffi::Destroy();
        if ret != 0 {
            return Err(ret);
        }
        Ok(ret)
    }

    pub fn add_sys_vpn_config(config: bridge::SysVpnConfig) -> Result<i32, i32> {
        if let Some(ref password) = config.password {
            if password.len() > 256 {
                return Err(401);
            }
        }
        if let Some(ref pkcs12Password) = config.pkcs12Password {
            if pkcs12Password.len() > 256 {
                return Err(401);
            }
        }
        let config_data: ffi::SysVpnConfigData = (&config).into();
        let ret = ffi::AddSysVpnConfig(&config_data);
        if ret != 0 {
            return Err(ret);
        }
        Ok(ret)
    }

    pub fn delete_sys_vpn_config(vpn_id: String) -> Result<i32, i32> {
        let ret = ffi::DeleteSysVpnConfig(&vpn_id);
        if ret != 0 {
            return Err(ret);
        }
        Ok(ret)
    }

    pub fn get_sys_vpn_config_list() -> Result<Vec<bridge::SysVpnConfig>, i32> {
        let mut ret = 0;
        let list = ffi::GetSysVpnConfigList(&mut ret);
        if ret != 0 {
            return Err(ret);
        }
        Ok(list.iter().map(Into::into).collect())
    }

    pub fn get_sys_vpn_config(vpn_id: String) -> Result<bridge::SysVpnConfig, i32> {
        let mut ret = 0;
        let config = ffi::GetSysVpnConfig(&vpn_id, &mut ret);
        if ret != 0 {
            return Err(ret);
        }
        Ok(bridge::SysVpnConfig::from(&config))
    }

    pub fn get_connected_sys_vpn_config() -> Result<bridge::SysVpnConfig, i32> {
        let mut ret = 0;
        let config = ffi::GetConnectedSysVpnConfig(&mut ret);
        if ret != 0 {
            return Err(ret);
        }
        Ok(bridge::SysVpnConfig::from(&config))
    }

    pub fn get_connected_vpn_app_info() -> Result<Vec<String>, i32> {
        let mut ret = 0;
        let app_info = ffi::GetConnectedVpnAppInfo(&mut ret);
        if ret != 0 {
            return Err(ret);
        }
        Ok(app_info.iter().map(|s| s.to_string()).collect())
    }
}

impl From<&bridge::NetAddress> for ffi::NetAddressData {
    fn from(addr: &bridge::NetAddress) -> Self {
        Self {
            address: addr.address.clone(),
            family: addr.family.unwrap_or(1),
            port: addr.port.unwrap_or(0),
        }
    }
}

impl From<&bridge::LinkAddress> for ffi::LinkAddressData {
    fn from(addr: &bridge::LinkAddress) -> Self {
        Self {
            address: (&addr.address).into(),
            prefix_length: addr.prefixLength,
        }
    }
}

impl From<&bridge::RouteInfo> for ffi::RouteInfoData {
    fn from(route: &bridge::RouteInfo) -> Self {
        Self {
            iface: route.iface.clone(),
            destination: (&route.destination).into(),
            gateway: (&route.gateway).into(),
            has_gateway: route.hasGateway,
            is_default_route: route.isDefaultRoute,
        }
    }
}

impl From<&bridge::VpnConfig> for ffi::VpnConfigData {
    fn from(config: &bridge::VpnConfig) -> Self {
        Self {
            vpn_id: config.vpnId.clone().unwrap_or_default(),
            addresses: config.addresses.iter().map(Into::into).collect(),
            routes: config.routes.as_ref().unwrap_or(&vec![]).iter().map(Into::into).collect(),
            dns_addresses: config.dnsAddresses.clone().unwrap_or_default(),
            search_domains: config.searchDomains.clone().unwrap_or_default(),
            mtu: config.mtu.unwrap_or(1500),
            is_ipv4_accepted: config.isIPv4Accepted.unwrap_or(true),
            is_ipv6_accepted: config.isIPv6Accepted.unwrap_or(false),
            is_legacy: config.isLegacy.unwrap_or(false),
            is_blocking: config.isBlocking.unwrap_or(false),
            trusted_applications: config.trustedApplications.clone().unwrap_or_default(),
            blocked_applications: config.blockedApplications.clone().unwrap_or_default(),
        }
    }
}

impl From<&bridge::SysVpnConfig> for ffi::SysVpnConfigData {
    fn from(config: &bridge::SysVpnConfig) -> Self {
        Self {
            addresses: config.addresses.iter().map(Into::into).collect(),
            routes: config.routes.as_ref().unwrap_or(&vec![]).iter().map(Into::into).collect(),
            dns_addresses: config.dnsAddresses.clone().unwrap_or_default(),
            search_domains: config.searchDomains.clone().unwrap_or_default(),
            mtu: config.mtu.unwrap_or(1500),
            is_ipv4_accepted: config.isIPv4Accepted.unwrap_or(true),
            is_ipv6_accepted: config.isIPv6Accepted.unwrap_or(false),
            is_legacy: config.isLegacy.unwrap_or(false),
            is_blocking: config.isBlocking.unwrap_or(false),
            trusted_applications: config.trustedApplications.clone().unwrap_or_default(),
            blocked_applications: config.blockedApplications.clone().unwrap_or_default(),
            vpn_id: config.vpnId.clone().unwrap_or_default(),
            vpn_name: config.vpnName.clone().unwrap_or_default(),
            vpn_type: config.vpnType.as_ref().map(|v| v.clone() as i32).unwrap_or(0),
            user_name: config.userName.clone().unwrap_or_default(),
            password: config.password.clone().unwrap_or_default(),
            save_login: config.saveLogin.unwrap_or(false),
            user_id: config.userId.unwrap_or(0),
            forwarding_routes: config.forwardingRoutes.clone().unwrap_or_default(),
            remote_addresses: config.remoteAddresses.clone().unwrap_or_default(),
            local_addresses: config.localAddresses.as_ref().unwrap_or(&vec![]).iter().map(Into::into).collect(),
            pkcs12_password: config.pkcs12Password.clone().unwrap_or_default(),
            pkcs12_file_data: config.pkcs12FileData.as_ref().map(|buf| buf.to_vec()).unwrap_or_default(),
            // IpsecVpnConfig fields (not in SysVpnConfig, use defaults)
            ipsec_pre_shared_key: String::new(),
            ipsec_identifier: String::new(),
            swanctl_config: String::new(),
            strong_swan_config: String::new(),
            ipsec_ca_cert_config: String::new(),
            ipsec_private_user_cert_config: String::new(),
            ipsec_public_user_cert_config: String::new(),
            ipsec_private_server_cert_config: String::new(),
            ipsec_public_server_cert_config: String::new(),
            ipsec_ca_cert_file_path: String::new(),
            ipsec_private_user_cert_file_path: String::new(),
            ipsec_public_user_cert_file_path: String::new(),
            ipsec_private_server_cert_file_path: String::new(),
            ipsec_public_server_cert_file_path: String::new(),
            // OpenVpnConfig fields (not in SysVpnConfig, use defaults)
            ovpn_port: String::new(),
            ovpn_protocol: 0,
            ovpn_config: String::new(),
            ovpn_auth_type: 0,
            askpass: String::new(),
            ovpn_config_file_path: String::new(),
            ovpn_ca_cert_file_path: String::new(),
            ovpn_user_cert_file_path: String::new(),
            ovpn_private_key_file_path: String::new(),
            // L2tpVpnConfig fields (not in SysVpnConfig, use defaults)
            ipsec_config: String::new(),
            ipsec_secrets: String::new(),
            options_l2tpd_client: String::new(),
            xl2tpd_config: String::new(),
            l2tp_shared_key: String::new(),
        }
    }
}

impl From<&ffi::NetAddressData> for bridge::NetAddress {
    fn from(data: &ffi::NetAddressData) -> Self {
        Self {
            address: data.address.clone(),
            family: Some(data.family),
            port: Some(data.port),
        }
    }
}

impl From<&ffi::LinkAddressData> for bridge::LinkAddress {
    fn from(data: &ffi::LinkAddressData) -> Self {
        Self {
            address: (&data.address).into(),
            prefixLength: data.prefix_length,
        }
    }
}

impl From<&ffi::RouteInfoData> for bridge::RouteInfo {
    fn from(data: &ffi::RouteInfoData) -> Self {
        Self {
            iface: data.iface.clone(),
            destination: (&data.destination).into(),
            gateway: (&data.gateway).into(),
            hasGateway: data.has_gateway,
            isDefaultRoute: data.is_default_route,
        }
    }
}

impl From<&ffi::SysVpnConfigData> for bridge::SysVpnConfig {
    fn from(data: &ffi::SysVpnConfigData) -> Self {
        Self {
            addresses: data.addresses.iter().map(Into::into).collect(),
            routes: Some(data.routes.iter().map(Into::into).collect()),
            dnsAddresses: Some(data.dns_addresses.iter().map(|s| s.clone()).collect()),
            searchDomains: Some(data.search_domains.iter().map(|s| s.clone()).collect()),
            mtu: Some(data.mtu),
            isIPv4Accepted: Some(data.is_ipv4_accepted),
            isIPv6Accepted: Some(data.is_ipv6_accepted),
            isLegacy: Some(data.is_legacy),
            isBlocking: Some(data.is_blocking),
            trustedApplications: Some(data.trusted_applications.iter().map(|s| s.to_string()).collect()),
            blockedApplications: Some(data.blocked_applications.iter().map(|s| s.to_string()).collect()),
            vpnId: if data.vpn_id.is_empty() { None } else { Some(data.vpn_id.to_string()) },
            vpnName: if data.vpn_name.is_empty() { None } else { Some(data.vpn_name.to_string()) },
            vpnType: match data.vpn_type {
                1 => Some(bridge::SysVpnType::IKEV2_IPSEC_MSCHAPV2),
                2 => Some(bridge::SysVpnType::IKEV2_IPSEC_PSK),
                3 => Some(bridge::SysVpnType::IKEV2_IPSEC_RSA),
                4 => Some(bridge::SysVpnType::L2TP_IPSEC_PSK),
                5 => Some(bridge::SysVpnType::L2TP_IPSEC_RSA),
                6 => Some(bridge::SysVpnType::IPSEC_XAUTH_PSK),
                7 => Some(bridge::SysVpnType::IPSEC_XAUTH_RSA),
                8 => Some(bridge::SysVpnType::IPSEC_HYBRID_RSA),
                9 => Some(bridge::SysVpnType::OPENVPN),
                // vpnType 10 (L2TP) is internal-only; map to None for the public bridge enum
                _ => None,
            },
            userName: if data.user_name.is_empty() { None } else { Some(data.user_name.to_string()) },
            password: if data.password.is_empty() { None } else { Some(data.password.to_string()) },
            saveLogin: Some(data.save_login),
            userId: Some(data.user_id),
            forwardingRoutes: if data.forwarding_routes.is_empty() { None } else { Some(data.forwarding_routes.to_string()) },
            remoteAddresses: Some(data.remote_addresses.iter().map(|s| s.to_string()).collect()),
            localAddresses: Some(data.local_addresses.iter().map(Into::into).collect()),
            pkcs12Password: if data.pkcs12_password.is_empty() { None } else { Some(data.pkcs12_password.to_string()) },
            pkcs12FileData: if data.pkcs12_file_data.is_empty() { None } else { Some(ani_rs::typed_array::ArrayBuffer::new_with_vec(data.pkcs12_file_data.clone())) },
        }
    }
}

impl bridge::VpnConnectState {
    pub fn from_ffi(info: &ffi::VpnStateInfo) -> Self {
        if info.is_connected {
            bridge::VpnConnectState::Connected
        } else {
            bridge::VpnConnectState::Disconnected
        }
    }
}

pub fn convert_to_business_error(code: i32) -> BusinessError {
    let mut mapped_code = code;
    let error_msg = ffi::GetErrorCodeAndMessage(&mut mapped_code);
    BusinessError::new(mapped_code, error_msg)
}

#[cxx::bridge(namespace = "OHOS::NetManagerAni")]
pub(crate) mod ffi {

    pub struct NetAddressData {
        pub address: String,
        pub family: i32,
        pub port: i32,
    }

    pub struct LinkAddressData {
        pub address: NetAddressData,
        pub prefix_length: i32,
    }

    pub struct RouteInfoData {
        pub iface: String,
        pub destination: LinkAddressData,
        pub gateway: NetAddressData,
        pub has_gateway: bool,
        pub is_default_route: bool,
    }

    pub struct VpnConfigData {
        pub vpn_id: String,
        pub addresses: Vec<LinkAddressData>,
        pub routes: Vec<RouteInfoData>,
        pub dns_addresses: Vec<String>,
        pub search_domains: Vec<String>,
        pub mtu: i32,
        pub is_ipv4_accepted: bool,
        pub is_ipv6_accepted: bool,
        pub is_legacy: bool,
        pub is_blocking: bool,
        pub trusted_applications: Vec<String>,
        pub blocked_applications: Vec<String>,
    }

    pub struct SysVpnConfigData {
        pub addresses: Vec<LinkAddressData>,
        pub routes: Vec<RouteInfoData>,
        pub dns_addresses: Vec<String>,
        pub search_domains: Vec<String>,
        pub mtu: i32,
        pub is_ipv4_accepted: bool,
        pub is_ipv6_accepted: bool,
        pub is_legacy: bool,
        pub is_blocking: bool,
        pub trusted_applications: Vec<String>,
        pub blocked_applications: Vec<String>,
        pub vpn_id: String,
        pub vpn_name: String,
        pub vpn_type: i32,
        pub user_name: String,
        pub password: String,
        pub save_login: bool,
        pub user_id: i32,
        pub forwarding_routes: String,
        pub remote_addresses: Vec<String>,
        pub local_addresses: Vec<LinkAddressData>,
        pub pkcs12_password: String,
        pub pkcs12_file_data: Vec<u8>,
        // IpsecVpnConfig specific fields
        pub ipsec_pre_shared_key: String,
        pub ipsec_identifier: String,
        pub swanctl_config: String,
        pub strong_swan_config: String,
        pub ipsec_ca_cert_config: String,
        pub ipsec_private_user_cert_config: String,
        pub ipsec_public_user_cert_config: String,
        pub ipsec_private_server_cert_config: String,
        pub ipsec_public_server_cert_config: String,
        pub ipsec_ca_cert_file_path: String,
        pub ipsec_private_user_cert_file_path: String,
        pub ipsec_public_user_cert_file_path: String,
        pub ipsec_private_server_cert_file_path: String,
        pub ipsec_public_server_cert_file_path: String,
        // OpenVpnConfig specific fields
        pub ovpn_port: String,
        pub ovpn_protocol: i32,
        pub ovpn_config: String,
        pub ovpn_auth_type: i32,
        pub askpass: String,
        pub ovpn_config_file_path: String,
        pub ovpn_ca_cert_file_path: String,
        pub ovpn_user_cert_file_path: String,
        pub ovpn_private_key_file_path: String,
        // L2tpVpnConfig specific fields (beyond ipsec-common)
        pub ipsec_config: String,
        pub ipsec_secrets: String,
        pub options_l2tpd_client: String,
        pub xl2tpd_config: String,
        pub l2tp_shared_key: String,
    }

    pub struct VpnStateInfo {
        pub is_connected: bool,
        pub bundle_name: String,
        pub vpn_id: String,
    }

    extern "Rust" {
        fn execute_vpn_state_changed(info: VpnStateInfo);
        fn execute_multi_vpn_state_changed(info: VpnStateInfo);
    }

    unsafe extern "C++" {
        include!("vpn_ani.h");

        fn CreateVpnConnection(ret: &mut i32) -> bool;
        fn SetUp(config: &VpnConfigData, fd: &mut i32) -> i32;
        fn Protect(socketFd: i32) -> i32;
        fn Destroy() -> i32;
        fn AddSysVpnConfig(config: &SysVpnConfigData) -> i32;
        fn DeleteSysVpnConfig(vpnId: &String) -> i32;
        fn GetSysVpnConfigList(ret: &mut i32) -> Vec<SysVpnConfigData>;
        fn GetSysVpnConfig(vpnId: &String, ret: &mut i32) -> SysVpnConfigData;
        fn GetConnectedSysVpnConfig(ret: &mut i32) -> SysVpnConfigData;
        fn GetConnectedVpnAppInfo(ret: &mut i32) -> Vec<String>;
        fn VpnObserverRegister() -> i32;
        fn VpnObserverUnRegister() -> i32;
        fn GetErrorCodeAndMessage(error_code: &mut i32) -> String;
    }
}
