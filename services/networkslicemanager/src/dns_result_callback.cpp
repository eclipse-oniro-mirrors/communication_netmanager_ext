/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "dns_result_callback.h"
#include "net_conn_client.h"
#include "networksliceutil.h"
#include "hwnetworkslicemanager.h"
 
namespace OHOS::NetManagerStandard {
static constexpr char DOT = '.';
DnsResultCallback::DnsResultCallback()
{
    NETMGR_EXT_LOG_I("DnsResultCallback constructor");
}
 
DnsResultCallback::~DnsResultCallback()
{
    NETMGR_EXT_LOG_I("DnsResultCallback destroy");
}
 
void DnsResultCallback::HandleConnectivityChanged(int32_t &wifiNetId, int32_t &cellularNetId)
{
    std::list<sptr<NetHandle>> netList;
    int32_t ret = NetConnClient::GetInstance().GetAllNets(netList);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_EXT_LOG_E("HandleConnectivityChanged, GetAllNets failed ret = %d", ret);
        return;
    }
 
    if (netList.empty()) {
        NETMGR_EXT_LOG_E("HandleConnectivityChanged, no net");
        return;
    }
 
    for (auto netHandlePtr : netList) {
        if (!netHandlePtr) {
            NETMGR_EXT_LOG_E("invalid netHandle");
            continue;
        }
        NetAllCapabilities netAllCap;
        NetConnClient::GetInstance().GetNetCapabilities(*netHandlePtr, netAllCap);
 
        if (netAllCap.bearerTypes_.count(BEARER_WIFI) > 0) {
            wifiNetId = netHandlePtr->GetNetId();
        }
 
        if (netAllCap.bearerTypes_.count(BEARER_CELLULAR) > 0) {
            cellularNetId = netHandlePtr->GetNetId();
        }
    }
}
 
int32_t DnsResultCallback::GetDefaultNetId()
{
    NetHandle defaultNet;
    NetConnClient::GetInstance().GetDefaultNet(defaultNet);
    return defaultNet.GetNetId();
}
 
int32_t DnsResultCallback::OnDnsResultReport(uint32_t size,
    const std::list<NetsysNative::NetDnsResultReport> netDnsResultReport)
{
    int32_t wifiNetId = 0;
    int32_t cellularNetId = 0;
    HandleConnectivityChanged(wifiNetId, cellularNetId);
    int32_t defaultNetId = GetDefaultNetId();
    for (auto &it : netDnsResultReport) {
        int32_t netId = static_cast<int32_t>(it.netid_);
        NetBearType netType;
        int targetNetId = netId > 0 ? netId : defaultNetId > 0 ? defaultNetId : 0;
        if (wifiNetId > 0 && wifiNetId == targetNetId) {
            netType = BEARER_WIFI;
        } else if (cellularNetId > 0 && cellularNetId == targetNetId) {
            netType = BEARER_CELLULAR;
        } else {
            NETMGR_EXT_LOG_E("DnsResultCallback unknow dns result %{public}d %{public}d %{public}d %{public}d",
                netId, defaultNetId, cellularNetId, wifiNetId);
            continue;
        }
        if (it.host_.find(DOT) == std::string::npos) {
            continue;
        }
 
        std::list<AddrInfo> addrInfoList;
        for (auto &info : it.addrlist_) {
            AddrInfo addrInfo;
            addrInfo.type_ = info.type_;
            addrInfo.addr_ = info.addr_;
            addrInfoList.push_back(addrInfo);
        }
        DelayedSingleton<HwNetworkSliceManager>::GetInstance()->RequestNetworkSliceForFqdn(
            static_cast<int32_t>(it.uid_), it.host_, addrInfoList);
    }
    return NETMANAGER_SUCCESS;
}
 
bool DnsResultCallback::IsValidNetId(int32_t netId, int32_t wifiNetId, int32_t cellularNetId)
{
    return (netId == wifiNetId || netId == cellularNetId || netId == 0);
}
} // namespace OHOS
