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

#[repr(transparent)]
pub struct AniEnv {
    pub inner: ani_rs::AniEnv<'static>,
}

#[repr(transparent)]
pub struct AniObject {
    pub inner: ani_rs::objects::AniObject<'static>,
}

pub struct VpnExtClient;

impl VpnExtClient {

    pub fn start_vpn_extension_ability<'local>(
        env: &ani_rs::AniEnv<'local>,
        want_object: &ani_rs::objects::AniObject<'local>,
    ) -> Result<(), i32> {
        let env = env as *const ani_rs::AniEnv as *mut ani_rs::AniEnv as *mut *mut AniEnv;
        let want_object = want_object as *const ani_rs::objects::AniObject as *mut ani_rs::objects::AniObject
            as *mut AniObject;
        let ret = unsafe { ffi::StartVpnExtensionAbility(env, want_object) };
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn stop_vpn_extension_ability<'local>(
        env: &ani_rs::AniEnv<'local>,
        want_object: &ani_rs::objects::AniObject<'local>,
    ) -> Result<(), i32> {
        let env = env as *const ani_rs::AniEnv as *mut ani_rs::AniEnv as *mut *mut AniEnv;
        let want_object = want_object as *const ani_rs::objects::AniObject as *mut ani_rs::objects::AniObject
            as *mut AniObject;
        let ret = unsafe { ffi::StopVpnExtensionAbility(env, want_object) };
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn set_always_on_vpn_enabled(enable: bool, bundle_name: String) -> Result<(), i32> {
        let ret = ffi::SetAlwaysOnVpnEnabled(enable, &bundle_name);
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn is_always_on_vpn_enabled(bundle_name: String) -> Result<bool, i32> {
        let mut is_enabled = false;
        let ret = ffi::IsAlwaysOnVpnEnabled(&bundle_name, &mut is_enabled);
        if ret != 0 {
            return Err(ret);
        }
        Ok(is_enabled)
    }

    pub fn update_vpn_authorized_state(bundle_name: String) -> Result<bool, i32> {
        let mut state = false;
        let ret = ffi::UpdateVpnAuthorizedState(&bundle_name, &mut state);
        if ret != 0 {
            return Err(ret);
        }
        Ok(state)
    }

    pub fn create_vpn_connection<'local>(
        env: &ani_rs::AniEnv<'local>,
        context_object: &ani_rs::objects::AniObject<'local>,
    ) -> Result<i64, i32> {
        let env = env as *const ani_rs::AniEnv as *mut ani_rs::AniEnv as *mut *mut AniEnv;
        let context_object = context_object as *const ani_rs::objects::AniObject as *mut ani_rs::objects::AniObject
            as *mut AniObject;
        let mut native_ptr = 0;
        let ret = unsafe { ffi::CreateVpnConnection(env, context_object, &mut native_ptr) };
        if ret != 0 {
            return Err(ret);
        }
        Ok(native_ptr)
    }

    pub fn destroy_vpn_connection(native_ptr: i64) -> Result<(), i32> {
        let ret = ffi::DestroyVpnConnection(native_ptr);
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn create_vpn_observer() -> Result<i64, i32> {
        let mut native_ptr = 0;
        let ret = ffi::CreateVpnObserver(&mut native_ptr);
        if ret != 0 {
            return Err(ret);
        }
        Ok(native_ptr)
    }

    pub fn destroy_vpn_observer(native_ptr: i64) -> Result<(), i32> {
        let ret = ffi::DestroyVpnObserver(native_ptr);
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn create(vpn_connection_ptr: i64, config: bridge::VpnConfig) -> Result<i32, i32> {
        let mut fd: i32 = -1;
        let config_data: ffi::VpnConfigData = (&config).into();
        let ret = ffi::Create(vpn_connection_ptr, &config_data, &mut fd);
        if ret != 0 {
            return Err(ret);
        }
        Ok(fd)
    }

    pub fn generate_vpn_id(vpn_connection_ptr: i64) -> Result<String, i32> {
        let mut vpn_id = String::new();
        let ret = ffi::GenerateVpnId(vpn_connection_ptr, &mut vpn_id);
        if ret != 0 {
            return Err(ret);
        }
        Ok(vpn_id)
    }

    pub fn protect(vpn_connection_ptr: i64, socket_fd: i32) -> Result<(), i32> {
        let ret = ffi::Protect(vpn_connection_ptr, socket_fd);
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn destroy(vpn_connection_ptr: i64, vpn_id: String) -> Result<(), i32> {
        let ret = ffi::DestroyVpn(vpn_connection_ptr, &vpn_id);
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn protect_process_net(vpn_connection_ptr: i64) -> Result<(), i32> {
        let ret = ffi::ProtectProcessNet(vpn_connection_ptr);
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
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
            prefix_length: addr.prefix_length,
        }
    }
}

impl From<&bridge::RouteInfo> for ffi::RouteInfoData {
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

impl From<&bridge::VpnConfig> for ffi::VpnConfigData {
    fn from(config: &bridge::VpnConfig) -> Self {
        Self {
            vpn_id: config.vpn_id.clone().unwrap_or_default(),
            addresses: config.addresses.iter().map(Into::into).collect(),
            routes: config
                .routes
                .as_ref()
                .unwrap_or(&vec![])
                .iter()
                .map(Into::into)
                .collect(),
            dns_addresses: config.dns_addresses.clone().unwrap_or_default(),
            search_domains: config.search_domains.clone().unwrap_or_default(),
            mtu: config.mtu.unwrap_or(1500),
            is_ipv4_accepted: config.is_i_pv4_accepted.unwrap_or(true),
            is_ipv6_accepted: config.is_i_pv6_accepted.unwrap_or(false),
            is_internal: config.is_internal.unwrap_or(false),
            is_blocking: config.is_blocking.unwrap_or(false),
            trusted_applications: config.trusted_applications.clone().unwrap_or_default(),
            blocked_applications: config.blocked_applications.clone().unwrap_or_default(),
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
        type AniEnv;
        type AniObject;

        fn execute_vpn_ext_authorization_result(authorized: bool);
    }

    unsafe extern "C++" {
        include!("vpnextension_ani.h");

        unsafe fn CreateVpnConnection(
            env: *mut *mut AniEnv,
            context: *mut AniObject,
            nativePtr: &mut i64,
        ) -> i32;

        fn DestroyVpnConnection(nativePtr: i64) -> i32;

        fn CreateVpnObserver(nativePtr: &mut i64) -> i32;

        fn DestroyVpnObserver(nativePtr: i64) -> i32;

        unsafe fn StartVpnExtensionAbility(
            env: *mut *mut AniEnv,
            want_object: *mut AniObject,
        ) -> i32;

        unsafe fn StopVpnExtensionAbility(
            env: *mut *mut AniEnv,
            want_object: *mut AniObject,
        ) -> i32;

        fn SetAlwaysOnVpnEnabled(enable: bool, bundleName: &String) -> i32;

        fn IsAlwaysOnVpnEnabled(bundleName: &String, isEnabled: &mut bool) -> i32;

        fn UpdateVpnAuthorizedState(bundleName: &String, state: &mut bool) -> i32;

        fn Create(ptr: i64, config: &VpnConfigData, fd: &mut i32) -> i32;
        fn Protect(ptr: i64, socketFd: i32) -> i32;
        fn DestroyVpn(ptr: i64, vpnId: &String) -> i32;
        fn ProtectProcessNet(ptr: i64) -> i32;
        fn GenerateVpnId(ptr: i64, vpnId: &mut String) -> i32;

        fn GetErrorCodeAndMessage(error_code: &mut i32) -> String;
    }
}

pub(crate) fn execute_vpn_ext_authorization_result(authorized: bool) {
    crate::register::execute_vpn_ext_authorization_result(authorized);
}
