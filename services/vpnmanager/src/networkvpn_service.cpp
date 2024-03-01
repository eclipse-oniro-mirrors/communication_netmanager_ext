/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "networkvpn_service.h"

#include <cerrno>
#include <ctime>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <nlohmann/json.hpp>
#include <string>
#include <fstream>
#include <thread>
#include <nlohmann/json.hpp>
#include <string>
#include <fstream>

#include "ipc_skeleton.h"
#include "securec.h"
#include "system_ability_definition.h"

#include "extended_vpn_ctl.h"
#include "net_event_report.h"
#include "net_manager_center.h"
#include "net_manager_constants.h"
#include "net_manager_ext_constants.h"
#include "netmanager_base_permission.h"
#include "netmgr_ext_log_wrapper.h"
#include "netsys_controller.h"
#include "networkvpn_hisysevent.h"
#include "net_datashare_utils_iface.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t MAX_CALLBACK_COUNT = 128;
constexpr const char *NET_ACTIVATE_WORK_THREAD = "VPN_CALLBACK_WORK_THREAD";
constexpr const char* VPN_CONFIG_FILE = "/data/service/el1/public/netmanager/vpn_config.json";
constexpr const char* VPN_EXTENSION_LABEL = ":vpn";
constexpr uint32_t MAX_GET_SERVICE_COUNT = 30;
constexpr uint32_t WAIT_FOR_SERVICE_TIME_S = 1;
constexpr uint32_t AGAIN_REGISTER_CALLBACK_INTERVAL = 500;
constexpr uint32_t MAX_RETRY_TIMES = 10;

const bool REGISTER_LOCAL_RESULT_NETVPN =
    SystemAbility::MakeAndRegisterAbility(&Singleton<NetworkVpnService>::GetInstance());

NetworkVpnService::NetworkVpnService() : SystemAbility(COMM_VPN_MANAGER_SYS_ABILITY_ID, true) {}
NetworkVpnService::~NetworkVpnService() = default;

void NetworkVpnService::OnStart()
{
    if (state_ == STATE_RUNNING) {
        NETMGR_EXT_LOG_D("OnStart Vpn Service state is already running");
        return;
    }
    if (!Init()) {
        NETMGR_EXT_LOG_E("OnStart Vpn init failed");
        VpnHisysEvent::SendFaultEvent(VpnEventType::TYPE_UNKNOWN, VpnEventOperator::OPERATION_START_SA,
                                      VpnEventErrorType::ERROR_INTERNAL_ERROR, "Start Vpn Service failed");
        return;
    }
    state_ = STATE_RUNNING;
    NETMGR_EXT_LOG_I("OnStart vpn successful");
}

void NetworkVpnService::OnStop()
{
    state_ = STATE_STOPPED;
    isServicePublished_ = false;

    if (policyCallRunner_) {
        policyCallRunner_->Stop();
    }
    NETMGR_EXT_LOG_I("OnStop vpn successful");
}

int32_t NetworkVpnService::Dump(int32_t fd, const std::vector<std::u16string> &args)
{
    std::string result;
    GetDumpMessage(result);
    NETMGR_EXT_LOG_I("Vpn dump fd: %{public}d, content: %{public}s", fd, result.c_str());
    int32_t ret = dprintf(fd, "%s\n", result.c_str());
    if (ret < 0) {
        NETMGR_EXT_LOG_E("dprintf failed, errno[%{public}d]", errno);
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

bool NetworkVpnService::Init()
{
    if (!REGISTER_LOCAL_RESULT_NETVPN) {
        NETMGR_EXT_LOG_E("Register to local sa manager failed");
        return false;
    }
    if (!isServicePublished_) {
        if (!Publish(&Singleton<NetworkVpnService>::GetInstance())) {
            NETMGR_EXT_LOG_E("Register to sa manager failed");
            return false;
        }
        isServicePublished_ = true;
    }

    AddSystemAbilityListener(COMM_NETSYS_NATIVE_SYS_ABILITY_ID);

    SubscribeCommonEvent();
    if (!vpnConnCallback_) {
        vpnConnCallback_ = std::make_shared<VpnConnStateCb>(*this);
    }

    if (!policyCallHandler_) {
        policyCallRunner_ = AppExecFwk::EventRunner::Create(NET_ACTIVATE_WORK_THREAD);
        policyCallHandler_ = std::make_shared<AppExecFwk::EventHandler>(policyCallRunner_);
    }
    vpnHapObserver_ = new VpnHapObserver(*this);
    RegisterFactoryResetCallback();
    return true;
}

void NetworkVpnService::GetDumpMessage(std::string &message)
{
    std::unique_lock<std::mutex> locker(netVpnMutex_);
    message.append("Net Vpn Info:\n");
    if (vpnObj_ != nullptr) {
        const auto &config = vpnObj_->GetVpnConfig();
        std::string isLegacy = (config->isLegacy_) ? "true" : "false";
        message.append("\tisLegacy: " + isLegacy + "\n");
        message.append("\tPackageName: " + vpnObj_->GetVpnPkg() + "\n");
        message.append("\tinterface: " + vpnObj_->GetInterfaceName() + "\n");
        message.append("\tstate: connected\n");
    } else {
        message.append("\tstate: disconnected\n");
    }
    message.append("\tend.\n");
}

void NetworkVpnService::VpnConnStateCb::OnVpnConnStateChanged(const VpnConnectState &state)
{
    NETMGR_EXT_LOG_I("receive new vpn connect state[%{public}d].", static_cast<uint32_t>(state));
    if (vpnService_.policyCallHandler_) {
        vpnService_.policyCallHandler_->PostSyncTask([this, &state]() {
            std::for_each(vpnService_.vpnEventCallbacks_.begin(), vpnService_.vpnEventCallbacks_.end(),
                          [&state](const auto &callback) {
                              callback->OnVpnStateChanged((VpnConnectState::VPN_CONNECTED == state) ? true : false);
                          });
        });
    }
}

void NetworkVpnService::OnVpnMultiUserSetUp()
{
    NETMGR_EXT_LOG_I("user multiple execute set up.");
    if (policyCallHandler_) {
        policyCallHandler_->PostSyncTask([this]() {
            std::for_each(vpnEventCallbacks_.begin(), vpnEventCallbacks_.end(),
                          [](const auto &callback) { callback->OnVpnMultiUserSetUp(); });
        });
    }
}

int32_t NetworkVpnService::Prepare(bool &isExistVpn, bool &isRun, std::string &pkg)
{
    std::unique_lock<std::mutex> locker(netVpnMutex_);
    isRun = false;
    isExistVpn = false;
    if (vpnObj_ != nullptr) {
        isExistVpn = true;
        isRun = vpnObj_->IsVpnConnecting();
        pkg = vpnObj_->GetVpnPkg();
    }
    NETMGR_EXT_LOG_I("NetworkVpnService Prepare successfully");
    return NETMANAGER_EXT_SUCCESS;
}

void NetworkVpnService::ConvertStringToConfig(sptr<VpnConfig> &vpnCfg, const nlohmann::json& doc)
{
    if (doc.contains("dnsAddresses") && doc.at("dnsAddresses").is_array()) {
        nlohmann::json jDnsAddrs = doc.at("dnsAddresses");
        for (const auto& mem : jDnsAddrs) {
            if (mem.is_string()) {
                vpnCfg->dnsAddresses_.push_back(mem);
            }
        }
    }
    if (doc.contains("searchDomains") && doc.at("searchDomains").is_array()) {
        nlohmann::json jDomains = doc.at("searchDomains");
        for (const auto& mem : jDomains) {
            if (mem.is_string()) {
                vpnCfg->searchDomains_.push_back(mem);
            }
        }
    }
    if (doc.contains("acceptedApplications") && doc.at("acceptedApplications").is_array()) {
        nlohmann::json jAcceptApp = doc.at("acceptedApplications");
        for (const auto& mem : jAcceptApp) {
            if (mem.is_string()) {
                vpnCfg->acceptedApplications_.push_back(mem);
            }
        }
    }
    if (doc.contains("refusedApplications") && doc.at("refusedApplications").is_array()) {
        nlohmann::json jRefuseApp = doc.at("refusedApplications");
        for (const auto& mem : jRefuseApp) {
            if (mem.is_string()) {
                vpnCfg->refusedApplications_.push_back(mem);
            }
        }
    }
}

void NetworkVpnService::ConvertNetAddrToConfig(INetAddr& tmp, const nlohmann::json& mem)
{
    if (mem.contains("type") && mem.at("type").is_number()) {
        tmp.type_ = static_cast<int32_t>(mem.at("type"));
    }
    if (mem.contains("family") && mem.at("family").is_number()) {
        tmp.family_ = static_cast<int32_t>(mem.at("family"));
    }
    if (mem.contains("prefixlen") && mem.at("prefixlen").is_number()) {
        tmp.prefixlen_ = static_cast<int32_t>(mem.at("prefixlen"));
    }
    if (mem.contains("address") && mem.at("address").is_string()) {
        tmp.address_ = mem.at("address");
    }
    if (mem.contains("netMask") && mem.at("netMask").is_string()) {
        tmp.netMask_ = mem.at("netMask");
    }
    if (mem.contains("hostName") && mem.at("hostName").is_string()) {
        tmp.hostName_ = mem.at("hostName");
    }
    if (mem.contains("port") && mem.at("port").is_number()) {
        tmp.port_ = static_cast<int32_t>(mem.at("port"));
    }
}

void NetworkVpnService::ConvertVecAddrToConfig(sptr<VpnConfig> &vpnCfg, const nlohmann::json& doc)
{
    if (doc.contains("addresses") && doc.at("addresses").is_array()) {
        nlohmann::json jAddrs = doc.at("addresses");
        for (const auto& mem : jAddrs) {
            if (mem.is_object()) {
                INetAddr tmp;
                ConvertNetAddrToConfig(tmp, mem);
                vpnCfg->addresses_.push_back(tmp);
            }
        }
    }
}

void NetworkVpnService::ConvertRouteToConfig(Route& tmp, const nlohmann::json& mem)
{
    if (mem.contains("iface") && mem.at("iface").is_string()) {
        tmp.iface_ = mem.at("iface");
    }
    if (mem.contains("rtnType") && mem.at("rtnType").is_number()) {
        tmp.rtnType_ = mem.at("rtnType");
    }
    if (mem.contains("mtu") && mem.at("mtu").is_number()) {
        tmp.mtu_ = mem.at("mtu");
    }
    if (mem.contains("isHost") && mem.at("isHost").is_boolean()) {
        tmp.isHost_ = mem.at("isHost");
    }
    if (mem.contains("hasGateway") && mem.at("hasGateway").is_boolean()) {
        tmp.hasGateway_ = mem.at("hasGateway");
    }
    if (mem.contains("isDefaultRoute") && mem.at("isDefaultRoute").is_boolean()) {
        tmp.isDefaultRoute_ = mem.at("isDefaultRoute");
    }
    if (mem.contains("destination") && mem.at("destination").is_object()) {
        nlohmann::json elem = mem.at("destination");
        INetAddr tmpINet;
        ConvertNetAddrToConfig(tmpINet, elem);
        tmp.destination_ = tmpINet;
    }
    if (mem.contains("gateway") && mem.at("gateway").is_object()) {
        nlohmann::json elem = mem.at("gateway");
        INetAddr tmpINet;
        ConvertNetAddrToConfig(tmpINet, elem);
        tmp.gateway_ = tmpINet;
    }
}

void NetworkVpnService::ConvertVecRouteToConfig(sptr<VpnConfig> &vpnCfg, const nlohmann::json& doc)
{
    if (doc.contains("routes") && doc.at("routes").is_array()) {
        nlohmann::json jRoutes = doc.at("routes");
        for (const auto& mem : jRoutes) {
            if (mem.is_object()) {
                Route tmp;
                ConvertRouteToConfig(tmp, mem);
                vpnCfg->routes_.push_back(tmp);
            }
        }
    }
}

void NetworkVpnService::ParseJsonToConfig(sptr<VpnConfig> &vpnCfg, const std::string& jsonString)
{
    if (jsonString.empty() || !nlohmann::json::accept(jsonString)) {
        return;
    }
    nlohmann::json doc = nlohmann::json::parse(jsonString);
    if (doc.contains("mtu") && doc.at("mtu").is_number()) {
        vpnCfg->mtu_ = doc.at("mtu");
    }
    if (doc.contains("isAcceptIPv4") && doc.at("isAcceptIPv4").is_boolean()) {
        vpnCfg->isAcceptIPv4_ = doc.at("isAcceptIPv4");
    }
    if (doc.contains("isAcceptIPv6") && doc.at("isAcceptIPv6").is_boolean()) {
        vpnCfg->isAcceptIPv6_ = doc.at("isAcceptIPv6");
    }
    if (doc.contains("isLegacy") && doc.at("isLegacy").is_boolean()) {
        vpnCfg->isLegacy_ = doc.at("isLegacy");
    }
    if (doc.contains("isMetered") && doc.at("isMetered").is_boolean()) {
        vpnCfg->isMetered_ = doc.at("isMetered");
    }
    if (doc.contains("isBlocking") && doc.at("isBlocking").is_boolean()) {
        vpnCfg->isBlocking_ = doc.at("isBlocking");
    }
    ConvertStringToConfig(vpnCfg, doc);

    ConvertVecAddrToConfig(vpnCfg, doc);

    ConvertVecRouteToConfig(vpnCfg, doc);
}

void NetworkVpnService::RecoverVpnConfig()
{
    sptr<VpnConfig> vpnCfg = new VpnConfig();
    std::ifstream ifs(VPN_CONFIG_FILE);
    if (!ifs) {
        NETMGR_EXT_LOG_D("file don't exist, don't need recover");
        return;
    }
    std::string jsonString;
    std::getline(ifs, jsonString);
    ParseJsonToConfig(vpnCfg, jsonString);
    SetUpVpn(vpnCfg);
}

void NetworkVpnService::ConvertNetAddrToJson(const INetAddr& netAddr, nlohmann::json& jInetAddr)
{
    jInetAddr["type"] = netAddr.type_;
    jInetAddr["family"] = netAddr.family_;
    jInetAddr["prefixlen"] = netAddr.prefixlen_;
    jInetAddr["address"] = netAddr.address_;
    jInetAddr["netMask"] = netAddr.netMask_;
    jInetAddr["hostName"] = netAddr.hostName_;
    jInetAddr["port"] = netAddr.port_;
}

void NetworkVpnService::ConvertVecRouteToJson(const std::vector<Route>& routes, nlohmann::json& jVecRoutes)
{
    for (const auto& mem : routes) {
        nlohmann::json jRoute;
        jRoute["iface"] = mem.iface_;
        nlohmann::json jDestination;
        ConvertNetAddrToJson(mem.destination_, jDestination);
        jRoute["destination"] = jDestination;
        nlohmann::json jGateway;
        ConvertNetAddrToJson(mem.gateway_, jGateway);
        jRoute["gateway"] = jGateway;
        jRoute["rtnType"] = mem.rtnType_;
        jRoute["mtu"] = mem.mtu_;
        jRoute["isHost"] = mem.isHost_;
        jRoute["hasGateway"] = mem.hasGateway_;
        jRoute["isDefaultRoute"] = mem.isDefaultRoute_;
        jVecRoutes.push_back(jRoute);
    }
}

void NetworkVpnService::ParseConfigToJson(const sptr<VpnConfig> &vpnCfg, std::string& jsonString)
{
    nlohmann::json jVpnCfg;
    nlohmann::json jVecAddrs = nlohmann::json::array();
    for (const auto& mem : vpnCfg->addresses_) {
        nlohmann::json jInetAddr;
        ConvertNetAddrToJson(mem, jInetAddr);
        jVecAddrs.push_back(jInetAddr);
    }
    jVpnCfg["addresses"] = jVecAddrs;

    nlohmann::json jVecRoutes = nlohmann::json::array();
    ConvertVecRouteToJson(vpnCfg->routes_, jVecRoutes);
    jVpnCfg["routes"] = jVecRoutes;

    jVpnCfg["mtu"] = vpnCfg->mtu_;
    jVpnCfg["isAcceptIPv4"] = vpnCfg->isAcceptIPv4_;
    jVpnCfg["isAcceptIPv6"] = vpnCfg->isAcceptIPv6_;
    jVpnCfg["isLegacy"] = vpnCfg->isLegacy_;
    jVpnCfg["isMetered"] = vpnCfg->isMetered_;
    jVpnCfg["isBlocking"] = vpnCfg->isBlocking_;

    nlohmann::json jVecDnsAddrs = nlohmann::json::array();
    for (const auto& mem : vpnCfg->dnsAddresses_) {
        jVecDnsAddrs.push_back(mem);
    }
    jVpnCfg["dnsAddresses"] = jVecDnsAddrs;

    nlohmann::json jVecDomains = nlohmann::json::array();
    for (const auto& mem : vpnCfg->searchDomains_) {
        jVecDomains.push_back(mem);
    }
    jVpnCfg["searchDomains"] = jVecDomains;

    nlohmann::json jVecAcceptApp = nlohmann::json::array();
    for (const auto& mem : vpnCfg->acceptedApplications_) {
        jVecAcceptApp.push_back(mem);
    }
    jVpnCfg["acceptedApplications"] = jVecAcceptApp;

    nlohmann::json jVecRefuseApp = nlohmann::json::array();
    for (const auto& mem : vpnCfg->refusedApplications_) {
        jVecRefuseApp.push_back(mem);
    }
    jVpnCfg["refusedApplications"] = jVecRefuseApp;

    jsonString = jVpnCfg.dump();
}

void NetworkVpnService::SaveVpnConfig(const sptr<VpnConfig> &vpnCfg)
{
    std::string jsonString;
    ParseConfigToJson(vpnCfg, jsonString);
    std::ofstream ofs(VPN_CONFIG_FILE);
    ofs << jsonString;
}

int32_t NetworkVpnService::SetUpVpn(const sptr<VpnConfig> &config, bool isVpnExtCall)
{
    std::unique_lock<std::mutex> locker(netVpnMutex_);
    int32_t userId = AppExecFwk::Constants::UNSPECIFIED_USERID;
    std::vector<int32_t> activeUserIds;
    int32_t ret = CheckCurrentAccountType(userId, activeUserIds);
    if (NETMANAGER_EXT_SUCCESS != ret) {
        return ret;
    }

    if (vpnObj_ != nullptr) {
        if (vpnObj_->GetUserId() == userId) {
            NETMGR_EXT_LOG_W("vpn exist already, please execute destory first");
            return NETWORKVPN_ERROR_VPN_EXIST;
        } else {
            OnVpnMultiUserSetUp();
            vpnObj_->Destroy();
        }
    }

    vpnObj_ = std::make_shared<ExtendedVpnCtl>(config, "", userId, activeUserIds);
    if (vpnObj_->RegisterConnectStateChangedCb(vpnConnCallback_) != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("SetUpVpn register internal callback fail.");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    ret = vpnObj_->SetUp();
    if (ret == NETMANAGER_EXT_SUCCESS && !vpnBundleName_.empty()) {
        std::vector<std::string> list = {vpnBundleName_, vpnBundleName_.append(VPN_EXTENSION_LABEL)};
        auto regRet =
            Singleton<AppExecFwk::AppMgrClient>::GetInstance().RegisterApplicationStateObserver(vpnHapObserver_, list);
        NETMGR_EXT_LOG_I("vpnHapOberver RegisterApplicationStateObserver ret = %{public}d", regRet);
    }
    NETMGR_EXT_LOG_I("NetworkVpnService SetUp");
    return ret;
}

int32_t NetworkVpnService::Protect(bool isVpnExtCall)
{
    /*
     * Only permission verification is performed and
     * the protected socket implements fwmark_service in the netsys process.
     */
    NETMGR_EXT_LOG_I("Protect vpn tunnel successfully.");
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnService::DestroyVpn(bool isVpnExtCall)
{
    std::unique_lock<std::mutex> locker(netVpnMutex_);
    int32_t userId = AppExecFwk::Constants::UNSPECIFIED_USERID;
    std::vector<int32_t> activeUserIds;
    int32_t ret = CheckCurrentAccountType(userId, activeUserIds);
    if (NETMANAGER_EXT_SUCCESS != ret) {
        return ret;
    }

    if ((vpnObj_ != nullptr) && (vpnObj_->Destroy() != NETMANAGER_EXT_SUCCESS)) {
        NETMGR_EXT_LOG_E("destroy vpn is failed");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    vpnObj_ = nullptr;
    // remove vpn config
    remove(VPN_CONFIG_FILE);
    vpnBundleName_ = "";

    NETMGR_EXT_LOG_I("Destroy vpn successfully.");
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnService::RegisterVpnEvent(const sptr<IVpnEventCallback> callback)
{
    int32_t ret = NETMANAGER_EXT_ERR_OPERATION_FAILED;
    if (policyCallHandler_) {
        policyCallHandler_->PostSyncTask([this, &callback, &ret]() { ret = SyncRegisterVpnEvent(callback); });
    }
    return ret;
}

int32_t NetworkVpnService::UnregisterVpnEvent(const sptr<IVpnEventCallback> callback)
{
    int32_t ret = NETMANAGER_EXT_ERR_OPERATION_FAILED;
    if (policyCallHandler_) {
        policyCallHandler_->PostSyncTask([this, &callback, &ret]() { ret = SyncUnregisterVpnEvent(callback); });
    }
    return ret;
}

int32_t NetworkVpnService::CreateVpnConnection(bool isVpnExtCall)
{
    /*
     * Only permission verification is performed
     */
    NETMGR_EXT_LOG_I("CreateVpnConnection successfully.");
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnService::CheckCurrentAccountType(int32_t &userId, std::vector<int32_t> &activeUserIds)
{
    int32_t uid = IPCSkeleton::GetCallingUid();
    int32_t userId_Max = 99;
    if (AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(uid, userId) != ERR_OK) {
        NETMGR_EXT_LOG_E("GetOsAccountLocalIdFromUid error, uid: %{public}d.", uid);
        return NETMANAGER_EXT_ERR_INTERNAL;
    }

    if (AccountSA::OsAccountManager::QueryActiveOsAccountIds(activeUserIds) != ERR_OK) {
        NETMGR_EXT_LOG_E("QueryActiveOsAccountIds error.");
    }

    if (userId >= 0 && userId <= userId_Max) {
       return NETMANAGER_EXT_SUCCESS;
    }

    auto itr = std::find_if(activeUserIds.begin(), activeUserIds.end(),
                            [userId](const int32_t &elem) { return (elem == userId) ? true : false; });
    if (itr == activeUserIds.end()) {
        NETMGR_EXT_LOG_E("userId: %{public}d is not active user. activeUserIds.size: %{public}zd", userId,
                         activeUserIds.size());
        return NETWORKVPN_ERROR_REFUSE_CREATE_VPN;
    }

    activeUserIds.clear();

    AccountSA::OsAccountInfo accountInfo;
    if (AccountSA::OsAccountManager::QueryOsAccountById(userId, accountInfo) != ERR_OK) {
        NETMGR_EXT_LOG_E("QueryOsAccountById error, userId: %{public}d.", userId);
        return NETMANAGER_EXT_ERR_INTERNAL;
    }

    if (accountInfo.GetType() == AccountSA::OsAccountType::GUEST) {
        NETMGR_EXT_LOG_E("The guest user cannot execute the VPN interface.");
        return NETWORKVPN_ERROR_REFUSE_CREATE_VPN;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnService::SyncRegisterVpnEvent(const sptr<IVpnEventCallback> callback)
{
    for (auto iterCb = vpnEventCallbacks_.begin(); iterCb != vpnEventCallbacks_.end(); iterCb++) {
        if ((*iterCb)->AsObject().GetRefPtr() == callback->AsObject().GetRefPtr()) {
            NETMGR_EXT_LOG_E("Register vpn event callback failed, callback already exists");
            return NETMANAGER_EXT_ERR_OPERATION_FAILED;
        }
    }

    if (vpnEventCallbacks_.size() >= MAX_CALLBACK_COUNT) {
        NETMGR_EXT_LOG_E("callback above max count, return error.");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }

    vpnEventCallbacks_.push_back(callback);
    NETMGR_EXT_LOG_I("Register vpn event callback successfully");
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnService::SyncUnregisterVpnEvent(const sptr<IVpnEventCallback> callback)
{
    for (auto iter = vpnEventCallbacks_.begin(); iter != vpnEventCallbacks_.end(); ++iter) {
        if (callback->AsObject().GetRefPtr() == (*iter)->AsObject().GetRefPtr()) {
            vpnEventCallbacks_.erase(iter);
            NETMGR_EXT_LOG_I("Unregister vpn event successfully.");
            return NETMANAGER_EXT_SUCCESS;
        }
    }
    NETMGR_EXT_LOG_E("Unregister vpn event callback is does not exist.");
    return NETMANAGER_EXT_ERR_OPERATION_FAILED;
}

void NetworkVpnService::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    NETMGR_EXT_LOG_D("NetworkVpnService::OnAddSystemAbility systemAbilityId[%{public}d]", systemAbilityId);
    if (systemAbilityId == COMM_NETSYS_NATIVE_SYS_ABILITY_ID) {
        if (hasSARemoved_) {
            OnNetSysRestart();
            hasSARemoved_ = false;
        }
    }
}

void NetworkVpnService::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    NETMGR_EXT_LOG_D("NetworkVpnService::OnRemoveSystemAbility systemAbilityId[%{public}d]", systemAbilityId);
    if (systemAbilityId == COMM_NETSYS_NATIVE_SYS_ABILITY_ID) {
        hasSARemoved_ = true;
    }
}

void NetworkVpnService::OnNetSysRestart()
{
    NETMGR_EXT_LOG_I("NetworkVpnService::OnNetSysRestart");

    if (vpnObj_ != nullptr) {
        vpnObj_->ResumeUids();
    }
}

int32_t NetworkVpnService::FactoryResetVpn()
{
    NETMGR_EXT_LOG_I("factory reset Vpn enter.");

    return NETMANAGER_EXT_SUCCESS;
}

void NetworkVpnService::RegisterFactoryResetCallback()
{
    std::thread t([this]() {
        uint32_t count = 0;
        while (NetConnClient::GetInstance().SystemReady() != NETMANAGER_SUCCESS && count < MAX_GET_SERVICE_COUNT) {
            std::this_thread::sleep_for(std::chrono::seconds(WAIT_FOR_SERVICE_TIME_S));
            count++;
        }
        NETMGR_EXT_LOG_W("NetConnClient Get SystemReady count: %{public}u", count);
        if (count > MAX_GET_SERVICE_COUNT) {
            NETMGR_EXT_LOG_E("Connect netconn service fail.");
        } else {
            netFactoryResetCallback_ = (std::make_unique<FactoryResetCallBack>(*this)).release();
            if (netFactoryResetCallback_ != nullptr) {
                int ret = NetConnClient::GetInstance().RegisterNetFactoryResetCallback(netFactoryResetCallback_);
                if (ret != NETMANAGER_SUCCESS) {
                    NETMGR_EXT_LOG_E("RegisterNetFactoryResetCallback ret: %{public}d.", ret);
                }
            } else {
                NETMGR_EXT_LOG_E("netFactoryResetCallback_ is null.");
            }
        }
    });
    std::string threadName = "vpnRegisterFactoryResetCallback";
    pthread_setname_np(t.native_handle(), threadName.c_str());
    t.detach();
}

int32_t NetworkVpnService::SetAlwaysOnVpn(std::string &pkg, bool &enable)
{
    int32_t ret = NetDataShareHelperUtilsIface::Update(ALWAYS_ON_VPN_URI, KEY_ALWAYS_ON_VPN, (enable ? pkg:""));
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("SetAlwaysOnVpn fail: %{public}d", ret);
        return NETMANAGER_ERR_INTERNAL;
    }
    NETMGR_EXT_LOG_I("SetAlwaysOnVpn success: %{public}s", pkg.c_str());

    StartAlwaysOnVpn();

    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnService::GetAlwaysOnVpn(std::string &pkg)
{
    std::string value = "";
    int32_t ret = NetDataShareHelperUtilsIface::Query(ALWAYS_ON_VPN_URI, KEY_ALWAYS_ON_VPN, value);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("GetAlwaysOnVpn fail: %{public}d", ret);
        return NETMANAGER_ERR_INTERNAL;
    }
    pkg = value;
    NETMGR_EXT_LOG_I("GetAlwaysOnVpn success: %{public}s", pkg.c_str());
    return NETMANAGER_EXT_SUCCESS;
}

void NetworkVpnService::StartAlwaysOnVpn()
{
    //first, according the uerId, query local vpn config, if exist apply
    //the config as VPN, if the local VPN is null, query the local kept
    //package if exist will call up the target app to provide the VPN
    std::string alwaysOnBundleName = "";
    int32_t ret = GetAlwaysOnVpn(alwaysOnBundleName);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("StartAlwaysOnVpn fail: %{public}d", ret);
        return;
    }

    if (alwaysOnBundleName != "") {
        if (vpnObj_ != nullptr) {
            std::string pkg = vpnObj_->GetVpnPkg();
            if (pkg != alwaysOnBundleName) {
                NETMGR_EXT_LOG_W("vpn [ %{public}s] exist, destroy vpn first", pkg.c_str());
                DestroyVpn();
            }
        }
        // recover vpn config
        RecoverVpnConfig();
    }
}

void NetworkVpnService::SubscribeCommonEvent()
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_SAVE_MODE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    // 1 means CORE_EVENT_PRIORITY
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<ReceiveMessage>(subscribeInfo, *this);
    uint32_t tryCount = 0;
    bool subscribeResult = false;
    while (!subscribeResult && tryCount <= MAX_RETRY_TIMES) {
        std::this_thread::sleep_for(std::chrono::milliseconds(AGAIN_REGISTER_CALLBACK_INTERVAL));
        subscribeResult = EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_);
        tryCount++;
        NETMGR_EXT_LOG_E("SubscribeCommonEvent try  %{public}d", tryCount);
    }

    if (!subscribeResult) {
        NETMGR_EXT_LOG_E("SubscribeCommonEvent fail: %{public}d", subscribeResult);
    }
}

void NetworkVpnService::ReceiveMessage::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    const auto &action = eventData.GetWant().GetAction();
    const auto &data = eventData.GetData();
    const auto &code = eventData.GetCode();
    NETMGR_EXT_LOG_I("NetVReceiveMessage::OnReceiveEvent(), event:[%{public}s], data:[%{public}s], code:[%{public}d]",
        action.c_str(), data.c_str(), code);
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_POWER_SAVE_MODE_CHANGED) {
        bool isPowerSave = (code == SAVE_MODE || code == LOWPOWER_MODE);
        if (isPowerSave) {
            vpnService_.StartAlwaysOnVpn();
        }
        return;
    }

    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED) {
        vpnService_.StartAlwaysOnVpn();
    }
}

int32_t NetworkVpnService::RegisterBundleName(const std::string &bundleName)
{
    std::unique_lock<std::mutex> locker(netVpnMutex_);
    NETMGR_EXT_LOG_I("VpnService RegisterBundleName %{public}s", bundleName.c_str());
    vpnBundleName_ = bundleName;
    return 0;
}

void NetworkVpnService::VpnHapObserver::OnExtensionStateChanged(const AppExecFwk::AbilityStateData &abilityStateData)
{
    NETMGR_EXT_LOG_I("VPN HAP is OnExtensionStateChanged");
}

void NetworkVpnService::VpnHapObserver::OnProcessCreated(const AppExecFwk::ProcessData &processData)
{
    NETMGR_EXT_LOG_I("VPN HAP is OnProcessCreated");
}

void NetworkVpnService::VpnHapObserver::OnProcessStateChanged(const AppExecFwk::ProcessData &processData)
{
    NETMGR_EXT_LOG_I("VPN HAP is OnProcessStateChanged");
}

void NetworkVpnService::VpnHapObserver::OnProcessDied(const AppExecFwk::ProcessData &processData)
{
    std::unique_lock<std::mutex> locker(vpnService_.netVpnMutex_);
    if ((vpnService_.vpnObj_ != nullptr) && (vpnService_.vpnObj_->Destroy() != NETMANAGER_EXT_SUCCESS)) {
        NETMGR_EXT_LOG_E("destroy vpn failed");
    }
    vpnService_.vpnObj_ = nullptr;
    vpnService_.vpnBundleName_ = "";
    NETMGR_EXT_LOG_I("VPN HAP is OnProcessDied");
}
} // namespace NetManagerStandard
} // namespace OHOS
