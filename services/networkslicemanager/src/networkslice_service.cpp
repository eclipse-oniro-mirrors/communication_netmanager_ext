/*
 * Copyright (C) 2025-2026 Huawei Device Co., Ltd.
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

#include <sys/time.h>
#include "networkslicemanager.h"
#include "hisysevent.h"
#include "netmgr_ext_log_wrapper.h"
#include "system_ability_definition.h"
#include "hisysevent.h"
#include "net_manager_constants.h"
#include "networkslicemsgcenter.h"
#include "networkslice_kernel_proxy.h"
#include "bundle_mgr_proxy.h"
#include "bundle_mgr_client.h"
#include "iservice_registry.h"
#include "pdp_profile_data.h"
#include "core_manager_inner.h"
#include "os_account_manager.h"
#include "state_utils.h"
#include "core_service_client.h"
#include "networkslice_service.h"

static std::string APN_DATA_DB_SELECTION = "datashare:///com.ohos.pdpprofileability/net/pdp_profile";
static std::string APN_DATA_DB_URI = "datashare:///com.ohos.pdpprofileability";
constexpr int MCC_LEN = 3;
constexpr int SLICE_MAXSIZE = 6;
constexpr int SLICE_SA_ID = 8301;
namespace OHOS {
namespace NetManagerStandard {
NetworkSliceService NetworkSliceService::instance_; /* NOLINT */

const bool REGISTER_LOCAL_RESULT_NETWORKSLICE =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<NetworkSliceService>::GetInstance().get());

NetworkSliceService::NetworkSliceService()
    : SystemAbility(SLICE_SA_ID, true), isRegistered_(false), state_(STATE_STOPPED)
{}

NetworkSliceService::~NetworkSliceService()
{
    state_ = STATE_STOPPED;
    isRegistered_ = false;
}

NetworkSliceService &NetworkSliceService::GetInstance()
{
    return instance_;
}

void NetworkSliceService::OnStart()
{
    NETMGR_EXT_LOG_I("NetworkSliceService OnStart begin");
    if (state_ == STATE_RUNNING) {
        NETMGR_EXT_LOG_I("NetworkSliceService the state is already running");
        return;
    }
    if (!Init()) {
        NETMGR_EXT_LOG_E("NetworkSliceService init failed");
        return;
    }
    state_ = STATE_RUNNING;
    NETMGR_EXT_LOG_I("NetworkSliceService OnStart end");
}

void NetworkSliceService::OnStop()
{
    NETMGR_EXT_LOG_I("NetworkSliceService OnStop begin");
    if (state_ == STATE_STOPPED) {
        return;
    }
    state_ = STATE_STOPPED;
    isRegistered_ = false;
}

bool NetworkSliceService::Init()
{
    NETMGR_EXT_LOG_I("NetworkSliceService init start");
    if (!REGISTER_LOCAL_RESULT_NETWORKSLICE) {
        NETMGR_EXT_LOG_I("Register to local sa manager failed");
        isRegistered_ = false;
        return false;
    }

    if (isRegistered_) {
        return true;
    }

    if (!Publish(DelayedSingleton<NetworkSliceService>::GetInstance().get())) {
        NETMGR_EXT_LOG_I("Register to sa manager failed");
        return false;
    }
    isRegistered_ = true;
    InitModule();
    NETMGR_EXT_LOG_I("Init success");
    return true;
}

void NetworkSliceService::InitModule()
{
    NETMGR_EXT_LOG_I("NetworkSliceService InitModule");
    DelayedSingleton<NetworkSliceManager>::GetInstance()->Init();
}

bool NetworkSliceService::UpdateNetworkSliceApn()
{
    NETMGR_EXT_LOG_I("NetworkSliceService UpdateNetworkSliceApn");
    sptr<ISystemAbilityManager> networksliceManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (networksliceManager == nullptr) {
        NETMGR_EXT_LOG_I("UpdateNetworkSliceApn networksliceManager == nullptr");
        return false;
    }
    sptr<IRemoteObject> remoteObj = networksliceManager->GetSystemAbility(SLICE_SA_ID);
    if (remoteObj == nullptr) {
        NETMGR_EXT_LOG_I("UpdateNetworkSliceApn remoteObj == nullptr");
        return false;
    }
    std::shared_ptr<DataShare::DataShareHelper> networksliceApnHelper =
        OHOS::DataShare::DataShareHelper::Creator(remoteObj, APN_DATA_DB_URI);
    if (networksliceApnHelper == nullptr) {
        NETMGR_EXT_LOG_I("UpdateNetworkSliceApn networksliceApnHelper == nullptr");
        return false;
    }

    DataShare::DataSharePredicates predicates;
    std::u16string operatorNumeric;
    int32_t slotId = StateUtils::GetPrimarySlotId();
    Telephony::CoreServiceClient::GetInstance().GetSimOperatorNumeric(slotId, operatorNumeric);
    std::string plmn = Str16ToStr8(operatorNumeric);
    NETMGR_EXT_LOG_I("UpdateNetworkSliceApn plmn = %{public}s", plmn.c_str());
    bool writeResult = false;
    for (int i = 1; i <= SLICE_MAXSIZE; i++) {
        std::string apntype = "snssai";
        apntype += std::to_string(i);
        predicates.EqualTo(OHOS::Telephony::PdpProfileData::MCCMNC, plmn)\
            ->EqualTo(OHOS::Telephony::PdpProfileData::APN_TYPES, apntype);
        Uri networksliceUrl(APN_DATA_DB_SELECTION);
        networksliceApnHelper->Delete(networksliceUrl, predicates);
        if (WriteNetworkSliceApnToDb(networksliceApnHelper, apntype) == true) {
            writeResult = true;
        }
    }
    networksliceApnHelper->Release();
    return writeResult;
}

int NetworkSliceService::WriteNetworkSliceApnToDb(std::shared_ptr<DataShare::DataShareHelper> networksliceApnHelper,
    std::string apntype)
{
    std::u16string operatorNumeric;
    int32_t slotId = StateUtils::GetPrimarySlotId();
    Telephony::CoreServiceClient::GetInstance().GetSimOperatorNumeric(slotId, operatorNumeric);
    std::string plmn = Str16ToStr8(operatorNumeric);
    NETMGR_EXT_LOG_I("WriteNetworkSliceApnToDb plmn = %{public}s", plmn.c_str());
    Uri networksliceUrl(APN_DATA_DB_SELECTION);
    DataShare::DataShareValuesBucket value;
    value.Put(Telephony::PdpProfileData::MCCMNC, plmn);
    value.Put(Telephony::PdpProfileData::MCC, plmn.substr(0, MCC_LEN));
    value.Put(Telephony::PdpProfileData::MNC, plmn.substr(MCC_LEN));
    value.Put(Telephony::PdpProfileData::AUTH_TYPE, "0");
    value.Put(Telephony::PdpProfileData::EDITED_STATUS, "1");
    value.Put(Telephony::PdpProfileData::OPKEY, plmn);
    value.Put(Telephony::PdpProfileData::APN_TYPES, apntype);
    value.Put(Telephony::PdpProfileData::BEARING_SYSTEM_TYPE, "0");
    value.Put(Telephony::PdpProfileData::IS_ROAMING_APN, "0");
    std::string apnName = apntype;
    if (!apnName.empty()) {
        value.Put(Telephony::PdpProfileData::APN, apnName.c_str());
    }
    value.Put(Telephony::PdpProfileData::PROFILE_NAME, apntype);
    value.Put(Telephony::PdpProfileData::APN_PROTOCOL, "IPV4V6");
    value.Put(Telephony::PdpProfileData::APN_ROAM_PROTOCOL, "IPV4V6");
    return networksliceApnHelper->Insert(networksliceUrl, value);
}

int32_t NetworkSliceService::SetNetworkSliceUePolicy(std::vector<uint8_t> buffer)
{
    NETMGR_EXT_LOG_I("NetworkSliceService::SetNetworkSliceUePolicy");
    std::shared_ptr<std::vector<uint8_t>> msg = std::make_shared<std::vector<uint8_t>>(buffer);
    Singleton<NetworkSliceMsgCenter>::GetInstance().Publish(EVENT_HANDLE_UE_POLICY, msg);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkSliceService::NetworkSliceAllowedNssaiRpt(std::vector<uint8_t> buffer)
{
    NETMGR_EXT_LOG_I("NetworkSliceService::NetworkSliceAllowedNssaiRpt");
    std::shared_ptr<std::vector<uint8_t>> msg = std::make_shared<std::vector<uint8_t>>(buffer);
    Singleton<NetworkSliceMsgCenter>::GetInstance().Publish(EVENT_HANDLE_ALLOWED_NSSAI, msg);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkSliceService::NetworkSliceEhplmnRpt(std::vector<uint8_t> buffer)
{
    NETMGR_EXT_LOG_I("NetworkSliceService::NetworkSliceEhplmnRpt");
    std::shared_ptr<std::vector<uint8_t>> msg = std::make_shared<std::vector<uint8_t>>(buffer);
    Singleton<NetworkSliceMsgCenter>::GetInstance().Publish(EVENT_HANDLE_EHPLMN, msg);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkSliceService::NetworkSliceInitUePolicy()
{
    NETMGR_EXT_LOG_I("NetworkSliceService::InitUePolicy");
    Singleton<NetworkSliceMsgCenter>::GetInstance().Publish(EVENT_INIT_UE_POLICY);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkSliceService::NetworkSliceGetRSDByAppDescriptor(std::shared_ptr<GetSlicePara>& getSlicePara)
{
    NETMGR_EXT_LOG_I("NetworkSliceService::GetRSDByAppDescriptor");
    Singleton<NetworkSliceMsgCenter>::GetInstance().Publish(EVENT_GET_SLICE_PARA, getSlicePara);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkSliceService::RecvKernelData(void* rcvMsg, int32_t dataLen)
{
    NETMGR_EXT_LOG_I("NetworkSliceService::RecvKernelData");
    if (rcvMsg == nullptr || dataLen <= 0) {
        return NETMANAGER_EXT_ERR_LOCAL_PTR_NULL;
    }
    KernelMsgNS *kernelMsg = reinterpret_cast<KernelMsgNS *>(rcvMsg);
    if (!kernelMsg) {
        NETMGR_EXT_LOG_I("NetworkSliceService RecvKernelData kernelMsg is null");
        return NETMANAGER_EXT_ERR_LOCAL_PTR_NULL;
    }

    int32_t buflen = dataLen - sizeof(short) - sizeof(short);
    if (buflen <= 0) {
        NETMGR_EXT_LOG_I("NetworkSliceServiceKernel msg buff is null.");
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    short len = (static_cast<unsigned char>(kernelMsg->buf[3]) << 8) |
                (static_cast<unsigned char>(kernelMsg->buf[2]));
    std::vector<uint8_t> msgData;
    msgData.resize(len);
    std::copy(kernelMsg->buf, kernelMsg->buf + len, msgData.data());
    for (int i = 0; i < (int)msgData.size(); ++i) {
        NETMGR_EXT_LOG_I("NetworkSliceService msgData[%{public}d] = %{public}d", i, msgData[i]);
    }
    std::shared_ptr<std::vector<uint8_t>> msg = std::make_shared<std::vector<uint8_t>>(msgData);
    Singleton<NetworkSliceMsgCenter>::GetInstance().Publish(EVENT_KERNEL_IP_ADDR_REPORT, msg);
    return NETMANAGER_EXT_SUCCESS;
}

int NetworkSliceService::GetBundleNameForUid(int32_t uid, std::string &bundleName)
{
    auto systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        NETMGR_EXT_LOG_E("GetBundleNameForUid failed, system ability manager is null");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        NETMGR_EXT_LOG_E("GetBundleNameForUid failed, bundle manager service is not ready");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    sptr<AppExecFwk::IBundleMgr> iBundleMgr = iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    if (!iBundleMgr) {
        NETMGR_EXT_LOG_E("GetBundleNameForUid failed, bundle manager is null");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    return iBundleMgr->GetNameForUid(uid, bundleName);
}

int32_t NetworkSliceService::BindToNetwork(std::map<std::string, std::string> buffer)
{
    NETMGR_EXT_LOG_I("NetworkSliceService::BindToNetwork");
    std::shared_ptr<std::map<std::string, std::string>> msg =
        std::make_shared<std::map<std::string, std::string>>(buffer);
    Singleton<NetworkSliceMsgCenter>::GetInstance().Publish(EVENT_BIND_TO_NETWORK, msg);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkSliceService::DelBindToNetwork(std::map<std::string, std::string> buffer)
{
    NETMGR_EXT_LOG_I("NetworkSliceService::DelBindToNetwork");
    std::shared_ptr<std::map<std::string, std::string>> msg =
        std::make_shared<std::map<std::string, std::string>>(buffer);
    Singleton<NetworkSliceMsgCenter>::GetInstance().Publish(EVENT_DEL_BIND_TO_NETWORK, msg);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkSliceService::GetUidByBundleName(const std::string& bundleName)
{
    AppExecFwk::BundleInfo info;
    AppExecFwk::BundleMgrClient client;
    if (!client.GetBundleInfo(bundleName, AppExecFwk::GET_BUNDLE_DEFAULT, info,
        AppExecFwk::Constants::ALL_USERID)) {
        NETMGR_EXT_LOG_E("Failed to query uid from bms");
    } else {
        NETMGR_EXT_LOG_D("Succ to get uid=%{public}d", info.uid);
    }
    return info.uid;
}

std::set<int32_t> NetworkSliceService::GetUidsByBundleName(const std::string& bundleName)
{
    AppExecFwk::BundleInfo info;
    AppExecFwk::BundleMgrClient client;
    if (!client.GetBundleInfo(bundleName, AppExecFwk::GET_BUNDLE_DEFAULT, info,
        AppExecFwk::Constants::ALL_USERID)) {
        NETMGR_EXT_LOG_E("Failed to query uid from bms");
    } else {
        NETMGR_EXT_LOG_D("Succ to get uid=%{public}d", info.uid);
    }
    int32_t bundleId = info.uid % 200000;
    std::vector<AccountSA::OsAccountInfo> osAccountInfos;
    AccountSA::OsAccountManager::QueryAllCreatedOsAccounts(osAccountInfos);
    std::set<int32_t> uids;
    for (int i = 0; i < (int)osAccountInfos.size(); ++i) {
        int32_t uid = osAccountInfos[i].GetLocalId() * 200000 + bundleId % 200000;
        uids.insert(uid);
    }
    return uids;
}

int32_t NetworkSliceService::GetRouteSelectionDescriptorByDNN(std::string dnn, std::string& snssai, uint8_t& sscMode)
{
    NETMGR_EXT_LOG_I("NetworkSliceService::GetRSDByAppDescriptor");
    DelayedSingleton<NetworkSliceManager>::GetInstance()->GetRouteSelectionDescriptorByDNN(dnn, snssai, sscMode);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkSliceService::GetRSDByNetCap(int32_t netcap, std::map<std::string, std::string>& networkSliceParas)
{
    NETMGR_EXT_LOG_I("NetworkSliceService::GetRSDByNetCap");
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->GetRSDByNetCap(netcap, networkSliceParas);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkSliceService::SetSaState(bool isSaState)
{
    NETMGR_EXT_LOG_I("NetworkSliceService::SetSaState");
    DelayedSingleton<NetworkSliceManager>::GetInstance()->SetSaState(isSaState);
    return NETMANAGER_EXT_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS
