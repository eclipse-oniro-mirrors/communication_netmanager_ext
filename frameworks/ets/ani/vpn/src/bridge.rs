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

#[ani_rs::ani(path = "@ohos.net.vpn.vpn.VpnConnectState")]
#[derive(Clone)]
pub enum VpnConnectState {
    Connected = 0,
    Disconnected = 1,
}

#[ani_rs::ani(path = "@ohos.net.vpn.vpn.MultiVpnConnectState")]
#[derive(Clone)]
pub struct MultiVpnConnectState {
    pub is_connected: bool,
    pub bundle_name: String,
    pub vpn_id: String,
}

#[ani_rs::ani(path = "@ohos.net.connection.connection.NetAddressInner")]
#[derive(Clone)]
pub struct NetAddress {
    pub address: String,
    pub family: Option<i32>,
    pub port: Option<i32>,
}

#[ani_rs::ani(path = "@ohos.net.connection.connection.LinkAddressInner")]
#[derive(Clone)]
pub struct LinkAddress {
    pub address: NetAddress,
    pub prefixLength: i32,
}

#[ani_rs::ani(path = "@ohos.net.connection.connection.RouteInfoInner")]
#[derive(Clone)]
pub struct RouteInfo {
    pub iface: String,
    pub destination: LinkAddress,
    pub gateway: NetAddress,
    pub hasGateway: bool,
    pub isDefaultRoute: bool,
}

#[ani_rs::ani(path = "@ohos.net.vpn.vpn.VpnConfigInner")]
#[derive(Clone)]
pub struct VpnConfig {
    pub vpnId: Option<String>,
    pub addresses: Vec<LinkAddress>,
    pub routes: Option<Vec<RouteInfo>>,
    pub dnsAddresses: Option<Vec<String>>,
    pub searchDomains: Option<Vec<String>>,
    pub mtu: Option<i32>,
    pub isIPv4Accepted: Option<bool>,
    pub isIPv6Accepted: Option<bool>,
    pub isLegacy: Option<bool>,
    pub isBlocking: Option<bool>,
    pub trustedApplications: Option<Vec<String>>,
    pub blockedApplications: Option<Vec<String>>,
}

#[ani_rs::ani(path = "@ohos.net.vpn.vpn.IpsecVpnConfigInner")]
#[derive(Clone)]
pub struct IpsecVpnConfig {
    pub vpnId: Option<String>,
    pub vpnName: Option<String>,
    pub vpnType: Option<SysVpnType>,
    pub userName: Option<String>,
    pub password: Option<String>,
    pub saveLogin: Option<bool>,
    pub userId: Option<i32>,
    pub forwardingRoutes: Option<String>,
    pub remoteAddresses: Option<Vec<String>>,
    pub localAddresses: Option<Vec<LinkAddress>>,
    pub pkcs12Password: Option<String>,
    pub pkcs12FileData: Option<ani_rs::typed_array::ArrayBuffer>,
    pub addresses: Vec<LinkAddress>,
    pub routes: Option<Vec<RouteInfo>>,
    pub dnsAddresses: Option<Vec<String>>,
    pub searchDomains: Option<Vec<String>>,
    pub mtu: Option<i32>,
    pub isIPv4Accepted: Option<bool>,
    pub isIPv6Accepted: Option<bool>,
    pub isLegacy: Option<bool>,
    pub isBlocking: Option<bool>,
    pub trustedApplications: Option<Vec<String>>,
    pub blockedApplications: Option<Vec<String>>,
    pub ipsecPreSharedKey: Option<String>,
    pub ipsecIdentifier: Option<String>,
    pub swanctlConfig: Option<String>,
    pub strongSwanConfig: Option<String>,
    pub ipsecCaCertConfig: Option<String>,
    pub ipsecPrivateUserCertConfig: Option<String>,
    pub ipsecPublicUserCertConfig: Option<String>,
    pub ipsecPrivateServerCertConfig: Option<String>,
    pub ipsecPublicServerCertConfig: Option<String>,
    pub ipsecCaCertFilePath: Option<String>,
    pub ipsecPrivateUserCertFilePath: Option<String>,
    pub ipsecPublicUserCertFilePath: Option<String>,
    pub ipsecPrivateServerCertFilePath: Option<String>,
    pub ipsecPublicServerCertFilePath: Option<String>,
}

#[ani_rs::ani(path = "@ohos.net.vpn.vpn.OpenVpnConfigInner")]
#[derive(Clone)]
pub struct OpenVpnConfig {
    pub vpnId: Option<String>,
    pub vpnName: Option<String>,
    pub vpnType: Option<SysVpnType>,
    pub userName: Option<String>,
    pub password: Option<String>,
    pub saveLogin: Option<bool>,
    pub userId: Option<i32>,
    pub forwardingRoutes: Option<String>,
    pub remoteAddresses: Option<Vec<String>>,
    pub localAddresses: Option<Vec<LinkAddress>>,
    pub pkcs12Password: Option<String>,
    pub pkcs12FileData: Option<ani_rs::typed_array::ArrayBuffer>,
    pub addresses: Vec<LinkAddress>,
    pub routes: Option<Vec<RouteInfo>>,
    pub dnsAddresses: Option<Vec<String>>,
    pub searchDomains: Option<Vec<String>>,
    pub mtu: Option<i32>,
    pub isIPv4Accepted: Option<bool>,
    pub isIPv6Accepted: Option<bool>,
    pub isLegacy: Option<bool>,
    pub isBlocking: Option<bool>,
    pub trustedApplications: Option<Vec<String>>,
    pub blockedApplications: Option<Vec<String>>,
    pub ovpnPort: Option<String>,
    pub ovpnProtocol: Option<i32>,
    pub ovpnConfig: Option<String>,
    pub ovpnAuthType: Option<i32>,
    pub askpass: Option<String>,
    pub ovpnConfigFilePath: Option<String>,
    pub ovpnCaCertFilePath: Option<String>,
    pub ovpnUserCertFilePath: Option<String>,
    pub ovpnPrivateKeyFilePath: Option<String>,
}

#[ani_rs::ani(path = "@ohos.net.vpn.vpn.L2tpVpnConfigInner")]
#[derive(Clone)]
pub struct L2tpVpnConfig {
    pub vpnId: Option<String>,
    pub vpnName: Option<String>,
    pub vpnType: Option<SysVpnType>,
    pub userName: Option<String>,
    pub password: Option<String>,
    pub saveLogin: Option<bool>,
    pub userId: Option<i32>,
    pub forwardingRoutes: Option<String>,
    pub remoteAddresses: Option<Vec<String>>,
    pub localAddresses: Option<Vec<LinkAddress>>,
    pub pkcs12Password: Option<String>,
    pub pkcs12FileData: Option<ani_rs::typed_array::ArrayBuffer>,
    pub addresses: Vec<LinkAddress>,
    pub routes: Option<Vec<RouteInfo>>,
    pub dnsAddresses: Option<Vec<String>>,
    pub searchDomains: Option<Vec<String>>,
    pub mtu: Option<i32>,
    pub isIPv4Accepted: Option<bool>,
    pub isIPv6Accepted: Option<bool>,
    pub isLegacy: Option<bool>,
    pub isBlocking: Option<bool>,
    pub trustedApplications: Option<Vec<String>>,
    pub blockedApplications: Option<Vec<String>>,
    pub ipsecConfig: Option<String>,
    pub ipsecSecrets: Option<String>,
    pub optionsL2tpdClient: Option<String>,
    pub xl2tpdConfig: Option<String>,
    pub l2tpSharedKey: Option<String>,
}

#[ani_rs::ani(path = "@ohos.net.vpn.vpn.SysVpnConfigInner")]
#[derive(Clone)]
pub struct SysVpnConfig {
    pub addresses: Vec<LinkAddress>,
    pub routes: Option<Vec<RouteInfo>>,
    pub dnsAddresses: Option<Vec<String>>,
    pub searchDomains: Option<Vec<String>>,
    pub mtu: Option<i32>,
    pub isIPv4Accepted: Option<bool>,
    pub isIPv6Accepted: Option<bool>,
    pub isLegacy: Option<bool>,
    pub isBlocking: Option<bool>,
    pub trustedApplications: Option<Vec<String>>,
    pub blockedApplications: Option<Vec<String>>,
    pub vpnId: Option<String>,
    pub vpnName: Option<String>,
    pub vpnType: Option<SysVpnType>,
    pub userName: Option<String>,
    pub password: Option<String>,
    pub saveLogin: Option<bool>,
    pub userId: Option<i32>,
    pub forwardingRoutes: Option<String>,
    pub remoteAddresses: Option<Vec<String>>,
    pub localAddresses: Option<Vec<LinkAddress>>,
    pub pkcs12Password: Option<String>,
    pub pkcs12FileData: Option<ani_rs::typed_array::ArrayBuffer>,
}

#[ani_rs::ani(path = "@ohos.net.vpn.vpn.SysVpnType")]
#[derive(Clone)]
pub enum SysVpnType {
    IKEV2_IPSEC_MSCHAPV2 = 1,
    IKEV2_IPSEC_PSK = 2,
    IKEV2_IPSEC_RSA = 3,
    L2TP_IPSEC_PSK = 4,
    L2TP_IPSEC_RSA = 5,
    IPSEC_XAUTH_PSK = 6,
    IPSEC_XAUTH_RSA = 7,
    IPSEC_HYBRID_RSA = 8,
    OPENVPN = 9,
}
