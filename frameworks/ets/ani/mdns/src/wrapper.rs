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
use cxx::SharedPtr;

use crate::bridge;
use crate::register::*;

#[repr(transparent)]
pub struct AniEnv {
    pub inner: ani_rs::AniEnv<'static>,
}

#[repr(transparent)]
pub struct AniObject {
    pub inner: ani_rs::objects::AniObject<'static>,
}

pub struct MDnsClient;

impl MDnsClient {
    pub fn add_local_service(
        bundle_name: String,
        service_info: bridge::LocalServiceInfo,
    ) -> Result<bridge::LocalServiceInfo, i32> {
        let ffi_info = ffi::MDnsServiceInfoAni::from(service_info);
        let mut ret = 0;
        let result = ffi::RegisterService(&bundle_name, &ffi_info, &mut ret);
        if ret != 0 {
            return Err(ret);
        }
        let bridge_result = bridge::LocalServiceInfo::from(result);
        Ok(bridge_result)
    }

    pub fn remove_local_service(
        bundle_name: String,
        service_info: bridge::LocalServiceInfo,
    ) -> Result<bridge::LocalServiceInfo, i32> {
        let ffi_info = ffi::MDnsServiceInfoAni::from(service_info.clone());
        let mut ret = 0;
        let result = ffi::UnRegisterService(&bundle_name, &ffi_info, &mut ret);
        if ret != 0 {
            return Err(ret);
        }
        let bridge_result = bridge::LocalServiceInfo::from(result);
        Ok(bridge_result)
    }

    pub fn resolve_local_service(
        service_info: bridge::LocalServiceInfo,
    ) -> Result<bridge::LocalServiceInfo, i32> {
        let ffi_info = ffi::MDnsServiceInfoAni::from(service_info.clone());
        let mut ret = 0;
        let result = ffi::ResolveService(&ffi_info, &mut ret);
        if ret != 0 {
            return Err(ret);
        }
        let bridge_result = bridge::LocalServiceInfo::from(result);
        Ok(bridge_result)
    }

    pub fn create_discovery_service(service_type: String) -> Result<i64, i32> {
        Ok(ffi::CreateDiscoveryServiceAni(&service_type))
    }

    pub fn start_searching_mdns(native_ptr: i64) -> Result<(), i32> {
        let ret = ffi::StartSearchingMDNSAni(native_ptr);
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn stop_searching_mdns(native_ptr: i64) -> Result<(), i32> {
        let ret = ffi::StopSearchingMDNSAni(native_ptr);
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn destroy_discovery_service(native_ptr: i64) {
        if native_ptr != 0 {
            ffi::DestroyDiscoveryServiceAni(native_ptr);
        }
    }
}

impl From<bridge::LocalServiceInfo> for ffi::MDnsServiceInfoAni {
    fn from(info: bridge::LocalServiceInfo) -> Self {
        let (addr, family) = if let Some(ref host) = info.host {
            let f = host.family.unwrap_or(-1);
            (host.address.clone(), if f == 1 { 0 } else if f == 2 { 1 } else { -1 })
        } else {
            (String::new(), -1)
        };
        let port = info.port.unwrap_or(-1);

        let (attr_keys, attr_values, attr_value_lengths) = if let Some(ref attrs) = info.service_attribute {
            let mut keys = Vec::with_capacity(attrs.len());
            let mut values = Vec::new();
            let mut lengths = Vec::with_capacity(attrs.len());
            for attr in attrs {
                keys.push(attr.key.clone());
                let bytes: Vec<u8> = attr.value.iter()
                .flat_map(|&v| v.to_le_bytes().to_vec())
                .collect();
                lengths.push(bytes.len() as i32);
                values.extend(bytes);
            }
            (keys, values, lengths)
        } else {
            (Vec::new(), Vec::new(), Vec::new())
        };

        ffi::MDnsServiceInfoAni {
            name: info.service_name,
            type_: info.service_type,
            family,
            addr,
            port,
            attr_keys,
            attr_values,
            attr_value_lengths,
        }
    }
}

impl From<ffi::MDnsServiceInfoAni> for bridge::LocalServiceInfo {
    fn from(info: ffi::MDnsServiceInfoAni) -> Self {
        let valid_port = if (info.port > -1) { Some(info.port) } else { None };
        let valid_address = (!info.addr.is_empty()).then(|| bridge::NetAddress {
            address: info.addr,
            family: (info.family > -1).then_some(info.family + 1),
            port: valid_port,
        });

        let service_attribute = if info.attr_keys.is_empty() {
            None
        } else {
            let mut attrs = Vec::with_capacity(info.attr_keys.len());
            let mut offset: usize = 0;
            for (i, key) in info.attr_keys.iter().enumerate() {
                let len = if i < info.attr_value_lengths.len() {
                    info.attr_value_lengths[i] as usize
                } else {
                    0
                };
                if len == 0 || offset + len > info.attr_values.len() {
                    break;
                }
                let bytes = &info.attr_values[offset..offset + len];
                let value: Vec<i32> = bytes.chunks(4)
                    .filter(|chunk| chunk.len() == 4)
                    .map(|chunk| i32::from_le_bytes(chunk.try_into().unwrap()))
                    .collect();
                attrs.push(bridge::ServiceAttribute {
                    key: key.clone(),
                    value,
                });
                offset += len;
            }
            if attrs.is_empty() { None } else { Some(attrs) }
        };

        bridge::LocalServiceInfo {
            service_type: info.type_,
            service_name: info.name,
            port: valid_port,
            host: valid_address,
            service_attribute,
        }
    }
}

pub fn convert_to_business_error(mut code: i32) -> BusinessError {
    let error_msg = ffi::GetErrorCodeAndMessage(&mut code);
    BusinessError::new(code, error_msg)
}

pub fn get_native_ptr(env: &ani_rs::AniEnv, obj: &ani_rs::objects::AniObject) -> i64 {
    let native_str = unsafe { CStr::from_bytes_with_nul_unchecked(b"nativePtr\0") };
    env.get_field::<i64>(obj, native_str).unwrap_or(0)
}

#[cxx::bridge(namespace = "OHOS::NetManagerStandard")]
pub(crate) mod ffi {

    #[namespace = "OHOS::NetManagerStandard"]
    struct MDnsServiceInfoAni {
        name: String,
        type_: String,
        family: i32,
        addr: String,
        port: i32,
        attr_keys: Vec<String>,
        attr_values: Vec<u8>,
        attr_value_lengths: Vec<i32>,
    }

    extern "Rust" {
        type AniEnv;
        type AniObject;

        fn execute_discovery_start(info: MDnsServiceInfoAni, ret_code: i32, native_ptr: i64);
        fn execute_discovery_stop(info: MDnsServiceInfoAni, ret_code: i32, native_ptr: i64);
        fn execute_service_found(info: MDnsServiceInfoAni, ret_code: i32, native_ptr: i64);
        fn execute_service_lost(info: MDnsServiceInfoAni, ret_code: i32, native_ptr: i64);
    }

    unsafe extern "C++" {
        include!("mdns_ani.h");

        unsafe fn IsStageContext(env: *mut AniEnv, ani_object: *mut AniObject) -> bool;
        unsafe fn GetStageModeContext(
            env: *mut *mut AniEnv,
            ani_object: *mut AniObject,
        ) -> SharedPtr<Context>;
        fn GetBundleName(context: SharedPtr<Context>) -> String;

        fn RegisterService(bundleName: &String, serviceInfo: &MDnsServiceInfoAni, ret: &mut i32)
            -> MDnsServiceInfoAni;
        fn UnRegisterService(bundleName: &String, serviceInfo: &MDnsServiceInfoAni, ret: &mut i32)
            -> MDnsServiceInfoAni;
        fn StartSearchingMDNSAni(nativePtr: i64) -> i32;
        fn StopSearchingMDNSAni(nativePtr: i64) -> i32;
        fn ResolveService(serviceInfo: &MDnsServiceInfoAni, ret: &mut i32)
            -> MDnsServiceInfoAni;
        fn GetErrorCodeAndMessage(error_code: &mut i32) -> String;

        fn CreateDiscoveryServiceAni(serviceType: &String) -> i64;
        fn GetServiceTypeFromPtr(nativePtr: i64) -> String;
        fn DestroyDiscoveryServiceAni(nativePtr: i64);

        #[namespace = "OHOS::AbilityRuntime"]
        type Context;
    }
}
