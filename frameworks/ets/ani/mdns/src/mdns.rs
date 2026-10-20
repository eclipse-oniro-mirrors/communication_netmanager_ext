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
    wrapper::{convert_to_business_error, get_native_ptr, MDnsClient},
    context::Context,
};

const DISCOVERY_SERVICE_INNER_CLASS: &CStr =
    unsafe { CStr::from_bytes_with_nul_unchecked(b"@ohos.net.mdns.mdns.DiscoveryServiceInner\0") };
const DISCOVERY_SERVICE_INNER_CTOR_SIG: &CStr =
    unsafe { CStr::from_bytes_with_nul_unchecked(b"l:\0") };

#[ani_rs::native]
pub fn add_local_service(
    env: &AniEnv,
    context: AniRef,
    service_info: bridge::LocalServiceInfo,
) -> Result<bridge::LocalServiceInfo, BusinessError> {
    let context_obj = AniObject::from(context);
    let context = Context::new(env, &context_obj);
    let bundle_name = context.get_bundle_name();
    MDnsClient::add_local_service(bundle_name, service_info).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn remove_local_service(
    env: &AniEnv,
    context: AniRef,
    service_info: bridge::LocalServiceInfo,
) -> Result<bridge::LocalServiceInfo, BusinessError> {
    let context_obj = AniObject::from(context);
    let context = Context::new(env, &context_obj);
    let bundle_name = context.get_bundle_name();
    MDnsClient::remove_local_service(bundle_name, service_info).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn create_discovery_service<'local>(
    env: &AniEnv<'local>,
    context: AniRef<'local>,
    service_type: String,
) -> Result<AniRef<'local>, BusinessError> {
    let context_obj = AniObject::from(context);
    let _context = Context::new(env, &context_obj);

    let native_ptr = MDnsClient::create_discovery_service(service_type)
        .map_err(convert_to_business_error)?;

    let class = env.find_class(DISCOVERY_SERVICE_INNER_CLASS).map_err(|e| {
        BusinessError::new(-1, format!("Failed to find DiscoveryServiceInner class: {}", e))
    })?;
    let obj = env
        .new_object_with_signature(&class, DISCOVERY_SERVICE_INNER_CTOR_SIG, (native_ptr,))
        .map_err(|e| {
            BusinessError::new(-1, format!("Failed to create DiscoveryServiceInner: {}", e))
        })?;
    Ok(obj.into())
}

#[ani_rs::native]
pub fn resolve_local_service(
    env: &AniEnv,
    context: AniRef,
    service_info: bridge::LocalServiceInfo,
) -> Result<bridge::LocalServiceInfo, BusinessError> {
    let context_obj = AniObject::from(context);
    let _context = Context::new(env, &context_obj);
    MDnsClient::resolve_local_service(service_info).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn start_searching_mdns(env: &AniEnv, this: AniObject) -> Result<(), BusinessError> {
    let native_ptr = get_native_ptr(env, &this);
    MDnsClient::start_searching_mdns(native_ptr).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn stop_searching_mdns(env: &AniEnv, this: AniObject) -> Result<(), BusinessError> {
    let native_ptr = get_native_ptr(env, &this);
    MDnsClient::stop_searching_mdns(native_ptr).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn destroy_discovery_service<'local>(
    env: &AniEnv<'local>,
    this: AniRef<'local>,
) -> Result<(), BusinessError> {
    let native_ptr = get_native_ptr(env, &this.into());
    MDnsClient::destroy_discovery_service(native_ptr);
    Ok(())
}
