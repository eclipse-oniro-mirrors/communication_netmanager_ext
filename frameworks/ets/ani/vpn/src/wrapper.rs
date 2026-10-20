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

use std::ffi::CStr;
use std::os::unix::io::RawFd;

use crate::bridge;
use crate::register::*;
use ani_rs::business_error::BusinessError;
use ani_rs::typed_array::Uint8Array;

pub struct VpnClient;

impl VpnClient {
    pub fn create_vpn_connection() -> Result<i64, i32> {
        let mut native_ptr = 0;
        let ret = ffi::CreateVpnConnection(&mut native_ptr);
        if ret != 0 {
            return Err(ret);
        }
        Ok(native_ptr)
    }

    pub fn set_up(native_ptr: i64, config: ffi::SysVpnConfigFullAni) -> Result<RawFd, i32> {
        let mut fd: RawFd = -1;
        let ret = ffi::SetUp(native_ptr, &config, &mut fd);
        if ret != 0 {
            return Err(ret);
        }
        Ok(fd)
    }

    pub fn protect(native_ptr: i64, socket_fd: i32) -> Result<(), i32> {
        let ret = ffi::Protect(native_ptr, socket_fd);
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn destroy(native_ptr: i64,) -> Result<(), i32> {
        let ret = ffi::Destroy(native_ptr);
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn add_sys_vpn_config(config: ffi::SysVpnConfigFullAni) -> Result<(), i32> {
        let ret = ffi::AddSysVpnConfig(&config);
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn delete_sys_vpn_config(vpn_id: String) -> Result<(), i32> {
        let ret = ffi::DeleteSysVpnConfig(&vpn_id);
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn get_sys_vpn_config_list() -> Result<Vec<ffi::SysVpnConfigFullAni>, i32> {
        let mut list: Vec<ffi::SysVpnConfigFullAni> = Vec::new();
        let ret = ffi::g_getSysVpnConfigList(&mut list);
        if ret != 0 {
            return Err(ret);
        }
        Ok(list)
    }

    pub fn get_sys_vpn_config(vpn_id: String) -> Result<ffi::SysVpnConfigFullAni, i32> {
        let mut config = empty_full();
        let ret = ffi::GetSysVpnConfig(&vpn_id, &mut config);
        if ret != 0 {
            return Err(ret);
        }
        Ok(config)
    }

    pub fn get_connected_sys_vpn_config() -> Result<ffi::SysVpnConfigFullAni, i32> {
        let mut config = empty_full();
        let mut ret = 0;
        let result = ffi::GetConnectedSysVpnConfig(&mut ret, &mut config);
        if result != 0 {
            return Err(result);
        }
        Ok(config)
    }

    pub fn get_connected_vpn_app_info() -> Result<Vec<String>, i32> {
        let mut app_info: Vec<String> = Vec::new();
        let ret = ffi::GetConnectedVpnAppInfo(&mut app_info);
        if ret != 0 {
            return Err(ret);
        }
        Ok(app_info)
    }
}

impl From<&bridge::NetAddress> for ffi::NetAddressAni {
    fn from(addr: &bridge::NetAddress) -> Self {
        Self {
            address: addr.address.clone(),
            family: addr.family.unwrap_or(1),
            port: addr.port.unwrap_or(0),
        }
    }
}

impl From<&bridge::LinkAddress> for ffi::LinkAddressAni {
    fn from(addr: &bridge::LinkAddress) -> Self {
        Self {
            address: (&addr.address).into(),
            prefix_length: addr.prefix_length,
        }
    }
}

impl From<&bridge::RouteInfo> for ffi::RouteInfoAni {
    fn from(route: &bridge::RouteInfo) -> Self {
        Self {
            iface: route.iface.clone(),
            destination: (&route.destination).into(),
            gateway: (&route.gateway).into(),
            has_gateway: route.has_gateway,
            is_default_route: route.is_default_route,
        }
    }
}

impl From<&bridge::VpnConfig> for ffi::VpnConfigAni {
    fn from(config: &bridge::VpnConfig) -> Self {
        Self {
            vpn_id: config.vpn_id.clone().unwrap_or_default(),
            addresses: config.addresses.iter().map(Into::into).collect(),
            routes: config.routes.as_ref().unwrap_or(&vec![]).iter().map(Into::into).collect(),
            dns_addresses: config.dns_addresses.clone().unwrap_or_default(),
            search_domains: config.search_domains.clone().unwrap_or_default(),
            mtu: config.mtu.unwrap_or(1500),
            is_ipv4_accepted: config.is_i_pv4_accepted.unwrap_or(true),
            is_ipv6_accepted: config.is_i_pv6_accepted.unwrap_or(false),
            is_legacy: config.is_legacy.unwrap_or(false),
            is_blocking: config.is_blocking.unwrap_or(false),
            trusted_applications: config.trusted_applications.clone().unwrap_or_default(),
            blocked_applications: config.blocked_applications.clone().unwrap_or_default(),
        }
    }
}


fn empty_full() -> ffi::SysVpnConfigFullAni {
    ffi::SysVpnConfigFullAni {
        // VpnConfig base
        vpn_id: String::new(),
        addresses: Vec::new(),
        routes: Vec::new(),
        dns_addresses: Vec::new(),
        search_domains: Vec::new(),
        mtu: 0,
        is_ipv4_accepted: false,
        is_ipv6_accepted: false,
        is_legacy: false,
        is_blocking: false,
        trusted_applications: Vec::new(),
        blocked_applications: Vec::new(),
        // SysVpnConfig base
        vpn_name: String::new(),
        vpn_type: 0,
        user_name: String::new(),
        password: String::new(),
        save_login: false,
        user_id: 0,
        forwarding_routes: String::new(),
        remote_addresses: Vec::new(),
        local_addresses: Vec::new(),
        pkcs12_password: String::new(),
        pkcs12_file_data: Vec::new(),
        // Ipsec-common
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
        // OpenVPN-specific
        ovpn_port: String::new(),
        ovpn_protocol: 0,
        ovpn_config: String::new(),
        ovpn_auth_type: 0,
        askpass: String::new(),
        ovpn_config_file_path: String::new(),
        ovpn_ca_cert_file_path: String::new(),
        ovpn_user_cert_file_path: String::new(),
        ovpn_private_key_file_path: String::new(),
        // L2TP-unique
        ipsec_config: String::new(),
        ipsec_secrets: String::new(),
        options_l2tpd_client: String::new(),
        xl2tpd_config: String::new(),
        l2tp_shared_key: String::new(),
    }
}

impl From<&bridge::VpnConfig> for ffi::SysVpnConfigFullAni {
    fn from(config: &bridge::VpnConfig) -> Self {
        Self {
            vpn_id: config.vpn_id.clone().unwrap_or_default(),
            addresses: config.addresses.iter().map(Into::into).collect(),
            routes: config.routes.as_ref().unwrap_or(&vec![]).iter().map(Into::into).collect(),
            dns_addresses: config.dns_addresses.clone().unwrap_or_default(),
            search_domains: config.search_domains.clone().unwrap_or_default(),
            mtu: config.mtu.unwrap_or(1500),
            is_ipv4_accepted: config.is_i_pv4_accepted.unwrap_or(true),
            is_ipv6_accepted: config.is_i_pv6_accepted.unwrap_or(false),
            is_legacy: config.is_legacy.unwrap_or(false),
            is_blocking: config.is_blocking.unwrap_or(false),
            trusted_applications: config.trusted_applications.clone().unwrap_or_default(),
            blocked_applications: config.blocked_applications.clone().unwrap_or_default(),
            ..empty_full()
        }
    }
}

fn fill_full_base(full: &mut ffi::SysVpnConfigFullAni, base: &bridge::SysVpnConfig) {
    // VpnConfig base fields
    full.vpn_id = base.vpn_id.clone().unwrap_or_default();
    full.addresses = base.addresses.iter().map(Into::into).collect();
    full.routes = base.routes.as_ref().unwrap_or(&vec![]).iter().map(Into::into).collect();
    full.dns_addresses = base.dns_addresses.clone().unwrap_or_default();
    full.search_domains = base.search_domains.clone().unwrap_or_default();
    full.mtu = base.mtu.unwrap_or(1500);
    full.is_ipv4_accepted = base.is_i_pv4_accepted.unwrap_or(true);
    full.is_ipv6_accepted = base.is_i_pv6_accepted.unwrap_or(false);
    full.is_legacy = base.is_legacy.unwrap_or(false);
    full.is_blocking = base.is_blocking.unwrap_or(false);
    full.trusted_applications = base.trusted_applications.clone().unwrap_or_default();
    full.blocked_applications = base.blocked_applications.clone().unwrap_or_default();
    // SysVpnConfig base fields
    full.vpn_name = base.vpn_name.clone().unwrap_or_default();
    full.vpn_type = base.vpn_type.as_ref().map(|v| *v as i32).unwrap_or(0);
    full.user_name = base.user_name.clone().unwrap_or_default();
    full.password = base.password.clone().unwrap_or_default();
    full.save_login = base.save_login.unwrap_or(false);
    full.user_id = base.user_id.unwrap_or(0);
    full.forwarding_routes = base.forwarding_routes.clone().unwrap_or_default();
    full.remote_addresses = base.remote_addresses.clone().unwrap_or_default();
    full.local_addresses = base
        .local_addresses
        .as_ref()
        .unwrap_or(&vec![])
        .iter()
        .map(Into::into)
        .collect();
    full.pkcs12_password = base.pkcs12_password.clone().unwrap_or_default();
    full.pkcs12_file_data = base.pkcs12_file_data.as_ref().map(|buf| buf.to_vec()).unwrap_or_default();
}

impl From<&bridge::SysVpnConfig> for ffi::SysVpnConfigFullAni {
    fn from(config: &bridge::SysVpnConfig) -> Self {
        let mut full = empty_full();
        fill_full_base(&mut full, config);
        full
    }
}

impl From<&bridge::OpenVpnConfig> for bridge::SysVpnConfig {
    fn from(config: &bridge::OpenVpnConfig) -> Self {
        Self {
            addresses: config.addresses.clone(),
            routes: config.routes.clone(),
            dns_addresses: config.dns_addresses.clone(),
            search_domains: config.search_domains.clone(),
            mtu: config.mtu,
            is_i_pv4_accepted: config.is_i_pv4_accepted,
            is_i_pv6_accepted: config.is_i_pv6_accepted,
            is_legacy: config.is_legacy,
            is_blocking: config.is_blocking,
            trusted_applications: config.trusted_applications.clone(),
            blocked_applications: config.blocked_applications.clone(),
            vpn_id: config.vpn_id.clone(),
            vpn_name: config.vpn_name.clone(),
            vpn_type: config.vpn_type,
            user_name: config.user_name.clone(),
            password: config.password.clone(),
            save_login: config.save_login,
            user_id: config.user_id,
            forwarding_routes: config.forwarding_routes.clone(),
            remote_addresses: config.remote_addresses.clone(),
            local_addresses: config.local_addresses.clone(),
            pkcs12_password: config.pkcs12_password.clone(),
            pkcs12_file_data: config.pkcs12_file_data.clone(),
        }
    }
}

impl From<&bridge::OpenVpnConfig> for ffi::SysVpnConfigFullAni {
    fn from(config: &bridge::OpenVpnConfig) -> Self {
        let base: bridge::SysVpnConfig = config.into();
        let mut full = empty_full();
        fill_full_base(&mut full, &base);
        // OpenVPN-specific fields
        full.ovpn_port = config.ovpn_port.clone().unwrap_or_default();
        full.ovpn_protocol = config.ovpn_protocol.unwrap_or(0);
        full.ovpn_config = config.ovpn_config.clone().unwrap_or_default();
        full.ovpn_auth_type = config.ovpn_auth_type.unwrap_or(0);
        full.askpass = config.askpass.clone().unwrap_or_default();
        full.ovpn_config_file_path = config.ovpn_config_file_path.clone().unwrap_or_default();
        full.ovpn_ca_cert_file_path = config.ovpn_ca_cert_file_path.clone().unwrap_or_default();
        full.ovpn_user_cert_file_path = config.ovpn_user_cert_file_path.clone().unwrap_or_default();
        full.ovpn_private_key_file_path = config.ovpn_private_key_file_path.clone().unwrap_or_default();
        full
    }
}

impl From<&bridge::IpsecVpnConfig> for bridge::SysVpnConfig {
    fn from(config: &bridge::IpsecVpnConfig) -> Self {
        Self {
            addresses: config.addresses.clone(),
            routes: config.routes.clone(),
            dns_addresses: config.dns_addresses.clone(),
            search_domains: config.search_domains.clone(),
            mtu: config.mtu,
            is_i_pv4_accepted: config.is_i_pv4_accepted,
            is_i_pv6_accepted: config.is_i_pv6_accepted,
            is_legacy: config.is_legacy,
            is_blocking: config.is_blocking,
            trusted_applications: config.trusted_applications.clone(),
            blocked_applications: config.blocked_applications.clone(),
            vpn_id: config.vpn_id.clone(),
            vpn_name: config.vpn_name.clone(),
            vpn_type: config.vpn_type,
            user_name: config.user_name.clone(),
            password: config.password.clone(),
            save_login: config.save_login,
            user_id: config.user_id,
            forwarding_routes: config.forwarding_routes.clone(),
            remote_addresses: config.remote_addresses.clone(),
            local_addresses: config.local_addresses.clone(),
            pkcs12_password: config.pkcs12_password.clone(),
            pkcs12_file_data: config.pkcs12_file_data.clone(),
        }
    }
}

impl From<&bridge::IpsecVpnConfig> for ffi::SysVpnConfigFullAni {
    fn from(config: &bridge::IpsecVpnConfig) -> Self {
        let base: bridge::SysVpnConfig = config.into();
        let mut full = empty_full();
        fill_full_base(&mut full, &base);
        // IPsec-specific fields
        full.ipsec_pre_shared_key = config.ipsec_pre_shared_key.clone().unwrap_or_default();
        full.ipsec_identifier = config.ipsec_identifier.clone().unwrap_or_default();
        full.swanctl_config = config.swanctl_config.clone().unwrap_or_default();
        full.strong_swan_config = config.strong_swan_config.clone().unwrap_or_default();
        full.ipsec_ca_cert_config = config.ipsec_ca_cert_config.clone().unwrap_or_default();
        full.ipsec_private_user_cert_config = config.ipsec_private_user_cert_config.clone().unwrap_or_default();
        full.ipsec_public_user_cert_config = config.ipsec_public_user_cert_config.clone().unwrap_or_default();
        full.ipsec_private_server_cert_config = config.ipsec_private_server_cert_config.clone().unwrap_or_default();
        full.ipsec_public_server_cert_config = config.ipsec_public_server_cert_config.clone().unwrap_or_default();
        full.ipsec_ca_cert_file_path = config.ipsec_ca_cert_file_path.clone().unwrap_or_default();
        full.ipsec_private_user_cert_file_path = config.ipsec_private_user_cert_file_path.clone().unwrap_or_default();
        full.ipsec_public_user_cert_file_path = config.ipsec_public_user_cert_file_path.clone().unwrap_or_default();
        full.ipsec_private_server_cert_file_path = config.ipsec_private_server_cert_file_path.clone().unwrap_or_default();
        full.ipsec_public_server_cert_file_path = config.ipsec_public_server_cert_file_path.clone().unwrap_or_default();
        full
    }
}

impl From<&bridge::L2tpVpnConfig> for bridge::SysVpnConfig {
    fn from(config: &bridge::L2tpVpnConfig) -> Self {
        Self {
            addresses: config.addresses.clone(),
            routes: config.routes.clone(),
            dns_addresses: config.dns_addresses.clone(),
            search_domains: config.search_domains.clone(),
            mtu: config.mtu,
            is_i_pv4_accepted: config.is_i_pv4_accepted,
            is_i_pv6_accepted: config.is_i_pv6_accepted,
            is_legacy: config.is_legacy,
            is_blocking: config.is_blocking,
            trusted_applications: config.trusted_applications.clone(),
            blocked_applications: config.blocked_applications.clone(),
            vpn_id: config.vpn_id.clone(),
            vpn_name: config.vpn_name.clone(),
            vpn_type: config.vpn_type,
            user_name: config.user_name.clone(),
            password: config.password.clone(),
            save_login: config.save_login,
            user_id: config.user_id,
            forwarding_routes: config.forwarding_routes.clone(),
            remote_addresses: config.remote_addresses.clone(),
            local_addresses: config.local_addresses.clone(),
            pkcs12_password: config.pkcs12_password.clone(),
            pkcs12_file_data: config.pkcs12_file_data.clone(),
        }
    }
}

impl From<&bridge::L2tpVpnConfig> for ffi::SysVpnConfigFullAni {
    fn from(config: &bridge::L2tpVpnConfig) -> Self {
        let base: bridge::SysVpnConfig = config.into();
        let mut full = empty_full();
        fill_full_base(&mut full, &base);
        // IPsec-common fields (L2tpVpnConfig shares these, minus swanctlConfig)
        full.ipsec_pre_shared_key = config.ipsec_pre_shared_key.clone().unwrap_or_default();
        full.ipsec_identifier = config.ipsec_identifier.clone().unwrap_or_default();
        full.strong_swan_config = config.strong_swan_config.clone().unwrap_or_default();
        full.ipsec_ca_cert_config = config.ipsec_ca_cert_config.clone().unwrap_or_default();
        full.ipsec_private_user_cert_config = config.ipsec_private_user_cert_config.clone().unwrap_or_default();
        full.ipsec_public_user_cert_config = config.ipsec_public_user_cert_config.clone().unwrap_or_default();
        full.ipsec_private_server_cert_config = config.ipsec_private_server_cert_config.clone().unwrap_or_default();
        full.ipsec_public_server_cert_config = config.ipsec_public_server_cert_config.clone().unwrap_or_default();
        full.ipsec_ca_cert_file_path = config.ipsec_ca_cert_file_path.clone().unwrap_or_default();
        full.ipsec_private_user_cert_file_path = config.ipsec_private_user_cert_file_path.clone().unwrap_or_default();
        full.ipsec_public_user_cert_file_path = config.ipsec_public_user_cert_file_path.clone().unwrap_or_default();
        full.ipsec_private_server_cert_file_path = config.ipsec_private_server_cert_file_path.clone().unwrap_or_default();
        full.ipsec_public_server_cert_file_path = config.ipsec_public_server_cert_file_path.clone().unwrap_or_default();
        // L2TP-unique fields
        full.ipsec_config = config.ipsec_config.clone().unwrap_or_default();
        full.ipsec_secrets = config.ipsec_secrets.clone().unwrap_or_default();
        full.options_l2tpd_client = config.options_l2tpd_client.clone().unwrap_or_default();
        full.xl2tpd_config = config.xl2tpd_config.clone().unwrap_or_default();
        full.l2tp_shared_key = config.l2tp_shared_key.clone().unwrap_or_default();
        full
    }
}

impl From<&ffi::SysVpnConfigFullAni> for bridge::VpnConfig {
    fn from(data: &ffi::SysVpnConfigFullAni) -> Self {
        Self {
            vpn_id: opt_string(&data.vpn_id),
            addresses: data.addresses.iter().map(Into::into).collect(),
            routes: Some(data.routes.iter().map(Into::into).collect()),
            dns_addresses: Some(data.dns_addresses.iter().cloned().collect()),
            search_domains: Some(data.search_domains.iter().cloned().collect()),
            mtu: Some(data.mtu),
            is_i_pv4_accepted: Some(data.is_ipv4_accepted),
            is_i_pv6_accepted: Some(data.is_ipv6_accepted),
            is_legacy: Some(data.is_legacy),
            is_blocking: Some(data.is_blocking),
            trusted_applications: Some(data.trusted_applications.iter().cloned().collect()),
            blocked_applications: Some(data.blocked_applications.iter().cloned().collect()),
        }
    }
}

impl From<&ffi::SysVpnConfigFullAni> for bridge::SysVpnConfig {
    fn from(data: &ffi::SysVpnConfigFullAni) -> Self {
        Self {
            addresses: data.addresses.iter().map(Into::into).collect(),
            routes: Some(data.routes.iter().map(Into::into).collect()),
            dns_addresses: Some(data.dns_addresses.iter().cloned().collect()),
            search_domains: Some(data.search_domains.iter().cloned().collect()),
            mtu: Some(data.mtu),
            is_i_pv4_accepted: Some(data.is_ipv4_accepted),
            is_i_pv6_accepted: Some(data.is_ipv6_accepted),
            is_legacy: Some(data.is_legacy),
            is_blocking: Some(data.is_blocking),
            trusted_applications: Some(data.trusted_applications.iter().cloned().collect()),
            blocked_applications: Some(data.blocked_applications.iter().cloned().collect()),
            vpn_id: opt_string(&data.vpn_id),
            vpn_name: opt_string(&data.vpn_name),
            vpn_type: sys_vpn_type_from_ffi(data.vpn_type),
            user_name: opt_string(&data.user_name),
            password: opt_string(&data.password),
            save_login: Some(data.save_login),
            user_id: Some(data.user_id),
            forwarding_routes: opt_string(&data.forwarding_routes),
            remote_addresses: Some(data.remote_addresses.iter().cloned().collect()),
            local_addresses: Some(data.local_addresses.iter().map(Into::into).collect()),
            pkcs12_password: opt_string(&data.pkcs12_password),
            pkcs12_file_data: if data.pkcs12_file_data.is_empty() {
                None
            } else {
                Some(Uint8Array::new_with_vec(data.pkcs12_file_data.clone()))
            },
        }
    }
}

impl From<&ffi::SysVpnConfigFullAni> for bridge::OpenVpnConfig {
    fn from(data: &ffi::SysVpnConfigFullAni) -> Self {
        Self {
            addresses: data.addresses.iter().map(Into::into).collect(),
            routes: Some(data.routes.iter().map(Into::into).collect()),
            dns_addresses: Some(data.dns_addresses.iter().cloned().collect()),
            search_domains: Some(data.search_domains.iter().cloned().collect()),
            mtu: Some(data.mtu),
            is_i_pv4_accepted: Some(data.is_ipv4_accepted),
            is_i_pv6_accepted: Some(data.is_ipv6_accepted),
            is_legacy: Some(data.is_legacy),
            is_blocking: Some(data.is_blocking),
            trusted_applications: Some(data.trusted_applications.iter().cloned().collect()),
            blocked_applications: Some(data.blocked_applications.iter().cloned().collect()),
            vpn_id: opt_string(&data.vpn_id),
            vpn_name: opt_string(&data.vpn_name),
            vpn_type: sys_vpn_type_from_ffi(data.vpn_type),
            user_name: opt_string(&data.user_name),
            password: opt_string(&data.password),
            save_login: Some(data.save_login),
            user_id: Some(data.user_id),
            forwarding_routes: opt_string(&data.forwarding_routes),
            remote_addresses: Some(data.remote_addresses.iter().cloned().collect()),
            local_addresses: Some(data.local_addresses.iter().map(Into::into).collect()),
            pkcs12_password: opt_string(&data.pkcs12_password),
            pkcs12_file_data: if data.pkcs12_file_data.is_empty() {
                None
            } else {
                Some(Uint8Array::new_with_vec(data.pkcs12_file_data.clone()))
            },
            ovpn_port: opt_string(&data.ovpn_port),
            ovpn_protocol: Some(data.ovpn_protocol),
            ovpn_config: opt_string(&data.ovpn_config),
            ovpn_auth_type: Some(data.ovpn_auth_type),
            askpass: opt_string(&data.askpass),
            ovpn_config_file_path: opt_string(&data.ovpn_config_file_path),
            ovpn_ca_cert_file_path: opt_string(&data.ovpn_ca_cert_file_path),
            ovpn_user_cert_file_path: opt_string(&data.ovpn_user_cert_file_path),
            ovpn_private_key_file_path: opt_string(&data.ovpn_private_key_file_path),
        }
    }
}

impl From<&ffi::SysVpnConfigFullAni> for bridge::IpsecVpnConfig {
    fn from(data: &ffi::SysVpnConfigFullAni) -> Self {
        Self {
            addresses: data.addresses.iter().map(Into::into).collect(),
            routes: Some(data.routes.iter().map(Into::into).collect()),
            dns_addresses: Some(data.dns_addresses.iter().cloned().collect()),
            search_domains: Some(data.search_domains.iter().cloned().collect()),
            mtu: Some(data.mtu),
            is_i_pv4_accepted: Some(data.is_ipv4_accepted),
            is_i_pv6_accepted: Some(data.is_ipv6_accepted),
            is_legacy: Some(data.is_legacy),
            is_blocking: Some(data.is_blocking),
            trusted_applications: Some(data.trusted_applications.iter().cloned().collect()),
            blocked_applications: Some(data.blocked_applications.iter().cloned().collect()),
            vpn_id: opt_string(&data.vpn_id),
            vpn_name: opt_string(&data.vpn_name),
            vpn_type: sys_vpn_type_from_ffi(data.vpn_type),
            user_name: opt_string(&data.user_name),
            password: opt_string(&data.password),
            save_login: Some(data.save_login),
            user_id: Some(data.user_id),
            forwarding_routes: opt_string(&data.forwarding_routes),
            remote_addresses: Some(data.remote_addresses.iter().cloned().collect()),
            local_addresses: Some(data.local_addresses.iter().map(Into::into).collect()),
            pkcs12_password: opt_string(&data.pkcs12_password),
            pkcs12_file_data: if data.pkcs12_file_data.is_empty() {
                None
            } else {
                Some(Uint8Array::new_with_vec(data.pkcs12_file_data.clone()))
            },
            ipsec_pre_shared_key: opt_string(&data.ipsec_pre_shared_key),
            ipsec_identifier: opt_string(&data.ipsec_identifier),
            swanctl_config: opt_string(&data.swanctl_config),
            strong_swan_config: opt_string(&data.strong_swan_config),
            ipsec_ca_cert_config: opt_string(&data.ipsec_ca_cert_config),
            ipsec_private_user_cert_config: opt_string(&data.ipsec_private_user_cert_config),
            ipsec_public_user_cert_config: opt_string(&data.ipsec_public_user_cert_config),
            ipsec_private_server_cert_config: opt_string(&data.ipsec_private_server_cert_config),
            ipsec_public_server_cert_config: opt_string(&data.ipsec_public_server_cert_config),
            ipsec_ca_cert_file_path: opt_string(&data.ipsec_ca_cert_file_path),
            ipsec_private_user_cert_file_path: opt_string(&data.ipsec_private_user_cert_file_path),
            ipsec_public_user_cert_file_path: opt_string(&data.ipsec_public_user_cert_file_path),
            ipsec_private_server_cert_file_path: opt_string(&data.ipsec_private_server_cert_file_path),
            ipsec_public_server_cert_file_path: opt_string(&data.ipsec_public_server_cert_file_path),
        }
    }
}

impl From<&ffi::SysVpnConfigFullAni> for bridge::L2tpVpnConfig {
    fn from(data: &ffi::SysVpnConfigFullAni) -> Self {
        Self {
            addresses: data.addresses.iter().map(Into::into).collect(),
            routes: Some(data.routes.iter().map(Into::into).collect()),
            dns_addresses: Some(data.dns_addresses.iter().cloned().collect()),
            search_domains: Some(data.search_domains.iter().cloned().collect()),
            mtu: Some(data.mtu),
            is_i_pv4_accepted: Some(data.is_ipv4_accepted),
            is_i_pv6_accepted: Some(data.is_ipv6_accepted),
            is_legacy: Some(data.is_legacy),
            is_blocking: Some(data.is_blocking),
            trusted_applications: Some(data.trusted_applications.iter().cloned().collect()),
            blocked_applications: Some(data.blocked_applications.iter().cloned().collect()),
            vpn_id: opt_string(&data.vpn_id),
            vpn_name: opt_string(&data.vpn_name),
            vpn_type: sys_vpn_type_from_ffi(data.vpn_type),
            user_name: opt_string(&data.user_name),
            password: opt_string(&data.password),
            save_login: Some(data.save_login),
            user_id: Some(data.user_id),
            forwarding_routes: opt_string(&data.forwarding_routes),
            remote_addresses: Some(data.remote_addresses.iter().cloned().collect()),
            local_addresses: Some(data.local_addresses.iter().map(Into::into).collect()),
            pkcs12_password: opt_string(&data.pkcs12_password),
            pkcs12_file_data: if data.pkcs12_file_data.is_empty() {
                None
            } else {
                Some(Uint8Array::new_with_vec(data.pkcs12_file_data.clone()))
            },
            // IPsec-common fields shared with IpsecVpnConfig (no swanctlConfig).
            ipsec_pre_shared_key: opt_string(&data.ipsec_pre_shared_key),
            ipsec_identifier: opt_string(&data.ipsec_identifier),
            strong_swan_config: opt_string(&data.strong_swan_config),
            ipsec_ca_cert_config: opt_string(&data.ipsec_ca_cert_config),
            ipsec_private_user_cert_config: opt_string(&data.ipsec_private_user_cert_config),
            ipsec_public_user_cert_config: opt_string(&data.ipsec_public_user_cert_config),
            ipsec_private_server_cert_config: opt_string(&data.ipsec_private_server_cert_config),
            ipsec_public_server_cert_config: opt_string(&data.ipsec_public_server_cert_config),
            ipsec_ca_cert_file_path: opt_string(&data.ipsec_ca_cert_file_path),
            ipsec_private_user_cert_file_path: opt_string(&data.ipsec_private_user_cert_file_path),
            ipsec_public_user_cert_file_path: opt_string(&data.ipsec_public_user_cert_file_path),
            ipsec_private_server_cert_file_path: opt_string(&data.ipsec_private_server_cert_file_path),
            ipsec_public_server_cert_file_path: opt_string(&data.ipsec_public_server_cert_file_path),
            // L2TP-unique fields.
            ipsec_config: opt_string(&data.ipsec_config),
            ipsec_secrets: opt_string(&data.ipsec_secrets),
            options_l2tpd_client: opt_string(&data.options_l2tpd_client),
            xl2tpd_config: opt_string(&data.xl2tpd_config),
            l2tp_shared_key: opt_string(&data.l2tp_shared_key),
        }
    }
}

impl From<&ffi::NetAddressAni> for bridge::NetAddress {
    fn from(data: &ffi::NetAddressAni) -> Self {
        Self {
            address: data.address.clone(),
            family: Some(data.family),
            port: Some(data.port),
        }
    }
}

impl From<&ffi::LinkAddressAni> for bridge::LinkAddress {
    fn from(data: &ffi::LinkAddressAni) -> Self {
        Self {
            address: (&data.address).into(),
            prefix_length: data.prefix_length,
        }
    }
}

impl From<&ffi::RouteInfoAni> for bridge::RouteInfo {
    fn from(data: &ffi::RouteInfoAni) -> Self {
        Self {
            iface: data.iface.clone(),
            destination: (&data.destination).into(),
            gateway: (&data.gateway).into(),
            has_gateway: data.has_gateway,
            is_default_route: data.is_default_route,
        }
    }
}

impl bridge::VpnConnectState {
    pub fn from_ffi(info: &ffi::VpnStateInfo) -> Self {
        if info.is_connected {
            bridge::VpnConnectState::VpnConnected
        } else {
            bridge::VpnConnectState::VpnDisconnected
        }
    }
}

pub fn convert_to_business_error(code: i32) -> BusinessError {
    let mut mapped_code = code;
    let error_msg = ffi::GetErrorCodeAndMessage(&mut mapped_code);
    BusinessError::new(mapped_code, error_msg)
}

pub fn get_native_ptr(env: &ani_rs::AniEnv, obj: &ani_rs::objects::AniObject) -> i64 {
    let native_str = unsafe { CStr::from_bytes_with_nul_unchecked(b"nativePtr\0") };
    env.get_field::<i64>(obj, native_str).unwrap_or(0)
}

fn opt_string(s: &str) -> Option<String> {
    if s.is_empty() {
        None
    } else {
        Some(s.to_string())
    }
}

fn sys_vpn_type_from_ffi(vpn_type: i32) -> Option<bridge::SysVpnType> {
    match vpn_type {
        1 => Some(bridge::SysVpnType::Ikev2IpsecMschapv2),
        2 => Some(bridge::SysVpnType::Ikev2IpsecPsk),
        3 => Some(bridge::SysVpnType::Ikev2IpsecRsa),
        4 => Some(bridge::SysVpnType::L2tpIpsecPsk),
        5 => Some(bridge::SysVpnType::L2tpIpsecRsa),
        6 => Some(bridge::SysVpnType::IpsecXauthPsk),
        7 => Some(bridge::SysVpnType::IpsecXauthRsa),
        8 => Some(bridge::SysVpnType::IpsecHybridRsa),
        9 => Some(bridge::SysVpnType::Openvpn),
        _ => None,
    }
}

#[cxx::bridge(namespace = "OHOS::NetManagerAni")]
pub(crate) mod ffi {

    pub struct NetAddressAni {
        pub address: String,
        pub family: i32,
        pub port: i32,
    }

    pub struct LinkAddressAni {
        pub address: NetAddressAni,
        pub prefix_length: i32,
    }

    pub struct RouteInfoAni {
        pub iface: String,
        pub destination: LinkAddressAni,
        pub gateway: NetAddressAni,
        pub has_gateway: bool,
        pub is_default_route: bool,
    }

    pub struct VpnConfigAni {
        pub vpn_id: String,
        pub addresses: Vec<LinkAddressAni>,
        pub routes: Vec<RouteInfoAni>,
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

    pub struct SysVpnConfigFullAni {
        // VpnConfig base
        pub vpn_id: String,
        pub addresses: Vec<LinkAddressAni>,
        pub routes: Vec<RouteInfoAni>,
        pub dns_addresses: Vec<String>,
        pub search_domains: Vec<String>,
        pub mtu: i32,
        pub is_ipv4_accepted: bool,
        pub is_ipv6_accepted: bool,
        pub is_legacy: bool,
        pub is_blocking: bool,
        pub trusted_applications: Vec<String>,
        pub blocked_applications: Vec<String>,
        // SysVpnConfig base
        pub vpn_name: String,
        pub vpn_type: i32,
        pub user_name: String,
        pub password: String,
        pub save_login: bool,
        pub user_id: i32,
        pub forwarding_routes: String,
        pub remote_addresses: Vec<String>,
        pub local_addresses: Vec<LinkAddressAni>,
        pub pkcs12_password: String,
        pub pkcs12_file_data: Vec<u8>,
        // Ipsec-common fields (IpsecVpnConfig / L2tpVpnConfig)
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
        // OpenVPN-specific fields
        pub ovpn_port: String,
        pub ovpn_protocol: i32,
        pub ovpn_config: String,
        pub ovpn_auth_type: i32,
        pub askpass: String,
        pub ovpn_config_file_path: String,
        pub ovpn_ca_cert_file_path: String,
        pub ovpn_user_cert_file_path: String,
        pub ovpn_private_key_file_path: String,
        // L2TP-unique fields
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

        fn CreateVpnConnection(nativePtr: &mut i64) -> i32;
        fn SetUp(nativePtr: i64, config: &SysVpnConfigFullAni, fd: &mut i32) -> i32;
        fn Protect(nativePtr: i64, socketFd: i32) -> i32;
        fn Destroy(nativePtr: i64) -> i32;
        fn AddSysVpnConfig(config: &SysVpnConfigFullAni) -> i32;
        fn DeleteSysVpnConfig(vpnId: &String) -> i32;
        fn g_getSysVpnConfigList(SysVpnConfigFullAniVec: &mut Vec<SysVpnConfigFullAni>) -> i32;
        fn GetSysVpnConfig(vpnId: &String, sysVpnConfigFull: &mut SysVpnConfigFullAni) -> i32;
        fn GetConnectedSysVpnConfig(ret: &mut i32, connectedSysVpnConfigFull: &mut SysVpnConfigFullAni) -> i32;
        fn GetConnectedVpnAppInfo(appInfo: &mut Vec<String>) -> i32;
        fn VpnObserverRegister() -> i32;
        fn VpnObserverUnRegister() -> i32;
        fn GetErrorCodeAndMessage(error_code: &mut i32) -> String;
    }
}
