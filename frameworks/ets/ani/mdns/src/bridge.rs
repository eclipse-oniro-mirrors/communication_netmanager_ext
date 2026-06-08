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

#[ani_rs::ani(path = "@ohos.net.mdns.mdns.ServiceAttribute")]
#[derive(Clone)]
pub struct ServiceAttribute {
    pub key: String,
    pub value: Vec<u8>,
}

#[ani_rs::ani(path = "@ohos.net.mdns.mdns.LocalServiceInfo")]
#[derive(Clone)]
pub struct LocalServiceInfo {
    pub service_type: String,
    pub service_name: String,
    pub port: Option<i32>,
    pub host: Option<NetAddress>,
    pub service_attribute: Option<Vec<ServiceAttribute>>,
}

#[ani_rs::ani(path = "@ohos.net.mdns.mdns.NetAddress")]
#[derive(Clone)]
pub struct NetAddress {
    pub address: String,
    pub family: Option<i32>,
    pub port: Option<i32>,
}

#[ani_rs::ani(path = "@ohos.net.mdns.mdns.DiscoveryEventInfo")]
#[derive(Clone)]
pub struct DiscoveryEventInfo {
    pub service_info: LocalServiceInfo,
    pub error_code: Option<i32>,
}

#[ani_rs::ani(path = "application.Context")]
#[derive(Clone)]
pub struct Context {
    pub application_info: ApplicationInfo,
}

#[ani_rs::ani(path = "application.ApplicationInfo")]
#[derive(Clone)]
pub struct ApplicationInfo {
    pub name: String,
}
