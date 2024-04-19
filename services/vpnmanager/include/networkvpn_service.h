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
#include "event_handler.h"
#include "i_vpn_conn_state_cb.h"
#include "net_vpn_impl.h"
#include "networkvpn_service_stub.h"
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

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr const char *ALWAYS_ON_VPN_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=sharing_always_on_vpn";
constexpr const char *KEY_ALWAYS_ON_VPN = "settings.netmanager.always_on_vpn";

} // namespace
using namespace OHOS::EventFwk;
class NetworkVpnService : public SystemAbility, public NetworkVpnServiceStub, protected NoCopyable {
    DECLARE_SYSTEM_ABILITY(NetworkVpnService)

    NetworkVpnService();
    virtual ~NetworkVpnService();

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
    class VpnConnStateCb : public IVpnConnStateCb {
    public:
        explicit VpnConnStateCb(const NetworkVpnService &vpnService) : vpnService_(vpnService){};
        virtual ~VpnConnStateCb() = default;
        void OnVpnConnStateChanged(const VpnConnectState &state) override;

    private:
        const NetworkVpnService &vpnService_;
    };

    class ReceiveMessage : public OHOS::EventFwk::CommonEventSubscriber {
    public:
        explicit ReceiveMessage(const EventFwk::CommonEventSubscribeInfo &subscriberInfo, NetworkVpnService &vpnService)
            : EventFwk::CommonEventSubscriber(subscriberInfo), vpnService_(vpnService){};

        virtual void OnReceiveEvent(const EventFwk::CommonEventData &eventData) override;

    private:
        NetworkVpnService &vpnService_;
    };

public:
    static NetworkVpnService &GetInstance()
    {
        static NetworkVpnService instance;
        return instance;
    }
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
    int32_t SetUpVpn(const sptr<VpnConfig> &config, bool isVpnExtCall = false) override;

    /**
     * protect vpn tunnel
     */
    int32_t Protect(bool isVpnExtCall = false) override;

    /**
     * stop the vpn connection
     */
    int32_t DestroyVpn(bool isVpnExtCall = false) override;

    /**
     * register callback
     */
    int32_t RegisterVpnEvent(const sptr<IVpnEventCallback> callback) override;

    /**
     * unregister callback
     */
    int32_t UnregisterVpnEvent(const sptr<IVpnEventCallback> callback) override;

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

private:
    ServiceRunningState state_ = ServiceRunningState::STATE_STOPPED;
    bool isServicePublished_ = false;
    std::shared_ptr<IVpnConnStateCb> vpnConnCallback_;
    std::shared_ptr<NetVpnImpl> vpnObj_;

    std::vector<sptr<IVpnEventCallback>> vpnEventCallbacks_;
    std::shared_ptr<ffrt::queue> networkVpnServiceFfrtQueue_ = nullptr;
    std::mutex netVpnMutex_;
    bool hasSARemoved_ = false;

    std::shared_ptr<ReceiveMessage> subscriber_ = nullptr;

private:
    void RegisterFactoryResetCallback();
    class FactoryResetCallBack : public IRemoteStub<INetFactoryResetCallback> {
    public:
        explicit FactoryResetCallBack(NetworkVpnService& vpnService):vpnService_(vpnService){};

        int32_t OnNetFactoryReset()
        {
            return vpnService_.FactoryResetVpn();
        }
    private:
        NetworkVpnService& vpnService_;
    };

    sptr<INetFactoryResetCallback> netFactoryResetCallback_ = nullptr;

public:
    int32_t RegisterBundleName(const std::string &bundleName) override;
    class VpnHapObserver : public AppExecFwk::ApplicationStateObserverStub {
    public:
        explicit VpnHapObserver(NetworkVpnService &vpnService) : vpnService_(vpnService){};
        virtual ~VpnHapObserver() = default;
        void OnExtensionStateChanged(const AppExecFwk::AbilityStateData &abilityStateData) override ;
        void OnProcessCreated(const AppExecFwk::ProcessData &processData) override ;
        void OnProcessStateChanged(const AppExecFwk::ProcessData &processData) override ;
        void OnProcessDied(const AppExecFwk::ProcessData &processData) override ;
    private:
        NetworkVpnService& vpnService_;
    };
private:
    sptr<VpnHapObserver> vpnHapObserver_ = nullptr;
    std::string vpnBundleName_ = "";
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETWORK_VPN_SERVICE_H
