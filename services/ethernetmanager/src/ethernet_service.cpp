/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "mac_address_info.h"
#include "interface_configuration.h"
#include "iremote_object.h"
#include "net_ethernet_base_service.h"
#include "net_manager_center.h"
#include "net_manager_constants.h"
#include "netmanager_base_permission.h"
#include "netmgr_ext_log_wrapper.h"
#include "netsys_controller.h"
#include "system_ability_definition.h"
#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
#include "eth_eap_auto_auth_controller.h"
#include "eth_eap_cert_store.h"
#include "eth_eap_config_store.h"
#include "eth_eap_state_manager.h"
#endif

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr uint16_t DEPENDENT_SERVICE_NET_CONN_MANAGER = 0x0001;
constexpr uint16_t DEPENDENT_SERVICE_COMMON_EVENT = 0x0002;
constexpr uint16_t DEPENDENT_SERVICE_All = 0x0003;
constexpr const char *NET_ACTIVATE_WORK_THREAD = "POLICY_CALLBACK_WORK_THREAD";
constexpr const char *ETH_PREFIX = "eth";
constexpr const char *ENTERPRISE_MANAGE_EAP = "ohos.permission.ENTERPRISE_MANAGE_EAP";
const bool REGISTER_LOCAL_RESULT_ETH =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<EthernetService>::GetInstance().get());
} // namespace

EthernetService::EthernetService() : SystemAbility(COMM_ETHERNET_MANAGER_SYS_ABILITY_ID, true)
{
    ethManagement_ = std::make_shared<EthernetManagement>();
}

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
    ethernetServiceFfrtQueue_.reset();
}

int32_t EthernetService::Dump(int32_t fd, const std::vector<std::u16string> &args)
{
    NETMGR_EXT_LOG_D("Start Dump, fd: %{public}d", fd);
    std::string result;
    ethManagement_->GetDumpInfo(result);
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
    interfaceStateCallback_ = new (std::nothrow) GlobalInterfaceStateCallback(*this);
    if (interfaceStateCallback_ == nullptr) {
        NETMGR_EXT_LOG_E("allInterfaceStateCallback_ is nullptr");
        return false;
    }
    serviceComm_ = new (std::nothrow) EthernetServiceCommon();
    if (serviceComm_ == nullptr) {
        NETMGR_EXT_LOG_E("serviceComm_ is nullptr");
        return false;
    }
    AddSystemAbilityListener(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID);
    AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
    NetsysController::GetInstance().RegisterCallback(interfaceStateCallback_);
    NetManagerCenter::GetInstance().RegisterEthernetService(serviceComm_);
    ethernetServiceFfrtQueue_ = std::make_shared<ffrt::queue>("EthernetService");
    return true;
}

void EthernetService::InitManagement()
{
    NETMGR_EXT_LOG_D("EthernetService::InitManagement Enter");
    ethManagement_->Init();
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
#if defined(NET_EXTENSIBLE_AUTHENTICATION) || defined(NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE)
    if (up && (ifName.find(ETH_PREFIX) != std::string::npos)) {
        NetEapHandler::GetInstance().SetEthEapIfName(ifName);
    }
#endif
#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
    if (up && (ifName.find(ETH_PREFIX) != std::string::npos)) {
        NETMGR_EXT_LOG_I("eth link up %{public}s, trigger auto auth", ifName.c_str());
        EthEapAutoAuthController::GetInstance().OnLinkUp(ifName);
    }
#endif // NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
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

int32_t EthernetService::GetMacAddress(std::vector<MacAddressInfo> &macAddrList)
{
    if (!NetManagerPermission::CheckPermission(Permission::GET_ETHERNET_LOCAL_MAC)) {
        NETMGR_EXT_LOG_E("EthernetService GetMacAddress no js permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }

    return ethManagement_->GetMacAddress(macAddrList);
}

int32_t EthernetService::SetIfaceConfig(const std::string &iface, const sptr<InterfaceConfiguration> &ic)
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

    return ethManagement_->UpdateDevInterfaceCfg(iface, ic);
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

    return ethManagement_->GetDevInterfaceCfg(iface, ifaceConfig);
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

    return ethManagement_->IsIfaceActive(iface, activeStatus);
}

#ifdef FEATURE_GET_IFACE_SUPPLIER_ID
int32_t EthernetService::GetIfaceSupplierId(const std::string &iface, uint32_t &supplierId)
{
    if (!NetManagerPermission::IsSystemCaller()) {
        NETMGR_EXT_LOG_E("Caller not have sys permission");
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::GET_NETWORK_INFO)) {
        NETMGR_EXT_LOG_E("EthernetService GetAllActiveIfaces no js permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }

    return ethManagement_->GetIfaceSupplierId(iface, supplierId);
}
#endif // FEATURE_GET_IFACE_SUPPLIER_ID

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

    return ethManagement_->GetAllActiveIfaces(activeIfaces);
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

    return ethManagement_->ResetFactory();
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
    if (!NetManagerPermission::IsSystemCaller()) {
        NETMGR_EXT_LOG_E("SetInterfaceUp Caller no sys permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    NETMGR_EXT_LOG_D("Set interface: %{public}s up", iface.c_str());
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        NETMGR_EXT_LOG_E("EthernetService SetInterfaceUp no permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return NetsysController::GetInstance().SetInterfaceUp(iface);
}

int32_t EthernetService::SetInterfaceDown(const std::string &iface)
{
    if (!NetManagerPermission::IsSystemCaller()) {
        NETMGR_EXT_LOG_E("SetInterfaceDown Caller no sys permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    NETMGR_EXT_LOG_D("Set interface: %{public}s down", iface.c_str());
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        NETMGR_EXT_LOG_E("EthernetService SetInterfaceDown no permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return NetsysController::GetInstance().SetInterfaceDown(iface);
}

int32_t EthernetService::GetInterfaceConfig(const std::string &iface, ConfigurationParcelIpc &cfgIpc)
{
    if (!NetManagerPermission::IsSystemCaller()) {
        NETMGR_EXT_LOG_E("GetInterfaceConfig Caller no sys permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    NETMGR_EXT_LOG_D("Get interface: %{public}s config", iface.c_str());
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        NETMGR_EXT_LOG_E("EthernetService GetInterfaceConfig no permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    OHOS::nmd::InterfaceConfigurationParcel config;
    config.ifName = iface;
    int32_t ret = NetsysController::GetInstance().GetInterfaceConfig(config);
    ConfigurationParcelIpc::ConvertNmdToEtherConfigParcel(cfgIpc, config);
    return ret;
}

int32_t EthernetService::SetInterfaceConfig(const std::string &iface, const ConfigurationParcelIpc &cfgIpc)
{
    if (!NetManagerPermission::IsSystemCaller()) {
        NETMGR_EXT_LOG_E("SetInterfaceConfig Caller no sys permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    NETMGR_EXT_LOG_D("Set interface: %{public}s config", iface.c_str());
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        NETMGR_EXT_LOG_E("EthernetService SetInterfaceConfig no permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    OHOS::nmd::InterfaceConfigurationParcel config;
    ConfigurationParcelIpc::ConvertEtherConfigParcelToNmd(cfgIpc, config);
    config.ifName = iface;
    return NetsysController::GetInstance().SetInterfaceConfig(config);
}

int32_t EthernetService::RegisterMonitorIfaceCallbackAsync(const sptr<InterfaceStateCallback> &callback)
{
    int32_t ret = NETMANAGER_EXT_ERR_OPERATION_FAILED;
    if (!ethernetServiceFfrtQueue_) {
        NETMGR_EXT_LOG_E("FFRT Init Fail");
        return ret;
    }
    ffrt::task_handle RegisterMonitorIfaceTask = ethernetServiceFfrtQueue_->submit_h([this, &callback, &ret]() {
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
    }, ffrt::task_attr().name("RegisterMonitorIfaceCallbackAsync"));
    ethernetServiceFfrtQueue_->wait(RegisterMonitorIfaceTask);
    return ret;
}

int32_t EthernetService::UnregisterMonitorIfaceCallbackAsync(const sptr<InterfaceStateCallback> &callback)
{
    int32_t ret = NETMANAGER_EXT_ERR_OPERATION_FAILED;
    if (!ethernetServiceFfrtQueue_) {
        NETMGR_EXT_LOG_E("FFRT Init Fail");
        return ret;
    }
    ffrt::task_handle UnregisterMonitorIfaceTask = ethernetServiceFfrtQueue_->submit_h([this, &callback, &ret]() {
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
    }, ffrt::task_attr().name("UnregisterMonitorIfaceCallbackAsync"));
    ethernetServiceFfrtQueue_->wait(UnregisterMonitorIfaceTask);
    return ret;
}

void EthernetService::NotifyMonitorIfaceCallbackAsync(OnFunctionT onFunction)
{
    if (!ethernetServiceFfrtQueue_) {
        NETMGR_EXT_LOG_E("FFRT Init Fail");
        return;
    }
    ffrt::task_handle NotifyMonitorIfaceTask_ = ethernetServiceFfrtQueue_->submit_h([this, &onFunction]() {
        std::for_each(monitorIfaceCallbacks_.begin(), monitorIfaceCallbacks_.end(), onFunction);
    }, ffrt::task_attr().name("NotifyMonitorIfaceCallbackAsync"));
    ethernetServiceFfrtQueue_->wait(NotifyMonitorIfaceTask_);
}

int32_t EthernetService::RegCustomEapHandler(int netType, const std::string &regCmd,
    const sptr<INetEapPostbackCallback> &callback)
{
    NETMGR_EXT_LOG_D("Enter RegCustomEapHandler");
    if (!NetManagerPermission::CheckPermission(Permission::MANAGE_ENTERPRISE_WIFI_CONNECTION) &&
        !NetManagerPermission::CheckPermission(ENTERPRISE_MANAGE_EAP)) {
        NETMGR_EXT_LOG_E("%{public}s no permission.", __func__);
        return EAP_ERRCODE_PERMISSION_DENIED;
    }
    return NetEapHandler::GetInstance().RegCustomEapHandler(static_cast<NetType>(netType), regCmd, callback);
}

int32_t EthernetService::ReplyCustomEapData(int eapResult, const sptr<EapData> &eapData)
{
    NETMGR_EXT_LOG_D("Enter ReplyCustomEapData");
    if (eapData == nullptr) {
        NETMGR_EXT_LOG_E("ReplyCustomEapData eapData nullptr");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    if (!NetManagerPermission::CheckPermission(Permission::MANAGE_ENTERPRISE_WIFI_CONNECTION) &&
        !NetManagerPermission::CheckPermission(ENTERPRISE_MANAGE_EAP)) {
        NETMGR_EXT_LOG_E("%{public}s no permission.", __func__);
        return EAP_ERRCODE_PERMISSION_DENIED;
    }
    return NetEapHandler::GetInstance().ReplyCustomEapData(eapResult, eapData);
}

int32_t EthernetService::RegisterCustomEapCallback(int netType, const sptr<INetRegisterEapCallback> &callback)
{
    NETMGR_EXT_LOG_D("Enter RegisterCustomEapCallback");
    if (!NetManagerPermission::CheckPermission(Permission::GET_NETWORK_INFO)) {
        NETMGR_EXT_LOG_E("%{public}s no permission.", __func__);
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return NetEapHandler::GetInstance().RegisterCustomEapCallback(static_cast<NetType>(netType), callback);
}

int32_t EthernetService::UnRegisterCustomEapCallback(int netType,
    const sptr<INetRegisterEapCallback> &callback)
{
    NETMGR_EXT_LOG_D("Enter UnRegisterCustomEapCallback");
    if (!NetManagerPermission::CheckPermission(Permission::GET_NETWORK_INFO)) {
        NETMGR_EXT_LOG_E("%{public}s no permission.", __func__);
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return NetEapHandler::GetInstance().UnRegisterCustomEapCallback(static_cast<NetType>(netType), callback);
}

int32_t EthernetService::NotifyWpaEapInterceptInfo(int netType, const sptr<EapData> &eapData)
{
    NETMGR_EXT_LOG_D("Enter NotifyWpaEapInterceptInfo");
    if (eapData == nullptr) {
        NETMGR_EXT_LOG_E("NotifyWpaEapInterceptInfo eapData nullptr");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    if (!NetManagerPermission::CheckPermission(Permission::GET_NETWORK_INFO)) {
        NETMGR_EXT_LOG_E("%{public}s no permission.", __func__);
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return NetEapHandler::GetInstance().NotifyWpaEapInterceptInfo(static_cast<NetType>(netType), eapData);
}

int32_t EthernetService::GetDeviceInformation(std::vector<EthernetDeviceInfo> &deviceInfoList)
{
    if (!NetManagerPermission::IsSystemCaller()) {
        NETMGR_EXT_LOG_E("GetDeviceInformation Caller no sys permission");
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::GET_NETWORK_INFO)) {
        NETMGR_EXT_LOG_E("GetDeviceInformation no js permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return ethManagement_->GetDeviceInformation(deviceInfoList);
}

int32_t EthernetService::StartEthEap(int32_t netId, const EthEapProfile& profile)
{
    NETMGR_EXT_LOG_I("EthernetService::StartEthEap netId=%{public}d", netId);
    if (!NetManagerPermission::CheckPermission(Permission::MANAGE_ENTERPRISE_WIFI_CONNECTION) &&
        !NetManagerPermission::CheckPermission(ENTERPRISE_MANAGE_EAP)) {
        NETMGR_EXT_LOG_E("%{public}s no permission.", __func__);
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
#ifdef NET_EXTENSIBLE_AUTHENTICATION
#if defined(NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE)
    EthEapAutoAuthController::GetInstance().OnEnterpriseAppOccupied();
#endif
    int32_t ret = NetEapHandler::GetInstance().StartEthEap(netId, profile);
    NETMGR_EXT_LOG_I("EthernetService::StartEthEap done ret=%{public}d", ret);
    return ret;
#endif
    return NETMANAGER_SUCCESS;
}

int32_t EthernetService::LogOffEthEap(int32_t netId)
{
    NETMGR_EXT_LOG_I("EthernetService::LogOffEthEap netId=%{public}d", netId);
    if (!NetManagerPermission::CheckPermission(Permission::MANAGE_ENTERPRISE_WIFI_CONNECTION) &&
        !NetManagerPermission::CheckPermission(ENTERPRISE_MANAGE_EAP)) {
        NETMGR_EXT_LOG_E("%{public}s no permission.", __func__);
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    int32_t ret = NetEapHandler::GetInstance().LogOffEthEap(netId);
#if defined(NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE)
    EthEapAutoAuthController::GetInstance().OnEnterpriseAppReleased();
#endif
    return ret;
#endif
    return NETMANAGER_SUCCESS;
}

#if defined(NET_EXTENSIBLE_AUTHENTICATION) || defined(NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE)
void EthernetService::ForceRestartDhcp(const std::string &dev)
{
    ethManagement_->ForceRestartDhcp(dev);
}
#endif

#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
int32_t EthernetService::SetEthEapConfig(const EthEapConfig &config)
{
    NETMGR_EXT_LOG_I("SetEthEapConfig");
    if (!NetManagerPermission::CheckPermission(ENTERPRISE_MANAGE_EAP)) {
        NETMGR_EXT_LOG_E("%{public}s no permission.", __func__);
        return EAP_ERRCODE_PERMISSION_DENIED;
    }
    // 企业占用期间不再整体拒绝：配置仅落盘不触发认证，企业应用保持认证控制权（对齐 d.ts）
    EthEapConfig existingConfig;
    bool hasExisting = (EthEapConfigStore::GetInstance().LoadConfig(existingConfig) == NETMANAGER_EXT_SUCCESS);
    if (hasExisting && existingConfig.locked) {
        if (!config.locked) {
            NETMGR_EXT_LOG_I("SetEthEapConfig: unlock request from enterprise app, allowed");
        } else {
            NETMGR_EXT_LOG_E("SetEthEapConfig: config is locked, modification denied");
            return EAP_ERRCODE_CONFIG_LOCKED;
        }
    }
    EthEapConfig configToSave = config;
    if (hasExisting) {
        // 敏感字段继承：入参为空时沿用已存凭据（IPC 回读时凭据不回传）
        if (config.profile.password.empty()) {
            configToSave.profile.password = existingConfig.profile.password;
        }
        if (config.profile.certPassword.empty()) {
            configToSave.profile.certPassword = existingConfig.profile.certPassword;
        }
        if (configToSave.profile.certEntry.empty()) {
            configToSave.profile.certEntry = existingConfig.profile.certEntry;
        }
    }
    // 证书字段：客户端 keyUri 透传 wpa；certEntry 校验落盘回填路径；CA keyUri 尽力解析落盘 ca.pem（失败降级不报错）
    int32_t certRet = EthEapCertStore::GetInstance().ProcessCertFields(configToSave.profile);
    if (certRet != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("SetEthEapConfig: process cert fields failed: %{public}d", certRet);
        return certRet;
    }
    int32_t ret = EthEapConfigStore::GetInstance().SaveConfig(configToSave);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("SetEthEapConfig: save failed: %{public}d", ret);
        return ret;
    }
    // 新配置关闭功能或自动认证时停止自动认证；否则触发一次自动认证周期
    // （占用/断连/认证中由 TryStartAuth 内部跳过，对齐 d.ts）
    if (!configToSave.enabled || !configToSave.autoAuth) {
        NETMGR_EXT_LOG_I("SetEthEapConfig: auth disabled by new config, stop auto auth");
        EthEapAutoAuthController::GetInstance().StopAutoAuth();
    } else {
        EthEapAutoAuthController::GetInstance().OnTriggerAutoAuth();
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t EthernetService::GetEthEapConfig(EthEapConfig &config)
{
    NETMGR_EXT_LOG_D("GetEthEapConfig");
    if (!NetManagerPermission::CheckPermission(ENTERPRISE_MANAGE_EAP)) {
        NETMGR_EXT_LOG_E("%{public}s no permission.", __func__);
        return EAP_ERRCODE_PERMISSION_DENIED;
    }
    int32_t ret = EthEapConfigStore::GetInstance().LoadConfig(config);
    if (ret == NETMANAGER_EXT_ERR_FILE_NOT_FOUND) {
        NETMGR_EXT_LOG_D("GetEthEapConfig: no config stored");
        return NETMANAGER_EXT_ERR_FILE_NOT_FOUND;
    }
    if (ret == NETMANAGER_EXT_SUCCESS) {
        config.profile.password.clear();
        config.profile.certPassword.clear();
        config.profile.certEntry.clear();
    }
    return ret;
}

int32_t EthernetService::DeleteEthEapConfig()
{
    NETMGR_EXT_LOG_I("DeleteEthEapConfig");
    if (!NetManagerPermission::CheckPermission(ENTERPRISE_MANAGE_EAP)) {
        NETMGR_EXT_LOG_E("%{public}s no permission.", __func__);
        return EAP_ERRCODE_PERMISSION_DENIED;
    }
    // locked 配置保护：企业应用锁定期间禁止删除（对齐 SetEthEapConfig 的锁定语义）
    EthEapConfig existingConfig;
    if (EthEapConfigStore::GetInstance().LoadConfig(existingConfig) == NETMANAGER_EXT_SUCCESS &&
        existingConfig.locked) {
        NETMGR_EXT_LOG_E("DeleteEthEapConfig: config is locked, delete denied");
        return EAP_ERRCODE_CONFIG_LOCKED;
    }
    EthEapAutoAuthController::GetInstance().StopAutoAuth();
    EthEapStateManager::GetInstance().SetState(EthEapState::STATE_IDLE);
    int32_t ret = EthEapConfigStore::GetInstance().DeleteConfig();
    if (ret == NETMANAGER_EXT_SUCCESS) {
        EthEapCertStore::GetInstance().Clear();
    }
    return ret;
}

int32_t EthernetService::GetEthEapStateInfo(EthEapStateInfo &stateInfo)
{
    if (!NetManagerPermission::CheckPermission(ENTERPRISE_MANAGE_EAP)) {
        NETMGR_EXT_LOG_E("%{public}s no permission.", __func__);
        return EAP_ERRCODE_PERMISSION_DENIED;
    }
    // 锁内一次性读取状态/重试计数/最近消息，保证一致性快照
    EthEapStateManager::GetInstance().GetStateInfo(stateInfo);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t EthernetService::RegisterEthEapStateCallback(const sptr<IEthEapStateCallback> &callback)
{
    if (callback == nullptr) {
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    if (!NetManagerPermission::CheckPermission(ENTERPRISE_MANAGE_EAP)) {
        NETMGR_EXT_LOG_E("%{public}s no permission.", __func__);
        return EAP_ERRCODE_PERMISSION_DENIED;
    }
    return EthEapStateManager::GetInstance().RegisterCallback(callback);
}

int32_t EthernetService::UnregisterEthEapStateCallback(const sptr<IEthEapStateCallback> &callback)
{
    if (callback == nullptr) {
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    if (!NetManagerPermission::CheckPermission(ENTERPRISE_MANAGE_EAP)) {
        NETMGR_EXT_LOG_E("%{public}s no permission.", __func__);
        return EAP_ERRCODE_PERMISSION_DENIED;
    }
    return EthEapStateManager::GetInstance().UnregisterCallback(callback);
}

/**
 * @brief 认证后断连或再次连接。
 * connect=true：手动按已保存配置发起认证（不受 autoAuth 约束，无配置时失败，
 *               企业占用期间不中断其认证）；false：停止自动认证并登出底层认证，状态复位 IDLE。
 */
int32_t EthernetService::ConnectEthEap(bool connect)
{
    NETMGR_EXT_LOG_I("ConnectEthEap, connect=%{public}d", static_cast<int32_t>(connect));
    if (!NetManagerPermission::CheckPermission(ENTERPRISE_MANAGE_EAP)) {
        NETMGR_EXT_LOG_E("%{public}s no permission.", __func__);
        return EAP_ERRCODE_PERMISSION_DENIED;
    }
    if (connect) {
        // 手动连接前置校验：无已保存配置时直接失败
        // （对齐 d.ts："fails if no configuration has been stored"）
        EthEapConfig storedConfig;
        int32_t loadRet = EthEapConfigStore::GetInstance().LoadConfig(storedConfig);
        if (loadRet == NETMANAGER_EXT_ERR_FILE_NOT_FOUND) {
            NETMGR_EXT_LOG_E("ConnectEthEap: no stored config, reject");
            return EAP_ERRCODE_INVALID_PROFILE;
        }
        if (loadRet != NETMANAGER_EXT_SUCCESS) {
            NETMGR_EXT_LOG_E("ConnectEthEap: load config failed: %{public}d", loadRet);
            return loadRet;
        }
        // 手动连接：不受 autoAuth（自动连接）开关限制，按已保存配置直接发起认证；
        // 企业应用占用期间不中断其认证（TryStartAuth 内部跳过触发），本次调用
        // 仍返回成功，认证结果经状态回调上报
        EthEapAutoAuthController::GetInstance().OnUserResumed();
        EthEapAutoAuthController::GetInstance().OnManualConnect();
        return NETMANAGER_EXT_SUCCESS;
    }
    // 断连：置位"用户已断连"标志（阻止挂起重试/link 抖动/企业释放后自动重连），
    // 再停止自动认证与重试，最后登出底层认证
    EthEapAutoAuthController::GetInstance().OnUserPaused();
    EthEapAutoAuthController::GetInstance().StopAutoAuth();
    int32_t ret = NetEapHandler::GetInstance().LogOffEthEap(-1);
    // 认证状态复位为空闲
    EthEapStateManager::GetInstance().SetState(EthEapState::STATE_IDLE);
    if (ret != EAP_ERRCODE_SUCCESS) {
        NETMGR_EXT_LOG_E("ConnectEthEap: LogOffEthEap failed: %{public}d", ret);
        return ret;
    }
    return NETMANAGER_EXT_SUCCESS;
}
#endif

#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
int32_t EthernetService::EnableEthernetInterface()
{
    NETMGR_EXT_LOG_I("EnableEthernetInterface");
    if (!NetManagerPermission::IsSystemCaller()) {
        NETMGR_EXT_LOG_E("EnableEthernetInterface Caller no sys permission");
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        NETMGR_EXT_LOG_E("EnableEthernetInterface no js permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return ethManagement_->EnableEthernet();
}

int32_t EthernetService::DisableEthernetInterface()
{
    NETMGR_EXT_LOG_I("DisableEthernetInterface");
    if (!NetManagerPermission::IsSystemCaller()) {
        NETMGR_EXT_LOG_E("DisableEthernetInterface Caller no sys permission");
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        NETMGR_EXT_LOG_E("DisableEthernetInterface no js permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return ethManagement_->DisableEthernet();
}

int32_t EthernetService::IsEthernetEnabled(int32_t &enabled)
{
    NETMGR_EXT_LOG_I("IsEthernetEnabled");
    if (!NetManagerPermission::IsSystemCaller()) {
        NETMGR_EXT_LOG_E("IsEthernetEnabled Caller no sys permission");
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::GET_NETWORK_INFO)) {
        NETMGR_EXT_LOG_E("IsEthernetEnabled no js permission");
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    enabled = ethManagement_->IsEthernetEnabled() ? 1 : 0;
    return NETMANAGER_EXT_SUCCESS;
}
#endif

} // namespace NetManagerStandard
} // namespace OHOS
