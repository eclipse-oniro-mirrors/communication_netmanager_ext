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
use crate::register::*;
use ani_rs::business_error::BusinessError;

pub struct MDnsClient;

impl MDnsClient {
    pub fn add_local_service(
        bundle_name: String,
        service_info: bridge::LocalServiceInfo,
    ) -> Result<bridge::LocalServiceInfo, i32> {
        let ffi_info = ffi::MDnsServiceInfoFFI::from(service_info);
        if ffi_info.port <= 0 || ffi_info.port > 65535 {
            return Err(-1);
        }
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
        let ffi_info = ffi::MDnsServiceInfoFFI::from(service_info.clone());
        if ffi_info.port <= 0 || ffi_info.port > 65535 {
            return Err(-1);
        }
        let mut ret = 0;
        let _ = ffi::UnRegisterService(&bundle_name, &ffi_info, &mut ret);
        if ret != 0 {
            return Err(ret);
        }
        let bridge_result = bridge::LocalServiceInfo::from(ffi::MDnsServiceInfoFFI::from(service_info));
        Ok(bridge_result)
    }

    pub fn resolve_local_service(
        service_info: bridge::LocalServiceInfo,
    ) -> Result<bridge::LocalServiceInfo, i32> {
        let ffi_info = ffi::MDnsServiceInfoFFI::from(service_info.clone());
        let mut ret = 0;
        let result = ffi::ResolveService(&ffi_info, &mut ret);
        if ret != 0 {
            return Err(ret);
        }
        let bridge_result = bridge::LocalServiceInfo::from(result);
        Ok(bridge_result)
    }

    pub fn start_searching_mdns(service_type: String) -> Result<(), i32> {
        if service_type.is_empty() || !service_type.starts_with('_') {
            return Err(-1);
        }
        let mut ret = 0;
        let _ = ffi::StartDiscoverService(&service_type, &mut ret);
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn stop_searching_mdns(service_type: String) -> Result<(), i32> {
        if service_type.is_empty() || !service_type.starts_with('_') {
            return Err(-1);
        }
        let mut ret = 0;
        let _ = ffi::StopDiscoverService(&service_type, &mut ret);
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn register_mdns_observer() -> Result<(), i32> {
        let res = ffi::MDnsObserverRegister();
        if res != 0 {
            return Err(res);
        }
        Ok(())
    }

    pub fn unregister_mdns_observer() -> Result<(), i32> {
        let res = ffi::MDnsObserverUnRegister();
        if res != 0 {
            return Err(res);
        }
        Ok(())
    }
}

// ---- Type conversions between bridge (ArkTS) and ffi (C++) ----

impl From<bridge::LocalServiceInfo> for ffi::MDnsServiceInfoFFI {
    fn from(info: bridge::LocalServiceInfo) -> Self {
        let (addr, family, host_port) = if let Some(ref host) = info.host {
            (host.address.clone(), host.family.unwrap_or(0), host.port.unwrap_or(0))
        } else {
            (String::new(), 0, 0)
        };
        let port = info.port.unwrap_or(host_port);

        let (attr_keys, attr_values, attr_value_lengths) = if let Some(ref attrs) = info.service_attribute {
            let mut keys = Vec::with_capacity(attrs.len());
            let mut values = Vec::new();
            let mut lengths = Vec::with_capacity(attrs.len());
            for attr in attrs {
                keys.push(attr.key.clone());
                lengths.push(attr.value.len() as i32);
                values.extend_from_slice(&attr.value);
            }
            (keys, values, lengths)
        } else {
            (Vec::new(), Vec::new(), Vec::new())
        };

        ffi::MDnsServiceInfoFFI {
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

impl From<ffi::MDnsServiceInfoFFI> for bridge::LocalServiceInfo {
    fn from(info: ffi::MDnsServiceInfoFFI) -> Self {
        let valid_port = if info.port > 0 { Some(info.port) } else { None };

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
                let value = info.attr_values[offset..offset + len].to_vec();
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
            host: Some(bridge::NetAddress {
                address: info.addr,
                family: Some(info.family),
                port: valid_port,
            }),
            service_attribute,
        }
    }
}

pub fn convert_to_business_error(mut code: i32) -> BusinessError {
    let error_msg = ffi::GetErrorCodeAndMessage(&mut code);
    BusinessError::new(code, error_msg)
}

#[cxx::bridge(namespace = "OHOS::NetManagerAni")]
pub(crate) mod ffi {

    #[namespace = "OHOS::NetManagerAni"]
    struct MDnsServiceInfoFFI {
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
        fn execute_discovery_start(info: MDnsServiceInfoFFI, ret_code: i32);
        fn execute_discovery_stop(info: MDnsServiceInfoFFI, ret_code: i32);
        fn execute_service_found(info: MDnsServiceInfoFFI, ret_code: i32);
        fn execute_service_lost(info: MDnsServiceInfoFFI, ret_code: i32);
        fn execute_resolve_result(info: MDnsServiceInfoFFI, ret_code: i32);
        fn execute_register_result(info: MDnsServiceInfoFFI, ret_code: i32);
    }

    unsafe extern "C++" {
        include!("mdns_ani.h");

        #[namespace = "OHOS::NetManagerAni"]
        type MDnsServiceInfoFFI;

        fn RegisterService(bundleName: &String, serviceInfo: &MDnsServiceInfoFFI, ret: &mut i32)
            -> MDnsServiceInfoFFI;
        fn UnRegisterService(bundleName: &String, serviceInfo: &MDnsServiceInfoFFI, ret: &mut i32) -> i32;
        fn StartDiscoverService(serviceType: &String, ret: &mut i32) -> i32;
        fn StopDiscoverService(serviceType: &String, ret: &mut i32) -> i32;
        fn ResolveService(serviceInfo: &MDnsServiceInfoFFI, ret: &mut i32)
            -> MDnsServiceInfoFFI;
        fn MDnsObserverRegister() -> i32;
        fn MDnsObserverUnRegister() -> i32;
        fn GetErrorCodeAndMessage(error_code: &mut i32) -> String;
    }
}
