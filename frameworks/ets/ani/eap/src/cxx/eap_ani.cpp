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
#include "netmanager_ext_log.h"
#include "netmanager_base_permission.h"

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
static uint32_t ComposeParam(int32_t eap_code, int32_t eap_type)
{
    uint32_t composeParam = static_cast<uint32_t>(eap_code) << OFFSET_EAPCODE;
    if (eap_code != EAP_CODE_SUCCESS && eap_code != EAP_CODE_FAILURE) {
        composeParam |= static_cast<uint32_t>(eap_type);
    }
    return composeParam;
}
#endif

int32_t ValidateEapParams(int32_t net_type, int32_t eap_code, int32_t eap_type)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    if (!NetManagerStandard::NetManagerPermission::CheckPermission(
        NetManagerStandard::Permission::MANAGE_ENTERPRISE_WIFI_CONNECTION)) {
        return NetManagerStandard::EAP_ERRCODE_PERMISSION_DENIED;
    }
    if (net_type < static_cast<int32_t>(NetManagerStandard::NetType::WLAN0) ||
        net_type >= static_cast<int32_t>(NetManagerStandard::NetType::INVALID)) {
        return NetManagerStandard::EAP_ERRCODE_INVALID_NET_TYPE;
    }
    if (eap_code < EAP_CODE_MIN || eap_code > EAP_CODE_MAX) {
        return NetManagerStandard::EAP_ERRCODE_INVALID_EAP_CODE;
    }
    if (eap_type < EAP_TYPE_MIN || eap_type > EAP_TYPE_MAX) {
        return NetManagerStandard::EAP_ERRCODE_INVALID_EAP_TYPE;
    }
    return NetManagerStandard::EAP_ERRCODE_SUCCESS;
#else
    return NetManagerStandard::EAP_ERRCODE_SUCCESS;
#endif
}

int32_t RegCustomEapHandler(int32_t triggerMode, int32_t net_type,
    const rust::Vec<EapCodeTypeAni> &regItems)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    if (net_type < static_cast<int32_t>(NetManagerStandard::NetType::WLAN0) ||
        net_type >= static_cast<int32_t>(NetManagerStandard::NetType::INVALID)) {
        return NetManagerStandard::EAP_ERRCODE_INVALID_NET_TYPE;
    }
    std::string regCmd = std::to_string(net_type) + ":" + std::to_string(regItems.size());
    for (const auto &item : regItems) {
        uint32_t composeParam = ComposeParam(item.eap_code, item.eap_type);
        regCmd += ":" + std::to_string(composeParam);
    }
    NETMANAGER_EXT_LOGI("RegCustomEapHandler enter, triggerMode:%{public}d, netType:%{public}d, regCmd:%{public}s",
        static_cast<int32_t>(triggerMode), net_type, regCmd.c_str());

    if (g_NetEapPostBackCallbackAni == nullptr) {
        return NetManagerStandard::EAP_ERRCODE_INTERNAL_ERROR;
    }
    auto type = static_cast<NetManagerStandard::NetType>(net_type);
    return DelayedSingleton<NetManagerStandard::EthernetClient>::GetInstance()->RegCustomEapHandler(
        type, regCmd, g_NetEapPostBackCallbackAni);
#else
    return NetManagerStandard::EAP_ERRCODE_SUCCESS;
#endif
}

int32_t ReplyCustomEapData(int32_t result, int32_t msg_id, int32_t buffer_len,
                           const rust::Vec<uint8_t> &eap_buffer)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    if (result < static_cast<int32_t>(NetManagerStandard::CustomResult::RESULT_FAIL) ||
        result > static_cast<int32_t>(NetManagerStandard::CustomResult::RESULT_FINISH)) {
        return NetManagerStandard::EAP_ERRCODE_INVALID_RESULT;
    }

    std::unique_lock<std::mutex> lock(g_mutex);
    sptr<NetManagerStandard::EapData> eapData = sptr<NetManagerStandard::EapData>::MakeSptr();
    eapData->msgId = msg_id;
    eapData->bufferLen = buffer_len;
    eapData->eapBuffer = std::vector<uint8_t>(eap_buffer.begin(), eap_buffer.end());
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
