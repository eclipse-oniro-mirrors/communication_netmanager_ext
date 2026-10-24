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
#include "net_manager_ext_constants.h"
#include "securec.h"
#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
#include <mutex>
#include "eth_eap_config.h"
#include "eth_eap_state_info.h"
#endif

#include "wrapper.rs.h"

namespace OHOS {
namespace NetManagerAni {

namespace {
sptr<EapCallbackObserverAni> g_eapCallbackObserverAni =
    sptr<EapCallbackObserverAni>(new (std::nothrow) EapCallbackObserverAni());
#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
sptr<EthEapStateCallbackAni> g_ethEapStateCallbackAni;
[[maybe_unused]] bool g_isEthEapStateCallbackRegistered = false;
std::mutex g_ethEapStateCallbackMutex;
#endif

// CXX owns these buffers after transfer from Rust. Wipe even on validation
// failures and feature-disabled paths before their storage is released.
class ScopedEthEapProfileWipe final {
public:
    explicit ScopedEthEapProfileWipe(EthEapConfig &profile) : profile_(profile) {}
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
    EthEapConfig &profile_;
};

#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
bool ConvertEthEapProfile(const EthEapConfig &config, NetManagerStandard::EthEapProfile &profile)
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

void ConvertEthEapProfileToAni(const NetManagerStandard::EthEapProfile &profile, EthEapConfig &config)
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

int32_t RegCustomEapHandler(int32_t net_type, int32_t eap_code, int32_t eap_type)
{
    auto type = static_cast<NetManagerStandard::NetType>(net_type);
    std::string regCmd = std::to_string(eap_code) + ":" + std::to_string(eap_type);
    if (g_eapCallbackObserverAni == nullptr) {
        return -1;
    }
    int32_t ret = DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->RegCustomEapHandler(
        type, regCmd, g_eapCallbackObserverAni);
    return ret;
}

int32_t UnregCustomEapHandler(int32_t net_type, int32_t eap_code, int32_t eap_type)
{
    auto type = static_cast<NetManagerStandard::NetType>(net_type);
    auto callback = sptr<NetManagerStandard::NetRegisterEapCallbackStub>(
        new (std::nothrow) NetManagerStandard::NetRegisterEapCallbackStub());
    if (callback == nullptr) {
        return -1;
    }
    int32_t ret = DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->UnRegisterCustomEapCallback(
        type, callback);
    return ret;
}

int32_t ReplyCustomEapData(int32_t result, int32_t msg_id, int32_t buffer_len,
                           const rust::Vec<uint8_t> &eap_buffer)
{
    if (buffer_len < 0 || static_cast<size_t>(buffer_len) != eap_buffer.size()) {
        return -1;
    }
    auto eapData = sptr<NetManagerStandard::EapData>(new (std::nothrow) NetManagerStandard::EapData());
    if (eapData == nullptr) {
        return -1;
    }
    eapData->msgId = msg_id;
    eapData->bufferLen = buffer_len;
    eapData->eapBuffer = std::vector<uint8_t>(eap_buffer.begin(), eap_buffer.end());
    int32_t ret =
        DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->ReplyCustomEapData(result, eapData);
    return ret;
}

int32_t StartEthEap(int32_t net_id, EthEapConfig config)
{
    static_assert(NetManagerStandard::EapMethod::EAP_UNAUTH_TLS == 8,
        "EAP_UNAUTH_TLS must be the maximum EapMethod value");
    static_assert(NetManagerStandard::Phase2Method::PHASE2_AKA_PRIME == 7,
        "PHASE2_AKA_PRIME must be the maximum Phase2Method value");
    auto eapMethod = static_cast<NetManagerStandard::EapMethod>(config.eap_method);
    if (config.eap_method < 0 || eapMethod > NetManagerStandard::EapMethod::EAP_UNAUTH_TLS) {
        return -1;
    }
    auto phase2Method = static_cast<NetManagerStandard::Phase2Method>(config.phase2_method);
    if (config.phase2_method < 0 || phase2Method > NetManagerStandard::Phase2Method::PHASE2_AKA_PRIME) {
        return -1;
    }
    NetManagerStandard::EthEapProfile profile;
    profile.eapMethod = eapMethod;
    profile.phase2Method = phase2Method;
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
    int32_t ret =
        DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->StartEthEap(net_id, profile);
    return ret;
}

int32_t LogOffEthEap(int32_t net_id)
{
    int32_t ret = DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->LogOffEthEap(net_id);
    return ret;
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

int32_t GetEthEapStateInfo(EthEapStateInfoAni &state_info)
{
#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
    NetManagerStandard::EthEapStateInfo nativeInfo;
    int32_t ret = DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->GetEthEapStateInfo(nativeInfo);
    if (ret == 0) {
        state_info = ConvertEthEapStateInfoToAni(nativeInfo);
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

int32_t EapCallbackObserverAni::OnEapSupplicantPostback(NetManagerStandard::NetType netType,
    const sptr<NetManagerStandard::EapData> &eapData)
{
    if (eapData == nullptr) {
        return -1;
    }
    EapAniData data{
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
