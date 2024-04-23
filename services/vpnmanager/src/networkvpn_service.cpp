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
#include <string>
#include <fstream>
#include <thread>

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
    SystemAbility::MakeAndRegisterAbility(&NetworkVpnService::GetInstance());

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
    networkVpnServiceFfrtQueue_ = std::make_shared<ffrt::queue>("NetworkVpnService");
    if (!networkVpnServiceFfrtQueue_) {
        NETMGR_EXT_LOG_E("FFRT Create Fail");
        return false;
    }
    if (!isServicePublished_) {
        if (!Publish(&NetworkVpnService::GetInstance())) {
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
    if (!vpnService_.networkVpnServiceFfrtQueue_) {
        NETMGR_EXT_LOG_E("FFRT Create Fail");
        return;
    }
    std::function<void()> OnVpnConnStateChangedFunction = [this, &state]() {
        std::for_each(vpnService_.vpnEventCallbacks_.begin(), vpnService_.vpnEventCallbacks_.end(),
                      [&state](const auto &callback) {
                          callback->OnVpnStateChanged((VpnConnectState::VPN_CONNECTED == state) ? true : false);
                      });
    };
    ffrt::task_handle OnVpnConnStateTask =
        vpnService_.networkVpnServiceFfrtQueue_->submit_h(OnVpnConnStateChangedFunction);
    vpnService_.networkVpnServiceFfrtQueue_->wait(OnVpnConnStateTask);
}

void NetworkVpnService::OnVpnMultiUserSetUp()
{
    NETMGR_EXT_LOG_I("user multiple execute set up.");
    if (!networkVpnServiceFfrtQueue_) {
        NETMGR_EXT_LOG_E("FFRT Create Fail");
        return;
    }
    std::function<void()> OnVpnMultiUserSetUpFunction = [this]() {
        std::for_each(vpnEventCallbacks_.begin(), vpnEventCallbacks_.end(),
                      [](const auto &callback) { callback->OnVpnMultiUserSetUp(); });
    };
    ffrt::task_handle OnVpnMultiUserSetUpTask =
        networkVpnServiceFfrtQueue_->submit_h(OnVpnMultiUserSetUpFunction);
    networkVpnServiceFfrtQueue_->wait(OnVpnMultiUserSetUpTask);
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

void NetworkVpnService::ConvertStringToConfig(sptr<VpnConfig> &vpnCfg, const cJSON* const doc)
{
    cJSON *dnsAddr = cJSON_GetObjectItem(doc, "dnsAddresses");
    if (dnsAddr != nullptr && cJSON_IsArray(dnsAddr)) {
        for (uint32_t i = 0; i < cJSON_GetArraySize(dnsAddr); i++) {
            cJSON *item = cJSON_GetArrayItem(dnsAddr, i);
            if (cJSON_IsString(item)) {
                std::string mem = cJSON_GetStringValue(item);
                NETMGR_EXT_LOG_D("dnsAddr = %{public}s", mem.c_str());
                vpnCfg->dnsAddresses_.push_back(mem);
            }
        }
    }
    cJSON *sDomain = cJSON_GetObjectItem(doc, "searchDomains");
    if (sDomain != nullptr && cJSON_IsArray(sDomain)) {
        for (uint32_t i = 0; i < cJSON_GetArraySize(sDomain); i++) {
            cJSON *item = cJSON_GetArrayItem(sDomain, i);
            if (cJSON_IsString(item)) {
                std::string mem = cJSON_GetStringValue(item);
                NETMGR_EXT_LOG_D("sDomain = %{public}s", mem.c_str());
                vpnCfg->searchDomains_.push_back(mem);
            }
        }
    }
    cJSON *acceptApp = cJSON_GetObjectItem(doc, "acceptedApplications");
    if (acceptApp != nullptr && cJSON_IsArray(acceptApp)) {
        for (uint32_t i = 0; i < cJSON_GetArraySize(acceptApp); i++) {
            cJSON *item = cJSON_GetArrayItem(acceptApp, i);
            if (cJSON_IsString(item)) {
                std::string mem = cJSON_GetStringValue(item);
                NETMGR_EXT_LOG_D("acceptApp = %{public}s", mem.c_str());
                vpnCfg->acceptedApplications_.push_back(mem);
            }
        }
    }
    cJSON *refusedApp = cJSON_GetObjectItem(doc, "refusedApplications");
    if (refusedApp != nullptr && cJSON_IsArray(refusedApp)) {
        for (uint32_t i = 0; i < cJSON_GetArraySize(refusedApp); i++) {
            cJSON *item = cJSON_GetArrayItem(refusedApp, i);
            if (cJSON_IsString(item)) {
                std::string mem = cJSON_GetStringValue(item);
                NETMGR_EXT_LOG_D("refusedApp = %{public}s", mem.c_str());
                vpnCfg->refusedApplications_.push_back(mem);
            }
        }
    }
}

void NetworkVpnService::ConvertNetAddrToConfig(INetAddr& tmp, const cJSON* const mem)
{
    cJSON *type = cJSON_GetObjectItem(mem, "type");
    if (type != nullptr && cJSON_IsNumber(type)) {
        tmp.type_ = static_cast<int32_t>(cJSON_GetNumberValue(type));
        NETMGR_EXT_LOG_D("type = %{public}d", tmp.type_);
    }
    cJSON *family = cJSON_GetObjectItem(mem, "family");
    if (family != nullptr && cJSON_IsNumber(family)) {
        tmp.family_ = static_cast<int32_t>(cJSON_GetNumberValue(family));
        NETMGR_EXT_LOG_D("family = %{public}d", tmp.family_);
    }
    cJSON *prefixlen = cJSON_GetObjectItem(mem, "prefixlen");
    if (prefixlen != nullptr && cJSON_IsNumber(prefixlen)) {
        tmp.prefixlen_ = static_cast<int32_t>(cJSON_GetNumberValue(prefixlen));
        NETMGR_EXT_LOG_D("prefixlen = %{public}d", tmp.prefixlen_);
    }
    cJSON *address = cJSON_GetObjectItem(mem, "address");
    if (address != nullptr && cJSON_IsString(address)) {
        tmp.address_ = cJSON_GetStringValue(address);
        NETMGR_EXT_LOG_D("address = %{public}s", tmp.address_.c_str());
    }
    cJSON *netMask = cJSON_GetObjectItem(mem, "netMask");
    if (netMask != nullptr && cJSON_IsString(netMask)) {
        tmp.netMask_ = cJSON_GetStringValue(netMask);
        NETMGR_EXT_LOG_D("netMask = %{public}s", tmp.netMask_.c_str());
    }
    cJSON *hostName = cJSON_GetObjectItem(mem, "hostName");
    if (hostName != nullptr && cJSON_IsString(hostName)) {
        tmp.hostName_ = cJSON_GetStringValue(hostName);
        NETMGR_EXT_LOG_D("hostName = %{public}s", tmp.hostName_.c_str());
    }
    cJSON *port = cJSON_GetObjectItem(mem, "port");
    if (port != nullptr && cJSON_IsNumber(port)) {
        tmp.port_ = static_cast<int32_t>(cJSON_GetNumberValue(port));
        NETMGR_EXT_LOG_D("port = %{public}d", tmp.port_);
    }
}

void NetworkVpnService::ConvertVecAddrToConfig(sptr<VpnConfig> &vpnCfg, const cJSON* const doc)
{
    cJSON *addresses = cJSON_GetObjectItem(doc, "addresses");
    if (addresses != nullptr && cJSON_IsArray(addresses)) {
        uint32_t itemSize = cJSON_GetArraySize(addresses);
        for (uint32_t i = 0; i < itemSize; i++) {
            cJSON *item = cJSON_GetArrayItem(addresses, i);
            if (cJSON_IsObject(item)) {
                INetAddr tmp;
                ConvertNetAddrToConfig(tmp, item);
                vpnCfg->addresses_.push_back(tmp);
            }
        }
    }
}

void NetworkVpnService::ConvertRouteToConfig(Route& tmp, const cJSON* const mem)
{
    cJSON *iface = cJSON_GetObjectItem(mem, "iface");
    if (iface != nullptr && cJSON_IsString(iface)) {
        tmp.iface_ = cJSON_GetStringValue(iface);
        NETMGR_EXT_LOG_D("iface = %{public}s", tmp.iface_.c_str());
    }
    cJSON *rtnType = cJSON_GetObjectItem(mem, "rtnType");
    if (rtnType != nullptr && cJSON_IsNumber(rtnType)) {
        tmp.rtnType_ = cJSON_GetNumberValue(rtnType);
        NETMGR_EXT_LOG_D("rtnType = %{public}d", tmp.rtnType_);
    }
    cJSON *mtu = cJSON_GetObjectItem(mem, "mtu");
    if (mtu != nullptr && cJSON_IsNumber(mtu)) {
        tmp.mtu_ = cJSON_GetNumberValue(mtu);
        NETMGR_EXT_LOG_D("mtu = %{public}d", tmp.mtu_);
    }
    cJSON *isHost = cJSON_GetObjectItem(mem, "isHost");
    if (isHost != nullptr && cJSON_IsBool(isHost)) {
        tmp.isHost_ = cJSON_IsTrue(isHost) ? true : false;
        NETMGR_EXT_LOG_D("isHost = %{public}d", tmp.isHost_);
    }
    cJSON *hasGateway = cJSON_GetObjectItem(mem, "hasGateway");
    if (hasGateway != nullptr && cJSON_IsBool(hasGateway)) {
        tmp.hasGateway_ = cJSON_IsTrue(hasGateway) ? true : false;
        NETMGR_EXT_LOG_D("hasGateway_ = %{public}d", tmp.hasGateway_);
    }
    cJSON *isDefaultRoute = cJSON_GetObjectItem(mem, "isDefaultRoute");
    if (isDefaultRoute != nullptr && cJSON_IsBool(isDefaultRoute)) {
        tmp.isDefaultRoute_ = cJSON_IsTrue(isDefaultRoute) ? true : false;
        NETMGR_EXT_LOG_D("isDefaultRoute_ = %{public}d", tmp.isDefaultRoute_);
    }
    cJSON *destination = cJSON_GetObjectItem(mem, "destination");
    if (destination != nullptr && cJSON_IsObject(destination)) {
        INetAddr tmpINet;
        ConvertNetAddrToConfig(tmpINet, destination);
        tmp.destination_ = tmpINet;
    }
    cJSON *gateway = cJSON_GetObjectItem(mem, "gateway");
    if (gateway != nullptr && cJSON_IsObject(gateway)) {
        INetAddr tmpINet;
        ConvertNetAddrToConfig(tmpINet, gateway);
        tmp.gateway_ = tmpINet;
    }
}

void NetworkVpnService::ConvertVecRouteToConfig(sptr<VpnConfig> &vpnCfg, const cJSON* const doc)
{
    cJSON *routes = cJSON_GetObjectItem(doc, "routes");
    if (routes != nullptr && cJSON_IsArray(routes)) {
        uint32_t itemSize = cJSON_GetArraySize(routes);
        for (uint32_t i = 0; i < itemSize; i++) {
            cJSON *item = cJSON_GetArrayItem(routes, i);
            if (cJSON_IsObject(item)) {
                Route tmp;
                ConvertRouteToConfig(tmp, item);
                vpnCfg->routes_.push_back(tmp);
            }
        }
    }
}

void NetworkVpnService::ParseJsonToConfig(sptr<VpnConfig> &vpnCfg, const std::string& jsonString)
{
    cJSON *doc = cJSON_Parse(jsonString.c_str());
    if (doc == nullptr) {
        NETMGR_EXT_LOG_E("jsonString parse failed!");
        return;
    }
    cJSON *mtu = cJSON_GetObjectItem(doc, "mtu");
    if (mtu != nullptr && cJSON_IsNumber(mtu)) {
        vpnCfg->mtu_ = cJSON_GetNumberValue(mtu);
        NETMGR_EXT_LOG_D("mtu = %{public}d", vpnCfg->mtu_);
    }
    cJSON *isAcceptIPv4 = cJSON_GetObjectItem(doc, "isAcceptIPv4");
    if (isAcceptIPv4 != nullptr && cJSON_IsBool(isAcceptIPv4)) {
        vpnCfg->isAcceptIPv4_ = cJSON_IsTrue(isAcceptIPv4);
        NETMGR_EXT_LOG_D("isAcceptIPv4 = %{public}d", vpnCfg->isAcceptIPv4_);
    }
    cJSON *isAcceptIPv6 = cJSON_GetObjectItem(doc, "isAcceptIPv6");
    if (isAcceptIPv6 != nullptr && cJSON_IsBool(isAcceptIPv6)) {
        vpnCfg->isAcceptIPv6_ = cJSON_IsTrue(isAcceptIPv6);
        NETMGR_EXT_LOG_D("isAcceptIPv6 = %{public}d", vpnCfg->isAcceptIPv6_);
    }
    cJSON *isLegacy = cJSON_GetObjectItem(doc, "isLegacy");
    if (isLegacy != nullptr && cJSON_IsBool(isLegacy)) {
        vpnCfg->isLegacy_ = cJSON_IsTrue(isLegacy);
        NETMGR_EXT_LOG_D("isLegacy = %{public}d", vpnCfg->isLegacy_);
    }
    cJSON *isMetered = cJSON_GetObjectItem(doc, "isMetered");
    if (isMetered != nullptr && cJSON_IsBool(isMetered)) {
        vpnCfg->isMetered_ = cJSON_IsTrue(isMetered);
        NETMGR_EXT_LOG_D("isMetered = %{public}d", vpnCfg->isMetered_);
    }
    cJSON *isBlocking = cJSON_GetObjectItem(doc, "isBlocking");
    if (isBlocking != nullptr && cJSON_IsBool(isBlocking)) {
        vpnCfg->isBlocking_ = cJSON_IsTrue(isBlocking);
        NETMGR_EXT_LOG_D("isBlocking = %{public}d", vpnCfg->isBlocking_);
    }

    ConvertStringToConfig(vpnCfg, doc);

    ConvertVecAddrToConfig(vpnCfg, doc);

    ConvertVecRouteToConfig(vpnCfg, doc);

    cJSON_Delete(doc);
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

void NetworkVpnService::ConvertNetAddrToJson(const INetAddr& netAddr, cJSON* jInetAddr)
{
    cJSON_AddItemToObject(jInetAddr, "type", cJSON_CreateNumber(netAddr.type_));
    cJSON_AddItemToObject(jInetAddr, "family", cJSON_CreateNumber(netAddr.family_));
    cJSON_AddItemToObject(jInetAddr, "prefixlen", cJSON_CreateNumber(netAddr.prefixlen_));
    cJSON_AddItemToObject(jInetAddr, "address", cJSON_CreateString(netAddr.address_.c_str()));
    cJSON_AddItemToObject(jInetAddr, "netMask", cJSON_CreateString(netAddr.netMask_.c_str()));
    cJSON_AddItemToObject(jInetAddr, "hostName", cJSON_CreateString(netAddr.hostName_.c_str()));
    cJSON_AddItemToObject(jInetAddr, "port", cJSON_CreateNumber(netAddr.port_));
}

void NetworkVpnService::ConvertVecRouteToJson(const std::vector<Route>& routes, cJSON* jVecRoutes)
{
    for (const auto& mem : routes) {
        cJSON *jRoute = cJSON_CreateObject();
        cJSON_AddItemToObject(jRoute, "iface", cJSON_CreateString(mem.iface_.c_str()));
        cJSON *jDestination = cJSON_CreateObject();
        ConvertNetAddrToJson(mem.destination_, jDestination);
        cJSON_AddItemToObject(jRoute, "destination", jDestination);
        cJSON *jGateway = cJSON_CreateObject();
        ConvertNetAddrToJson(mem.gateway_, jGateway);
        cJSON_AddItemToObject(jRoute, "gateway", jGateway);
        cJSON_AddItemToObject(jRoute, "rtnType", cJSON_CreateNumber(mem.rtnType_));
        cJSON_AddItemToObject(jRoute, "mtu", cJSON_CreateNumber(mem.mtu_));
        cJSON_AddItemToObject(jRoute, "isHost", cJSON_CreateBool(mem.isHost_));
        cJSON_AddItemToObject(jRoute, "hasGateway", cJSON_CreateBool(mem.hasGateway_));
        cJSON_AddItemToObject(jRoute, "isDefaultRoute", cJSON_CreateBool(mem.isDefaultRoute_));
        cJSON_AddItemToArray(jVecRoutes, jRoute);
    }
}

void NetworkVpnService::ParseConfigToJson(const sptr<VpnConfig> &vpnCfg, std::string& jsonString)
{
    cJSON *root = cJSON_CreateObject();
    cJSON *jVecAddrs = cJSON_CreateArray();
    for (const auto& mem : vpnCfg->addresses_) {
        cJSON *jInetAddr = cJSON_CreateObject();
        ConvertNetAddrToJson(mem, jInetAddr);
        cJSON_AddItemToArray(jVecAddrs, jInetAddr);
    }
    cJSON_AddItemToObject(root, "addresses", jVecAddrs);

    cJSON *jVecRoutes = cJSON_CreateArray();
    ConvertVecRouteToJson(vpnCfg->routes_, jVecRoutes);
    cJSON_AddItemToObject(root, "routes", jVecRoutes);

    cJSON_AddItemToObject(root, "mtu", cJSON_CreateNumber(vpnCfg->mtu_));
    cJSON_AddItemToObject(root, "isAcceptIPv4", cJSON_CreateBool(vpnCfg->isAcceptIPv4_));
    cJSON_AddItemToObject(root, "isAcceptIPv6", cJSON_CreateBool(vpnCfg->isAcceptIPv6_));
    cJSON_AddItemToObject(root, "isLegacy", cJSON_CreateBool(vpnCfg->isLegacy_));
    cJSON_AddItemToObject(root, "isMetered", cJSON_CreateBool(vpnCfg->isMetered_));
    cJSON_AddItemToObject(root, "isBlocking", cJSON_CreateBool(vpnCfg->isBlocking_));

    cJSON *jVecDnsAddrs = cJSON_CreateArray();
    for (const auto& mem : vpnCfg->dnsAddresses_) {
        cJSON_AddItemToArray(jVecDnsAddrs, cJSON_CreateString(mem.c_str()));
    }
    cJSON_AddItemToObject(root, "dnsAddresses", jVecDnsAddrs);

    cJSON *jVecDomains = cJSON_CreateArray();
    for (const auto& mem : vpnCfg->searchDomains_) {
        cJSON_AddItemToArray(jVecDomains, cJSON_CreateString(mem.c_str()));
    }
    cJSON_AddItemToObject(root, "searchDomains", jVecDomains);

    cJSON *jVecAcceptApp = cJSON_CreateArray();
    for (const auto& mem : vpnCfg->acceptedApplications_) {
        cJSON_AddItemToArray(jVecAcceptApp, cJSON_CreateString(mem.c_str()));
    }
    cJSON_AddItemToObject(root, "acceptedApplications", jVecAcceptApp);

    cJSON *jVecRefuseApp = cJSON_CreateArray();
    for (const auto& mem : vpnCfg->refusedApplications_) {
        cJSON_AddItemToArray(jVecRefuseApp, cJSON_CreateString(mem.c_str()));
    }
    cJSON_AddItemToObject(root, "refusedApplications", jVecRefuseApp);
    char *str = cJSON_Print(root);
    if (str == nullptr) {
        cJSON_Delete(root);
        return;
    }
    jsonString = str;
    cJSON_Delete(root);
    free(str);
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
            NETMGR_EXT_LOG_W("vpn using by other user");
            return NETWORKVPN_ERROR_VPN_EXIST;
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
    if (!networkVpnServiceFfrtQueue_) {
        NETMGR_EXT_LOG_E("FFRT Create Fail");
        return ret;
    }
    ffrt::task_handle RegisterVpnEventTask = networkVpnServiceFfrtQueue_->submit_h([this, &callback, &ret]() {
        ret = SyncRegisterVpnEvent(callback);
    });
    networkVpnServiceFfrtQueue_->wait(RegisterVpnEventTask);
    return ret;
}

int32_t NetworkVpnService::UnregisterVpnEvent(const sptr<IVpnEventCallback> callback)
{
    int32_t ret = NETMANAGER_EXT_ERR_OPERATION_FAILED;
    if (!networkVpnServiceFfrtQueue_) {
        NETMGR_EXT_LOG_E("FFRT Create Fail");
        return ret;
    }
    ffrt::task_handle UnregisterVpnEventTask = networkVpnServiceFfrtQueue_->submit_h([this, &callback, &ret]() {
        ret = SyncUnregisterVpnEvent(callback);
    });
    networkVpnServiceFfrtQueue_->wait(UnregisterVpnEventTask);
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
