/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef NETWORKSLICEMANAGER_H
#define NETWORKSLICEMANAGER_H

#include <cstdint>
#include <string>
#include <vector>
#include "event_handler.h"
#include "nrunsolicitedmsgparser.h"
#include "singleton.h"
#include "networkslicemsgcenter.h"
#include "networkslice_service_base.h"
#include "networkslicecommconfig.h"
#include "broadcast_proxy.h"
namespace OHOS {
namespace NetManagerStandard {

const int32_t INVALID_FOREGROUND_UID = -1;
class NetworkSliceManager : public NetworkSliceServiceBase {
    DECLARE_DELAYED_SINGLETON(NetworkSliceManager);
public:
    std::vector<ForbiddenRouteDescriptor> mNormalForbiddenRules;
    bool mIsIpParaReportEnable = false;
    bool mApnStartflag = true;
    void OnInit() override;;
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
    void ProcessEventEx(const AppExecFwk::InnerEvent::Pointer& event);
    void InitUePolicy();
    void HandleForegroundAppChanged(const std::shared_ptr<AppExecFwk::AppStateData>& msg);
    void HandleUrspChanged(const std::shared_ptr<std::map<std::string, std::string>>& msg);
    void HandleSimStateChanged();
    void HandleUrspFromUnsolData(const std::shared_ptr<std::vector<uint8_t>>& buffer);
    void HandleIpRpt(const std::shared_ptr<std::vector<uint8_t>>& msg);
    void HandleAllowedNssaiFromUnsolData(const std::shared_ptr<std::vector<uint8_t>>& msg);
    void HandleEhplmnFromUnsolData(const std::shared_ptr<std::vector<uint8_t>>& msg);
    void SendUrspUpdateMsg();
    void GetRouteSelectionDescriptorByAppDescriptor(const std::shared_ptr<GetSlicePara>& getSlicePara);
    bool isMeetNetworkSliceConditions();
    void NotifySlicePara(const std::shared_ptr<GetSlicePara>& getSlicePara);
    bool GetAppDescriptor(std::map<std::string, std::string>& data, AppDescriptor& appDescriptor);
    void SetAppId(AppDescriptor& appDescriptor, const std::vector<std::string>& values, const std::string& appId);
    void FillRouteSelectionDescriptor(std::map<std::string, std::string>& ret,
        SelectedRouteDescriptor routeRule);
    bool isSaState();
    bool hasAvailableUrspRule();
    bool isNrSlicesSupported();
    bool isDefaultDataOnMainCard();
    bool isWifiConnected();
    bool isScreenOn();
    bool isAirPlaneModeOn();
    bool isInVpnMode();
    void SetSaState(bool isSaState);
    bool isRouteRuleInForbiddenList(const SelectedRouteDescriptor& routeRule);
    void HandleIpv4Rpt(int& startIndex, const std::vector<uint8_t>& buffer,
        std::map<std::string, std::string>& bundle, AppDescriptor& appDescriptor);
    void HandleIpv6Rpt(int& startIndex, const std::vector<uint8_t>& buffer,
        std::map<std::string, std::string>& bundle, AppDescriptor& appDescriptor);
    void DumpAppDescriptor(AppDescriptor appDescriptor);
    void DumpSelectedRouteDescriptor(SelectedRouteDescriptor routeRule);
    void IpParaReportControl();
    void BindProcessToNetworkByFullPara(std::shared_ptr<std::map<std::string, std::string>> msg);
    bool GetUidRoutePara(AddRoutePara& addRoutePara, std::map<std::string, std::string>& data);
    bool GetRoutePara(AddRoutePara& addRoutePara, std::map<std::string, std::string>& data);
    void GetRouteParaEx(AddRoutePara& addRoutePara, std::map<std::string, std::string>& data);
    bool CalculateParaLen(AddRoutePara& addRoutePara);
    bool FillRoutePara(std::vector<uint8_t>& buffer, AddRoutePara addRoutePara);
    void DeleteNetworkBindByFullPara(std::shared_ptr<std::map<std::string, std::string>> msg);
    void FillDeletePara(short len, int type, std::vector<int> precedenceArray, std::vector<int> uidArrays,
        std::map<std::string, std::string> data);
    std::vector<int> GetUidArray(std::string uids);
    std::vector<int> GetPrecedenceArray(std::string precedences);
    void CloseTcpSocketsForUid(int uid);
    void SendIpPara(AppDescriptor appDescriptor, std::map<std::string, std::string> bundle);
    void onUrspAvailableStateChanged();
    void ProcessNetworkParaForbiddenTimeOut();
    void StartNetworkParaForbiddenTimer(int64_t timeMillies);
    void HandleNetworkActivateResult(std::shared_ptr<std::map<std::string, std::any>> data);
    int32_t BindUidProcessToNetworkForDns(int netid, int uid);
    bool isCanRequestNetwork();
    void HandleAirModeChanged(int32_t mode);
    void HandleWifiConnChanged(int32_t state);
    void HandleVpnModeChanged(bool mode);
    bool isMobileDataClose();
    void HandleScreenOn();
    void HandleScreenOff();
    int32_t GetForeGroundAppUid();
    void GetRouteSelectionDescriptorByDNN(const std::string dnn, std::string& snssai, uint8_t& sscmode);
private:
    int32_t foregroundApp_uid = INVALID_FOREGROUND_UID;
    bool mIsUrspFirstReported = false;
    bool airModeOn_ { false };
    bool wifiConn_ { false };
    bool vpnMode_ { false };
    bool screenOn_ { false };
    bool isSaState_ { false };
};

extern std::shared_ptr<NrUnsolicitedMsgParser> sNrUnsolicitedMsgParser_;
extern std::shared_ptr<UrspConfig> sUrspConfig_;


} // namespace NetManagerStandard
} // namespace OHOS

#endif  // NETWORKSLICEMANAGER_H
