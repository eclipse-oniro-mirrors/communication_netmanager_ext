/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef HWNETWORKSLICEMANAGER_H
#define HWNETWORKSLICEMANAGER_H

#include <cstdint>
#include <string>
#include <vector>
#include <set>

#include "urspconfig.h"
#include "networkslice_service.h"
#include "networksliceutil.h"
#include "model/fqdnips.h"
#include "model/networksliceinfo.h"
#include "model/hwosappId.h"

namespace OHOS {
namespace NetManagerStandard {
struct AddrInfo {
    uint32_t type_;
    std::string addr_;
};
class HwNetworkSliceManager {
    DECLARE_DELAYED_SINGLETON(HwNetworkSliceManager);
public:
    void Init();
    std::vector<std::shared_ptr<NetworkSliceInfo>> mNetworkSliceInfos;
    void InitNetworkSliceInfos();
    void HandleUrspChanged(std::map<std::string, std::string> data);
    void CleanEnvironment();
    std::shared_ptr<NetworkSliceInfo> RequestNetworkSlice(std::shared_ptr<TrafficDescriptorsInfo> td);
    void RequestNetworkSliceForFqdn(int uid, std::string fqdn, std::list<AddrInfo> addresses);
    std::shared_ptr<NetworkSliceInfo> GetNetworkSliceInfoByParaRsd(
        RouteSelectionDescriptorInfo& rsd, NetworkSliceInfo::ParaType type);
    std::shared_ptr<NetworkSliceInfo> GetNetworkSliceInfoByParaNull(NetworkSliceInfo::ParaType type);
    std::shared_ptr<NetworkSliceInfo> GetNetworkSliceInfoByParaNetCap(NetCap netCap);
    bool isUpToToplimit();
    void ChangeNetworkSliceCounter(int changeType);
    std::shared_ptr<NetworkSliceInfo> HandleRsdRequestAgain(std::shared_ptr<NetworkSliceInfo> requestAgain,
        std::shared_ptr<TrafficDescriptorsInfo> requestTd, std::shared_ptr<TrafficDescriptorsInfo> tdsInUrsp);
    void TryAddSignedUid(int uid, TrafficDescriptorsInfo tds, std::shared_ptr<NetworkSliceInfo> nsi);
    std::shared_ptr<NetworkSliceInfo> HandleMultipleUrspFirstBind(std::shared_ptr<NetworkSliceInfo> requestAgain,
        std::shared_ptr<TrafficDescriptorsInfo> requestTd, std::shared_ptr<TrafficDescriptorsInfo> tdsInUrsp);
    int BindNetworkSliceProcessToNetworkForRequestAgain(int uid, std::shared_ptr<NetworkSliceInfo> nsi,
        std::shared_ptr<FqdnIps> fqdnIps, std::shared_ptr<TrafficDescriptorsInfo> tds);
    int BindNetworkSliceProcessToNetwork(int uid, const std::set<int>& triggerActivationUids,
        std::shared_ptr<NetworkSliceInfo> nsi, std::shared_ptr<FqdnIps> fqdnIps,
        std::shared_ptr<TrafficDescriptorsInfo> tds);
    std::map<std::string, std::string> FillNetworkSliceRequest(std::shared_ptr<TrafficDescriptorsInfo> td);
    void FillUidBindParasForRequestAgain(std::map<std::string, std::string>& bindParas, int uid,
        std::shared_ptr<NetworkSliceInfo> nsi, std::shared_ptr<TrafficDescriptorsInfo> tds);
    void FillUidBindParas(std::map<std::string, std::string>& bindParas, std::shared_ptr<TrafficDescriptorsInfo> tds,
        const std::set<int>& triggerActivationUids, std::shared_ptr<NetworkSliceInfo> nsi, int uid);
    std::set<int> GetAutoUids(const std::shared_ptr<TrafficDescriptorsInfo>& tds);
    std::string GetUidsFromAppIds(const std::string& originAppIds);
    std::set<std::string> GetAppIdsWithoutOsId(const std::string& originAppIds);
    void FillIpBindParas(std::map<std::string, std::string>& bindParas, std::shared_ptr<TrafficDescriptorsInfo> tds,
        std::shared_ptr<FqdnIps> fqdnIps, std::shared_ptr<NetworkSliceInfo> nsi);
    std::shared_ptr<NetworkSliceInfo> HandleInvalidNetwork(std::shared_ptr<NetworkSliceInfo> requestAgain,
        std::shared_ptr<TrafficDescriptorsInfo> tdsInUrsp, std::shared_ptr<TrafficDescriptorsInfo> requestTd);
    bool isCanRequestNetwork();
    void RequestNetwork(int uid, std::shared_ptr<NetworkSliceInfo> networkSliceInfo);
    void TryToActivateSliceForForegroundApp();
    void RequestNetworkSliceForPackageName(int uid, std::string& packageName);
    void FillRsdIntoNetworkRequest(const sptr<NetSpecifier> request, const RouteSelectionDescriptorInfo& rsd,
        const TrafficDescriptorsInfo& tds);
    void RequestNetwork(int uid, std::shared_ptr<NetworkSliceInfo> networkSliceInfo, int requestId, int timeoutMs);
    void RequestNetworkSliceForIp(int uid, std::string ip, std::string protocolId, std::string remotePort);
    void FillIpBindParasForFqdn(std::map<std::string, std::string>& bindParas, const FqdnIps& newFqdnIps);
    void FillIpBindParasForIpTriad(std::map<std::string, std::string>& bindParas,
        std::shared_ptr<TrafficDescriptorsInfo> tds);
    int BindProcessToNetwork(std::map<std::string, std::string> bindParas);
    bool isCanMatchNetworkSlices();
    bool isEnvironmentReady();
    bool isUrspAvailable();
    void SetUrspAvailable(bool urspAvailable);
    void HandleIpReport(std::map<std::string, std::string> data);
    bool isNeedToRequestSliceForAppIdAuto(std::string appId);
    bool isNeedToRequestSliceForFqdnAuto(std::string fqdn, int uid);
    bool isNeedToRequestSliceForDnnAuto(std::string dnn, int uid);
    bool isCooperativeApp(std::string packageName);
    bool isCooperativeAppByUid(int uid);
    void ReadAppIdWhiteList(TrafficDescriptorWhiteList whiteList);
    void ReadFqdnWhiteList(TrafficDescriptorWhiteList whiteList);
    void ReadCctWhiteList(TrafficDescriptorWhiteList whiteList);
    void ReadDnnWhiteList(TrafficDescriptorWhiteList whiteList);
    void GetTrafficDescriptorWhiteList(TrafficDescriptorWhiteList whiteList);
    int UnbindAllRoute();
    int UnbindSingleNetId(int netId);
    int UnbindUids(uint netId, const std::string& uids, uint8_t urspPrecedence);
    void UnbindAllProccessToNetwork();
    int UnbindProcessToNetwork(std::string uids, int netId);
    void RestoreSliceEnvironment();
    void RequestMatchAllSlice();
    void OnNetworkAvailable(NetCap netCap, int32_t netId);
    void OnNetworkLost(NetCap netCap, int32_t netId);
    void OnNetworkUnavailable(NetCap netCap);
    void RecoveryNetworkSlice(std::shared_ptr<NetworkSliceInfo> networkSliceInfo);
    void ReleaseNetworkSlice(std::shared_ptr<NetworkSliceInfo> networkSliceInfo);
    void CleanRouteSelectionDescriptor(std::shared_ptr<NetworkSliceInfo> networkSliceInfo);
    void UnbindProcessToNetworkForSingleUid(int uid, std::shared_ptr<NetworkSliceInfo> nsi, bool isNeedToRemoveUid);
    void BindAllProccessToNetwork();
    void ReleaseNetworkSliceByApp(int32_t uid);
    void HandleUidRemoved(std::string packageName);
    void HandleUidGone(int uid);
    void onWifiNetworkStateChanged(bool isWifiConnect);
    void DumpNetworkSliceInfos();
    void GetRSDByNetCap(int32_t netcap, std::map<std::string, std::string>& sliceParasbyNetcap);
private:
    std::atomic<int> mNetworkSliceCounter;
    std::atomic<bool> mIsReady = false;
    std::atomic<bool> mHasMatchAllSlice = false;
    std::atomic<bool> mIsMatchAllRequsted = false;
    bool mIsUrspAvailable = false;
    bool mIsMatchRequesting = false;
    std::map<int, std::map<std::string, std::string>> networkSliceParas;
    std::vector<HwOsAppId> mWhiteListForOsAppId = std::vector<HwOsAppId>();
    std::vector<std::string> mWhiteListForDnn = std::vector<std::string>();
    std::vector<std::string> mWhiteListForFqdn = std::vector<std::string>();
    std::vector<std::string> mWhiteListForCct = std::vector<std::string>();
    std::vector<std::string> mWhiteListForCooperativeApp = std::vector<std::string>();
};

} // namespace NetManagerStandard
} // namespace OHOS

#endif  // NETWORKSLICEMANAGER_H
