/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef NETWORKSHARE_TRACKER_H
#define NETWORKSHARE_TRACKER_H

#include <map>
#include <any>

#include "event_handler.h"
#include "net_manager_ext_constants.h"
#include "i_netshare_result_callback.h"
#include "i_sharing_event_callback.h"
#include "netsys_controller_callback.h"
#include "networkshare_main_statemachine.h"
#include "networkshare_sub_statemachine.h"
#include "networkshare_configuration.h"
#include "networkshare_upstreammonitor.h"
#include "networkshare_hisysevent.h"
#include "wifi_hotspot.h"
#include "wifi_ap_msg.h"
#include "i_wifi_hotspot_callback.h"
#include "bluetooth_pan.h"
#include "bluetooth_remote_device.h"

namespace OHOS {
namespace NetManagerStandard {
enum class EHandlerEventType { EVENT_HANDLER_MSG_FIR = 1, EVENT_HANDLER_MSG_SEC = 2 };

class NetworkShareMainStateMachine;
class NetworkShareTracker {
    class NetsysCallback : public NetsysControllerCallback {
    public:
        NetsysCallback() = default;
        ~NetsysCallback() = default;

        int32_t OnInterfaceAddressUpdated(const std::string &, const std::string &, int, int) override;
        int32_t OnInterfaceAddressRemoved(const std::string &, const std::string &, int, int) override;
        int32_t OnInterfaceAdded(const std::string &iface) override;
        int32_t OnInterfaceRemoved(const std::string &iface) override;
        int32_t OnInterfaceChanged(const std::string &, bool) override;
        int32_t OnInterfaceLinkStateChanged(const std::string &, bool) override;
        int32_t OnRouteChanged(bool, const std::string &, const std::string &, const std::string &) override;
        int32_t OnDhcpSuccess(NetsysControllerCallback::DhcpResult &dhcpResult) override;
        int32_t OnBandwidthReachedLimit(const std::string &limitName, const std::string &iface) override;
    };

    class ManagerEventHandler : public AppExecFwk::EventHandler {
    public:
        explicit ManagerEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner);
        ~ManagerEventHandler() = default;

        void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
    };

    class MainSmUpstreamCallback : public NetworkShareUpstreamMonitor::NotifyUpstreamCallback {
    public:
        MainSmUpstreamCallback() = default;
        virtual ~MainSmUpstreamCallback() = default;

        void OnUpstreamStateChanged(int msgName, int param1);
        void OnUpstreamStateChanged(int msgName, int param1, int param2, const std::any &messageObj);
    };

    class SubSmUpstreamCallback : public NetworkShareSubStateMachine::SubStateMachineCallback {
    public:
        SubSmUpstreamCallback() = default;
        virtual ~SubSmUpstreamCallback() = default;

        void OnUpdateInterfaceState(const std::shared_ptr<NetworkShareSubStateMachine> &paraSubStateMachine, int state,
                                    int lastError);
    };

    class WifiShareHotspotEventCallback : public Wifi::IWifiHotspotCallback {
    public:
        WifiShareHotspotEventCallback() = default;
        virtual ~WifiShareHotspotEventCallback() = default;

        void OnHotspotStateChanged(int state) override;
        void OnHotspotStaJoin(const Wifi::StationInfo &info) override;
        void OnHotspotStaLeave(const Wifi::StationInfo &info) override;
        OHOS::sptr<OHOS::IRemoteObject> AsObject() override;
    };

    class SharingPanObserver : public Bluetooth::PanObserver {
    public:
        SharingPanObserver() = default;
        virtual ~SharingPanObserver() = default;

        void OnConnectionStateChanged(const Bluetooth::BluetoothRemoteDevice &device, int state);
    };

    class NetSharingSubSmState {
    public:
        NetSharingSubSmState(const std::shared_ptr<NetworkShareSubStateMachine> &subStateMachine, bool isNcm);
        ~NetSharingSubSmState() = default;

    public:
        std::shared_ptr<NetworkShareSubStateMachine> subStateMachine_;
        int lastState_;
        int lastError_;
        bool isNcm_;
    };

public:
    static NetworkShareTracker &GetInstance();
    ~NetworkShareTracker() = default;

    /**
     * Init
     */
    bool Init();

    /**
     * Uninit
     */
    void Uninit();

    /**
     * is surpport share network
     */
    int32_t IsNetworkSharingSupported();

    /**
     * has sharing network
     */
    int32_t IsSharing();

    /**
     * start share network by type
     */
    int32_t StartNetworkSharing(const SharingIfaceType &type);

    /**
     * stop share netwaork by type
     */
    int32_t StopNetworkSharing(const SharingIfaceType &type);

    /**
     * get sharable regexs
     */
    std::vector<std::string> GetSharableRegexs(SharingIfaceType type);

    /**
     * get sharing type
     */
    int32_t GetSharingState(SharingIfaceType type, SharingIfaceState &state);

    /**
     * get sharing ifaces name
     */
    std::vector<std::string> GetNetSharingIfaces(const SharingIfaceState &state);

    /**
     * register callback
     */
    int32_t RegisterSharingEvent(sptr<ISharingEventCallback> callback);

    /**
     * unregister callback
     */
    int32_t UnregisterSharingEvent(sptr<ISharingEventCallback> callback);

    /**
     * is need update upstream network
     */
    bool UpstreamWanted();

    /**
     * modify shared sub state machine list
     */
    void ModifySharedSubStateMachineList(bool isAdd, const std::shared_ptr<NetworkShareSubStateMachine> &subSm);

    /**
     * get the main state machine
     */
    std::shared_ptr<NetworkShareMainStateMachine> &GetMainStateMachine();

    /**
     * notify shared sub state machine to update upstream interface when upstream network changed
     */
    void SetUpstreamNetHandle(const std::shared_ptr<UpstreamNetworkInfo> &netinfo);

    /**
     * get the upstream info
     */
    void GetUpstreamInfo(std::shared_ptr<UpstreamNetworkInfo> &upstreamInfo);

    /**
     * notify shared sub state machine to update upstream interface
     */
    void NotifyDownstreamsHasNewUpstreamIface(const std::shared_ptr<UpstreamNetworkInfo> &netinfo);

    int32_t GetSharedSubSMTraffic(const TrafficType &type);

private:
    NetworkShareTracker() = default;

    void HandleSubSmUpdateInterfaceState(const std::shared_ptr<NetworkShareSubStateMachine> &who, int32_t state,
                                         int32_t lastError);
    int32_t EnableNetSharingInternal(const SharingIfaceType &type, bool enable);
    int32_t SetWifiNetworkSharing(bool enable);
    int32_t SetUsbNetworkSharing(bool enable);
    int32_t SetBluetoothNetworkSharing(bool enable);
    void EnableWifiSubStateMachine();
    void EnableBluetoothSubStateMachine();
    int32_t Sharing(std::string iface, int32_t reqState);
    void SendGlobalSharingStateChange();
    void SendIfaceSharingStateChange(const SharingIfaceType type, const std::string iface,
                                     const SharingIfaceState state);
    void SendSharingUpstreamChange(const sptr<NetHandle> netHandle);
    int32_t CreateSubStateMachine(const std::string &iface, const SharingIfaceType &interfaceType, bool isNcm);
    void StopSubStateMachine(const std::string iface, const SharingIfaceType &interfaceType);
    bool IsInterfaceMatchType(const std::string &iface, const SharingIfaceType &type);
    bool InterfaceNameToType(const std::string &iface, SharingIfaceType &type);
    bool IsHandleNetlinkEvent(const SharingIfaceType &type, bool up);
    bool FindSubStateMachine(const std::string &iface, const SharingIfaceType &interfaceType,
                             std::shared_ptr<NetworkShareSubStateMachine> &subSM, std::string &findKey);
    void InterfaceAdded(const std::string &iface);
    void InterfaceRemoved(const std::string &iface);
    void InterfaceStatusChanged(const std::string &iface, bool up);
    void SetDnsForwarders(const NetHandle &netHandle);
    void StopDnsProxy();
    SharingIfaceState SubSmStateToExportState(const int32_t state);
    void RegisterWifiApCallback();
    void RegisterBtPanCallback();
    void SetWifiState(const Wifi::ApState &state);
    void SetBluetoothState(const Bluetooth::BTConnectState &state);

private:
    std::mutex mutex_;
    std::shared_ptr<NetworkShareConfiguration> configuration_ = nullptr;
    sptr<NetsysControllerCallback> netsysCallback_ = nullptr;
    std::shared_ptr<NetworkShareTracker::ManagerEventHandler> eventHandler_ = nullptr;
    std::weak_ptr<NetworkShareUpstreamMonitor::MonitorEventHandler> monitorHandler_;
    std::shared_ptr<NetworkShareMainStateMachine> mainStateMachine_ = nullptr;
    std::map<std::string, std::shared_ptr<NetSharingSubSmState>> subStateMachineMap_;
    std::vector<sptr<ISharingEventCallback>> sharingEventCallback_;
    std::shared_ptr<SharingPanObserver> panObserver_ = nullptr;
    std::unique_ptr<Wifi::WifiHotspot> wifiHotspotPtr_ = nullptr;
    bool isNetworkSharing_ = false;
    std::shared_ptr<UpstreamNetworkInfo> upstreamInfo_ = nullptr;
    std::vector<SharingIfaceType> clientRequestsVector_;
    std::vector<std::shared_ptr<NetworkShareSubStateMachine>> sharedSubSM_;
    bool isStartDnsProxy_ = false;
    int32_t wifiShareCount_ = 0;
    int32_t bluetoothShareCount_ = 0;
    Wifi::ApState curWifiState_ = Wifi::ApState::AP_STATE_NONE;
    Bluetooth::BTConnectState curBluetoothState_ = Bluetooth::BTConnectState::DISCONNECTED;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETWORKSHARE_TRACKER_H
