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

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t MAX_CALLBACK_COUNT = 128;
constexpr const char *NET_ACTIVATE_WORK_THREAD = "VPN_CALLBACK_WORK_THREAD";

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

    if (!vpnConnCallback_) {
        vpnConnCallback_ = std::make_shared<VpnConnStateCb>(*this);
    }

    if (!policyCallHandler_) {
        policyCallRunner_ = AppExecFwk::EventRunner::Create(NET_ACTIVATE_WORK_THREAD);
        policyCallHandler_ = std::make_shared<AppExecFwk::EventHandler>(policyCallRunner_);
    }
    return true;
}

void NetworkVpnService::GetDumpMessage(std::string &message)
{
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

int32_t NetworkVpnService::SetUpVpn(const sptr<VpnConfig> &config)
{
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
    NETMGR_EXT_LOG_I("NetworkVpnService SetUp");
    return vpnObj_->SetUp();
}

int32_t NetworkVpnService::Protect()
{
    /*
     * Only permission verification is performed and
     * the protected socket implements fwmark_service in the netsys process.
     */
    NETMGR_EXT_LOG_I("Protect vpn tunnel successfully.");
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnService::DestroyVpn()
{
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

int32_t NetworkVpnService::CreateVpnConnection()
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

} // namespace NetManagerStandard
} // namespace OHOS
