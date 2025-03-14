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
#include "networksliceutil.h"
#include "net_conn_client.h"
#include "networkslicemanager.h"
#include "net_conn_callback.h"
#include "parameters.h"
#include "hwnetworkslicemanager.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int MAX_NETWORK_SLICE = 6;
constexpr int CHANGE_TYPE_ZEROING = 0;
constexpr int CHANGE_TYPE_INCREASE = 1;
constexpr int CHANGE_TYPE_DECREASE = -1;
constexpr int INVALID_UID = -1;
constexpr int BINDNETWORKSLICESUCCESS = 0;
constexpr int ERROR_NO_SERVICE = -1;
constexpr int ERROR_INVALID_PARAM = -3;
constexpr int REQUEST_NETWORK_TIMEOUT = 10 * 1000;
constexpr int IPV4_LEN = 4;
constexpr int IPV6_LEN = 16;
const std::string NETMANAGER_EXT_NETWORKSLICE_ABILITY = "persist.netmgr_ext.networkslice";
static bool g_isNrSliceSupport = system::GetBoolParameter(NETMANAGER_EXT_NETWORKSLICE_ABILITY, false);
static std::string REQUEST_NETWORK_SLICE_OS_ID = "01020304050607080102030405060708#";
static std::string REQUEST_NETWORK_SLICE_APPID = "appId";
static std::string REQUEST_NETWORK_SLICE_UID = "uid";
static std::string REQUEST_NETWORK_SLICE_FQDN = "fqdn";
static std::string REQUEST_NETWORK_SLICE_DNN = "dnn";
static std::string REQUEST_NETWORK_SLICE_IP = "ip";
static std::string REQUEST_NETWORK_SLICE_PROTOCOL = "protocolId";
static std::string REQUEST_NETWORK_SLICE_REMOTE_PORT = "remotePort";
static std::string REQUEST_NETWORK_SLICE_CONNECTION_CAPABILITY = "connectionCapability";
static std::string BIND_ROUTE_UID = "uids";
static std::string BIND_ROUTE_URSP_PRECEDENCE = "urspPrecedence";
static std::string BIND_ROUTE_NETID = "netId";
static std::string BIND_ROUTE_IPV4_NUM = "ipv4Num";
static std::string BIND_ROUTE_IPV4_ADDRANDMASK = "ipv4AddrAndMask";
static std::string BIND_ROUTE_IPV6_NUM = "ipv6Num";
static std::string BIND_ROUTE_IPV6_ADDRANDPREFIX = "ipv6AddrAndPrefix";
static std::string BIND_ROUTE_PROTOCOL_IDS = "protocolIds";
static std::string BIND_ROUTE_REMOTE_PORTS = "remotePorts";
static std::string UNBIND_ROUTE_TYPE = "type";
static std::string SEPARATOR_FOR_NORMAL_DATA = ",";
constexpr int UNBIND_TYPE_ALL = 0;
constexpr int UNBIND_TYPE_NETID = 1;
constexpr int UNBIND_TYPE_UIDS = 2;

HwNetworkSliceManager::HwNetworkSliceManager() {}
 
HwNetworkSliceManager::~HwNetworkSliceManager() {}

void HwNetworkSliceManager::Init()
{
    if (g_isNrSliceSupport == false) {
        return;
    }
    InitNetworkSliceInfos();
}

void HwNetworkSliceManager::InitNetworkSliceInfos()
{
    NETMGR_EXT_LOG_I("HwNetworkSliceManager::InitNetworkSliceInfos");
    for (int nc = NET_CAPABILITY_SNSSAI1; nc <= NET_CAPABILITY_SNSSAI6; nc++) {
        sptr<NetSpecifier> request(new NetSpecifier());
        request->SetCapability(static_cast<OHOS::NetManagerStandard::NetCap>(nc));
        request->SetType(BEARER_CELLULAR);
        std::shared_ptr<NetworkSliceInfo> networkSliceInfo = std::make_shared<NetworkSliceInfo>();
        networkSliceInfo->setNetworkRequest(request);
        networkSliceInfo->setNetworkCapability(nc);
        mNetworkSliceInfos.emplace_back(networkSliceInfo);
    }
    ChangeNetworkSliceCounter(CHANGE_TYPE_ZEROING);
}

void HwNetworkSliceManager::GetTrafficDescriptorWhiteList(TrafficDescriptorWhiteList whiteList)
{
    NETMGR_EXT_LOG_I("HwNetworkSliceManager::GetTrafficDescriptorWhiteList");
    ReadAppIdWhiteList(whiteList);
    ReadDnnWhiteList(whiteList);
    ReadFqdnWhiteList(whiteList);
    ReadCctWhiteList(whiteList);
}

void HwNetworkSliceManager::HandleUrspChanged(std::map<std::string, std::string> data)
{
    NETMGR_EXT_LOG_I("HwNetworkSliceManager::HandleUrspChanged");
    if (!g_isNrSliceSupport) {
        NETMGR_EXT_LOG_E("requestNetworkSlice, current environment cannot match slices");
        return;
    }
    if (data.empty()) {
        NETMGR_EXT_LOG_E("handleUrspChanged, data is null");
        return;
    }
    mIsReady.store(false, std::memory_order_relaxed);
    mIsUrspAvailable = true;
    CleanEnvironment();
    RouteSelectionDescriptorInfo rsd = RouteSelectionDescriptorInfo::makeRouteSelectionDescriptor(data);
    mIsReady.store(true, std::memory_order_relaxed);
    TryToActivateSliceForForegroundApp();
    if (!rsd.isMatchAll()) {
        mHasMatchAllSlice.store(false, std::memory_order_relaxed);
        return;
    }
    mHasMatchAllSlice.store(true, std::memory_order_relaxed);
    std::shared_ptr<NetworkSliceInfo> networkSliceInfo =
        GetNetworkSliceInfoByParaNull(NetworkSliceInfo::ParaType::ROUTE_SELECTION_DESCRIPTOR);
    if (networkSliceInfo != nullptr) {
        NETMGR_EXT_LOG_I("networkSliceInfo != null is true");
        networkSliceInfo->setRouteSelectionDescriptor(rsd);
        uint8_t route_bitmap = 0;
        if (data.find(TrafficDescriptorsInfo::TDS_ROUTE_BITMAP) != data.end()) {
            route_bitmap = std::stoi(data[TrafficDescriptorsInfo::TDS_ROUTE_BITMAP]);
        }
        networkSliceInfo->setTempTrafficDescriptors(TrafficDescriptorsInfo::Builder()
            .setRouteBitmap(route_bitmap)
            .build());
        ChangeNetworkSliceCounter(CHANGE_TYPE_INCREASE);
    }

    NETMGR_EXT_LOG_I("match all slice start to active when ursp changed");

    if (!isCanRequestNetwork()) {
        NETMGR_EXT_LOG_E("handleUrspChanged can not request network");
        return;
    }
    mIsMatchRequesting = true;
    RequestNetwork(INVALID_UID, networkSliceInfo);
}

void HwNetworkSliceManager::HandleIpReport(std::map<std::string, std::string> data)
{
    if (data.empty()) {
        NETMGR_EXT_LOG_I("IP_REPORT data is null, return");
        return;
    }
    int uid = std::stoi(data[REQUEST_NETWORK_SLICE_UID]);
    std::string ip = data[REQUEST_NETWORK_SLICE_IP];
    std::string protocolId = data[REQUEST_NETWORK_SLICE_PROTOCOL];
    std::string remotePort = data[REQUEST_NETWORK_SLICE_REMOTE_PORT];
    RequestNetworkSliceForIp(uid, ip, protocolId, remotePort);
}

void HwNetworkSliceManager::TryToActivateSliceForForegroundApp()
{
    NETMGR_EXT_LOG_I("TryToActivateSliceForForegroundApp");
    int uid = DelayedSingleton<NetworkSliceManager>::GetInstance()->GetForeGroundAppUid();
    if (uid == INVALID_UID) {
        return;
    }
    std::string packageName;
    int ret = DelayedSingleton<NetworkSliceService>::GetInstance()->GetBundleNameForUid(uid, packageName);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("Error GetBundleNameForUid fail");
        return;
    }
    NETMGR_EXT_LOG_I("TryToActivateSliceForForegroundApp uid = %{public}d, packagename = %{public}s",
        uid, packageName.c_str());
    if (packageName == "com.ohos.sceneboard") {
        return;
    }
    RequestNetworkSliceForPackageName(uid, packageName);
}


void HwNetworkSliceManager::RequestNetworkSliceForPackageName(int uid, std::string& packageName)
{
    NETMGR_EXT_LOG_I("RequestNetworkSliceForPackageName");
    if (!isCanMatchNetworkSlices()) {
        NETMGR_EXT_LOG_I("requestNetworkSlice, current environment cannot match slices");
        return;
    }

    if (!isNeedToRequestSliceForAppIdAuto(packageName)) {
        NETMGR_EXT_LOG_I("No need to requestNetwork for uid = %{public}d", uid);
        return;
    }

    TrafficDescriptorsInfo td = TrafficDescriptorsInfo::Builder().setUid(uid).build();
    std::shared_ptr<TrafficDescriptorsInfo> tdinfo = std::make_shared<TrafficDescriptorsInfo>(td);
    std::shared_ptr<NetworkSliceInfo> networkSliceInfo = RequestNetworkSlice(tdinfo);
    RequestNetwork(uid, networkSliceInfo);
}

void HwNetworkSliceManager::RequestNetworkSliceForFqdn(int uid, std::string fqdn, std::list<AddrInfo> addresses)
{
    if (!isCanMatchNetworkSlices()) {
        NETMGR_EXT_LOG_D("requestNetworkSlice, current environment cannot match slices");
        return;
    }
    if (addresses.size() == 0) {
        NETMGR_EXT_LOG_D("requestNetworkSliceForFqdn ipAddresses is null.");
        return;
    }
    if (!isNeedToRequestSliceForFqdnAuto(fqdn, uid)) {
        NETMGR_EXT_LOG_D("requestNetworkSliceForFqdn No need to requestNetwork for uid = %{public}d", uid);
        return;
    }
    std::set<INetAddr> ipv4;
    std::set<INetAddr> ipv6;
    for (auto &it : addresses) {
        INetAddr ip;
        ip.address_ = it.addr_;
        ip.hostName_ = fqdn;
        switch (it.type_) {
            case 0:
                ip.type_ = INetAddr::IPV4;
                ipv4.insert(ip);
                break;
            case 1:
                ip.type_ = INetAddr::IPV6;
                ipv6.insert(ip);
                break;
            default:
                NETMGR_EXT_LOG_E("ip length wrong, len = %{public}d", ip.prefixlen_);
        }
    }
    FqdnIps fqdnIps;
    fqdnIps.setIpv4Addr(ipv4);
    fqdnIps.setIpv4Addr(ipv6);
    TrafficDescriptorsInfo td = TrafficDescriptorsInfo::Builder().setUid(uid).setFqdn(fqdn).setFqdnIps(fqdnIps).build();
    std::shared_ptr<TrafficDescriptorsInfo> tdinfo = std::make_shared<TrafficDescriptorsInfo>(td);
    std::shared_ptr<NetworkSliceInfo> networkSliceInfo = RequestNetworkSlice(tdinfo);
    RequestNetwork(uid, networkSliceInfo);
}

void HwNetworkSliceManager::RequestNetworkSliceForIp(int uid, std::string ip, std::string protocolId,
    std::string remotePort)
{
    if (!isCanMatchNetworkSlices()) {
        NETMGR_EXT_LOG_D("requestNetworkSlice, current environment cannot match slices");
        return;
    }
    INetAddr inetAddress;
    inetAddress.address_ = ip;
    TrafficDescriptorsInfo td = TrafficDescriptorsInfo::Builder().setUid(uid).setIp(inetAddress)
        .setProtocolId(protocolId).setRemotePort(remotePort).build();
    std::shared_ptr<TrafficDescriptorsInfo> tdinfo = std::make_shared<TrafficDescriptorsInfo>(td);
    std::shared_ptr<NetworkSliceInfo> networkSliceInfo = RequestNetworkSlice(tdinfo);
    RequestNetwork(uid, networkSliceInfo);
}

std::shared_ptr<NetworkSliceInfo> HwNetworkSliceManager::RequestNetworkSlice(std::shared_ptr<TrafficDescriptorsInfo> td)
{
    NETMGR_EXT_LOG_I("HwNetworkSliceManager::RequestNetworkSlice");
    if (td == nullptr) {
        NETMGR_EXT_LOG_I("requestNetworkSlice, the TrafficDescriptorsInfo is null");
        return nullptr;
    }
    std::shared_ptr<GetSlicePara> getSlicePara = std::make_shared<GetSlicePara>();
    getSlicePara->data = FillNetworkSliceRequest(td);
    DelayedSingleton<NetworkSliceManager>::GetInstance()->GetRouteSelectionDescriptorByAppDescriptor(getSlicePara);
    std::map<std::string, std::string> result = getSlicePara->ret;
    if (result.size() == 0) {
        NETMGR_EXT_LOG_E("can't get network slice");
        return nullptr;
    }

    RouteSelectionDescriptorInfo rsd = RouteSelectionDescriptorInfo::makeRouteSelectionDescriptor(result);
    TrafficDescriptorsInfo tds = TrafficDescriptorsInfo::makeTrafficDescriptorsInfo(result);
    std::shared_ptr<NetworkSliceInfo> requestAgain =
        GetNetworkSliceInfoByParaRsd(rsd, NetworkSliceInfo::ParaType::ROUTE_SELECTION_DESCRIPTOR);
    if (requestAgain != nullptr) {
        std::shared_ptr<TrafficDescriptorsInfo> tdsInUrsp = std::make_shared<TrafficDescriptorsInfo>(tds);
        return HandleRsdRequestAgain(requestAgain, td, tdsInUrsp);
    }
    if (isUpToToplimit()) {
        NETMGR_EXT_LOG_I("already has 6 network slices, do not request again. uid = %{public}d", td->getUid());
        return nullptr;
    }
    std::shared_ptr<NetworkSliceInfo> networkSliceInfo =
        GetNetworkSliceInfoByParaNull(NetworkSliceInfo::ParaType::ROUTE_SELECTION_DESCRIPTOR);
    if (networkSliceInfo != nullptr) {
        networkSliceInfo->setRouteSelectionDescriptor(rsd);
        networkSliceInfo->cacheTrafficDescriptors(tds);
        networkSliceInfo->setTempTrafficDescriptors(tds);
        ChangeNetworkSliceCounter(CHANGE_TYPE_INCREASE);
        if (tds.isMatchFqdn()) {
            networkSliceInfo->setFqdnIps(td->getFqdnIps(), tds);
        }
        NETMGR_EXT_LOG_I("Slice network has binded.");
    }
    return networkSliceInfo;
}

std::shared_ptr<NetworkSliceInfo> HwNetworkSliceManager::HandleRsdRequestAgain(
    std::shared_ptr<NetworkSliceInfo> requestAgain, std::shared_ptr<TrafficDescriptorsInfo> requestTd,
    std::shared_ptr<TrafficDescriptorsInfo> tdsInUrsp)
{
    NETMGR_EXT_LOG_I("HandleRsdRequestAgain");
    if (requestAgain == nullptr || requestTd == nullptr) {
        return nullptr;
    }
    std::shared_ptr<SliceRouteInfo> sri;
    if (tdsInUrsp != nullptr) {
        sri = requestAgain->getSliceRouteInfo(*tdsInUrsp);
    }
    if (sri == nullptr) {
        // sri is null means the tdsInUrsp is first bind to process.
        return HandleMultipleUrspFirstBind(requestAgain, tdsInUrsp, requestTd);
    }
    // IP bind do not need to bind again
    bool isBinded = requestAgain->isBindCompleted(requestTd->getUid(), requestTd->getFqdnIps(), tdsInUrsp);
    if (tdsInUrsp->isIpTriad() || isBinded) {
        // unbindAllProccessToNetwork will clear usedUid, and when app appears foreground we should cache it again
        if (tdsInUrsp->isUidRouteBindType() && sri != nullptr) {
            sri->addUsedUid(requestTd->getUid());
        }
        NETMGR_EXT_LOG_I("networkSlice has allready binded uid: %{public}d", requestTd->getUid());
        if (requestTd->isNeedToCreateRequest()) {
            requestTd->setRequestAgain(true);
            return requestAgain;
        }
        return nullptr;
    }
    if (requestAgain->getNetId() == NetworkSliceInfo::INVALID_NET_ID) {
        // RSD not null but NetId is invalid, that means the slice network is activing.
        return HandleInvalidNetwork(requestAgain, tdsInUrsp, requestTd);
    }
    int bindResult = BindNetworkSliceProcessToNetworkForRequestAgain(requestTd->getUid(), requestAgain,
        std::make_shared<FqdnIps>(requestTd->getFqdnIps()), tdsInUrsp);
    if (bindResult == BINDNETWORKSLICESUCCESS && tdsInUrsp->isUidRouteBindType()) {
        requestAgain->addUid(requestTd->getUid(), *tdsInUrsp);
        requestAgain->addUsedUid(requestTd->getUid(), *tdsInUrsp);
        TryAddSignedUid(requestTd->getUid(), *tdsInUrsp, requestAgain);
    }
    NETMGR_EXT_LOG_I("no need to request this slice again. uid = %{public}d and bind result %{public}d",
        requestTd->getUid(), bindResult);
    if (requestTd->isNeedToCreateRequest()) {
        requestTd->setRequestAgain(true);
        return requestAgain;
    }
    return nullptr;
}

bool HwNetworkSliceManager::isUpToToplimit()
{
    return mNetworkSliceCounter.load() >= MAX_NETWORK_SLICE;
}

std::shared_ptr<NetworkSliceInfo> HwNetworkSliceManager::GetNetworkSliceInfoByParaRsd(
    RouteSelectionDescriptorInfo& rsd, NetworkSliceInfo::ParaType type)
{
    int i = 0;
    for (auto sliceInfo : mNetworkSliceInfos) {
        NETMGR_EXT_LOG_I("GetNetworkSliceInfoByParaRsd i = %{public}d", i);
        ++i;
        if (sliceInfo->isRightNetworkSliceRsd(rsd, type)) {
            NETMGR_EXT_LOG_I("getNetworkSliceInfoByPara, sliceInfo");
            return sliceInfo;
        }
    }
    NETMGR_EXT_LOG_I("getNetworkSliceInfoByPara, return null");
    return std::shared_ptr<NetworkSliceInfo>();
}

std::shared_ptr<NetworkSliceInfo> HwNetworkSliceManager::GetNetworkSliceInfoByParaNull(NetworkSliceInfo::ParaType type)
{
    NETMGR_EXT_LOG_I("GetNetworkSliceInfoByParaNull");
    int i = 0;
    for (auto sliceInfo : mNetworkSliceInfos) {
        NETMGR_EXT_LOG_I("GetNetworkSliceInfoByParaNull i = %{public}d", i);
        ++i;
        if (sliceInfo->isRightNetworkSliceNull(type)) {
            NETMGR_EXT_LOG_I("getNetworkSliceInfoByNull, sliceInfo");
            return sliceInfo;
        }
    }
    NETMGR_EXT_LOG_I("getNetworkSliceInfoByNull, return null");
    return std::shared_ptr<NetworkSliceInfo>();
}

std::shared_ptr<NetworkSliceInfo> HwNetworkSliceManager::GetNetworkSliceInfoByParaNetCap(NetCap netCap)
{
    NETMGR_EXT_LOG_I("GetNetworkSliceInfoByParaNetCap = %{public}d", netCap);
    for (auto sliceInfo : mNetworkSliceInfos) {
        if (sliceInfo->isRightNetworkSliceNetCap(netCap)) {
            NETMGR_EXT_LOG_I("getNetworkSliceInfoByNetCap, sliceInfo");
            return sliceInfo;
        }
    }
    NETMGR_EXT_LOG_I("getNetworkSliceInfoByNetCap, return null");
    return std::shared_ptr<NetworkSliceInfo>();
}

std::shared_ptr<NetworkSliceInfo> HwNetworkSliceManager::HandleMultipleUrspFirstBind(
    std::shared_ptr<NetworkSliceInfo> requestAgain, std::shared_ptr<TrafficDescriptorsInfo> requestTd,
    std::shared_ptr<TrafficDescriptorsInfo> tdsInUrsp)
{
    NETMGR_EXT_LOG_I("HandleMultipleUrspFirstBind");
    if (requestAgain == nullptr || requestTd == nullptr || tdsInUrsp == nullptr) {
        return nullptr;
    }
    requestAgain->cacheTrafficDescriptors(*tdsInUrsp);
    std::set<int> triggerActivationUids;
    triggerActivationUids.insert(requestTd->getUid());
    TryAddSignedUid(requestTd->getUid(), *tdsInUrsp, requestAgain);

    if (tdsInUrsp->isMatchFqdn()) {
        requestAgain->setFqdnIps(requestTd->getFqdnIps(), *tdsInUrsp);
    }
    BindNetworkSliceProcessToNetwork(requestTd->getUid(), triggerActivationUids, requestAgain,
        requestAgain->getFqdnIps(*tdsInUrsp), tdsInUrsp);
    if (requestTd->isNeedToCreateRequest()) {
        requestTd->setRequestAgain(true);
        return requestAgain;
    }
    NETMGR_EXT_LOG_I("HandleMultipleUrspFirstBind End");
    return nullptr;
}

std::shared_ptr<NetworkSliceInfo> HwNetworkSliceManager::HandleInvalidNetwork(
    std::shared_ptr<NetworkSliceInfo> requestAgain, std::shared_ptr<TrafficDescriptorsInfo> tdsInUrsp,
    std::shared_ptr<TrafficDescriptorsInfo> requestTd)
{
    NETMGR_EXT_LOG_I("HandleInvalidNetwork");
    if (requestAgain == nullptr || tdsInUrsp == nullptr || requestTd == nullptr) {
        return nullptr;
    }
    std::vector<FqdnIps> waittingFqdnIps = requestAgain->getWaittingFqdnIps(*tdsInUrsp);
    if (waittingFqdnIps.size() == 0) {
        waittingFqdnIps = std::vector<FqdnIps>();
        requestAgain->setWaittingFqdnIps(waittingFqdnIps, *tdsInUrsp);
    }
    if (requestAgain->getFqdnIps(*tdsInUrsp) != nullptr) {
        waittingFqdnIps.push_back(requestAgain->getFqdnIps(*tdsInUrsp)->getNewFqdnIps(requestTd->getFqdnIps()));
    }

    if (requestTd->isNeedToCreateRequest()) {
        requestTd->setRequestAgain(true);
        return requestAgain;
    }
    return nullptr;
}

void HwNetworkSliceManager::TryAddSignedUid(int uid, TrafficDescriptorsInfo tds, std::shared_ptr<NetworkSliceInfo> nsi)
{
    NETMGR_EXT_LOG_I("TryAddSignedUid");
    std::shared_ptr<TrafficDescriptorsInfo> tds_ptr = std::make_shared<TrafficDescriptorsInfo>(tds);
    if (tds_ptr == nullptr || nsi == nullptr) {
        return;
    }
    std::string packageName;
    int ret = DelayedSingleton<NetworkSliceService>::GetInstance()->GetBundleNameForUid(uid, packageName);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("Error GetBundleNameForUid fail");
        return;
    }
    if (!tds_ptr->isAtiveTriggeringApp(packageName)) {
        return;
    }
    nsi->addSignedUid(uid, tds);
}

static void FillBindParas(int netId, int urspPrecedence, std::map<std::string, std::string>& bindParas)
{
    bindParas[BIND_ROUTE_NETID] = std::to_string(netId);
    bindParas[BIND_ROUTE_URSP_PRECEDENCE] = std::to_string(urspPrecedence);
}

static void FillBindParas(const std::set<int>& uids, std::map<std::string, std::string>& bindParas)
{
    NETMGR_EXT_LOG_E("FillBindParas");
    if (uids.empty() || bindParas.empty()) {
        return;
    }
    std::string uidsStr;
    for (const auto& uid : uids) {
        uidsStr += std::to_string(uid);
        uidsStr += TrafficDescriptorsInfo::SEPARATOR;
    }
    NETMGR_EXT_LOG_E("FillBindParas uidsStr = %{public}s", uidsStr.c_str());
    bindParas[BIND_ROUTE_UID] = uidsStr;
}

int HwNetworkSliceManager::BindNetworkSliceProcessToNetwork(int uid, const std::set<int>& triggerActivationUids,
    std::shared_ptr<NetworkSliceInfo> nsi, std::shared_ptr<FqdnIps> fqdnIps,
    std::shared_ptr<TrafficDescriptorsInfo> tds)
{
    NETMGR_EXT_LOG_I("bindNetworkSliceProcessToNetwork Begin");
    if (nsi == nullptr || nsi->getNetId() == NetworkSliceInfo::INVALID_NET_ID || tds == nullptr) {
        return ERROR_INVALID_PARAM;
    }
    std::map<std::string, std::string> bindParas;
    FillBindParas(nsi->getNetId(), tds->getUrspPrecedence(), bindParas);
    NETMGR_EXT_LOG_I("bindNetworkSliceProcessToNetwork NetId = %{public}d, Uid = %{public}d, Precedence = %{public}d",
        nsi->getNetId(), uid, tds->getUrspPrecedence());
    NETMGR_EXT_LOG_I("bindNetworkSliceProcessToNetwork appid = %{public}s", tds->getAppIds().c_str());
    switch (tds->getRouteBindType()) {
        case TrafficDescriptorsInfo::UID_TDS:
            FillUidBindParas(bindParas, tds, triggerActivationUids, nsi, uid);
            break;
        case TrafficDescriptorsInfo::IP_TDS:
            NETMGR_EXT_LOG_E("TrafficDescriptorsInfo::IP_TDS");
            FillIpBindParas(bindParas, tds, fqdnIps, nsi);
            break;
        case TrafficDescriptorsInfo::UID_IP_TDS:
            NETMGR_EXT_LOG_E("TrafficDescriptorsInfo::IP_TDS");
            FillUidBindParas(bindParas, tds, triggerActivationUids, nsi, uid);
            FillIpBindParas(bindParas, tds, fqdnIps, nsi);
            break;
        case TrafficDescriptorsInfo::INVALID_TDS:
        default:
            NETMGR_EXT_LOG_E("Can not bind invalid tds");
            break;
    }
    NETMGR_EXT_LOG_I("bindNetworkSliceProcessToNetwork End");
    return BindProcessToNetwork(bindParas);
}

int HwNetworkSliceManager::BindNetworkSliceProcessToNetworkForRequestAgain(int uid,
    std::shared_ptr<NetworkSliceInfo> nsi, std::shared_ptr<FqdnIps> fqdnIps,
    std::shared_ptr<TrafficDescriptorsInfo> tds)
{
    NETMGR_EXT_LOG_I("bindNetworkSliceProcessToNetworkForRequestAgain");
    if (nsi == nullptr || nsi->getNetId() == NetworkSliceInfo::INVALID_NET_ID || tds == nullptr) {
        return ERROR_INVALID_PARAM;
    }
    if (nsi->isMatchAll()) {
        return BINDNETWORKSLICESUCCESS;
    }
    std::map<std::string, std::string> bindParas;
    FillBindParas(nsi->getNetId(), tds->getUrspPrecedence(), bindParas);

    switch (tds->getRouteBindType()) {
        case TrafficDescriptorsInfo::UID_TDS:
            FillUidBindParasForRequestAgain(bindParas, uid, nsi, tds);
            break;
        case TrafficDescriptorsInfo::IP_TDS:
            FillIpBindParas(bindParas, tds, fqdnIps, nsi);
            break;
        case TrafficDescriptorsInfo::UID_IP_TDS:
            FillUidBindParasForRequestAgain(bindParas, uid, nsi, tds);
            FillIpBindParas(bindParas, tds, fqdnIps, nsi);
            break;
        default:
            NETMGR_EXT_LOG_E("Can not bind invalid tds");
            break;
    }
    return BindProcessToNetwork(bindParas);
}

void HwNetworkSliceManager::FillUidBindParasForRequestAgain(std::map<std::string, std::string>& bindParas, int uid,
    std::shared_ptr<NetworkSliceInfo> nsi, std::shared_ptr<TrafficDescriptorsInfo> tds)
{
    std::set<int> triggerActivationUids;
    triggerActivationUids.insert(uid);
    FillBindParas(triggerActivationUids, bindParas);
    nsi->addUid(uid, *tds);
    nsi->addUsedUid(uid, *tds);
}

void HwNetworkSliceManager::FillUidBindParas(std::map<std::string, std::string>& bindParas,
    std::shared_ptr<TrafficDescriptorsInfo> tds, const std::set<int>& triggerActivationUids,
    std::shared_ptr<NetworkSliceInfo> nsi, int uid)
{
    NETMGR_EXT_LOG_E("FillUidBindParas");
    if (tds->isMatchNetworkCap()) {
        NETMGR_EXT_LOG_E("FillUidBindParas isMatchNetworkCap");
        FillBindParas(triggerActivationUids, bindParas);
        nsi->addUids(triggerActivationUids, *tds);
    } else {
        std::set<int> allUids;
        std::set<int> autoUids = GetAutoUids(tds);
        allUids.insert(autoUids.begin(), autoUids.end());
        allUids.insert(nsi->getSignedUids(*tds).begin(), nsi->getSignedUids(*tds).end());
        NETMGR_EXT_LOG_E("FillUidBindParas autoUids.size = %{public}d, allUids.size = %{public}d",
            (int)autoUids.size(), (int)allUids.size());
        FillBindParas(allUids, bindParas);
        nsi->replaceUids(*tds, autoUids);
    }

    if (tds->isMatchNetworkCap()) {
        nsi->addUsedUids(triggerActivationUids, *tds);
    } else {
        if (uid != INVALID_UID) {
            nsi->addUsedUid(uid, *tds);
        }
    }
}

void HwNetworkSliceManager::FillIpBindParas(std::map<std::string, std::string>& bindParas,
    std::shared_ptr<TrafficDescriptorsInfo> tds, std::shared_ptr<FqdnIps> fqdnIps,
    std::shared_ptr<NetworkSliceInfo> nsi)
{
    NETMGR_EXT_LOG_E("FillIpBindParas");
    if (tds->isMatchFqdn()) {
        FqdnIps newFqdnIps = *fqdnIps;
        std::shared_ptr<FqdnIps> nsiFqdnIps = nsi->getFqdnIps(*tds);
        if (nsiFqdnIps == nullptr) {
            nsi->setFqdnIps(newFqdnIps, *tds);
        } else {
            newFqdnIps = nsiFqdnIps->getNewFqdnIps(*fqdnIps);
            nsi->mergeFqdnIps(*fqdnIps, *tds);
        }
        FillIpBindParasForFqdn(bindParas, newFqdnIps);
    } else {
        FillIpBindParasForIpTriad(bindParas, tds);
    }
}

std::map<std::string, std::string> HwNetworkSliceManager::FillNetworkSliceRequest(
    std::shared_ptr<TrafficDescriptorsInfo> td)
{
    NETMGR_EXT_LOG_I("FillNetworkSliceRequest");
    std::string appId;
    int ret = DelayedSingleton<NetworkSliceService>::GetInstance()->GetBundleNameForUid(td->getUid(), appId);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("Error GetBundleNameForUid fail");
    }
    NETMGR_EXT_LOG_E("FillNetworkSliceRequest Uid = %{public}d, appId = %{public}s", td->getUid(), appId.c_str());
    appId = REQUEST_NETWORK_SLICE_OS_ID + appId;
    std::string ip = td->getIp().address_;
    std::map<std::string, std::string> bundle;

    if (!appId.empty()) {
        bundle[REQUEST_NETWORK_SLICE_APPID] = appId;
        NETMGR_EXT_LOG_I("bundle appId = %{public}s", bundle[REQUEST_NETWORK_SLICE_APPID].c_str());
    }
    if (!td->getDnn().empty()) {
        bundle[REQUEST_NETWORK_SLICE_DNN] = td->getDnn();
        NETMGR_EXT_LOG_I("bundle DNN = %{public}s", bundle[REQUEST_NETWORK_SLICE_DNN].c_str());
    }
    if (!td->getFqdn().empty()) {
        bundle[REQUEST_NETWORK_SLICE_FQDN] = td->getFqdn();
        NETMGR_EXT_LOG_I("bundle FQDN = %{public}s", bundle[REQUEST_NETWORK_SLICE_FQDN].c_str());
    }
    if (!ip.empty()) {
        bundle[REQUEST_NETWORK_SLICE_IP] = ip;
        NETMGR_EXT_LOG_I("bundle IP = %{public}s", bundle[REQUEST_NETWORK_SLICE_IP].c_str());
    }
    if (!td->getProtocolId().empty()) {
        bundle[REQUEST_NETWORK_SLICE_PROTOCOL] = td->getProtocolId();
        NETMGR_EXT_LOG_I("bundle PROTOCOL = %{public}s", bundle[REQUEST_NETWORK_SLICE_PROTOCOL].c_str());
    }
    if (!td->getRemotePort().empty()) {
        bundle[REQUEST_NETWORK_SLICE_REMOTE_PORT] = td->getRemotePort();
        NETMGR_EXT_LOG_I("bundle REMOTE_PORT = %{public}s", bundle[REQUEST_NETWORK_SLICE_REMOTE_PORT].c_str());
    }
    if (td->getCct() != 0) {
        bundle[REQUEST_NETWORK_SLICE_CONNECTION_CAPABILITY] = std::to_string(td->getCct());
        NETMGR_EXT_LOG_I("bundle CONNECTION_CAPABILITY = %{public}s",
            bundle[REQUEST_NETWORK_SLICE_CONNECTION_CAPABILITY].c_str());
    }
    return bundle;
}

void HwNetworkSliceManager::FillIpBindParasForFqdn(
    std::map<std::string, std::string>& bindParas, const FqdnIps& newFqdnIps)
{
    if (!bindParas.empty() && !newFqdnIps.getIpv4AddrAndMask().empty() && !newFqdnIps.getIpv6AddrAndPrefix().empty()) {
        bindParas[BIND_ROUTE_IPV4_NUM] = std::to_string(newFqdnIps.getIpv4Num());
        bindParas[BIND_ROUTE_IPV4_ADDRANDMASK] = ConvertUint8vecToString(newFqdnIps.getIpv4AddrAndMask());
        bindParas[BIND_ROUTE_IPV6_NUM] = std::to_string(newFqdnIps.getIpv6Num());
        bindParas[BIND_ROUTE_IPV6_ADDRANDPREFIX] = ConvertUint8vecToString(newFqdnIps.getIpv6AddrAndPrefix());
        bindParas[BIND_ROUTE_PROTOCOL_IDS] = "";
        bindParas[BIND_ROUTE_REMOTE_PORTS] = "";
    }
}

void HwNetworkSliceManager::FillIpBindParasForIpTriad(std::map<std::string, std::string>& bindParas,
    std::shared_ptr<TrafficDescriptorsInfo> tds)
{
    if (!bindParas.empty() && tds != nullptr) {
        bindParas[BIND_ROUTE_IPV4_NUM] = std::to_string(tds->getIpv4Num());
        bindParas[BIND_ROUTE_IPV4_ADDRANDMASK] = ConvertUint8vecToString(tds->getIpv4AddrAndMask());
        bindParas[BIND_ROUTE_IPV6_NUM] = std::to_string(tds->getIpv6Num());
        bindParas[BIND_ROUTE_IPV6_ADDRANDPREFIX] = ConvertUint8vecToString(tds->getIpv6AddrAndPrefix());
        bindParas[BIND_ROUTE_PROTOCOL_IDS] = tds->getProtocolIds();
        bindParas[BIND_ROUTE_REMOTE_PORTS] = tds->getRemotePorts();
    }
}

std::set<int> HwNetworkSliceManager::GetAutoUids(const std::shared_ptr<TrafficDescriptorsInfo>& tds)
{
    if (tds == nullptr) {
        return std::set<int>();
    }
    std::string uidsStr = GetUidsFromAppIds(tds->getAppIds());
    if (uidsStr.empty()) {
        NETMGR_EXT_LOG_E("GetAutoUids uidsStr.empty()");
        return std::set<int>();
    }
    std::vector<std::string> uidStrs = Split(uidsStr, SEPARATOR_FOR_NORMAL_DATA);
    if (uidStrs.empty()) {
        NETMGR_EXT_LOG_E("GetAutoUids uidStrs.empty()");
        return std::set<int>();
    }
    std::set<int> tempUids;
    for (const auto& uidStr : uidStrs) {
        if (!uidStr.empty()) {
            tempUids.insert(std::stoi(uidStr));
        }
    }
    NETMGR_EXT_LOG_E("bindNetworkSliceProcessToNetwork getUidsFromAppIds= %{public}s", uidsStr.c_str());
    return tempUids;
}

std::string HwNetworkSliceManager::GetUidsFromAppIds(const std::string& originAppIds)
{
    NETMGR_EXT_LOG_I("GetUidsFromAppIds originAppIds = %{public}s", originAppIds.c_str());
    std::set<std::string> appIds = GetAppIdsWithoutOsId(originAppIds);
    if (appIds.empty()) {
        NETMGR_EXT_LOG_E("GetUidsFromAppIds appIds.empty()");
        return "";
    }
    NETMGR_EXT_LOG_I("GetUidsFromAppIds appIds.size() = %{public}d", (int)appIds.size());
    std::string result = std::accumulate(appIds.begin(), appIds.end(), std::string{},
        [this](const std::string& current, const std::string& element) {
            return current +
            std::to_string(DelayedSingleton<NetworkSliceService>::GetInstance()->GetUidByBundleName(element)) +
            SEPARATOR_FOR_NORMAL_DATA;
        });
    NETMGR_EXT_LOG_I("getUidsFromAppIds, uids = %{public}s", result.c_str());
    return result;
}

std::set<std::string> HwNetworkSliceManager::GetAppIdsWithoutOsId(const std::string& originAppIds)
{
    NETMGR_EXT_LOG_I("GetAppIdsWithoutOsId");
    std::vector<std::string> orgins = Split(originAppIds, SEPARATOR_FOR_NORMAL_DATA);
    if (orgins.empty()) {
        NETMGR_EXT_LOG_E("getAppIdsWithoutOsId orgins == null, should not run here.");
        return std::set<std::string>();
    }
    std::set<std::string> appIds;
    for (const auto& osIdAppId : orgins) {
        NETMGR_EXT_LOG_I("GetAppIdsWithoutOsId osIdAppId = %{public}s", osIdAppId.c_str());
        HwOsAppId osAppId = HwOsAppId::Create(osIdAppId);
        NETMGR_EXT_LOG_I("GetAppIdsWithoutOsId Create OsId = %{public}s, AppId = %{public}s",
            osAppId.getOsId().c_str(), osAppId.getAppId().c_str());
        if (osAppId.getAppId() == "") {
            continue;
        }
        appIds.insert(osAppId.getAppId());
    }
    return appIds;
}

void HwNetworkSliceManager::RequestNetwork(int uid, std::shared_ptr<NetworkSliceInfo> networkSliceInfo)
{
    if (uid == INVALID_UID) {
        return;
    }
    if (networkSliceInfo == nullptr || networkSliceInfo->getNetworkRequest() == nullptr) {
        NETMGR_EXT_LOG_E("networkSliceInfo is null with no request id");
        return;
    }
    RequestNetwork(uid, networkSliceInfo, 0, REQUEST_NETWORK_TIMEOUT);
}

void HwNetworkSliceManager::RequestNetwork(int uid, std::shared_ptr<NetworkSliceInfo> networkSliceInfo,
    int requestId, int timeoutMs)
{
    NETMGR_EXT_LOG_I("HwNetworkSliceManager::RequestNetwork");
    if (networkSliceInfo == nullptr) {
        NETMGR_EXT_LOG_E("requestNetwork networkSliceInfo is null");
        return;
    }
    std::shared_ptr<RouteSelectionDescriptorInfo> rsd = networkSliceInfo->getRouteSelectionDescriptor();
    std::shared_ptr<TrafficDescriptorsInfo> tds = networkSliceInfo->getTempTrafficDescriptors();
    if (rsd == nullptr || tds == nullptr) {
        NETMGR_EXT_LOG_E("requestNetwork rsd is null or tds is null");
        return;
    }
    sptr<NetSpecifier> request = networkSliceInfo->getNetworkRequest();
    if (request == nullptr) {
        // normally it can not run here.
        NETMGR_EXT_LOG_E("Can not get request by capability:%{public}d", networkSliceInfo->getNetworkCapability());
        CleanRouteSelectionDescriptor(networkSliceInfo);
        return;
    }
    FillRsdIntoNetworkRequest(request, *rsd, *tds); // need check

    sptr<NetConnCallback> callback = networkSliceInfo->getNetworkCallback();
    if (callback == nullptr) {
        callback = sptr<NetConnCallback>(new NetConnCallback());
    }
    if (callback == nullptr) {
        return;
    }
    callback->SetNetCap(*request->netCapabilities_.netCaps_.begin());
    callback->CacheRequestUid(uid);
    callback->SetUid(uid);
    int32_t ret = NetConnClient::GetInstance().RequestNetConnection(request, callback, timeoutMs);
    networkSliceInfo->setNetworkCallback(callback);
}

void HwNetworkSliceManager::FillRsdIntoNetworkRequest(const sptr<NetSpecifier> request,
    const RouteSelectionDescriptorInfo& rsd, const TrafficDescriptorsInfo& tds)
{
    NETMGR_EXT_LOG_I("FillRsdIntoNetworkRequest");
    std::map<std::string, std::string> bundle;
    bundle["dnn"] = rsd.getDnn();
    bundle["snssai"] =  rsd.getSnssai();
    bundle["sscmode"] = std::to_string(rsd.getSscMode());
    bundle["pdusessiontype"] = std::to_string(rsd.getPduSessionType());
    bundle["routebitmap"] = std::to_string(tds.getRouteBitmap());
    NETMGR_EXT_LOG_I("FillRsdIntoNetworkRequest sscMode = %{public}d", rsd.getSscMode());
    NETMGR_EXT_LOG_I("FillRsdIntoNetworkRequest dnn = %{public}s", rsd.getDnn().c_str());
    NETMGR_EXT_LOG_I("FillRsdIntoNetworkRequest snssai = %{public}s", rsd.getSnssai().c_str());
    NETMGR_EXT_LOG_I("FillRsdIntoNetworkRequest pdusession = %{public}d", rsd.getPduSessionType());
    NETMGR_EXT_LOG_I("FillRsdIntoNetworkRequest routebitmap = %{public}d", tds.getRouteBitmap());
    networkSliceParas[*request->netCapabilities_.netCaps_.begin()] = bundle;
}

void HwNetworkSliceManager::GetRSDByNetCap(int32_t netcap, std::map<std::string, std::string>& sliceParasbyNetcap)
{
    NETMGR_EXT_LOG_I("GetRSDByNetCap");
    auto it = networkSliceParas.find(netcap);
    if (it != networkSliceParas.end()) {
        sliceParasbyNetcap = it->second;
    }
}

void HwNetworkSliceManager::ChangeNetworkSliceCounter(int changeType)
{
    switch (changeType) {
        case CHANGE_TYPE_INCREASE:
            mNetworkSliceCounter.fetch_add(1, std::memory_order_relaxed);
            break;
        case CHANGE_TYPE_DECREASE:
            mNetworkSliceCounter.fetch_sub(1, std::memory_order_relaxed);
            break;
        case CHANGE_TYPE_ZEROING:
            mNetworkSliceCounter.store(0, std::memory_order_relaxed);
            break;
        default:
            NETMGR_EXT_LOG_I("wrong type of network slice counter: %{public}d", changeType);
            break;
    }
}

bool HwNetworkSliceManager::isCanRequestNetwork()
{
    return DelayedSingleton<NetworkSliceManager>::GetInstance()->isCanRequestNetwork();
}

bool HwNetworkSliceManager::isCanMatchNetworkSlices()
{
    if (!isEnvironmentReady()) {
        return false;
    }
    if (!isCanRequestNetwork()) {
        return false;
    }
    return true;
}

bool HwNetworkSliceManager::isEnvironmentReady()
{
    if (!g_isNrSliceSupport) {
        return false;
    }
    if (!mIsReady) {
        return false;
    }
    if (!isUrspAvailable()) {
        return false;
    }
    return true;
}

bool HwNetworkSliceManager::isUrspAvailable()
{
    return mIsUrspAvailable;
}

void HwNetworkSliceManager::SetUrspAvailable(bool urspAvailable)
{
    mIsUrspAvailable = urspAvailable;
}

bool HwNetworkSliceManager::isNeedToRequestSliceForAppIdAuto(std::string appId)
{
    if (isCooperativeApp(appId)) {
        return false;
    }
    HwOsAppId id = HwOsAppId::Create(REQUEST_NETWORK_SLICE_OS_ID + appId);
    return std::find(mWhiteListForOsAppId.begin(), mWhiteListForOsAppId.end(), id)
        != mWhiteListForOsAppId.end();
}

bool HwNetworkSliceManager::isNeedToRequestSliceForFqdnAuto(std::string fqdn, int uid)
{
    if (isCooperativeApp(uid)) {
        return false;
    }
    return std::find(mWhiteListForFqdn.begin(), mWhiteListForFqdn.end(), fqdn)
        != mWhiteListForFqdn.end();
}

bool HwNetworkSliceManager::isNeedToRequestSliceForDnnAuto(std::string dnn, int uid)
{
    if (isCooperativeApp(uid)) {
        return false;
    }
    return std::find(mWhiteListForDnn.begin(), mWhiteListForDnn.end(), dnn)
        != mWhiteListForDnn.end();
}

bool HwNetworkSliceManager::isCooperativeApp(int uid)
{
    std::string packageName;
    int ret = DelayedSingleton<NetworkSliceService>::GetInstance()->GetBundleNameForUid(uid, packageName);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("Error GetBundleNameForUid fail");
        return false;
    }
    return isCooperativeApp(packageName);
}

bool HwNetworkSliceManager::isCooperativeApp(std::string packageName)
{
    return std::find(mWhiteListForCooperativeApp.begin(), mWhiteListForCooperativeApp.end(), packageName)
        != mWhiteListForCooperativeApp.end();
}

void HwNetworkSliceManager::ReadAppIdWhiteList(TrafficDescriptorWhiteList whiteList)
{
    std::string osAppIds = whiteList.osAppIds;
    if (!osAppIds.empty()) {
        mWhiteListForOsAppId.clear();
        std::vector<std::string> values = Split(osAppIds, SEPARATOR_FOR_NORMAL_DATA);
        if (values.size() != 0) {
            for (size_t i = 0; i < values.size(); ++i) {
                mWhiteListForOsAppId.push_back(HwOsAppId::Create(values[i]));
            }
        }
    }
}

void HwNetworkSliceManager::ReadDnnWhiteList(TrafficDescriptorWhiteList whiteList)
{
    std::string dnns = whiteList.dnns;
    if (!dnns.empty()) {
        mWhiteListForDnn.clear();
        std::vector<std::string> values = Split(dnns, SEPARATOR_FOR_NORMAL_DATA);
        if (values.size() != 0) {
            mWhiteListForDnn = values;
        }
    }
}

void HwNetworkSliceManager::ReadFqdnWhiteList(TrafficDescriptorWhiteList whiteList)
{
    std::string fqdns = whiteList.fqdns;
    if (!fqdns.empty()) {
        std::vector<std::string> values = Split(fqdns, SEPARATOR_FOR_NORMAL_DATA);
        if (values.size() != 0) {
            mWhiteListForFqdn = values;
        }
    }
}

void HwNetworkSliceManager::ReadCctWhiteList(TrafficDescriptorWhiteList whiteList)
{
    std::string ccts = whiteList.cct;
    if (!ccts.empty()) {
        std::vector<std::string> values = Split(ccts, SEPARATOR_FOR_NORMAL_DATA);
        if (values.size() != 0) {
            mWhiteListForCct = values;
        }
    }
}

void HwNetworkSliceManager::UnbindAllProccessToNetwork()
{
    for (auto nsi : mNetworkSliceInfos) {
        if (nsi->isMatchAll()) {
            continue;
        }
        if (nsi->getNetId() == NetworkSliceInfo::INVALID_NET_ID) {
            continue;
        }
        auto entries = nsi->getSliceRouteInfos();
        for (auto it = entries.begin(); it != entries.end(); ++it) {
            const TrafficDescriptorsInfo& tds = it->first;
            if (tds.isMatchNetworkCap()) {
                continue;
            }
            std::shared_ptr<SliceRouteInfo> sri = nsi->getSliceRouteInfo(tds);
            if (sri == nullptr) {
                continue;
            }
            UnbindUids(nsi->getNetId(), sri->getUidsStr(), tds.getUrspPrecedence());
        }
    }
}

void HwNetworkSliceManager::CleanEnvironment()
{
    int result = UnbindAllRoute();
    NETMGR_EXT_LOG_I("unbind all route, result = %{public}d", result);
    for (auto nsi : mNetworkSliceInfos) {
        sptr<NetConnCallback> networkCallback = nsi->getNetworkCallback();
        if (networkCallback != nullptr) {
            NetConnClient::GetInstance().UnregisterNetConnCallback(networkCallback);
            OnNetworkLost(networkCallback->GetNetCap(), networkCallback->GetNetId());
        }
        nsi->clear();
    }
    mNetworkSliceInfos.clear();
    InitNetworkSliceInfos();
    networkSliceParas.clear();
    NETMGR_EXT_LOG_I("Clean Environment done");
}

int HwNetworkSliceManager::UnbindAllRoute()
{
    std::map<std::string, std::string> input;
    input[BIND_ROUTE_NETID] = std::to_string(UNBIND_TYPE_ALL);
    input[UNBIND_ROUTE_TYPE] = std::to_string(UNBIND_TYPE_ALL);
    int ret = DelayedSingleton<NetworkSliceService>::GetInstance()->DelBindToNetwork(input);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("Error UnbindUids fail");
    }
    return ret;
}

int HwNetworkSliceManager::UnbindSingleNetId(int netId)
{
    std::map<std::string, std::string> input;
    input[BIND_ROUTE_NETID] = std::to_string(netId);
    input[UNBIND_ROUTE_TYPE] = std::to_string(UNBIND_TYPE_NETID);
    int ret = DelayedSingleton<NetworkSliceService>::GetInstance()->DelBindToNetwork(input);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("Error UnbindUids fail");
    }
    return ret;
}

int HwNetworkSliceManager::UnbindUids(uint netId, const std::string& uids, uint8_t urspPrecedence)
{
    std::map<std::string, std::string> input;
    input[BIND_ROUTE_NETID] = std::to_string(netId);
    input[BIND_ROUTE_UID] = uids;
    input[BIND_ROUTE_URSP_PRECEDENCE] = std::to_string(static_cast<int>(urspPrecedence));
    input[UNBIND_ROUTE_TYPE] = std::to_string(UNBIND_TYPE_UIDS);
    int ret = DelayedSingleton<NetworkSliceService>::GetInstance()->DelBindToNetwork(input);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("Error UnbindUids fail");
    }
    return ret;
}

int HwNetworkSliceManager::BindProcessToNetwork(std::map<std::string, std::string> bindParas)
{
    NETMGR_EXT_LOG_I("BindProcessToNetwork");
    int ret = DelayedSingleton<NetworkSliceService>::GetInstance()->BindToNetwork(bindParas);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("Error UnbindUids fail");
    }
    return ret;
}

int HwNetworkSliceManager::UnbindProcessToNetwork(std::string uids, int netId)
{
    std::map<std::string, std::string> input;
    input[BIND_ROUTE_NETID] = std::to_string(netId);
    input[BIND_ROUTE_UID] = uids;
    int ret = DelayedSingleton<NetworkSliceService>::GetInstance()->DelBindToNetwork(input);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("Error UnbindUids fail");
    }
    return ret;
}


void HwNetworkSliceManager::RestoreSliceEnvironment()
{
    RequestMatchAllSlice();
    TryToActivateSliceForForegroundApp();
}

void HwNetworkSliceManager::RequestMatchAllSlice()
{
    if (!mHasMatchAllSlice == true || mIsMatchAllRequsted == true || mIsMatchRequesting == true) {
        return;
    }

    if (!isCanRequestNetwork()) {
        return;
    }
    mIsMatchRequesting = true;
    for (std::shared_ptr<NetworkSliceInfo> nsi : mNetworkSliceInfos) {
        if (nsi->isMatchAll()) {
            RequestNetwork(INVALID_UID, nsi);
            break;
        }
    }
}

void HwNetworkSliceManager::OnNetworkAvailable(NetCap netCap, int32_t netId)
{
    NETMGR_EXT_LOG_I("HwNetworkSliceManager:OnNetworkAvailable");
    std::shared_ptr<NetworkSliceInfo> networkSliceInfo = GetNetworkSliceInfoByParaNetCap(netCap);
    if (networkSliceInfo == nullptr) {
        return;
    }
    networkSliceInfo->setNetId(netId);
    if (networkSliceInfo->isMatchAll()) {
        NETMGR_EXT_LOG_E("match_all do not need to bind route");
        mIsMatchAllRequsted.store(true, std::memory_order_relaxed);
        mIsMatchRequesting = false;
        return;
    }
    if (networkSliceInfo->getNetworkCallback() == nullptr) {
        NETMGR_EXT_LOG_E("getNetworkCallback() == nullptr");
        return;
    }
    int32_t uid = networkSliceInfo->getNetworkCallback()->GetUid();
    std::set<int32_t> triggerActivationUids = networkSliceInfo->getNetworkCallback()->GetRequestUids();
    for (TrafficDescriptorsInfo td : networkSliceInfo->getTrafficDescriptorsInfos()) {
        std::shared_ptr<TrafficDescriptorsInfo> tds = std::make_shared<TrafficDescriptorsInfo>(td);
        BindNetworkSliceProcessToNetwork(uid,
            triggerActivationUids,
            networkSliceInfo,
            networkSliceInfo->getFqdnIps(td),
            tds);
 
        std::shared_ptr<SliceRouteInfo> sri = networkSliceInfo->getSliceRouteInfo(td);
        if (sri == nullptr) {
            continue;
        }
        for (FqdnIps fqdnIp : sri->getWaittingFqdnIps()) {
            std::shared_ptr<FqdnIps> fqdnIps = std::make_shared<FqdnIps>(fqdnIp);
            BindNetworkSliceProcessToNetwork(uid, triggerActivationUids, networkSliceInfo, fqdnIps, tds);
        }
 
        // clear WaittingFqdnIps, because it only used in activating slice network
        networkSliceInfo->clearWaittingFqdnIps(td);
        NETMGR_EXT_LOG_I("bind success networkSliceInfo");
    }
}
 
void HwNetworkSliceManager::OnNetworkLost(NetCap netCap, int32_t netId)
{
    NETMGR_EXT_LOG_I("HwNetworkSliceManager:OnNetworkLost");
    std::shared_ptr<NetworkSliceInfo> networkSliceInfo = GetNetworkSliceInfoByParaNetCap(netCap);
    if (networkSliceInfo == nullptr) {
        NETMGR_EXT_LOG_I("OnNetworkLost networkSliceInfo == nullptr");
        return;
    }
    if (networkSliceInfo->isMatchAll()) {
        NETMGR_EXT_LOG_E("match_all do not need to bind route");
        mIsMatchAllRequsted.store(false, std::memory_order_relaxed);
        mIsMatchRequesting = false;
        return;
    }
 
    networkSliceInfo->clearUsedUids();
    int result = UnbindSingleNetId(networkSliceInfo->getNetId());
    NETMGR_EXT_LOG_I("unbind uid to network slice result = %{public}d", result);
}
 
void HwNetworkSliceManager::OnNetworkUnavailable(NetCap netCap)
{
    NETMGR_EXT_LOG_I("HwNetworkSliceManager:OnNetworkUnavailable");
    std::shared_ptr<NetworkSliceInfo> networkSliceInfo = GetNetworkSliceInfoByParaNetCap(netCap);
    if (networkSliceInfo == nullptr) {
        return;
    }
    if (networkSliceInfo->isMatchAll()) {
        NETMGR_EXT_LOG_E("match_all do not need to bind route");
        mIsMatchAllRequsted.store(false, std::memory_order_relaxed);
        mIsMatchRequesting = false;
        return;
    }
    RecoveryNetworkSlice(networkSliceInfo);
}
 
void HwNetworkSliceManager::RecoveryNetworkSlice(std::shared_ptr<NetworkSliceInfo> networkSliceInfo)
{
    if (networkSliceInfo == nullptr) {
        return;
    }
    UnbindSingleNetId(networkSliceInfo->getNetId());
    sptr<NetConnCallback> networkCallback = networkSliceInfo->getNetworkCallback();
    NetConnClient::GetInstance().UnregisterNetConnCallback(networkCallback);
    networkSliceInfo->setNetId(NetworkSliceInfo::INVALID_NET_ID);
    networkSliceInfo->clearUids();
    networkSliceInfo->clearUsedUids();
    networkSliceInfo->clear();
 
    ChangeNetworkSliceCounter(CHANGE_TYPE_DECREASE);
}

void HwNetworkSliceManager::ReleaseNetworkSlice(std::shared_ptr<NetworkSliceInfo> networkSliceInfo)
{
    if (networkSliceInfo == nullptr) {
        return;
    }
    UnbindSingleNetId(networkSliceInfo->getNetId());
    sptr<NetConnCallback> networkCallback = networkSliceInfo->getNetworkCallback();
    if (networkCallback != nullptr) {
        NetConnClient::GetInstance().UnregisterNetConnCallback(networkCallback);
        OnNetworkLost(networkCallback->GetNetCap(), networkCallback->GetNetId());
    }
    networkSliceInfo->clear();
    CleanRouteSelectionDescriptor(networkSliceInfo);
}
 
void HwNetworkSliceManager::CleanRouteSelectionDescriptor(std::shared_ptr<NetworkSliceInfo> networkSliceInfo)
{
    if (networkSliceInfo == nullptr) {
        return;
    }
    RouteSelectionDescriptorInfo rsd(0, "", "", false, 0);
    networkSliceInfo->setRouteSelectionDescriptor(rsd);
    ChangeNetworkSliceCounter(CHANGE_TYPE_DECREASE);
}

void HwNetworkSliceManager::UnbindProcessToNetworkForSingleUid(int uid,
    std::shared_ptr<NetworkSliceInfo> nsi, bool isNeedToRemoveUid)
{
    if (nsi == nullptr || nsi->isMatchAll()) {
        return;
    }
    auto entries = nsi->getSliceRouteInfos();
    for (auto iter = entries.begin(); iter != entries.end(); iter++) {
        auto tds = iter->first;
        std::shared_ptr<TrafficDescriptorsInfo> td = std::make_shared<TrafficDescriptorsInfo>(tds);
        auto uids = nsi->getUsedUids(tds);
        if (nsi->isUidRouteBindType(td) && nsi->isInUsedUids(uid, tds)) {
            if (isNeedToRemoveUid) {
                UnbindUids(nsi->getNetId(), std::to_string(uid), tds.getUrspPrecedence());
                nsi->removeUid(uid, tds);
            }
            nsi->removeUsedUid(uid, tds);
            nsi->removeSignedUid(uid, tds);
            nsi->getNetworkCallback()->RemoveRequestUid(uid);
        }
        if (nsi->isUsedUidEmpty(tds)) {
            entries.erase(iter->first);
        }
    }
    if (nsi->getSliceRouteInfos().empty()) {
        ReleaseNetworkSlice(nsi);
    }
}

void HwNetworkSliceManager::HandleUidRemoved(std::string packageName)
{
    if (!g_isNrSliceSupport) {
        return;
    }
    std::set<int> removedUids = DelayedSingleton<NetworkSliceService>::GetInstance()->GetUidsByBundleName(packageName);
    if (removedUids.empty()) {
        return;
    }
    for (int uid : removedUids) {
        for (auto nsi : mNetworkSliceInfos) {
            UnbindProcessToNetworkForSingleUid(uid, nsi, true);
        }
    }
}

void HwNetworkSliceManager::HandleUidGone(int uid)
{
    if (!g_isNrSliceSupport) {
        return;
    }
    for (auto nsi : mNetworkSliceInfos) {
        UnbindProcessToNetworkForSingleUid(uid, nsi, false);
    }
}

void HwNetworkSliceManager::ReleaseNetworkSliceByApp(int32_t uid)
{
    for (auto sliceInfo : mNetworkSliceInfos) {
        if (sliceInfo == nullptr) {
            continue;
        }

        auto tds = sliceInfo->getTrafficDescriptorsInfos();
        for (auto td : tds) {
            auto uids = sliceInfo->getUids(td);
            auto iter = std::find(uids.begin(), uids.end(), uid);
            if (iter != uids.end()) {
                UnbindProcessToNetworkForSingleUid(uid, sliceInfo, true);
            }
        }
    }
}

void HwNetworkSliceManager::BindAllProccessToNetwork()
{
    for (auto nsi : mNetworkSliceInfos) {
        if (nsi->isMatchAll()) {
            continue;
        }
        nsi->clearUsedUids();
        sptr<NetConnCallback> nsc = nsi->getNetworkCallback();
        int result = ERROR_INVALID_PARAM;
        auto entries = nsi->getSliceRouteInfos();
        for (auto it = entries.begin(); it != entries.end(); ++it) {
            const TrafficDescriptorsInfo& tds = it->first;
            if (tds.isMatchNetworkCap()) {
                // intercept request isn't rebind when wifi or dds recovery.
                continue;
            }
            if (nsc != nullptr && nsi->getNetId() != NetworkSliceInfo::INVALID_NET_ID) {
                std::shared_ptr<TrafficDescriptorsInfo> tdsptr = std::make_shared<TrafficDescriptorsInfo>(tds);
                result = BindNetworkSliceProcessToNetwork(INVALID_UID, nsc->GetRequestUids(),
                    nsi, nsi->getFqdnIps(tds), tdsptr);
            }
            NETMGR_EXT_LOG_I("bindAllProccessToNetwork, bind uid to network slice result = %{public}d", result);
        }
    }
}

void HwNetworkSliceManager::onWifiNetworkStateChanged(bool isWifiConnect)
{
    if (isWifiConnect) {
        UnbindAllProccessToNetwork();
    } else {
        RestoreSliceEnvironment();
        BindAllProccessToNetwork();
    }
}

void HwNetworkSliceManager::DumpNetworkSliceInfos()
{
    NETMGR_EXT_LOG_E("DumpNetworkSliceInfos size = %{public}d", (int)mNetworkSliceInfos.size());
    for (auto sliceInfo : mNetworkSliceInfos) {
        if (sliceInfo != nullptr) {
            std::shared_ptr<RouteSelectionDescriptorInfo> rsdinfo = sliceInfo->getRouteSelectionDescriptor();
            if (rsdinfo == nullptr) {
                NETMGR_EXT_LOG_I("DumpNetworkSliceInfos rsdinfo == nullptr");
                return;
            }
            NETMGR_EXT_LOG_E("NetworkSliceInfos SscMode = %{public}d", rsdinfo->getSscMode());
            NETMGR_EXT_LOG_E("NetworkSliceInfos Snssai = %{public}s", rsdinfo->getSnssai().c_str());
            NETMGR_EXT_LOG_E("NetworkSliceInfos Dnn = %{public}s", rsdinfo->getDnn().c_str());
            NETMGR_EXT_LOG_E("NetworkSliceInfos PduSessionType = %{public}d", rsdinfo->getPduSessionType());
            NETMGR_EXT_LOG_E("NetworkSliceInfos isMatchAll = %{public}d", rsdinfo->isMatchAll());
            return;
        }
    }
    NETMGR_EXT_LOG_E("DumpNetworkSliceInfos sliceInfo == nullptr");
}

}
}
