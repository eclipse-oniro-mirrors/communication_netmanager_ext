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
#include "netmanager_ext_log.h"
#include "networkvpn_client.h"

namespace OHOS {
namespace NetManagerAni {

constexpr size_t MAX_PASSWORD_LENGTH = 256;

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

sptr<VpnEventCallbackObserverAni> g_vpnEventCallbackObserverAni = nullptr;
bool g_isVpnObserverRegistered = false;
std::mutex g_vpnObserverMutex;

static NetAddressAni ConvertToNetAddressData(const NetManagerStandard::INetAddr &addr)
{
    NetAddressAni data;
    data.address = rust::String(addr.address_);
    data.family = addr.type_;
    data.port = addr.port_;
    return data;
}

static LinkAddressAni ConvertToLinkAddressData(const NetManagerStandard::INetAddr &addr)
{
    LinkAddressAni data;
    data.address = ConvertToNetAddressData(addr);
    data.prefix_length = addr.prefixlen_;
    return data;
}

static RouteInfoAni ConvertToRouteInfoData(const NetManagerStandard::Route &route)
{
    RouteInfoAni data;
    data.iface = rust::String(route.iface_);
    data.destination = ConvertToLinkAddressData(route.destination_);
    data.gateway = ConvertToNetAddressData(route.gateway_);
    data.has_gateway = route.hasGateway_;
    data.is_default_route = route.isDefaultRoute_;
    return data;
}

static NetManagerStandard::INetAddr ConvertFromNetAddressData(const NetAddressAni &data)
{
    NetManagerStandard::INetAddr addr;
    addr.address_ = std::string(data.address);
    addr.type_ = data.family;
    addr.port_ = data.port;
    return addr;
}

static NetManagerStandard::INetAddr ConvertFromLinkAddressData(const LinkAddressAni &data)
{
    NetManagerStandard::INetAddr addr = ConvertFromNetAddressData(data.address);
    addr.prefixlen_ = data.prefix_length;
    return addr;
}

static NetManagerStandard::Route ConvertFromRouteInfoData(const RouteInfoAni &data)
{
    NetManagerStandard::Route route;
    route.iface_ = std::string(data.iface);
    route.destination_ = ConvertFromLinkAddressData(data.destination);
    route.gateway_ = ConvertFromNetAddressData(data.gateway);
    route.hasGateway_ = data.has_gateway;
    route.isDefaultRoute_ = data.is_default_route;
    return route;
}

static void FillVpnConfigBase(const VpnConfigAni &data,
    const sptr<NetManagerStandard::VpnConfig> &config)
{
    if (config == nullptr) {
        return;
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
}

static void FillSysVpnConfigBaseFull(const SysVpnConfigFullAni &data,
    const sptr<NetManagerStandard::SysVpnConfig> &config)
{
    if (config == nullptr) {
        return;
    }
    // VpnConfig base
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
    // SysVpnConfig base
    config->vpnName_ = std::string(data.vpn_name);
    config->vpnType_ = data.vpn_type;
    config->userName_ = std::string(data.user_name);
    config->password_ = std::string(data.password);
    config->saveLogin_ = data.save_login;
    config->userId_ = data.user_id;
    config->forwardingRoutes_ = std::string(data.forwarding_routes);
    for (auto &addr : data.remote_addresses) {
        config->remoteAddresses_.push_back(std::string(addr));
    }
    for (auto &addr : data.local_addresses) {
        config->localAddresses_.push_back(ConvertFromLinkAddressData(addr));
    }
    config->pkcs12Password_ = std::string(data.pkcs12_password);
    for (auto &byte : data.pkcs12_file_data) {
        config->pkcs12FileData_.push_back(byte);
    }
}

static void FillIpsecVpnConfigFields(const SysVpnConfigFullAni &data,
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

static void FillOpenvpnConfigFields(const SysVpnConfigFullAni &data,
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

static void FillL2tpVpnConfigFields(const SysVpnConfigFullAni &data,
    NetManagerStandard::L2tpVpnConfig *config)
{
    if (config == nullptr) {
        return;
    }

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
    config->ipsecConf_ = std::string(data.ipsec_config);
    config->ipsecSecrets_ = std::string(data.ipsec_secrets);
    config->optionsL2tpdClient_ = std::string(data.options_l2tpd_client);
    config->xl2tpdConf_ = std::string(data.xl2tpd_config);
    config->l2tpSharedKey_ = std::string(data.l2tp_shared_key);
}

sptr<NetManagerStandard::VpnConfig> ConvertToVpnConfig(const VpnConfigAni &data)
{
    auto config = sptr<NetManagerStandard::VpnConfig>(new (std::nothrow) NetManagerStandard::VpnConfig());
    if (config == nullptr) {
        return nullptr;
    }
    FillVpnConfigBase(data, config);
    return config;
}

sptr<NetManagerStandard::VpnConfig> ConvertToVpnConfigFull(const SysVpnConfigFullAni &data)
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

sptr<NetManagerStandard::SysVpnConfig> ConvertToSysVpnConfig(const SysVpnConfigFullAni &data)
{
    int32_t vpnType = data.vpn_type;
    if (IsIpsecType(vpnType)) {
        auto config = sptr<NetManagerStandard::IpsecVpnConfig>(
            new (std::nothrow) NetManagerStandard::IpsecVpnConfig());
        if (config == nullptr) {
            return nullptr;
        }
        FillSysVpnConfigBaseFull(data, config);
        FillIpsecVpnConfigFields(data, config.GetRefPtr());
        return config;
    }
    if (IsOpenvpnType(vpnType)) {
        auto config = sptr<NetManagerStandard::OpenvpnConfig>(
            new (std::nothrow) NetManagerStandard::OpenvpnConfig());
        if (config == nullptr) {
            return nullptr;
        }
        FillSysVpnConfigBaseFull(data, config);
        FillOpenvpnConfigFields(data, config.GetRefPtr());
        return config;
    }
    if (IsL2tpType(vpnType)) {
        auto config = sptr<NetManagerStandard::L2tpVpnConfig>(
            new (std::nothrow) NetManagerStandard::L2tpVpnConfig());
        if (config == nullptr) {
            return nullptr;
        }
        FillSysVpnConfigBaseFull(data, config);
        FillL2tpVpnConfigFields(data, config.GetRefPtr());
        return config;
    }
    auto config = sptr<NetManagerStandard::SysVpnConfig>(
        new (std::nothrow) NetManagerStandard::SysVpnConfig());
    if (config == nullptr) {
        return nullptr;
    }
    FillSysVpnConfigBaseFull(data, config);
    return config;
}


static void WriteVpnConfigBaseFull(SysVpnConfigFullAni &data,
    const NetManagerStandard::SysVpnConfig &config)
{
    data.vpn_id = rust::String(config.vpnId_);
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
    for (auto &app : config.acceptedApplications_) {
        data.trusted_applications.push_back(rust::String(app));
    }
    for (auto &app : config.refusedApplications_) {
        data.blocked_applications.push_back(rust::String(app));
    }
}

static void WriteSysVpnConfigBaseFull(SysVpnConfigFullAni &data,
    const NetManagerStandard::SysVpnConfig &config)
{
    data.vpn_name = rust::String(config.vpnName_);
    data.vpn_type = config.vpnType_;
    data.user_name = rust::String(config.userName_);
    data.password = rust::String("");
    data.save_login = config.saveLogin_;
    data.user_id = config.userId_;
    data.forwarding_routes = rust::String(config.forwardingRoutes_);
    for (auto &addr : config.remoteAddresses_) {
        data.remote_addresses.push_back(rust::String(addr));
    }
    for (auto &addr : config.localAddresses_) {
        data.local_addresses.push_back(ConvertToLinkAddressData(addr));
    }
    data.pkcs12_password = rust::String("");
}

static void WriteIpsecCommonFull(SysVpnConfigFullAni &data,
    const NetManagerStandard::SysVpnConfig &config, int32_t vpnType)
{
    if (IsIpsecType(vpnType)) {
        const auto &ipsec = static_cast<const NetManagerStandard::IpsecVpnConfig &>(config);
        data.ipsec_pre_shared_key = rust::String(ipsec.ipsecPreSharedKey_);
        data.ipsec_identifier = rust::String(ipsec.ipsecIdentifier_);
        data.swanctl_config = rust::String(ipsec.swanctlConf_);
        data.strong_swan_config = rust::String(ipsec.strongswanConf_);
        data.ipsec_ca_cert_config = rust::String(ipsec.ipsecCaCertConf_);
        data.ipsec_private_user_cert_config = rust::String(ipsec.ipsecPrivateUserCertConf_);
        data.ipsec_public_user_cert_config = rust::String(ipsec.ipsecPublicUserCertConf_);
        data.ipsec_private_server_cert_config = rust::String(ipsec.ipsecPrivateServerCertConf_);
        data.ipsec_public_server_cert_config = rust::String(ipsec.ipsecPublicServerCertConf_);
        data.ipsec_ca_cert_file_path = rust::String(ipsec.ipsecCaCertFilePath_);
        data.ipsec_private_user_cert_file_path = rust::String(ipsec.ipsecPrivateUserCertFilePath_);
        data.ipsec_public_user_cert_file_path = rust::String(ipsec.ipsecPublicUserCertFilePath_);
        data.ipsec_private_server_cert_file_path = rust::String(ipsec.ipsecPrivateServerCertFilePath_);
        data.ipsec_public_server_cert_file_path = rust::String(ipsec.ipsecPublicServerCertFilePath_);
        return;
    }
    if (IsL2tpType(vpnType)) {
        const auto &l2tp = static_cast<const NetManagerStandard::L2tpVpnConfig &>(config);
        data.ipsec_pre_shared_key = rust::String(l2tp.ipsecPreSharedKey_);
        data.ipsec_identifier = rust::String(l2tp.ipsecIdentifier_);
        data.swanctl_config = rust::String(""); // L2tpVpnConfig has no swanctlConf_
        data.strong_swan_config = rust::String(l2tp.strongswanConf_);
        data.ipsec_ca_cert_config = rust::String(l2tp.ipsecCaCertConf_);
        data.ipsec_private_user_cert_config = rust::String(l2tp.ipsecPrivateUserCertConf_);
        data.ipsec_public_user_cert_config = rust::String(l2tp.ipsecPublicUserCertConf_);
        data.ipsec_private_server_cert_config = rust::String(l2tp.ipsecPrivateServerCertConf_);
        data.ipsec_public_server_cert_config = rust::String(l2tp.ipsecPublicServerCertConf_);
        data.ipsec_ca_cert_file_path = rust::String(l2tp.ipsecCaCertFilePath_);
        data.ipsec_private_user_cert_file_path = rust::String(l2tp.ipsecPrivateUserCertFilePath_);
        data.ipsec_public_user_cert_file_path = rust::String(l2tp.ipsecPublicUserCertFilePath_);
        data.ipsec_private_server_cert_file_path = rust::String(l2tp.ipsecPrivateServerCertFilePath_);
        data.ipsec_public_server_cert_file_path = rust::String(l2tp.ipsecPublicServerCertFilePath_);
    }
}

static void WriteOpenvpnFull(SysVpnConfigFullAni &data,
    const NetManagerStandard::SysVpnConfig &config)
{
    const auto &ovpn = static_cast<const NetManagerStandard::OpenvpnConfig &>(config);
    data.ovpn_port = rust::String(ovpn.ovpnPort_);
    data.ovpn_protocol = ovpn.ovpnProtocol_;
    data.ovpn_config = rust::String(ovpn.ovpnConfig_);
    data.ovpn_auth_type = ovpn.ovpnAuthType_;
    data.askpass = rust::String(ovpn.askpass_);
    data.ovpn_config_file_path = rust::String(ovpn.ovpnConfigFilePath_);
    data.ovpn_ca_cert_file_path = rust::String(ovpn.ovpnCaCertFilePath_);
    data.ovpn_user_cert_file_path = rust::String(ovpn.ovpnUserCertFilePath_);
    data.ovpn_private_key_file_path = rust::String(ovpn.ovpnPrivateKeyFilePath_);
}

static void WriteL2tpUniqueFull(SysVpnConfigFullAni &data,
    const NetManagerStandard::SysVpnConfig &config)
{
    const auto &l2tp = static_cast<const NetManagerStandard::L2tpVpnConfig &>(config);
    data.ipsec_config = rust::String(l2tp.ipsecConf_);
    data.ipsec_secrets = rust::String(l2tp.ipsecSecrets_);
    data.options_l2tpd_client = rust::String(l2tp.optionsL2tpdClient_);
    data.xl2tpd_config = rust::String(l2tp.xl2tpdConf_);
    data.l2tp_shared_key = rust::String(l2tp.l2tpSharedKey_);
}

SysVpnConfigFullAni ConvertFromSysVpnConfigFull(const NetManagerStandard::SysVpnConfig &config)
{
    SysVpnConfigFullAni data;
    WriteVpnConfigBaseFull(data, config);
    WriteSysVpnConfigBaseFull(data, config);
    int32_t vpnType = config.vpnType_;
    if (IsIpsecType(vpnType)) {
        WriteIpsecCommonFull(data, config, vpnType);
    } else if (IsOpenvpnType(vpnType)) {
        WriteOpenvpnFull(data, config);
    } else if (IsL2tpType(vpnType)) {
        WriteIpsecCommonFull(data, config, vpnType);
        WriteL2tpUniqueFull(data, config);
    }
    return data;
}

int32_t CreateVpnConnection(int64_t &ptr)
{
    auto errCode = NetManagerStandard::NetworkVpnClient::GetInstance().CreateVpnConnection();
    if (errCode == NetManagerStandard::NETMANAGER_EXT_SUCCESS) {
        ptr = reinterpret_cast<int64_t>(&NetManagerStandard::NetworkVpnClient::GetInstance());
    }

    return errCode;
}

int32_t SetUp(int64_t nativePtr, const SysVpnConfigFullAni &config, int32_t &fd)
{
    if (nativePtr == 0) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    int32_t result = NetManagerStandard::NETMANAGER_EXT_SUCCESS;
#ifdef SUPPORT_SYSVPN
    if (config.vpn_type != 0) {
        auto sysVpnConfig = ConvertToSysVpnConfig(config);
        if (sysVpnConfig == nullptr) {
            return NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
        }
        result = NetManagerStandard::NetworkVpnClient::GetInstance().SetUpVpn(sysVpnConfig);
    } else {
        auto vpnConfig = ConvertToVpnConfigFull(config);
        if (vpnConfig == nullptr) {
            return NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
        }
        result =  NetManagerStandard::NetworkVpnClient::GetInstance().SetUpVpn(vpnConfig, fd);
    }
#else
    auto vpnConfig = ConvertToVpnConfigFull(config);
    if (vpnConfig == nullptr) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
    }
    result = NetManagerStandard::NetworkVpnClient::GetInstance().SetUpVpn(vpnConfig, fd);
#endif
    return result;
}

int32_t Protect(int64_t nativePtr, int32_t socketFd)
{
    if (nativePtr == 0) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    return NetManagerStandard::NetworkVpnClient::GetInstance().Protect(socketFd);
}

int32_t Destroy(int64_t nativePtr)
{
    if (nativePtr == 0) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    return NetManagerStandard::NetworkVpnClient::GetInstance().DestroyVpn(false);
}

int32_t AddSysVpnConfig(const SysVpnConfigFullAni &config)
{
    if (config.password.length() > MAX_PASSWORD_LENGTH ||
        config.pkcs12_password.length() > MAX_PASSWORD_LENGTH) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
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
    if (vpnId.empty()) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
#ifdef SUPPORT_SYSVPN
    return NetManagerStandard::NetworkVpnClient::GetInstance().DeleteSysVpnConfig(std::string(vpnId));
#else
    return NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
#endif
}

int32_t g_getSysVpnConfigList(rust::Vec<SysVpnConfigFullAni> &SysVpnConfigFullAniVec)
{
#ifdef SUPPORT_SYSVPN
    std::vector<sptr<NetManagerStandard::SysVpnConfig>> vpnList;
    int32_t ret = NetManagerStandard::NetworkVpnClient::GetInstance().GetSysVpnConfigList(vpnList);
    if (ret == NetManagerStandard::NETMANAGER_EXT_SUCCESS) {
        for (auto &config : vpnList) {
            if (config != nullptr) {
                SysVpnConfigFullAniVec.push_back(ConvertFromSysVpnConfigFull(*config));
            }
        }
    }
    return ret;
#else
    return NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
#endif
}

int32_t GetSysVpnConfig(const rust::String &vpnId, SysVpnConfigFullAni &sysVpnConfigFull)
{
    if (vpnId.empty()) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
#ifdef SUPPORT_SYSVPN
    sptr<NetManagerStandard::SysVpnConfig> config;
    int32_t errCode = NetManagerStandard::NetworkVpnClient::GetInstance().GetSysVpnConfig(
        config, std::string(vpnId));
    if (config != nullptr && errCode == NetManagerStandard::NETMANAGER_EXT_SUCCESS) {
        sysVpnConfigFull = ConvertFromSysVpnConfigFull(*config);
    }
    return errCode;
#else
    return NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
#endif
}

int32_t GetConnectedSysVpnConfig(int32_t &ret, SysVpnConfigFullAni &connectedSysVpnConfigFull)
{
#ifdef SUPPORT_SYSVPN
    sptr<NetManagerStandard::SysVpnConfig> config;
    int32_t errCode = NetManagerStandard::NetworkVpnClient::GetInstance().GetConnectedSysVpnConfig(config);
    ret = errCode;
    if (config != nullptr && errCode == NetManagerStandard::NETMANAGER_EXT_SUCCESS) {
        connectedSysVpnConfigFull = ConvertFromSysVpnConfigFull(*config);
    }
    return errCode;
#else
    ret = NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
    return ret;
#endif
}

int32_t GetConnectedVpnAppInfo(rust::Vec<rust::String> &appInfo)
{
#ifdef SUPPORT_SYSVPN
    std::vector<std::string> bundleNameList;
    int32_t errCode = NetManagerStandard::NetworkVpnClient::GetInstance().GetConnectedVpnAppInfo(
        bundleNameList);
    for (auto &name : bundleNameList) {
        appInfo.push_back(rust::String(name));
    }
    return errCode;
#else
    return NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
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
    int32_t errCode = NetManagerStandard::NetworkVpnClient::GetInstance().RegisterVpnEvent(
        g_vpnEventCallbackObserverAni);
#ifdef SUPPORT_SYSVPN
    if (errCode == NetManagerStandard::NETMANAGER_EXT_SUCCESS) {
        errCode = NetManagerStandard::NetworkVpnClient::GetInstance().RegisterMultiVpnEvent(
            g_vpnEventCallbackObserverAni);
    }
#endif
    if (errCode == NetManagerStandard::NETMANAGER_EXT_SUCCESS) {
        g_isVpnObserverRegistered = true;
    } else {
        NetManagerStandard::NetworkVpnClient::GetInstance().UnregisterVpnEvent(
            g_vpnEventCallbackObserverAni);
        g_vpnEventCallbackObserverAni = nullptr;
    }
    return errCode;
}

int32_t VpnObserverUnRegister()
{
    std::lock_guard<std::mutex> lock(g_vpnObserverMutex);
    if (g_vpnEventCallbackObserverAni == nullptr) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }

    int32_t errCode = NetManagerStandard::NetworkVpnClient::GetInstance().UnregisterVpnEvent(
        g_vpnEventCallbackObserverAni);
#ifdef SUPPORT_SYSVPN
    if (errCode == NetManagerStandard::NETMANAGER_EXT_SUCCESS) {
        errCode = NetManagerStandard::NetworkVpnClient::GetInstance().UnregisterMultiVpnEvent(
            g_vpnEventCallbackObserverAni);
    }
#endif
    if (errCode == NetManagerStandard::NETMANAGER_EXT_SUCCESS) {
        g_vpnEventCallbackObserverAni = nullptr;
        g_isVpnObserverRegistered = false;
    }
    return errCode;
}

} // namespace NetManagerAni
} // namespace OHOS
