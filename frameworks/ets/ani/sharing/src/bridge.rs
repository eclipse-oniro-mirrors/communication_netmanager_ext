// Copyright (C) 2025 Huawei Device Co., Ltd.
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

use serde::{Deserialize, Serialize};

#[ani_rs::ani(path = "L@ohos/net/sharing/sharing/SharingIfaceState")]
#[derive(Clone)]
pub enum SharingIfaceState {
    SharingNicServing = 1,

    SharingNicCanServer = 2,

    SharingNicError = 3,
}

#[ani_rs::ani(path = "L@ohos/net/sharing/sharing/SharingIfaceType")]
#[derive(Clone)]
pub enum SharingIfaceType {
    SharingWifi = 0,

    SharingUsb = 1,

    SharingBluetooth = 2,
}

#[derive(Serialize, Deserialize, Clone)]
#[serde(rename = "@ohos.net.sharing.sharing.InterfaceSharingStateInfoInner\0")]
pub struct InterfaceSharingStateInfo {
    #[serde(rename = "type\0")]
    pub share_type: SharingIfaceType,
    #[serde(rename = "iface\0")]
    pub iface: String,
    #[serde(rename = "state\0")]
    pub state: SharingIfaceState,
}

#[ani_rs::ani(path = "L@ohos/net/connection/connection/NetHandleInner")]
#[derive(Clone)]
pub struct NetHandle {
    pub net_id: i32,
}
