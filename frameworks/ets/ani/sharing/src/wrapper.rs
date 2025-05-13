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

use cxx::UniquePtr;

use crate::bridge;
use crate::register::SharingCallback;

pub struct SharingClient;

impl SharingClient {
    pub fn is_sharing_supported() -> Result<bool, i32> {
        let mut ret = 0;
        let is_share = ffi::IsSharingSupported(&mut ret);
        if ret != 0 {
            return Err(ret);
        }
        Ok(is_share)
    }

    pub fn is_sharing() -> Result<bool, i32> {
        let mut ret = 0;
        let is_share = ffi::IsSharing(&mut ret);
        if ret != 0 {
            return Err(ret);
        }
        Ok(is_share)
    }

    pub fn start_sharing(share_type: bridge::SharingIfaceType) -> Result<i32, i32> {
        let ret = ffi::StartSharing(&share_type.into());
        if ret != 0 {
            return Err(ret);
        }
        Ok(ret)
    }

    pub fn stop_sharing(share_type: bridge::SharingIfaceType) -> Result<i32, i32> {
        let ret = ffi::StopSharing(&share_type.into());
        if ret != 0 {
            return Err(ret);
        }
        Ok(ret)
    }

    pub fn get_stats_rx_bytes() -> Result<i32, i32> {
        let mut bytes = 0;
        let ret = ffi::GetStatsRxBytes(&mut bytes);
        if ret != 0 {
            return Err(ret);
        }
        Ok(bytes)
    }

    pub fn get_stats_tx_bytes() -> Result<i32, i32> {
        let mut bytes = 0;
        let ret = ffi::GetStatsTxBytes(&mut bytes);
        if ret != 0 {
            return Err(ret);
        }
        Ok(bytes)
    }

    pub fn get_stats_total_bytes() -> Result<i32, i32> {
        let mut bytes = 0;
        let ret = ffi::GetStatsTotalBytes(&mut bytes);
        if ret != 0 {
            return Err(ret);
        }
        Ok(bytes)
    }

    pub fn get_sharing_ifaces(state: bridge::SharingIfaceState) -> Result<Vec<String>, i32> {
        let mut ifaces = Vec::new();
        let ret = ffi::GetSharingIfaces(&state.into(), &mut ifaces);
        if ret != 0 {
            return Err(ret);
        }
        Ok(ifaces)
    }

    pub fn get_sharing_state(
        share_type: bridge::SharingIfaceType,
    ) -> Result<bridge::SharingIfaceState, i32> {
        let mut state = ffi::SharingIfaceState::SHARING_NIC_CAN_SERVER;
        let ret = ffi::GetSharingState(&share_type.into(), &mut state);
        if ret != 0 {
            return Err(ret);
        }
        Ok(state.into())
    }

    pub fn get_sharable_regexs(share_type: bridge::SharingIfaceType) -> Result<Vec<String>, i32> {
        let mut iface_regexs = Vec::new();
        let ret = ffi::GetSharableRegexs(&share_type.into(), &mut iface_regexs);
        if ret != 0 {
            return Err(ret);
        }
        Ok(iface_regexs)
    }

    pub fn register_sharing_callback(
        callback: SharingCallback,
    ) -> Result<SharingCallbackUnregister, i32> {
        let mut ret = 0;
        let unregister = ffi::RegisterSharingCallback(Box::new(callback), &mut ret);
        if ret != 0 {
            return Err(ret);
        }
        Ok(SharingCallbackUnregister::new(unregister))
    }
}

pub struct SharingCallbackUnregister {
    inner: UniquePtr<ffi::SharingCallbackUnregister>,
}

unsafe impl Send for SharingCallbackUnregister {}
unsafe impl Sync for SharingCallbackUnregister {}

impl SharingCallbackUnregister {
    fn new(inner: UniquePtr<ffi::SharingCallbackUnregister>) -> Self {
        Self { inner }
    }

    pub fn unregister(&mut self) -> Result<(), i32> {
        let ret = self.inner.pin_mut().Unregister();
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }
}

impl From<bridge::SharingIfaceType> for ffi::SharingIfaceType {
    fn from(share_type: bridge::SharingIfaceType) -> Self {
        match share_type {
            bridge::SharingIfaceType::SharingWifi => ffi::SharingIfaceType::SHARING_WIFI,
            bridge::SharingIfaceType::SharingUsb => ffi::SharingIfaceType::SHARING_USB,
            bridge::SharingIfaceType::SharingBluetooth => ffi::SharingIfaceType::SHARING_BLUETOOTH,
        }
    }
}

impl From<bridge::SharingIfaceState> for ffi::SharingIfaceState {
    fn from(state: bridge::SharingIfaceState) -> Self {
        match state {
            bridge::SharingIfaceState::SharingNicServing => {
                ffi::SharingIfaceState::SHARING_NIC_SERVING
            }
            bridge::SharingIfaceState::SharingNicCanServer => {
                ffi::SharingIfaceState::SHARING_NIC_CAN_SERVER
            }
            bridge::SharingIfaceState::SharingNicError => ffi::SharingIfaceState::SHARING_NIC_ERROR,
        }
    }
}

impl From<ffi::SharingIfaceState> for bridge::SharingIfaceState {
    fn from(state: ffi::SharingIfaceState) -> Self {
        match state {
            ffi::SharingIfaceState::SHARING_NIC_SERVING => {
                bridge::SharingIfaceState::SharingNicServing
            }
            ffi::SharingIfaceState::SHARING_NIC_CAN_SERVER => {
                bridge::SharingIfaceState::SharingNicCanServer
            }
            ffi::SharingIfaceState::SHARING_NIC_ERROR => bridge::SharingIfaceState::SharingNicError,
            _ => unimplemented!(),
        }
    }
}

impl From<ffi::SharingIfaceType> for bridge::SharingIfaceType {
    fn from(share_type: ffi::SharingIfaceType) -> Self {
        match share_type {
            ffi::SharingIfaceType::SHARING_WIFI => bridge::SharingIfaceType::SharingWifi,
            ffi::SharingIfaceType::SHARING_USB => bridge::SharingIfaceType::SharingUsb,
            ffi::SharingIfaceType::SHARING_BLUETOOTH => bridge::SharingIfaceType::SharingBluetooth,
            _ => unimplemented!(),
        }
    }
}

impl From<ffi::InterfaceSharingStateInfo> for bridge::InterfaceSharingStateInfo {
    fn from(info: ffi::InterfaceSharingStateInfo) -> Self {
        bridge::InterfaceSharingStateInfo {
            share_type: info.share_type.into(),
            iface: info.iface,
            state: info.state.into(),
        }
    }
}

impl From<ffi::NetHandle> for bridge::NetHandle {
    fn from(handle: ffi::NetHandle) -> Self {
        bridge::NetHandle {
            net_id: handle.net_id,
        }
    }
}

#[cxx::bridge(namespace = "OHOS::NetManagerAni")]
pub(crate) mod ffi {

    #[namespace = "OHOS::NetManagerStandard"]
    #[repr(i32)]
    enum SharingIfaceType {
        SHARING_NONE = -1,
        SHARING_WIFI,
        SHARING_USB,
        SHARING_BLUETOOTH,
    }

    #[namespace = "OHOS::NetManagerStandard"]
    #[repr(i32)]
    enum SharingIfaceState {
        SHARING_NIC_SERVING = 1,
        SHARING_NIC_CAN_SERVER,
        SHARING_NIC_ERROR,
    }

    pub struct InterfaceSharingStateInfo {
        pub share_type: SharingIfaceType,
        pub iface: String,
        pub state: SharingIfaceState,
    }

    pub struct NetHandle {
        pub net_id: i32,
    }

    extern "Rust" {
        type SharingCallback;

        fn on_sharing_state_change(self: &SharingCallback, is_running: bool);
        fn on_interface_sharing_state_change(
            self: &SharingCallback,
            info: InterfaceSharingStateInfo,
        );

        fn on_sharing_upstream_change(self: &SharingCallback, net_handle: NetHandle);
    }

    unsafe extern "C++" {
        include!("sharing_ani.h");

        #[namespace = "OHOS::NetManagerStandard"]
        type SharingIfaceType;
        #[namespace = "OHOS::NetManagerStandard"]
        type SharingIfaceState;

        type SharingCallbackUnregister;

        fn Unregister(self: &SharingCallbackUnregister) -> i32;

        fn IsSharingSupported(ret: &mut i32) -> bool;
        fn IsSharing(ret: &mut i32) -> bool;
        fn StartSharing(share_type: &SharingIfaceType) -> i32;
        fn StopSharing(share_type: &SharingIfaceType) -> i32;
        fn GetStatsRxBytes(bytes: &mut i32) -> i32;
        fn GetStatsTxBytes(bytes: &mut i32) -> i32;
        fn GetStatsTotalBytes(bytes: &mut i32) -> i32;
        fn GetSharingIfaces(state: &SharingIfaceState, ifaces: &mut Vec<String>) -> i32;
        fn GetSharingState(share_type: &SharingIfaceType, state: &mut SharingIfaceState) -> i32;
        fn GetSharableRegexs(share_type: &SharingIfaceType, ifaceRegexs: &mut Vec<String>) -> i32;

        fn RegisterSharingCallback(
            callback: Box<SharingCallback>,
            ret: &mut i32,
        ) -> UniquePtr<SharingCallbackUnregister>;
    }
}
