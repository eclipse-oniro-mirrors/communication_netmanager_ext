/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "eap_ani.h"

#include <utility>

#include "ethernet_client.h"
#include "eth_eap_profile.h"
#include "eap_data.h"
#include "net_eap_callback_stub.h"
#include "netmanager_ext_log.h"
#include "netmanager_base_permission.h"
#include "net_manager_ext_constants.h"
#include "securec.h"
#include <mutex>
#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
#include "eth_eap_config.h"
#include "eth_eap_state_info.h"
#endif

#include "wrapper.rs.h"

namespace OHOS {
namespace NetManagerAni {

namespace {
sptr<NetEapPostBackCallbackAni> g_NetEapPostBackCallbackAni =
    sptr<NetEapPostBackCallbackAni>(new (std::nothrow) NetEapPostBackCallbackAni());

constexpr int32_t WIFI_DEVICE_SA_ID = 1120;
constexpr int32_t COMM_ETHERNET_MANAGER_SYS_ABILITY_ID = 1157;
#ifdef NET_EXTENSIBLE_AUTHENTICATION
static std::mutex g_mutex;
static constexpr const int MAX_EAP_DATA_LENGTH = 4096;
constexpr int32_t EAP_CODE_SUCCESS = 3;
constexpr int32_t EAP_CODE_FAILURE = 4;
constexpr int32_t OFFSET_EAPCODE = 8;
#endif
#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
sptr<EthEapStateCallbackAni> g_ethEapStateCallbackAni;
[[maybe_unused]] bool g_isEthEapStateCallbackRegistered = false;
std::mutex g_ethEapStateCallbackMutex;
#endif

// CXX owns these buffers after transfer from Rust. Wipe even on validation
// failures and feature-disabled paths before their storage is released.
class ScopedEthEapProfileWipe final {
public:
    explicit ScopedEthEapProfileWipe(EthEapProfileAni &profile) : profile_(profile) {}
    ~ScopedEthEapProfileWipe()
    {
        if (!profile_.password.empty()) {
            (void)memset_s(profile_.password.begin(), profile_.password.size(),
                0, profile_.password.size());
        }
        if (!profile_.cert_password.empty()) {
            (void)memset_s(profile_.cert_password.begin(), profile_.cert_password.size(),
                0, profile_.cert_password.size());
        }
        if (!profile_.cert_entry.empty()) {
            (void)memset_s(profile_.cert_entry.data(), profile_.cert_entry.size(), 0, profile_.cert_entry.size());
        }
    }

private:
    EthEapProfileAni &profile_;
};

#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
bool ConvertEthEapProfile(const EthEapProfileAni &config, NetManagerStandard::EthEapProfile &profile)
{
    static_assert(NetManagerStandard::EapMethod::EAP_UNAUTH_TLS == 8,
        "EAP_UNAUTH_TLS must be the maximum EapMethod value");
    static_assert(NetManagerStandard::Phase2Method::PHASE2_AKA_PRIME == 7,
        "PHASE2_AKA_PRIME must be the maximum Phase2Method value");
    if (config.eap_method < 0 || config.eap_method > NetManagerStandard::EapMethod::EAP_UNAUTH_TLS ||
        config.phase2_method < 0 || config.phase2_method > NetManagerStandard::Phase2Method::PHASE2_AKA_PRIME) {
        return false;
    }
    profile.eapMethod = static_cast<NetManagerStandard::EapMethod>(config.eap_method);
    profile.phase2Method = static_cast<NetManagerStandard::Phase2Method>(config.phase2_method);
    profile.identity = std::string(config.identity);
    profile.anonymousIdentity = std::string(config.anonymous_identity);
    profile.password.append(config.password.data(), config.password.size());
    profile.caCertAliases = std::string(config.ca_cert_aliases);
    profile.caPath = std::string(config.ca_path);
    profile.clientCertAliases = std::string(config.client_cert_aliases);
    profile.certEntry = std::vector<uint8_t>(config.cert_entry.begin(), config.cert_entry.end());
    profile.certPassword.append(config.cert_password.data(), config.cert_password.size());
    profile.altSubjectMatch = std::string(config.alt_subject_match);
    profile.domainSuffixMatch = std::string(config.domain_suffix_match);
    profile.realm = std::string(config.realm);
    profile.plmn = std::string(config.plmn);
    profile.eapSubId = config.eap_sub_id;
    return true;
}

void ConvertEthEapProfileToAni(const NetManagerStandard::EthEapProfile &profile, EthEapProfileAni &config)
{
    config.eap_method = static_cast<int32_t>(profile.eapMethod);
    config.phase2_method = static_cast<int32_t>(profile.phase2Method);
    config.identity = rust::String(profile.identity);
    config.anonymous_identity = rust::String(profile.anonymousIdentity);
    config.ca_cert_aliases = rust::String(profile.caCertAliases);
    config.ca_path = rust::String(profile.caPath);
    config.client_cert_aliases = rust::String(profile.clientCertAliases);
    config.alt_subject_match = rust::String(profile.altSubjectMatch);
    config.domain_suffix_match = rust::String(profile.domainSuffixMatch);
    config.realm = rust::String(profile.realm);
    config.plmn = rust::String(profile.plmn);
    config.eap_sub_id = profile.eapSubId;
    // Passwords and certificate contents are never returned over the ANI bridge.
}

EthEapStateInfoAni ConvertEthEapStateInfoToAni(const NetManagerStandard::EthEapStateInfo &stateInfo)
{
    EthEapStateInfoAni info{};
    info.state = static_cast<int32_t>(stateInfo.state);
    info.retry_count = stateInfo.retryCount;
    info.message = rust::String(stateInfo.message);
    return info;
}
#endif
} // namespace

bool IsEapExtensibleAuthenticationEnabled()
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    return true;
#else
    return false;
#endif
}

#ifdef NET_EXTENSIBLE_AUTHENTICATION
static uint32_t ComposeParam(int32_t eapCode, int32_t eapType)
{
    uint32_t composeParam = static_cast<uint32_t>(eapCode) << OFFSET_EAPCODE;
    if (eapCode != EAP_CODE_SUCCESS && eapCode != EAP_CODE_FAILURE) {
        composeParam |= static_cast<uint32_t>(eapType);
    }
    return composeParam;
}
#endif

int32_t ValidateEapParams(int32_t netType, int32_t eapCode, int32_t eapType)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    if (!NetManagerStandard::NetManagerPermission::CheckPermission(
        NetManagerStandard::Permission::MANAGE_ENTERPRISE_WIFI_CONNECTION)) {
        return NetManagerStandard::EAP_ERRCODE_PERMISSION_DENIED;
    }
    if (netType < static_cast<int32_t>(NetManagerStandard::NetType::WLAN0) ||
        netType >= static_cast<int32_t>(NetManagerStandard::NetType::INVALID)) {
        return NetManagerStandard::EAP_ERRCODE_INVALID_NET_TYPE;
    }
    if (eapCode < EAP_CODE_MIN || eapCode > EAP_CODE_MAX) {
        return NetManagerStandard::EAP_ERRCODE_INVALID_EAP_CODE;
    }
    if (eapType < EAP_TYPE_MIN || eapType > EAP_TYPE_MAX) {
        return NetManagerStandard::EAP_ERRCODE_INVALID_EAP_TYPE;
    }
    return NetManagerStandard::EAP_ERRCODE_SUCCESS;
#else
    return NetManagerStandard::EAP_ERRCODE_SUCCESS;
#endif
}

int32_t RegCustomEapHandler(int32_t triggerMode, int32_t netType,
    const rust::Vec<EapCodeTypeAni> &regItems)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    if (netType < static_cast<int32_t>(NetManagerStandard::NetType::WLAN0) ||
        netType >= static_cast<int32_t>(NetManagerStandard::NetType::INVALID)) {
        return NetManagerStandard::EAP_ERRCODE_INVALID_NET_TYPE;
    }
    std::string regCmd = std::to_string(netType) + ":" + std::to_string(regItems.size());
    for (const auto &item : regItems) {
        uint32_t composeParam = ComposeParam(item.eap_code, item.eap_type);
        regCmd += ":" + std::to_string(composeParam);
    }
    NETMANAGER_EXT_LOGI("RegCustomEapHandler enter, triggerMode:%{public}d, netType:%{public}d, regCmd:%{public}s",
        static_cast<int32_t>(triggerMode), netType, regCmd.c_str());

    if (g_NetEapPostBackCallbackAni == nullptr) {
        return NetManagerStandard::EAP_ERRCODE_INTERNAL_ERROR;
    }
    auto type = static_cast<NetManagerStandard::NetType>(netType);
    return DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->RegCustomEapHandler(
        type, regCmd, g_NetEapPostBackCallbackAni);
#else
    return NetManagerStandard::EAP_ERRCODE_SUCCESS;
#endif
}

int32_t ReplyCustomEapData(int32_t result, int32_t msgId, int32_t bufferLen,
                           const rust::Vec<uint8_t> &eapBuffer)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    if (result < static_cast<int32_t>(NetManagerStandard::CustomResult::RESULT_FAIL) ||
        result > static_cast<int32_t>(NetManagerStandard::CustomResult::RESULT_FINISH)) {
        return NetManagerStandard::EAP_ERRCODE_INVALID_RESULT;
    }

    std::unique_lock<std::mutex> lock(g_mutex);
    sptr<NetManagerStandard::EapData> eapData = sptr<NetManagerStandard::EapData>::MakeSptr();
    eapData->msgId = msgId;
    eapData->bufferLen = bufferLen;
    eapData->eapBuffer = std::vector<uint8_t>(eapBuffer.begin(), eapBuffer.end());
    if (eapData->bufferLen > MAX_EAP_DATA_LENGTH) {
        return NetManagerStandard::EAP_ERRCODE_INVALID_SIZE_OF_EAPDATA;
    }
    if (eapData->bufferLen != static_cast<int32_t>(eapData->eapBuffer.size())) {
        return NetManagerStandard::EAP_ERRCODE_INVALID_SIZE_OF_EAPDATA;
    }
    return DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->ReplyCustomEapData(result, eapData);
#else
    return NetManagerStandard::EAP_ERRCODE_SUCCESS;
#endif
}

int32_t StartEthEap(int32_t netId, EthEapProfileAni profileAni)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    auto eapMethod = static_cast<NetManagerStandard::EapMethod>(profileAni.eap_method);
    if (eapMethod < NetManagerStandard::EapMethod::EAP_NONE ||
        eapMethod > NetManagerStandard::EapMethod::EAP_UNAUTH_TLS) {
        return NetManagerStandard::EAP_ERRCODE_INVALID_PROFILE;
    }
    auto phase2Method = static_cast<NetManagerStandard::Phase2Method>(profileAni.phase2_method);
    if (phase2Method < NetManagerStandard::Phase2Method::PHASE2_NONE ||
        phase2Method > NetManagerStandard::Phase2Method::PHASE2_AKA_PRIME) {
        return NetManagerStandard::EAP_ERRCODE_INVALID_PROFILE;
    }
    NetManagerStandard::EthEapProfile profile;
    profile.eapMethod = eapMethod;
    profile.phase2Method = phase2Method;
    profile.identity = std::string(profileAni.identity);
    profile.anonymousIdentity = std::string(profileAni.anonymous_identity);
    profile.password.assign(std::string(profileAni.password));
    profile.caCertAliases = std::string(profileAni.ca_cert_aliases);
    profile.caPath = std::string(profileAni.ca_path);
    profile.clientCertAliases = std::string(profileAni.client_cert_aliases);
    profile.certEntry = std::vector<uint8_t>(profileAni.cert_entry.begin(), profileAni.cert_entry.end());
    profile.certPassword.assign(std::string(profileAni.cert_password));
    profile.altSubjectMatch = std::string(profileAni.alt_subject_match);
    profile.domainSuffixMatch = std::string(profileAni.domain_suffix_match);
    profile.realm = std::string(profileAni.realm);
    profile.plmn = std::string(profileAni.plmn);
    profile.eapSubId = profileAni.eap_sub_id;
    return DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->StartEthEap(netId, profile);
#else
    return NetManagerStandard::EAP_ERRCODE_SUCCESS;
#endif
}

int32_t LogOffEthEap(int32_t netId)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    return DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->LogOffEthEap(netId);
#else
    return NetManagerStandard::EAP_ERRCODE_SUCCESS;
#endif
}

void NetManagerAniAbilityStatusChange::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    switch (systemAbilityId) {
        case WIFI_DEVICE_SA_ID:
        case COMM_ETHERNET_MANAGER_SYS_ABILITY_ID:
            on_sa_status_change(static_cast<int32_t>(NetManagerStandard::NetType::WLAN0));
            break;
        default:
            return;
    }
}

void NetManagerAniAbilityStatusChange::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    NETMANAGER_EXT_LOGI("OnRemoveSystemAbility enter, systemAbilityId=%{public}d", systemAbilityId);
}

void InitEapAniSaListener()
{
    auto samgrProxy = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        return;
    }
    sptr<NetManagerAniAbilityStatusChange> listener = sptr<NetManagerAniAbilityStatusChange>::MakeSptr();
    samgrProxy->SubscribeSystemAbility(WIFI_DEVICE_SA_ID, listener);
    samgrProxy->SubscribeSystemAbility(COMM_ETHERNET_MANAGER_SYS_ABILITY_ID, listener);
}

int32_t SetEthEapConfig(EthEapConfigAni config)
{
    ScopedEthEapProfileWipe wipe(config.profile);
#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
    NetManagerStandard::EthEapConfig nativeConfig;
    if (!ConvertEthEapProfile(config.profile, nativeConfig.profile)) {
        return -1;
    }
    nativeConfig.enabled = config.enabled;
    nativeConfig.autoAuth = config.auto_auth;
    nativeConfig.locked = config.locked;
    int32_t ret = DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->SetEthEapConfig(nativeConfig);
    nativeConfig.profile.password.clear();
    nativeConfig.profile.certPassword.clear();
    if (!nativeConfig.profile.certEntry.empty()) {
        (void)memset_s(nativeConfig.profile.certEntry.data(), nativeConfig.profile.certEntry.size(),
            0, nativeConfig.profile.certEntry.size());
        nativeConfig.profile.certEntry.clear();
    }
    return ret;
#else
    return NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
#endif
}

int32_t GetEthEapConfig(EthEapConfigAni &config)
{
#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
    NetManagerStandard::EthEapConfig nativeConfig;
    int32_t ret = DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->GetEthEapConfig(nativeConfig);
    if (ret != 0) {
        return ret;
    }
    EthEapConfigAni result{};
    ConvertEthEapProfileToAni(nativeConfig.profile, result.profile);
    result.enabled = nativeConfig.enabled;
    result.auto_auth = nativeConfig.autoAuth;
    result.locked = nativeConfig.locked;
    {
        ScopedEthEapProfileWipe wipe(config.profile);
    }
    config = std::move(result);
    return ret;
#else
    return NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
#endif
}

int32_t DeleteEthEapConfig()
{
#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
    return DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->DeleteEthEapConfig();
#else
    return NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
#endif
}

int32_t GetEthEapStateInfo(EthEapStateInfoAni &stateInfo)
{
#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
    NetManagerStandard::EthEapStateInfo nativeInfo;
    int32_t ret = DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->GetEthEapStateInfo(nativeInfo);
    if (ret == 0) {
        stateInfo = ConvertEthEapStateInfoToAni(nativeInfo);
    }
    return ret;
#else
    return NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
#endif
}

int32_t RegisterEthEapStateCallback()
{
#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
    std::lock_guard<std::mutex> lock(g_ethEapStateCallbackMutex);
    if (g_isEthEapStateCallbackRegistered) {
        return 0;
    }
    if (g_ethEapStateCallbackAni == nullptr) {
        g_ethEapStateCallbackAni = new (std::nothrow) EthEapStateCallbackAni();
    }
    if (g_ethEapStateCallbackAni == nullptr) {
        return NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
    }
    int32_t ret = DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->RegisterEthEapStateCallback(
        g_ethEapStateCallbackAni);
    if (ret == 0) {
        g_isEthEapStateCallbackRegistered = true;
    }
    return ret;
#else
    return NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
#endif
}

int32_t UnregisterEthEapStateCallback()
{
#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
    std::lock_guard<std::mutex> lock(g_ethEapStateCallbackMutex);
    if (!g_isEthEapStateCallbackRegistered || g_ethEapStateCallbackAni == nullptr) {
        return 0;
    }
    int32_t ret = DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->UnregisterEthEapStateCallback(
        g_ethEapStateCallbackAni);
    if (ret == 0) {
        g_isEthEapStateCallbackRegistered = false;
    }
    return ret;
#else
    return NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
#endif
}

int32_t ConnectEthEap(bool connect)
{
#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
    return DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->ConnectEthEap(connect);
#else
    return NetManagerStandard::NETMANAGER_EXT_ERR_OPERATION_FAILED;
#endif
}

#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
int32_t EthEapStateCallbackAni::OnEthEapStateChanged(const NetManagerStandard::EthEapStateInfo &stateInfo)
{
    execute_eth_eap_state_changed(ConvertEthEapStateInfoToAni(stateInfo));
    return 0;
}
#endif

int32_t NetEapPostBackCallbackAni::OnEapSupplicantPostback(NetManagerStandard::NetType netType,
    const sptr<NetManagerStandard::EapData> &eapData)
{
    if (eapData == nullptr) {
        return NetManagerStandard::NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    if (eapData->eapCode < EAP_CODE_MIN || eapData->eapCode > EAP_CODE_MAX) {
        NETMANAGER_EXT_LOGE("OnEapSupplicantPostback eapCode %{public}d invalid.", eapData->eapCode);
        return NetManagerStandard::NETMANAGER_ERR_PARAMETER_ERROR;
    }
    if (eapData->eapBuffer.size() == 0) {
        NETMANAGER_EXT_LOGE("OnEapSupplicantPostback eapBuffer size is 0.");
        return NetManagerStandard::NETMANAGER_ERR_PARAMETER_ERROR;
    }
    EapDataAni data{
        .net_type = static_cast<int32_t>(netType),
        .eap_code = eapData->eapCode,
        .eap_type = eapData->eapType,
        .msg_id = eapData->msgId,
        .buffer_len = eapData->bufferLen,
        .eap_buffer = rust::Vec<uint8_t>(),
    };
    data.eap_buffer.reserve(eapData->eapBuffer.size());
    for (const auto &byte : eapData->eapBuffer) {
        data.eap_buffer.push_back(byte);
    }
    execute_eap_data(data);
    return 0;
}

} // namespace NetManagerAni
} // namespace OHOS
