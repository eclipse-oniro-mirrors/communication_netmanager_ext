/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "vpn_ani.h"

#include <cstdint>

#include "cxx.h"
#include "net_manager_ext_constants.h"
#include "net_manager_constants.h"
#include "errorcode_convertor.h"

namespace OHOS {
namespace NetManagerAni {

// Maximum password length for VPN configuration
constexpr size_t MAX_PASSWORD_LENGTH = 256;

// Forward declarations for ConvertToSysVpnConfig helper functions
static void ConvertSysVpnBasicConfig(const SysVpnConfigData &data,
    const sptr<NetManagerStandard::SysVpnConfig> &config);
static void ConvertSysVpnAppConfig(const SysVpnConfigData &data,
    const sptr<NetManagerStandard::SysVpnConfig> &config);
static bool ConvertSysVpnAuthConfig(const SysVpnConfigData &data,
    const sptr<NetManagerStandard::SysVpnConfig> &config);
static void ConvertSysVpnRouteConfig(const SysVpnConfigData &data,
    const sptr<NetManagerStandard::SysVpnConfig> &config);
static bool ConvertSysVpnPkcs12Config(const SysVpnConfigData &data,
    const sptr<NetManagerStandard::SysVpnConfig> &config);
static void FillIpsecVpnConfigFields(const SysVpnConfigData &data,
    NetManagerStandard::IpsecVpnConfig *config);
static void FillOpenvpnConfigFields(const SysVpnConfigData &data,
    NetManagerStandard::OpenvpnConfig *config);
static void FillL2tpVpnConfigFields(const SysVpnConfigData &data,
    NetManagerStandard::L2tpVpnConfig *config);

// Subtype discriminants matching the public SysVpnType (values 1-9).
// C++ VpnType enum also has internal values (L2TP=10, INTERNAL_CHANNEL=11, VIRTUAL_VPN=12)
// which are not exposed through the ANI bridge.
enum VpnTypeValue : int32_t {
    VPN_TYPE_IKEV2_IPSEC_MSCHAPV2 = 1,
    VPN_TYPE_IKEV2_IPSEC_PSK = 2,
    VPN_TYPE_IKEV2_IPSEC_RSA = 3,
    VPN_TYPE_L2TP_IPSEC_PSK = 4,
    VPN_TYPE_L2TP_IPSEC_RSA = 5,
    VPN_TYPE_IPSEC_XAUTH_PSK = 6,
    VPN_TYPE_IPSEC_XAUTH_RSA = 7,
    VPN_TYPE_IPSEC_HYBRID_RSA = 8,
    VPN_TYPE_OPENVPN = 9,
};

static bool IsIpsecType(int32_t vpnType)
{
    return vpnType == VPN_TYPE_IKEV2_IPSEC_MSCHAPV2 ||
           vpnType == VPN_TYPE_IKEV2_IPSEC_PSK ||
           vpnType == VPN_TYPE_IKEV2_IPSEC_RSA ||
           vpnType == VPN_TYPE_IPSEC_XAUTH_PSK ||
           vpnType == VPN_TYPE_IPSEC_XAUTH_RSA ||
           vpnType == VPN_TYPE_IPSEC_HYBRID_RSA;
}

static bool IsOpenvpnType(int32_t vpnType)
{
    return vpnType == VPN_TYPE_OPENVPN;
}

static bool IsL2tpType(int32_t vpnType)
{
    return vpnType == VPN_TYPE_L2TP_IPSEC_PSK ||
           vpnType == VPN_TYPE_L2TP_IPSEC_RSA;
}

sptr<VpnEventCallbackObserverAni> g_vpnEventCallbackObserverAni =
    sptr<VpnEventCallbackObserverAni>(
        new (std::nothrow) VpnEventCallbackObserverAni());

bool g_isVpnObserverRegistered = false;
std::mutex g_vpnObserverMutex;

static NetAddressData ConvertToNetAddressData(const NetManagerStandard::INetAddr &addr)
{
    NetAddressData data;
    data.address = rust::String(addr.address_);
    data.family = addr.type_;
    data.port = addr.port_;
    return data;
}

static LinkAddressData ConvertToLinkAddressData(const NetManagerStandard::INetAddr &addr)
{
    LinkAddressData data;
    data.address = ConvertToNetAddressData(addr);
    data.prefix_length = addr.prefixlen_;
    return data;
}

static RouteInfoData ConvertToRouteInfoData(const NetManagerStandard::Route &route)
{
    RouteInfoData data;
    data.iface = rust::String(route.iface_);
    data.destination = ConvertToLinkAddressData(route.destination_);
    data.gateway = ConvertToNetAddressData(route.gateway_);
    data.has_gateway = route.hasGateway_;
    data.is_default_route = route.isDefaultRoute_;
    return data;
}

static NetManagerStandard::INetAddr ConvertFromNetAddressData(const NetAddressData &data)
{
    NetManagerStandard::INetAddr addr;
    addr.address_ = std::string(data.address);
    addr.type_ = data.family;
    addr.port_ = data.port;
    return addr;
}

static NetManagerStandard::INetAddr ConvertFromLinkAddressData(const LinkAddressData &data)
{
    NetManagerStandard::INetAddr addr = ConvertFromNetAddressData(data.address);
    addr.prefixlen_ = data.prefix_length;
    return addr;
}

static NetManagerStandard::Route ConvertFromRouteInfoData(const RouteInfoData &data)
{
    NetManagerStandard::Route route;
    route.iface_ = std::string(data.iface);
    route.destination_ = ConvertFromLinkAddressData(data.destination);
    route.gateway_ = ConvertFromNetAddressData(data.gateway);
    route.hasGateway_ = data.has_gateway;
    route.isDefaultRoute_ = data.is_default_route;
    return route;
}

sptr<NetManagerStandard::VpnConfig> ConvertToVpnConfig(const VpnConfigData &data)
{
    auto config = sptr<NetManagerStandard::VpnConfig>(new (std::nothrow) NetManagerStandard::VpnConfig());
    if (config == nullptr) {
        return nullptr;
    }
    config->vpnId_ = std::string(data.vpn_id);
    for (auto &addr : data.addresses) {
        config->addresses_.push_back(ConvertFromLinkAddressData(addr));
    }
    for (auto &route : data.routes) {
        config->routes_.push_back(ConvertFromRouteInfoData(route));
    }
    for (auto &dns : data.dns_addresses) {
        config->dnsAddresses_.push_back(std::string(dns));
    }
    for (auto &domain : data.search_domains) {
        config->searchDomains_.push_back(std::string(domain));
    }
    config->mtu_ = data.mtu;
    config->isAcceptIPv4_ = data.is_ipv4_accepted;
    config->isAcceptIPv6_ = data.is_ipv6_accepted;
    config->isLegacy_ = data.is_legacy;
    config->isBlocking_ = data.is_blocking;
    for (auto &app : data.trusted_applications) {
        config->acceptedApplications_.push_back(std::string(app));
    }
    for (auto &app : data.blocked_applications) {
        config->refusedApplications_.push_back(std::string(app));
    }
    return config;
}

sptr<NetManagerStandard::SysVpnConfig> ConvertToSysVpnConfig(const SysVpnConfigData &data)
{
    int32_t vpnType = data.vpn_type;
    sptr<NetManagerStandard::SysVpnConfig> config;

    if (IsIpsecType(vpnType)) {
        auto ipsecConfig = sptr<NetManagerStandard::IpsecVpnConfig>(
            new (std::nothrow) NetManagerStandard::IpsecVpnConfig());
        if (ipsecConfig == nullptr) {
            return nullptr;
        }
        config = ipsecConfig;
        FillIpsecVpnConfigFields(data, ipsecConfig.GetRefPtr());
    } else if (IsOpenvpnType(vpnType)) {
        auto openvpnConfig = sptr<NetManagerStandard::OpenvpnConfig>(
            new (std::nothrow) NetManagerStandard::OpenvpnConfig());
        if (openvpnConfig == nullptr) {
            return nullptr;
        }
        config = openvpnConfig;
        FillOpenvpnConfigFields(data, openvpnConfig.GetRefPtr());
    } else if (IsL2tpType(vpnType)) {
        auto l2tpConfig = sptr<NetManagerStandard::L2tpVpnConfig>(
            new (std::nothrow) NetManagerStandard::L2tpVpnConfig());
        if (l2tpConfig == nullptr) {
            return nullptr;
        }
        config = l2tpConfig;
        FillL2tpVpnConfigFields(data, l2tpConfig.GetRefPtr());
    } else {
        config = sptr<NetManagerStandard::SysVpnConfig>(new (std::nothrow) NetManagerStandard::SysVpnConfig());
    }

    if (config == nullptr) {
        return nullptr;
    }
    ConvertSysVpnBasicConfig(data, config);
    ConvertSysVpnAppConfig(data, config);
    if (!ConvertSysVpnAuthConfig(data, config)) {
        return nullptr;
    }
    ConvertSysVpnRouteConfig(data, config);
    if (!ConvertSysVpnPkcs12Config(data, config)) {
        return nullptr;
    }
    return config;
}

static void ConvertSysVpnBasicConfig(const SysVpnConfigData &data,
    const sptr<NetManagerStandard::SysVpnConfig> &config)
{
    for (auto &addr : data.addresses) {
        config->addresses_.push_back(ConvertFromLinkAddressData(addr));
    }
    for (auto &route : data.routes) {
        config->routes_.push_back(ConvertFromRouteInfoData(route));
    }
    for (auto &dns : data.dns_addresses) {
        config->dnsAddresses_.push_back(std::string(dns));
    }
    for (auto &domain : data.search_domains) {
        config->searchDomains_.push_back(std::string(domain));
    }
    config->mtu_ = data.mtu;
    config->isAcceptIPv4_ = data.is_ipv4_accepted;
    config->isAcceptIPv6_ = data.is_ipv6_accepted;
    config->isLegacy_ = data.is_legacy;
    config->isBlocking_ = data.is_blocking;
    config->vpnId_ = std::string(data.vpn_id);
    config->vpnName_ = std::string(data.vpn_name);
    config->vpnType_ = data.vpn_type;
    config->userName_ = std::string(data.user_name);
}

static void ConvertSysVpnAppConfig(const SysVpnConfigData &data,
    const sptr<NetManagerStandard::SysVpnConfig> &config)
{
    for (auto &app : data.trusted_applications) {
        config->acceptedApplications_.push_back(std::string(app));
    }
    for (auto &app : data.blocked_applications) {
        config->refusedApplications_.push_back(std::string(app));
    }
}

static bool ConvertSysVpnAuthConfig(const SysVpnConfigData &data,
    const sptr<NetManagerStandard::SysVpnConfig> &config)
{
    if (data.password.length() > MAX_PASSWORD_LENGTH) {
        return false;
    }
    config->password_ = std::string(data.password);
    config->saveLogin_ = data.save_login;
    config->userId_ = data.user_id;
    return true;
}

static void ConvertSysVpnRouteConfig(const SysVpnConfigData &data,
    const sptr<NetManagerStandard::SysVpnConfig> &config)
{
    config->forwardingRoutes_ = std::string(data.forwarding_routes);
    for (auto &addr : data.remote_addresses) {
        config->remoteAddresses_.push_back(std::string(addr));
    }
    for (auto &addr : data.local_addresses) {
        config->localAddresses_.push_back(ConvertFromLinkAddressData(addr));
    }
}

static bool ConvertSysVpnPkcs12Config(const SysVpnConfigData &data,
    const sptr<NetManagerStandard::SysVpnConfig> &config)
{
    if (data.pkcs12_password.length() > MAX_PASSWORD_LENGTH) {
        return false;
    }
    config->pkcs12Password_ = std::string(data.pkcs12_password);
    for (auto &byte : data.pkcs12_file_data) {
        config->pkcs12FileData_.push_back(byte);
    }
    return true;
}

static void FillIpsecVpnConfigFields(const SysVpnConfigData &data,
    NetManagerStandard::IpsecVpnConfig *config)
{
    if (config == nullptr) {
        return;
    }
    config->ipsecPreSharedKey_ = std::string(data.ipsec_pre_shared_key);
    config->ipsecIdentifier_ = std::string(data.ipsec_identifier);
    config->swanctlConf_ = std::string(data.swanctl_config);
    config->strongswanConf_ = std::string(data.strong_swan_config);
    config->ipsecCaCertConf_ = std::string(data.ipsec_ca_cert_config);
    config->ipsecPrivateUserCertConf_ = std::string(data.ipsec_private_user_cert_config);
    config->ipsecPublicUserCertConf_ = std::string(data.ipsec_public_user_cert_config);
    config->ipsecPrivateServerCertConf_ = std::string(data.ipsec_private_server_cert_config);
    config->ipsecPublicServerCertConf_ = std::string(data.ipsec_public_server_cert_config);
    config->ipsecCaCertFilePath_ = std::string(data.ipsec_ca_cert_file_path);
    config->ipsecPrivateUserCertFilePath_ = std::string(data.ipsec_private_user_cert_file_path);
    config->ipsecPublicUserCertFilePath_ = std::string(data.ipsec_public_user_cert_file_path);
    config->ipsecPrivateServerCertFilePath_ = std::string(data.ipsec_private_server_cert_file_path);
    config->ipsecPublicServerCertFilePath_ = std::string(data.ipsec_public_server_cert_file_path);
}

static void FillOpenvpnConfigFields(const SysVpnConfigData &data,
    NetManagerStandard::OpenvpnConfig *config)
{
    if (config == nullptr) {
        return;
    }
    config->ovpnPort_ = std::string(data.ovpn_port);
    config->ovpnProtocol_ = data.ovpn_protocol;
    config->ovpnConfig_ = std::string(data.ovpn_config);
    config->ovpnAuthType_ = data.ovpn_auth_type;
    config->askpass_ = std::string(data.askpass);
    config->ovpnConfigFilePath_ = std::string(data.ovpn_config_file_path);
    config->ovpnCaCertFilePath_ = std::string(data.ovpn_ca_cert_file_path);
    config->ovpnUserCertFilePath_ = std::string(data.ovpn_user_cert_file_path);
    config->ovpnPrivateKeyFilePath_ = std::string(data.ovpn_private_key_file_path);
}

static void FillL2tpVpnConfigFields(const SysVpnConfigData &data,
    NetManagerStandard::L2tpVpnConfig *config)
{
    if (config == nullptr) {
        return;
    }
    // Ipsec-common fields shared with IpsecVpnConfig
    config->ipsecPreSharedKey_ = std::string(data.ipsec_pre_shared_key);
    config->ipsecIdentifier_ = std::string(data.ipsec_identifier);
    config->strongswanConf_ = std::string(data.strong_swan_config);
    config->ipsecCaCertConf_ = std::string(data.ipsec_ca_cert_config);
    config->ipsecPrivateUserCertConf_ = std::string(data.ipsec_private_user_cert_config);
    config->ipsecPublicUserCertConf_ = std::string(data.ipsec_public_user_cert_config);
    config->ipsecPrivateServerCertConf_ = std::string(data.ipsec_private_server_cert_config);
    config->ipsecPublicServerCertConf_ = std::string(data.ipsec_public_server_cert_config);
    config->ipsecCaCertFilePath_ = std::string(data.ipsec_ca_cert_file_path);
    config->ipsecPrivateUserCertFilePath_ = std::string(data.ipsec_private_user_cert_file_path);
    config->ipsecPublicUserCertFilePath_ = std::string(data.ipsec_public_user_cert_file_path);
    config->ipsecPrivateServerCertFilePath_ = std::string(data.ipsec_private_server_cert_file_path);
    config->ipsecPublicServerCertFilePath_ = std::string(data.ipsec_public_server_cert_file_path);
    // L2TP-unique fields
    config->ipsecConf_ = std::string(data.ipsec_config);
    config->ipsecSecrets_ = std::string(data.ipsec_secrets);
    config->optionsL2tpdClient_ = std::string(data.options_l2tpd_client);
    config->xl2tpdConf_ = std::string(data.xl2tpd_config);
    config->l2tpSharedKey_ = std::string(data.l2tp_shared_key);
}

static void WriteSysVpnBasicConfig(SysVpnConfigData &data,
    const NetManagerStandard::SysVpnConfig &config);
static void WriteSysVpnAppConfig(SysVpnConfigData &data,
    const NetManagerStandard::SysVpnConfig &config);
static void WriteSysVpnAuthConfig(SysVpnConfigData &data,
    const NetManagerStandard::SysVpnConfig &config);
static void WriteSysVpnRouteConfig(SysVpnConfigData &data,
    const NetManagerStandard::SysVpnConfig &config);


static void ReadIpsecCommonFields(SysVpnConfigData &data,
    const NetManagerStandard::SysVpnConfig &config, int32_t vpnType)
{
    if (IsIpsecType(vpnType)) {
        const auto &ipsecCfg = static_cast<const NetManagerStandard::IpsecVpnConfig &>(config);
        data.ipsec_pre_shared_key = rust::String(ipsecCfg.ipsecPreSharedKey_);
        data.ipsec_identifier = rust::String(ipsecCfg.ipsecIdentifier_);
        data.swanctl_config = rust::String(ipsecCfg.swanctlConf_);
        data.strong_swan_config = rust::String(ipsecCfg.strongswanConf_);
        data.ipsec_ca_cert_config = rust::String(ipsecCfg.ipsecCaCertConf_);
        data.ipsec_private_user_cert_config = rust::String(ipsecCfg.ipsecPrivateUserCertConf_);
        data.ipsec_public_user_cert_config = rust::String(ipsecCfg.ipsecPublicUserCertConf_);
        data.ipsec_private_server_cert_config = rust::String(ipsecCfg.ipsecPrivateServerCertConf_);
        data.ipsec_public_server_cert_config = rust::String(ipsecCfg.ipsecPublicServerCertConf_);
        data.ipsec_ca_cert_file_path = rust::String(ipsecCfg.ipsecCaCertFilePath_);
        data.ipsec_private_user_cert_file_path = rust::String(ipsecCfg.ipsecPrivateUserCertFilePath_);
        data.ipsec_public_user_cert_file_path = rust::String(ipsecCfg.ipsecPublicUserCertFilePath_);
        data.ipsec_private_server_cert_file_path = rust::String(ipsecCfg.ipsecPrivateServerCertFilePath_);
        data.ipsec_public_server_cert_file_path = rust::String(ipsecCfg.ipsecPublicServerCertFilePath_);
        return;
    }
    // L2tpVpnConfig (vpnType 4,5 — L2TP+IPSEC combos share ipsec fields)
    if (IsL2tpType(vpnType)) {
        const auto &l2tpCfg = static_cast<const NetManagerStandard::L2tpVpnConfig &>(config);
        data.ipsec_pre_shared_key = rust::String(l2tpCfg.ipsecPreSharedKey_);
        data.ipsec_identifier = rust::String(l2tpCfg.ipsecIdentifier_);
        data.swanctl_config = rust::String("");          // L2tpVpnConfig has no swanctlConf_
        data.strong_swan_config = rust::String(l2tpCfg.strongswanConf_);
        data.ipsec_ca_cert_config = rust::String(l2tpCfg.ipsecCaCertConf_);
        data.ipsec_private_user_cert_config = rust::String(l2tpCfg.ipsecPrivateUserCertConf_);
        data.ipsec_public_user_cert_config = rust::String(l2tpCfg.ipsecPublicUserCertConf_);
        data.ipsec_private_server_cert_config = rust::String(l2tpCfg.ipsecPrivateServerCertConf_);
        data.ipsec_public_server_cert_config = rust::String(l2tpCfg.ipsecPublicServerCertConf_);
        data.ipsec_ca_cert_file_path = rust::String(l2tpCfg.ipsecCaCertFilePath_);
        data.ipsec_private_user_cert_file_path = rust::String(l2tpCfg.ipsecPrivateUserCertFilePath_);
        data.ipsec_public_user_cert_file_path = rust::String(l2tpCfg.ipsecPublicUserCertFilePath_);
        data.ipsec_private_server_cert_file_path = rust::String(l2tpCfg.ipsecPrivateServerCertFilePath_);
        data.ipsec_public_server_cert_file_path = rust::String(l2tpCfg.ipsecPublicServerCertFilePath_);
    }
}

// Read L2TP-unique fields from C++ config back into FFI data.
// Caller guarantees config.vpnType_ indicates a L2tpVpnConfig subtype.
static void ReadL2tpUniqueFields(SysVpnConfigData &data,
    const NetManagerStandard::SysVpnConfig &config)
{
    const auto &l2tpCfg = static_cast<const NetManagerStandard::L2tpVpnConfig &>(config);
    data.ipsec_config = rust::String(l2tpCfg.ipsecConf_);
    data.ipsec_secrets = rust::String(l2tpCfg.ipsecSecrets_);
    data.options_l2tpd_client = rust::String(l2tpCfg.optionsL2tpdClient_);
    data.xl2tpd_config = rust::String(l2tpCfg.xl2tpdConf_);
    data.l2tp_shared_key = rust::String(l2tpCfg.l2tpSharedKey_);
}

// Read OpenVPN fields from C++ config back into FFI data.
// Caller guarantees config.vpnType_ indicates an OpenvpnConfig subtype.
static void ReadOpenvpnFields(SysVpnConfigData &data,
    const NetManagerStandard::SysVpnConfig &config)
{
    const auto &ovpnCfg = static_cast<const NetManagerStandard::OpenvpnConfig &>(config);
    data.ovpn_port = rust::String(ovpnCfg.ovpnPort_);
    data.ovpn_protocol = ovpnCfg.ovpnProtocol_;
    data.ovpn_config = rust::String(ovpnCfg.ovpnConfig_);
    data.ovpn_auth_type = ovpnCfg.ovpnAuthType_;
    data.askpass = rust::String(ovpnCfg.askpass_);
    data.ovpn_config_file_path = rust::String(ovpnCfg.ovpnConfigFilePath_);
    data.ovpn_ca_cert_file_path = rust::String(ovpnCfg.ovpnCaCertFilePath_);
    data.ovpn_user_cert_file_path = rust::String(ovpnCfg.ovpnUserCertFilePath_);
    data.ovpn_private_key_file_path = rust::String(ovpnCfg.ovpnPrivateKeyFilePath_);
}

SysVpnConfigData ConvertFromSysVpnConfig(const NetManagerStandard::SysVpnConfig &config)
{
    SysVpnConfigData data;
    WriteSysVpnBasicConfig(data, config);
    WriteSysVpnAppConfig(data, config);
    WriteSysVpnAuthConfig(data, config);
    WriteSysVpnRouteConfig(data, config);

    // Read back subtype-specific fields based on vpnType.
    int32_t vpnType = config.vpnType_;
    if (IsIpsecType(vpnType)) {
        ReadIpsecCommonFields(data, config, vpnType);
    } else if (IsOpenvpnType(vpnType)) {
        ReadOpenvpnFields(data, config);
    } else if (IsL2tpType(vpnType)) {
        ReadIpsecCommonFields(data, config, vpnType);
        ReadL2tpUniqueFields(data, config);
    }
    return data;
}

static void WriteSysVpnBasicConfig(SysVpnConfigData &data,
    const NetManagerStandard::SysVpnConfig &config)
{
    for (auto &addr : config.addresses_) {
        data.addresses.push_back(ConvertToLinkAddressData(addr));
    }
    for (auto &route : config.routes_) {
        data.routes.push_back(ConvertToRouteInfoData(route));
    }
    for (auto &dns : config.dnsAddresses_) {
        data.dns_addresses.push_back(rust::String(dns));
    }
    for (auto &domain : config.searchDomains_) {
        data.search_domains.push_back(rust::String(domain));
    }
    data.mtu = config.mtu_;
    data.is_ipv4_accepted = config.isAcceptIPv4_;
    data.is_ipv6_accepted = config.isAcceptIPv6_;
    data.is_legacy = config.isLegacy_;
    data.is_blocking = config.isBlocking_;
    data.vpn_id = rust::String(config.vpnId_);
    data.vpn_name = rust::String(config.vpnName_);
    data.vpn_type = config.vpnType_;
    data.user_name = rust::String(config.userName_);
}

static void WriteSysVpnAppConfig(SysVpnConfigData &data,
    const NetManagerStandard::SysVpnConfig &config)
{
    for (auto &app : config.acceptedApplications_) {
        data.trusted_applications.push_back(rust::String(app));
    }
    for (auto &app : config.refusedApplications_) {
        data.blocked_applications.push_back(rust::String(app));
    }
}

static void WriteSysVpnAuthConfig(SysVpnConfigData &data,
    const NetManagerStandard::SysVpnConfig &config)
{
    // Intentionally clear password fields for security:
    // Do NOT expose passwords through Get operations.
    data.password = rust::String("");
    data.save_login = config.saveLogin_;
    data.user_id = config.userId_;
}

static void WriteSysVpnRouteConfig(SysVpnConfigData &data,
    const NetManagerStandard::SysVpnConfig &config)
{
    data.forwarding_routes = rust::String(config.forwardingRoutes_);
    for (auto &addr : config.remoteAddresses_) {
        data.remote_addresses.push_back(rust::String(addr));
    }
    for (auto &addr : config.localAddresses_) {
        data.local_addresses.push_back(ConvertToLinkAddressData(addr));
    }
}

bool CreateVpnConnection(int32_t &ret)
{
    ret = NetManagerStandard::NetworkVpnClient::GetInstance().CreateVpnConnection(false);
    return ret == NetManagerStandard::NETMANAGER_EXT_SUCCESS;
}

int32_t SetUp(const VpnConfigData &config, int32_t &fd)
{
    auto vpnConfig = ConvertToVpnConfig(config);
    if (vpnConfig == nullptr) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
    }
    return NetManagerStandard::NetworkVpnClient::GetInstance().SetUpVpn(vpnConfig, fd);
}

int32_t Protect(int32_t socketFd)
{
    return NetManagerStandard::NetworkVpnClient::GetInstance().Protect(socketFd);
}

int32_t Destroy()
{
    return NetManagerStandard::NetworkVpnClient::GetInstance().DestroyVpn(false);
}

int32_t AddSysVpnConfig(const SysVpnConfigData &config)
{
#ifdef SUPPORT_SYSVPN
    auto sysVpnConfig = ConvertToSysVpnConfig(config);
    if (sysVpnConfig == nullptr) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
    }
    sptr<NetManagerStandard::SysVpnConfig> configPtr = sysVpnConfig;
    return NetManagerStandard::NetworkVpnClient::GetInstance().AddSysVpnConfig(configPtr);
#else
    return NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
#endif
}

int32_t DeleteSysVpnConfig(const rust::String &vpnId)
{
#ifdef SUPPORT_SYSVPN
    return NetManagerStandard::NetworkVpnClient::GetInstance().DeleteSysVpnConfig(std::string(vpnId));
#else
    return NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
#endif
}

rust::Vec<SysVpnConfigData> GetSysVpnConfigList(int32_t &ret)
{
#ifdef SUPPORT_SYSVPN
    rust::Vec<SysVpnConfigData> list;
    std::vector<sptr<NetManagerStandard::SysVpnConfig>> vpnList;
    ret = NetManagerStandard::NetworkVpnClient::GetInstance().GetSysVpnConfigList(vpnList);
    for (auto &config : vpnList) {
        if (config != nullptr) {
            list.push_back(ConvertFromSysVpnConfig(*config));
        }
    }
    return list;
#else
    ret = NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
    return rust::Vec<SysVpnConfigData>();
#endif
}

SysVpnConfigData GetSysVpnConfig(const rust::String &vpnId, int32_t &ret)
{
#ifdef SUPPORT_SYSVPN
    sptr<NetManagerStandard::SysVpnConfig> config;
    ret = NetManagerStandard::NetworkVpnClient::GetInstance().GetSysVpnConfig(config, std::string(vpnId));
    if (config != nullptr && ret == NetManagerStandard::NETMANAGER_EXT_SUCCESS) {
        return ConvertFromSysVpnConfig(*config);
    }
    return SysVpnConfigData();
#else
    ret = NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
    return SysVpnConfigData();
#endif
}

SysVpnConfigData GetConnectedSysVpnConfig(int32_t &ret)
{
#ifdef SUPPORT_SYSVPN
    sptr<NetManagerStandard::SysVpnConfig> config;
    ret = NetManagerStandard::NetworkVpnClient::GetInstance().GetConnectedSysVpnConfig(config);
    if (config != nullptr && ret == NetManagerStandard::NETMANAGER_EXT_SUCCESS) {
        return ConvertFromSysVpnConfig(*config);
    }
    return SysVpnConfigData();
#else
    ret = NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
    return SysVpnConfigData();
#endif
}

rust::Vec<rust::String> GetConnectedVpnAppInfo(int32_t &ret)
{
#ifdef SUPPORT_SYSVPN
    rust::Vec<rust::String> appInfoList;
    std::vector<std::string> bundleNameList;
    ret = NetManagerStandard::NetworkVpnClient::GetInstance().GetConnectedVpnAppInfo(bundleNameList);
    for (auto &name : bundleNameList) {
        appInfoList.push_back(rust::String(name));
    }
    return appInfoList;
#else
    ret = NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
    return rust::Vec<rust::String>();
#endif
}

ErrCode VpnEventCallbackObserverAni::OnVpnStateChanged(
    bool isConnected, const sptr<NetManagerStandard::VpnState> &vpnState)
{
    VpnStateInfo info;
    info.is_connected = isConnected;
    if (vpnState != nullptr) {
        info.bundle_name = rust::String(vpnState->vpnPacketName_);
        info.vpn_id = rust::String(vpnState->vpnId_);
    }
    execute_vpn_state_changed(info);
    return 0;
}

ErrCode VpnEventCallbackObserverAni::OnMultiVpnStateChanged(
    bool isConnected, const std::string &bundleName, const std::string &vpnId)
{
    VpnStateInfo info;
    info.is_connected = isConnected;
    info.bundle_name = rust::String(bundleName);
    info.vpn_id = rust::String(vpnId);
    execute_multi_vpn_state_changed(info);
    return 0;
}

rust::String GetErrorCodeAndMessage(int32_t &errorCode)
{
    NetManagerStandard::NetBaseErrorCodeConvertor convertor;
    return rust::String(convertor.ConvertErrorCode(errorCode));
}

int32_t VpnObserverRegister()
{
    std::lock_guard<std::mutex> lock(g_vpnObserverMutex);
    if (g_isVpnObserverRegistered) {
        return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
    }

    if (g_vpnEventCallbackObserverAni == nullptr) {
        g_vpnEventCallbackObserverAni = sptr<VpnEventCallbackObserverAni>(
            new (std::nothrow) VpnEventCallbackObserverAni());
    }

    if (g_vpnEventCallbackObserverAni == nullptr) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
    }

    int32_t result = NetManagerStandard::NetworkVpnClient::GetInstance().RegisterVpnEvent(
        g_vpnEventCallbackObserverAni);
    if (result == NetManagerStandard::NETMANAGER_EXT_SUCCESS) {
        g_isVpnObserverRegistered = true;
    } else {
        g_vpnEventCallbackObserverAni = nullptr;
    }
    return result;
}

int32_t VpnObserverUnRegister()
{
    std::lock_guard<std::mutex> lock(g_vpnObserverMutex);
    if (g_vpnEventCallbackObserverAni == nullptr) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }

    int32_t result = NetManagerStandard::NetworkVpnClient::GetInstance().UnregisterVpnEvent(
        g_vpnEventCallbackObserverAni);
    if (result == NetManagerStandard::NETMANAGER_EXT_SUCCESS) {
        g_isVpnObserverRegistered = false;
    }
    return result;
}

} // namespace NetManagerAni
} // namespace OHOS
