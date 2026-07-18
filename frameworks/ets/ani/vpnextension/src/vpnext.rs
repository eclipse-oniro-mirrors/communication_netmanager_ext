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

use ani_rs::business_error::BusinessError;
use ani_rs::objects::{AniObject, AniRef};
use ani_rs::AniEnv;

use crate::{
    bridge,
    wrapper::{convert_to_business_error, VpnExtClient},
};

const VPN_CONNECTION_INNER_CLASS: &CStr =
    unsafe { CStr::from_bytes_with_nul_unchecked(b"@ohos.net.vpnExtension.vpnExtension.VpnConnectionInner\0") };
const VPN_OBSERVER_INNER_CLASS: &CStr =
    unsafe { CStr::from_bytes_with_nul_unchecked(b"@ohos.net.vpnExtension.vpnExtension.VpnObserverInner\0") };
const VPN_INNER_CLASS_CTOR_SIG: &CStr =
    unsafe { CStr::from_bytes_with_nul_unchecked(b"l:\0") };

pub fn get_native_ptr<'local>(env: &AniEnv<'local>, obj: &AniObject) -> i64 {
    let native_str = unsafe { CStr::from_bytes_with_nul_unchecked(b"nativePtr\0") };
    env.get_field::<i64>(obj, native_str).unwrap_or(0)
}

#[ani_rs::native]
pub fn start_vpn_extension_ability<'local>(
    env: &AniEnv<'local>,
    want: AniRef<'local>,
) -> Result<(), BusinessError> {
    let want_obj = AniObject::from(want);
    VpnExtClient::start_vpn_extension_ability(env, &want_obj)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn stop_vpn_extension_ability<'local>(
    env: &AniEnv<'local>,
    want: AniRef<'local>,
) -> Result<(), BusinessError> {
    let want_obj = AniObject::from(want);
    VpnExtClient::stop_vpn_extension_ability(env, &want_obj)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn set_always_on_vpn_enabled(enable: bool, bundle_name: String) -> Result<(), BusinessError> {
    VpnExtClient::set_always_on_vpn_enabled(enable, bundle_name)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn is_always_on_vpn_enabled(bundle_name: String) -> Result<bool, BusinessError> {
    VpnExtClient::is_always_on_vpn_enabled(bundle_name).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn update_vpn_authorized_state(bundle_name: String) -> Result<bool, BusinessError> {
    VpnExtClient::update_vpn_authorized_state(bundle_name)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn create_vpn_connection<'local>(
    env: &AniEnv<'local>,
    context: AniRef<'local>,
) -> Result<AniRef<'local>, BusinessError> {
    let context_obj = AniObject::from(context);
    let native_ptr = VpnExtClient::create_vpn_connection(env, &context_obj).map_err(convert_to_business_error)?;

    let class = env.find_class(VPN_CONNECTION_INNER_CLASS).map_err(|e| {
        BusinessError::new(-1, format!("Failed to find VpnConnectionInner class: {}", e))
    })?;
    let obj = env
        .new_object_with_signature(&class, VPN_INNER_CLASS_CTOR_SIG, (native_ptr,))
        .map_err(|e| {
            BusinessError::new(-1, format!("Failed to create VpnConnectionInner: {}", e))
        })?;
    Ok(obj.into())
}

#[ani_rs::native]
pub(crate) fn destroy_vpn_connection<'local>(
    env: &AniEnv<'local>,
    ani_this: AniObject<'local>,
) -> Result<(), BusinessError> {
    let vpn_connection_ptr = get_native_ptr(&env, &ani_this.into());
    VpnExtClient::destroy_vpn_connection(vpn_connection_ptr).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn create<'local>(
    env: &AniEnv<'local>,
    connection_ptr: AniObject<'local>,
    config: bridge::VpnConfig,
) -> Result<i32, BusinessError> {
    let vpn_connection_ptr = get_native_ptr(&env, &connection_ptr.into());
    VpnExtClient::create(vpn_connection_ptr, config).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn generate_vpn_id<'local>(
    env: &AniEnv<'local>,
    connection_ptr: AniObject<'local>,
) -> Result<String, BusinessError> {
    let vpn_connection_ptr = get_native_ptr(&env, &connection_ptr.into());
    VpnExtClient::generate_vpn_id(vpn_connection_ptr).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn protect<'local>(
    env: &AniEnv<'local>,
    connection_ptr: AniObject<'local>,
    socket_fd: i32,
) -> Result<(), BusinessError> {
    let vpn_connection_ptr = get_native_ptr(&env, &connection_ptr.into());
    VpnExtClient::protect(vpn_connection_ptr, socket_fd).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn destroy<'local>(
    env: &AniEnv<'local>,
    connection_ptr: AniObject<'local>,
    vpn_id_opt: Option<String>,
) -> Result<(), BusinessError> {
    let vpn_connection_ptr = get_native_ptr(&env, &connection_ptr.into());
    let vpn_id = vpn_id_opt.unwrap_or_default();
    VpnExtClient::destroy(vpn_connection_ptr, vpn_id).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn protect_process_net<'local>(
    env: &AniEnv<'local>,
    connection_ptr: AniObject<'local>,
) -> Result<(), BusinessError> {
    let vpn_connection_ptr = get_native_ptr(&env, &connection_ptr.into());
    VpnExtClient::protect_process_net(vpn_connection_ptr).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn create_vpn_observer<'local>(
    env: &AniEnv<'local>,
) -> Result<AniRef<'local>, BusinessError> {
    let native_ptr = VpnExtClient::create_vpn_observer().map_err(convert_to_business_error)?;

    let class = env.find_class(VPN_OBSERVER_INNER_CLASS).map_err(|e| {
        BusinessError::new(-1, format!("Failed to find VpnObserverInner class: {}", e))
    })?;
    let obj = env
        .new_object_with_signature(&class, VPN_INNER_CLASS_CTOR_SIG, (native_ptr,))
        .map_err(|e| {
            BusinessError::new(-1, format!("Failed to create VpnObserverInner: {}", e))
        })?;
    Ok(obj.into())
}

#[ani_rs::native]
pub(crate) fn destroy_vpn_observer<'local>(
    env: &AniEnv<'local>,
    ani_this: AniObject<'local>,
) -> Result<(), BusinessError> {
    let vpn_observer_ptr = get_native_ptr(&env, &ani_this.into());
    crate::register::unregister_all(vpn_observer_ptr);
    VpnExtClient::destroy_vpn_observer(vpn_observer_ptr).map_err(convert_to_business_error)
}