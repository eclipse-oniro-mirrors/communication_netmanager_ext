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
#include "ani_base_context.h"
#include "ani_common_want.h"
#include "extension_ability_info.h"
#include "net_conn_client.h"
#include "net_datashare_utils_iface.h"
#include "net_manager_constants.h"
#include "netmanager_ext_log.h"
#include "errorcode_convertor.h"
#include "parameters.h"
#include "want.h"
#include "want_params.h"

#ifdef SUPPORT_SYSVPN
#include "os_account_manager.h"
#include "ipc_skeleton.h"
#include "uuid.h"
#endif // SUPPORT_SYSVPN

namespace OHOS {
namespace NetManagerAni {

namespace {
constexpr const char *VPNEXT_MODE_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=vpnext_mode";
constexpr const char *ALWAYS_ON_VPN_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=vpnalwayson_mode";
constexpr const char *KEY_ALWAYS_ON_VPN = "settings.alwayson.vpn";
constexpr const char *VPN_DIALOG_POSTFIX = "**vpndialog**";
constexpr const char *VPN_DIALOG_BUNDLENAME = "com.ohos.vpndialog";
constexpr const char *EDM_VPN_DISABLE_KEY = "persist.edm.vpn_disable";
constexpr int32_t MAX_BUNDLE_NAME_LENGTH = 256;

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
constexpr int32_t PERM_RESULT_AUTHORIZED = 0;
constexpr int32_t VPN_DIALOG_TIMEOUT_SEC = 30;
} // anonymous namespace

class VpnAbilityConn : public AAFwk::AbilityConnectionStub {
    void OnAbilityConnectDone(const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int32_t resultCode) override
    {
        NETMANAGER_EXT_LOGI("connect done");
    }
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode) override
    {
        NETMANAGER_EXT_LOGI("disconnect done");
    }
};

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
        NETMANAGER_EXT_LOGI("CacheCurrentWant: cached want %{public}s", want.GetElement().GetBundleName().c_str());
    }

    AAFwk::Want GetCachedWant()
    {
        std::lock_guard<std::mutex> lock(wantMutex_);
        if (!cachedWant_.GetElement().GetBundleName().empty()) {
            NETMANAGER_EXT_LOGI("GetCachedWant: get cachedwant success");
            return cachedWant_;
        }
        return AAFwk::Want();
    }

    bool ShowVpnDialog(const std::string &bundleName, const std::string &abilityName,
                       const std::string &appName)
    {
        auto abmc = AAFwk::AbilityManagerClient::GetInstance();
        if (abmc == nullptr) {
            NETMANAGER_EXT_LOGE("GetInstance failed");
            return false;
        }

        AAFwk::Want cachedwant = GetCachedWant();
        AAFwk::WantParams cachedParams = cachedwant.GetParams();
        AAFwk::Want want;
        AAFwk::WantParams wantParams = want.GetParams();
        wantParams.SetParam("myParams", AAFwk::WantParamWrapper::Box(cachedParams));
        want.SetParams(wantParams);
        want.SetElementName(VPN_DIALOG_BUNDLENAME, "VpnServiceExtAbility");
        want.SetParam("bundleName", bundleName);
        want.SetParam("abilityName", abilityName + VPN_DIALOG_POSTFIX);
        want.SetParam("appName", appName);

        sptr<VpnAbilityConn> vpnAbilityConn = sptr<VpnAbilityConn>::MakeSptr();
        auto ret = abmc->ConnectAbility(want, vpnAbilityConn, -1);
        if (ret != 0) {
            NETMANAGER_EXT_LOGE("connectAbility failed %{public}d", ret);
            return false;
        }
        NETMANAGER_EXT_LOGI("click done");
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

static std::string Replace(std::string s)
{
    std::string tmp = VPN_DIALOG_POSTFIX;
    auto pos = s.find(tmp);
    if (pos == std::string::npos) {
        return s;
    }
    s.replace(pos, tmp.length(), "");
    return s;
}

#ifdef SUPPORT_SYSVPN
static int32_t CheckVpnPermission(const std::string &bundleName, std::string &vpnExtMode)
{
    int32_t userId = AppExecFwk::Constants::UNSPECIFIED_USERID;
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(uid, userId) != ERR_OK) {
        NETMANAGER_EXT_LOGE("checkVpnPermission::GetOsAccountLocalIdFromUid error, uid: %{public}d.", uid);
        return -1;
    }
    NETMANAGER_EXT_LOGI("checkVpnPermission uid: %{public}d, userid: %{public}d", uid, userId);

    std::string userKey = bundleName + "_" + std::to_string(userId);
    int32_t ret = NetManagerStandard::NetDataShareHelperUtilsIface::Query(VPNEXT_MODE_URI, userKey, vpnExtMode);
    if (ret == 0 && vpnExtMode == "1") {
        return 0;
    }

    ret = NetManagerStandard::NetDataShareHelperUtilsIface::Query(VPNEXT_MODE_URI, bundleName, vpnExtMode);
    if (ret != 0 || vpnExtMode != "1") {
        NETMANAGER_EXT_LOGE("checkVpnPermission::dataShareHelperUtils Query error, err = %{public}d", ret);
        return -1;
    }

    return 0;
}
#endif // SUPPORT_SYSVPN

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
    std::mutex &waitMutex, std::condition_variable &waitCv,
    std::atomic<bool> &hasFired, std::atomic<bool> &isAuthorized)
{
    std::unique_lock<std::mutex> lock(waitMutex);
    bool waitResult = waitCv.wait_for(lock, std::chrono::seconds(VPN_DIALOG_TIMEOUT_SEC), [&hasFired]() {
        return hasFired.load();
    });
    if (!waitResult) {
        if (callbackId) {
            NetManagerStandard::NetDataShareHelperUtilsIface::UnregisterObserver(VPNEXT_MODE_URI, *callbackId);
        }
        return PERM_RESULT_FAILED;
    }
    return isAuthorized.load() ? PERM_RESULT_AUTHORIZED_AFTER_DIALOG : PERM_RESULT_DENIED_AFTER_DIALOG;
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
        NETMANAGER_EXT_LOGE("WaitForVpnDialogAuthorization::BuildVpnDialogKey failed, bundleName: %{public}s",
                            bundleName.c_str());
        return PERM_RESULT_FAILED;
    }
    std::mutex waitMutex;
    std::condition_variable waitCv;
    std::atomic<bool> hasFired{false};
    std::atomic<bool> isAuthorized{false};
    auto once = std::make_shared<std::once_flag>();
    auto callbackId = std::make_shared<int32_t>();

    auto onChange = [once, &waitMutex, &waitCv, &hasFired, &isAuthorized, callbackId, key]() {
        std::call_once(*once, [&waitMutex, &waitCv, &hasFired, &isAuthorized, callbackId, key]() {
            std::string vpnExtModeAfter = "0";
            int32_t ret = NetManagerStandard::NetDataShareHelperUtilsIface::Query(
                VPNEXT_MODE_URI, key, vpnExtModeAfter);
            NETMANAGER_EXT_LOGI("query vpn state after dialog: %{public}d %{public}s", ret, vpnExtModeAfter.c_str());
            if (callbackId) {
                NetManagerStandard::NetDataShareHelperUtilsIface::UnregisterObserver(VPNEXT_MODE_URI, *callbackId);
            }
            {
                std::lock_guard<std::mutex> lock(waitMutex);
                isAuthorized.store(vpnExtModeAfter == "1");
                hasFired.store(true);
            }
            waitCv.notify_all();
        });
    };

    *callbackId = NetManagerStandard::NetDataShareHelperUtilsIface::RegisterObserver(VPNEXT_MODE_URI, onChange);
    return WaitForObserverResult(callbackId, waitMutex, waitCv, hasFired, isAuthorized);
}

static int32_t ProcessPermissionRequests(const std::string &bundleName, const std::string &abilityName,
    std::string &selfAppName)
{
    std::string selfBundleName;
    auto getAppNameRes = NetManagerStandard::NetworkVpnClient::GetInstance()
        .GetSelfAppName(selfAppName, selfBundleName);
    NETMANAGER_EXT_LOGI("StartVpnExtensionAbility SelfAppName = %{public}s %{public}d",
                        selfAppName.c_str(), getAppNameRes);

    if (bundleName != selfBundleName) {
        NETMANAGER_EXT_LOGE("Not allowed to start other bundleName vpn!");
        return PERM_RESULT_FAILED;
    }

    bool vpnDialogSelect = false;
    std::string vpnExtMode = std::to_string(vpnDialogSelect);
    int32_t ret;
#ifdef SUPPORT_SYSVPN
    ret = CheckVpnPermission(bundleName, vpnExtMode);
#else
    ret = NetManagerStandard::NetDataShareHelperUtilsIface::Query(VPNEXT_MODE_URI, bundleName, vpnExtMode);
#endif // SUPPORT_SYSVPN
    if (ret != 0 || vpnExtMode != "1") {
        NETMANAGER_EXT_LOGE("dataShareHelperUtils Query error, err = %{public}d", ret);
        return WaitForVpnDialogAuthorization(bundleName, abilityName, selfAppName);
    }
    return PERM_RESULT_AUTHORIZED;
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

int32_t CreateVpnConnection(AniEnv **env, AniObject *contextObj, int64_t &ptr)
{
    if (env == nullptr || *env == nullptr || contextObj == nullptr) {
        NETMANAGER_EXT_LOGE("CreateVpnConnection function parameter error");
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }

    auto aniEnv = reinterpret_cast<ani_env *>(*env);
    auto obj = *reinterpret_cast<ani_object *>(contextObj);
    auto context = AbilityRuntime::GetStageModeContext(aniEnv, obj);
    if (context == nullptr) {
        NETMANAGER_EXT_LOGE("context is nullptr");
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }

    auto errCode = NetManagerStandard::NetworkVpnClient::GetInstance().CreateVpnConnection(true);
    if (errCode == NetManagerStandard::NETMANAGER_EXT_SUCCESS) {
        ptr = reinterpret_cast<int64_t>(&NetManagerStandard::NetworkVpnClient::GetInstance());
    }

    return errCode;
}

int32_t DestroyVpnConnection(int64_t ptr)
{
    (void)ptr;
    return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
}

int32_t CreateVpnObserver(int64_t &ptr)
{
    static std::atomic<int64_t> gVpnObserverId = 0;
    ptr = ++gVpnObserverId;
    return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
}

int32_t DestroyVpnObserver(const int64_t ptr)
{
    (void)ptr;
    return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
}

int32_t StartVpnExtensionAbility(AniEnv **env, AniObject *wantObj)
{
    if (env == nullptr || *env == nullptr || wantObj == nullptr) {
        NETMANAGER_EXT_LOGE("funciton prameter error");
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }

    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(reinterpret_cast<ani_env *>(*env), *reinterpret_cast<ani_object *>(wantObj), want)) {
        NETMANAGER_EXT_LOGE("Failed to parse want");
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }

    AniVpnMonitor::GetInstance().CacheCurrentWant(want);
    std::string bundleName = want.GetElement().GetBundleName();
    std::string abilityName = want.GetElement().GetAbilityName();
    std::string selfAppName;
    std::string selfBundleName;
    NetManagerStandard::NetworkVpnClient::GetInstance().GetSelfAppName(selfAppName, selfBundleName);
    if (selfBundleName != VPN_DIALOG_BUNDLENAME || abilityName.find(VPN_DIALOG_POSTFIX) == std::string::npos) {
        int32_t permResult = ProcessPermissionRequests(bundleName, abilityName, selfAppName);
        if (permResult == PERM_RESULT_AUTHORIZED_AFTER_DIALOG) {
            execute_vpn_ext_authorization_result(true);
            return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
        } else if (permResult == PERM_RESULT_DENIED_AFTER_DIALOG) {
            execute_vpn_ext_authorization_result(false);
            return NetManagerStandard::NETMANAGER_EXT_ERR_PERMISSION_DENIED;
        } else if (permResult == PERM_RESULT_FAILED) {
            return NetManagerStandard::NETMANAGER_EXT_ERR_PERMISSION_DENIED;
        }
    }

    auto elem = want.GetElement();
    elem.SetAbilityName(Replace(abilityName));
    want.SetElement(elem);
    if (OHOS::system::GetBoolParameter(EDM_VPN_DISABLE_KEY, false)) {
        NETMANAGER_EXT_LOGE("persist.edm.vpn_disable disallowed setting up vpn");
        return NetManagerStandard::NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }

    auto err = NetManagerStandard::NetworkVpnClient::GetInstance().StartVpnExtensionAbility(want);
    NETMANAGER_EXT_LOGI("execute StartVpnExtensionAbility result: %{public}d", err);
    return err;
}

int32_t StopVpnExtensionAbility(AniEnv **env, AniObject *wantObj)
{
    if (env == nullptr || *env == nullptr || wantObj == nullptr) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }

    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(reinterpret_cast<ani_env *>(*env), *reinterpret_cast<ani_object *>(wantObj), want)) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }

    std::string bundleName = want.GetElement().GetBundleName();
    std::string selfAppName;
    std::string selfBundleName;
    NetManagerStandard::NetworkVpnClient::GetInstance().GetSelfAppName(selfAppName, selfBundleName);
    if (bundleName != selfBundleName) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }

    bool vpnDialogSelect = false;
    std::string vpnExtMode = std::to_string(vpnDialogSelect);
    int32_t ret = 0;
#ifdef SUPPORT_SYSVPN
    ret = CheckVpnPermission(bundleName, vpnExtMode);
#else
    ret = NetManagerStandard::NetDataShareHelperUtilsIface::Query(VPNEXT_MODE_URI, bundleName, vpnExtMode);
#endif // SUPPORT_SYSVPN
    if (ret != 0 || vpnExtMode != "1") {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }

    return NetManagerStandard::NetworkVpnClient::GetInstance().StopVpnExtensionAbility(want);
}

int32_t SetAlwaysOnVpnEnabled(bool enable, const rust::String &bundleName)
{
    std::string bn = std::string(bundleName);
    if (!IsValidBundleName(bn)) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    auto ret = NetManagerStandard::NetDataShareHelperUtilsIface::Update(ALWAYS_ON_VPN_URI,
        KEY_ALWAYS_ON_VPN, (enable ? bn : ""));
    if (ret != NetManagerStandard::NETMANAGER_EXT_SUCCESS) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }

    return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
}

int32_t IsAlwaysOnVpnEnabled(const rust::String &bundleName, bool &isEnabled)
{
    std::string bn = std::string(bundleName);
    if (!IsValidBundleName(bn)) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    bool vpnDialogSelect = false;
    std::string vpnExtMode = std::to_string(vpnDialogSelect);
    int32_t ret = 0;
#ifdef SUPPORT_SYSVPN
    ret = CheckVpnPermission(bn, vpnExtMode);
#else
    ret = NetManagerStandard::NetDataShareHelperUtilsIface::Query(VPNEXT_MODE_URI, bn, vpnExtMode);
#endif // SUPPORT_SYSVPN
    if (ret != 0 || vpnExtMode != "1") {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    std::string value = "";
    ret = NetManagerStandard::NetDataShareHelperUtilsIface::Query(ALWAYS_ON_VPN_URI, KEY_ALWAYS_ON_VPN, value);
    if (ret != NetManagerStandard::NETMANAGER_EXT_SUCCESS) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }

    isEnabled = (value == bn);
    return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
}

int32_t UpdateVpnAuthorizedState(const rust::String &bundleName, bool &state)
{
    std::string bn = std::string(bundleName);
    if (!IsValidBundleName(bn)) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    bool vpnDialogSelect = true;
    if (bn.find(VPN_DIALOG_POSTFIX) != std::string::npos) {
        vpnDialogSelect = false;
        bn = Replace(bn);
    }
    std::string vpnExtMode = std::to_string(vpnDialogSelect);

    int32_t ret = 0;
#ifdef SUPPORT_SYSVPN
    int32_t userId = AppExecFwk::Constants::UNSPECIFIED_USERID;
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(uid, userId) != ERR_OK) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
    }
    std::string key = bn + "_" + std::to_string(userId);
    ret = NetManagerStandard::NetDataShareHelperUtilsIface::Update(VPNEXT_MODE_URI, key, vpnExtMode);
#else
    ret = NetManagerStandard::NetDataShareHelperUtilsIface::Update(VPNEXT_MODE_URI, bn, vpnExtMode);
#endif // SUPPORT_SYSVPN
    if (ret == NetManagerStandard::NETMANAGER_EXT_SUCCESS) {
        state = true;
    }
    return ret;
}

int32_t Create(const int64_t ptr, const VpnConfigData &config, int32_t &fd)
{
    if (ptr == 0) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    auto vpnConfig = ConvertToVpnConfig(config);
    if (vpnConfig == nullptr) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    return NetManagerStandard::NetworkVpnClient::GetInstance().SetUpVpn(vpnConfig, fd, true);
}

int32_t Protect(const int64_t ptr, const int32_t socketFd)
{
    if (ptr == 0) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    return NetManagerStandard::NetworkVpnClient::GetInstance().Protect(socketFd, true);
}

int32_t DestroyVpn(const int64_t ptr, const rust::String &vpnId)
{
    if (ptr == 0) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    int32_t result = NetManagerStandard::NETMANAGER_EXT_SUCCESS;
#ifdef SUPPORT_SYSVPN
    if (!vpnId.empty()) {
        result = NetManagerStandard::NetworkVpnClient::GetInstance().DestroyVpn(std::string(vpnId));
    } else {
        result = NetManagerStandard::NetworkVpnClient::GetInstance().DestroyVpn(true);
    }
#else
    result = NetManagerStandard::NetworkVpnClient::GetInstance().DestroyVpn(true);
#endif // SUPPORT_SYSVPN
    return result;
}

int32_t ProtectProcessNet(const int64_t ptr)
{
    if (ptr == 0) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    return NetManagerStandard::NetConnClient::GetInstance().ProtectProcessNet();
}

int32_t GenerateVpnId(const int64_t ptr, rust::String &vpnId)
{
    if (ptr == 0) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
#ifdef SUPPORT_SYSVPN
    vpnId = rust::String(NetManagerStandard::UUID::RandomUUID().ToString());
    return NetManagerStandard::NETMANAGER_EXT_SUCCESS;
#else
    return NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
#endif
}

rust::String GetErrorCodeAndMessage(int32_t &errorCode)
{
    NetManagerStandard::NetBaseErrorCodeConvertor convertor;
    return rust::String(convertor.ConvertErrorCode(errorCode));
}

} // namespace NetManagerAni
} // namespace OHOS
