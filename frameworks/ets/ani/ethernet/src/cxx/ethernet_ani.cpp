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

#include "ethernet_ani.h"

#include <cstdlib>
#include <sstream>

#include "errorcode_convertor.h"
#include "ethernet_client.h"
#include "interface_configuration.h"
#include "net_manager_constants.h"

#include "wrapper.rs.h"

namespace OHOS {
namespace NetManagerAni {

namespace {
sptr<InterfaceStateCallbackObserverAni> g_interfaceStateCallbackObserverAni =
    sptr<InterfaceStateCallbackObserverAni>(new (std::nothrow) InterfaceStateCallbackObserverAni());

bool g_isInterfaceStateObserverRegistered = false;
} // namespace

// Returns the enabled status; error code is output via ret parameter.
// This dual-return pattern is required by the cxx bridge: the Rust side needs
// both the business result and the error code, but cxx only supports a single
// return value.
bool IsEthernetEnabled(int32_t &ret)
{
#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
    int32_t enabled = 0;
    ret = DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->IsEthernetEnabled(enabled);
    return enabled != 0;
#else
    // Feature not compiled; return standard error code for operation failure.
    ret = NetManagerStandard::NETMANAGER_ERR_OPERATION_FAILED;
    return false;
#endif
}

int32_t DisableEthernetInterface()
{
#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
    int32_t ret = DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->DisableEthernetInterface();
    return ret;
#else
    return NetManagerStandard::NETMANAGER_ERR_OPERATION_FAILED;
#endif
}

int32_t EnableEthernetInterface()
{
#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
    int32_t ret = DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->EnableEthernetInterface();
    return ret;
#else
    return NetManagerStandard::NETMANAGER_ERR_OPERATION_FAILED;
#endif
}

int32_t g_getAllActiveIfaces(rust::Vec<rust::String> &activeIfaces)
{
    std::vector<std::string> ifacesVec;
    int32_t ret =
        DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->GetAllActiveIfaces(ifacesVec);
    for (auto &iface : ifacesVec) {
        activeIfaces.push_back(iface);
    }
    return ret;
}

int32_t IsIfaceActive(const rust::String &iface, int32_t &activeStatus)
{
    int32_t ret = DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->IsIfaceActive(
        std::string(iface), activeStatus);
    return ret;
}

static std::string GetFirstAddrFromList(const std::vector<NetManagerStandard::INetAddr> &addrList)
{
    if (!addrList.empty()) {
        return addrList[0].address_;
    }
    return std::string();
}

// Escape commas and semicolons in field content so that delimiter-based
// parsing is not confused by field-internal occurrences of these characters.
static std::string EscapeField(const std::string &field)
{
    std::string escaped;
    escaped.reserve(field.size());
    for (char c : field) {
        if (c == ',' || c == ';') {
            escaped.push_back('\\');
        }
        escaped.push_back(c);
    }
    return escaped;
}

// Unescape a field that was produced by EscapeField.
// ESCAPE_SEQUENCE_LEN: a backslash ('\') plus the escaped character behind it.
static constexpr size_t ESCAPE_SEQUENCE_LEN = 2;

static std::string UnescapeField(const std::string &field)
{
    std::string result;
    result.reserve(field.size());
    size_t i = 0;
    while (i < field.size()) {
        if (field[i] == '\\' && i + 1 < field.size() && (field[i + 1] == ',' || field[i + 1] == ';')) {
            result.push_back(field[i + 1]);
            i += ESCAPE_SEQUENCE_LEN;
        } else {
            result.push_back(field[i]);
            ++i;
        }
    }
    return result;
}

static std::string SerializeDnsServers(const std::vector<NetManagerStandard::INetAddr> &dnsServers)
{
    std::string dnsStr;
    for (size_t i = 0; i < dnsServers.size(); ++i) {
        if (i > 0) {
            dnsStr += ";";
        }
        dnsStr += dnsServers[i].address_;
    }
    return dnsStr;
}

static std::string SerializeHttpProxy(const NetManagerStandard::HttpProxy &proxy)
{
    std::ostringstream oss;
    if (!proxy.GetHost().empty()) {
        oss << proxy.GetHost() << ":" << proxy.GetPort() << ":";
        const auto &exclList = proxy.GetExclusionList();
        bool first = true;
        for (const auto &excl : exclList) {
            if (!first) {
                oss << ";";
            }
            oss << EscapeField(excl);
            first = false;
        }
    }
    return oss.str();
}

// Returns the serialized config string; error code is output via ret parameter.
// Empty string on error. The dual-return pattern is required by cxx bridge.
rust::String GetIfaceConfig(const rust::String &iface, int32_t &ret)
{
    sptr<NetManagerStandard::InterfaceConfiguration> config;
    ret = DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->GetIfaceConfig(
        std::string(iface), config);
    if (ret != 0 || config == nullptr) {
        return rust::String();
    }
    std::ostringstream oss;
    oss << static_cast<int32_t>(config->mode_)
        << "," << EscapeField(GetFirstAddrFromList(config->ipStatic_.ipAddrList_))
        << "," << EscapeField(GetFirstAddrFromList(config->ipStatic_.routeList_))
        << "," << EscapeField(GetFirstAddrFromList(config->ipStatic_.gatewayList_))
        << "," << EscapeField(GetFirstAddrFromList(config->ipStatic_.netMaskList_))
        << "," << EscapeField(SerializeDnsServers(config->ipStatic_.dnsServers_))
        << "," << EscapeField(SerializeHttpProxy(config->httpProxy_));
    return rust::String(oss.str());
}

static void AddNetAddrToList(std::vector<NetManagerStandard::INetAddr> &list, const rust::String &addr)
{
    if (addr.empty()) {
        return;
    }
    NetManagerStandard::INetAddr netAddr;
    netAddr.address_ = std::string(addr);
    list.push_back(netAddr);
}

static void AddDnsServersToList(std::vector<NetManagerStandard::INetAddr> &list, const rust::String &dnsServers)
{
    if (dnsServers.empty()) {
        return;
    }
    std::vector<std::string> dnsList;
    size_t pos = 0;
    std::string dnsStr = std::string(dnsServers);
    std::string delimiter = ";";
    while ((pos = dnsStr.find(delimiter)) != std::string::npos) {
        dnsList.push_back(dnsStr.substr(0, pos));
        dnsStr.erase(0, pos + delimiter.length());
    }
    if (!dnsStr.empty()) {
        dnsList.push_back(dnsStr);
    }
    for (auto &dns : dnsList) {
        if (dns.empty()) {
            continue;
        }
        NetManagerStandard::INetAddr dnsNet;
        dnsNet.address_ = dns;
        list.push_back(dnsNet);
    }
}

static bool SafeStoi(const std::string &s, int32_t &out)
{
    if (s.empty()) {
        return false;
    }
    char *end = nullptr;
    long val = std::strtol(s.c_str(), &end, 10);
    if (end == s.c_str() || *end != '\0' || val < INT32_MIN || val > INT32_MAX) {
        return false;
    }
    out = static_cast<int32_t>(val);
    return true;
}

static std::vector<std::string> ParseConfigString(const std::string &configStr)
{
    std::vector<std::string> parts;
    std::string part;
    bool escaped = false;
    for (char c : configStr) {
        if (escaped) {
            part.push_back(c);
            escaped = false;
            continue;
        }
        if (c == '\\') {
            escaped = true;
            continue;
        }
        if (c == ',') {
            parts.push_back(part);
            part.clear();
            continue;
        }
        part.push_back(c);
    }
    parts.push_back(part);
    return parts;
}

static int32_t ValidateAndParseMode(const std::vector<std::string> &parts)
{
    const int32_t CONFIG_INDEX_MODE = 0;
    // Standard error code for invalid parameter, consistent with FFI error codes.
    const int32_t ERR_INVALID_PARAMETER = 2100001;
    int32_t mode = 0;
    if (!parts.empty()) {
        if (!SafeStoi(parts[CONFIG_INDEX_MODE], mode)) {
            return ERR_INVALID_PARAMETER;
        }
    }
    if (mode < static_cast<int32_t>(NetManagerStandard::IPSetMode::STATIC) ||
        mode > static_cast<int32_t>(NetManagerStandard::IPSetMode::LAN_DHCP)) {
        return ERR_INVALID_PARAMETER;
    }
    return mode;
}

static void PopulateInterfaceConfig(sptr<NetManagerStandard::InterfaceConfiguration> config,
                                    const std::vector<std::string> &parts)
{
    const int32_t CONFIG_INDEX_IP_ADDR = 1;
    const int32_t CONFIG_INDEX_ROUTE = 2;
    const int32_t CONFIG_INDEX_GATEWAY = 3;
    const int32_t CONFIG_INDEX_NET_MASK = 4;
    const int32_t CONFIG_INDEX_DNS_SERVERS = 5;

    if (parts.size() > CONFIG_INDEX_IP_ADDR) {
        AddNetAddrToList(config->ipStatic_.ipAddrList_, rust::String(UnescapeField(parts[CONFIG_INDEX_IP_ADDR])));
    }
    if (parts.size() > CONFIG_INDEX_ROUTE) {
        AddNetAddrToList(config->ipStatic_.routeList_, rust::String(UnescapeField(parts[CONFIG_INDEX_ROUTE])));
    }
    if (parts.size() > CONFIG_INDEX_GATEWAY) {
        AddNetAddrToList(config->ipStatic_.gatewayList_, rust::String(UnescapeField(parts[CONFIG_INDEX_GATEWAY])));
    }
    if (parts.size() > CONFIG_INDEX_NET_MASK) {
        AddNetAddrToList(config->ipStatic_.netMaskList_,
                         rust::String(UnescapeField(parts[CONFIG_INDEX_NET_MASK])));
    }
    if (parts.size() > CONFIG_INDEX_DNS_SERVERS) {
        AddDnsServersToList(config->ipStatic_.dnsServers_,
                            rust::String(UnescapeField(parts[CONFIG_INDEX_DNS_SERVERS])));
    }
}

static std::list<std::string> ParseProxyExclusionList(const std::string &exclStr)
{
    std::list<std::string> exclList;
    std::istringstream exclIss(exclStr);
    std::string excl;
    while (std::getline(exclIss, excl, ';')) {
        if (!excl.empty()) {
            exclList.push_back(UnescapeField(excl));
        }
    }
    return exclList;
}

static bool ParseIpv6ProxyParts(const std::string &proxyStr, std::vector<std::string> &proxyParts)
{
    const size_t MIN_IPV6_PREFIX_LEN = 2;
    const size_t OFFSET_AFTER_BRACKET = 2;
    if (proxyStr.size() < MIN_IPV6_PREFIX_LEN || proxyStr[0] != '[') {
        return false;
    }
    auto closingBracket = proxyStr.find(']');
    if (closingBracket == std::string::npos) {
        return false;
    }
    proxyParts.push_back(proxyStr.substr(0, closingBracket + 1)); // host: [::1]
    if (closingBracket + OFFSET_AFTER_BRACKET < proxyStr.size() && proxyStr[closingBracket + 1] == ':') {
        std::string remaining = proxyStr.substr(closingBracket + OFFSET_AFTER_BRACKET);
        std::istringstream remainIss(remaining);
        std::string remainPart;
        while (std::getline(remainIss, remainPart, ':')) {
            proxyParts.push_back(remainPart);
        }
    }
    return true;
}

static std::string JoinProxyExclusionParts(const std::vector<std::string> &proxyParts, size_t startIndex)
{
    std::string exclStr;
    for (size_t i = startIndex; i < proxyParts.size(); ++i) {
        if (proxyParts[i].empty()) {
            continue;
        }
        if (!exclStr.empty()) {
            exclStr += ";";
        }
        exclStr += proxyParts[i];
    }
    return exclStr;
}

static void ApplyProxyConfig(NetManagerStandard::InterfaceConfiguration &config,
                             std::vector<std::string> &proxyParts)
{
    const int32_t PROXY_INDEX_HOST = 0;
    const int32_t PROXY_INDEX_PORT = 1;
    const int32_t PROXY_INDEX_EXCL_LIST = 2;

    config.httpProxy_.SetHost(std::move(proxyParts[PROXY_INDEX_HOST]));
    int32_t portVal = 0;
    if (SafeStoi(proxyParts[PROXY_INDEX_PORT], portVal) && portVal > 0 && portVal <= 0xFFFF) {
        config.httpProxy_.SetPort(static_cast<uint16_t>(portVal));
    } else {
        config.httpProxy_.SetPort(0);
    }
    if (proxyParts.size() <= PROXY_INDEX_EXCL_LIST) {
        return;
    }
    std::string exclStr = JoinProxyExclusionParts(proxyParts, PROXY_INDEX_EXCL_LIST);
    if (!exclStr.empty()) {
        config.httpProxy_.SetExclusionList(ParseProxyExclusionList(exclStr));
    }
}

static void ParseHttpProxy(NetManagerStandard::InterfaceConfiguration &config, const std::string &proxyStr)
{
    const int32_t PROXY_MIN_PARTS_FOR_HOST_PORT = 2;

    std::vector<std::string> proxyParts;
    if (!ParseIpv6ProxyParts(proxyStr, proxyParts)) {
        std::istringstream proxyIss(proxyStr);
        std::string proxyPart;
        while (std::getline(proxyIss, proxyPart, ':')) {
            proxyParts.push_back(proxyPart);
        }
    }
    if (proxyParts.size() < PROXY_MIN_PARTS_FOR_HOST_PORT || proxyParts[0].empty()) {
        return;
    }
    ApplyProxyConfig(config, proxyParts);
}

int32_t SetIfaceConfig(const rust::String &iface, const rust::String &configStr)
{
    std::string str = std::string(configStr);
    std::vector<std::string> parts = ParseConfigString(str);

    const int32_t CONFIG_INDEX_HTTP_PROXY = 6;
    // Standard error codes, consistent with FFI error codes.
    const int32_t ERR_INVALID_PARAMETER = 2100001;
    const int32_t ERR_INTERNAL = 2100003;

    int32_t mode = ValidateAndParseMode(parts);
    if (mode == ERR_INVALID_PARAMETER) {
        return ERR_INVALID_PARAMETER;
    }

    // sptr is a ref-counted smart pointer; config is automatically released when
    // it goes out of scope regardless of the return path below.
    sptr<NetManagerStandard::InterfaceConfiguration> config =
        new (std::nothrow) NetManagerStandard::InterfaceConfiguration();
    if (config == nullptr) {
        return ERR_INTERNAL;
    }
    config->mode_ = static_cast<NetManagerStandard::IPSetMode>(mode);

    PopulateInterfaceConfig(config, parts);

    // Parse HttpProxy: host:port:excl1;excl2
    if (parts.size() > CONFIG_INDEX_HTTP_PROXY && !parts[CONFIG_INDEX_HTTP_PROXY].empty()) {
        ParseHttpProxy(*config, parts[CONFIG_INDEX_HTTP_PROXY]);
    }

    config->ipStatic_.domain_ = std::string("");

    int32_t ret = DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->SetIfaceConfig(
        std::string(iface), config);
    return ret;
}

// Returns serialized device info; error code via ret. Dual-return for cxx bridge.
rust::String GetDeviceInformation(int32_t &ret)
{
    std::vector<NetManagerStandard::EthernetDeviceInfo> deviceInfoList;
    ret = DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->GetDeviceInformation(deviceInfoList);
    if (ret != 0) {
        return rust::String();
    }
    std::ostringstream oss;
    for (auto &info : deviceInfoList) {
        if (oss.tellp() > 0) {
            oss << ";";
        }
        oss << EscapeField(info.ifaceName_) << "," << EscapeField(info.deviceName_) << "," <<
            static_cast<int32_t>(info.connectionMode_) << "," <<
            EscapeField(info.supplierName_) << "," << EscapeField(info.supplierId_) << "," <<
            EscapeField(info.productName_) << "," << EscapeField(info.maximumRate_);
    }
    return rust::String(oss.str());
}

// Returns serialized MAC address info; error code via ret. Dual-return for cxx bridge.
rust::String GetMacAddress(int32_t &ret)
{
    std::vector<NetManagerStandard::MacAddressInfo> macAddrList;
    ret = DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->GetMacAddress(macAddrList);
    if (ret != 0) {
        return rust::String();
    }
    std::ostringstream oss;
    for (auto &info : macAddrList) {
        if (oss.tellp() > 0) {
            oss << ";";
        }
        oss << EscapeField(info.iface_) << "," << EscapeField(info.macAddress_);
    }
    return rust::String(oss.str());
}

rust::String GetErrorCodeAndMessage(int32_t &errorCode)
{
    NetManagerStandard::NetBaseErrorCodeConvertor convertor;
    rust::String result = rust::string(convertor.ConvertErrorCode(errorCode));
    return result;
}

int32_t InterfaceStateCallbackObserverAni::OnInterfaceAdded(const std::string &ifName)
{
    execute_interface_added(rust::String(ifName));
    return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
}

int32_t InterfaceStateCallbackObserverAni::OnInterfaceRemoved(const std::string &ifName)
{
    execute_interface_removed(rust::String(ifName));
    return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
}

int32_t InterfaceStateCallbackObserverAni::OnInterfaceChanged(const std::string &ifName, bool up)
{
    InterfaceStateInfo info{
        .iface = rust::String(ifName),
        .active = up,
    };
    execute_interface_changed(info);
    return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
}

int32_t RegisterInterfaceStateCallback()
{
    if (g_isInterfaceStateObserverRegistered) {
        return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
    }

    // Ensure the observer instance is valid before proceeding. If the initial
    // nothrow allocation failed (unlikely but possible under memory pressure),
    // attempt a re-creation so that subsequent registration requests can succeed
    // once memory is available again, rather than permanently failing.
    if (g_interfaceStateCallbackObserverAni == nullptr) {
        g_interfaceStateCallbackObserverAni =
            sptr<InterfaceStateCallbackObserverAni>(new (std::nothrow) InterfaceStateCallbackObserverAni());
        if (g_interfaceStateCallbackObserverAni == nullptr) {
            return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
        }
    }

    auto client = DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance();
    if (client == nullptr) {
        return NetManagerStandard::NETMANAGER_ERR_OPERATION_FAILED;
    }

    int32_t result = client->RegisterIfacesStateChanged(g_interfaceStateCallbackObserverAni);
    if (result == NetManagerStandard::NETMANAGER_EXT_SUCCESS) {
        g_isInterfaceStateObserverRegistered = true;
    }
    return result;
}

int32_t UnregisterInterfaceStateCallback()
{
    if (g_interfaceStateCallbackObserverAni == nullptr) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }

    auto client = DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance();
    if (client == nullptr) {
        return NetManagerStandard::NETMANAGER_ERR_OPERATION_FAILED;
    }

    int32_t result = client->UnregisterIfacesStateChanged(g_interfaceStateCallbackObserverAni);
    if (result == NetManagerStandard::NETMANAGER_EXT_SUCCESS) {
        g_isInterfaceStateObserverRegistered = false;
    }
    return result;
}

} // namespace NetManagerAni
} // namespace OHOS
