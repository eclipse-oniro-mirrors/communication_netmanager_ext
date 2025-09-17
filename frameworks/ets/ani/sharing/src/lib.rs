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

mod bridge;
mod register;
mod sharing;
mod wrapper;

ani_rs::ani_constructor! {
    namespace "@ohos.net.sharing.sharing"
    [
        "isSharingSupportedSync": sharing::is_sharing_supported,
        "isSharingSync": sharing::is_sharing,
        "startSharingSync": sharing::start_sharing,
        "stopSharingSync" : sharing::stop_sharing,
        "getStatsRxBytesSync": sharing::get_stats_rx_bytes,
        "getStatsTxBytesSync": sharing::get_stats_tx_bytes,
        "getStatsTotalBytesSync": sharing::get_stats_total_bytes,
        "getSharingIfacesSync": sharing::get_sharing_ifaces,
        "getSharingStateSync": sharing::get_sharing_state,
        "getSharableRegexesSync": sharing::get_sharable_regexs,
        "onSharingStateChange": register::on_sharing_state_change,
        "onInterfaceSharingStateChange": register::on_interface_sharing_state_change,
        "onSharingUpstreamChange": register::on_sharing_upstream_change,
        "offSharingStateChange": register::off_sharing_state_change,
        "offInterfaceSharingStateChange": register::off_interface_sharing_state_change,
        "offSharingUpstreamChange": register::off_sharing_upstream_change,
    ]
}
