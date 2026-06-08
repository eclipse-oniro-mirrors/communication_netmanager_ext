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

use ani_rs::business_error::BusinessError;

use crate::{
    bridge,
    wrapper::{convert_to_business_error, MDnsClient},
};

#[ani_rs::native]
pub fn add_local_service(
    context: bridge::Context,
    service_info: bridge::LocalServiceInfo,
) -> Result<bridge::LocalServiceInfo, BusinessError> {
    let bundle_name = context.application_info.name;
    MDnsClient::add_local_service(bundle_name, service_info).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn remove_local_service(
    context: bridge::Context,
    service_info: bridge::LocalServiceInfo,
) -> Result<bridge::LocalServiceInfo, BusinessError> {
    let bundle_name = context.application_info.name;
    MDnsClient::remove_local_service(bundle_name, service_info).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn resolve_local_service(
    context: bridge::Context,
    service_info: bridge::LocalServiceInfo,
) -> Result<bridge::LocalServiceInfo, BusinessError> {
    MDnsClient::resolve_local_service(service_info).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn start_searching_mdns(service_type: String) -> Result<(), BusinessError> {
    MDnsClient::start_searching_mdns(service_type).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn stop_searching_mdns(service_type: String) -> Result<(), BusinessError> {
    MDnsClient::stop_searching_mdns(service_type).map_err(convert_to_business_error)
}
