/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "ethernet_service.h"

#include <new>
#include <sys/time.h>

#include "ethernet_management.h"
#include "interface_configuration.h"
#include "iremote_object.h"
#include "net_ethernet_base_service.h"
#include "net_manager_center.h"
#include "net_manager_constants.h"
#include "netmanager_base_permission.h"
#include "netmgr_ext_log_wrapper.h"
#include "netsys_controller.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr uint16_t DEPENDENT_SERVICE_NET_CONN_MANAGER = 0x0001;
constexpr uint16_t DEPENDENT_SERVICE_COMMON_EVENT = 0x0002;
constexpr uint16_t DEPENDENT_SERVICE_All = 0x0003;
constexpr const char *NET_ACTIVATE_WORK_THREAD = "POLICY_CALLBACK_WORK_THREAD";
const bool REGISTER_LOCAL_RESULT_ETH =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<EthernetService>::GetInstance().get());
} // namespace

EthernetService::EthernetService() : SystemAbility(COMM_ETHERNET_MANAGER_SYS_ABILITY_ID, true) {}

EthernetService::~EthernetService() = default;

void EthernetService::OnStart()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    NETMGR_EXT_LOG_D("EthernetService::OnStart begin");
    if (state_ == STATE_RUNNING) {
        NETMGR_EXT_LOG_D("EthernetService the state is already running");
        return;
    }
    if (!Init()) {
        NETMGR_EXT_LOG_E("EthernetService init failed");
        return;
    }
    state_ = STATE_RUNNING;
    gettimeofday(&tv, nullptr);
    NETMGR_EXT_LOG_D("EthernetService::OnStart end");
}

void EthernetService::OnStop()
{
    state_ = STATE_STOPPED;
    registerToService_ = false;

    if (policyCallRunner_) {
        policyCallRunner_->Stop();
    }
}

int32_t EthernetService::Dump(int32_t fd, const std::vector<std::u16string> &args)
{
    NETMGR_EXT_LOG_D("Start Dump, fd: %{public}d", fd);
    std::string result;
    ethManagement_.GetDumpInfo(result);
    int32_t ret = dprintf(fd, "%s\n", result.c_str());
    return ret < 0 ? NETMANAGER_EXT_ERR_LOCAL_PTR_NULL : NETMANAGER_EXT_SUCCESS;
}

void EthernetService::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    switch (systemAbilityId) {
        case COMM_NET_CONN_MANAGER_SYS_ABILITY_ID:
            NETMGR_EXT_LOG_D("EthernetService::OnAddSystemAbility Conn");
            dependentServiceState_ |= DEPENDENT_SERVICE_NET_CONN_MANAGER;
            break;
        case COMMON_EVENT_SERVICE_ID:
            NETMGR_EXT_LOG_D("EthernetService::OnAddSystemAbility CES");
            dependentServiceState_ |= DEPENDENT_SERVICE_COMMON_EVENT;
            break;
        default:
            NETMGR_EXT_LOG_D("EthernetService::OnAddSystemAbility unhandled sysabilityId:%{public}d", systemAbilityId);
            break;
    }
    if (dependentServiceState_ == DEPENDENT_SERVICE_All) {
        InitManagement();
    }
}

bool EthernetService::Init()
{
    if (!REGISTER_LOCAL_RESULT_ETH) {
        NETMGR_EXT_LOG_E("EthernetService Register to local sa manager failed");
        return false;
    }
    if (!registerToService_) {
        if (!Publish(DelayedSingleton<EthernetService>::GetInstance().get())) {
            NETMGR_EXT_LOG_E("EthernetService Register to sa manager failed");
            return false;
        }
        registerToService_ = true;
    }
    AddSystemAbilityListener(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID);
    AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
    interfaceStateCallback_ = new (std::nothrow) GlobalInterfaceStateCallback(*this);
    if (interfaceStateCallback_ == nullptr) {
        NETMGR_EXT_LOG_E("allInterfaceStateCallback_ is nullptr");
        return false;
    }
    NetsysController::GetInstance().RegisterCallback(interfaceStateCallback_);
    serviceComm_ = new (std::nothrow) EthernetServiceCommon();
    if (serviceComm_ == nullptr) {
        NETMGR_EXT_LOG_E("serviceComm_ is nullptr");
        return false;
    }
    NetManagerCenter::GetInstance().RegisterEthernetService(serviceComm_);

    if (!policyCallRunner_) {
        policyCallRunner_ = AppExecFwk::EventRunner::Create(NET_ACTIVATE_WORK_THREAD);
    }
    if (!policyCallHandler_) {
        policyCallHandler_ = std::make_shared<AppExecFwk::EventHandler>(policyCallRunner_);
    }
    return true;
}

void EthernetService::InitManagement()
{
    NETMGR_EXT_LOG_D("EthernetService::InitManagement Enter");
    ethManagement_.Init();
}

int32_t EthernetService::GlobalInterfaceStateCallback::OnInterfaceAddressUpdated(const std::string &addr,
                                                                                 const std::string &ifName, int flags,
                                                                                 int scope)
{
    return 0;
}

int32_t EthernetService::GlobalInterfaceStateCallback::OnInterfaceAddressRemoved(const std::string &addr,
                                                                                 const std::string &ifName, int flags,
                                                                                 int scope)
{
    return 0;
}

int32_t EthernetService::GlobalInterfaceStateCallback::OnInterfaceAdded(const std::string &iface)
{
    NETMGR_EXT_LOG_D("iface: %{public}s, added", iface.c_str());
    ethernetService_.NotifyMonitorIfaceCallbackAsync(
        [=](const sptr<InterfaceStateCallback> &callback) { callback->OnInterfaceAdded(iface); });
    return 0;
}

int32_t EthernetService::GlobalInterfaceStateCallback::OnInterfaceRemoved(const std::string &iface)
{
    NETMGR_EXT_LOG_D("iface: %{public}s, removed", iface.c_str());
    ethernetService_.NotifyMonitorIfaceCallbackAsync(
        [=](const sptr<InterfaceStateCallback> &callback) { callback->OnInterfaceRemoved(iface); });
    return 0;
}

int32_t EthernetService::GlobalInterfaceStateCallback::OnInterfaceChanged(const std::string &iface, bool up)
{
    return 0;
}

int32_t EthernetService::GlobalInterfaceStateCallback::OnInterfaceLinkStateChanged(const std::string &ifName, bool up)
{
    NETMGR_EXT_LOG_D("iface: %{public}s, up: %{public}d", ifName.c_str(), up);
    ethernetService_.NotifyMonitorIfaceCallbackAsync(
        [=](const sptr<InterfaceStateCallback> &callback) { callback->OnInterfaceChanged(ifName, up); });
    return 0;
}

int32_t EthernetService::GlobalInterfaceStateCallback::OnRouteChanged(bool updated, const std::string &route,
                                                                      const std::string &gateway,
                                                                      const std::string &ifName)
{
    return 0;
}

int32_t EthernetService::GlobalInterfaceStateCallback::OnDhcpSuccess(NetsysControllerCallback::DhcpResult &dhcpResult)
{
    return 0;
}

int32_t EthernetService::GlobalInterfaceStateCallback::OnBandwidthReachedLimit(const std::string &limitName,
                                                                               const std::string &iface)
{
    return 0;
}

int32_t EthernetService::SetIfaceConfig(const std::string &iface, sptr<InterfaceConfiguration> &ic)
{
    NETMGR_EXT_LOG_D("Set iface: %{public}s config", iface.c_str());
    if (!NetManagerPermission::IsSystemCaller()) {
        NETMGR_EXT_LOG_E("Caller not have sys permission");
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        NETMGR_EXT_LOG_E("EthernetService SetIfaceConfig no js permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }

    return ethManagement_.UpdateDevInterfaceCfg(iface, ic);
}

int32_t EthernetService::GetIfaceConfig(const std::string &iface, sptr<InterfaceConfiguration> &ifaceConfig)
{
    NETMGR_EXT_LOG_D("Get iface: %{public}s config", iface.c_str());
    if (!NetManagerPermission::IsSystemCaller()) {
        NETMGR_EXT_LOG_E("Caller not have sys permission");
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::GET_NETWORK_INFO)) {
        NETMGR_EXT_LOG_E("EthernetService GetIfaceConfig no js permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }

    return ethManagement_.GetDevInterfaceCfg(iface, ifaceConfig);
}

int32_t EthernetService::IsIfaceActive(const std::string &iface, int32_t &activeStatus)
{
    NETMGR_EXT_LOG_D("Get iface: %{public}s is active", iface.c_str());
    if (!NetManagerPermission::IsSystemCaller()) {
        NETMGR_EXT_LOG_E("Caller not have sys permission");
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::GET_NETWORK_INFO)) {
        NETMGR_EXT_LOG_E("EthernetService IsIfaceActive no js permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }

    return ethManagement_.IsIfaceActive(iface, activeStatus);
}

int32_t EthernetService::GetAllActiveIfaces(std::vector<std::string> &activeIfaces)
{
    if (!NetManagerPermission::IsSystemCaller()) {
        NETMGR_EXT_LOG_E("Caller not have sys permission");
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::GET_NETWORK_INFO)) {
        NETMGR_EXT_LOG_E("EthernetService GetAllActiveIfaces no js permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }

    return ethManagement_.GetAllActiveIfaces(activeIfaces);
}

int32_t EthernetService::ResetFactory()
{
    if (!NetManagerPermission::IsSystemCaller()) {
        NETMGR_EXT_LOG_E("Caller not have sys permission");
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        NETMGR_EXT_LOG_E("EthernetService GetAllActiveIfaces no js permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }

    return ethManagement_.ResetFactory();
}

int32_t EthernetService::RegisterIfacesStateChanged(const sptr<InterfaceStateCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_EXT_LOG_E("Register interface callback failed");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    if (!NetManagerPermission::IsSystemCaller()) {
        NETMGR_EXT_LOG_E("Caller not have sys permission");
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::GET_NETWORK_INFO)) {
        NETMGR_EXT_LOG_E("RegisterIfacesStateChanged no permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return RegisterMonitorIfaceCallbackAsync(callback);
}

int32_t EthernetService::UnregisterIfacesStateChanged(const sptr<InterfaceStateCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_EXT_LOG_E("Unregister interface callback failed");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    if (!NetManagerPermission::IsSystemCaller()) {
        NETMGR_EXT_LOG_E("Caller not have sys permission");
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::GET_NETWORK_INFO)) {
        NETMGR_EXT_LOG_E("RegisterIfacesStateChanged no permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return UnregisterMonitorIfaceCallbackAsync(callback);
}

int32_t EthernetService::SetInterfaceUp(const std::string &iface)
{
    NETMGR_EXT_LOG_D("Set interface: %{public}s up", iface.c_str());
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        NETMGR_EXT_LOG_E("EthernetService SetInterfaceUp no permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return NetsysController::GetInstance().SetInterfaceUp(iface);
}

int32_t EthernetService::SetInterfaceDown(const std::string &iface)
{
    NETMGR_EXT_LOG_D("Set interface: %{public}s down", iface.c_str());
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        NETMGR_EXT_LOG_E("EthernetService SetInterfaceDown no permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return NetsysController::GetInstance().SetInterfaceDown(iface);
}

int32_t EthernetService::GetInterfaceConfig(const std::string &iface, OHOS::nmd::InterfaceConfigurationParcel &config)
{
    NETMGR_EXT_LOG_D("Get interface: %{public}s config", iface.c_str());
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        NETMGR_EXT_LOG_E("EthernetService GetInterfaceConfig no permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    config.ifName = iface;
    return NetsysController::GetInstance().GetInterfaceConfig(config);
}

int32_t EthernetService::SetInterfaceConfig(const std::string &iface, OHOS::nmd::InterfaceConfigurationParcel &cfg)
{
    NETMGR_EXT_LOG_D("Set interface: %{public}s config", iface.c_str());
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        NETMGR_EXT_LOG_E("EthernetService SetInterfaceConfig no permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    cfg.ifName = iface;
    return NetsysController::GetInstance().SetInterfaceConfig(cfg);
}

int32_t EthernetService::RegisterMonitorIfaceCallbackAsync(const sptr<InterfaceStateCallback> &callback)
{
    int32_t ret = NETMANAGER_EXT_ERR_OPERATION_FAILED;
    if (policyCallHandler_) {
        policyCallHandler_->PostSyncTask([this, &callback, &ret]() {
            for (auto iterCb = monitorIfaceCallbacks_.begin(); iterCb != monitorIfaceCallbacks_.end(); iterCb++) {
                if ((*iterCb)->AsObject().GetRefPtr() == callback->AsObject().GetRefPtr()) {
                    NETMGR_EXT_LOG_D("Register interface callback failed, callback already exists");
                    ret = NETMANAGER_EXT_ERR_OPERATION_FAILED;
                    return;
                }
            }
            monitorIfaceCallbacks_.push_back(callback);
            NETMGR_EXT_LOG_D("Register interface callback success");
            ret = NETMANAGER_EXT_SUCCESS;
        });
    }
    return ret;
}

int32_t EthernetService::UnregisterMonitorIfaceCallbackAsync(const sptr<InterfaceStateCallback> &callback)
{
    int32_t ret = NETMANAGER_EXT_ERR_OPERATION_FAILED;
    if (policyCallHandler_) {
        policyCallHandler_->PostSyncTask([this, &callback, &ret]() {
            for (auto iterCb = monitorIfaceCallbacks_.begin(); iterCb != monitorIfaceCallbacks_.end(); iterCb++) {
                if ((*iterCb)->AsObject().GetRefPtr() == callback->AsObject().GetRefPtr()) {
                    monitorIfaceCallbacks_.erase(iterCb);
                    NETMGR_EXT_LOG_D("Unregister interface callback success.");
                    ret = NETMANAGER_EXT_SUCCESS;
                    return;
                }
            }
            NETMGR_EXT_LOG_E("Unregister interface callback is doesnot exist.");
            ret = NETMANAGER_EXT_ERR_OPERATION_FAILED;
        });
    }
    return ret;
}

void EthernetService::NotifyMonitorIfaceCallbackAsync(OnFunctionT onFunction)
{
    if (policyCallHandler_) {
        policyCallHandler_->PostSyncTask([this, &onFunction]() {
            std::for_each(monitorIfaceCallbacks_.begin(), monitorIfaceCallbacks_.end(), onFunction);
        });
    }
}
} // namespace NetManagerStandard
} // namespace OHOS
