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

#include "vpnextension_ani.h"

#include <array>
#include <atomic>
#include <cerrno>
#include <condition_variable>
#include <cstdint>
#include <ctime>
#include <sys/time.h>
#include <mutex>
#include <string>

#include "ability_manager_client.h"
#include "ability_connect_callback_stub.h"
#include "cxx.h"
#include "errorcode_convertor.h"
#include "extension_ability_info.h"
#include "ipc_skeleton.h"
#include "net_conn_client.h"
#include "net_datashare_utils_iface.h"
#include "net_manager_ext_constants.h"
#include "net_manager_constants.h"
#include "parameters.h"
#include "want.h"
#include "want_params.h"

#ifdef SUPPORT_SYSVPN
#include "os_account_manager.h"
#endif // SUPPORT_SYSVPN

namespace OHOS {
namespace NetManagerAni {

// ============================================================================
// Constants (mirrored from NAPI vpnext module)
// ============================================================================
namespace {
constexpr const char *VPNEXT_MODE_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=vpnext_mode";
constexpr const char *VPNEXT_ALWAYSON_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=vpnext_alwayson";
constexpr const char *VPN_DIALOG_POSTFIX = "**vpndialog**";
constexpr const char *VPN_DIALOG_BUNDLENAME = "com.ohos.vpndialog";
constexpr const char *EDM_VPN_DISABLE_KEY = "persist.edm.vpn_disable";
constexpr int32_t MAX_BUNDLE_NAME_LENGTH = 256;
constexpr int32_t UUID128_BYTES_LEN = 16;
constexpr int32_t HEX_CHARS_PER_BYTE = 2;

// 16-byte UUID indices (access by name, not magic number)
constexpr int32_t UUID_BYTE_0  = 0;
constexpr int32_t UUID_BYTE_1  = 1;
constexpr int32_t UUID_BYTE_2  = 2;
constexpr int32_t UUID_BYTE_3  = 3;
constexpr int32_t UUID_BYTE_4  = 4;
constexpr int32_t UUID_BYTE_5  = 5;
constexpr int32_t UUID_BYTE_6  = 6;
constexpr int32_t UUID_BYTE_7  = 7;
constexpr int32_t UUID_BYTE_8  = 8;
constexpr int32_t UUID_BYTE_9  = 9;
constexpr int32_t UUID_BYTE_10 = 10;
constexpr int32_t UUID_BYTE_11 = 11;
constexpr int32_t UUID_BYTE_12 = 12;
constexpr int32_t UUID_BYTE_13 = 13;
constexpr int32_t UUID_BYTE_14 = 14;
constexpr int32_t UUID_BYTE_15 = 15;

// Byte-level bit shift amounts (1 byte = 8 bits)
constexpr int32_t SHIFT_HALF_BYTE = 4;
constexpr int32_t SHIFT_1_BYTE = 8;
constexpr int32_t SHIFT_2_BYTES = 16;
constexpr int32_t SHIFT_3_BYTES = 24;
constexpr int32_t SHIFT_4_BYTES = 32;

// Nibble mask (low 4 bits)
constexpr uint8_t NIBBLE_MASK = 0xF;

// UUID format section positions (RFC 4122 layout)
constexpr size_t UUID_TIME_LOW_START = 0;
constexpr size_t UUID_TIME_LOW_LEN = 8;
constexpr size_t UUID_TIME_MID_START = 8;
constexpr size_t UUID_TIME_MID_LEN = 4;
constexpr size_t UUID_TIME_HIGH_START = 12;
constexpr size_t UUID_TIME_HIGH_LEN = 4;
constexpr size_t UUID_CLOCK_SEQ_START = 16;
constexpr size_t UUID_CLOCK_SEQ_LEN = 4;
constexpr size_t UUID_NODE_START = 20;

// Permission check result codes (ProcessPermissionRequests)
constexpr int32_t PERM_RESULT_AUTHORIZED = 0;

// Helper: Validate bundle name contains only safe characters for URI construction
// BundleName should only contain alphanumeric, dot, and underscore characters
static bool IsValidBundleName(const std::string &bundleName)
{
    if (bundleName.empty() || bundleName.length() > MAX_BUNDLE_NAME_LENGTH) {
        return false;
    }
    for (char c : bundleName) {
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '.' || c == '_')) {
            return false;
        }
    }
    return true;
}
constexpr int32_t PERM_RESULT_AUTHORIZED_AFTER_DIALOG = 1;
constexpr int32_t PERM_RESULT_DENIED_AFTER_DIALOG = 2;
constexpr int32_t PERM_RESULT_FAILED = -1;

// Dialog timeout
constexpr int32_t VPN_DIALOG_TIMEOUT_SEC = 30;
} // anonymous namespace

// ============================================================================
// UUID Generation (equivalent to vpnext NAPI UUID class)
// ============================================================================
class UuidGenerator {
public:
    static std::string Generate()
    {
        std::array<uint8_t, UUID128_BYTES_LEN> uuid = {0x00};
        struct timeval tv {};
        struct timezone tz {};
        struct tm randomTime {};
        unsigned int randNum = 0;

        rand_r(&randNum);
        gettimeofday(&tv, &tz);
        if (localtime_r(&tv.tv_sec, &randomTime) == nullptr) {
            return "";
        }

        uuid[UUID_BYTE_15] = static_cast<uint8_t>(tv.tv_usec & 0xFF);
        uuid[UUID_BYTE_14] = static_cast<uint8_t>((tv.tv_usec & 0xFF00) >> SHIFT_1_BYTE);
        uuid[UUID_BYTE_13] = static_cast<uint8_t>((tv.tv_usec & 0xFF0000) >> SHIFT_2_BYTES);
        uuid[UUID_BYTE_12] = static_cast<uint8_t>((tv.tv_usec & 0xFF000000) >> SHIFT_3_BYTES);
        uuid[UUID_BYTE_11] = static_cast<uint8_t>(tv.tv_sec & 0xFF);
        uuid[UUID_BYTE_10] = static_cast<uint8_t>((tv.tv_sec & 0xFF00) >> SHIFT_1_BYTE);
        uuid[UUID_BYTE_9]  = static_cast<uint8_t>((tv.tv_sec & 0xFF0000) >> SHIFT_2_BYTES);
        uuid[UUID_BYTE_8]  = static_cast<uint8_t>((tv.tv_sec & 0xFF000000) >> SHIFT_3_BYTES);
        uuid[UUID_BYTE_7]  = static_cast<uint8_t>((tv.tv_sec & 0xFF00000000) >> SHIFT_4_BYTES);
        uuid[UUID_BYTE_6]  = static_cast<uint8_t>((randomTime.tm_sec + static_cast<int>(randNum)) & 0xFF);
        uuid[UUID_BYTE_5]  = static_cast<uint8_t>((randomTime.tm_min + (randNum >> SHIFT_1_BYTE)) & 0xFF);
        uuid[UUID_BYTE_4]  = static_cast<uint8_t>((randomTime.tm_hour + (randNum >> SHIFT_2_BYTES)) & 0xFF);
        uuid[UUID_BYTE_3]  = static_cast<uint8_t>((randomTime.tm_mday + (randNum >> SHIFT_3_BYTES)) & 0xFF);
        uuid[UUID_BYTE_2]  = static_cast<uint8_t>(randomTime.tm_mon & 0xFF);
        uuid[UUID_BYTE_1]  = static_cast<uint8_t>(randomTime.tm_year & 0xFF);
        uuid[UUID_BYTE_0]  = static_cast<uint8_t>((randomTime.tm_year & 0xFF00) >> SHIFT_1_BYTE);

        return FormatUuid(uuid);
    }

private:
    static std::string FormatUuid(const std::array<uint8_t, UUID128_BYTES_LEN> &uuid)
    {
        static const char *hex = "0123456789ABCDEF";
        std::string tmp;
        tmp.reserve(UUID128_BYTES_LEN * HEX_CHARS_PER_BYTE);
        for (auto byte : uuid) {
            tmp.push_back(hex[(byte >> SHIFT_HALF_BYTE) & NIBBLE_MASK]);
            tmp.push_back(hex[byte & NIBBLE_MASK]);
        }
        return tmp.substr(UUID_TIME_LOW_START, UUID_TIME_LOW_LEN) + "-" +
               tmp.substr(UUID_TIME_MID_START, UUID_TIME_MID_LEN) + "-" +
               tmp.substr(UUID_TIME_HIGH_START, UUID_TIME_HIGH_LEN) + "-" +
               tmp.substr(UUID_CLOCK_SEQ_START, UUID_CLOCK_SEQ_LEN) + "-" +
               tmp.substr(UUID_NODE_START);
    }
};

// ============================================================================
// VpnAbilityConn - connection callback for VPN dialog (mirrors NAPI VpnMonitor::VpnAbilityConn)
// ============================================================================
class VpnAbilityConn : public AAFwk::AbilityConnectionStub {
    void OnAbilityConnectDone(const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int32_t resultCode) override
    {
    }
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode) override
    {
    }
};

// ============================================================================
// VpnMonitor replacement (no NAPI dependencies)
// ============================================================================
class AniVpnMonitor {
public:
    static AniVpnMonitor &GetInstance()
    {
        static AniVpnMonitor instance;
        return instance;
    }

    void CacheCurrentWant(const AAFwk::Want &want)
    {
        std::lock_guard<std::mutex> lock(wantMutex_);
        cachedWant_ = want;
    }

    AAFwk::Want GetCachedWant()
    {
        std::lock_guard<std::mutex> lock(wantMutex_);
        if (!cachedWant_.GetElement().GetBundleName().empty()) {
            return cachedWant_;
        }
        return AAFwk::Want();
    }

    bool ShowVpnDialog(const std::string &bundleName, const std::string &abilityName,
                       const std::string &appName)
    {
        auto abmc = AAFwk::AbilityManagerClient::GetInstance();
        if (abmc == nullptr) {
            return false;
        }

        AAFwk::Want cachedwant = GetCachedWant();
        if (cachedwant.GetElement().GetBundleName().empty()) {
            return false;
        }
        AAFwk::WantParams cachedParams = cachedwant.GetParams();
        AAFwk::Want want;
        AAFwk::WantParams wantParams = want.GetParams();
        wantParams.SetParam("myParams", AAFwk::WantParamWrapper::Box(cachedParams));
        want.SetParams(wantParams);
        want.SetElementName(VPN_DIALOG_BUNDLENAME, "VpnServiceExtAbility");
        want.SetParam("bundleName", bundleName);
        std::string finalAbilityName = abilityName;
        if (abilityName.length() < std::strlen(VPN_DIALOG_POSTFIX) ||
            abilityName.find(VPN_DIALOG_POSTFIX) == std::string::npos) {
            finalAbilityName = abilityName + VPN_DIALOG_POSTFIX;
        }
        want.SetParam("abilityName", finalAbilityName);
        want.SetParam("appName", appName);

        sptr<VpnAbilityConn> vpnAbilityConn = sptr<VpnAbilityConn>::MakeSptr();
        auto ret = abmc->ConnectAbility(want, vpnAbilityConn, -1);
        if (ret != 0) {
            return false;
        }
        return true;
    }

private:
    AniVpnMonitor() = default;
    ~AniVpnMonitor() = default;
    AniVpnMonitor(const AniVpnMonitor &) = delete;
    AniVpnMonitor &operator=(const AniVpnMonitor &) = delete;

    AAFwk::Want cachedWant_;
    std::mutex wantMutex_;
};

// ============================================================================
// VPnExtensionObserver - IPC callback stub for VPN events
// Bridges state changes to Rust register layer for onAuthorizationResult
// ============================================================================
class VpnExtObserverCallback : public IRemoteStub<NetManagerStandard::IVpnEventCallback> {
public:
    int32_t OnVpnStateChanged(bool isConnected, const sptr<NetManagerStandard::VpnState> &vpnState) override
    {
        execute_vpn_ext_authorization_result(isConnected);
        return 0;
    }
    int32_t OnMultiVpnStateChanged(bool isConnected, const std::string &bundleName,
        const std::string &vpnId) override
    {
        return 0;
    }
    int32_t OnVpnMultiUserSetUp() override
    {
        return 0;
    }
};

// ============================================================================
// Global observer pointer
// ============================================================================
static sptr<VpnExtObserverCallback> g_vpnExtObserver = nullptr;
static std::mutex g_vpnExtObserverMutex;

// ============================================================================
// Helper: Remove VPN_DIALOG_POSTFIX from ability name
// ============================================================================
static std::string RemoveVpnDialogPostfix(const std::string &s)
{
    constexpr std::string_view postfix = VPN_DIALOG_POSTFIX;
    auto pos = s.rfind(postfix);
    if (pos == std::string::npos || pos + postfix.length() != s.length()) {
        return s;
    }
    std::string result = s;
    result.erase(pos);
    return result;
}

#ifdef SUPPORT_SYSVPN
static int32_t CheckVpnPermission(const std::string &bundleName, std::string &vpnExtMode)
{
    int32_t userId = AppExecFwk::Constants::UNSPECIFIED_USERID;
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(uid, userId) != ERR_OK) {
        return -1;
    }

    // First, check user-specific configuration (takes priority)
    std::string userKey = bundleName + "_" + std::to_string(userId);
    int32_t ret = NetManagerStandard::NetDataShareHelperUtilsIface::Query(VPNEXT_MODE_URI, userKey, vpnExtMode);
    if (ret == 0 && vpnExtMode == "1") {
        // User-specific config exists and is authorized
        return 0;
    }

    // User-specific config not found or not authorized, fall back to global config
    ret = NetManagerStandard::NetDataShareHelperUtilsIface::Query(VPNEXT_MODE_URI, bundleName, vpnExtMode);
    if (ret != 0 || vpnExtMode != "1") {
        return -1;
    }

    return 0;
}
#endif // SUPPORT_SYSVPN

// ============================================================================
// ProcessPermissionRequests - Check VPN authorization via DataShare
// Uses PERM_RESULT_* constants defined above
static std::string BuildVpnDialogKey(const std::string &bundleName)
{
    std::string key = bundleName;
#ifdef SUPPORT_SYSVPN
    int32_t userId = AppExecFwk::Constants::UNSPECIFIED_USERID;
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(uid, userId) != ERR_OK) {
        return "";
    }
    key = bundleName + "_" + std::to_string(userId);
#endif
    return key;
}

static int32_t WaitForObserverResult(std::shared_ptr<int32_t> callbackId,
    std::mutex &waitMutex, std::condition_variable &waitCv, std::atomic<bool> &observerFired)
{
    std::unique_lock<std::mutex> lock(waitMutex);
    bool waitResult = waitCv.wait_for(lock, std::chrono::seconds(VPN_DIALOG_TIMEOUT_SEC), [&observerFired]() {
        return observerFired.load();
    });
    if (!waitResult) {
        if (callbackId) {
            NetManagerStandard::NetDataShareHelperUtilsIface::UnregisterObserver(VPNEXT_MODE_URI, *callbackId);
        }
        return PERM_RESULT_FAILED;
    }
    return observerFired.load() ? PERM_RESULT_AUTHORIZED_AFTER_DIALOG : PERM_RESULT_DENIED_AFTER_DIALOG;
}

// Show VPN authorization dialog, register DataShare observer, and block until user responds.
// Returns PERM_RESULT_AUTHORIZED_AFTER_DIALOG, PERM_RESULT_DENIED_AFTER_DIALOG, or PERM_RESULT_FAILED.
static int32_t WaitForVpnDialogAuthorization(const std::string &bundleName,
    const std::string &abilityName, const std::string &selfAppName)
{
    if (!AniVpnMonitor::GetInstance().ShowVpnDialog(bundleName, abilityName, selfAppName)) {
        return PERM_RESULT_FAILED;
    }

    std::string key = BuildVpnDialogKey(bundleName);
    if (key.empty()) {
        return PERM_RESULT_FAILED;
    }

    std::mutex waitMutex;
    std::condition_variable waitCv;
    std::atomic<bool> observerFired{false};
    auto once = std::make_shared<std::once_flag>();
    auto callbackId = std::make_shared<int32_t>();

    auto onChange = [once, &waitMutex, &waitCv, &observerFired, callbackId, key]() {
        std::call_once(*once, [&waitMutex, &waitCv, &observerFired, callbackId, key]() {
            std::string vpnExtModeAfter = "0";
            NetManagerStandard::NetDataShareHelperUtilsIface::Query(
                VPNEXT_MODE_URI, key, vpnExtModeAfter);
            if (callbackId) {
                NetManagerStandard::NetDataShareHelperUtilsIface::UnregisterObserver(VPNEXT_MODE_URI, *callbackId);
            }
            {
                std::lock_guard<std::mutex> lock(waitMutex);
                observerFired.store(vpnExtModeAfter == "1");
            }
            waitCv.notify_all();
        });
    };

    *callbackId = NetManagerStandard::NetDataShareHelperUtilsIface::RegisterObserver(VPNEXT_MODE_URI, onChange);
    return WaitForObserverResult(callbackId, waitMutex, waitCv, observerFired);
}

// Check VPN authorization via DataShare. If not authorized, show dialog and wait for user decision.
// Uses PERM_RESULT_* constants defined above.
static int32_t ProcessPermissionRequests(const std::string &bundleName, const std::string &abilityName,
    std::string &selfAppName)
{
    std::string selfBundleName;
    NetManagerStandard::NetworkVpnClient::GetInstance()
        .GetSelfAppName(selfAppName, selfBundleName);

    if (bundleName != selfBundleName) {
        return PERM_RESULT_FAILED;
    }

    std::string vpnExtMode = "0";
    int32_t ret;
#ifdef SUPPORT_SYSVPN
    ret = CheckVpnPermission(bundleName, vpnExtMode);
#else
    ret = NetManagerStandard::NetDataShareHelperUtilsIface::Query(VPNEXT_MODE_URI, bundleName, vpnExtMode);
#endif // SUPPORT_SYSVPN
    if (ret != 0 || vpnExtMode != "1") {
        return WaitForVpnDialogAuthorization(bundleName, abilityName, selfAppName);
    }
    return PERM_RESULT_AUTHORIZED;
}

// ============================================================================
// Static data conversion helpers
// ============================================================================
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
    config->isLegacy_ = data.is_internal;
    config->isBlocking_ = data.is_blocking;
    for (auto &app : data.trusted_applications) {
        config->acceptedApplications_.push_back(std::string(app));
    }
    for (auto &app : data.blocked_applications) {
        config->refusedApplications_.push_back(std::string(app));
    }
    return config;
}

// ============================================================================
// Public API implementations
// ============================================================================

int32_t StartVpnExtensionAbility(const rust::String &bundleName,
    const rust::String &abilityName, int32_t &ret)
{
    std::string bn = std::string(bundleName);
    std::string an = std::string(abilityName);

    AAFwk::Want want;
    want.SetElementName(bn, an);

    // 1. Cache the want (for VpnMonitor / dialog use)
    AniVpnMonitor::GetInstance().CacheCurrentWant(want);

    // 2. Get self app name for permission checks
    std::string selfAppName;
    std::string selfBundleName;
    auto getAppNameRes = NetManagerStandard::NetworkVpnClient::GetInstance()
        .GetSelfAppName(selfAppName, selfBundleName);
    if (getAppNameRes != NetManagerStandard::NETMANAGER_EXT_SUCCESS) {
        ret = NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
        return ret;
    }

    // 3. Process permission requests (non-VPN-dialog apps)
    if (selfBundleName != VPN_DIALOG_BUNDLENAME || an.find(VPN_DIALOG_POSTFIX) == std::string::npos) {
        int32_t permResult = ProcessPermissionRequests(bn, an, selfAppName);
        if (permResult == PERM_RESULT_FAILED) {
            ret = NetManagerStandard::NETMANAGER_EXT_ERR_PERMISSION_DENIED;
            return ret;
        }
        if (permResult == PERM_RESULT_DENIED_AFTER_DIALOG) {
            ret = NetManagerStandard::NETMANAGER_EXT_ERR_PERMISSION_DENIED;
            return ret;
        }
        // PERM_RESULT_AUTHORIZED: already authorized, PERM_RESULT_AUTHORIZED_AFTER_DIALOG: authorized after dialog
    }

    // 4. Remove VPN_DIALOG_POSTFIX suffix from ability name
    auto elem = want.GetElement();
    elem.SetAbilityName(RemoveVpnDialogPostfix(an));
    want.SetElement(elem);

    // 5. EDM policy check
    if (OHOS::system::GetBoolParameter(EDM_VPN_DISABLE_KEY, false)) {
        ret = NetManagerStandard::NETMANAGER_EXT_ERR_PERMISSION_DENIED;
        return ret;
    }

    // 6. Call NetworkVpnClient
    ret = NetManagerStandard::NetworkVpnClient::GetInstance().StartVpnExtensionAbility(want);
    return ret;
}

int32_t StopVpnExtensionAbility(const rust::String &bundleName,
    const rust::String &abilityName, int32_t &ret)
{
    std::string bn = std::string(bundleName);
    std::string an = std::string(abilityName);

    AAFwk::Want want;
    want.SetElementName(bn, an);

    // 1. Verify bundleName matches self
    std::string selfAppName;
    std::string selfBundleName;
    NetManagerStandard::NetworkVpnClient::GetInstance()
        .GetSelfAppName(selfAppName, selfBundleName);

    if (bn != selfBundleName) {
        ret = NetManagerStandard::NETMANAGER_EXT_ERR_PERMISSION_DENIED;
        return ret;
    }

    // 2. Check VPN authorization via DataShare
    std::string vpnExtMode = "0";
    int32_t queryRet;
#ifdef SUPPORT_SYSVPN
    queryRet = CheckVpnPermission(bn, vpnExtMode);
#else
    queryRet = NetManagerStandard::NetDataShareHelperUtilsIface::Query(VPNEXT_MODE_URI, bn, vpnExtMode);
#endif // SUPPORT_SYSVPN
    if (queryRet != 0 || vpnExtMode != "1") {
        ret = NetManagerStandard::NETMANAGER_EXT_ERR_PERMISSION_DENIED;
        return ret;
    }

    // 3. Call NetworkVpnClient
    ret = NetManagerStandard::NetworkVpnClient::GetInstance().StopVpnExtensionAbility(want);
    return ret;
}

int32_t SetAlwaysOnVpnEnabled(bool enable, const rust::String &bundleName)
{
    std::string bn = std::string(bundleName);
    std::string value = enable ? "1" : "0";
    int32_t ret = NetManagerStandard::NetDataShareHelperUtilsIface::Update(VPNEXT_ALWAYSON_URI, bn, value);
    return (ret == 0) ? NetManagerStandard::NETMANAGER_EXT_SUCCESS
                      : NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
}

bool IsAlwaysOnVpnEnabled(const rust::String &bundleName, int32_t &ret)
{
    std::string bn = std::string(bundleName);
    std::string value = "0";
    int32_t queryRet = NetManagerStandard::NetDataShareHelperUtilsIface::Query(VPNEXT_ALWAYSON_URI, bn, value);
    ret = (queryRet == 0) ? NetManagerStandard::NETMANAGER_EXT_SUCCESS
                          : NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
    return (value == "1");
}

bool UpdateVpnAuthorizedState(const rust::String &bundleName)
{
    std::string bn = std::string(bundleName);
    if (!IsValidBundleName(bn)) {
        return false;
    }
    bool authorized = true;
    if (bn.find(VPN_DIALOG_POSTFIX) != std::string::npos) {
        authorized = false;
        bn = RemoveVpnDialogPostfix(bn);
    }
    std::string vpnExtMode = std::to_string(authorized);

    int32_t ret;
#ifdef SUPPORT_SYSVPN
    int32_t userId = AppExecFwk::Constants::UNSPECIFIED_USERID;
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(uid, userId) != ERR_OK) {
        return false;
    }
    std::string key = bn + "_" + std::to_string(userId);
    ret = NetManagerStandard::NetDataShareHelperUtilsIface::Update(VPNEXT_MODE_URI, key, vpnExtMode);
#else
    ret = NetManagerStandard::NetDataShareHelperUtilsIface::Update(VPNEXT_MODE_URI, bn, vpnExtMode);
#endif // SUPPORT_SYSVPN

    return (ret == 0);
}

bool CreateVpnConnection(int32_t &ret)
{
    ret = NetManagerStandard::NetworkVpnClient::GetInstance().CreateVpnConnection(true);
    return ret == NetManagerStandard::NETMANAGER_EXT_SUCCESS;
}

int32_t Create(const VpnConfigData &config, int32_t &fd)
{
    auto vpnConfig = ConvertToVpnConfig(config);
    if (vpnConfig == nullptr) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
    }
    return NetManagerStandard::NetworkVpnClient::GetInstance().SetUpVpn(vpnConfig, fd, true);
}

int32_t Protect(int32_t socketFd)
{
    return NetManagerStandard::NetworkVpnClient::GetInstance().Protect(socketFd, true);
}

int32_t Destroy()
{
    return NetManagerStandard::NetworkVpnClient::GetInstance().DestroyVpn(true);
}

int32_t DestroyVpn(const rust::String &vpnId)
{
#ifdef SUPPORT_SYSVPN
    return NetManagerStandard::NetworkVpnClient::GetInstance().DestroyVpn(std::string(vpnId));
#else
    return NetManagerStandard::NetworkVpnClient::GetInstance().DestroyVpn(true);
#endif
}

int32_t ProtectProcessNet()
{
    int32_t result = NetManagerStandard::NetConnClient::GetInstance().ProtectProcessNet();
    if (result != NetManagerStandard::NETMANAGER_EXT_SUCCESS) {
    }
    return result;
}

rust::String GenerateVpnId(int32_t &ret)
{
#ifdef SUPPORT_SYSVPN
    std::string vpnId = UuidGenerator::Generate();
    ret = NetManagerStandard::NETMANAGER_EXT_SUCCESS;
    return rust::String(vpnId);
#else
    ret = NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
    return rust::String("");
#endif
}

int32_t VpnExtObserverRegister()
{
    std::lock_guard<std::mutex> lock(g_vpnExtObserverMutex);
    if (g_vpnExtObserver != nullptr) {
        return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
    }
    g_vpnExtObserver = sptr<VpnExtObserverCallback>::MakeSptr();
    if (g_vpnExtObserver == nullptr) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
    }
    int32_t ret = NetManagerStandard::NetworkVpnClient::GetInstance()
        .RegisterVpnEvent(g_vpnExtObserver);
    if (ret != 0) {
        g_vpnExtObserver = nullptr;
    }
    return ret;
}

int32_t VpnExtObserverUnRegister()
{
    std::lock_guard<std::mutex> lock(g_vpnExtObserverMutex);
    if (g_vpnExtObserver == nullptr) {
        return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
    }
    int32_t ret = NetManagerStandard::NetworkVpnClient::GetInstance()
        .UnregisterVpnEvent(g_vpnExtObserver);
    g_vpnExtObserver = nullptr;
    return ret;
}

rust::String GetErrorCodeAndMessage(int32_t &errorCode)
{
    NetManagerStandard::NetBaseErrorCodeConvertor convertor;
    return rust::String(convertor.ConvertErrorCode(errorCode));
}

} // namespace NetManagerAni
} // namespace OHOS
