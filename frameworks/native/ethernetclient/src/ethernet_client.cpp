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

#include "ethernet_client.h"

#include <thread>

#include "iethernet_service.h"
#include "if_system_ability_manager.h"
#include "mac_address_info.h"
#include "interface_configuration.h"
#include "ipc_types.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "refbase.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace NetManagerStandard {

static constexpr uint32_t WAIT_FOR_SERVICE_TIME_MS = 500;
static constexpr uint32_t MAX_GET_SERVICE_COUNT = 10;

EthernetClient::EthernetClient() : ethernetService_(nullptr), deathRecipient_(nullptr), callback_(nullptr) {}

EthernetClient::~EthernetClient()
{
    NETMGR_EXT_LOG_I("~EthernetClient : Destroy EthernetClient");
    sptr<IEthernetService> proxy = GetProxy();
    if (proxy == nullptr) {
        return;
    }
 
    auto serviceRemote = proxy->AsObject();
    if (serviceRemote == nullptr) {
        return;
    }
    if (deathRecipient_) {
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
    }
}

int32_t EthernetClient::GetMacAddress(std::vector<MacAddressInfo> &macAddrList)
{
    sptr<IEthernetService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    return proxy->GetMacAddress(macAddrList);
}

int32_t EthernetClient::SetIfaceConfig(const std::string &iface, sptr<InterfaceConfiguration> &ic)
{
    sptr<IEthernetService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    return proxy->SetIfaceConfig(iface, ic);
}

int32_t EthernetClient::GetIfaceConfig(const std::string &iface, sptr<InterfaceConfiguration> &ifaceConfig)
{
    sptr<IEthernetService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    return proxy->GetIfaceConfig(iface, ifaceConfig);
}

int32_t EthernetClient::IsIfaceActive(const std::string &iface, int32_t &activeStatus)
{
    sptr<IEthernetService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    return proxy->IsIfaceActive(iface, activeStatus);
}

int32_t EthernetClient::GetAllActiveIfaces(std::vector<std::string> &activeIfaces)
{
    sptr<IEthernetService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    return proxy->GetAllActiveIfaces(activeIfaces);
}

int32_t EthernetClient::ResetFactory()
{
    sptr<IEthernetService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    return proxy->ResetFactory();
}

sptr<IEthernetService> EthernetClient::GetProxy()
{
    std::lock_guard lock(mutex_);
    if (ethernetService_) {
        NETMGR_EXT_LOG_D("get proxy is ok");
        return ethernetService_;
    }
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        NETMGR_EXT_LOG_E("GetProxy, get SystemAbilityManager failed");
        return nullptr;
    }
    sptr<IRemoteObject> remote = sam->CheckSystemAbility(COMM_ETHERNET_MANAGER_SYS_ABILITY_ID);
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("get Remote service failed");
        return nullptr;
    }
    deathRecipient_ = new (std::nothrow) EthernetDeathRecipient(*this);
    if (deathRecipient_ == nullptr) {
        NETMGR_EXT_LOG_E("Recipient new failed!");
    }
    if ((remote->IsProxyObject()) && (!remote->AddDeathRecipient(deathRecipient_))) {
        NETMGR_EXT_LOG_E("add death recipient failed");
        return nullptr;
    }
    ethernetService_ = iface_cast<IEthernetService>(remote);
    if (ethernetService_ == nullptr) {
        NETMGR_EXT_LOG_E("get Remote service proxy failed");
        return nullptr;
    }
    return ethernetService_;
}

void EthernetClient::RecoverCallback()
{
    uint32_t count = 0;
    while (GetProxy() == nullptr && count < MAX_GET_SERVICE_COUNT) {
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_FOR_SERVICE_TIME_MS));
        count++;
    }
    auto proxy = GetProxy();
    NETMGR_EXT_LOG_D("Get proxy %{public}s, count: %{public}u", proxy == nullptr ? "failed" : "success", count);
    if (proxy != nullptr && callback_ != nullptr) {
        int32_t ret = proxy->RegisterIfacesStateChanged(callback_);
        NETMGR_EXT_LOG_D("Register result %{public}d", ret);
    }
}

void EthernetClient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("remote object is nullptr");
        return;
    }
    std::lock_guard lock(mutex_);
    if (ethernetService_ == nullptr) {
        NETMGR_EXT_LOG_E("ethernetService_ is nullptr");
        return;
    }
    sptr<IRemoteObject> local = ethernetService_->AsObject();
    if (local == nullptr) {
        NETMGR_EXT_LOG_E("local is nullptr");
        return;
    }
    if (local != remote.promote()) {
        NETMGR_EXT_LOG_E("proxy and stub is not same remote object");
        return;
    }
    local->RemoveDeathRecipient(deathRecipient_);
    ethernetService_ = nullptr;

    if (callback_ != nullptr) {
        NETMGR_EXT_LOG_D("on remote died recover callback");
        std::thread t([this]() {
            RecoverCallback();
        });
        std::string threadName = "ethernetRecoverCallback";
        pthread_setname_np(t.native_handle(), threadName.c_str());
        t.detach();
    }
}

int32_t EthernetClient::RegisterIfacesStateChanged(const sptr<InterfaceStateCallback> &callback)
{
    NETMGR_EXT_LOG_D("RegisterIfacesStateChanged client in.");
    sptr<IEthernetService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    int32_t ret = proxy->RegisterIfacesStateChanged(callback);
    if (ret == NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_D("RegisterIfacesStateChanged success, save callback.");
        callback_ = callback;
    }

    return ret;
}

int32_t EthernetClient::UnregisterIfacesStateChanged(const sptr<InterfaceStateCallback> &callback)
{
    NETMGR_EXT_LOG_D("UnRegisterIfacesStateChanged client in.");
    sptr<IEthernetService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("proxy is nullptr");
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    int32_t ret = proxy->UnregisterIfacesStateChanged(callback);
    if (ret == NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_D("UnRegisterIfacesStateChanged success, delete callback.");
        callback_ = nullptr;
    }

    return ret;
}

int32_t EthernetClient::SetInterfaceUp(const std::string &iface)
{
    sptr<IEthernetService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    return proxy->SetInterfaceUp(iface);
}

int32_t EthernetClient::SetInterfaceDown(const std::string &iface)
{
    sptr<IEthernetService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    return proxy->SetInterfaceDown(iface);
}

int32_t EthernetClient::GetInterfaceConfig(const std::string &iface, OHOS::nmd::InterfaceConfigurationParcel &cfg)
{
    sptr<IEthernetService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    ConfigurationParcelIpc configIpc;
    int32_t ret = proxy->GetInterfaceConfig(iface, configIpc);
    ConfigurationParcelIpc::ConvertEtherConfigParcelToNmd(configIpc, cfg);
    return ret;
}

int32_t EthernetClient::SetInterfaceConfig(const std::string &iface, OHOS::nmd::InterfaceConfigurationParcel &cfg)
{
    sptr<IEthernetService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    ConfigurationParcelIpc configIpc;
    ConfigurationParcelIpc::ConvertNmdToEtherConfigParcel(configIpc, cfg);
    return proxy->SetInterfaceConfig(iface, configIpc);
}

int32_t EthernetClient::RegCustomEapHandler(NetType netType, const std::string &regCmd,
    const sptr<INetEapPostbackCallback> &callback)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    if (callback == nullptr) {
        NETMGR_EXT_LOG_E("%{public}s callback is nullptr.", __func__);
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    sptr<IEthernetService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("%{public}s proxy is nullptr.", __func__);
        return NETMANAGER_ERR_GET_PROXY_FAIL;
    }
    return proxy->RegCustomEapHandler(static_cast<int>(netType), regCmd, callback);
#else
    return NETMANAGER_SUCCESS;
#endif
}
 
int32_t EthernetClient::ReplyCustomEapData(int result, const sptr<EapData> &eapData)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    if (eapData == nullptr) {
        NETMGR_EXT_LOG_E("%{public}s, eapData is nullptr", __func__);
        return NETMANAGER_ERR_GET_PROXY_FAIL;
    }
    sptr<IEthernetService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("%{public}s, proxy is nullptr.", __func__);
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return proxy->ReplyCustomEapData(result, eapData);
#else
    return NETMANAGER_SUCCESS;
#endif
}
 
int32_t EthernetClient::RegisterCustomEapCallback(const NetType netType, const sptr<INetRegisterEapCallback> &callback)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    sptr<IEthernetService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("%{public}s proxy is nullptr.", __func__);
        return NETMANAGER_ERR_GET_PROXY_FAIL;
    }
    return proxy->RegisterCustomEapCallback(static_cast<int>(netType), callback);
#else
    return NETMANAGER_SUCCESS;
#endif
}
 
int32_t EthernetClient::UnRegisterCustomEapCallback(const NetType netType,
    const sptr<INetRegisterEapCallback> &callback)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    sptr<IEthernetService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("%{public}s proxy is nullptr.", __func__);
        return NETMANAGER_ERR_GET_PROXY_FAIL;
    }
 
    return proxy->UnRegisterCustomEapCallback(static_cast<int>(netType), callback);
#else
    return NETMANAGER_SUCCESS;
#endif
}
 
int32_t EthernetClient::NotifyWpaEapInterceptInfo(const NetType netType, const sptr<EapData> &eapData)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    if (eapData == nullptr) {
        NETMGR_EXT_LOG_E("%{public}s eapData is nullptr.", __func__);
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    sptr<IEthernetService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("%{public}s proxy is nullptr.", __func__);
        return NETMANAGER_ERR_GET_PROXY_FAIL;
    }
 
    return proxy->NotifyWpaEapInterceptInfo(static_cast<int>(netType), eapData);
#else
    return NETMANAGER_SUCCESS;
#endif
}
 
int32_t EthernetClient::GetDeviceInformation(std::vector<EthernetDeviceInfo> &deviceInfoList)
{
    sptr<IEthernetService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    return proxy->GetDeviceInformation(deviceInfoList);
}

int32_t EthernetClient::StartEthEap(int32_t netId, const EthEapProfile& profile)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    sptr<IEthernetService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    return proxy->StartEthEap(netId, profile);
#else
    return NETMANAGER_SUCCESS;
#endif
}
 
int32_t EthernetClient::LogOffEthEap(int32_t netId)
{
#ifdef NET_EXTENSIBLE_AUTHENTICATION
    sptr<IEthernetService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    return proxy->LogOffEthEap(netId);
#else
    return NETMANAGER_SUCCESS;
#endif
}
} // namespace NetManagerStandard
} // namespace OHOS
