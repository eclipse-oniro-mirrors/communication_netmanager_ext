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

#include "ethernet_client.h"
#include "eth_eap_profile.h"
#include "eap_data.h"
#include "net_eap_callback_stub.h"

#include "wrapper.rs.h"

namespace OHOS {
namespace NetManagerAni {

namespace {
sptr<EapCallbackObserverAni> g_eapCallbackObserverAni =
    sptr<EapCallbackObserverAni>(new (std::nothrow) EapCallbackObserverAni());
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
    profile.password = std::string(config.password);
    profile.caCertAliases = std::string(config.ca_cert_aliases);
    profile.caPath = std::string(config.ca_path);
    profile.clientCertAliases = std::string(config.client_cert_aliases);
    profile.certEntry = std::vector<uint8_t>(config.cert_entry.begin(), config.cert_entry.end());
    profile.certPassword = std::string(config.cert_password);
    profile.altSubjectMatch = std::string(config.alt_subject_match);
    profile.domainSuffixMatch = std::string(config.domain_suffix_match);
    profile.realm = std::string(config.realm);
    profile.plmn = std::string(config.plmn);
    profile.eapSubId = config.eap_sub_id;
    int32_t ret =
        DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->StartEthEap(net_id, profile);

    // Clear sensitive data from memory after use using memset_s for secure erasure
    if (!profile.password.empty()) {
        memset_s(profile.password.data(), profile.password.size(), 0, profile.password.size());
    }
    profile.password.clear();
    profile.password.shrink_to_fit();
    if (!profile.certPassword.empty()) {
        memset_s(profile.certPassword.data(), profile.certPassword.size(), 0, profile.certPassword.size());
    }
    profile.certPassword.clear();
    profile.certPassword.shrink_to_fit();

    return ret;
}

int32_t LogOffEthEap(int32_t net_id)
{
    int32_t ret = DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->LogOffEthEap(net_id);
    return ret;
}

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
