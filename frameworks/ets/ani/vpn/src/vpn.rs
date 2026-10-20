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

use ani_rs::business_error::BusinessError;
use ani_rs::objects::{AniObject, AniRef};
use ani_rs::AniEnv;

use crate::{
    bridge,
    wrapper::{convert_to_business_error, get_native_ptr, ffi, VpnClient},
};

const VPN_CONNECTION_INNER_CLASS: &CStr =
    unsafe { CStr::from_bytes_with_nul_unchecked(b"@ohos.net.vpn.vpn.VpnConnectionInner\0") };
const VPN_CONNECTION_INNER_CTOR_SIG: &CStr =
    unsafe { CStr::from_bytes_with_nul_unchecked(b"l:\0") };

const VPN_TYPE_PROP: &CStr = unsafe { CStr::from_bytes_with_nul_unchecked(b"vpnType\0") };

#[ani_rs::native]
pub fn create_vpn_connection<'local>(
    env: &AniEnv<'local>,
    context: AniRef<'local>,
) -> Result<AniRef<'local>, BusinessError> {
    // context unused
    let _ = context;

    let native_ptr = VpnClient::create_vpn_connection().map_err(convert_to_business_error)?;

    let class = env.find_class(VPN_CONNECTION_INNER_CLASS).map_err(|e| {
        BusinessError::new(-1, format!("Failed to find VpnConnectionInner class: {}", e))
    })?;
    let obj = env
        .new_object_with_signature(&class, VPN_CONNECTION_INNER_CTOR_SIG, (native_ptr,))
        .map_err(|e| {
            BusinessError::new(-1, format!("Failed to create VpnConnectionInner: {}", e))
        })?;
    Ok(obj.into())
}

#[ani_rs::native]
pub fn set_up<'local>(
    env: &AniEnv<'local>,
    this: AniObject<'local>,
    config: AniRef<'local>,
) -> Result<i32, BusinessError> {
    let native_ptr = get_native_ptr(env, &this);
    let config_obj = AniObject::from(config);
    let full: ffi::SysVpnConfigFullAni = if has_vpn_type(env, &config_obj)? {
        flatten_sys_vpn_config(env, &config_obj)?
    } else {
        let cfg: bridge::VpnConfig = env.deserialize(config_obj).map_err(|e| {
            BusinessError::new(-1, format!("Failed to deserialize VpnConfig: {}", e))
        })?;
        ffi::SysVpnConfigFullAni::from(&cfg)
    };
    VpnClient::set_up(native_ptr, full)
        .map(|fd: RawFd| fd as i32)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn protect(env: &AniEnv, this: AniObject, socket_fd: i32) -> Result<(), BusinessError> {
    let native_ptr = get_native_ptr(env, &this);
    VpnClient::protect(native_ptr, socket_fd).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn destroy(env: &AniEnv, this: AniObject) -> Result<(), BusinessError> {
    let native_ptr = get_native_ptr(env, &this);
    VpnClient::destroy(native_ptr).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn add_sys_vpn_config<'local>(
    env: &AniEnv<'local>,
    config: AniRef<'local>,
) -> Result<(), BusinessError> {
    let config_obj = AniObject::from(config);
    let full = flatten_sys_vpn_config(env, &config_obj)?;
    VpnClient::add_sys_vpn_config(full).map_err(convert_to_business_error)
}

fn flatten_sys_vpn_config<'local>(
    env: &AniEnv<'local>,
    config_obj: &AniObject<'local>,
) -> Result<ffi::SysVpnConfigFullAni, BusinessError> {
    let vpn_type = peek_vpn_type(env, config_obj)?;
    match vpn_type {
        bridge::SysVpnType::Openvpn => {
            let cfg: bridge::OpenVpnConfig = env.deserialize(config_obj.clone()).map_err(|e| {
                BusinessError::new(-1, format!("Failed to deserialize OpenVpnConfig: {}", e))
            })?;
            Ok(ffi::SysVpnConfigFullAni::from(&cfg))
        }
        bridge::SysVpnType::L2tpIpsecPsk | bridge::SysVpnType::L2tpIpsecRsa => {
            let cfg: bridge::L2tpVpnConfig = env.deserialize(config_obj.clone()).map_err(|e| {
                BusinessError::new(-1, format!("Failed to deserialize L2tpVpnConfig: {}", e))
            })?;
            Ok(ffi::SysVpnConfigFullAni::from(&cfg))
        }
        bridge::SysVpnType::Ikev2IpsecMschapv2 | bridge::SysVpnType::Ikev2IpsecPsk |
        bridge::SysVpnType::Ikev2IpsecRsa | bridge::SysVpnType::IpsecXauthPsk |
        bridge::SysVpnType::IpsecXauthRsa | bridge::SysVpnType::IpsecHybridRsa => {
            let cfg: bridge::IpsecVpnConfig = env.deserialize(config_obj.clone()).map_err(|e| {
                BusinessError::new(-1, format!("Failed to deserialize IpsecVpnConfig: {}", e))
            })?;
            Ok(ffi::SysVpnConfigFullAni::from(&cfg))
        }
        _ => Err(BusinessError::new(-1, "Unsupported VPN type".to_string())),
    }
}

fn has_vpn_type<'local>(
    env: &AniEnv<'local>,
    config: &AniObject<'local>,
) -> Result<bool, BusinessError> {
    let vpn_id_ref: AniRef<'local> = env
        .get_property(config, VPN_TYPE_PROP)
        .map_err(|e| BusinessError::new(-1, format!("Failed to get vpnType: {}", e)))?;
    env.is_undefined(&vpn_id_ref)
        .map(|is_undef| !is_undef)
        .map_err(|e| BusinessError::new(-1, format!("Failed to check vpnType: {}", e)))
}

fn peek_vpn_type<'local>(
    env: &AniEnv<'local>,
    config: &AniObject<'local>,
) -> Result<bridge::SysVpnType, BusinessError> {
    let vpn_type_ref: AniRef<'local> = env
        .get_property(config, VPN_TYPE_PROP)
        .map_err(|e| BusinessError::new(-1, format!("Failed to get vpnType: {}", e)))?;
    if env
        .is_undefined(&vpn_type_ref)
        .map_err(|e| BusinessError::new(-1, format!("Failed to check vpnType: {}", e)))?
    {
        return Err(BusinessError::new(
            -1,
            "vpnType is not set on the config object".to_string(),
        ));
    }
    let vpn_type_obj = AniObject::from(vpn_type_ref);
    env.deserialize(vpn_type_obj)
        .map_err(|e| BusinessError::new(-1, format!("Failed to deserialize vpnType: {}", e)))
}

#[ani_rs::native]
pub fn delete_sys_vpn_config(vpn_id: String) -> Result<(), BusinessError> {
    VpnClient::delete_sys_vpn_config(vpn_id).map_err(convert_to_business_error)
}

fn serialize_sys_vpn_config<'local>(
    env: &AniEnv<'local>,
    data: &ffi::SysVpnConfigFullAni,
) -> Result<AniRef<'local>, BusinessError> {
    let vpn_type = data.vpn_type;
    let ani_ref = if vpn_type == bridge::SysVpnType::Openvpn as i32 {
        let config: bridge::OpenVpnConfig = data.into();
        env.serialize(&config)
    } else if vpn_type == bridge::SysVpnType::L2tpIpsecPsk as i32
        || vpn_type == bridge::SysVpnType::L2tpIpsecRsa as i32
    {
        let config: bridge::L2tpVpnConfig = data.into();
        env.serialize(&config)
    } else if vpn_type >= 1 && vpn_type <= 8 {
        let config: bridge::IpsecVpnConfig = data.into();
        env.serialize(&config)
    } else {
        let config: bridge::SysVpnConfig = data.into();
        env.serialize(&config)
    };
    ani_ref.map_err(|e| {
        BusinessError::new(-1, format!("Failed to serialize VPN config: {}", e))
    })
}

#[ani_rs::native]
pub fn get_sys_vpn_config_list<'local>(
    env: &AniEnv<'local>,
) -> Result<Vec<AniRef<'local>>, BusinessError> {
    let configs = VpnClient::get_sys_vpn_config_list().map_err(convert_to_business_error)?;
    configs
        .iter()
        .map(|c| serialize_sys_vpn_config(env, c))
        .collect()
}

#[ani_rs::native]
pub fn get_sys_vpn_config<'local>(
    env: &AniEnv<'local>,
    vpn_id: String,
) -> Result<AniRef<'local>, BusinessError> {
    let config = VpnClient::get_sys_vpn_config(vpn_id).map_err(convert_to_business_error)?;
    serialize_sys_vpn_config(env, &config)
}

#[ani_rs::native]
pub fn get_connected_sys_vpn_config<'local>(
    env: &AniEnv<'local>,
) -> Result<AniRef<'local>, BusinessError> {
    let config =
        VpnClient::get_connected_sys_vpn_config().map_err(convert_to_business_error)?;
    serialize_sys_vpn_config(env, &config)
}

#[ani_rs::native]
pub fn get_connected_vpn_app_info() -> Result<Vec<String>, BusinessError> {
    VpnClient::get_connected_vpn_app_info().map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn destroy_vpn_connection<'local>(
    env: &AniEnv<'local>,
    this: AniRef<'local>,
) -> Result<(), BusinessError> {
    let native_ptr = get_native_ptr(env, &this.into());
    VpnClient::destroy(native_ptr).map_err(convert_to_business_error)
}