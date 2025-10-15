/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef NETWORK_VPN_SERVICE_H
#define NETWORK_VPN_SERVICE_H

#include <memory>
#include <string>
#include "event_handler.h"
#include "i_vpn_conn_state_cb.h"
#include "net_vpn_impl.h"
#include "network_vpn_service_stub.h"
#include "os_account_manager.h"
#include "singleton.h"
#include "system_ability.h"
#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"
#include "application_state_observer_stub.h"
#include "app_mgr_client.h"
#include "cJSON.h"
#include "ffrt.h"
#ifdef SUPPORT_SYSVPN
#include "ipsec_vpn_ctl.h"
#include "vpn_database_helper.h"
#endif // SUPPORT_SYSVPN

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr const char *ALWAYS_ON_VPN_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=sharing_always_on_vpn";
constexpr const char *KEY_ALWAYS_ON_VPN = "settings.netmanager.always_on_vpn";

} // namespace
using namespace OHOS::EventFwk;
class NetworkVpnService : public SystemAbility, public NetworkVpnServiceStub, protected NoCopyable,
    public std::enable_shared_from_this<NetworkVpnService> {
    DECLARE_DELAYED_SINGLETON(NetworkVpnService)
    DECLARE_SYSTEM_ABILITY(NetworkVpnService)

    enum ServiceRunningState {
        STATE_STOPPED = 0,
        STATE_RUNNING,
    };

    enum {
        POWER_MODE_MIN = 600,
        NORMAL_MODE = POWER_MODE_MIN,
        SAVE_MODE,
        EXTREME_MODE,
        LOWPOWER_MODE,
        POWER_MODE_MAX = LOWPOWER_MODE
    };
    class VpnHapObserver;
    class VpnConnStateCb : public IVpnConnStateCb {
    public:
        explicit VpnConnStateCb(NetworkVpnService &vpnService) : vpnService_(vpnService) {};
        virtual ~VpnConnStateCb() = default;
        void OnVpnConnStateChanged(const VpnConnectState &state) override;
        void OnMultiVpnConnStateChanged(const VpnConnectState &state, const std::string &vpnId) override;

    private:
        NetworkVpnService &vpnService_;
    };

    class ReceiveMessage : public OHOS::EventFwk::CommonEventSubscriber {
    public:
        ReceiveMessage(const EventFwk::CommonEventSubscribeInfo &subscriberInfo,
            std::weak_ptr<NetworkVpnService> vpnService)
            : EventFwk::CommonEventSubscriber(subscriberInfo), vpnService_(vpnService) {};

        virtual void OnReceiveEvent(const EventFwk::CommonEventData &eventData) override;

    private:
        std::weak_ptr<NetworkVpnService> vpnService_;
    };
#ifdef SUPPORT_SYSVPN
    struct MultiVpnEventCallback : RefBase {
        std::string bundleName;
        int32_t userId;
        sptr<IVpnEventCallback> callback;
    };
#endif // SUPPORT_SYSVPN

public:
    /**
     * service start
     */
    void OnStart() override;

    /**
     * service stop
     */
    void OnStop() override;

    /**
     * check current whether has vpn is running
     */
    int32_t Prepare(bool &isExistVpn, bool &isRun, std::string &pkg) override;

    /**
     * This function is called when the three-party vpn application negotiation ends
     */
    int32_t SetUpVpn(const VpnConfig &config, bool isVpnExtCall = false) override;

    /**
     * protect vpn tunnel
     */
    int32_t Protect(bool isVpnExtCall = false) override;

    /**
     * stop the vpn connection
     */
    int32_t DestroyVpn(bool isVpnExtCall = false) override;

#ifdef SUPPORT_SYSVPN
    /**
     * get vpn cert data
     */
    int32_t GetVpnCertData(const int32_t certType, std::vector<int8_t> &certData) override;

    /**
     * stop the vpn connection
     */
    int32_t DestroyVpn(const std::string &vpnId) override;

    /**
     * This function is called when the system vpn application negotiation ends
     */
    int32_t SetUpSysVpn(const sptr<SysVpnConfig> &config, bool isVpnExtCall = false) override;

    /**
     * save the vpn config
     */
    int32_t AddSysVpnConfig(const sptr<SysVpnConfig> &config) override;

    /**
     * get the vpn config list
     */
    int32_t DeleteSysVpnConfig(const std::string &vpnId) override;

    /**
     * get the app info of connected vpn
     */
    int32_t GetConnectedVpnAppInfo(std::vector<std::string> &bundleNameList) override;

    /**
     * get the vpn config listGetConnectedSysVpnConfig
     */
    int32_t GetSysVpnConfigList(std::vector<sptr<SysVpnConfig>> &vpnList) override;

    /**
     * get the vpn config
     */
    int32_t GetSysVpnConfig(sptr<SysVpnConfig> &config, const std::string &vpnId) override;

    /**
     * get the vpn connection state
     */
    int32_t GetConnectedSysVpnConfig(sptr<SysVpnConfig> &config) override;

    /**
     * notify the vpn connection stage and result
     */
    int32_t NotifyConnectStage(const std::string &stage, const int32_t result) override;

    int32_t GetSysVpnCertUri(const int32_t certType, std::string &certUri) override;

    /**
     * register multi vpn callback
     */
    int32_t RegisterMultiVpnEvent(const sptr<IVpnEventCallback> &callback) override;

    /**
     * unregister multi vpn callback
     */
    int32_t UnregisterMultiVpnEvent(const sptr<IVpnEventCallback> &callback) override;
#endif // SUPPORT_SYSVPN

    /**
     * register callback
     */
    int32_t RegisterVpnEvent(const sptr<IVpnEventCallback> &callback) override;

    /**
     * unregister callback
     */
    int32_t UnregisterVpnEvent(const sptr<IVpnEventCallback> &callback) override;

    /**
     * create the vpn connection
     */
    int32_t CreateVpnConnection(bool isVpnExtCall = false) override;

    /**
     * dump function
     */
    int32_t Dump(int32_t fd, const std::vector<std::u16string> &args) override;

	 /**
     * factory reset vpn , such as always on vpn
     *
     * @return Returns 0 success. Otherwise fail
     */
    int32_t FactoryResetVpn() override;

    /**
     * persist the always on vpn's package
     * pass empty will disable always on VPN
    */
    int32_t SetAlwaysOnVpn(std::string &pkg, bool &enable);

    /**
     * read the persisted always on vpn's package
    */
    int32_t GetAlwaysOnVpn(std::string &pkg);

    int32_t GetSelfAppName(std::string &selfAppName, std::string &selfBundleName) override;

    int32_t SetSelfVpnPid() override;

protected:
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;

private:
    bool Init();
    void GetDumpMessage(std::string &message);
    int32_t CheckCurrentAccountType(int32_t &userId, std::vector<int32_t> &activeUserIds);

    void OnVpnMultiUserSetUp();
    int32_t SyncRegisterVpnEvent(const sptr<IVpnEventCallback> callback);
    int32_t SyncUnregisterVpnEvent(const sptr<IVpnEventCallback> callback);

    #ifdef SUPPORT_SYSVPN
    int32_t SyncRegisterMultiVpnEvent(const sptr<IVpnEventCallback> callback, const std::string &vpnBundleName);
    int32_t SyncUnregisterMultiVpnEvent(const sptr<IVpnEventCallback> callback);
    #endif // SUPPORT_SYSVPN

    void OnNetSysRestart();
    void ConvertVecRouteToJson(const std::vector<Route>& routes, cJSON* jVecRoutes);
    void ConvertNetAddrToJson(const INetAddr& netAddr, cJSON* jInetAddr);
    void ParseConfigToJson(const sptr<VpnConfig> &vpnCfg, std::string& jsonString);
    void SaveVpnConfig(const sptr<VpnConfig> &vpnCfg);

    void ConvertRouteToConfig(Route& tmp, const cJSON* const mem);
    void ConvertVecRouteToConfig(sptr<VpnConfig> &vpnCfg, const cJSON* const doc);
    void ConvertNetAddrToConfig(INetAddr& tmp, const cJSON* const mem);
    void ConvertVecAddrToConfig(sptr<VpnConfig> &vpnCfg, const cJSON* const doc);
    void ConvertStringToConfig(sptr<VpnConfig> &vpnCfg, const cJSON* const doc);
    void ParseJsonToConfig(sptr<VpnConfig> &vpnCfg, const std::string& jsonString);
    void RecoverVpnConfig();

    void StartAlwaysOnVpn();
    void SubscribeCommonEvent();
    int32_t CheckIpcPermission(const std::string &strPermission);
    bool CheckSystemCall(const std::string &bundleName);
    bool CheckVpnExtPermission(const std::string &bundleName);
    bool PublishEvent(const OHOS::AAFwk::Want &want, int eventCode,
         bool isOrdered, bool isSticky, const std::vector<std::string> &permissions) const;
    void PublishVpnConnectionStateEvent(const VpnConnectState &state) const;
#ifdef SUPPORT_SYSVPN
    std::shared_ptr<NetVpnImpl> CreateSysVpnCtl(const sptr<SysVpnConfig> &config, int32_t userId,
        std::vector<int32_t> &activeUserIds, bool isVpnExtCall);
    std::shared_ptr<NetVpnImpl> CreateOpenvpnCtl(const sptr<SysVpnConfig> &config, int32_t userId,
        std::vector<int32_t> &activeUserIds);
    std::shared_ptr<IpsecVpnCtl> CreateIpsecVpnCtl(const sptr<SysVpnConfig> &config, int32_t userId,
        std::vector<int32_t> &activeUserIds);
    int32_t QueryVpnData(const sptr<SysVpnConfig> config, sptr<VpnDataBean> &vpnBean);
    std::shared_ptr<IpsecVpnCtl> CreateL2tpCtl(const sptr<SysVpnConfig> &config, int32_t userId,
        std::vector<int32_t> &activeUserIds);
    int32_t DestroyMultiVpn(int32_t callingUid);
    int32_t DestroyMultiVpn(const std::shared_ptr<NetVpnImpl> &vpnObj, bool needErase = true);
    int32_t InitMultiVpnInfo(const std::string &vpnId, int32_t vpnType,
        std::string &vpnBundleName, int32_t userId, std::shared_ptr<NetVpnImpl> &vpnObj);
#endif // SUPPORT_SYSVPN
    int32_t IsSetUpReady(const std::string &vpnId, std::string &vpnBundleName,
        int32_t &userId, std::vector<int32_t> &activeUserIds);
    std::string GetBundleName();
    std::string GetCurrentVpnBundleName();
    std::vector<std::string> GetCurrentVpnAbilityName();
    void ClearCurrentVpnUserInfo();
    void UnregVpnHpObserver(const sptr<NetworkVpnService::VpnHapObserver> &VpnHapObserver);
    bool IsCurrentVpnPid(int32_t uid, int32_t pid);
    bool CheckVpnPermission(const std::string &bundleName);
    void OnVpnConnStateChanged(const VpnConnectState &state);
#ifdef SUPPORT_SYSVPN
    void OnMultiVpnConnStateChanged(const VpnConnectState &state, const std::string &vpnId, int32_t userId);
#endif

private:
    ServiceRunningState state_ = ServiceRunningState::STATE_STOPPED;
    bool isServicePublished_ = false;
    std::shared_ptr<IVpnConnStateCb> vpnConnCallback_;
    ffrt::shared_mutex netVpnMutex_;
    std::shared_ptr<NetVpnImpl> vpnObj_;
#ifdef SUPPORT_SYSVPN
    std::shared_ptr<NetVpnImpl> connectingObj_;
    std::map<std::string, std::shared_ptr<NetVpnImpl>> vpnObjMap_;
    ffrt::shared_mutex multiVpnEventCallbacksMutex_;
    std::vector<sptr<MultiVpnEventCallback>> multiVpnEventCallbacks_;
#endif // SUPPORT_SYSVPN
    ffrt::shared_mutex vpnEventCallbacksMutex_;
    std::vector<sptr<IVpnEventCallback>> vpnEventCallbacks_;
    bool hasSARemoved_ = false;
    int32_t userId_ = -1;

    std::shared_ptr<ReceiveMessage> subscriber_ = nullptr;

private:
    void RegisterFactoryResetCallback();
    class FactoryResetCallBack : public IRemoteStub<INetFactoryResetCallback> {
    public:
        explicit FactoryResetCallBack(NetworkVpnService& vpnService) : vpnService_(vpnService) {};

        int32_t OnNetFactoryReset()
        {
            return vpnService_.FactoryResetVpn();
        }
    private:
        NetworkVpnService& vpnService_;
    };

    sptr<INetFactoryResetCallback> netFactoryResetCallback_ = nullptr;

public:
    int32_t RegisterBundleName(const std::string &bundleName, const std::string &abilityName) override;
    class VpnHapObserver : public AppExecFwk::ApplicationStateObserverStub {
    public:
        explicit VpnHapObserver(NetworkVpnService &vpnService, const std::string &bundleName)
            : vpnService_(vpnService), bundleName_(bundleName), hasAbilityName_(false) {};
        explicit VpnHapObserver(NetworkVpnService &vpnService, const std::string &bundleName,
            const std::string &abilityName)
            : vpnService_(vpnService), bundleName_(bundleName), abilityName_(abilityName), hasAbilityName_(true) {};
        virtual ~VpnHapObserver() = default;
        void OnExtensionStateChanged(const AppExecFwk::AbilityStateData &abilityStateData) override ;
        void OnProcessCreated(const AppExecFwk::ProcessData &processData) override ;
        void OnProcessStateChanged(const AppExecFwk::ProcessData &processData) override ;
        void OnProcessDied(const AppExecFwk::ProcessData &processData) override ;
    private:
        NetworkVpnService& vpnService_;
        std::string bundleName_;
        std::string abilityName_;
        bool hasAbilityName_ = false;
    };
private:
    class VpnAppDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        VpnAppDeathRecipient(std::weak_ptr<NetworkVpnService> client) : client_(client) {}
        ~VpnAppDeathRecipient() override = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override
        {
            auto client = client_.lock();
            if (client == nullptr) {
                return;
            }
            client->OnRemoteDied(remote);
        }

    private:
        std::weak_ptr<NetworkVpnService> client_;
    };
    void OnRemoteDied(const wptr<IRemoteObject> &remoteObject);
    bool AddClientDeathRecipient(const sptr<IVpnEventCallback> &callback);
    void RemoveClientDeathRecipient(const sptr<IVpnEventCallback> &callback);
    void RemoveALLClientDeathRecipient();

    std::mutex vpnNameMutex_;
    std::mutex cesMutex_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ = nullptr;
    bool registeredCommonEvent_ = false;
    int32_t hasOpenedVpnUid_ = 0;
    std::string currentVpnBundleName_;
    ffrt::shared_mutex vpnPidMapMutex_;
    std::map<int32_t, int32_t> setVpnPidMap_;
    int32_t currSetUpVpnPid_ = 0;
    std::vector<std::string> currentVpnAbilityName_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETWORK_VPN_SERVICE_H
