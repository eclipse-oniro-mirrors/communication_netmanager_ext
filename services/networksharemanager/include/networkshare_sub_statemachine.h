/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef NETWORKSHARE_SUB_STATEMACHINE_H
#define NETWORKSHARE_SUB_STATEMACHINE_H

#include "dhcp_c_api.h"
#include "net_manager_constants.h"
#include "net_manager_ext_constants.h"
#include "networkshare_configuration.h"
#include "networkshare_constants.h"
#include "networkshare_hisysevent.h"
#include "networkshare_state_common.h"
#include "router_advertisement_daemon.h"
#include "router_advertisement_params.h"
#include <any>
#include <cstring>
#include <map>
#include <mutex>
#include <set>
#include <sstream>

namespace OHOS {
namespace NetManagerStandard {
class NetworkShareSubStateMachine : public std::enable_shared_from_this<NetworkShareSubStateMachine> {
    using HandleFunc = int (NetworkShareSubStateMachine::*)(const std::any &messageObj);

public:
    NetworkShareSubStateMachine() = delete;
    NetworkShareSubStateMachine(const std::string &ifaceName, const SharingIfaceType &interfaceType,
                                const std::shared_ptr<NetworkShareConfiguration> &configuration);
    ~NetworkShareSubStateMachine();

    /**
     * get sub state machine share type
     */
    SharingIfaceType GetNetShareType() const;

    /**
     * get sub state machine interface name
     */
    const std::string &GetInterfaceName() const;

    class SubStateMachineCallback {
    public:
        virtual void OnUpdateInterfaceState(const std::shared_ptr<NetworkShareSubStateMachine> &paraSubStateMachine,
                                            int state, int lastError) = 0;
    };

    /**
     * register callback
     */
    void RegisterSubSMCallback(const std::shared_ptr<SubStateMachineCallback> &callback);

    /**
     * execute state switch
     */
    void SubSmStateSwitch(int newState);

    /**
     * execute event
     */
    void SubSmEventHandle(int eventId, const std::any &messageObj);

    /**
     * get down interface name
     */
    void GetDownIfaceName(std::string &downIface);

    /**
     * get up interface name
     */
    void GetUpIfaceName(std::string &upIface);

    void HandleConnection();

private:
    void CreateInitStateTable();
    void CreateSharedStateTable();
    void InitStateEnter();
    void SharedStateEnter();
    void UnavailableStateEnter();
    void InitStateExit();
    void SharedStateExit();
    void UnavailableStateExit();
    int HandleInitSharingRequest(const std::any &messageObj);
    int HandleInitInterfaceDown(const std::any &messageObj);
    int HandleSharedUnrequest(const std::any &messageObj);
    int HandleSharedInterfaceDown(const std::any &messageObj);
    int HandleSharedConnectionChange(const std::any &messageObj);
    int HandleSharedErrors(const std::any &messageObj);

    bool ConfigureShareDhcp(bool enabled);
    bool RequestIpv4Address(std::shared_ptr<INetAddr> &netAddr);
    bool StartDhcp(const std::shared_ptr<INetAddr> &netAddr);
    bool SetRange(DhcpRange &range, const std::string &ipHead, const std::string &strStartip,
                  const std::string &strEndip, const std::string &mask);
    bool StopDhcp();
    void HandleConnectionChanged(const std::shared_ptr<UpstreamNetworkInfo> &upstreamNetInfo);
    void RemoveRoutesToLocalNetwork();
    void AddRoutesToLocalNetwork();
    void CleanupUpstreamInterface();
    bool HasChangeUpstreamIfaceSet(const std::string &newUpstreamIface);
    bool GetWifiHotspotDhcpFlag();
    bool GetBtDestinationAddr(std::string &addrStr);
    bool GetWifiApDestinationAddr(std::string &addrStr);
    bool GetWifiApDstIpv6Addr();
    bool GetUsbDestinationAddr(std::string &addrStr);
    bool CheckConfig(std::string &endIp, std::string &mask);
    bool FindDestinationAddr(std::string &destination);
    std::recursive_mutex &getUsefulMutex();
    bool StartIpv6();
    void StopIpv6();
    std::string MacToEui64Addr(std::string &mac);
    int32_t GenerateIpv6(const std::string &iface);
    bool GetShareIpv6Prefix(const std::string &iface);
    void AddIpv6AddrToLocalNetwork();
    void AddIpv6InfoToLocalNetwork();

private:
    struct SubSmStateTable {
        int32_t event_;
        int32_t curState_;
        HandleFunc func_;
        int32_t nextState_;
    };
    std::recursive_mutex mutex_;
    std::string ifaceName_;
    SharingIfaceType netShareType_;
    int32_t lastError_ = NETMANAGER_EXT_SUCCESS;
    std::string upstreamIfaceName_;
    std::shared_ptr<SubStateMachineCallback> trackerCallback_ = nullptr;
    std::shared_ptr<NetworkShareConfiguration> configuration_ = nullptr;
    int32_t curState_ = SUBSTATE_INIT;
    std::vector<SubSmStateTable> stateTable_;
    std::shared_ptr<RouterAdvertisementDaemon> raDaemon_ = nullptr;
    RaParams lastRaParams_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETWORKSHARE_SUB_STATEMACHINE_H
