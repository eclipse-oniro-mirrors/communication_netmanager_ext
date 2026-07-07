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

use crate::bridge;
use ani_rs::business_error::BusinessError;

pub struct VpnExtClient;

impl VpnExtClient {
    pub fn start_vpn_extension_ability(bundle_name: String, ability_name: String) -> Result<i32, i32> {
        let mut ret = 0;
        let result = ffi::StartVpnExtensionAbility(&bundle_name, &ability_name, &mut ret);
        if ret != 0 {
            return Err(ret);
        }
        Ok(result)
    }

    pub fn stop_vpn_extension_ability(bundle_name: String, ability_name: String) -> Result<i32, i32> {
        let mut ret = 0;
        let result = ffi::StopVpnExtensionAbility(&bundle_name, &ability_name, &mut ret);
        if ret != 0 {
            return Err(ret);
        }
        Ok(result)
    }

    pub fn set_always_on_vpn_enabled(enable: bool, bundle_name: String) -> Result<i32, i32> {
        let ret = ffi::SetAlwaysOnVpnEnabled(enable, &bundle_name);
        if ret != 0 {
            return Err(ret);
        }
        Ok(ret)
    }

    pub fn is_always_on_vpn_enabled(bundle_name: String) -> Result<bool, i32> {
        let mut ret = 0;
        let result = ffi::IsAlwaysOnVpnEnabled(&bundle_name, &mut ret);
        if ret != 0 {
            return Err(ret);
        }
        Ok(result)
    }

    pub fn update_vpn_authorized_state(bundle_name: String) -> Result<bool, i32> {
        if ffi::UpdateVpnAuthorizedState(&bundle_name) {
            Ok(true)
        } else {
            Ok(false)
        }
    }

    pub fn create_vpn_connection() -> Result<i32, i32> {
        let mut ret = 0;
        ffi::CreateVpnConnection(&mut ret);
        if ret != 0 {
            return Err(ret);
        }
        Ok(ret)
    }

    pub fn create(config: bridge::VpnConfig) -> Result<i32, i32> {
        let mut fd = -1;
        let config_data: ffi::VpnConfigData = (&config).into();
        let ret = ffi::Create(&config_data, &mut fd);
        if ret != 0 {
            return Err(ret);
        }
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

    pub fn destroy_by_vpn_id(vpn_id: String) -> Result<i32, i32> {
        let ret = ffi::DestroyVpn(&vpn_id);
        if ret != 0 {
            return Err(ret);
        }
        Ok(ret)
    }

    pub fn protect_process_net() -> Result<i32, i32> {
        let ret = ffi::ProtectProcessNet();
        if ret != 0 {
            return Err(ret);
        }
        Ok(ret)
    }

    pub fn generate_vpn_id() -> Result<String, i32> {
        let mut ret = 0;
        let vpn_id = ffi::GenerateVpnId(&mut ret);
        if ret != 0 {
            return Err(ret);
        }
        Ok(vpn_id)
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
            is_internal: config.isInternal.unwrap_or(false),
            is_blocking: config.isBlocking.unwrap_or(false),
            trusted_applications: config.trustedApplications.clone().unwrap_or_default(),
            blocked_applications: config.blockedApplications.clone().unwrap_or_default(),
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
        pub is_internal: bool,
        pub is_blocking: bool,
        pub trusted_applications: Vec<String>,
        pub blocked_applications: Vec<String>,
    }

    extern "Rust" {
        fn execute_vpn_ext_authorization_result(authorized: bool);
    }

    unsafe extern "C++" {
        include!("vpnextension_ani.h");

        fn StartVpnExtensionAbility(
            bundleName: &String,
            abilityName: &String,
            ret: &mut i32,
        ) -> i32;

        fn StopVpnExtensionAbility(
            bundleName: &String,
            abilityName: &String,
            ret: &mut i32,
        ) -> i32;

        fn SetAlwaysOnVpnEnabled(enable: bool, bundleName: &String) -> i32;

        fn IsAlwaysOnVpnEnabled(bundleName: &String, ret: &mut i32) -> bool;

        fn UpdateVpnAuthorizedState(bundleName: &String) -> bool;

        fn CreateVpnConnection(ret: &mut i32) -> bool;

        fn Create(config: &VpnConfigData, fd: &mut i32) -> i32;
        fn Protect(socketFd: i32) -> i32;
        fn Destroy() -> i32;
        fn DestroyVpn(vpnId: &String) -> i32;
        fn ProtectProcessNet() -> i32;
        fn GenerateVpnId(ret: &mut i32) -> String;

        fn GetErrorCodeAndMessage(error_code: &mut i32) -> String;

        fn VpnExtObserverRegister() -> i32;
        fn VpnExtObserverUnRegister() -> i32;
    }
}

pub(crate) fn execute_vpn_ext_authorization_result(authorized: bool) {
    crate::register::execute_vpn_ext_authorization_result(authorized);
}
